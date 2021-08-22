// 21.2.5 or later AnalysisBase is recommendated
#include <iostream>
#include "CxAODMaker/MuonHandler.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "MuonMomentumCorrections/MuonCalibrationAndSmearingTool.h"
#include "MuonMomentumCorrections/MuonCalibrationPeriodTool.h"
#include "MuonSelectorTools/MuonSelectionTool.h"
#include "MuonEfficiencyCorrections/MuonEfficiencyScaleFactors.h"
#include "IsolationSelection/IsolationSelectionTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODTruth/TruthParticleContainer.h"

MuonHandler::MuonHandler(const std::string& name, ConfigStore & config, xAOD::TEvent *event,
                         EventInfoHandler & eventInfoHandler) :
  ObjectHandler(name, config, event, eventInfoHandler),
  m_muonCalibrationPeriodTool(nullptr),
  m_muonSelectionToolLoose(nullptr),
  m_muonSelectionToolMedium(nullptr),
  m_muonSelectionToolTight(nullptr),
  m_muonSelectionToolHighPt(nullptr),
  m_muonSelectionToolLowPtEfficiency(nullptr),
  m_muonSelectionToolVeryLoose(nullptr),
  m_muonEfficiencyScaleFactorsTTVA(nullptr),
  m_muonEfficiencyScaleFactorsLoose(nullptr),
  m_muonEfficiencyScaleFactorsMedium(nullptr), 
  m_muonEfficiencyScaleFactorsTight(nullptr), 
  m_muonEfficiencyScaleFactorsHighPt(nullptr), 
  m_muonEfficiencyScaleFactorsLowPt(nullptr), 
  m_muonEfficiencyScaleFactorsFixedCutLooseIso(nullptr),
  m_muonEfficiencyScaleFactorsFixedCutTightIso(nullptr),
  m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso(nullptr),
  m_muonEfficiencyScaleFactorsFixedCutPflowTightIso(nullptr),
  m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso(nullptr),
  m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso(nullptr),
  m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso(nullptr),
  m_muonEfficiencyScaleFactorsFCTight_FixedRadIso(nullptr),
  m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso(nullptr),

  m_isIso(nullptr),
  m_trigDecTool(nullptr),
  m_doTrigMatch(false),
  m_doResolution(false),
  m_doSagittaCorrection(false),
  m_turnOffMuonTRTRequirement(false),
  m_triggersToMatch()
{
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind(&MuonHandler::passLooseMuon, this, _1));
  m_config.getif<bool>("doResolution",m_doResolution);
  m_config.getif<bool>("doSagittaCorrection",m_doSagittaCorrection);
  m_config.getif<bool>("turnOffMuonTRTRequirement",m_turnOffMuonTRTRequirement);
}

MuonHandler::~MuonHandler() {

  //delete tool:
  delete m_muonCalibrationPeriodTool;
  delete m_muonSelectionToolLoose;
  delete m_muonSelectionToolMedium;
  delete m_muonSelectionToolTight;
  delete m_muonSelectionToolHighPt;
  delete m_muonSelectionToolLowPtEfficiency;
  delete m_muonSelectionToolVeryLoose;
  delete m_muonEfficiencyScaleFactorsTTVA;
  delete m_muonEfficiencyScaleFactorsLoose;
  delete m_muonEfficiencyScaleFactorsMedium;
  delete m_muonEfficiencyScaleFactorsTight;
  delete m_muonEfficiencyScaleFactorsHighPt;
  delete m_muonEfficiencyScaleFactorsLowPt;
  delete m_muonEfficiencyScaleFactorsFixedCutLooseIso,
  delete m_muonEfficiencyScaleFactorsFixedCutTightIso,
  delete m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso,
  delete m_muonEfficiencyScaleFactorsFixedCutPflowTightIso,
  delete m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso,
  delete m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso,
  delete m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso,
  delete m_muonEfficiencyScaleFactorsFCTight_FixedRadIso,
  delete m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso,
  delete m_isIso;
}


