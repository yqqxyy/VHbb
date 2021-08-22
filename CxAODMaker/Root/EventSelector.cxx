#include "CxAODMaker/EventSelector.h"
#include "CxAODMaker/ObjectHandlerBase.h"
#include "CxAODMaker/OverlapRemover.h"

#include "CxAODTools/OverlapRemoval.h"
#include "CxAODTools/OverlapRegisterAccessor.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODTools/ConfigStore.h"
#include "CxAODTools/EventSelection.h"
#include "CxAODTools/CutFlowCounter.h"


#include "CxAODMaker/ElectronHandler.h"
#include "CxAODMaker/ForwardElectronHandler.h"
#include "CxAODMaker/PhotonHandler.h"
#include "CxAODMaker/MuonHandler.h"
#include "CxAODMaker/TauHandler.h"
#include "CxAODMaker/DiTauJetHandler.h"
#include "CxAODMaker/JetHandler.h"
#include "CxAODMaker/FatJetHandler.h"
#include "CxAODMaker/TrackJetHandler.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "CxAODMaker/TruthEventHandler.h"
#include "CxAODMaker/TruthProcessor.h"
#include "CxAODMaker/METHandler.h"

#include <iterator>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <utility>

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"
#include "IsolationSelection/IIsolationSelectionTool.h"
#include "IsolationSelection/IIsolationCloseByCorrectionTool.h"

EventSelector::EventSelector(ConfigStore& config) :
  m_config(config),
  m_debug(false),
  m_doCloseByIsoCorr(false),
  m_jets(nullptr),
  m_fatjets(nullptr),
  m_fatjetsAlt(nullptr),
  m_trackjets(nullptr),
  m_subjets(nullptr),
  m_muons(nullptr),
  m_taus(nullptr),
  m_ditaus(nullptr),
  m_electrons(nullptr),
  m_photons(nullptr),
  m_met(nullptr),
  m_metTrack(nullptr),
  m_metMJTight(nullptr),
  m_metFJVT(nullptr),
  m_metMJMUTight(nullptr),
  m_metMJMiddle(nullptr),
  m_metMJLoose(nullptr),
  m_info(nullptr),
  m_truthElectrons(nullptr),
  m_truthMuons(nullptr),
  m_truthNeutrinos(nullptr),
  m_doOR(true),
  m_selection(nullptr),
  m_overlapRegAcc(nullptr),
  m_systematics(),
  m_isFirstCall(true),
  m_trig_sfmuon(nullptr)
{
  m_config.getif<bool>("removeOverlap", m_doOR);
  m_OR=new OverlapRemover(config);
  m_config.getif<bool>("debug", m_debug);
  m_config.getif<bool>("doCloseByIsoCorr", m_doCloseByIsoCorr);

  // Recognize weight variations ! 
  std::vector<std::string> weightVariations;
  m_config.getif< std::vector<std::string> >("weightVariations", weightVariations);
  std::vector<std::string> pulls = {"1down", "1up"};
  for (std::string name : weightVariations) {
    for (std::string pull : pulls) {
      m_weightVariations.push_back((name + "__" + pull).c_str());
    }
  }
}

EventSelector::~EventSelector() {
  // we are the owner of the EventSelection object
  delete m_selection;
  delete m_OR;
  delete m_trig_sfmuon;
}

