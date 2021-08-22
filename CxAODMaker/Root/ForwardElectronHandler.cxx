#include <iostream>

#include "CxAODMaker/ForwardElectronHandler.h"
#include "CxAODMaker/TruthProcessor.h"
#include "CxAODMaker/EventInfoHandler.h"

#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "ElectronPhotonSelectorTools/AsgForwardElectronLikelihoodTool.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

#include "IsolationSelection/IsolationSelectionTool.h"
#include "IsolationCorrections/IsolationCorrectionTool.h"
#include "TrigBunchCrossingTool/WebBunchCrossingTool.h"

ForwardElectronHandler::ForwardElectronHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                                 EventInfoHandler & eventInfoHandler) : 
  ObjectHandler(name, config, event, eventInfoHandler),
  m_EgammaCalibrationAndSmearingTool(nullptr),
  m_effToolLooseLH(nullptr),
  m_effToolMediumLH(nullptr),
  m_effToolTightLH(nullptr),
  m_checkLooseLH(nullptr),
  m_checkMediumLH(nullptr),
  m_checkTightLH(nullptr),
  m_isIso(nullptr),
  m_doResolution(false)
{
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind(&ForwardElectronHandler::passLooseElectron, this, _1));
  m_config.getif<bool>("doResolution",m_doResolution);  
}


ForwardElectronHandler::~ForwardElectronHandler() 
{
  //delete tools
  delete m_EgammaCalibrationAndSmearingTool;
  delete m_effToolLooseLH;
  delete m_effToolMediumLH;
  delete m_effToolTightLH;
  delete m_checkLooseLH;
  delete m_checkMediumLH;
  delete m_checkTightLH;
  delete m_isIso;
}

EL::StatusCode ForwardElectronHandler::initializeTools() 
{
  // TODO atlfast / fullsim
  bool isFullSim = !m_eventInfoHandler.get_isAFII();
  //  enum DataType {Data = 0, Full = 1, FastShower = 2, Fast = 3, True = 4}
  int dataType = PATCore::ParticleDataType::Full;
  if(!isFullSim) dataType = PATCore::ParticleDataType::Fast;

  //calibration tool
  //------------------

  m_EgammaCalibrationAndSmearingTool = new CP::EgammaCalibrationAndSmearingTool("EgammaCalibrationAndSmearingTool_FwdElectrons");
  m_EgammaCalibrationAndSmearingTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->setProperty("ESModel", "es2018_R21_v0"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->setProperty("decorrelationModel", "1NP_v1"));
  int FullSim_AFII_flag = isFullSim? 0 :1 ;
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->setProperty("useAFII", FullSim_AFII_flag));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->initialize());

  // efficiency tool loose
  //-----------------
  //N.B. central ID from Moriond18
  m_effToolLooseLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_FwdLooseLH");
  m_effToolLooseLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListLoose;
  correctionFileNameListLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/offline/efficiencySF.offline.FwdLooseLLH_v1.root");
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolLooseLH->setProperty("CorrectionFileNameList", correctionFileNameListLoose));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolLooseLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolLooseLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolLooseLH->initialize());
  
  // efficiency tool medium
  //-----------------
  //N.B. central ID from Moriond18
  m_effToolMediumLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_FwdMediumLH");
  m_effToolMediumLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListMedium;
  correctionFileNameListMedium.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/offline/efficiencySF.offline.FwdMediumLLH_v1.root");
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolMediumLH->setProperty("CorrectionFileNameList", correctionFileNameListMedium));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolMediumLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolMediumLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolMediumLH->initialize());
  
  // efficiency tool tight
  //-----------------
  //N.B. central ID from Moriond18
  m_effToolTightLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_FwdTightLH");
  m_effToolTightLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTight;
  correctionFileNameListTight.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/offline/efficiencySF.offline.FwdTightLLH_v1.root");
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolTightLH->setProperty("CorrectionFileNameList", correctionFileNameListTight));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolTightLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolTightLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_effToolTightLH->initialize());

  std::string confDir = "ElectronPhotonSelectorTools/offline/mc16_20170828/"; //release 21
  
  //LooseLH
  m_checkLooseLH = new AsgForwardElectronLikelihoodTool("checkFwdLooseLH");
  m_checkLooseLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_checkLooseLH->setProperty("WorkingPoint","LooseLHForwardElectron"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_checkLooseLH->initialize());
  //MediumLH
  m_checkMediumLH = new AsgForwardElectronLikelihoodTool("checkFwdMediumLH");
  m_checkMediumLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_checkMediumLH->setProperty("WorkingPoint","MediumLHForwardElectron"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_checkMediumLH->initialize());
  //TightLH
  m_checkTightLH = new AsgForwardElectronLikelihoodTool("checkFwdTightLH");
  m_checkTightLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_checkTightLH->setProperty("WorkingPoint","TightLHForwardElectron"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()", m_checkTightLH->initialize());


  //Isolation WP under study
  m_isIso = new CP::IsolationSelectionTool("ElectronIso");
  TOOL_CHECK("ForwardElectronHandler::initializeTools()",m_isIso->setProperty("ElectronWP", "FCHighPtCaloOnly"));
  TOOL_CHECK("ForwardElectronHandler::initializeTools()",m_isIso->initialize());

  if (!m_eventInfoHandler.get_isMC()) {
    m_bct = new Trig::WebBunchCrossingTool("WebBunchCrossingTool");
    TOOL_CHECK("ForwardElectronHandler::initializeTools()",m_bct->setProperty("ServerAddress", "atlas-trigconf.cern.ch"));
    TOOL_CHECK("ForwardElectronHandler::initializeTools()",m_bct->initialize());
  }

  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear();
  m_sysToolList.push_back(m_EgammaCalibrationAndSmearingTool);
  m_sysToolList.push_back(m_effToolLooseLH);
  m_sysToolList.push_back(m_effToolMediumLH);
  m_sysToolList.push_back(m_effToolTightLH);

  return EL::StatusCode::SUCCESS;
}

