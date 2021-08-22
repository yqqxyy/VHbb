#include "iostream"

#include "CxAODMaker/METHandler.h"

#include "CxAODMaker/MuonHandler.h"
#include "CxAODMaker/JetHandler.h"
#include "CxAODMaker/ElectronHandler.h"
#include "CxAODMaker/PhotonHandler.h"
#include "CxAODMaker/TauHandler.h"
#include "CxAODMaker/EventInfoHandler.h"

#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODEventInfo/EventInfo.h"

METHandler::METHandler(const std::string& name, ConfigStore & config, xAOD::TEvent *event, EventInfoHandler & eventInfoHandler) :
  ObjectHandlerBase(name, config, event),
  m_eventInfoHandler(eventInfoHandler),
  m_softTerm("PVSoftTrk"),
  m_selObjForMET(false),
  m_rebuiltMETforOriginal(false),
  m_jetHandler(nullptr),
  m_muonHandler(nullptr),
  m_electronHandler(nullptr),
  m_photonHandler(nullptr),
  m_tauHandler(nullptr),
  m_doRebuild(true),
  m_saveMETSoft(false),
  m_metSysTool(0)
{
}

EL::StatusCode METHandler::initializeTools() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::initializeTools()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  if(m_handlerName == "MET") m_outMETTerm = "FinalTrk";
  // Always not rebuild the met for truth met
  else if(m_handlerName == "METTruth") {m_outMETTerm = "Int"; m_doRebuild = false;}
  else if(m_handlerName == "METTrack") m_outMETTerm = "Track"; 
  else if(m_handlerName == "METFJVT") m_outMETTerm = "FJVT"; 
  else if(m_handlerName == "METMJTight") m_outMETTerm = "MJTight";
  else if(m_handlerName == "METMJMUTight") m_outMETTerm = "MJMUTight";
  else if(m_handlerName == "METMJMiddle") m_outMETTerm = "MJMiddle";
  else if(m_handlerName == "METMJLoose") m_outMETTerm = "MJLoose";

  //there is only a choice for MET!
  m_config.getif<std::string>(m_handlerName + "Term", m_outMETTerm);
  m_config.getif<bool>("saveMETSoft",m_saveMETSoft);
  
  // do not allow soft terms saving for METTrack, MJ MET, and FJVT MET
  if (m_handlerName == "METTrack" || m_handlerName.find("MJ")!=std::string::npos || m_handlerName=="METFJVT") {
    m_saveMETSoft = false;
  }

  if(m_softTerm != "PVSoftTrk") Error("METHandler::initializeTools()","m_softTerm is not set as PVSoftTrk but as %s",m_softTerm.c_str()); //Simple check

  m_config.getif<bool>("selectedObjectsForMET",m_selObjForMET);//if true, use only selected analysis objects in the met rebuilding
  m_config.getif<bool>("rebuiltMETforOriginal",m_rebuiltMETforOriginal); //if false, use MET from (D)xAOD
  
  m_eleTerm      = "RefEle";
  m_gammaTerm    = "RefGamma";
  m_tauTerm      = "RefTau";
  m_jetTerm      = "RefJet";
  m_jetTrkTerm   = "RefJetTrk";
  m_muonTerm     = "Muons";

  // the met container name depends on the jet collection
  std::string jetAlgoName;
  m_config.getif<std::string>("jetAlgoName",jetAlgoName);
  m_inputAssocMap = "METAssoc_";
  m_inputAssocMap += jetAlgoName;
  m_inputMETCoreCont = "MET_Core_";
  m_inputMETCoreCont += jetAlgoName;

  Info("METHandler::initializeTools()","Settings for rebuilding MET: ");
  Info("METHandler::initializeTools()","SoftTerm: %s",m_softTerm.c_str());
  Info("METHandler::initializeTools()","Output MET Term: %s",m_outMETTerm.c_str());

  // make sure the correct jetAlgo/METContainer combination is set in the config
  // this only applies for MET (not MET_Track)
  if(m_handlerName == "MET" &&  m_containerName.find(jetAlgoName) == std::string::npos) {
      Error("METHandler::initializeTools()", "METContainer does not match jetAlgoName in config!");
      return EL::StatusCode::FAILURE;
  }

  std::string m_significance_type;
  m_config.getif<std::string>("METSigTerm", m_significance_type); 

  // initialize the MET rebuilding tool
  m_metSignif.setTypeAndName("met::METSignificance/metSignif"+m_handlerName);
  if (m_significance_type == "Basic"){ 
    TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("SoftTermParam",met::Random));
  } else if (m_significance_type == "Hard"){ 
    TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("SoftTermParam",met::PthardParam));
  } else if (m_significance_type == "Soft"){ 
    TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("SoftTermParam",met::TSTParam));
  } else {
    Error("METHandler::initializeTools()", "METSignificance has been initialized with wrong METSigTerm in config");
    return EL::StatusCode::FAILURE;
  }
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MetSignificance
  TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("TreatPUJets",   false));
  TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("IsDataJet",   false)); // how to treat the JER for metsig, default value.
  TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("IsDataMuon",   false)); // how to treat the muon resolution for metsig, default value.
  TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("IsAFII",   false));
  TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("DoPhiReso",   true));
  TOOL_CHECK("METHandler::METSignificance", m_metSignif.setProperty("JetCollection", jetAlgoName));// support PFlowjets
  TOOL_CHECK("METHandler::METSignificance", m_metSignif.retrieve() ); // need to run retrieve instead of initialize for the handler; copy from MET

  m_metSignif_PU.setTypeAndName("met::METSignificance/metSignif_PU"+m_handlerName);
  if (m_significance_type == "Basic"){ 
    TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("SoftTermParam",met::Random));
  } else if (m_significance_type == "Hard"){ 
    TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("SoftTermParam",met::PthardParam));
  } else if (m_significance_type == "Soft"){ 
    TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("SoftTermParam",met::TSTParam));
  } else {
    Error("METHandler::initializeTools()", "METSignificance has been initialized with wrong METSigTerm in config");
    return EL::StatusCode::FAILURE;
  }
  TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("TreatPUJets",   true));
  TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("IsDataJet",   false)); // how to treat the JER for metsig, default value.
  TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("IsDataMuon",   false)); // how to treat the muon resolution for metsig, default value.
  TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("IsAFII",   false));
  TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("DoPhiReso",   true));
  TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.setProperty("JetCollection", jetAlgoName));// support PFlowjets
  TOOL_CHECK("METHandler::METSignificance", m_metSignif_PU.retrieve() ); // need to run retrieve instead of initialize for the handler; copy from MET


  // initialize the MET rebuilding tool
  if(m_debug) Info("METHandler::initializeTools()", m_handlerName.c_str());
  m_metMaker.setTypeAndName("met::METMaker/metMaker"+m_handlerName);
  m_metSysTool = new met::METSystematicsTool("METSystematicsTool_"+m_handlerName);
  TOOL_CHECK("METHandler::METMaker",   m_metMaker.setProperty("DoRemoveMuonJets", true));
  TOOL_CHECK("METHandler::METMaker",   m_metMaker.setProperty("DoSetMuonJetEMScale", true));
  TOOL_CHECK("METHandler::METMaker",   m_metMaker.setProperty("ORCaloTaggedMuons", true));
  TOOL_CHECK("METHandler::METMaker",   m_metMaker.setProperty("JetRejectionDec","passFJVTTight"));
  if (jetAlgoName.find("EMPFlow")!=std::string::npos) {
    TOOL_CHECK("METHandler::METMaker",   m_metMaker.setProperty("DoPFlow", true));
  } // see recomandations from https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EtmissRecommendationsRel21p2
  TOOL_CHECK("METHandler::METHandler", m_metMaker.retrieve() ); // need to run retrieve instead of initialize for the handler

  if (m_handlerName == "METTrack"){ //Configure the track met if we use it
    TOOL_CHECK("METHandler::METHandler", m_metSysTool->setProperty("ConfigPrefix",  "METUtilities/data16_13TeV/rec_Dec16v1/")); // set last recomandation path
    TOOL_CHECK("METHandler::METHandler", m_metSysTool->setProperty("ConfigJetTrkFile",  "JetTrackSyst.config")); // additional systs
  } else{ // For the rest, use the latest config
    TOOL_CHECK("METHandler::METHandler", m_metSysTool->setProperty("ConfigPrefix",  "METUtilities/data17_13TeV/prerec_Jan16/")); // set last recomandation path
    TOOL_CHECK("METHandler::METHandler", m_metSysTool->setProperty("ConfigSoftCaloFile",  "")); // no softCalo in case TrkSoftTerm
    TOOL_CHECK("METHandler::METHandler", m_metSysTool->setProperty("ConfigSoftTrkFile",  "TrackSoftTerms.config"));
    if (jetAlgoName.find("EMPFlow")!=std::string::npos) { // set the proper pflow folder
      TOOL_CHECK("METHandler::METHandler", m_metSysTool->setProperty("ConfigSoftTrkFile", "TrackSoftTerms-pflow.config"));
    }
  }

  m_metSysTool->msg().setLevel(m_msgLevel);
  TOOL_CHECK( "METHandler::METHandler",m_metSysTool->initialize());

  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear();
  m_sysToolList.push_back(m_metSysTool);

  return EL::StatusCode::SUCCESS;
}