EL::StatusCode MuonHandler::initializeTools() 
{  


  // initialize the muon selector tool
  //---------------------------------------------------
  m_muonSelectionToolLoose = new CP::MuonSelectionTool("MuonSelectionToolLoose");
  m_muonSelectionToolLoose->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLoose->setProperty("MaxEta", 2.7)); //2.7 should be default, but better to state it explicitly if later we want to change
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLoose->setProperty("MuQuality", (int) xAOD::Muon::Quality::Loose));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLoose->setProperty("TrtCutOff",m_turnOffMuonTRTRequirement));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLoose->initialize());
  m_muonSelectionToolMedium = new CP::MuonSelectionTool("MuonSelectionToolMedium");
  m_muonSelectionToolMedium->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolMedium->setProperty("MaxEta", 2.5));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolMedium->setProperty("MuQuality", (int) xAOD::Muon::Quality::Medium));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolMedium->setProperty("TrtCutOff",m_turnOffMuonTRTRequirement));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolMedium->initialize());
  m_muonSelectionToolTight = new CP::MuonSelectionTool("MuonSelectionToolTight");
  m_muonSelectionToolTight->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolTight->setProperty("MaxEta", 2.5)); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolTight->setProperty("MuQuality", (int) xAOD::Muon::Quality::Tight));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolTight->setProperty("TrtCutOff",m_turnOffMuonTRTRequirement));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolTight->initialize());
  m_muonSelectionToolHighPt = new CP::MuonSelectionTool("MuonSelectionToolHighPt");
  m_muonSelectionToolHighPt->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolHighPt->setProperty("MaxEta", 2.5)); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolHighPt->setProperty("MuQuality", 4));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolHighPt->setProperty("TrtCutOff",m_turnOffMuonTRTRequirement));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolHighPt->initialize());
  m_muonSelectionToolLowPtEfficiency = new CP::MuonSelectionTool("MuonSelectionToolLowPtEfficiency");
  m_muonSelectionToolLowPtEfficiency->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLowPtEfficiency->setProperty("MaxEta", 2.5)); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLowPtEfficiency->setProperty("MuQuality", 5));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLowPtEfficiency->setProperty("TrtCutOff",m_turnOffMuonTRTRequirement));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLowPtEfficiency->setProperty("UseAllAuthors",true)); // Do it with AnalysisBase 21.2.5 or later (Comment out with 21.2.4 or older) 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolLowPtEfficiency->initialize());
  m_muonSelectionToolVeryLoose = new CP::MuonSelectionTool("MuonSelectionToolVeryLoose");
  m_muonSelectionToolVeryLoose->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolVeryLoose->setProperty("MaxEta", 2.5)); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolVeryLoose->setProperty("MuQuality", (int) xAOD::Muon::Quality::VeryLoose));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolVeryLoose->setProperty("TrtCutOff",m_turnOffMuonTRTRequirement));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionToolVeryLoose->initialize());


  // Initialize the muon calibration and smearing tool (Sagitta bias correction)
  //---------------------------------------------------
  m_muonCalibrationPeriodTool = new CP::MuonCalibrationPeriodTool("CP::MuonCalibrationPeriodTool");
  m_muonCalibrationPeriodTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonCalibrationPeriodTool->initialize());

  // initialize the muon efficiency tool
  //---------------------------------------------------
  m_muonEfficiencyScaleFactorsTTVA = new CP::MuonEfficiencyScaleFactors("TTVA");
  m_muonEfficiencyScaleFactorsTTVA->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsTTVA->setProperty("WorkingPoint", "TTVA"));
  if(m_turnOffMuonTRTRequirement) TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsTTVA->setProperty("CalibrationRelease", "190312_Winter_r21")); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsTTVA->initialize());

  m_muonEfficiencyScaleFactorsLoose = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolLoose");
  m_muonEfficiencyScaleFactorsLoose->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsLoose->setProperty("WorkingPoint", "Loose"));
  if(m_turnOffMuonTRTRequirement) TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsLoose->setProperty("CalibrationRelease", "190312_Winter_r21")); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsLoose->initialize());
  m_muonEfficiencyScaleFactorsMedium = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolMedium");
  m_muonEfficiencyScaleFactorsMedium->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsMedium->setProperty("WorkingPoint", "Medium"));
  if(m_turnOffMuonTRTRequirement) TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsMedium->setProperty("CalibrationRelease", "190312_Winter_r21")); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsMedium->initialize());
  m_muonEfficiencyScaleFactorsTight = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolTight");
  m_muonEfficiencyScaleFactorsTight->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsTight->setProperty("WorkingPoint", "Tight"));
  if(m_turnOffMuonTRTRequirement) TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsTight->setProperty("CalibrationRelease", "190312_Winter_r21")); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsTight->initialize());
  m_muonEfficiencyScaleFactorsHighPt = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolHighPt");
  m_muonEfficiencyScaleFactorsHighPt->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsHighPt->setProperty("WorkingPoint", "HighPt"));
  if(m_turnOffMuonTRTRequirement) TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsHighPt->setProperty("CalibrationRelease", "190312_Winter_r21")); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsHighPt->initialize());
  m_muonEfficiencyScaleFactorsLowPt = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolLowPt");
  m_muonEfficiencyScaleFactorsLowPt->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsLowPt->setProperty("WorkingPoint", "LowPt"));
  if(m_turnOffMuonTRTRequirement) TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsLowPt->setProperty("CalibrationRelease", "190312_Winter_r21")); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsLowPt->initialize());


  //Isolation WP under study
  m_isIso = new CP::IsolationSelectionTool("MuonIso");
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->setProperty("MuonWP", "FCLoose"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->initialize());
  //Add all the WP you may want to check in the TAccept output
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FixedCutHighPtTrackOnly"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FCTightTrackOnly"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FCTight"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FixedCutPflowTight"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FixedCutPflowLoose"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FCTight_FixedRad"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FCLoose_FixedRad"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addMuonWP("FCTightTrackOnly_FixedRad"));

   //GP Add "VeryTight" working point - for MultiJet TF tests
  std::vector< std::pair<xAOD::Iso::IsolationType, std::string> > myCuts;
  myCuts.push_back(std::make_pair<xAOD::Iso::IsolationType, std::string>(xAOD::Iso::ptvarcone30, "99")); 
  myCuts.push_back(std::make_pair<xAOD::Iso::IsolationType, std::string>(xAOD::Iso::topoetcone30, "92")); 
  TOOL_CHECK("MuonHandler::initializeTools()",m_isIso->addUserDefinedWP("VeryTight", xAOD::Type::Muon, myCuts));

  //Isolation SF for all WP

  m_muonEfficiencyScaleFactorsFixedCutLooseIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFixedCutLooseIso");
  m_muonEfficiencyScaleFactorsFixedCutLooseIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutLooseIso->setProperty("WorkingPoint", "FixedCutLooseIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutLooseIso->initialize());

  m_muonEfficiencyScaleFactorsFixedCutTightIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFixedCutTightIso");
  m_muonEfficiencyScaleFactorsFixedCutTightIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutTightIso->setProperty("WorkingPoint", "FixedCutTightIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutTightIso->initialize());

  m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFixedCutPflowLooseIso");
  m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso->setProperty("WorkingPoint", "FixedCutPflowLooseIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso->initialize());
  
  m_muonEfficiencyScaleFactorsFixedCutPflowTightIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFixedCutPflowTightIso");
  m_muonEfficiencyScaleFactorsFixedCutPflowTightIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutPflowTightIso->setProperty("WorkingPoint", "FixedCutPflowTightIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutPflowTightIso->initialize());

  m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFixedCutHighPtTrackOnlyIso");
  m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso->setProperty("WorkingPoint", "FixedCutHighPtTrackOnlyIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso->initialize());

  m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFixedCutTightTrackOnlyIso");
  m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso->setProperty("WorkingPoint", "FixedCutTightTrackOnlyIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso->initialize());

  m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFCLoose_FixedRadIso");
  m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso->setProperty("WorkingPoint", "FCLoose_FixedRadIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso->initialize());

  m_muonEfficiencyScaleFactorsFCTight_FixedRadIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFCTight_FixedRadIso");
  m_muonEfficiencyScaleFactorsFCTight_FixedRadIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFCTight_FixedRadIso->setProperty("WorkingPoint", "FCTight_FixedRadIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFCTight_FixedRadIso->initialize());

  m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso = new CP::MuonEfficiencyScaleFactors("MuonEfficiencyToolFCTightTrackOnly_FixedRadIso");
  m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso->setProperty("WorkingPoint", "FCTightTrackOnly_FixedRadIso"));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso->initialize());

  //retrieve TrigDecTool from EventInfo
  //------------------------------------
  m_trigDecTool = m_eventInfoHandler.get_TrigDecTool();

  //initialize muon trigger matching
  //--------------------------------
  if(m_trigDecTool){
 
    ToolHandle<Trig::TrigDecisionTool> m_trigDec(m_trigDecTool);
    TOOL_CHECK("MuonHandler::initializeTools()",m_trigDec.retrieve());
    m_match_Tool.setTypeAndName("Trig::MatchingTool/MuonMatchingTool");
    TOOL_CHECK("ElectronHandler::initializeTools()",m_match_Tool.retrieve());
 }

  std::unordered_map<std::string, PROP<int>*> triggersToMatch {
      {"HLT_mu50_0eta105_msonly", &Props::matchHLT_mu50_0eta105_msonly}, 
      {"HLT_mu60_0eta105_msonly", &Props::matchHLT_mu60_0eta105_msonly}, 
      {"HLT_mu26_imedium", &Props::matchHLT_mu26_imedium},
      {"HLT_mu50", &Props::matchHLT_mu50},
      {"HLT_mu60", &Props::matchHLT_mu60},
      {"HLT_mu80", &Props::matchHLT_mu80},
      {"HLT_mu14", &Props::matchHLT_mu14},
      {"HLT_mu20_iloose_L1MU15", &Props::matchHLT_mu20_iloose_L1MU15},
      {"HLT_mu24_iloose_L1MU15", &Props::matchHLT_mu24_iloose_L1MU15},
      {"HLT_mu24_imedium", &Props::matchHLT_mu24_imedium},
      {"HLT_mu40", &Props::matchHLT_mu40},
      {"HLT_mu28_ivarmedium", &Props::matchHLT_mu28_ivarmedium},
      {"HLT_mu26_ivarmedium", &Props::matchHLT_mu26_ivarmedium},
      {"HLT_mu24_ivarmedium", &Props::matchHLT_mu24_ivarmedium},
      {"HLT_mu24_ivarloose", &Props::matchHLT_mu24_ivarloose},
      {"HLT_mu24_ivarloose_L1MU15", &Props::matchHLT_mu24_ivarloose_L1MU15},
      {"HLT_mu24_iloose", &Props::matchHLT_mu24_iloose},
      {"HLT_mu18_mu8noL1", &Props::matchHLT_mu18_mu8noL1},
      {"HLT_mu22_mu8noL1", &Props::matchHLT_mu22_mu8noL1},
      {"HLT_2mu14", &Props::matchHLT_2mu14},
      {"HLT_e17_lhloose_mu14", &Props::matchHLT_e17_lhloose_mu14},
      {"HLT_e17_lhloose_nod0_mu14", &Props::matchHLT_e17_lhloose_nod0_mu14},
      {"HLT_e26_lhmedium_nod0_mu8noL1", &Props::matchHLT_e26_lhmedium_nod0_mu8noL1},
      {"HLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1", &Props::matchHLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1},
      {"HLT_e7_lhmedium_mu24", &Props::matchHLT_e7_lhmedium_mu24},
      {"HLT_e7_lhmedium_nod0_mu24", &Props::matchHLT_e7_lhmedium_nod0_mu24}
  };
  m_triggersToMatch = triggersToMatch;

  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear();
  m_sysToolList.push_back(m_muonCalibrationPeriodTool);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsTTVA);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsLoose);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsMedium);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsTight);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsHighPt);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsLowPt);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFixedCutLooseIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFixedCutTightIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFixedCutPflowTightIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFCTight_FixedRadIso);
  m_sysToolList.push_back(m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso);

  return EL::StatusCode::SUCCESS;

}