bool ForwardElectronHandler::passLooseElectron(xAOD::Electron * electron) {
  bool passSel = true;

  passSel &= (Props::isLooseLH.get(electron));
  passSel &= (electron->pt() > 20000.);

  Props::passPreSel.set(electron, passSel);
  return passSel;
}

EL::StatusCode ForwardElectronHandler::decorateOriginParticle(const xAOD::Electron * electron)
{
  // All values here are independent of energy calibration
  //selection tools
  //---------------
  //retrieve decision from tools and decorate electron with it
  //perform actual selection later
  int isLooseLH = 0;
  int isMediumLH = 0;
  int isTightLH = 0;
  // LooseLH < MediumLH < TightLH
  isLooseLH = electron -> isAvailable<char>("DFCommonForwardElectronsLHLoose") ? static_cast<int>(electron -> auxdata<char>("DFCommonForwardElectronsLHLoose")) : static_cast<int>(m_checkLooseLH->accept(electron));
  if(isLooseLH) {
    isMediumLH = electron -> isAvailable<char>("DFCommonForwardElectronsLHMedium") ? static_cast<int>(electron -> auxdata<char>("DFCommonForwardElectronsLHMedium")) : static_cast<int>(m_checkMediumLH->accept(electron));
    if(isMediumLH) {
      isTightLH = electron -> isAvailable<char>("DFCommonForwardElectronsLHTight") ? static_cast<int>(electron -> auxdata<char>("DFCommonForwardElectronsLHTight")) : static_cast<int>(m_checkTightLH->accept(electron));
    }
  }

  unsigned int bcid = m_eventInfoHandler.getEventInfo()->bcid();
  bool passCleaning = true;
  if (!m_eventInfoHandler.get_isMC() && m_eventInfoHandler.getEventInfo()->runNumber()>=266904 && m_eventInfoHandler.getEventInfo()->runNumber()<=311481) {
    double distance =  m_bct->gapBeforeTrain(bcid)>500 ? m_bct->distanceFromFront(bcid) : m_bct->distanceFromFront(bcid)+m_bct->gapBeforeTrain(bcid) + ( m_bct->distanceFromFront(bcid)+m_bct->distanceFromTail(bcid) );
    if ( distance< 5.5*25) passCleaning = false;
  }
  
  Props::isLooseLH.set(electron, isLooseLH && passCleaning);
  Props::isMediumLH.set(electron, isMediumLH && passCleaning);
  Props::isTightLH.set(electron, isTightLH && passCleaning);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ForwardElectronHandler::decorate(xAOD::Electron * electron)
{
  // for the record: used to eat up to 47% of the total Maker CPU...
  // so some basic optimization is welcome, like not computing what cannot be used at all...

  //selection tools
  //---------------
  //retrieve decisions stored on the original container
  int isLooseLH  = Props::isLooseLH.get(electron);
  int isMediumLH = Props::isMediumLH.get(electron);
  int isTightLH  = Props::isTightLH.get(electron);

  Root::TAccept isoAccept = m_isIso->accept( *electron);

  Props::isFixedCutHighPtCaloOnlyIso.set(electron, isoAccept.getCutResult("FCHighPtCaloOnly"));

  if(m_doResolution){
    const xAOD::Egamma* aaa=electron;
    float res=-1;
    res=m_EgammaCalibrationAndSmearingTool->getResolution(*aaa, false);
    Props::resolution.set(electron, res);
  }

  // get SFs from electron SF tool
  //--------------------------
  int isMC = m_eventInfoHandler.get_isMC();
 
  double effSFlooseLH = 1.;
  double effSFmediumLH = 1.;
  double effSFtightLH = 1.;

  const xAOD::CaloCluster* cluster = electron->caloCluster();
  float cluster_eta = 10;
  if (cluster) {
    //cluster_eta = cluster->eta();
    cluster_eta = cluster->etaBE(2); 
  }
  Props::ElClusterEta.set(electron, cluster_eta);

  if (isMC && electron->pt() >= 20000.) {
    if(isLooseLH) {
      if (m_effToolLooseLH->getEfficiencyScaleFactor(*electron, effSFlooseLH) == CP::CorrectionCode::Error) {
	Error("ForwardElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
	return EL::StatusCode::FAILURE;
      }
      if(isMediumLH) {

	if (m_effToolMediumLH->getEfficiencyScaleFactor(*electron, effSFmediumLH) == CP::CorrectionCode::Error) {
          Error("ForwardElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
        if(isTightLH) {
          if (m_effToolTightLH->getEfficiencyScaleFactor(*electron, effSFtightLH) == CP::CorrectionCode::Error) {
            Error("ForwardElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
        }
      }
    }
  }
  Props::effSFlooseLH.set(electron, effSFlooseLH);
  Props::effSFmediumLH.set(electron, effSFmediumLH);
  Props::effSFtightLH.set(electron, effSFtightLH);

  return EL::StatusCode::SUCCESS;  

}

EL::StatusCode ForwardElectronHandler::calibrate() {
  // Important to call the parent function!!
  return ObjectHandler<xAOD::ElectronContainer>::calibrate();
}

EL::StatusCode ForwardElectronHandler::calibrateCopies(xAOD::ElectronContainer * particles, const CP::SystematicSet & sysSet) 
{

  //calibration tool
  //-----------------
  // tell tool to apply systematic variation
  CP_CHECK("ForwardElectronHandler::calibrateCopies()",m_EgammaCalibrationAndSmearingTool->applySystematicVariation(sysSet),m_debug);

  // tell tool to apply systematic variation
  CP_CHECK("ForwardElectronHandler::calibrateCopies()",m_effToolLooseLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ForwardElectronHandler::calibrateCopies()",m_effToolMediumLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ForwardElectronHandler::calibrateCopies()",m_effToolTightLH->applySystematicVariation(sysSet),m_debug);

  for (xAOD::Electron * electron : *particles) {
    if ( decorate( electron ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
  }
  return EL::StatusCode::SUCCESS;

}


EL::StatusCode ForwardElectronHandler::writeOutputVariables(xAOD::Electron * inElectron, xAOD::Electron * outElectron, bool isKinVar, bool isWeightVar, const TString& sysName) 
{
  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    writeAllVariations(inElectron, outElectron, sysName);
  } 
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    writeKinematicVariations(inElectron, outElectron, sysName);
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes  
    writeWeightVariations(inElectron, outElectron, sysName);
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited 
    writeAllVariations(inElectron, outElectron, sysName);
    writeKinematicVariations(inElectron, outElectron, sysName);
    writeWeightVariations(inElectron, outElectron, sysName);
    writeNominal(inElectron, outElectron, sysName);
  } else assert(false);
 
  return writeCustomVariables(inElectron, outElectron, isKinVar, isWeightVar, sysName);

}

EL::StatusCode ForwardElectronHandler::writeAllVariations(xAOD::Electron*, xAOD::Electron*, const TString&)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ForwardElectronHandler::writeKinematicVariations(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& /*sysName*/)
{
  // 4-vector components affected by variations:
  Props::pt.copy(inElectron, outElectron);
  
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ForwardElectronHandler::writeWeightVariations(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& sysName)
{
  
  // Write variables only for specific variation. Has to be written to Nominal as well.
  bool isIDVar   = (sysName == "Nominal" || sysName.BeginsWith("EL_EFF_ID_"));
  
  // SF are only weight changes
  if (m_eventInfoHandler.get_isMC() && isIDVar) {
    Props::effSFlooseLH.copy(inElectron, outElectron);
    Props::effSFmediumLH.copy(inElectron, outElectron);
    Props::effSFtightLH.copy(inElectron, outElectron);
  }

  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ForwardElectronHandler::writeNominal(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& /*sysName*/)
{
  // 4-vector components not affected by variations:
  Props::eta.copy(inElectron, outElectron);
  if(m_saveNonCrucialVariables){
    Props::ElClusterEta.copy(inElectron, outElectron);
  }
  Props::phi.copy(inElectron, outElectron);
  Props::m.copy(inElectron, outElectron);

  Props::charge.copy(inElectron, outElectron); 

  if(m_doResolution){
    Props::resolution.copy(inElectron, outElectron);
  }

  //simply needed to rerun OR on CxAODs
  if (inElectron->isAvailable<char>("Loose")) { // is this decoration available?
    outElectron->setPassSelection(inElectron->passSelection("Loose"), "Loose");
  }
  
  //set isolation
  if(inElectron->isAvailable< float >("topoetcone20")){
    outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::topoetcone20),xAOD::Iso::topoetcone20);}
  
  Props::isFixedCutHighPtCaloOnlyIso.copy(inElectron, outElectron);

  //identification WPs
  Props::isLooseLH.copy(inElectron, outElectron);
  Props::isMediumLH.copy(inElectron, outElectron);
  Props::isTightLH.copy(inElectron, outElectron);

  //Write Truth Match property
  Props::TruthMatch.copyIfExists(inElectron, outElectron);

  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ForwardElectronHandler::writeCustomVariables(xAOD::Electron*, xAOD::Electron*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ForwardElectronHandler::clearEvent() 
{
  
  EL_CHECK("ForwardElectronHandler::clearEvent()",ObjectHandler::clearEvent());
  return EL::StatusCode::SUCCESS;

}

//Overload of ObjectHandlerBase link copier
EL::StatusCode ForwardElectronHandler::fillOutputLinks(){

  if(m_debug){
    Info("ForwardElectronHandler::fillOutputLinks", " Copying input container element links to output container (%s)", m_handlerName.c_str());
  }

  //Check that the input container is non-zero
  if( m_inContainer.size() == 0 ){ return EL::StatusCode::SUCCESS; }

  //Check that the output container is non-zero in size
  if( m_outContainer.size() == 0 ){ return EL::StatusCode::SUCCESS; }

  //Now use only the nominal input container to copy partile links
  const auto &inCont = *m_inContainer["Nominal"];

  //Write to only the nominal output container
  auto &outCont = *m_outContainer["Nominal"];

  //Create an accessor
  std::string RecoToTruthLinkName = "TruthPart";
  m_config.getif<std::string>("RecoToTruthLinkName", RecoToTruthLinkName);
  SG::AuxElement::Accessor< ElementLink< const xAOD::TruthParticleContainer > > truthPartElAcc (RecoToTruthLinkName);

  //Loop through the electrons in the output container
  for( unsigned int j = 0; j < outCont.size(); ++j){
    
    //Get the particle index from the input container
    unsigned int index = Props::partIndex.get(outCont.at(j));    
    
    //Check that the electron is truth matched
    if ( !Props::TruthMatch.exists(inCont.at(index)) || !Props::TruthMatch.get(inCont.at(index)) 
	 || !(truthPartElAcc.isAvailable( *(inCont.at(index)) )) ) continue;

    //Extract the element link for the current electron
    ElementLink< const xAOD::TruthParticleContainer> el = truthPartElAcc(*(inCont.at(index)));

    //Now assign to the output particle
    truthPartElAcc( *(outCont.at(j)) ) = el;   
    

    //=============== Alterantively use the methods in ParticleLinker.cxx class =================
    //Otherwise copy particle link to output container
    //m_linker->copyParticleLinks(*(inCont.at(j)), *(outCont.at(j)), "TruthPart", m_handlerName);    
    //===========================================================================================
    
  }

  return EL::StatusCode::SUCCESS;
  
}