METHandler::~METHandler() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::~METHandler()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  if (m_metSysTool) {
    delete m_metSysTool;
    m_metSysTool = nullptr;
  }
}

EL::StatusCode METHandler::addParticleVariations() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::addParticleVariations()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  if (!m_doRebuild) {
    return EL::StatusCode::SUCCESS;
  }

  const bool skipWeightVar = true;
  if (m_muonHandler)     EL_CHECK("addCPVariations",addCPVariations(m_muonHandler     -> getAllVariations(false), false, skipWeightVar));
  if (m_electronHandler) EL_CHECK("addCPVariations",addCPVariations(m_electronHandler -> getAllVariations(false), false, skipWeightVar));
  if (m_handlerName != "METTrack") {
    if (m_jetHandler)      EL_CHECK("addCPVariations",addCPVariations(m_jetHandler      -> getAllVariations(false), false, skipWeightVar));
    if (m_photonHandler)   EL_CHECK("addCPVariations",addCPVariations(m_photonHandler   -> getAllVariations(false), false, skipWeightVar));
    if (m_tauHandler)      EL_CHECK("addCPVariations",addCPVariations(m_tauHandler      -> getAllVariations(false), false, skipWeightVar));
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode METHandler::setObjects() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::setObjects()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  // rebuilding has to be done after all other objects are calibrated
  // -> setMET() needs to be called from AnalysisBase separately
  if (m_doRebuild) {
    return EL::StatusCode::SUCCESS;
  }

  // otherwise we can set MET here (e.g. METTrack, METTruth)
  return setMET();
}

