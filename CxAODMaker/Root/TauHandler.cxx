#include <iostream>
//tau includes
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "CxAODMaker/TauHandler.h"
#include "CxAODMaker/TruthProcessor.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "CxAODTools/PUReweightingTool.h"

#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"
#include "TrigTauMatching/TrigTauMatching.h"
//#include "AsgTools/ToolHandle.h"

TauHandler::TauHandler(const std::string& name, ConfigStore & config, xAOD::TEvent *event,
                         EventInfoHandler & eventInfoHandler) :
  ObjectHandler(name, config, event, eventInfoHandler),
  m_truthproc(0),
  m_SearchedTrueTaus(false),
  m_runTruthMatchTool(false),
  m_useTausInMET(false),
  m_trigDecTool(nullptr),
  m_tauIDWP("medium"),
  m_writeRNNVars(false),
  m_useRNNTaus(false),
  m_suppressTauCalibration(false),
  m_doTrigMatch(false),
  m_SkipTruthMatch(false),
  m_triggersToMatch(),
  m_match_Tool(nullptr)
{
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind(&TauHandler::passLooseTau, this, _1));

  m_config.getif<std::string>("TauIDWP", m_tauIDWP);
  m_config.getif<bool>("runTauTruthMatchTool",m_runTruthMatchTool);
  m_config.getif<bool>("useTausInMET",m_useTausInMET);
  m_config.getif<bool>("writeRNNVars",m_writeRNNVars);
  m_config.getif<bool>("useRNNTaus",m_useRNNTaus);
  m_config.getif<bool>("suppressTauCalibration",m_suppressTauCalibration);
  m_config.getif<bool>("SkipTruthMatch",m_SkipTruthMatch);
  m_config.getif<bool>("AntiTau",m_antitau);
}

TauHandler::~TauHandler() {
}