EL::StatusCode EventSelector::initialize()
{
  Info("EventSelector::initialize()", "Initialize muon trigger SF access.");

  m_trig_sfmuon = new CP::MuonTriggerScaleFactors ("TrigSFClass");
  TOOL_CHECK("TriggerTool::initialize muon trig SF", m_trig_sfmuon->setProperty("MuonQuality","Loose"));
  TOOL_CHECK("TriggerTool::initialize muon trig SF", m_trig_sfmuon->initialize());

  if (m_doCloseByIsoCorr) {
    m_isoTool.setTypeAndName("CP::IsolationSelectionTool/IsoTool");
    std::vector<std::string> elWPs;
    std::vector<std::string> muWPs;
    elWPs.push_back("FCTight");
    elWPs.push_back("FCLoose");
    elWPs.push_back("FCTight_FixedRad");
    elWPs.push_back("FCLoose_FixedRad");
    elWPs.push_back("Gradient");
    elWPs.push_back("FCHighPtCaloOnly");

    muWPs.push_back("FCTight");
    muWPs.push_back("FCLoose");
    muWPs.push_back("FCTightTrackOnly");
    muWPs.push_back("FCTight_FixedRad");
    muWPs.push_back("FCLoose_FixedRad");
    muWPs.push_back("FCTightTrackOnly_FixedRad");
    muWPs.push_back("FixedCutHighPtTrackOnly");
    muWPs.push_back("FixedCutPflowTight");
    muWPs.push_back("FixedCutPflowLoose");

    TOOL_CHECK("IsolationSelectionTool::retrieve",  m_isoTool.setProperty("ElectronWPVec", elWPs) );
    TOOL_CHECK("IsolationSelectionTool::retrieve",  m_isoTool.setProperty("MuonWPVec",     muWPs) );
    TOOL_CHECK("IsolationSelectionTool::retrieve",  m_isoTool.retrieve() );

    m_isoCloseByTool.setTypeAndName("CP::IsolationCloseByCorrectionTool/IsoCloseByTool");
    TOOL_CHECK("IsolationCloseByCorrectionTool::retrieve",  m_isoCloseByTool.setProperty("IsolationSelectionTool", m_isoTool) );
    TOOL_CHECK("IsolationCloseByCorrectionTool::retrieve",  m_isoCloseByTool.retrieve() );
  }

  return EL::StatusCode::SUCCESS;

}

void EventSelector::setOverlapRegisterAccessor(OverlapRegisterAccessor * overlapRegAcc) {
  m_overlapRegAcc = overlapRegAcc;
}

void EventSelector::setOverlapRemoval(OverlapRemoval* OR) {
  m_OR->setOverlapRemoval(OR);
}

void EventSelector::setJets(JetHandler* jets) {
  m_jets = jets;
  m_OR->setJets(jets);
  if(!jets) return;
  if(m_doOR) jets->useForOR(true);
}

void EventSelector::setFatJets(FatJetHandler* jets) {
  m_fatjets = jets;
  m_OR->setFatJets(jets);
  if(!jets) return;
  if(m_doOR) jets->useForOR(true);
}

void EventSelector::setFatJetsAlt(FatJetHandler* jets) {
  m_fatjetsAlt = jets;
  if(!jets) return;
}

void EventSelector::setTrackJets(TrackJetHandler* jets) {
  m_trackjets = jets;
}

void EventSelector::setCoMSubJets(TrackJetHandler* jets){
  m_subjets = jets;
}

void EventSelector::setMuons(MuonHandler* muons) {
  m_muons = muons;
  m_OR->setMuons(muons);
  if(!muons) return;
  if(m_doOR) muons->useForOR(true);
}

void EventSelector::setTaus(TauHandler* taus) {
  m_taus = taus;
  m_OR->setTaus(taus);
  if(!taus) return;
  if(m_doOR) taus->useForOR(true);
}

void EventSelector::setDiTaus(DiTauJetHandler* ditaus) {
  m_ditaus = ditaus;
  m_OR->setDiTaus(ditaus);
  if(!ditaus) return;
  if(m_doOR) ditaus->useForOR(true);
}

void EventSelector::setElectrons(ElectronHandler* electrons) {
  m_electrons = electrons;
  m_OR->setElectrons(electrons);
  if(!electrons) return;
  if(m_doOR) electrons->useForOR(true);
}

void EventSelector::setForwardElectrons(ForwardElectronHandler* electrons) {
  m_forwardElectrons = electrons;
}

void EventSelector::setPhotons(PhotonHandler* photons) {
  m_photons = photons;
  if(!photons) return;
  m_OR->setPhotons(photons);
  if(m_doOR) photons->useForOR(true);
}