EL::StatusCode METHandler::setMET() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::setMET()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  // disable rebuilding if no particle collection is present
  if(!m_electronHandler && !m_photonHandler && !m_tauHandler && !m_muonHandler && !m_jetHandler){
    m_doRebuild = false;
  }

  // retrieve input object container
  const xAOD::MissingETContainer * constContainer = nullptr;
  if(m_handlerName == "MET" || m_handlerName == "METTruth" || m_handlerName == "METTrack"){
    TOOL_CHECK("METHandler::setMET()", m_event->retrieve(constContainer, m_containerName));
  }

  // clear containers
  m_inContainer.clear();
  m_outContainer.clear();
  m_metSig.clear();
  m_metSig_PU.clear();
  m_metOverSqrtSumET.clear();
  m_metOverSqrtHT.clear();

  // create new containers for all systematic variations including "Nominal" and "Original"
  std::vector<TString> listOfContainers = getAllVariations(true);
  int isyst=0; // used for random seed for soft term resolution systematics
  for (const TString & sysName : listOfContainers) {
    if (m_msgLevel == MSG::DEBUG) {
      Info("METHandler::setMET()", "Variation '%s'.", sysName.Data());
    }

    if ((m_doRebuild && sysName != "Original") ||
	(m_doRebuild && (sysName == "Original") && m_rebuiltMETforOriginal)) {
      xAOD::MissingETContainer * inContainer = new xAOD::MissingETContainer();
      xAOD::AuxContainerBase * inContainerAux = (xAOD::AuxContainerBase*) new xAOD::MissingETAuxContainer();
      inContainer->setStore(inContainerAux);
      m_inContainer[sysName] = inContainer;

      EL_CHECK("calibrateMET",calibrateMET(m_inContainer[sysName], sysName, isyst));
    }
    else {//if no rebuilding, take MET from input file
      m_inContainer[sysName]  =  (xAOD::MissingETContainer *)constContainer;
    }

    isyst++;
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode METHandler::calibrateMET(xAOD::MissingETContainer* METCont, TString sysName, int isyst){

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::calibrateMET()", "Called for variation '%s'.", sysName.Data());
  }

  xAOD::ElectronContainer * electrons = nullptr;
  xAOD::PhotonContainer   * photons   = nullptr;
  xAOD::TauJetContainer   * taus      = nullptr;
  xAOD::MuonContainer     * muons     = nullptr;
  xAOD::JetContainer      * jets      = nullptr;

  if (m_electronHandler) electrons = m_electronHandler -> getInParticleVariation(sysName);
  if (m_photonHandler)   photons   = m_photonHandler   -> getInParticleVariation(sysName);
  if (m_tauHandler)      taus      = m_tauHandler      -> getInParticleVariation(sysName);
  if (m_muonHandler)     muons     = m_muonHandler     -> getInParticleVariation(sysName);
  if (m_jetHandler)      jets      = m_jetHandler      -> getInParticleVariation(sysName);

  //needed for rebuilding MET
  const xAOD::MissingETAssociationMap* metMap = nullptr;
  const xAOD::MissingETContainer* inputMETCoreContainer = nullptr;
  TOOL_CHECK("METHandler::calibrateMET()",m_event->retrieve(metMap, m_inputAssocMap));
  TOOL_CHECK("METHandler::calibrateMET()",m_event->retrieve(inputMETCoreContainer, m_inputMETCoreCont));

  metMap->resetObjSelectionFlags(); // must be called for systematic calculations

  if (electrons) {
    if (m_debug) Info("METHandler::calibrateMET()", "Rebuilding electron term.");
    ConstDataVector<xAOD::ElectronContainer> metEle(SG::VIEW_ELEMENTS);
    for(const auto& ele : *electrons) {
      if(m_handlerName == "METMJTight"){
      	// Definition of WHsignalElectron is changed by isMJ flag, so cannot use this flag for MET rebuild
      	////int passSel = 0;
      	////Props::isWHSignalElectron.get(ele, passSel);
      	////Props::passSel.get(ele, passSel);
      	////if(passSel && Props::isTightLH.get(ele)) metEle.push_back(ele);
      	if((!m_selObjForMET || Props::forMETRebuild.get(ele))&& Props::isTightLH.get(ele) && Props::isFixedCutTightIso.get(ele)) metEle.push_back(ele);
      }else if(m_handlerName == "METMJMiddle"){
      	int passMJSel = 0;
      	Props::passMJSel.get(ele, passMJSel);
      	if(passMJSel) metEle.push_back(ele);
      }else if(m_handlerName == "METMJLoose"){ // using only the signal leptons
      	//if(Props::passPreSel.get(ele)) metEle.push_back(ele);
      	int passSel = 0;
      	Props::passSel.get(ele, passSel);
      	if(passSel) metEle.push_back(ele);
      }else{
	       if(!m_selObjForMET || Props::forMETRebuild.get(ele)) metEle.push_back(ele);
      }
    }
    if (m_debug) Info("METHandler::calibrateMET()", "Using %lu of %lu electrons, start rebuilding...", metEle.size(), electrons->size());
    EL_CHECK("METHandler::calibrateMET()",
             m_metMaker->rebuildMET(m_eleTerm, xAOD::Type::Electron, METCont,  metEle.asDataVector(), metMap));
  }
  if (photons) {
    if (m_debug) Info("METHandler::calibrateMET()", "Rebuilding photon term.");
    ConstDataVector<xAOD::PhotonContainer> metPho(SG::VIEW_ELEMENTS);
    for(const auto& pho : *photons) {
      if(!m_selObjForMET || Props::forMETRebuild.get(pho)) metPho.push_back(pho);
    }
    if (m_debug) Info("METHandler::calibrateMET()", "Using %lu of %lu photons, start rebuilding...", metPho.size(), photons->size());
    EL_CHECK("METHandler::calibrateMET()",
             m_metMaker->rebuildMET(m_gammaTerm, xAOD::Type::Photon, METCont, metPho.asDataVector(), metMap));
  }
  if (taus) {
    if (m_debug) Info("METHandler::calibrateMET()", "Rebuilding tau term.");
    ConstDataVector<xAOD::TauJetContainer> metTau(SG::VIEW_ELEMENTS);
    for(const auto& tau : *taus) {
      if(!m_selObjForMET || Props::forMETRebuild.get(tau)) {
      	if (m_debug) std::cout << "HH: MET tau " << tau << std::endl;
      	metTau.push_back(tau);
      }
    }
    if (m_debug) Info("METHandler::calibrateMET()", "Using %lu of %lu taus, start rebuilding...", metTau.size(), taus->size());
    EL_CHECK("METHandler::calibrateMET()",
             m_metMaker->rebuildMET(m_tauTerm, xAOD::Type::Tau, METCont, metTau.asDataVector(), metMap));
  }
  if (muons) {
    if (m_debug) Info("METHandler::calibrateMET()", "Rebuilding muon term.");
    ConstDataVector<xAOD::MuonContainer> metMuo(SG::VIEW_ELEMENTS);
    for(const auto& muo : *muons) {
      if(m_handlerName == "METMJMUTight"){
	     if((!m_selObjForMET || Props::forMETRebuild.get(muo))&& (Props::muonQuality.get(muo) < 2) && Props::isFixedCutTightTrackOnlyIso.get(muo)) metMuo.push_back(muo);
      }
      else{
	     if(!m_selObjForMET || Props::forMETRebuild.get(muo)) metMuo.push_back(muo);
      }
    }
    if (m_debug) Info("METHandler::calibrateMET()", "Using %lu of %lu muons, start rebuilding...", metMuo.size(), muons->size());
    EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildMET(
            m_muonTerm, xAOD::Type::Muon, METCont, metMuo.asDataVector(), metMap));
  }

  if (jets) {//all jets are always added, EtMiss group knows what to do with them
    if (m_debug) Info("METHandler::calibrateMET()", "Rebuilding jet term.");
    if(m_handlerName == "MET"){
      if (m_debug) Info("METHandler::calibrateMET()", "...for jet met.");
      EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildJetMET(
              m_jetTerm, m_softTerm, METCont, jets, inputMETCoreContainer, metMap, true));
    }
    else if(m_handlerName == "METFJVT"){
      if (m_debug) Info("METHandler::calibrateMET()", "...for forward jet tagging MET.");
      EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildJetMET(
              m_jetTerm, m_softTerm, METCont, jets, inputMETCoreContainer, metMap, true));
    }
    else if(m_handlerName == "METMJTight"){
      if (m_debug) Info("METHandler::calibrateMET()", "...for jet met MJ tight.");
      EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildJetMET(
              m_jetTerm, m_softTerm, METCont, jets, inputMETCoreContainer, metMap, true));
    }
    else if(m_handlerName == "METMJMUTight"){
      if (m_debug) Info("METHandler::calibrateMET()", "...for jet met MJ MU tight.");
      EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildJetMET(
              m_jetTerm, m_softTerm, METCont, jets, inputMETCoreContainer, metMap, true));
    }
    else if(m_handlerName == "METMJMiddle"){
      if (m_debug) Info("METHandler::calibrateMET()", "...for jet met MJ middle.");
      EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildJetMET(
              m_jetTerm, m_softTerm, METCont, jets, inputMETCoreContainer, metMap, true));
    }
    else if(m_handlerName == "METMJLoose"){
      if (m_debug) Info("METHandler::calibrateMET()", "...for jet met MJ loose.");
      EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildJetMET(
              m_jetTerm, m_softTerm, METCont, jets, inputMETCoreContainer, metMap, true));
    }
    else if(m_handlerName == "METTrack" ) { //METTrack - only use TST soft term
      if (m_debug) Info("METHandler::calibrateMET()", "...for track met.");
      EL_CHECK("METHandler::calibrateMET()", m_metMaker->rebuildTrackMET(
              m_jetTrkTerm, m_softTerm, METCont, jets, inputMETCoreContainer, metMap, true));
    }
  }

  ///////////// MET SPECIFIC SYS //////////
  //Make a sysSet with only one sys name (because applySystematicVariation wants it)
  CP::SystematicSet iSysSet( sysName.Data());

  //If the sys is good for the METSysTool it will be used to vary the soft term
  if(m_metSysTool->isAffectedBySystematic(*iSysSet.begin())){
    // NB: also if the sys has no effect, the soft term rebuilding will change its values
    //     a bit. TODO: cross check with EtMiss that this is not a bug!
    int RunNumber=m_eventInfoHandler.get_mcChannelNumber();
    unsigned long long EventNumber=m_eventInfoHandler.get_mcEventNumber();
    m_metSysTool->setRandomSeed(EventNumber*10000*(isyst+1)+RunNumber);

    CP_CHECK("METHandler::calibrateMET()", m_metSysTool->applySystematicVariation(iSysSet), m_debug);

    xAOD::MissingET * softTrkMet = (*METCont)["PVSoftTrk"];
    assert( softTrkMet != nullptr); //check we retrieved the soft trk term
    CP_CHECK("METHandler::calibrateMET()", m_metSysTool->applyCorrection(*softTrkMet), m_debug);

    if (m_handlerName == "METTrack" && jets!=nullptr ){
      xAOD::MissingET * jetTrkMet = (*METCont)["RefJetTrk"];
      assert( jetTrkMet != nullptr); //check we retrieved the trk term
      CP_CHECK("METHandler::calibrateMET()", m_metSysTool->applyCorrection(*jetTrkMet, metMap), m_debug);
    }
  }


  ///////    Build sum   //////
  EL_CHECK("METHandler::calibrateMET()", m_metMaker->buildMETSum(m_outMETTerm, METCont,(*METCont)[m_softTerm]->source()));
  ///////  Build metsig  //////
  EL_CHECK("METHandler::calibrateMET()", m_metSignif->varianceMET(METCont, m_eventInfoHandler.get_averageInteractionsPerCrossing(), "RefJet", "PVSoftTrk",m_outMETTerm));
  m_metSig[sysName]=m_metSignif->GetSignificance();
  m_metOverSqrtSumET[sysName]=m_metSignif->GetMETOverSqrtSumET();
  m_metOverSqrtHT[sysName]=m_metSignif->GetMETOverSqrtHT();
  EL_CHECK("METHandler::calibrateMET()", m_metSignif_PU->varianceMET(METCont, m_eventInfoHandler.get_averageInteractionsPerCrossing(), "RefJet", "PVSoftTrk",m_outMETTerm));
  m_metSig_PU[sysName]=m_metSignif_PU->GetSignificance();
    
  if (m_msgLevel == MSG::DEBUG) {
    double rebuilt_mpx = (*METCont)[m_outMETTerm]->mpx();
    double rebuilt_mpy = (*METCont)[m_outMETTerm]->mpy();
    Info("METHandler::calibrateMET()", "Rebuilt MET: Missing Et (x,y): (%f, %f,) met %f",
            rebuilt_mpx, rebuilt_mpy, (*METCont)[m_outMETTerm]->met());
  }

  return EL::StatusCode::SUCCESS;
}