EL::StatusCode TauHandler::initializeTools()
{
  // ===================
  // Setup Tau-ID levels
  // ===================

  // ID threshold for signal taus
  if (m_tauIDWP == "tight") {
    if (m_useRNNTaus) {
      m_TAT_IDLevel = TauAnalysisTools::JETIDRNNTIGHT;
      m_xAOD_IDLevel = xAOD::TauJetParameters::IsTauFlag::JetRNNSigTight;
      Info("TauHandler::initializeTools()", "Setting up TauHandler with RNN tight Tau-ID");
    } else {
      m_TAT_IDLevel = TauAnalysisTools::JETIDBDTTIGHT;
      m_xAOD_IDLevel = xAOD::TauJetParameters::IsTauFlag::JetBDTSigTight;
      Info("TauHandler::initializeTools()", "Setting up TauHandler with BDT tight Tau-ID");
    }
  } else if (m_tauIDWP == "medium") {
    if (m_useRNNTaus) {
      m_TAT_IDLevel = TauAnalysisTools::JETIDRNNMEDIUM;
      m_xAOD_IDLevel = xAOD::TauJetParameters::IsTauFlag::JetRNNSigMedium;
      Info("TauHandler::initializeTools()", "Setting up TauHandler with RNN medium Tau-ID");
    } else {
      m_TAT_IDLevel = TauAnalysisTools::JETIDBDTMEDIUM;
      m_xAOD_IDLevel = xAOD::TauJetParameters::IsTauFlag::JetBDTSigMedium;
      Info("TauHandler::initializeTools()", "Setting up TauHandler with BDT medium Tau-ID");
    }
  } else if (m_tauIDWP == "loose") {
    if (m_useRNNTaus) {
      m_TAT_IDLevel = TauAnalysisTools::JETIDRNNLOOSE;
      m_xAOD_IDLevel = xAOD::TauJetParameters::IsTauFlag::JetRNNSigLoose;
      Info("TauHandler::initializeTools()", "Setting up TauHandler with RNN loose Tau-ID");
    } else {
      m_TAT_IDLevel = TauAnalysisTools::JETIDBDTLOOSE;
      m_xAOD_IDLevel = xAOD::TauJetParameters::IsTauFlag::JetBDTSigLoose;
      Info("TauHandler::initializeTools()", "Setting up TauHandler with BDT loose Tau-ID");
    }
  } else {
    Error("TauHandler::initializeTools()", "TauIDWP property is only supporting 'tight', 'medium' and 'loose'");
    return EL::StatusCode::FAILURE;
  }

  // Lower ID threshold for anti-taus
  if (m_useRNNTaus) {
    m_TAT_IDLevel_AntiTau = TauAnalysisTools::JETIDRNNVERYLOOSE;
    m_xAOD_IDLevel_AntiTau = xAOD::TauJetParameters::IsTauFlag::JetRNNSigVeryLoose;
  } else {
    m_TAT_IDLevel_AntiTau = TauAnalysisTools::JETIDBDTVERYLOOSE;
    m_xAOD_IDLevel_AntiTau = xAOD::TauJetParameters::IsTauFlag::JetBDTSigVeryLoose;
  }

  // Init Calibrationtool
  // _____________________________________________
  m_tauSmearingTool.reset(new TauAnalysisTools ::TauSmearingTool("TauSmearTool"));
  m_tauSmearingTool->msg().setLevel( m_msgLevel );
  if(m_SkipTruthMatch) TOOL_CHECK("TauHandler::initializeTools()",m_tauSmearingTool->setProperty("SkipTruthMatchCheck",true));
  TOOL_CHECK("TauHandler::initializeTools()",m_tauSmearingTool->setProperty("RecommendationTag","2018-summer"));
  TOOL_CHECK("TauHandler::initializeTools()",m_tauSmearingTool->initialize());

  // Init TauSelector
  // _____________________________________________

  // Raise an error if we set fake since this shouldn't be used anymore
  bool fake = false;
  m_config.getif<bool>("Fake", fake);
  if (fake) {
    Error("TauHandler::initializeTools()", "The 'fake' flag is deprecated and should always be false.");
    return EL::StatusCode::FAILURE;
  }

  // TauSelectionTool configuration
  // Following tau selection recommended by TauCP
  const double ptMin = 20.;
  const std::vector<double> absEtaRegion = {0, 1.37, 1.52, 2.5};
  const int absCharge = 1;
  const std::vector<size_t> nTracks = {1, 3};
  const bool eleOLR = false;
  const int eleBDTWP = TauAnalysisTools::ELEIDBDTOLDLOOSE;

  int jetIDWP = m_TAT_IDLevel;
  if (m_antitau) {
    jetIDWP = m_TAT_IDLevel_AntiTau;
  }

  const int selectionCuts = TauAnalysisTools::CutPt
    | TauAnalysisTools::CutAbsEta
    | TauAnalysisTools::CutAbsCharge
    | TauAnalysisTools::CutNTrack
    | TauAnalysisTools::CutEleOLR
    | TauAnalysisTools::CutEleBDTWP
    | TauAnalysisTools::CutJetIDWP;

  m_tauSelectionTool.reset(new TauAnalysisTools::TauSelectionTool("TauSelectTool"));
  m_tauSelectionTool->msg().setLevel( m_msgLevel );

  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("PtMin", ptMin));
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("AbsEtaRegion", absEtaRegion));
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("AbsCharge", absCharge));
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("NTracks", nTracks));
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("EleOLR", eleOLR));
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("EleBDTWP", eleBDTWP));
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("JetIDWP", jetIDWP));
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("SelectionCuts", selectionCuts));

  // Disable default config file
  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->setProperty("ConfigPath", ""));

  TOOL_CHECK("TauHandler::initializeTools()",
             m_tauSelectionTool->initialize());

  // Init ID Correction
  // _____________________________________________
  ToolHandle<TauAnalysisTools::ITauSelectionTool> TauSelToolHandle = m_tauSelectionTool.get();
  PUReweightingTool* puTool = m_eventInfoHandler.get_PUReweightingTool();
  ToolHandle<CP::IPileupReweightingTool> m_tPRWToolHandle = puTool->getPileupReweightingTool().getHandle();

  m_tauEffCorr.reset(new TauAnalysisTools::TauEfficiencyCorrectionsTool("TauEfficiencyTool"));
  m_tauEffCorr->msg().setLevel( m_msgLevel );
  TOOL_CHECK("TauHandler::initializeTools()",m_tauEffCorr->setProperty("IDLevel", m_TAT_IDLevel));
  TOOL_CHECK("TauHandler::initializeTools()",m_tauEffCorr->setProperty("OLRLevel", (int)TauAnalysisTools::ELEBDTLOOSE));
  TOOL_CHECK("TauHandler::initializeTools()",m_tauEffCorr->setProperty("RecommendationTag","2018-summer"));
  //This overwrites the IDLevel
  //TOOL_CHECK("TauHandler::initializeTools()",m_tauEffCorrLoose->setProperty("TauSelectionTool", TauSelToolHandle));
  TOOL_CHECK("TauHandler::initializeTools()",m_tauEffCorr->setProperty("PileupReweightingTool", m_tPRWToolHandle ));
  TOOL_CHECK("TauHandler::initializeTools()",m_tauEffCorr->initialize());

  // ============================================
  // Efficiency correction tools for tau triggers
  // ============================================
  const std::map<std::string, std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool>*> trigEffToolMap = {
    {"HLT_tau25_medium1_tracktwo", &m_tauTriggerEffTool25},
    {"HLT_tau35_medium1_tracktwo", &m_tauTriggerEffTool35},
    {"HLT_tau80_medium1_tracktwo", &m_tauTriggerEffTool80},
    {"HLT_tau125_medium1_tracktwo", &m_tauTriggerEffTool125},
    {"HLT_tau160_medium1_tracktwo", &m_tauTriggerEffTool160}
  };

  for (auto trig_tool : trigEffToolMap) {
    auto trig = trig_tool.first;
    auto &tool = *(trig_tool.second);

    tool.reset(new TauAnalysisTools::TauEfficiencyCorrectionsTool("TauTriggerEffCorrection_" + trig));
    tool->msg().setLevel( m_msgLevel );
    TOOL_CHECK("TauHandler::initializeTools()",tool->setProperty("EfficiencyCorrectionTypes", std::vector<int>({TauAnalysisTools::SFTriggerHadTau})));
    TOOL_CHECK("TauHandler::initializeTools()",tool->setProperty("TriggerName", trig));
    TOOL_CHECK("TauHandler::initializeTools()",tool->setProperty("IDLevel", m_TAT_IDLevel));
    TOOL_CHECK("TauHandler::initializeTools()",tool->setProperty("PileupReweightingTool", m_tPRWToolHandle));
    TOOL_CHECK("TauHandler::initializeTools()",tool->initialize());
  }

  // =======================
  // Tau truth-matching tool
  // =======================
  if (m_runTruthMatchTool && m_eventInfoHandler.get_isMC()) {
    m_tauTruthMatchingTool.reset(new TauAnalysisTools::TauTruthMatchingTool("T2MT"));
    m_tauTruthMatchingTool->msg().setLevel( m_msgLevel );
    TOOL_CHECK("TauHandler::initializeTools()",m_tauTruthMatchingTool->setProperty("MaxDeltaR",0.2)); // TODO make this configurable
    TOOL_CHECK("TauHandler::initializeTools()",m_tauTruthMatchingTool->setProperty("WriteTruthTaus",true));
    TOOL_CHECK("TauHandler::initializeTools()",m_tauTruthMatchingTool->initialize());
  }

  //retrieve TrigDecTool from EventInfo
  //------------------------------------
  m_trigDecTool = m_eventInfoHandler.get_TrigDecTool();

  //initialize tau trigger matching
  //--------------------------------
  if(m_trigDecTool){
    ToolHandle<Trig::TrigDecisionTool> m_trigDec(m_trigDecTool);

    m_match_Tool = new Trig::TrigTauMatchingTool("TauMatch");
    m_match_Tool->msg().setLevel( m_msgLevel );
    TOOL_CHECK("TauHandler::initializeTools()",m_match_Tool->setProperty("TrigDecisionTool",m_trigDec));
    TOOL_CHECK("TauHandler::initializeTools()",m_match_Tool->initialize());
  }

  // Chris: For bbtautau this is overwritten in the dedicated TauHandler of the
  // analysis. Does anyone else need this?
  std::unordered_map<std::string, PROP<int>*> triggersToMatch {
    {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20ITAU12I_J25", &Props::matchHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20ITAU12I_J25 },
    {"HLT_tau80_medium1_tracktwo", &Props::matchHLT_tau80_medium1_tracktwo },
    {"HLT_e17_lhmedium_tau25_medium1_tracktwo", &Props::matchHLT_e17_lhmedium_tau25_medium1_tracktwo },
    {"HLT_e17_lhmedium_iloose_tau25_medium1_tracktwo", &Props::matchHLT_e17_lhmedium_iloose_tau25_medium1_tracktwo },
    {"HLT_e17_medium_iloose_tau25_medium1_tracktwo", &Props::matchHLT_e17_medium_iloose_tau25_medium1_tracktwo },
    {"HLT_e17_medium_tau80_medium1_tracktwo", &Props::matchHLT_e17_medium_tau80_medium1_tracktwo },
    {"HLT_mu14_tau25_medium1_tracktwo", &Props::matchHLT_mu14_tau25_medium1_tracktwo },
    {"HLT_mu14_tau35_medium1_tracktwo_L1TAU20", &Props::matchHLT_mu14_tau35_medium1_tracktwo_L1TAU20 },
    {"HLT_mu14_tau35_medium1_tracktwo", &Props::matchHLT_mu14_tau35_medium1_tracktwo }
  };
  m_triggersToMatch = triggersToMatch;


  // register ISystematicsTools
  // _____________________________________________
  m_sysToolList.clear();
  m_sysToolList.push_back(m_tauEffCorr.get());
  m_sysToolList.push_back(m_tauTriggerEffTool25.get());
  m_sysToolList.push_back(m_tauTriggerEffTool35.get());
  m_sysToolList.push_back(m_tauTriggerEffTool80.get());
  m_sysToolList.push_back(m_tauTriggerEffTool125.get());
  m_sysToolList.push_back(m_tauTriggerEffTool160.get());
  m_sysToolList.push_back(m_tauSmearingTool.get());

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode TauHandler::writeOutputVariables(xAOD::TauJet * inTau, xAOD::TauJet * outTau, bool isKinVar, bool isWeightVar, const TString& sysName) 
{

  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    EL_CHECK("TauHandler::writeOutputVariables",writeAllVariations(inTau, outTau, sysName));
  }
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    EL_CHECK("TauHandler::writeOutputVariables",writeKinematicVariations(inTau, outTau, sysName));
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes  
    EL_CHECK("TauHandler::writeOutputVariables",writeWeightVariations(inTau, outTau, sysName));
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited 
    EL_CHECK("TauHandler::writeOutputVariables",writeAllVariations(inTau, outTau, sysName));
    EL_CHECK("TauHandler::writeOutputVariables",writeKinematicVariations(inTau, outTau, sysName));
    EL_CHECK("TauHandler::writeOutputVariables",writeWeightVariations(inTau, outTau, sysName));
    EL_CHECK("TauHandler::writeOutputVariables",writeNominal(inTau, outTau, sysName));
  } else assert(false);
  
  return writeCustomVariables(inTau, outTau, isKinVar, isWeightVar, sysName);
  
}