bool MuonHandler::passLooseMuon(xAOD::Muon * muon) {
  bool passSel = true;

  passSel &= (Props::acceptedMuonTool.get(muon));
  passSel &= (muon->pt() > 7000.);

  Props::passPreSel.set(muon, passSel);
  return passSel;
}

EL::StatusCode MuonHandler::decorateOriginParticle(const xAOD::Muon * muon)
{
  const xAOD::TrackParticle *trackPart = muon->primaryTrackParticle();
  if (!trackPart) Warning("MuonHandler::decorateOriginParticle","Failed to get the TrackParticle to retrieve d0/z0...");
  bool isValidCov = trackPart;
  if(!isValidCov || !xAOD::TrackingHelpers::hasValidCov(trackPart) 
     || trackPart->definingParametersCovMatrixVec().size() == 0 ) {
    isValidCov = false; //good particle but with broken cov matrix used to get d0/z0 significance
    Warning("MuonHandler::decorateOriginParticle","Good trackParticle with invalid CovMatrix!");
  }

  if(isValidCov){
    if( trackPart->definingParametersCovMatrixVec().at(0) <= 0. ){
      isValidCov = false; //good particle but with valid cov matrix used to get wrong d0/z0 significance
      Warning("MuonHandler::decorateOriginParticle","Good trackParticle with valid CovMatrix but negetive d0 significance!");
    }
  }

  Props::innerTrackPt.set(muon,!trackPart ? -99999. : trackPart->pt());
  Props::innerTrackEta.set(muon,!trackPart ? -99999. : trackPart->eta());
  Props::innerTrackPhi.set(muon,!trackPart ? -99999. : trackPart->phi());

  //compute some stuff, e.g. d0,z0, decorate muon object, but apply cuts later (in passVHLooseMuon,...)!
  //---------------
  if ( m_debug ) {
    if ( isValidCov ) {
      Info("MuonHandler::decorateOriginParticle",
      "The size of the CovMatrix is: %i", int( trackPart->definingParametersCovMatrixVec().size()) );
      for ( auto vec : trackPart->definingParametersCovMatrixVec() ){
        Info("MuonHandler::decorateOriginParticle",
        "The element of the CovMatrix : %f", vec);
      }
    }
  }
  float pvZ = m_eventInfoHandler.get_ZPV();
  float d0 = !trackPart ? -99999. : trackPart->d0(); 
  if (m_debug) Info("MuonHandler::decorateOriginParticle","start d0sig!");
  float d0sig = !isValidCov ? -99999. : xAOD::TrackingHelpers::d0significance(trackPart);
  if (m_debug) Info("MuonHandler::decorateOriginParticle","start d0sig_BL!");
  float d0sig_BL = !isValidCov ? -99999. : xAOD::TrackingHelpers::d0significance(trackPart,m_eventInfoHandler.get_beamPosUncX(),m_eventInfoHandler.get_beamPosUncY(),m_eventInfoHandler.get_beamPosCorrXY());
  if (m_debug) Info("MuonHandler::decorateOriginParticle","finish d0sig and d0sig_BL!");
  float z0 = !trackPart ? -99999. : (trackPart->z0()+ trackPart->vz() - pvZ);
  float z0sinTheta = !trackPart ? -99999. : z0*TMath::Sin(trackPart->theta());

  // z0 significance
  float z0sig_BL = -99999.;
  if (isValidCov) {
    static SG::AuxElement::Accessor< std::vector<float> > accTrackParCov( "definingParametersCovMatrix" );
    trackPart->definingParametersCovMatrixVec().at(2);
    float sigma_z0 = trackPart->definingParametersCovMatrixVec().at(2);
    if ( sigma_z0 > 0. ) {
      z0sig_BL = z0 / sqrt(sigma_z0);
    }
    else z0sig_BL = -99999.;  
  }

  Props::d0.set(muon, d0);
  Props::d0sig.set(muon, d0sig);
  Props::d0sigBL.set(muon, d0sig_BL);
  Props::z0.set(muon, z0);
  Props::z0sinTheta.set(muon, z0sinTheta);
  Props::z0sigBL.set(muon, z0sig_BL);

  // more muon tracking, but now for the ID track particle only
  const xAOD::TrackParticle *trackPartID = muon->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
  if (!trackPartID && muon->muonType() == xAOD::Muon::Combined)
    Warning("MuonHandler::decorateOriginParticle", "Failed to get the ID TrackParticle for combined muon to retrieve d0/z0...");
  bool isValidCovID = ( xAOD::TrackingHelpers::hasValidCov(trackPartID) && (int)(trackPartID->definingParametersCovMatrixVec().size()) > 0 && trackPart->definingParametersCovMatrixVec().at(0) > 0. );
  if(!isValidCovID && muon->muonType() == xAOD::Muon::Combined)
    Warning("MuonHandler::decorateOriginParticle","Good ID trackParticle with invalid CovMatrix!");
  
  float d0_ID       = !trackPartID  ? -99999. : trackPartID->d0(); 
  float d0sig_ID    = !isValidCovID ? -99999. : xAOD::TrackingHelpers::d0significance(trackPartID);
  float d0sigBL_ID  = !isValidCovID ? -99999. : xAOD::TrackingHelpers::d0significance(trackPartID, 
                                                                                      m_eventInfoHandler.get_beamPosUncX(), 
                                                                                      m_eventInfoHandler.get_beamPosUncY(), 
                                                                                      m_eventInfoHandler.get_beamPosCorrXY());
  float z0_ID         = !trackPartID      ? -99999. : trackPartID->z0() + trackPartID->vz() - pvZ;
  float sigma_z0_ID   = !isValidCovID     ? -99999. : trackPartID->definingParametersCovMatrixVec().at(2);
  float z0sigBL_ID    = (sigma_z0_ID > 0) ? -99999. : z0_ID / sqrt(sigma_z0_ID);
  float z0sinTheta_ID = !trackPartID      ? -99999. : z0_ID * TMath::Sin(trackPartID->theta());

  Props::d0_ID.set(         muon, d0_ID);
  Props::d0sig_ID.set(      muon, d0sig_ID);
  Props::d0sigBL_ID.set(    muon, d0sigBL_ID);
  Props::z0_ID.set(         muon, z0_ID);
  Props::z0sinTheta_ID.set( muon, z0sinTheta_ID);
  Props::z0sigBL_ID.set(    muon, z0sigBL_ID);
  
  // muon trigger matching is independent of pT -> independent of calib
  m_doTrigMatch = false;
  if (m_event->contains<xAOD::MuonContainer>("HLT_xAOD__MuonContainer_MuonEFInfo")) {
    m_doTrigMatch = true;
  }

  if ( m_doTrigMatch ) {
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    std::string trig_name;
    std::vector<const xAOD::IParticle*> myParticles;
    myParticles.clear();
    myParticles.push_back(muon);

    // only call the trigger matching for the triggers that really need it !
    // loop over the triggers we declared as match-able for muons
    for (auto& trig : m_triggersToMatch) {
      // check if it is in the trigDecorators. Maybe we don't want this trigger.
      if(trigDecorators.count(trig.first)) {
        trig_name = trig.first;
        int is_matched = 0;
        if (m_match_Tool->match(myParticles,trig_name,0.1)) is_matched = 1;
        trig.second->set(muon, is_matched);
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MuonHandler::decorate(xAOD::Muon * muon) 
{
  //get decision from muon selection tool
  //---------------------------------
  bool passedIDCuts = m_muonSelectionToolLoose->passedIDCuts(*muon);//ID Cuts should be already included in next cut
  bool accepted = m_muonSelectionToolLoose->accept(*muon);
  xAOD::Muon::Quality muonQuality = m_muonSelectionToolLoose->getQuality(*muon);

  Props::isLoose.set(muon, m_muonSelectionToolLoose->accept(*muon));
  Props::isMedium.set(muon, m_muonSelectionToolMedium->accept(*muon));
  Props::isTight.set(muon, m_muonSelectionToolTight->accept(*muon));
  Props::isHighPt.set(muon, m_muonSelectionToolHighPt->accept(*muon));
  Props::isLowPtEfficiency.set(muon, m_muonSelectionToolLowPtEfficiency->accept(*muon));
  Props::isVeryLoose.set(muon, m_muonSelectionToolVeryLoose->accept(*muon));

  //get SFs from muon SF tool
  //--------------------------
  int isMC = m_eventInfoHandler.get_isMC();
  float TTVAEffSF = 1.;
  float looseEffSF = 1.;
  float mediumEffSF = 1.;
  float tightEffSF = 1.;
  float highPtEffSF = 1.;
  float lowPtEffSF = 1.;
  float looseTrackOnlyIsoSF = 1.;
  float fixedCutLooseIsoSF = 1.;
  float fixedCutTightIsoSF = 1.;
  float fixedCutHighPtTrackOnlyIsoSF = 1.;
  float fixedCutTightTrackOnlyIsoSF = 1.;
  float fixedCutPflowTightIsoSF = 1.;
  float fixedCutPflowLooseIsoSF = 1.;
  float fCLoose_FixedRadIsoSF = 1.;
  float fCTight_FixedRadIsoSF = 1.;
  float fCTightTrackOnly_FixedRadIsoSF = 1.;

  if (isMC) {
    // do not compute SF for muons that are not accepted (not even Loose)
    if(accepted && m_eventInfoHandler.get_RandomRunNumber() != 0) {
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsTTVA->getEfficiencyScaleFactor(*muon, TTVAEffSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsLoose->getEfficiencyScaleFactor(*muon, looseEffSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsMedium->getEfficiencyScaleFactor(*muon, mediumEffSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsTight->getEfficiencyScaleFactor(*muon, tightEffSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsHighPt->getEfficiencyScaleFactor(*muon, highPtEffSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsLowPt->getEfficiencyScaleFactor(*muon, lowPtEffSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFixedCutLooseIso->getEfficiencyScaleFactor(*muon, fixedCutLooseIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFixedCutTightIso->getEfficiencyScaleFactor(*muon, fixedCutTightIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso->getEfficiencyScaleFactor(*muon, fixedCutPflowLooseIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFixedCutPflowTightIso->getEfficiencyScaleFactor(*muon, fixedCutPflowTightIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso->getEfficiencyScaleFactor(*muon, fixedCutHighPtTrackOnlyIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso->getEfficiencyScaleFactor(*muon, fixedCutTightTrackOnlyIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso->getEfficiencyScaleFactor(*muon, fCLoose_FixedRadIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFCTight_FixedRadIso->getEfficiencyScaleFactor(*muon, fCTight_FixedRadIsoSF),m_debug);
      CP_CHECK("MuonHandler::decorate()",m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso->getEfficiencyScaleFactor(*muon, fCTightTrackOnly_FixedRadIsoSF),m_debug);
    }
  }

  float PromptLeptonIsoMuon = (muon)->auxdata< float >("PromptLeptonIso");
  float PromptLeptonVetoMuon = (muon)->auxdata< float >("PromptLeptonVeto");


  Root::TAccept isoAccept =  m_isIso->accept( *muon);

  //decorate muon 
  //---------------
  //Variables from MuonSelector. The tool needs muons with momentum correction applied
  Props::passedIDCuts.set(muon, passedIDCuts);
  Props::acceptedMuonTool.set(muon, accepted);
  Props::muonQuality.set(muon, muonQuality);

  float charge=muon->charge();  // add the charge of muons
  const xAOD::TrackParticle *primTrkPart = muon->primaryTrackParticle();
  float primCharge =  primTrkPart->charge();
  if(primCharge != charge){
    //A bug in Si-StandAlone muon reconstruction was forcing the ID tracklet charge of the container instead that the CB 
    // charge. This fix reverts to the "best known" charge if a mismatch is found
    Warning("MuonHandler::decorate","Muon primary track has charge different from one in container! Using the first...");
    charge = primCharge;
  }
  Props::charge.set(muon, charge);

  if(m_doResolution){
    float res=-1;
    int dettype=-1;
    if(muon->muonType()==0){//Combined
      dettype=3;//CB
    }
    if(muon->muonType()==1){//MuonStandAlone 
      dettype=1;//MS
    }
    if(muon->muonType()>1){//Segment, Calo, Silicon
      dettype=2;//ID
    }
    res=m_muonCalibrationPeriodTool->expectedResolution(dettype,(*muon),false);
    Props::resolution.set(muon, res);
  }

  Props::isFixedCutLooseIso.set(muon, isoAccept.getCutResult("FCLoose"));
  Props::isFixedCutTightIso.set(muon, isoAccept.getCutResult("FCTight"));
  Props::isFixedCutHighPtTrackOnlyIso.set(muon, isoAccept.getCutResult("FixedCutHighPtTrackOnly"));
  Props::isFixedCutTightTrackOnlyIso.set(muon, isoAccept.getCutResult("FCTightTrackOnly"));
  Props::isFixedCutPflowTightIso.set(muon, isoAccept.getCutResult("FixedCutPflowTight"));
  Props::isFixedCutPflowLooseIso.set(muon, isoAccept.getCutResult("FixedCutPflowLoose"));
  Props::isFCLoose_FixedRadIso.set(muon, isoAccept.getCutResult("FCLoose_FixedRad"));
  Props::isFCTight_FixedRadIso.set(muon, isoAccept.getCutResult("FCTight_FixedRad"));
  Props::isFCTightTrackOnly_FixedRadIso.set(muon, isoAccept.getCutResult("FCTightTrackOnly_FixedRad"));

  // set generic effSF = looseEffSF
  Props::effSF.set(muon, looseEffSF);

  Props::TTVAEffSF.set(muon, TTVAEffSF);
  Props::looseEffSF.set(muon, looseEffSF);
  Props::mediumEffSF.set(muon, mediumEffSF);
  Props::tightEffSF.set(muon, tightEffSF);
  Props::highPtEffSF.set(muon, highPtEffSF);
  Props::lowPtEffSF.set(muon, lowPtEffSF);
  Props::looseTrackOnlyIsoSF.set(muon, looseTrackOnlyIsoSF);
  Props::fixedCutLooseIsoSF.set(muon, fixedCutLooseIsoSF);
  Props::fixedCutTightIsoSF.set(muon, fixedCutTightIsoSF);
  Props::fixedCutHighPtTrackOnlyIsoSF.set(muon, fixedCutHighPtTrackOnlyIsoSF);
  Props::fixedCutTightTrackOnlyIsoSF.set(muon, fixedCutTightTrackOnlyIsoSF);
  Props::fixedCutPflowTightIsoSF.set(muon, fixedCutPflowTightIsoSF);
  Props::fixedCutPflowLooseIsoSF.set(muon, fixedCutPflowLooseIsoSF);
  Props::fCLoose_FixedRadIsoSF.set(muon, fCLoose_FixedRadIsoSF);
  Props::fCTight_FixedRadIsoSF.set(muon, fCTight_FixedRadIsoSF);
  Props::fCTightTrackOnly_FixedRadIsoSF.set(muon, fCTightTrackOnly_FixedRadIsoSF);

  Props::forMETRebuild.set(muon, false);//setting as false as default for MET rebuilding

  Props::PromptLeptonIsoMuon.set(muon, PromptLeptonIsoMuon);
  Props::PromptLeptonVetoMuon.set(muon, PromptLeptonVetoMuon);

  return EL::StatusCode::SUCCESS;
  
}

EL::StatusCode MuonHandler::calibrate() {
   // Important to call the parent function!!
   return ObjectHandler<xAOD::MuonContainer>::calibrate();
}

EL::StatusCode MuonHandler::calibrateCopies(xAOD::MuonContainer * particles, const CP::SystematicSet & sysSet) 
{

  // tell tool to apply systematic variation
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonCalibrationPeriodTool->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsTTVA->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsLoose->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsMedium->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsTight->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsHighPt->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsLowPt->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFixedCutLooseIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFixedCutTightIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFixedCutPflowTightIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFCTight_FixedRadIso->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("MuonHandler::calibrateCopies()",m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso->applySystematicVariation(sysSet),m_debug);

  // TODO this behaves badly for Original, since calibrate() is not called
  // TODO call applySystematicVariation() in ObjectHandler via m_sysToolList

  // loop over the particles in the container
  for (xAOD::Muon * muon : *particles) {
    
    //calibration
    //------------
    CP_CHECK("MuonHandler::calibrateCopies()", m_muonCalibrationPeriodTool->applyCorrection(*muon), m_debug);
	
    // decorate muon
    if ( decorate( muon ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
  }

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode MuonHandler::writeOutputVariables(xAOD::Muon * inMuon, xAOD::Muon * outMuon, bool isKinVar, bool isWeightVar, const TString& sysName) 
{
  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    EL_CHECK("MuonHandler::writeOutputVariables",writeAllVariations(inMuon, outMuon, sysName));
  }
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    EL_CHECK("MuonHandler::writeOutputVariables",writeKinematicVariations(inMuon, outMuon, sysName));
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes  
    EL_CHECK("MuonHandler::writeOutputVariables",writeWeightVariations(inMuon, outMuon, sysName));
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited 
    EL_CHECK("MuonHandler::writeOutputVariables",writeAllVariations(inMuon, outMuon, sysName));
    EL_CHECK("MuonHandler::writeOutputVariables",writeKinematicVariations(inMuon, outMuon, sysName));
    EL_CHECK("MuonHandler::writeOutputVariables",writeWeightVariations(inMuon, outMuon, sysName));
    EL_CHECK("MuonHandler::writeOutputVariables",writeNominal(inMuon, outMuon, sysName));
  } else assert(false);

  return writeCustomVariables(inMuon, outMuon, isKinVar, isWeightVar, sysName);
}

EL::StatusCode MuonHandler::writeAllVariations(xAOD::Muon* /*inMuon*/, xAOD::Muon* /*outMuon*/, const TString& /*sysName*/)
{    
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MuonHandler::writeKinematicVariations(xAOD::Muon* inMuon, xAOD::Muon* outMuon, const TString& /*sysName*/)
{
  // 4-vector components affected by variations:
  Props::pt.copy(inMuon, outMuon);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MuonHandler::writeWeightVariations(xAOD::Muon* inMuon, xAOD::Muon* outMuon, const TString& sysName)
{
  
  // Write variables only for specific variation. Has to be written to Nominal as well.
  bool isEffVar  = (sysName == "Nominal" || sysName.BeginsWith("MUON_EFF_"));
  bool isTTVAVar = (sysName == "Nominal" || sysName.BeginsWith("MUON_TTVA_"));
  bool isIsoVar  = (sysName == "Nominal" || sysName.BeginsWith("MUON_ISO_"));
  
   // SF are only weight chages
  if (m_eventInfoHandler.get_isMC() && isEffVar) {
    Props::effSF.copy(inMuon, outMuon);
    Props::looseEffSF.copy(inMuon, outMuon);
    Props::mediumEffSF.copy(inMuon, outMuon);
    Props::tightEffSF.copy(inMuon, outMuon);
    Props::highPtEffSF.copy(inMuon, outMuon);
    Props::lowPtEffSF.copy(inMuon, outMuon);
  }
  
  if (m_eventInfoHandler.get_isMC() && isTTVAVar) {
    Props::TTVAEffSF.copy(inMuon, outMuon);
  }
  
  if (m_eventInfoHandler.get_isMC() && isIsoVar) {
    if(m_saveNonCrucialVariables){
    }
    Props::looseTrackOnlyIsoSF.copy(inMuon, outMuon);
    Props::fixedCutLooseIsoSF.copy(inMuon, outMuon);
    Props::fixedCutTightIsoSF.copy(inMuon, outMuon);
    Props::fixedCutHighPtTrackOnlyIsoSF.copy(inMuon, outMuon);
    Props::fixedCutTightTrackOnlyIsoSF.copy(inMuon, outMuon);
    Props::fixedCutPflowTightIsoSF.copy(inMuon, outMuon);
    Props::fixedCutPflowLooseIsoSF.copy(inMuon, outMuon);
    Props::fCLoose_FixedRadIsoSF.copy(inMuon, outMuon);
    Props::fCTight_FixedRadIsoSF.copy(inMuon, outMuon);
    Props::fCTightTrackOnly_FixedRadIsoSF.copy(inMuon, outMuon);
  }
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MuonHandler::writeNominal(xAOD::Muon* inMuon, xAOD::Muon* outMuon, const TString& /*sysName*/)
{
  // 4-vector components not affected by variations:
  Props::eta.copy(inMuon, outMuon);
  Props::phi.copy(inMuon, outMuon);
  // Note: m not needed for Muon 4-vector

  if(m_doResolution){
    Props::resolution.copy(inMuon, outMuon);
  }

  //set muon type
  xAOD::Muon::MuonType muonType = inMuon->muonType();
  outMuon->setMuonType(muonType);
  // set something without having a pre-defined method
  Props::muonQuality.copy(inMuon, outMuon);

  Props::isLoose.copy(inMuon, outMuon);
  Props::isMedium.copy(inMuon, outMuon);
  Props::isTight.copy(inMuon, outMuon);
  Props::isHighPt.copy(inMuon, outMuon);
  Props::isLowPtEfficiency.copy(inMuon, outMuon);
  Props::isVeryLoose.copy(inMuon, outMuon);

  if(m_saveNonCrucialVariables){
    Props::d0.copy(inMuon, outMuon);
    Props::d0sig.copy(inMuon, outMuon);
    Props::z0.copy(inMuon, outMuon);
    Props::z0sigBL.copy(inMuon, outMuon);

    Props::d0_ID.copy(inMuon, outMuon);
    Props::d0sig_ID.copy(inMuon, outMuon);
    Props::z0_ID.copy(inMuon, outMuon);
    Props::z0sigBL_ID.copy(inMuon, outMuon);
    Props::d0sigBL_ID.copy(inMuon, outMuon);
    Props::z0sinTheta_ID.copy(inMuon, outMuon);
  }
  Props::d0sigBL.copy(inMuon, outMuon);
  Props::z0sinTheta.copy(inMuon, outMuon);

  Props::charge.copy(inMuon, outMuon);
  //
  float trackIso = -999.;
  inMuon->isolation(trackIso, xAOD::Iso::ptvarcone30);
  outMuon->setIsolation(trackIso, xAOD::Iso::ptvarcone30);
  float trackIso1 = -999.;
  inMuon->isolation(trackIso1, xAOD::Iso::ptvarcone20);
  outMuon->setIsolation(trackIso1, xAOD::Iso::ptvarcone20);
  float trackIso2 = -999.;
  inMuon->isolation(trackIso2, xAOD::Iso::ptvarcone40);
  outMuon->setIsolation(trackIso2, xAOD::Iso::ptvarcone40);
  float trackIso4 = -999.;
  inMuon->isolation(trackIso4, xAOD::Iso::ptcone20);
  outMuon->setIsolation(trackIso4, xAOD::Iso::ptcone20);
  float trackIso5 = -999.;
  inMuon->isolation(trackIso5, xAOD::Iso::ptcone30);
  outMuon->setIsolation(trackIso5, xAOD::Iso::ptcone30);
  float trackIso6 = -999.;
  inMuon->isolation(trackIso6, xAOD::Iso::ptcone40);
  outMuon->setIsolation(trackIso6, xAOD::Iso::ptcone40);
  float trackIso7 = -999.;
  inMuon->isolation(trackIso7, xAOD::Iso::ptvarcone30_TightTTVA_pt1000);
  outMuon->setIsolation(trackIso7, xAOD::Iso::ptvarcone30_TightTTVA_pt1000);

  //
  float caloIso = -999.;
  inMuon->isolation(caloIso, xAOD::Iso::topoetcone20);
  outMuon->setIsolation(caloIso, xAOD::Iso::topoetcone20);
  float caloIso1 = -999.;
  inMuon->isolation(caloIso1, xAOD::Iso::topoetcone30);
  outMuon->setIsolation(caloIso1, xAOD::Iso::topoetcone30);
  float caloIso2 = -999.;
  inMuon->isolation(caloIso2, xAOD::Iso::topoetcone40);
  outMuon->setIsolation(caloIso2, xAOD::Iso::topoetcone40);
  
  //new isolation wp's   
  if(m_saveNonCrucialVariables){
  }
  Props::isFixedCutHighPtTrackOnlyIso.copy(inMuon, outMuon);
  Props::isFixedCutTightTrackOnlyIso.copy(inMuon, outMuon);
  Props::isFixedCutLooseIso.copy(inMuon, outMuon);
  Props::isFixedCutTightIso.copy(inMuon, outMuon);
  Props::isFixedCutPflowTightIso.copy(inMuon, outMuon);
  Props::isFixedCutPflowLooseIso.copy(inMuon, outMuon);
  Props::isFCLoose_FixedRadIso.copy(inMuon, outMuon);
  Props::isFCTight_FixedRadIso.copy(inMuon, outMuon);
  Props::isFCTightTrackOnly_FixedRadIso.copy(inMuon, outMuon);
  
  if(m_saveNonCrucialVariables){
    //inner track
    Props::innerTrackPt.copy(inMuon, outMuon);
    Props::innerTrackEta.copy(inMuon, outMuon);
    Props::innerTrackPhi.copy(inMuon, outMuon);
  }
  
  // trigger Props are independent of energy scale (at least for now)
  if ( m_doTrigMatch ) {
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    for(auto& trig : m_triggersToMatch) {
      if(trigDecorators.count(trig.first)) {
        trig.second->copy(inMuon, outMuon);
      }
    }
  }
  if ( m_doTrigMatch ){
//    const xAOD::IParticleContainer* muons = 0;
//    CHECK( evtStore()->retrieve( muons, "Muons") );   
    std::vector<const xAOD::IParticle*> myParticles;
    myParticles.clear();
    myParticles.push_back(inMuon); 
  }

  // PromptLeptonIso & PromptLeptonVeto
  Props::PromptLeptonIsoMuon.copy(inMuon, outMuon);
  Props::PromptLeptonVetoMuon.copy(inMuon, outMuon);

  //Write Truth Match property
  Props::TruthMatch.copyIfExists(inMuon, outMuon);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MuonHandler::writeCustomVariables(xAOD::Muon*, xAOD::Muon*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MuonHandler::clearEvent() 
{
  EL_CHECK("MuonHandler::clearEvent()",ObjectHandler::clearEvent());
  return EL::StatusCode::SUCCESS;

}


//Overload of ObjectHandlerBase link copier
EL::StatusCode MuonHandler::fillOutputLinks(){

  if( m_debug ){
    Info("MuonHandler::fillOutputLinks", " Copying input container element links to output container (%s)", m_handlerName.c_str());
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