void METHandler::printMET(const xAOD::MissingETContainer* METCont) {

  std::cout << " Missing Et (x,y) " << m_outMETTerm << ": (" << (*METCont)[m_outMETTerm]->mpx() << "," <<  (*METCont)[m_outMETTerm]->mpy() << ")" << std::endl;
  std::cout << "      " << m_eleTerm << ": (" << (*METCont)[m_eleTerm]->mpx() << "," <<  (*METCont)[m_eleTerm]->mpy() << ")" << std::endl;
  std::cout << "      " << m_muonTerm << ": (" << (*METCont)[m_muonTerm]->mpx() << "," <<  (*METCont)[m_muonTerm]->mpy() << ")" << std::endl;
  std::cout << "      " << m_jetTerm << ": (" << (*METCont)[m_jetTerm]->mpx() << "," <<  (*METCont)[m_jetTerm]->mpy() << ")" << std::endl;
  std::cout << "      " << m_gammaTerm << ": (" << (*METCont)[m_gammaTerm]->mpx() << "," <<  (*METCont)[m_gammaTerm]->mpy() << ")" << std::endl;
  std::cout << "      " << m_tauTerm << ": (" << (*METCont)[m_tauTerm]->mpx() << "," <<  (*METCont)[m_tauTerm]->mpy() << ")" << std::endl;
  std::cout << "      " << m_softTerm << ": (" << (*METCont)[m_softTerm]->mpx() << "," <<  (*METCont)[m_softTerm]->mpy() << ")" << std::endl;

}

