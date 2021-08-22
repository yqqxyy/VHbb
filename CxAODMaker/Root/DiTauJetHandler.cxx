//tau includes
#include "xAODCore/AuxStoreAccessorMacros.h"
//#include "xAODTracking/TrackParticle.h"
//#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODTau/DiTauJet.h"
#include "xAODTau/DiTauJetAuxContainer.h"
#include "CxAODMaker/DiTauJetHandler.h"
//#include "xAODTruth/TruthParticleContainer.h"
//#include "TauAnalysisTools/Enums.h"
//#include "TauAnalysisTools/TauSelectionTool.h"
//#include "TauAnalysisTools/TauSmearingTool.h"
//#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
//#include "TauAnalysisTools/TauTruthMatchingTool.h"
//#include "TrigTauMatching/TrigTauMatching.h"
#include "tauRecTools/DiTauDiscriminantTool.h"
#include "tauRecTools/DiTauIDVarCalculator.h"
//#include "AsgTools/ToolHandle.h"
#include <iostream>
#include <vector>

DiTauJetHandler::DiTauJetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent *event,
                         EventInfoHandler & eventInfoHandler) :
  ObjectHandler(name, config, event, eventInfoHandler),
  //m_tauSmearingTool(0),
  //m_tauSelectionTool(0),
  //m_tauEffCorrLoose(0),
  //m_tauEffCorrMedium(0),
  //m_tauEfficiencyCorrections(0),
  //m_tauTriggerEffToolMedium25(0),
  //m_tauTriggerEffToolMedium35(0),
  //m_tauTriggerEffToolMedium80(0),
  //m_tauTriggerEffToolMedium125(0),
  //m_tauTriggerEffToolLoose25(0),
  //m_tauTriggerEffToolLoose35(0),
  //m_tauTriggerEffToolLoose80(0),
  //m_tauTriggerEffToolLoose125(0),
  //m_tauEfficiencyCorrections_eveto(0),
  //m_tauTruthMatchingTool(0),
  //m_truthhandler(0), 
  //m_SearchedTrueTaus(false),
  //m_runTruthMatchTool(false),
  //m_useTausInMET(false),
  //m_trigDecTool(nullptr),
  //m_PUReweightingTool(nullptr),
  //m_suppressTauCalibration(false),
  //m_doTrigMatch(false),
  //m_triggersToMatch(),
  //m_match_Tool(nullptr),
  m_IDVarCalculator(nullptr),
  m_DiscrTool(nullptr)
{
  using std::placeholders::_1;
  m_selectFcns.clear();
  //m_selectFcns.push_back(std::bind(&DiTauJetHandler::passLooseTau, this, _1));

  //m_config.getif<bool>("runTauTruthMatchTool",m_runTruthMatchTool);
  //m_config.getif<bool>("useTausInMET",m_useTausInMET);
  //m_config.getif<bool>("suppressTauCalibration",m_suppressTauCalibration);
  //m_config.getif<bool>("AntiTau",m_antitau);
}

DiTauJetHandler::~DiTauJetHandler() {

  //delete tool:
  //delete m_tauSmearingTool;
  //delete m_tauSelectionTool;
  //delete m_tauEfficiencyCorrections;
  //delete m_tauEffCorrLoose;
  //delete m_tauEffCorrMedium;
  //delete m_tauTriggerEffToolMedium25;
  //delete m_tauTriggerEffToolMedium35;
  //delete m_tauTriggerEffToolMedium80;
  //delete m_tauTriggerEffToolMedium125;
  //delete m_tauTriggerEffToolLoose25;
  //delete m_tauTriggerEffToolLoose35;
  //delete m_tauTriggerEffToolLoose80;
  //delete m_tauTriggerEffToolLoose125;
  //delete m_tauEfficiencyCorrections_eveto;

}