EL::StatusCode TauHandler::writeAllVariations(xAOD::TauJet*, xAOD::TauJet*, const TString&)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::writeKinematicVariations(xAOD::TauJet* inTau, xAOD::TauJet* outTau, const TString& /*sysName*/)
{
  // set four momentum
  setP4( inTau , outTau );

  //Props::BDTCut.copy(inTau, outTau);  // For testing
  Props::isAntiTau.copy(inTau, outTau);
  Props::passTauSelector.copy(inTau, outTau);
  Props::TruthMatch.copy(inTau, outTau);
  Props::LeptonTruthMatch.copy(inTau, outTau);
  if(m_runTruthMatchTool) {
    Props::TATTruthMatch.copy(inTau, outTau);
  }
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::writeWeightVariations(xAOD::TauJet* inTau, xAOD::TauJet* outTau, const TString& /*sysName*/)
{
  if (m_eventInfoHandler.get_isMC()) {
    //Props::effSFeveto.copy(inTau, outTau);
    Props::effSF.copy(inTau, outTau);
  }
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::writeNominal(xAOD::TauJet* inTau, xAOD::TauJet* outTau, const TString& /*sysName*/)
{
  //    
  outTau->setCharge(inTau->charge());
  //
  Props::nTracks.copy(inTau, outTau);
  Props::tauTJVAIndex.copy(inTau, outTau);
  Props::isBDTVeryLoose.copy(inTau, outTau);
  Props::isBDTLoose.copy(inTau, outTau);
  Props::isBDTMedium.copy(inTau, outTau);
  Props::isBDTTight.copy(inTau, outTau);
  // Props::passEvetoLoose.copy(inTau, outTau);
  // Props::passEvetoMedium.copy(inTau, outTau);
  // Props::passEvetoTight.copy(inTau, outTau);
  Props::charge.copy(inTau, outTau); // TODO: remove this? It's redundant as charge is already a member of the class
  Props::BDTScoreDeprecated.copy(inTau, outTau);
  Props::BDTScore.copy(inTau, outTau);

  Props::isRNNVeryLoose.copyIfExists(inTau, outTau);
  Props::isRNNLoose.copyIfExists(inTau, outTau);
  Props::isRNNMedium.copyIfExists(inTau, outTau);
  Props::isRNNTight.copyIfExists(inTau, outTau);
  Props::RNNScoreDeprecated.copyIfExists(inTau, outTau);
  Props::RNNScore.copyIfExists(inTau, outTau);

  //
  Props::EleBDTScore.copy(inTau, outTau);
  Props::EleORBDTscore.copy(inTau, outTau);
  Props::EleORLH.copy(inTau, outTau);
  Props::TrackEta.copy(inTau, outTau);
  //
  // trigger Props are independent of energy scale (at least for now)
  if ( m_doTrigMatch ) {
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    for(auto& trig : m_triggersToMatch) {
      if(trigDecorators.count(trig.first)) {
        trig.second->copy(inTau, outTau);
      }
    }
  }

  //Write Truth Match property
  Props::TruthMatch.copyIfExists(inTau, outTau);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::writeCustomVariables(xAOD::TauJet*, xAOD::TauJet*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::decorateOriginParticle(const xAOD::TauJet * tau) 
{
  // This method should only be applied to things that don't depend on the calibration

  //bool medium = tau->isTau(xAOD::TauJetParameters::JetBDTSigLoose);

  m_doTrigMatch = true;                         
  if (m_event->contains<xAOD::TauJetContainer>("HLT_xAOD__TauJetContainer_TrigTauRecMerged")) {
    m_doTrigMatch = true;
  }

  // tau trigger matching is independent of pT -> independent of calib
  if ( m_doTrigMatch ) {
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    std::string trig_name;

    // only call the trigger matching for the triggers that really need it !
    // loop over the triggers we declared as match-able for taus
    for (auto& trig : m_triggersToMatch) {
      // check if it is in the trigDecorators. Maybe we don't want this trigger.

      if(trigDecorators.count(trig.first)) {
        trig_name = trig.first;

        int is_matched = 0;
        // Do not do trigger matching if the tau is not accepted -> now need to for antitaus
        //if(medium) {
	if (m_match_Tool->match(tau,trig_name)) is_matched = 1;
        //}
        trig.second->set(tau, is_matched);
      }
    }
  }

  // needed in case decorations are not in the derivation
  if (m_runTruthMatchTool && m_eventInfoHandler.get_isMC()) {
    m_tauTruthMatchingTool->applyTruthMatch(*tau);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::addCPVariations(const std::vector<TString> &variations, 
					   const bool filterByTools, const bool /*skipWeightVar*/) {

  // add the usual variations, filtered by registered tools if requested
  EL_CHECK("TauHandler::addCPVariations()", ObjectHandlerBase::addCPVariations(variations, filterByTools));
  // see if we want some additional ones
  std::vector<TString> addVariations;  for (const TString &variation : variations) {
    bool allow = false;
    allow |= variation.EqualTo("ANTITAU_BDT_CUT");
    if (!allow) continue;
    addVariations.push_back(variation);
  }  

  // add them to the list w/o further filtering
  return ObjectHandlerBase::addCPVariations(addVariations, false);
}

EL::StatusCode TauHandler::decorate(xAOD::TauJet * tau) 
{

  /* Set tau variables - derived ones*/

  Props::nTracks.set(tau, tau->nTracks());

  float leadtrketa=-100;
  if (tau->nTracks() > 0){
    leadtrketa = tau->track(0)->eta();
  }
  Props::TrackEta.set(tau, leadtrketa );

  // BDT-based ID
  const bool isBDTVeryLoose = tau->isTau(xAOD::TauJetParameters::JetBDTSigVeryLoose);
  const bool isBDTLoose = tau->isTau(xAOD::TauJetParameters::JetBDTSigLoose);
  const bool isBDTMedium = tau->isTau(xAOD::TauJetParameters::JetBDTSigMedium);
  const bool isBDTTight = tau->isTau(xAOD::TauJetParameters::JetBDTSigTight);

  const float bdtscore_deprecated = tau->discriminant(xAOD::TauJetParameters::BDTJetScore);
  const float bdtscore = tau->discriminant(xAOD::TauJetParameters::BDTJetScoreSigTrans);

  Props::isBDTVeryLoose.set(tau, isBDTVeryLoose);
  Props::isBDTLoose.set(tau, isBDTLoose);
  Props::isBDTMedium.set(tau, isBDTMedium);
  Props::isBDTTight.set(tau, isBDTTight);

  Props::BDTScoreDeprecated.set(tau, bdtscore_deprecated);
  Props::BDTScore.set(tau, bdtscore);

  // RNN-based ID (only available in newer derivations)
  static SG::AuxElement::Accessor<float> rnn_acc("RNNJetScore");
  if ((m_writeRNNVars || m_useRNNTaus) && rnn_acc.isAvailable(*tau)) {
    const bool isRNNVeryLoose = tau->isTau(xAOD::TauJetParameters::JetRNNSigVeryLoose);
    const bool isRNNLoose = tau->isTau(xAOD::TauJetParameters::JetRNNSigLoose);
    const bool isRNNMedium = tau->isTau(xAOD::TauJetParameters::JetRNNSigMedium);
    const bool isRNNTight = tau->isTau(xAOD::TauJetParameters::JetRNNSigTight);

    float rnnscore_deprecated = tau->discriminant(xAOD::TauJetParameters::RNNJetScore);
    float rnnscore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);

    Props::isRNNVeryLoose.set(tau, isRNNVeryLoose);
    Props::isRNNLoose.set(tau, isRNNLoose);
    Props::isRNNMedium.set(tau, isRNNMedium);
    Props::isRNNTight.set(tau, isRNNTight);

    Props::RNNScoreDeprecated.set(tau, rnnscore_deprecated);
    Props::RNNScore.set(tau, rnnscore);
  }

  // bool passevetoloose=! tau->isTau(xAOD::TauJetParameters::EleBDTLoose);
  // Props::passEvetoLoose.set(tau, passevetoloose);
  // bool passevetomedium=! tau->isTau(xAOD::TauJetParameters::EleBDTMedium);
  // Props::passEvetoMedium.set(tau, passevetomedium);
  // bool passevetotight=! tau->isTau(xAOD::TauJetParameters::EleBDTTight);
  // Props::passEvetoTight.set(tau, passevetotight);

  float bdtelescore= tau->discriminant(xAOD::TauJetParameters::BDTEleScore);
  Props::EleBDTScore.set(tau, bdtelescore);

  float eleORBDTscore= tau->auxdata<float>("BDTEleScoreSigTrans");
  Props::EleORBDTscore.set(tau, eleORBDTscore);

  float eleORLH= tau->auxdata<float>("EleMatchLikelihoodScore");
  Props::EleORLH.set(tau,eleORLH);

  bool accept=m_tauSelectionTool->accept( *tau );
  Props::passTauSelector.set(tau, accept);

  int tauVertIndex = tau->vertex()->index();
  Props::tauTJVAIndex.set(tau, tauVertIndex);

  float charge=tau->charge();
  Props::charge.set(tau, charge); // TODO: Remove this? It's redundant...
  // bool presel=accept;
  // Props::passPreSel.set(tau,presel);

  if( !Props::effSF.exists(tau) ){
    Props::effSF.set(tau, 1);
    // Props::effSFeveto.set(tau, 1);
   }

  bool isAntiTau = false;
  if (tau->isTau(m_xAOD_IDLevel_AntiTau) && !tau->isTau(m_xAOD_IDLevel)) {
    isAntiTau = true;
  }
  Props::isAntiTau.set(tau, isAntiTau);

  EL_CHECK("TauHandler::decorate",TruthMatch(tau));
  if(m_runTruthMatchTool) {
    EL_CHECK("TauHandler::decorate",TruthMatchTAT(tau));
  }

  return EL::StatusCode::SUCCESS;
  
}


EL::StatusCode TauHandler::FindTrueTaus(){
  if(! m_truthproc ) {
    Error("TauHandler::TruthMatch()", "Cannot find truthparticle handler");
    return EL::StatusCode::FAILURE;
  }
  
  // thats how it usually done, working on the Nominal InParticleVariation
  // Build a vector of true taus/leptons
  if (m_truthproc->isNewStyle()) {
    const xAOD::TruthParticleContainer* truthTaus =  m_truthproc -> getTaus();
    const xAOD::TruthParticleContainer* truthEle =  m_truthproc -> getElectrons();
    const xAOD::TruthParticleContainer* truthMu =  m_truthproc -> getMuons();
    const xAOD::TruthParticleContainer* truthHS =  m_truthproc -> getInParticle("HardScatterParticles");
  
    if (!truthTaus || !truthEle || !truthMu || !truthHS) {
      Error("TauHandler::TruthMatch()", "Cannot find nominal outParticle container for mctruth");
      return EL::StatusCode::FAILURE;
    }

    for (const xAOD::TruthParticle* part : *truthTaus) {
      if (!part) continue;
      m_truetaus.push_back(part);
      if (part->auxdata<char>("IsHadronicTau")) {
        m_truetaus_had.push_back(part);
      } else {
        m_truetaus_lep.push_back(part);
      }
    }
    for (const xAOD::TruthParticle* part : *truthEle) {
      if (!part) continue;
      m_trueeles.push_back(part);
    }
    for (const xAOD::TruthParticle* part : *truthMu) {
      if (!part) continue;
      m_truemuons.push_back(part);
    }
    // now collect quarks
    for (const xAOD::TruthParticle* part : *truthHS) {
      if (!part) continue;
      int pdgId = (int) fabs(part->pdgId()); // get it only once for the if below for speed
      // also register partons above 4 gev (pt cut to speed up processing time..)
      if ((part->pt() > 10e3) && (std::fabs(part->eta()) < 3.5) && ((pdgId > 0 && pdgId < 5) || pdgId == 21)) {
        //std::cout<<"pdg:"<< part->pdgId()<<" pt=" << part->pt() << " eta="<< part->eta() <<" status:"<<part->status()<<"children:"<<part->nChildren()<<std::endl;
        //if ( part->status() == 2 ||  part->status() == 11 ||  part->status() == 3) 
        m_truepartons.push_back(part);
      }
    }
  } else {
    const xAOD::TruthParticleContainer* truthPart =  m_truthproc -> getInParticle("TruthParticles");
    if (!truthPart) {
      Error("TauHandler::TruthMatch()", "Cannot find nominal outParticle container for mctruth");
      return EL::StatusCode::FAILURE;
    }
    for (const xAOD::TruthParticle* part : *truthPart) {
      if (!part) continue;

      int pdgId = (int) fabs(part->pdgId()); // get it only once for the if below for speed
      // also register partons above 4 gev (pt cut to speed up processing time..)
      if ((part->pt() > 10e3) && (std::fabs(part->eta()) < 3.5) && ((pdgId > 0 && pdgId < 5) || pdgId == 21)) {
        //std::cout<<"pdg:"<< part->pdgId()<<" pt=" << part->pt() << " eta="<< part->eta() <<" status:"<<part->status()<<"children:"<<part->nChildren()<<std::endl;
        //if ( part->status() == 2 ||  part->status() == 11 ||  part->status() == 3) 
        m_truepartons.push_back(part);
      }
    
      // Leptons only
      if (!(pdgId == 15 || pdgId == 11 || pdgId == 13)) continue;
        
      bool isEW = m_truthproc->isEWLepton(part);
      if (isEW) {
        if (pdgId == 11) m_trueeles.push_back(part);
        if (pdgId == 13) m_truemuons.push_back(part);
        if (pdgId == 15) m_truetaus.push_back(part);
      }
    }

    // Classify tau decays
    for (const xAOD::TruthParticle* part : m_truetaus) {
      if (!part) continue;
      bool ishad = true;  
      for (unsigned int iChild=0; iChild< part->nChildren(); ++iChild) {
        const xAOD::TruthParticle* child = (const xAOD::TruthParticle*) part->child(iChild);       
        if (!child) continue;
        int pdgId = (int) fabs(part->pdgId()); // get it only once for the if below for speed
        //if (child->status() == 3 ) continue;
        if (pdgId == 11 || pdgId == 13) {
  	  m_truetaus_lep.push_back(part);
	  ishad = false;
	  break;
        }
      }

      if(ishad) {
        m_truetaus_had.push_back(part);
      }
    }
  
  }

  //std::cout << "TAU CAT " << m_truetaus_had.size() << " " << m_truetaus_lep.size() << std::endl;
  m_SearchedTrueTaus = true;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::TruthMatch(xAOD::TauJet * tau){
  
  Props::TruthMatch.set(tau, 0);
  Props::LeptonTruthMatch.set(tau, 0);
  bool hasmatch=false;
  
  int isMC = m_eventInfoHandler.get_isMC();
  if (!isMC) {
    return EL::StatusCode::SUCCESS;
  }

  // only perform time consuming truth matching for tau candidates passing some basic criteria: 
  if ((tau->pt() < 15e3) || (std::fabs(tau->eta()) > 2.5) || fabs(tau->charge()) != 1)
    return EL::StatusCode::SUCCESS;

  if(!m_SearchedTrueTaus){ 
    if(FindTrueTaus() != EL::StatusCode::SUCCESS){
      Error("in TauHandler::TruthMatch()","Failed to FindTrueTaus. Exiting.");
      return EL::StatusCode::FAILURE; 
    }
  }

  // std::cout<<"number of true had taus:"<<m_truetaus_had.size()<<std::endl;
  // std::cout<<"number of true lep taus:"<<m_truetaus_lep.size()<<std::endl;
  //  std::cout<<"number of partons:"<<m_truepartons.size()<<std::endl;

  // Check hadronic taus: 
  float mindr = 0.2;
  for (unsigned int iPart = 0; iPart < m_truetaus_had.size(); ++iPart) {
    if ( (m_truetaus_had[iPart]->p4()).DeltaR(tau->p4()) < mindr) {
      Props::TruthMatch.set(tau, 1);
      hasmatch = true;
    }
  }// thads

  for (unsigned int iPart = 0; iPart < m_truetaus_lep.size(); ++iPart) {
    if ( (m_truetaus_lep[iPart]->p4()).DeltaR(tau->p4()) < mindr) {
      Props::TruthMatch.set(tau, 2);
      hasmatch = true;
    }
  }// tleps

  for (unsigned int iPart = 0; iPart < m_trueeles.size(); ++iPart) {
    if ( (m_trueeles[iPart]->p4()).DeltaR(tau->p4()) < mindr) {
      Props::LeptonTruthMatch.set(tau, 1); // matched to true electron
      hasmatch=true;
    }
  }// true eles

  for (unsigned int iPart = 0; iPart < m_truemuons.size(); ++iPart) {
    if ( (m_truemuons[iPart]->p4()).DeltaR(tau->p4()) < mindr) {
      Props::LeptonTruthMatch.set(tau, 2); // matched to true muon
      hasmatch=true;
    }
  }// true muons
  

  // look whether it has a parton match
  if (!hasmatch) {
    float highestpt=1.; // at least 1 mev please
    int partonindex=-1;
    for(unsigned int iPart = 0; iPart < m_truepartons.size(); ++iPart){

      if ( m_truepartons[iPart]->pt() >highestpt && (m_truepartons[iPart]->p4()).DeltaR(tau->p4())< 0.4 ){
	partonindex = iPart;
	highestpt = m_truepartons[iPart]->pt();
      }
    }
    if (partonindex>=0){
      int partonpdgid=fabs(m_truepartons[partonindex]->pdgId());
      Props::TruthMatch.set(tau, -(partonpdgid));
      //      std::cout<<"Matched tau to parton, tau pt: "<<tau->pt()/1000. <<" pdgID: "<< partonpdgid<<" pt:" << highestpt/1000. <<std::endl;
    }
    // else{
    //   std::cout<<"No match found for tau pt "<<tau->pt()<<std::endl;
    // }
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::TruthMatchTAT(xAOD::TauJet * tau){

  int isMC = m_eventInfoHandler.get_isMC();
  if (!isMC) {
    Props::TATTruthMatch.set(tau, 0); 
    return EL::StatusCode::SUCCESS;
  }


  // Fix jet link which is missing in some derivations
  if(!tau->isAvailable<ElementLink<xAOD::JetContainer>>("truthJetLink") || 
     !tau->auxdecor<ElementLink<xAOD::JetContainer>>("truthJetLink").isValid()) {
    const xAOD::JetContainer* jetContainer;
    EL_CHECK("TauHandler::TruthMatchTAT()", m_event->retrieve(jetContainer , "AntiKt4TruthJets") );
    double dPtMax = 0;
    const xAOD::Jet* xTruthJetMatch = nullptr;
    for (auto xTruthJetIt : *jetContainer) {
      if (tau->p4().DeltaR(xTruthJetIt->p4()) <= 0.2) {
	if (xTruthJetIt->pt()<dPtMax)
	  continue;
	xTruthJetMatch = xTruthJetIt;
	dPtMax = xTruthJetIt->pt();
      }
    }
    
    if (xTruthJetMatch) {
      ElementLink < xAOD::JetContainer > lTruthParticleLink(xTruthJetMatch, *jetContainer);
      tau->auxdecor< ElementLink< xAOD::JetContainer > >("truthJetLink") = lTruthParticleLink;
    } else {
      ElementLink < xAOD::JetContainer > lTruthParticleLink;
      tau->auxdecor< ElementLink< xAOD::JetContainer > >("truthJetLink") = lTruthParticleLink;
    } 
  }

  if (tau->auxdata<char>("IsTruthMatched")) {
    auto truthPart = (*tau->auxdata< ElementLink< xAOD::TruthParticleContainer > >("truthParticleLink"));
    
    if (truthPart->isTau()) {
      bool isHadTau(false);
      try { 
	isHadTau = (bool)truthPart->auxdata<char>("IsHadronicTau");
      } catch (const SG::ExcAuxTypeMismatch& e) {
	isHadTau = truthPart->auxdata<bool>("IsHadronicTau");
      } 
      
      if (isHadTau) {
	Props::TATTruthMatch.set(tau, 15);
	return EL::StatusCode::SUCCESS;
      } else {
	if (m_tauTruthMatchingTool->getNTauDecayParticles(*tau, 11, true) > 0) {
	  //tau->enunu
	  Props::TATTruthMatch.set(tau, -11);
	  return EL::StatusCode::SUCCESS;
	} else if (m_tauTruthMatchingTool->getNTauDecayParticles(*tau, 13, true) > 0) {
	  //tau->mununu
	  Props::TATTruthMatch.set(tau, -13);
	  return EL::StatusCode::SUCCESS;	
	}
      }
    } 
  
    if (truthPart->isMuon()) {
      Props::TATTruthMatch.set(tau, 13);
      return EL::StatusCode::SUCCESS;
    }
    
    if (truthPart->isElectron()) {
      Props::TATTruthMatch.set(tau, 11);
      return EL::StatusCode::SUCCESS;
    }
  } else {

    // Jet
    const ElementLink< xAOD::JetContainer>  jet = tau->auxdecor<ElementLink<xAOD::JetContainer>>("truthJetLink");
    if (!jet.isValid()) {
      Props::TATTruthMatch.set(tau, 0); // Not matched
      return EL::StatusCode::SUCCESS;
    }
    
    int pdgid;
    (*jet)->getAttribute("PartonTruthLabelID", pdgid); 
    Props::TATTruthMatch.set(tau, pdgid);     
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TauHandler::calibrateCopies(xAOD::TauJetContainer * particles, const CP::SystematicSet & sysSet)
{

  int isMC = m_eventInfoHandler.get_isMC();

  // tell tool to apply systematic variation
  CP_CHECK("TauHandler::calibrateCopies()",m_tauSmearingTool->applySystematicVariation(sysSet),m_debug);

  CP_CHECK("TauHandler::calibrateCopies()",m_tauEffCorr->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool25->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool35->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool80->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool125->applySystematicVariation(sysSet),m_debug);
  CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool160->applySystematicVariation(sysSet),m_debug);

  for (xAOD::TauJet * tau : *particles) {

    //calibration
    //------------
    setP4( tau , tau );
    static SG::AuxElement::Accessor< xAOD::TauJet_v3::TauTrackLinks_t > acc( "tauTrackLinks" );

    //props initialised just in case the decorate is not run.
    Props::passTauSelector.set(tau,false);
    Props::forMETRebuild.set(tau, false);
    
    Props::BDTCut.set(tau, 0.05);
    // Antitau low BDT cut variation
    if (sysSet.name() == "ANTITAU_BDT_CUT") {
      Props::BDTCut.set(tau, 0.10);
    }

    double effSF = 1.0;
    double effSF_trigger25 = 1.0;
    double effSF_trigger35 = 1.0;
    double effSF_trigger80 = 1.0;
    double effSF_trigger125 = 1.0;
    double effSF_trigger160 = 1.0;

    if (tau->nTracks() > 0){
      bool accepted = m_tauSelectionTool->accept( *tau );
      bool passTauID = tau->isTau(m_xAOD_IDLevel);

      // Tau smearing tool correction is applied in data and mc
      // If accepted and not antitau it should be BDTMedium so don't need to check explicitly
      // For antitau we need to ask explicitly for BDTMedium as we will be accepting other looser identified taus as well
      if(!m_suppressTauCalibration && accepted) {
        CP_CHECK("TauHandler::calibrateCopies()",m_tauSmearingTool->applyCorrection(*tau),m_debug);

        if (isMC) {
          if (passTauID) {
            CP_CHECK("TauHandler::calibrateCopies()",m_tauEffCorr->getEfficiencyScaleFactor(*tau, effSF),m_debug);

            if (!m_useRNNTaus) {
              // TODO: Trigger scale factors for RNN not yet available
              CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool25->getEfficiencyScaleFactor(*tau, effSF_trigger25),m_debug);
              CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool35->getEfficiencyScaleFactor(*tau, effSF_trigger35),m_debug);
              CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool80->getEfficiencyScaleFactor(*tau, effSF_trigger80),m_debug);
              CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool125->getEfficiencyScaleFactor(*tau, effSF_trigger125),m_debug);
              CP_CHECK("TauHandler::calibrateCopies()",m_tauTriggerEffTool160->getEfficiencyScaleFactor(*tau, effSF_trigger160),m_debug);
            }

          } else {
            effSF = 1;
          }
        }
      }
    }

    Props::effSF.set(tau, effSF);
    Props::effSFtrigger25.set(tau, effSF_trigger25);
    Props::effSFtrigger35.set(tau, effSF_trigger35);
    Props::effSFtrigger80.set(tau, effSF_trigger80);
    Props::effSFtrigger125.set(tau, effSF_trigger125);
    Props::effSFtrigger160.set(tau, effSF_trigger160);

    // decorate tau
    if (decorate(tau) != EL::StatusCode::SUCCESS) return EL::StatusCode::FAILURE;
  }
  return EL::StatusCode::SUCCESS;
}

bool TauHandler::passLooseTau(xAOD::TauJet * tau) {
  bool passSel = true;
  passSel &= Props::passTauSelector.get(tau);

  if (m_antitau) {
    passSel &= tau->isTau(m_xAOD_IDLevel_AntiTau);

    // Chris: we might have to do something like this again if we want to derive systematics
    //
    // float bdtscore = tau->discriminant(xAOD::TauJetParameters::BDTJetScoreSigTrans);
    // if (m_useRNNTaus) {
    //   bdtscore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
    // }
    // passSel &= (bdtscore > Props::BDTCut.get(tau));
  }

  if(m_useTausInMET) Props::forMETRebuild.set(tau,passSel);//useTausInMET is set in the config file for MET rebuilding. Then it includes taus that have passed the default tau selection
  Props::passPreSel.set(tau, passSel);
  return passSel;
}

EL::StatusCode TauHandler::clearEvent() 
{
  m_truetaus_had.clear();
  m_truetaus_lep.clear();
  m_truetaus.clear();
  m_trueeles.clear();
  m_truemuons.clear();
  m_truepartons.clear();

  // to speed up memory allocation in the main loop
  m_truetaus_had.reserve(10);
  m_truetaus_lep.reserve(10);
  m_truetaus.reserve(10);
  m_trueeles.reserve(10);
  m_truemuons.reserve(10);
  m_truepartons.reserve(10);

  m_SearchedTrueTaus = false;
  EL_CHECK("TauHandler::clearEvent()",ObjectHandler::clearEvent());

  return EL::StatusCode::SUCCESS;
}

//Overload of ObjectHandlerBase link copier
EL::StatusCode TauHandler::fillOutputLinks(){

  if(m_debug){
    Info("TauHandler::fillOutputLinks", " Copying input container element links to output container (%s)", m_handlerName.c_str());
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