void EventSelector::setMET(METHandler* met) { m_met = met; }
void EventSelector::setMETTrack(METHandler* met) { m_metTrack = met; }
void EventSelector::setMETMJTight(METHandler* met) { m_metMJTight = met; }
void EventSelector::setMETFJVT(METHandler* met) { m_metFJVT = met; }
void EventSelector::setMETMJMUTight(METHandler* met) { m_metMJMUTight = met; }
void EventSelector::setMETMJMiddle(METHandler* met) { m_metMJMiddle = met; }
void EventSelector::setMETMJLoose(METHandler* met) { m_metMJLoose = met; }

void EventSelector::setEventInfo(EventInfoHandler* info) { m_info = info; }

void EventSelector::setTruthEvent(TruthEventHandler* truthevent) { m_truthevent = truthevent; }
void EventSelector::setTruthElectronHandler(TruthProcessor* truthElectrons) { m_truthElectrons = truthElectrons; }
void EventSelector::setTruthMuonHandler(TruthProcessor* truthMuons) { m_truthMuons = truthMuons; }
void EventSelector::setTruthNeutrinoHandler(TruthProcessor* truthNeutrinos) { m_truthNeutrinos = truthNeutrinos; }


EL::StatusCode EventSelector::fillSystematics(ObjectHandlerBase* obj) {
  if(m_debug) Info("EventSelector::fillSystematics()","Filling systematics.");
  if(obj) {
    std::vector<TString> systs = obj->getAllVariations();
    for (const TString& sysName : systs) {
      m_systematics.insert(sysName.Data());
    }
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventSelector::fillSystematics() {
  // as we use a set<>, the systematics should be there only once in the end
  // so no risk to double count the nominal, or e.g jet systematics between JetHandler
  // and METHandler
  if(fillSystematics(m_jets) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for jets! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_fatjets) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for fatjets! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_fatjetsAlt) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for fatjets! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_trackjets) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for trackjets! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_muons) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for muons! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_taus) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for taus! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_ditaus) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for di taus! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_electrons) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for electrons! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_photons) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for photons! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_met) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for met! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_metMJTight) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for met Electron MJ! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_metFJVT) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for met FJVT! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_metMJMUTight) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for met Electron MJ! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_metMJMiddle) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for met Middle MJ! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_metMJLoose) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for met Loose MJ! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(fillSystematics(m_metTrack) != EL::StatusCode::SUCCESS){
    Error("EventSelector::fillSystematics()","Failed to fill systematics for met track! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if(m_debug) Info("EventSelector::fillSystematics()","Filling systematics for event info.");
  if (m_info) {
    std::vector<TString> systs = m_info->getAllVariations();
    for (const TString& sysName : systs) {
      m_systematics.insert(sysName.Data());
    }
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventSelector::performSelection(bool& pass) {
  if(m_debug) Info("EventSelector::performSelection()","Performing selection.");

  if (m_doCloseByIsoCorr) {
    xAOD::IParticleContainer leptons(SG::VIEW_ELEMENTS);
    for(auto pobj: *m_electrons->getInParticleVariation( "Nominal" )) if (Props::passPreSel.get(pobj)) leptons.push_back(pobj);
    for(auto pobj: *m_muons->getInParticleVariation( "Nominal" )) if (Props::passPreSel.get(pobj)) leptons.push_back(pobj);

    //correct isolation and propagate to signal deco for electrons
    for (const auto& electron : *m_electrons->getInParticleVariation( "Nominal" )) if (Props::passPreSel.get(electron))  {
      Root::TAccept isoAccept = m_isoCloseByTool->acceptCorrected(*electron, leptons);
      Props::isFixedCutLooseIso.set(electron, isoAccept.getCutResult("FCLoose"));
      Props::isFixedCutTightIso.set(electron, isoAccept.getCutResult("FCTight"));
      Props::isGradientIso.set(electron, isoAccept.getCutResult("Gradient"));
      Props::isFixedCutHighPtCaloOnlyIso.set(electron, isoAccept.getCutResult("FCHighPtCaloOnly"));
      Props::isFixedCutLooseFixedRadIso.set(electron, isoAccept.getCutResult("FCLoose_FixedRad"));
      Props::isFixedCutTightFixedRadIso.set(electron, isoAccept.getCutResult("FCTight_FixedRad"));
    }

    //correct isolation and propagate to signal deco for electrons
    for (const auto& muon : *m_muons->getInParticleVariation( "Nominal" )) if (Props::passPreSel.get(muon)) {
      Root::TAccept isoAccept = m_isoCloseByTool->acceptCorrected(*muon, leptons);
      Props::isFixedCutTightIso.set(muon, isoAccept.getCutResult("FCTight"));
      Props::isFixedCutLooseIso.set(muon, isoAccept.getCutResult("FCLoose"));
      Props::isFixedCutTightTrackOnlyIso.set(muon, isoAccept.getCutResult("FCTightTrackOnly"));
      Props::isFCTight_FixedRadIso.set(muon, isoAccept.getCutResult("FCTight_FixedRad"));
      Props::isFCLoose_FixedRadIso.set(muon, isoAccept.getCutResult("FCLoose_FixedRad"));
      Props::isFCTightTrackOnly_FixedRadIso.set(muon, isoAccept.getCutResult("FCTightTrackOnly_FixedRad"));
      Props::isFixedCutHighPtTrackOnlyIso.set(muon, isoAccept.getCutResult("FixedCutHighPtTrackOnly"));
      Props::isFixedCutPflowTightIso.set(muon, isoAccept.getCutResult("FixedCutPflowTight"));
      Props::isFixedCutPflowLooseIso.set(muon, isoAccept.getCutResult("FixedCutPflowLoose"));
    }
  }

  // clear selection result
  m_selectionResult.clear();

  if(m_isFirstCall) {
    if(fillSystematics() != EL::StatusCode::SUCCESS){
      Error("EventSelector::fillSystematics()","Failed to fill systematics! Exiting.");
      return EL::StatusCode::FAILURE;
    }
  }

  if(!m_selection) {
    if(m_isFirstCall) {
      Warning("EventSelector::performSelection()", "No event preselection has been set ! All events will pass");
    }
  }

  m_isFirstCall = false;

  // run on all systematics
  if ( m_overlapRegAcc ) EL_CHECK("EventSelector::performSelection()", m_overlapRegAcc->prepareRegister());
  pass = false;
  for(const std::string& sysName : m_systematics) {
    bool sysPass = passORandPreSel(sysName.c_str());
    pass |=sysPass;
    m_selectionResult.insert( std::make_pair(sysName, sysPass) );
    if ( m_overlapRegAcc ) {
      // Okay, this is a bit annoying : The MET systematics are renamed by METHandler (from e.g. SysNameUp to SysName__1up - to follow same scheme as other tools), so we need to do the same here :(
      // Possibly, we should avoid renaming them in METHandler in the frst place, but on the other side it's nice to keep the name consistent with the other tools. So for now we just rename them here 
      // as well to avoid mismatch in naming between containers and OverlapRegisters in the output. Clearly, this should be revisited when the OverlapRegister code is fully integrated in the work-flow...
      TString tempName = sysName;
      if( sysName.find("MET_") != std::string::npos ){
        tempName.ReplaceAll("ResoPara", "ResoPara__1up");
        tempName.ReplaceAll("ResoPerp", "ResoPerp__1up");
        tempName.ReplaceAll("ScaleUp", "Scale__1up");
        tempName.ReplaceAll("ScaleDown", "Scale__1down");
      }

      OverlapRegisterAccessor::Containers containers;
      containers.variation = tempName.Data();
      if ( m_jets      ) containers.jets      = m_jets      -> getInParticleVariation( sysName );
      if ( m_fatjets   ) containers.fatjets   = m_fatjets   -> getInParticleVariation( sysName );
      if ( m_muons     ) containers.muons     = m_muons     -> getInParticleVariation( sysName );
      if ( m_electrons ) containers.electrons = m_electrons -> getInParticleVariation( sysName );
      if ( m_taus      ) containers.taus      = m_taus      -> getInParticleVariation( sysName );
      if ( m_ditaus    ) containers.ditaus    = m_ditaus    -> getInParticleVariation( sysName );
      if ( m_photons   ) containers.photons   = m_photons   -> getInParticleVariation( sysName );
      EL_CHECK("EventSelector::performSelection()",m_overlapRegAcc->fillRegister( containers ));
    }
  }

  return EL::StatusCode::SUCCESS;

}

bool EventSelector::passORandPreSel(const TString& sysName) {
  if(m_debug) Info("EventSelector::passORandPreSel()","Passing OR and pre-selection.");

  // if no OR, then no selection at all
  if(!m_doOR) { return true; }
  
  EL::StatusCode res;

  // or tool fails if no PV
  std::string selectionName;
  m_config.getif<std::string>("selectionName", selectionName);
  if( (selectionName.find("vbf")!=std::string::npos) 
      and (! Props::hasPV.get(m_info->getEventInfo())) ){
    Warning("EventSelector::passORandPreSel", "No PV in this event.  Setting all objects to fail OR");
    res = m_OR->setAllObjectsFail(sysName);
  }
  else res = m_OR->removeOverlap(sysName);

  if(res != EL::StatusCode::SUCCESS) {
    Error("EventSelector::passORandPreSel", "There has been an issue with the OR. Please investigate !");
    exit(EXIT_FAILURE);
  }

  if(!m_selection) {
    return true;
  }

  return performSelection(sysName);
}

bool EventSelector::performSelection(const TString& sysName) {

  if(m_debug) Info("EventSelector::performSelection()", "Performing selection for variation '%s'." , sysName.Data());

  SelectionContainers containers;

  bool applyMETAfterSelection(false);
  m_config.getif<bool>("applyMETAfterSelection",applyMETAfterSelection);

  // first, retrieve the containers
  if ( m_jets      ) { containers.jets      = m_jets->getInParticleVariation(sysName)      ; }
  if ( m_fatjets   ) { containers.fatjets   = m_fatjets->getInParticleVariation(sysName)   ; }
  if ( m_fatjetsAlt) { containers.fatjetsAlt = m_fatjetsAlt->getInParticleVariation(sysName)   ; }
  if ( m_trackjets ) { containers.trackjets = m_trackjets->getInParticleVariation(sysName) ; }
  if ( m_subjets )   { containers.subjets   = m_subjets->getInParticleVariation(sysName) ; }
  if ( m_electrons ) { containers.electrons = m_electrons->getInParticleVariation(sysName) ; }
  if ( m_photons   ) { containers.photons   = m_photons->getInParticleVariation(sysName)   ; }
  if ( m_muons     ) { containers.muons     = m_muons->getInParticleVariation(sysName)     ; }
  if ( m_taus      ) { containers.taus      = m_taus->getInParticleVariation(sysName)      ; }
  if ( m_ditaus    ) { containers.ditaus    = m_ditaus->getInParticleVariation(sysName)    ; }
  if ( m_met && !applyMETAfterSelection) { containers.met       = m_met->getMET(sysName)   ; }
  if ( m_metMJTight && !applyMETAfterSelection) { containers.metmjtight = m_metMJTight->getMET(sysName); }
  if ( m_metFJVT && !applyMETAfterSelection) { containers.metfjvt = m_metFJVT->getMET(sysName); }
  if ( m_metMJMUTight && !applyMETAfterSelection) { containers.metmjmutight = m_metMJMUTight->getMET(sysName); }
  if ( m_metMJMiddle && !applyMETAfterSelection) { containers.metmjmiddle = m_metMJMiddle->getMET(sysName); }
  if ( m_metMJLoose && !applyMETAfterSelection) { containers.metmjloose = m_metMJLoose->getMET(sysName); }
  if ( m_metTrack && !applyMETAfterSelection) { containers.mettrack = m_metTrack->getMET(sysName); }

  if ( m_truthElectrons  ) {
    containers.truthElectrons = m_truthElectrons->getElectrons();
  }
  if ( m_truthMuons  ) {
    containers.truthMuons = m_truthMuons->getMuons();
  }
  if ( m_truthNeutrinos  ) {
    containers.truthNeutrinos = m_truthNeutrinos->getNeutrinos();
  }
  
  if(m_debug) Info("EventSelector::performSelection()","Successfully retrieved InParticleVariations." );


  // at the moment no systematics for METTrack. Will hopefully change.
  // --> not used yet - will ned to update SelectionContainer when this is to be used!
  //if( m_metTrack ) { containers.mettrack = m_metTrack->getMET(); }
  
  // making all events in period a (data15+16 and mc16a) to fail if one electron is in the crack region
  // or if ptag > p3830 reject events that have electrons or photons with DFCommonCrackVetoCleaning flags
  // this is to mitigate some met pflow weird shape observed. See https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/HowToCleanJetsR21#EGamma_Crack_Electron_topocluste
  bool rejectCrackEl = false;
  std::vector<std::string> prwInFiles = m_config.get< std::vector<std::string> >("configFiles");
  std::string jetAlgoName = m_config.get<std::string>("jetAlgoName");
  for (auto prwf : prwInFiles) {
    if(prwf.find("mc16a") != std::string::npos 
      && jetAlgoName.find("EMPFlow")!=std::string::npos){
      if(m_debug) Info("EventSelector::performSelection()", "Found period mc16a and dealing with pflow. Going to reject crack eletrons");
      rejectCrackEl = true;
    }
  }
  // force the value of the parameter to the value in the config for tests
  m_config.getif< bool >("RejectCrackEl", rejectCrackEl);
    
  bool inCrack = false;
  if(containers.electrons){
    for(const auto& ele : *containers.electrons) {
      if (!ele->isAvailable<char>("DFCommonCrackVetoCleaning")){
        if(ele->caloCluster()){ 
          if (1.37<fabs(ele->caloCluster()->etaBE(2)) 
          && fabs(ele->caloCluster()->etaBE(2))<1.52) inCrack = true;
        }
      } else {
        if(!(ele->auxdata<char>("DFCommonCrackVetoCleaning"))) inCrack = true;
      }
      if(rejectCrackEl && inCrack && Props::forMETRebuild.get(ele)){
          if(m_debug) Info("EventSelector::performSelection()", 
          "Event contains a bad electron for pflow! Should reject the event if the data period is a!");
          return false;
      }
    }
  }
  if(containers.photons){
    for(const auto& photon : *containers.photons) {
      if (photon->isAvailable<char>("DFCommonCrackVetoCleaning")){
        if(rejectCrackEl && !(photon->auxdata<char>("DFCommonCrackVetoCleaning"))&& Props::forMETRebuild.get(photon)){
          if(m_debug) Info("EventSelector::performSelection()", 
          "Event contains a bad photon for pflow! Should reject the event if the data period is a!");
          return false;
        }
      }
    }
  }
  // event info is required !
  if(! m_info) {
    Error("EventSelector::performSelection",
          "We need to know about EventInfo ! Please call setEventInfo before any attempt to apply selections");
    exit(EXIT_FAILURE);
  }

  // input event info for selection
  containers.evtinfo = m_info->getEventInfo();
 
  m_selection->setSysName(sysName);

  bool skipCutFlow = (sysName != "Nominal");
  // presence of m_selection has been checked in performSelection(bool)
  return m_selection->passPreSelection(containers, skipCutFlow);
}


void EventSelector::setSelection(EventSelection* selection) {
  m_selection = selection;
}

EventSelection* EventSelector::getSelection() const {
  return m_selection;
}

const CutFlowCounter& EventSelector::getCutFlowCounter() const {
  return m_selection->getCutFlowCounter();
}


EL::StatusCode EventSelector::fillEventVariables() {
  if(m_debug) Info("EventSelector::fillEventVariables()","Performing selection.");

  if(m_isFirstCall) {
    if(fillSystematics() != EL::StatusCode::SUCCESS){
      Error("EventSelector::fillEventVariables()","Failed to fill systematics! Exiting.");
      return EL::StatusCode::FAILURE;
    }
  }

  if(!m_selection) {
    if(m_isFirstCall) {
      Warning("EventSelector::fillEventVariables()", "No event preselection has been set ! no event-vars will be written");
    }
  }

  m_isFirstCall = false;

  if(!m_selection)  return true; 

  for(const std::string& sysName : m_systematics) {
    SelectionContainers containers;

    // first, retrieve the containers
    if ( m_jets      ) { containers.jets      = m_jets->getInParticleVariation(sysName)      ; }
    if ( m_fatjets   ) { containers.fatjets   = m_fatjets->getInParticleVariation(sysName)   ; }
    if ( m_fatjetsAlt) { containers.fatjetsAlt = m_fatjetsAlt->getInParticleVariation(sysName)   ; }
    if ( m_trackjets ) { containers.trackjets = m_trackjets->getInParticleVariation(sysName) ; }
    if ( m_electrons ) { containers.electrons = m_electrons->getInParticleVariation(sysName) ; }
    if ( m_photons   ) { containers.photons   = m_photons->getInParticleVariation(sysName)   ; }
    if ( m_muons     ) { containers.muons     = m_muons->getInParticleVariation(sysName)     ; }
    if ( m_taus      ) { containers.taus      = m_taus->getInParticleVariation(sysName)      ; }
    if ( m_ditaus    ) { containers.ditaus    = m_ditaus->getInParticleVariation(sysName)    ; }
    if ( m_met       ) { containers.met       = m_met->getMET(sysName)                       ; }
    if ( m_metMJTight) { containers.metmjtight= m_metMJTight->getMET(sysName)                ; }
    if ( m_metFJVT) { containers.metfjvt =  m_metFJVT->getMET(sysName)                ; }
    if ( m_metMJMUTight) { containers.metmjmutight= m_metMJMUTight->getMET(sysName)                ; }
    if ( m_metMJMiddle) { containers.metmjmiddle= m_metMJMiddle->getMET(sysName)             ; }
    if ( m_metMJLoose) { containers.metmjloose= m_metMJLoose->getMET(sysName)                ; }
    if ( m_metTrack) { containers.mettrack= m_metTrack->getMET(sysName)                      ; }
    
    if(m_debug) Info("EventSelector::fillEventVariables()","Successfully retrieved InParticleVariations." );
    
    //TODO  at the moment no systematics for METTrack. Will hopefully change. 
    // --> not used yet - will ned to update SelectionContainer when this is to be used!
    //if( m_metTrack ) { containers.mettrack = m_metTrack->getMET(); }

    // event info is required !
    if(! m_info) {
      Error("EventSelector::fillEventVariables",
            "We need to know about EventInfo ! Please call setEventInfo before any attempt to fill event-variables");
      exit(EXIT_FAILURE);
    }

    // input event info for selection
    containers.evtinfo =  m_info->getEventInfo();

    m_selection->setSysName(sysName);
    m_selection->passPreSelection(containers, true);
    
    // is it a variation?
    bool isKinVar = (sysName != "Nominal");
    bool isWeightVar = false;
    for(unsigned int iWeightVar=0; iWeightVar<m_weightVariations.size() ; ++iWeightVar ){
      if( sysName == m_weightVariations[iWeightVar]) {
        isWeightVar = true;
        isKinVar = false;
      }
    }
    
    // write event variables for variations that have a corresponding event info
    xAOD::EventInfo* evtinfoOut = m_info->getOutEventInfoVariation(sysName, false);
    if (evtinfoOut) {
      if (m_debug) {
        Info("EventSelector::fillEventVariables()", "Writing event variables.");
      }
      EL_CHECK("EventSelector::fillEventVariables()", m_selection->writeEventVariables(containers.evtinfo, evtinfoOut, isKinVar, isWeightVar, sysName, m_info->get_RandomRunNumber(), m_trig_sfmuon));
    }
  }
  return EL::StatusCode::SUCCESS;
}


bool EventSelector::getSelectionResult(const TString & sysName) const
{
  
  std::map<TString, bool>::const_iterator iter = m_selectionResult.find(sysName);
  if ( iter == m_selectionResult.end() ) {
    Error("EventSelector::getSelectionResult()", "Couldn't find systematic '%s', returning 'false'!!!", sysName.Data());
    return false;
  }
  
  return iter->second;
  
} 