xAOD::MissingET* METHandler::getMET(const TString &variation) {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::getMET()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  xAOD::MissingETContainer* metCont = nullptr;
  if (m_inContainer.count(variation)) {
    metCont = m_inContainer[variation];
  } else if (m_inContainer.count("Nominal")) {
    metCont = m_inContainer["Nominal"];
  } else {
    return nullptr;
  }

  xAOD::MissingETContainer::const_iterator met_it = metCont->find(m_outMETTerm);
  if(met_it == metCont->end()) {
    return nullptr;
  }
  return *met_it;
}


void METHandler::copyContainer(xAOD::MissingETContainer* inCont, xAOD::MissingETContainer* outCont, bool isNominal) {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::copyContainer()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  xAOD::MissingETContainer::const_iterator met_it = inCont->find(m_outMETTerm);
  if(met_it == inCont->end()) {
    Warning("METHandler::copyContainer()", "Input container has no entry '%s'!", m_outMETTerm.c_str());
    return;
  }
  xAOD::MissingET* inMET = *met_it;
  xAOD::MissingET* outMET = nullptr;
  if (isNominal) {
    outMET = new xAOD::MissingET();
    outCont->push_back(outMET);
  } else {
    outMET     = outCont->at(0);
  }

  if (m_handlerName != "METTruth"){
      outMET -> setMpx(inMET -> mpx());
      outMET -> setMpy(inMET -> mpy());
      if (isNominal) outMET -> setSumet(inMET -> sumet()); 
  }

  else {

  // There a 4 definition for the Truth MET:
  //NonInt - all stable, non-interacting particles including neutrinos, SUSY LSPs, Kaluza-Klein particles etc.
  //Int - all stable, interacting particles within detector acceptance (|eta|<5) excluding muons (approximate calo term)
  //IntOut - all stable, interacting particles outside detector acceptance (|eta|>5)
  //IntMuons - all final state muons in |eta|<5 and with pt>6 GeV

  //the truth MET is calculated as being the sum of 'Int' and 'IntMuons' informations, to be coherent with the definiton of the Truth MET in the TruthFramework 

    xAOD::MissingETContainer::const_iterator met_second_it = inCont->find("IntMuons");

    if(met_second_it == inCont->end()) {
      Warning("METHandler::copyContainer()", "Input container has no entry 'IntMuons' !");
      return;
    }

    xAOD::MissingET* inMET_second = *met_second_it;

    outMET -> setMpx(inMET -> mpx() + inMET_second -> mpx());
    outMET -> setMpy(inMET -> mpy() + inMET_second -> mpy());
    if (isNominal) outMET -> setSumet(inMET -> sumet() + inMET_second -> sumet()); 
  }

  if (isNominal) {
    if ( m_handlerName == "MET" ){
      Props::metSig.set(outMET, m_metSig["Nominal"]);
      Props::metOverSqrtSumET.set(outMET, m_metOverSqrtSumET["Nominal"]);
      Props::metOverSqrtHT.set(outMET, m_metOverSqrtHT["Nominal"]);
      Props::metSig_PU.set(outMET, m_metSig_PU["Nominal"]);
    }
  }

  // decorate soft terms to nominal MET
  xAOD::MissingETContainer::const_iterator met_soft_it = inCont->find(m_softTerm);
  if(m_saveMETSoft && met_soft_it!=inCont->end() && isNominal){
    Props::soft_mpx.set(outMET, (*met_soft_it) -> mpx());
    Props::soft_mpy.set(outMET, (*met_soft_it) -> mpy());
    Props::soft_sumet.set(outMET, (*met_soft_it) -> sumet());
  }
}

