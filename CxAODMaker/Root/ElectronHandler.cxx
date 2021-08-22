#include <iostream>

#include "CxAODMaker/ElectronHandler.h"
#include "CxAODMaker/TruthProcessor.h"
#include "CxAODMaker/EventInfoHandler.h"

#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

#include "IsolationSelection/IsolationSelectionTool.h"
#include "IsolationCorrections/IsolationCorrectionTool.h"

ElectronHandler::ElectronHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                                 EventInfoHandler & eventInfoHandler) : 
  ObjectHandler(name, config, event, eventInfoHandler),
  m_EgammaCalibrationAndSmearingTool(nullptr),
  m_trigEffToolLooseLHIsoFixedCutLoose(nullptr),
  m_trigToolLooseLHIsoFixedCutLoose(nullptr),
  m_trigEffToolTightLHIsoFixedCutTight(nullptr),
  m_trigToolTightLHIsoFixedCutTight(nullptr),
  m_trigEffToolTightLHIsoFixedCutLoose(nullptr),
  m_trigToolTightLHIsoFixedCutLoose(nullptr),
  m_e17trigEffToolMediumLHIsoFixedCutLoose(nullptr),
  m_e17trigToolMediumLHIsoFixedCutLoose(nullptr),
  m_trigEffToolMediumLH_offTightIsoFixedCutLoose(nullptr),
  m_trigToolMediumLH_offTightIsoFixedCutLoose(nullptr),
  m_effToolLooseLH(nullptr),
  m_effToolMediumLH(nullptr),
  m_effToolTightLH(nullptr),
  m_effToolReco(nullptr),
  m_effToolIsoGradientLooseLH(nullptr),
  m_effToolIsoGradientMediumLH(nullptr),
  m_effToolIsoGradientTightLH(nullptr),
  m_effToolIsoFixedCutLooseLooseLH(nullptr),
  m_effToolIsoFixedCutLooseMediumLH(nullptr),
  m_effToolIsoFixedCutLooseTightLH(nullptr),
  m_effToolIsoFixedCutTightLooseLH(nullptr),
  m_effToolIsoFixedCutTightMediumLH(nullptr),
  m_effToolIsoFixedCutTightTightLH(nullptr),
  m_effToolIsoFixedCutHighPtCaloOnlyLooseLH(nullptr),
  m_effToolIsoFixedCutHighPtCaloOnlyMediumLH(nullptr),
  m_effToolIsoFixedCutHighPtCaloOnlyTightLH(nullptr),
  m_checkVeryLooseLH(nullptr),
  m_checkLooseNoBLLH(nullptr),
  m_checkLooseLH(nullptr),
  m_checkMediumLH(nullptr),
  m_checkTightLH(nullptr),
  m_isIso(nullptr),
  m_isoCorr_tool(nullptr),
  m_trigDecTool(nullptr),
  m_doTrigMatch(false),
  m_doResolution(false),
  m_triggersToMatch()
{
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind(&ElectronHandler::passLooseElectron, this, _1));
  m_config.getif<bool>("doResolution",m_doResolution);  
}


ElectronHandler::~ElectronHandler() 
{
  //delete tools
  delete m_EgammaCalibrationAndSmearingTool;
  delete m_trigEffToolLooseLHIsoFixedCutLoose;
  delete m_trigToolLooseLHIsoFixedCutLoose;
  delete m_trigEffToolTightLHIsoFixedCutTight;
  delete m_trigToolTightLHIsoFixedCutTight;
  delete m_trigEffToolTightLHIsoFixedCutLoose;
  delete m_trigToolTightLHIsoFixedCutLoose;
  delete m_e17trigEffToolMediumLHIsoFixedCutLoose;
  delete m_e17trigToolMediumLHIsoFixedCutLoose;
  delete m_trigEffToolMediumLH_offTightIsoFixedCutLoose;
  delete m_trigToolMediumLH_offTightIsoFixedCutLoose;
  delete m_effToolLooseLH;
  delete m_effToolMediumLH;
  delete m_effToolTightLH;
  delete m_effToolReco;
  delete m_effToolIsoGradientLooseLH;
  delete m_effToolIsoGradientMediumLH;
  delete m_effToolIsoGradientTightLH;
  delete m_effToolIsoFixedCutLooseLooseLH;
  delete m_effToolIsoFixedCutLooseMediumLH;
  delete m_effToolIsoFixedCutLooseTightLH;
  delete m_effToolIsoFixedCutTightLooseLH;
  delete m_effToolIsoFixedCutTightMediumLH;
  delete m_effToolIsoFixedCutTightTightLH;
  delete m_effToolIsoFixedCutHighPtCaloOnlyLooseLH;
  delete m_effToolIsoFixedCutHighPtCaloOnlyMediumLH;
  delete m_effToolIsoFixedCutHighPtCaloOnlyTightLH;
  delete m_checkVeryLooseLH;
  delete m_checkLooseNoBLLH;
  delete m_checkLooseLH;
  delete m_checkMediumLH;
  delete m_checkTightLH;
  delete m_isIso;
  delete m_isoCorr_tool;
}


EL::StatusCode ElectronHandler::initializeTools() 
{
  // TODO atlfast / fullsim
  bool isFullSim = !m_eventInfoHandler.get_isAFII();
  //  enum DataType {Data = 0, Full = 1, FastShower = 2, Fast = 3, True = 4}
  int dataType = PATCore::ParticleDataType::Full;
  if(!isFullSim) dataType = PATCore::ParticleDataType::Fast;

  //calibration tool
  //------------------

  m_EgammaCalibrationAndSmearingTool = new CP::EgammaCalibrationAndSmearingTool("EgammaCalibrationAndSmearingTool_Electrons");
  m_EgammaCalibrationAndSmearingTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->setProperty("ESModel", "es2018_R21_v0"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->setProperty("decorrelationModel", "1NP_v1"));
  int FullSim_AFII_flag = isFullSim? 0 :1 ;
  TOOL_CHECK("ElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->setProperty("useAFII", FullSim_AFII_flag));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_EgammaCalibrationAndSmearingTool->initialize());

  // trigger tool for looseLHIsoFixedCutLoose triggers
  //-----------------
  m_trigToolLooseLHIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigLooseLHIsoFixedCutLoose");
  m_trigToolLooseLHIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigLooseLHIsoFixedCutLoose;
  correctionFileNameListTrigLooseLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiencySF.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.LooseAndBLayerLLH_d0z0_v13_isolFCLoose.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolLooseLHIsoFixedCutLoose->setProperty("CorrectionFileNameList", correctionFileNameListTrigLooseLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolLooseLHIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolLooseLHIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolLooseLHIsoFixedCutLoose->initialize());

  m_trigEffToolLooseLHIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigEffLooseLHIsoFixedCutLoose");
  m_trigEffToolLooseLHIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigEffLooseLHIsoFixedCutLoose;
  correctionFileNameListTrigEffLooseLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiency.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.LooseAndBLayerLLH_d0z0_v13_isolFCLoose.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolLooseLHIsoFixedCutLoose->setProperty("CorrectionFileNameList", correctionFileNameListTrigEffLooseLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolLooseLHIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolLooseLHIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolLooseLHIsoFixedCutLoose->initialize());

  // trigger tool for tightLHIsoFixedCutTight triggers
  //-----------------
  m_trigToolTightLHIsoFixedCutTight = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigTightLHIsoFixedCutTight");
  m_trigToolTightLHIsoFixedCutTight->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigTightLHIsoFixedCutTight;
  correctionFileNameListTrigTightLHIsoFixedCutTight.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiencySF.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.TightLLH_d0z0_v13_isolFCTight.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutTight->setProperty("CorrectionFileNameList", correctionFileNameListTrigTightLHIsoFixedCutTight));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutTight->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutTight->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutTight->initialize());

  m_trigEffToolTightLHIsoFixedCutTight = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigEffTightLHIsoFixedCutTight");
  m_trigEffToolTightLHIsoFixedCutTight->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigEffTightLHIsoFixedCutTight;
  correctionFileNameListTrigEffTightLHIsoFixedCutTight.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiency.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.TightLLH_d0z0_v13_isolFCTight.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutTight->setProperty("CorrectionFileNameList", correctionFileNameListTrigEffTightLHIsoFixedCutTight));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutTight->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutTight->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutTight->initialize());