EL::StatusCode DiTauJetHandler::initializeTools()
{

  // ----------------------------------------------------------------------------
  // initialize tool to calculate ID variables
  m_IDVarCalculator = new tauRecTools::DiTauIDVarCalculator("IDVarCalculator");
  TOOL_CHECK("DiTauJetHandler::initializeTools()", m_IDVarCalculator->initialize() );

  // specify the BDT weights file
  //std::string sWeightsDir = getenv("WorkDir_DIR");
  //sWeightsDir += "/data/tauRecTools/";
  //std::string sWeightsFile = sWeightsDir + "DiTau_JetBDT_prelim.weights.xml";

  // initialize ditau discriminant tool
  m_DiscrTool = new tauRecTools::DiTauDiscriminantTool("DiTauDiscriminantTool");
  //TOOL_CHECK("DiTauJetHandler::initializeTools()", m_DiscrTool->setProperty("WeightsFile", sWeightsFile) );
  TOOL_CHECK("DiTauJetHandler::initializeTools()", m_DiscrTool->initialize() );

  std::cout << "Initialize Tools DiTauJetHandler" << std::endl;

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode DiTauJetHandler::writeOutputVariables(xAOD::DiTauJet * inTau, xAOD::DiTauJet * outTau, bool isKinVar, bool isWeightVar, const TString& sysName)
{

  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    EL_CHECK("DiTauJetHandler::writeOutputVariables",writeAllVariations(inTau, outTau, sysName));
  }
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    EL_CHECK("DiTauJetHandler::writeOutputVariables",writeKinematicVariations(inTau, outTau, sysName));
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes  
    EL_CHECK("DiTauJetHandler::writeOutputVariables",writeWeightVariations(inTau, outTau, sysName));
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited 
    EL_CHECK("DiTauJetHandler::writeOutputVariables",writeAllVariations(inTau, outTau, sysName));
    EL_CHECK("DiTauJetHandler::writeOutputVariables",writeKinematicVariations(inTau, outTau, sysName));
    EL_CHECK("DiTauJetHandler::writeOutputVariables",writeWeightVariations(inTau, outTau, sysName));
    EL_CHECK("DiTauJetHandler::writeOutputVariables",writeNominal(inTau, outTau, sysName));
  } else assert(false);

  return writeCustomVariables(inTau, outTau, isKinVar, isWeightVar, sysName);
  
}

EL::StatusCode DiTauJetHandler::writeAllVariations(xAOD::DiTauJet*, xAOD::DiTauJet*, const TString&)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::writeKinematicVariations(xAOD::DiTauJet* inTau, xAOD::DiTauJet* outTau, const TString& /*sysName*/)
{
  // set four momentum
  setP4( inTau , outTau );

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::writeWeightVariations(xAOD::DiTauJet* /*inTau*/, xAOD::DiTauJet* /*outTau*/, const TString& /*sysName*/)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::writeNominal(xAOD::DiTauJet* inTau, xAOD::DiTauJet* outTau, const TString& /*sysName*/)
{
  Props::BDTScore.copy(inTau,outTau);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::writeCustomVariables(xAOD::DiTauJet*, xAOD::DiTauJet*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::decorateOriginParticle(const xAOD::DiTauJet * /*ditau*/) 
{
  // This method should only be applied to things that don't depend on the calibration
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::addCPVariations(const std::vector<TString> &variations,
                  const bool filterByTools, const bool /*skipWeightVar*/) {

  // add the usual variations, filtered by registered tools if requested
  EL_CHECK("DiTauJetHandler::addCPVariations()", ObjectHandlerBase::addCPVariations(variations, filterByTools));
  // see if we want some additional ones
  std::vector<TString> addVariations;  for (TString variation : variations) {
    bool allow = false;
    allow |= variation.EqualTo("ANTITAU_BDT_CUT");
    if (!allow) continue;
    addVariations.push_back(variation);
  }  

  // add them to the list w/o further filtering
  return ObjectHandlerBase::addCPVariations(addVariations, false);
}

EL::StatusCode DiTauJetHandler::decorate(xAOD::DiTauJet * ditau)
{
  /* Set tau variables - derived ones*/
  //Props::BDTScore.set(ditau,ditau->auxdata<float>("BDT"));

  double iBDTScore;
  TOOL_CHECK("DiTauJetHandler::decorate()", m_IDVarCalculator->calculateIDVariables(*ditau) );
  iBDTScore = m_DiscrTool->getJetBDTScore(*ditau);

  Props::BDTScore.set(ditau,iBDTScore);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::calibrateCopies(xAOD::DiTauJetContainer * particles, const CP::SystematicSet & /*sysSet*/)
{

  for (xAOD::DiTauJet * ditau : *particles) {
      //calibration
      //------------
      setP4( ditau , ditau );

      // decorate ditau
      if ( decorate( ditau ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode DiTauJetHandler::clearEvent() 
{
  EL_CHECK("DiTauJetHandler::clearEvent()",ObjectHandler::clearEvent());

  return EL::StatusCode::SUCCESS;

}