EL::StatusCode METHandler::fillOutputContainer() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::fillOutputContainer()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  // fill nominal container
  xAOD::MissingETContainer* outContainerNominal = new xAOD::MissingETContainer();
  xAOD::AuxContainerBase* outContainerNominalAux = new xAOD::AuxContainerBase();
  outContainerNominal->setStore(outContainerNominalAux);
  copyContainer(m_inContainer["Nominal"], outContainerNominal, true);
  if (outContainerNominal->size() == 0) {
    Error("METHandler::fillOutputContainer()", "Output container is empty!");
    return EL::StatusCode::FAILURE;
  }
  if (m_msgLevel == MSG::DEBUG) Info("METHandler::fillOutputContainer()", "MET sig.: %f in Nominal .", m_metSig["Nominal"] );
  if (m_msgLevel == MSG::DEBUG) Info("METHandler::fillOutputContainer()", "MET sig. w/ PU: %f in Nominal .", m_metSig_PU["Nominal"] );

  m_outContainer["Nominal"] = outContainerNominal;
  EL_CHECK("METHandler::fillOutputContainer()",
          m_event->record(outContainerNominal, m_containerName + "___Nominal"));
  EL_CHECK("METHandler::fillOutputContainer()",
          m_event->record(outContainerNominalAux, m_containerName + "___NominalAux."));

  // create shallow copies for each syst variation + Original
  std::vector<TString> listOfContainers = getAllVariations(false);
  if (m_handlerName.find("MJ")==std::string::npos && m_handlerName!="METFJVT"){ //Do not need to save out MJ MET containers with systematic variations
    for (const TString & sysName : listOfContainers) {
      std::pair<xAOD::MissingETContainer*, xAOD::ShallowAuxContainer*> outContainerSC = xAOD::shallowCopyContainer(*outContainerNominal);
      copyContainer(m_inContainer[sysName], outContainerSC.first, false);
      if ( m_handlerName == "MET" ){
        	Props::metSig.set((outContainerSC.first)->at(0), m_metSig[sysName]);
      }
      if (m_msgLevel == MSG::DEBUG) Info( "METHandler::fillOutputContainer()", "MET sig.: %f in %s .", m_metSig[sysName], sysName.Data() );

      // add container to output objects
      m_outContainer[sysName] = outContainerSC.first;
      if( sysName.Contains("MET_") ){
        TString tempName=sysName;
        tempName.ReplaceAll("ResoCorr", "ResoCorr__1up");
        tempName.ReplaceAll("ResoPara", "ResoPara__1up");
        tempName.ReplaceAll("ResoPerp", "ResoPerp__1up");
        tempName.ReplaceAll("ScaleUp", "Scale__1up");
        tempName.ReplaceAll("ScaleDown", "Scale__1down");
        EL_CHECK("METHandler::fillOutputContainer()",
	         m_event->record(outContainerSC.first, m_containerName + "___" + tempName.Data()));
        EL_CHECK("METHandler::fillOutputContainer()",
	         m_event->record(outContainerSC.second, m_containerName + "___" + tempName.Data() + "Aux."));
      }else{
        EL_CHECK("METHandler::fillOutputContainer()",
	         m_event->record(outContainerSC.first, m_containerName + "___" + sysName.Data()));
        EL_CHECK("METHandler::fillOutputContainer()",
  	         m_event->record(outContainerSC.second, m_containerName + "___" + sysName.Data() + "Aux."));
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode METHandler::clearEvent() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::clearEvent()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  // no containers have been created w/o rebuilding
  if (!m_doRebuild) {
    return EL::StatusCode::SUCCESS;
  }

  // delete containers used for calibration
  for (std::pair<TString, xAOD::MissingETContainer*> metSet : m_inContainer) {
    // skip Original if not rebuilt
    if (metSet.first == "Original" && !m_rebuiltMETforOriginal){
      continue;
    }

    if (!metSet.second) continue;

    // delete the aux container associated to the met container
    delete metSet.second->getConstStore();
    metSet.second->setConstStore(nullptr);
    // delete the met container
    delete metSet.second;
    metSet.second = nullptr;
  }

  // clear containers
  m_inContainer.clear();
  m_outContainer.clear();

  return EL::StatusCode::SUCCESS;
}


xAOD::MissingETContainer* METHandler::getParticleVariation(std::map<TString, xAOD::MissingETContainer *> &container, const TString &variation)
{

  if (m_msgLevel == MSG::DEBUG) {
    Info("METHandler::getParticleVariation()", "Called for variation '%s'.", variation.Data());
  }

  if (container.count(variation)) {
    return container[variation];
  }

  if (container.count("Nominal")) {
    return container["Nominal"];
  }

  return nullptr;

}