// trigger tool for bbtautau triggers

  m_trigToolTightLHIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigTightLHIsoFixedCutLoose");
  m_trigToolTightLHIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigTightLHIsoFixedCutLoose;
  correctionFileNameListTrigTightLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiencySF.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.TightLLH_d0z0_v13_isolFCLoose.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutLoose->setProperty("CorrectionFileNameList", correctionFileNameListTrigTightLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolTightLHIsoFixedCutLoose->initialize());

  m_trigEffToolTightLHIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigEffTightLHIsoFixedCutLoose");
  m_trigEffToolTightLHIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigEffTightLHIsoFixedCutLoose;
  correctionFileNameListTrigEffTightLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiency.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.TightLLH_d0z0_v13_isolFCLoose.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutLoose->setProperty("CorrectionFileNameList", correctionFileNameListTrigEffTightLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolTightLHIsoFixedCutLoose->initialize());


  m_e17trigToolMediumLHIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_e17trigMediumLHIsoLoose");
  m_e17trigToolMediumLHIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > e17correctionFileNameListTrigMediumLHIsoFixedCutLoose;
  e17correctionFileNameListTrigMediumLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiencySF.e17_lhvloose_nod0.MediumLLH_d0z0_v13_isolFCLoose.2016.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigToolMediumLHIsoFixedCutLoose->setProperty("CorrectionFileNameList", e17correctionFileNameListTrigMediumLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigToolMediumLHIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigToolMediumLHIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigToolMediumLHIsoFixedCutLoose->initialize());

  m_e17trigEffToolMediumLHIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_e17trigEffMediumLHIsoLoose");
  m_e17trigEffToolMediumLHIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > e17correctionFileNameListTrigEffMediumLHIsoFixedCutLoose;
  e17correctionFileNameListTrigEffMediumLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiency.e17_lhvloose_nod0.MediumLLH_d0z0_v13_isolFCLoose.2016.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigEffToolMediumLHIsoFixedCutLoose->setProperty("CorrectionFileNameList", e17correctionFileNameListTrigEffMediumLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigEffToolMediumLHIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigEffToolMediumLHIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_e17trigEffToolMediumLHIsoFixedCutLoose->initialize());


  m_trigToolMediumLH_offTightIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigMediumLH_offTightIsoFixedCutLoose");
  m_trigToolMediumLH_offTightIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigMedium_offTightLHIsoFixedCutLoose;
  correctionFileNameListTrigMedium_offTightLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiencySF.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.TightLLH_d0z0_v13_isolFCLoose.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolMediumLH_offTightIsoFixedCutLoose->setProperty("CorrectionFileNameList", correctionFileNameListTrigMedium_offTightLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolMediumLH_offTightIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolMediumLH_offTightIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigToolMediumLH_offTightIsoFixedCutLoose->initialize());
  
  m_trigEffToolMediumLH_offTightIsoFixedCutLoose = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigEffMediumLH_offTightIsoFixedCutLoose");
  m_trigEffToolMediumLH_offTightIsoFixedCutLoose->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTrigEffMedium_offTightLHIsoFixedCutLoose;
  correctionFileNameListTrigEffMedium_offTightLHIsoFixedCutLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/trigger/efficiency.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.TightLLH_d0z0_v13_isolFCLoose.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolMediumLH_offTightIsoFixedCutLoose->setProperty("CorrectionFileNameList", correctionFileNameListTrigEffMedium_offTightLHIsoFixedCutLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolMediumLH_offTightIsoFixedCutLoose->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolMediumLH_offTightIsoFixedCutLoose->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_trigEffToolMediumLH_offTightIsoFixedCutLoose->initialize());

  
  // efficiency tool loose
  //-----------------
  //N.B. central ID from Moriond18
  m_effToolLooseLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_LooseLH");
  m_effToolLooseLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListLoose;
  correctionFileNameListLoose.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v1/offline/efficiencySF.offline.LooseAndBLayerLLH_d0z0_v13.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolLooseLH->setProperty("CorrectionFileNameList", correctionFileNameListLoose));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolLooseLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolLooseLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolLooseLH->initialize());
  
  // efficiency tool medium
  //-----------------
  //N.B. central ID from Moriond18
  m_effToolMediumLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_MediumLH");
  m_effToolMediumLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListMedium;
  correctionFileNameListMedium.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v1/offline/efficiencySF.offline.MediumLLH_d0z0_v13.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolMediumLH->setProperty("CorrectionFileNameList", correctionFileNameListMedium));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolMediumLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolMediumLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolMediumLH->initialize());
  
  // efficiency tool tight
  //-----------------
  //N.B. central ID from Moriond18
  m_effToolTightLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_TightLH");
  m_effToolTightLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListTight;
  correctionFileNameListTight.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v1/offline/efficiencySF.offline.TightLLH_d0z0_v13.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolTightLH->setProperty("CorrectionFileNameList", correctionFileNameListTight));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolTightLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolTightLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolTightLH->initialize());

  // efficiency tool reco
  //-----------------
  //N.B. Reco still from Moriond18
  m_effToolReco = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_Reco");
  m_effToolReco->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListReco;
  correctionFileNameListReco.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v1/offline/efficiencySF.offline.RecoTrk.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolReco->setProperty("CorrectionFileNameList", correctionFileNameListReco));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolReco->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolReco->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolReco->initialize());

  // efficiency tool gradient Iso LooseLH
  //-----------------
  m_effToolIsoGradientLooseLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoGradientLooseLH");
  m_effToolIsoGradientLooseLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoGradientLooseLH;
  correctionFileNameListIsoGradientLooseLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.LooseAndBLayerLLH_d0z0_v13_Gradient.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientLooseLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoGradientLooseLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientLooseLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientLooseLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientLooseLH->initialize());

  // efficiency tool gradient Iso MediumLH
  //-----------------
  m_effToolIsoGradientMediumLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoGradientMediumLH");
  m_effToolIsoGradientMediumLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoGradientMediumLH;
  correctionFileNameListIsoGradientMediumLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.MediumLLH_d0z0_v13_Gradient.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientMediumLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoGradientMediumLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientMediumLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientMediumLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientMediumLH->initialize());

  // efficiency tool gradient Iso TightLH
  //-----------------
  m_effToolIsoGradientTightLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoGradientTightLH");
  m_effToolIsoGradientTightLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoGradientTightLH;
  correctionFileNameListIsoGradientTightLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.TightLLH_d0z0_v13_Gradient.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientTightLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoGradientTightLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientTightLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientTightLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoGradientTightLH->initialize());

  // efficiency tool FixedCutLoose Iso LooseLH
  //-----------------
  m_effToolIsoFixedCutLooseLooseLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutLooseLooseLH");
  m_effToolIsoFixedCutLooseLooseLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutLooseLooseLH;
  correctionFileNameListIsoFixedCutLooseLooseLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.LooseAndBLayerLLH_d0z0_v13_FCLoose.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseLooseLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutLooseLooseLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseLooseLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseLooseLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseLooseLH->initialize());

  // efficiency tool FixedCutLoose Iso MediumLH
  //-----------------
  m_effToolIsoFixedCutLooseMediumLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutLooseMediumLH");
  m_effToolIsoFixedCutLooseMediumLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutLooseMediumLH;
  correctionFileNameListIsoFixedCutLooseMediumLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.MediumLLH_d0z0_v13_FCLoose.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseMediumLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutLooseMediumLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseMediumLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseMediumLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseMediumLH->initialize());

  // efficiency tool FixedCutLoose Iso TightLH
  //-----------------
  m_effToolIsoFixedCutLooseTightLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutLooseTightLH");
  m_effToolIsoFixedCutLooseTightLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutLooseTightLH;
  correctionFileNameListIsoFixedCutLooseTightLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.TightLLH_d0z0_v13_FCLoose.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseTightLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutLooseTightLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseTightLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseTightLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutLooseTightLH->initialize());


  // efficiency tool FixedCutTight Iso LooseLH
  //-----------------
  m_effToolIsoFixedCutTightLooseLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutTightLooseLH");
  m_effToolIsoFixedCutTightLooseLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutTightLooseLH;
  correctionFileNameListIsoFixedCutTightLooseLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.LooseAndBLayerLLH_d0z0_v13_FCTight.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightLooseLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutTightLooseLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightLooseLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightLooseLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightLooseLH->initialize());

  // efficiency tool FixedCutTight Iso MediumLH
  //-----------------
  m_effToolIsoFixedCutTightMediumLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutTightMediumLH");
  m_effToolIsoFixedCutTightMediumLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutTightMediumLH;
  correctionFileNameListIsoFixedCutTightMediumLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.MediumLLH_d0z0_v13_FCTight.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightMediumLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutTightMediumLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightMediumLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightMediumLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightMediumLH->initialize());

  // efficiency tool FixedCutTight Iso TightLH
  //-----------------
  m_effToolIsoFixedCutTightTightLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutTightTightLH");
  m_effToolIsoFixedCutTightTightLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutTightTightLH;
  correctionFileNameListIsoFixedCutTightTightLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.TightLLH_d0z0_v13_FCTight.root"); 
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightTightLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutTightTightLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightTightLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightTightLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutTightTightLH->initialize());

  // efficiency tool FixedCutHighPtCaloOnly Iso LooseLH
  //-----------------
  m_effToolIsoFixedCutHighPtCaloOnlyLooseLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutHighPtCaloOnlyLooseLH");
  m_effToolIsoFixedCutHighPtCaloOnlyLooseLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutHighPtCaloOnlyLooseLH;
  correctionFileNameListIsoFixedCutHighPtCaloOnlyLooseLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.LooseAndBLayerLLH_d0z0_v13_FCHighPtCaloOnly.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyLooseLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutHighPtCaloOnlyLooseLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyLooseLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyLooseLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyLooseLH->initialize());

  // efficiency tool FixedCutHighPtCaloOnly Iso MediumLH
  //-----------------
  m_effToolIsoFixedCutHighPtCaloOnlyMediumLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutHighPtCaloOnlyMediumLH");
  m_effToolIsoFixedCutHighPtCaloOnlyMediumLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutHighPtCaloOnlyMediumLH;
  correctionFileNameListIsoFixedCutHighPtCaloOnlyMediumLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.MediumLLH_d0z0_v13_FCHighPtCaloOnly.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyMediumLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutHighPtCaloOnlyMediumLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyMediumLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyMediumLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyMediumLH->initialize());

  // efficiency tool FixedCutHighPtCaloOnly Iso TightLH
  //-----------------
  m_effToolIsoFixedCutHighPtCaloOnlyTightLH = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_IsoFixedCutHighPtCaloOnlyTightLH");
  m_effToolIsoFixedCutHighPtCaloOnlyTightLH->msg().setLevel( m_msgLevel );
  std::vector< std::string > correctionFileNameListIsoFixedCutHighPtCaloOnlyTightLH;
  correctionFileNameListIsoFixedCutHighPtCaloOnlyTightLH.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/isolation/efficiencySF.Isolation.TightLLH_d0z0_v13_FCHighPtCaloOnly.root");
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyTightLH->setProperty("CorrectionFileNameList", correctionFileNameListIsoFixedCutHighPtCaloOnlyTightLH));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyTightLH->setProperty("ForceDataType", dataType));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyTightLH->setProperty("CorrelationModel", "TOTAL"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_effToolIsoFixedCutHighPtCaloOnlyTightLH->initialize());

  //LH selector tools
  //-----------------
  //    std::string confDir = "ElectronPhotonSelectorTools/offline/mc15_20160512/"; // release 20.7
    std::string confDir = "ElectronPhotonSelectorTools/offline/mc16_20170828/"; //release 21
  
  //VeryLooseLH
  m_checkVeryLooseLH = new AsgElectronLikelihoodTool("checkVeryLooseLH");
  m_checkVeryLooseLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkVeryLooseLH->setProperty("primaryVertexContainer", "PrimaryVertices"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkVeryLooseLH->setProperty("ConfigFile",confDir+"ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth.conf")); //NO SF for this WP
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkVeryLooseLH->initialize());
  
  //LooseNoBLLH
  m_checkLooseNoBLLH = new AsgElectronLikelihoodTool("checkLooseNoBLLH");
  m_checkLooseNoBLLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkLooseNoBLLH->setProperty("primaryVertexContainer", "PrimaryVertices"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkLooseNoBLLH->setProperty("ConfigFile",confDir+"ElectronLikelihoodLooseOfflineConfig2017_Smooth.conf"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkLooseNoBLLH->initialize());

  //LooseLH
  m_checkLooseLH = new AsgElectronLikelihoodTool("checkLooseLH");
  m_checkLooseLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkLooseLH->setProperty("primaryVertexContainer", "PrimaryVertices"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkLooseLH->setProperty("ConfigFile",confDir+"ElectronLikelihoodLooseOfflineConfig2017_CutBL_Smooth.conf"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkLooseLH->initialize());

  //MediumLH
  m_checkMediumLH = new AsgElectronLikelihoodTool("checkMediumLH");
  m_checkMediumLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkMediumLH->setProperty("primaryVertexContainer", "PrimaryVertices"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkMediumLH->setProperty("ConfigFile",confDir+"ElectronLikelihoodMediumOfflineConfig2017_Smooth.conf"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkMediumLH->initialize());
  //TightLH
  m_checkTightLH = new AsgElectronLikelihoodTool("checkTightLH");
  m_checkTightLH->msg().setLevel( m_msgLevel );
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkTightLH->setProperty("primaryVertexContainer", "PrimaryVertices"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkTightLH->setProperty("ConfigFile",confDir+"ElectronLikelihoodTightOfflineConfig2017_Smooth.conf"));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_checkTightLH->initialize());


  //Isolation WP under study
  m_isIso = new CP::IsolationSelectionTool("ElectronIso");
  TOOL_CHECK("ElectronHandler::initializeTools()",m_isIso->setProperty("ElectronWP", "FCLoose"));
  TOOL_CHECK("ElectronHandler::initializeTools()",m_isIso->initialize());
  //Add all the WP you may want to check in the TAccept output
  TOOL_CHECK("ElectronHandler::initializeTools()",m_isIso->addElectronWP("FCTight"));
  TOOL_CHECK("ElectronHandler::initializeTools()",m_isIso->addElectronWP("Gradient"));
  TOOL_CHECK("ElectronHandler::initializeTools()",m_isIso->addElectronWP("FCHighPtCaloOnly"));
  TOOL_CHECK("ElectronHandler::initializeTools()",m_isIso->addElectronWP("FCLoose_FixedRad"));
  TOOL_CHECK("ElectronHandler::initializeTools()",m_isIso->addElectronWP("FCTight_FixedRad"));

  //Isolation Leachage Correction Tool
  m_isoCorr_tool = new CP::IsolationCorrectionTool("isoCorr_tool_electron");
  //  TOOL_CHECK("ElectronHandler::initializeTools()", m_isoCorr_tool->setProperty("Apply_datadriven", false));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_isoCorr_tool->setProperty("IsMC", m_eventInfoHandler.get_isMC() ? true : false));
  TOOL_CHECK("ElectronHandler::initializeTools()", m_isoCorr_tool->initialize());
 

  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear();
  m_sysToolList.push_back(m_EgammaCalibrationAndSmearingTool);
  m_sysToolList.push_back(m_trigEffToolLooseLHIsoFixedCutLoose);
  m_sysToolList.push_back(m_trigEffToolTightLHIsoFixedCutTight);
  m_sysToolList.push_back(m_trigEffToolTightLHIsoFixedCutLoose);
  m_sysToolList.push_back(m_e17trigEffToolMediumLHIsoFixedCutLoose);
  m_sysToolList.push_back(m_trigEffToolMediumLH_offTightIsoFixedCutLoose);
  m_sysToolList.push_back(m_trigToolLooseLHIsoFixedCutLoose);
  m_sysToolList.push_back(m_trigToolTightLHIsoFixedCutTight);
  m_sysToolList.push_back(m_trigToolTightLHIsoFixedCutLoose);
  m_sysToolList.push_back(m_e17trigToolMediumLHIsoFixedCutLoose);
  m_sysToolList.push_back(m_trigToolMediumLH_offTightIsoFixedCutLoose);
  m_sysToolList.push_back(m_effToolLooseLH);
  m_sysToolList.push_back(m_effToolMediumLH);
  m_sysToolList.push_back(m_effToolTightLH);
  m_sysToolList.push_back(m_effToolReco);
  m_sysToolList.push_back(m_effToolIsoGradientLooseLH);
  m_sysToolList.push_back(m_effToolIsoGradientMediumLH);
  m_sysToolList.push_back(m_effToolIsoGradientTightLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutLooseLooseLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutLooseMediumLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutLooseTightLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutTightLooseLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutTightMediumLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutTightTightLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutHighPtCaloOnlyLooseLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutHighPtCaloOnlyMediumLH);
  m_sysToolList.push_back(m_effToolIsoFixedCutHighPtCaloOnlyTightLH);

  // register Trigger Matching Tools
  // -------------------------------
  // Commented out until Egamma Matching Tool validated
  m_trigDecTool = m_eventInfoHandler.get_TrigDecTool();

  if (m_trigDecTool){

    ToolHandle<Trig::TrigDecisionTool> m_trigDec(m_trigDecTool);
    TOOL_CHECK("ElectronHandler::initializeTools()",m_trigDec.retrieve());  
    m_match_Tool.setTypeAndName("Trig::MatchingTool/ElectronMatchingTool");
    TOOL_CHECK("ElectronHandler::initializeTools()",m_match_Tool.retrieve());

  }

  std::unordered_map<std::string, PROP<int>*> triggersToMatch {
      {"HLT_e26_tight_iloose",&Props::matchHLT_e26_tight_iloose},
      {"HLT_e26_lhtight_iloose",&Props::matchHLT_e26_lhtight_iloose},
      {"HLT_e60_medium",&Props::matchHLT_e60_medium},
      {"HLT_e60_lhmedium",&Props::matchHLT_e60_lhmedium},
      {"HLT_e17_lhloose_L1EM15",&Props::matchHLT_e17_lhloose_L1EM15},
      {"HLT_e24_lhvloose_L1EM18VH",&Props::matchHLT_e24_lhvloose_L1EM18VH},
      {"HLT_e24_lhvloose_L1EM20VH",&Props::matchHLT_e24_lhvloose_L1EM20VH},
      {"HLT_e24_lhmedium_L1EM18VH",&Props::matchHLT_e24_lhmedium_L1EM18VH},
      {"HLT_e24_lhmedium_L1EM20VH",&Props::matchHLT_e24_lhmedium_L1EM20VH},
      {"HLT_e24_lhmedium_iloose_L1EM18VH",&Props::matchHLT_e24_lhmedium_iloose_L1EM18VH},
      {"HLT_e24_lhmedium_iloose_L1EM20VH",&Props::matchHLT_e24_lhmedium_iloose_L1EM20VH},
      {"HLT_e24_medium_iloose_L1EM18VH",&Props::matchHLT_e24_medium_iloose_L1EM18VH},
      {"HLT_e24_medium_iloose_L1EM20VH",&Props::matchHLT_e24_medium_iloose_L1EM20VH},
      {"HLT_e24_lhtight_iloose",&Props::matchHLT_e24_lhtight_iloose},
      {"HLT_e120_lhloose", &Props::matchHLT_e120_lhloose},
      {"HLT_e140_lhloose", &Props::matchHLT_e140_lhloose},
      {"HLT_e26_lhtight_ivarloose", &Props::matchHLT_e26_lhtight_ivarloose},
      {"HLT_e26_lhmedium_nod0_ivarloose", &Props::matchHLT_e26_lhmedium_nod0_ivarloose},
      {"HLT_e24_lhtight_ivarloose", &Props::matchHLT_e24_lhtight_ivarloose},
      {"HLT_e24_lhtight_nod0_ivarloose", &Props::matchHLT_e24_lhtight_nod0_ivarloose},
      {"HLT_e24_lhmedium_ivarloose", &Props::matchHLT_e24_lhmedium_ivarloose},
      {"HLT_e24_lhmedium_nod0_ivarloose", &Props::matchHLT_e24_lhmedium_nod0_ivarloose},
      {"HLT_e24_lhmedium_nod0_L1EM20VH", &Props::matchHLT_e24_lhmedium_nod0_L1EM20VH},
      {"HLT_e26_lhtight_nod0_ivarloose", &Props::matchHLT_e26_lhtight_nod0_ivarloose},
      {"HLT_e26_lhtight_smooth_ivarloose", &Props::matchHLT_e26_lhtight_smooth_ivarloose},
      {"HLT_e60_lhmedium_nod0", &Props::matchHLT_e60_lhmedium_nod0},
      {"HLT_e80_lhmedium_nod0", &Props::matchHLT_e80_lhmedium_nod0},
      {"HLT_e120_lhloose_nod0", &Props::matchHLT_e120_lhloose_nod0},
      {"HLT_e140_lhloose_nod0", &Props::matchHLT_e140_lhloose_nod0},
      {"HLT_e300_etcut", &Props::matchHLT_e300_etcut},
      {"HLT_2e12_lhloose_L12EM10VH", &Props::matchHLT_2e12_lhloose_L12EM10VH},
      {"HLT_2e17_lhvloose_nod0", &Props::matchHLT_2e17_lhvloose_nod0},
      {"HLT_2e17_lhvloose_nod0_L12EM15VHI", &Props::matchHLT_2e17_lhvloose_nod0_L12EM15VHI},
      {"HLT_2e24_lhvloose_nod0", &Props::matchHLT_2e24_lhvloose_nod0},
      {"HLT_e17_lhloose_mu14", &Props::matchHLT_e17_lhloose_mu14},
      {"HLT_e17_lhloose_nod0_mu14", &Props::matchHLT_e17_lhloose_nod0_mu14},
      {"HLT_e26_lhmedium_nod0_mu8noL1", &Props::matchHLT_e26_lhmedium_nod0_mu8noL1},
      {"HLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1", &Props::matchHLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1},
      {"HLT_e7_lhmedium_mu24", &Props::matchHLT_e7_lhmedium_mu24},
      {"HLT_e7_lhmedium_nod0_mu24", &Props::matchHLT_e7_lhmedium_nod0_mu24}
  };
  m_triggersToMatch = triggersToMatch;

  return EL::StatusCode::SUCCESS;
}

bool ElectronHandler::passLooseElectron(xAOD::Electron * electron) {
  bool passSel = true;

  passSel &= (Props::isLooseLH.get(electron));
  passSel &= (fabs(electron->eta()) < 2.47);
  passSel &= (electron->pt() > 7000.);

  Props::passPreSel.set(electron, passSel);
  return passSel;
}

EL::StatusCode ElectronHandler::decorateOriginParticle(const xAOD::Electron * electron)
{
  m_doTrigMatch = false;
  if (m_event->contains<xAOD::ElectronContainer>("HLT_xAOD__ElectronContainer_egamma_Electrons")) {
    m_doTrigMatch = true;
  }

  // All values here are independent of energy calibration
  const xAOD::TrackParticle *trackPart = electron->trackParticle();
  //Pt > 7e3 : to prevent the warning to appear in case of skimmed derivations with a lower cut on the eletron track Pt. 
  if (electron->pt() > 7000 && !trackPart)  Warning("ElectronHandler::decorateOriginParticle",
						       "Failed to get the TrackParticle to retrieve track pt/eta/phi...");
     
  //selection tools
  //---------------
  //retrieve decision from tools and decorate electron with it
  //perform actual selection later
  int isVeryLooseLH = 0;
  int isLooseNoBLLH = 0;
  int isLooseLH = 0;
  int isMediumLH = 0;
  int isTightLH = 0;
  if(trackPart){
    isVeryLooseLH = electron->isAvailable<char>("DFCommonElectronsLHVeryLoose") ? static_cast<int>(electron->auxdata<char>("DFCommonElectronsLHVeryLoose")) : static_cast<int>(m_checkVeryLooseLH->accept(electron));
    
    isLooseNoBLLH = electron -> isAvailable<char>("DFCommonElectronsLHLoose") ? static_cast<int>(electron -> auxdata<char>("DFCommonElectronsLHLoose")) : static_cast<int>(m_checkLooseNoBLLH->accept(electron));
  
    // LooseLH < MediumLH < TightLH
    isLooseLH = electron -> isAvailable<char>("DFCommonElectronsLHLooseBL") ? static_cast<int>(electron -> auxdata<char>("DFCommonElectronsLHLooseBL")) : static_cast<int>(m_checkLooseLH->accept(electron));
    if(isLooseLH) {
      isMediumLH = electron -> isAvailable<char>("DFCommonElectronsLHMedium") ? static_cast<int>(electron -> auxdata<char>("DFCommonElectronsLHMedium")) : static_cast<int>(m_checkMediumLH->accept(electron));
      if(isMediumLH) {
	isTightLH = electron -> isAvailable<char>("DFCommonElectronsLHTight") ? static_cast<int>(electron -> auxdata<char>("DFCommonElectronsLHTight")) : static_cast<int>(m_checkTightLH->accept(electron));
      }
    }
  }
  
  Props::isVeryLooseLH.set(electron, isVeryLooseLH);
  Props::isLooseNoBLLH.set(electron, isLooseNoBLLH);
  Props::isLooseLH.set(electron, isLooseLH);
  Props::isMediumLH.set(electron, isMediumLH);
  Props::isTightLH.set(electron, isTightLH);

  //Trigger Matching
 if(m_doTrigMatch){
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    std::string trig_name;
    std::vector<const xAOD::IParticle*> myParticles;
    myParticles.clear();
    myParticles.push_back(electron);

    // only call the trigger matching for the triggers that really need it !
    // loop over the triggers we declared as match-able for electrons
    for (auto& trig : m_triggersToMatch) {
      // check if it is in the trigDecorators. Maybe we don't want this trigger.
      if(trigDecorators.count(trig.first)) {
        trig_name = trig.first;
        int is_matched = 0;
        //++++++++ TODO: check why this trigger "e24_lhmedium_iloose_L1EM20VH" throws a warning!
        // Do not do trigger matching if the electron is not passing any ID requirement.
        if(isVeryLooseLH) {
          if (m_match_Tool->match(myParticles,trig_name,0.07)) is_matched = 1;
        }
        trig.second->set(electron, is_matched);
      }
    }
  }

  ///Track properties: do we need them?
  Props::innerTrackPt.set(electron, !trackPart ? 0. : trackPart->pt());
  Props::innerTrackEta.set(electron, !trackPart ? 0. : trackPart->eta());
  Props::innerTrackPhi.set(electron, !trackPart ? 0. : trackPart->phi());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ElectronHandler::decorate(xAOD::Electron * electron)
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

  Props::isFixedCutLooseIso.set(electron, isoAccept.getCutResult("FCLoose"));
  Props::isFixedCutTightIso.set(electron, isoAccept.getCutResult("FCTight"));
  Props::isGradientIso.set(electron, isoAccept.getCutResult("Gradient"));
  Props::isFixedCutHighPtCaloOnlyIso.set(electron, isoAccept.getCutResult("FCHighPtCaloOnly"));
  Props::isFixedCutLooseFixedRadIso.set(electron, isoAccept.getCutResult("FCLoose_FixedRad"));
  Props::isFixedCutTightFixedRadIso.set(electron, isoAccept.getCutResult("FCTight_FixedRad"));


  //compute d0, z0
  //---------------
  float pvZ = 0., d0sig_BL = 0., z0 = 0, z0sinTheta = 0.;
  //  float  d0 = 0., d0sig = 0., z0sig_BL = -99999.;
  const xAOD::TrackParticle *trackPart = electron->trackParticle();
  if(trackPart){
    pvZ = m_eventInfoHandler.get_ZPV();
    // d0 = trackPart->d0(); //clean electron handler
    //    d0sig = xAOD::TrackingHelpers::d0significance(trackPart); //clean electron handler
    d0sig_BL = xAOD::TrackingHelpers::d0significance(trackPart,m_eventInfoHandler.get_beamPosUncX(),m_eventInfoHandler.get_beamPosUncY(),m_eventInfoHandler.get_beamPosCorrXY());
    z0 = trackPart->z0()+ trackPart->vz() - pvZ;
    z0sinTheta = z0*TMath::Sin(trackPart->theta());

    //clean electron handler (keep it commented if somebody complains)
    // z0 significance
    //    static SG::AuxElement::Accessor< std::vector<float> > accTrackParCov( "definingParametersCovMatrix" );
    //if( accTrackParCov.isAvailable( *trackPart ) ) {
    // trackPart->definingParametersCovMatrixVec().at(2);
    //      float sigma_z0 = trackPart->definingParametersCovMatrixVec().at(2);
    //      if ( sigma_z0 > 0. ) { 
    //	z0sig_BL = z0 / sqrt(sigma_z0);
    //}
    //else z0sig_BL = 99999.;
    //}
  }else {
    Warning("ElectronHandler::calibrateCopies","Failed to get the TrackParticle to retrieve d0/z0...");
  }

  float charge=electron->charge();
  Props::charge.set(electron, charge); // TODO: this is redundant - charge is a member of the object already! Maybe we should remove it!
  
  if(m_doResolution){
    const xAOD::Egamma* aaa=electron;
    float res=-1;
    res=m_EgammaCalibrationAndSmearingTool->getResolution(*aaa, false);
    Props::resolution.set(electron, res);
  }

  // get SFs from electron SF tool
  //--------------------------
  int isMC = m_eventInfoHandler.get_isMC();
  double trigEFFlooseLHIsoFixedCutLoose = 1.;
  double trigEFFtightLHIsoFixedCutTight = 1.;
  double trigEFFtightLHIsoFixedCutLoose = 1.;
  double e17trigEFFmediumLHIsoFixedCutLoose = 1.;
  double trigEFFmediumLH_offTightIsoFixedCutLoose = 1.;
  double trigSFlooseLHIsoFixedCutLoose = 1.;
  double trigSFtightLHIsoFixedCutTight = 1.;
  double trigSFtightLHIsoFixedCutLoose = 1.;
  double e17trigSFmediumLHIsoFixedCutLoose = 1.;
  double trigSFmediumLH_offTightIsoFixedCutLoose = 1.;
 
  double effSFlooseLH = 1.;
  double effSFmediumLH = 1.;
  double effSFtightLH = 1.;
  double effSFReco = 1.;
  double effSFIsoGradientLooseLH = 1.;
  double effSFIsoGradientMediumLH = 1.;
  double effSFIsoGradientTightLH = 1.;
  double effSFIsoFixedCutLooseLooseLH = 1.;
  double effSFIsoFixedCutLooseMediumLH = 1.;
  double effSFIsoFixedCutLooseTightLH = 1.;
  double effSFIsoFixedCutTightLooseLH = 1.;
  double effSFIsoFixedCutTightMediumLH = 1.;
  double effSFIsoFixedCutTightTightLH = 1.;
  double effSFIsoFixedCutHighPtCaloOnlyLooseLH = 1.;
  double effSFIsoFixedCutHighPtCaloOnlyMediumLH = 1.;
  double effSFIsoFixedCutHighPtCaloOnlyTightLH = 1.;

  // TODO remove range cuts, but tool prints error message currently
  // http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/PhysicsAnalysis/ElectronPhotonID/ElectronEfficiencyCorrection/Root/AsgElectronEfficiencyCorrectionTool.cxx

  const xAOD::CaloCluster* cluster = electron->caloCluster();
  float cluster_eta = 10;
  if (cluster) {
    //cluster_eta = cluster->eta();
    cluster_eta = cluster->etaBE(2); 
  }
  Props::ElClusterEta.set(electron, cluster_eta);

  float PromptLeptonIsoEl = (electron)->auxdata< float >("PromptLeptonIso");
  float PromptLeptonVetoEl = (electron)->auxdata< float >("PromptLeptonVeto");

  //NM 16-01-19: egamma recommendations now start at 7GeV, so scale factor should be there... to check
  if (isMC && electron->pt() >= 7000. && fabs(cluster_eta) < 2.47) {//TODO: Check again this safety guard after full-recommendations are out!
    if (m_effToolReco->getEfficiencyScaleFactor(*electron, effSFReco) == CP::CorrectionCode::Error) {
      Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
      return EL::StatusCode::FAILURE;
    }
    // do not compute SF if the ID does not match the SF requirement...
    if(isLooseLH) {
      if (m_trigToolLooseLHIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, trigSFlooseLHIsoFixedCutLoose) == CP::CorrectionCode::Error) {
        Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
      if (m_trigEffToolLooseLHIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, trigEFFlooseLHIsoFixedCutLoose) == CP::CorrectionCode::Error) {
        Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
      if (m_effToolLooseLH->getEfficiencyScaleFactor(*electron, effSFlooseLH) == CP::CorrectionCode::Error) {
	Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
	return EL::StatusCode::FAILURE;
      }
      if (m_effToolIsoGradientLooseLH->getEfficiencyScaleFactor(*electron, effSFIsoGradientLooseLH) == CP::CorrectionCode::Error) {
        Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
      if (m_effToolIsoFixedCutLooseLooseLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutLooseLooseLH) == CP::CorrectionCode::Error) {
        Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
      if (m_effToolIsoFixedCutTightLooseLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutTightLooseLH) == CP::CorrectionCode::Error) {
        Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
      if (m_effToolIsoFixedCutHighPtCaloOnlyLooseLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutHighPtCaloOnlyLooseLH) == CP::CorrectionCode::Error) {
        Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
      if(isMediumLH) {

	if (m_e17trigToolMediumLHIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, e17trigSFmediumLHIsoFixedCutLoose) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
	if (m_e17trigEffToolMediumLHIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, e17trigEFFmediumLHIsoFixedCutLoose) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
	if (m_trigToolMediumLH_offTightIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, trigSFmediumLH_offTightIsoFixedCutLoose) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
	if (m_trigEffToolMediumLH_offTightIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, trigEFFmediumLH_offTightIsoFixedCutLoose) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
	if (m_effToolMediumLH->getEfficiencyScaleFactor(*electron, effSFmediumLH) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
        if (m_effToolIsoGradientMediumLH->getEfficiencyScaleFactor(*electron, effSFIsoGradientMediumLH) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
        if (m_effToolIsoFixedCutLooseMediumLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutLooseMediumLH) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
        if (m_effToolIsoFixedCutTightMediumLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutTightMediumLH) == CP::CorrectionCode::Error) {
          Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
          return EL::StatusCode::FAILURE;
        }
	if (m_effToolIsoFixedCutHighPtCaloOnlyMediumLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutHighPtCaloOnlyMediumLH) == CP::CorrectionCode::Error) {
	  Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
	  return EL::StatusCode::FAILURE;
	}
        if(isTightLH) {
	  if (m_trigToolTightLHIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, trigSFtightLHIsoFixedCutLoose) == CP::CorrectionCode::Error) {
            Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
	  if (m_trigEffToolTightLHIsoFixedCutLoose->getEfficiencyScaleFactor(*electron, trigEFFtightLHIsoFixedCutLoose) == CP::CorrectionCode::Error) {
            Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
	  if (m_trigToolTightLHIsoFixedCutTight->getEfficiencyScaleFactor(*electron, trigSFtightLHIsoFixedCutTight) == CP::CorrectionCode::Error) {
            Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
	  if (m_trigEffToolTightLHIsoFixedCutTight->getEfficiencyScaleFactor(*electron, trigEFFtightLHIsoFixedCutTight) == CP::CorrectionCode::Error) {
	    Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
	    return EL::StatusCode::FAILURE;
	  }
          if (m_effToolTightLH->getEfficiencyScaleFactor(*electron, effSFtightLH) == CP::CorrectionCode::Error) {
            Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
	  if (m_effToolIsoGradientTightLH->getEfficiencyScaleFactor(*electron, effSFIsoGradientTightLH) == CP::CorrectionCode::Error) {
	    Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
	    return EL::StatusCode::FAILURE;
	  }
          if (m_effToolIsoFixedCutLooseTightLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutLooseTightLH) == CP::CorrectionCode::Error) {
            Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
          if (m_effToolIsoFixedCutTightTightLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutTightTightLH) == CP::CorrectionCode::Error) {
            Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
	  if (m_effToolIsoFixedCutHighPtCaloOnlyTightLH->getEfficiencyScaleFactor(*electron, effSFIsoFixedCutHighPtCaloOnlyTightLH) == CP::CorrectionCode::Error) {
            Error("ElectronHandler::decorate()", "ElectronEfficiencyCorrection returned CP::CorrectionCode::Error");
            return EL::StatusCode::FAILURE;
          }
        }
      }
    }
  }
  Props::trigEFFlooseLHIsoFixedCutLoose.set(electron, trigEFFlooseLHIsoFixedCutLoose);
  Props::trigSFlooseLHIsoFixedCutLoose.set(electron, trigSFlooseLHIsoFixedCutLoose);
  Props::trigEFFtightLHIsoFixedCutTight.set(electron, trigEFFtightLHIsoFixedCutTight);
  Props::trigSFtightLHIsoFixedCutTight.set(electron, trigSFtightLHIsoFixedCutTight);
  Props::trigEFFtightLHIsoFixedCutLoose.set(electron, trigEFFtightLHIsoFixedCutLoose);
  Props::trigSFtightLHIsoFixedCutLoose.set(electron, trigSFtightLHIsoFixedCutLoose);
  Props::e17trigEFFmediumLHIsoFixedCutLoose.set(electron, e17trigEFFmediumLHIsoFixedCutLoose);
  Props::e17trigSFmediumLHIsoFixedCutLoose.set(electron, e17trigSFmediumLHIsoFixedCutLoose);
  Props::trigEFFmediumLH_offTightIsoFixedCutLoose.set(electron, trigEFFmediumLH_offTightIsoFixedCutLoose);
  Props::trigSFmediumLH_offTightIsoFixedCutLoose.set(electron, trigSFmediumLH_offTightIsoFixedCutLoose);
  Props::effSFlooseLH.set(electron, effSFlooseLH);
  Props::effSFmediumLH.set(electron, effSFmediumLH);
  Props::effSFtightLH.set(electron, effSFtightLH);
  Props::effSFReco.set(electron, effSFReco);
  Props::effSFIsoGradientLooseLH.set(electron, effSFIsoGradientLooseLH);
  Props::effSFIsoGradientMediumLH.set(electron, effSFIsoGradientMediumLH);
  Props::effSFIsoGradientTightLH.set(electron, effSFIsoGradientTightLH);
  Props::effSFIsoFixedCutLooseLooseLH.set(electron,  effSFIsoFixedCutLooseLooseLH);
  Props::effSFIsoFixedCutLooseMediumLH.set(electron, effSFIsoFixedCutLooseMediumLH);
  Props::effSFIsoFixedCutLooseTightLH.set(electron,  effSFIsoFixedCutLooseTightLH);
  Props::effSFIsoFixedCutTightLooseLH.set(electron,  effSFIsoFixedCutTightLooseLH);
  Props::effSFIsoFixedCutTightMediumLH.set(electron, effSFIsoFixedCutTightMediumLH);
  Props::effSFIsoFixedCutTightTightLH.set(electron,  effSFIsoFixedCutTightTightLH);
  Props::effSFIsoFixedCutHighPtCaloOnlyLooseLH.set(electron,  effSFIsoFixedCutHighPtCaloOnlyLooseLH);
  Props::effSFIsoFixedCutHighPtCaloOnlyMediumLH.set(electron, effSFIsoFixedCutHighPtCaloOnlyMediumLH);
  Props::effSFIsoFixedCutHighPtCaloOnlyTightLH.set(electron,  effSFIsoFixedCutHighPtCaloOnlyTightLH);

  //Props::d0.set(electron, d0); //commented out to save space
  //Props::d0sig.set(electron, d0sig); //commented out to save space
  Props::d0sigBL.set(electron, d0sig_BL);
  //Props::z0.set(electron, z0); //commented out to save space
  Props::z0sinTheta.set(electron, z0sinTheta);
  // Props::z0sigBL.set(electron, z0sig_BL); //commented out to save space

  Props::PromptLeptonIsoEl.set(electron, PromptLeptonIsoEl);
  Props::PromptLeptonVetoEl.set(electron, PromptLeptonVetoEl);

  Props::forMETRebuild.set(electron, false);//setting as false as default for MET rebuilding
  return EL::StatusCode::SUCCESS;  

}


EL::StatusCode ElectronHandler::calibrate() {
  // Important to call the parent function!!
  return ObjectHandler<xAOD::ElectronContainer>::calibrate();
}


//void ElectronHandler::getHLTElectronObjects(){

EL::StatusCode ElectronHandler::calibrateCopies(xAOD::ElectronContainer * particles, const CP::SystematicSet & sysSet) 
{

  //calibration tool
  //-----------------
  // tell tool to apply systematic variation
  CP_CHECK("ElectronHandler::calibrateCopies()",m_EgammaCalibrationAndSmearingTool->applySystematicVariation(sysSet),m_debug);

  // tell tool to apply systematic variation
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigEffToolLooseLHIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigToolLooseLHIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigEffToolTightLHIsoFixedCutTight->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigToolTightLHIsoFixedCutTight->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigEffToolTightLHIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigToolTightLHIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_e17trigEffToolMediumLHIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_e17trigToolMediumLHIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigEffToolMediumLH_offTightIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_trigToolMediumLH_offTightIsoFixedCutLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolLooseLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolMediumLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolTightLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolReco->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoGradientLooseLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoGradientMediumLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoGradientTightLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutLooseLooseLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutLooseMediumLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutLooseTightLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutTightLooseLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutTightMediumLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutTightTightLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutHighPtCaloOnlyLooseLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutHighPtCaloOnlyMediumLH->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("ElectronHandler::calibrateCopies()",m_effToolIsoFixedCutHighPtCaloOnlyTightLH->applySystematicVariation(sysSet),m_debug);

  for (xAOD::Electron * electron : *particles) {

    //calibration tool
    //-----------------

    if (!electron->trackParticle(0)) continue;
    // Ne need to compute energy correction for electrons that are not even LHLoose
    int isVeryLooseLH  = Props::isVeryLooseLH.get(electron);
    if(isVeryLooseLH) {
      CP_CHECK("ElectronHandler::calibrateCopies()",m_EgammaCalibrationAndSmearingTool->applyCorrection(*electron),m_debug);
    }

    //TODO: follow up and check this is fixed
    const xAOD::CaloCluster* cluster = electron->caloCluster();
    float cluster_et = cluster->e() / cosh(cluster->etaBE(2));
    if(cluster_et/1000. > 7. && fabs(cluster->etaBE(2)) < 2.47){
          CP_CHECK("ElectronHandler::calibrateCopies()", m_isoCorr_tool->CorrectLeakage(*electron), m_debug);
    }


    // decorate electron
   
    if ( decorate( electron ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
  }
  return EL::StatusCode::SUCCESS;

}


EL::StatusCode ElectronHandler::writeOutputVariables(xAOD::Electron * inElectron, xAOD::Electron * outElectron, bool isKinVar, bool isWeightVar, const TString& sysName) 
{
  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    EL_CHECK("ElectronHandler::writeOutputVariables",writeAllVariations(inElectron, outElectron, sysName));
  } 
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    EL_CHECK("ElectronHandler::writeOutputVariables",writeKinematicVariations(inElectron, outElectron, sysName));
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes  
    EL_CHECK("ElectronHandler::writeOutputVariables",writeWeightVariations(inElectron, outElectron, sysName));
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited 
    EL_CHECK("ElectronHandler::writeOutputVariables",writeAllVariations(inElectron, outElectron, sysName));
    EL_CHECK("ElectronHandler::writeOutputVariables",writeKinematicVariations(inElectron, outElectron, sysName));
    EL_CHECK("ElectronHandler::writeOutputVariables",writeWeightVariations(inElectron, outElectron, sysName));
    EL_CHECK("ElectronHandler::writeOutputVariables",writeNominal(inElectron, outElectron, sysName));
  } else assert(false);
 
  return writeCustomVariables(inElectron, outElectron, isKinVar, isWeightVar, sysName);

}

EL::StatusCode ElectronHandler::writeAllVariations(xAOD::Electron*, xAOD::Electron*, const TString&)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ElectronHandler::writeKinematicVariations(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& /*sysName*/)
{
  // 4-vector components affected by variations:
  Props::pt.copy(inElectron, outElectron);
  
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ElectronHandler::writeWeightVariations(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& sysName)
{
  
  // Write variables only for specific variation. Has to be written to Nominal as well.
  bool isTrigVar = (sysName == "Nominal" || sysName.BeginsWith("EL_EFF_Trigger_"));
  bool isIDVar   = (sysName == "Nominal" || sysName.BeginsWith("EL_EFF_ID_"));
  bool isRecoVar = (sysName == "Nominal" || sysName.BeginsWith("EL_EFF_Reco_"));
  bool isIsoVar  = (sysName == "Nominal" || sysName.BeginsWith("EL_EFF_Iso_"));
  
  // SF are only weight changes
  if (m_eventInfoHandler.get_isMC() && isTrigVar) {
    Props::trigSFlooseLHIsoFixedCutLoose.copy(inElectron, outElectron);
    Props::trigSFtightLHIsoFixedCutTight.copy(inElectron, outElectron);
    Props::trigSFtightLHIsoFixedCutLoose.copy(inElectron, outElectron);
    Props::e17trigSFmediumLHIsoFixedCutLoose.copy(inElectron, outElectron);
    Props::trigSFmediumLH_offTightIsoFixedCutLoose.copy(inElectron, outElectron);
  }

  if (m_eventInfoHandler.get_isMC() && isIDVar) {
    Props::effSFlooseLH.copy(inElectron, outElectron);
    Props::effSFmediumLH.copy(inElectron, outElectron);
    Props::effSFtightLH.copy(inElectron, outElectron);
  }

  if (m_eventInfoHandler.get_isMC() && isRecoVar) {
    Props::effSFReco.copy(inElectron, outElectron);
  }

  if (m_eventInfoHandler.get_isMC() && isIsoVar) {
    Props::effSFIsoGradientLooseLH.copy(inElectron, outElectron);
    Props::effSFIsoGradientMediumLH.copy(inElectron, outElectron);
    Props::effSFIsoGradientTightLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutLooseLooseLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutLooseMediumLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutLooseTightLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutTightLooseLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutTightMediumLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutTightTightLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutHighPtCaloOnlyLooseLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutHighPtCaloOnlyMediumLH.copy(inElectron, outElectron);
    Props::effSFIsoFixedCutHighPtCaloOnlyTightLH.copy(inElectron, outElectron);
}

  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ElectronHandler::writeNominal(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& /*sysName*/)
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
  bool applyNewToolOR = false;
  bool applyVBFGammaOR = false;
  bool removeOverlap = true;
  m_config.getif<bool>("applyNewToolOR", applyNewToolOR);
  m_config.getif<bool>("removeOverlap",removeOverlap);
  m_config.getif<bool>("applyVBFGammaOR", applyVBFGammaOR);
  bool writeFlagForEleMuOR = !(applyNewToolOR || applyVBFGammaOR); // only write if using the legacy overlap removal
  if(removeOverlap && writeFlagForEleMuOR){ //decoration only exists if OR tool is run
    Props::hasSharedTrack.copy(inElectron, outElectron);
  }
  
  //set isolation
  // old only
  if(inElectron->isAvailable< float >("ptvarcone20")){
    outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::ptvarcone20),xAOD::Iso::ptvarcone20);}
  // current
  if(inElectron->isAvailable< float >("ptvarcone20_TightTTVA_pt1000")){
    outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::ptvarcone20_TightTTVA_pt1000),xAOD::Iso::ptvarcone20_TightTTVA_pt1000);}
  // current
  if(inElectron->isAvailable< float >("topoetcone20")){
    outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::topoetcone20),xAOD::Iso::topoetcone20);}
  // technically no sfs for pflow or other r&d wps at the moment, but some may come at some point
  // if(inElectron->isAvailable< float >("ptvarcone30_TightTTVA_pt1000")){
  //   outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::ptvarcone30_TightTTVA_pt1000),xAOD::Iso::ptvarcone30_TightTTVA_pt1000);}
  // if(inElectron->isAvailable< float >("ptvarcone40")){
  //   outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::ptvarcone40),xAOD::Iso::ptvarcone40);}
  
  // new isolation WPs
  if(m_saveNonCrucialVariables){
    Props::isGradientIso.copy(inElectron, outElectron);
  }
  Props::isFixedCutTightIso.copy(inElectron, outElectron);
  Props::isFixedCutLooseIso.copy(inElectron, outElectron);
  Props::isFixedCutHighPtCaloOnlyIso.copy(inElectron, outElectron);
  Props::isFixedCutLooseFixedRadIso.copy(inElectron, outElectron);
  Props::isFixedCutTightFixedRadIso.copy(inElectron, outElectron);

  //identification WPs
  Props::isVeryLooseLH.copy(inElectron, outElectron);
  Props::isLooseNoBLLH.copy(inElectron, outElectron);
  Props::isLooseLH.copy(inElectron, outElectron);
  Props::isMediumLH.copy(inElectron, outElectron);
  Props::isTightLH.copy(inElectron, outElectron);
  
  if(m_saveNonCrucialVariables){
    //inner track
    Props::innerTrackPt.copy(inElectron, outElectron);
    Props::innerTrackEta.copy(inElectron, outElectron);
    Props::innerTrackPhi.copy(inElectron, outElectron);

    //cmmented out to save space
    //Props::d0.copy(inElectron, outElectron);
    //Props::d0sig.copy(inElectron, outElectron);
    //Props::z0.copy(inElectron, outElectron);
    //Props::z0sigBL.copy(inElectron, outElectron);
  }  
  
  Props::d0sigBL.copy(inElectron, outElectron);
  Props::z0sinTheta.copy(inElectron, outElectron);

  // Trigger efficiencies are not affected by any variation (opposed to trigger SFs)
  Props::trigEFFlooseLHIsoFixedCutLoose.copy(inElectron, outElectron);
  Props::trigEFFtightLHIsoFixedCutTight.copy(inElectron, outElectron);
  Props::trigEFFtightLHIsoFixedCutLoose.copy(inElectron, outElectron);
  Props::e17trigEFFmediumLHIsoFixedCutLoose.copy(inElectron, outElectron);
  Props::trigEFFmediumLH_offTightIsoFixedCutLoose.copy(inElectron, outElectron);

  // trigger matching
  if(m_doTrigMatch){
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    for(auto& trig : m_triggersToMatch) {
      if(trigDecorators.count(trig.first)) {
        trig.second->copy(inElectron, outElectron);
      }
    }
  }

  //PromptLepton variables
  Props::PromptLeptonIsoEl.copy(inElectron, outElectron);
  Props::PromptLeptonVetoEl.copy(inElectron, outElectron);

  //Write Truth Match property
  Props::TruthMatch.copyIfExists(inElectron, outElectron);

  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ElectronHandler::writeCustomVariables(xAOD::Electron*, xAOD::Electron*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ElectronHandler::clearEvent() 
{
  
  EL_CHECK("ElectronHandler::clearEvent()",ObjectHandler::clearEvent());
  return EL::StatusCode::SUCCESS;

}

//Overload of ObjectHandlerBase link copier
EL::StatusCode ElectronHandler::fillOutputLinks(){

  if(m_debug){
    Info("ElectronHandler::fillOutputLinks", " Copying input container element links to output container (%s)", m_handlerName.c_str());
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
