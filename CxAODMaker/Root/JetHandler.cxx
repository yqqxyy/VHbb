#include <iostream>

#include "CxAODMaker/JetHandler.h"
#include "CxAODMaker/JetSemileptonic.h"
#include "CxAODMaker/EventInfoHandler.h"
//#include "JetResolution/JERTool.h"
//#include "JetResolution/JERSmearingTool.h"
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
#include "PathResolver/PathResolver.h"
#include "JetMomentTools/JetForwardJvtTool.h"
#include "JetJvtEfficiency/JetJvtEfficiency.h"
#include "BoostedJetTaggers/JetQGTagger.h"
#include "JetTileCorrection/JetTileCorrectionTool.h"
#include "xAODBTagging/BTagging.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/TActiveStore.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/JetTrackFilterTool.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "METUtilities/METHelpers.h"


JetHandler::JetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
    EventInfoHandler & eventInfoHandler) :
  ObjectHandler(name, config, event, eventInfoHandler),
  m_jetCalibration(nullptr),
  m_jetCalibToGetRes(nullptr),
  m_jetCleaning(nullptr),
  //m_jetResolution(nullptr),
  //m_jetResolutionSmearing(nullptr),
  //m_jesProvider(nullptr),
  m_fjvtTool(nullptr),
  m_jvtCalib(nullptr),
  m_fjvtCalib(nullptr),
  m_qgtagger(nullptr),
  m_jetTileCorrectionTool(nullptr),
  m_trackSelectionTool(nullptr),
  m_trackFakeTool(nullptr),
  m_trackOriginTool(nullptr),
  m_jetTrackFilterTool(nullptr),
  m_trigDecTool(nullptr),
  m_triggersToMatch(),
  m_jetAlgoName("none"),
  m_fjvtSF("Tight"),
  m_sysName(""),
  m_isMC(0),
  m_applyJetSemileptonic(false),
  m_countElectronInJet(false),
  m_correctMuonInJet(false),
  m_saveSemileptonicInfoInJet(false),
  m_applyJetRegression(false),
  m_writeRegVars(false),
  m_matchTruthJet(false),
  m_storeGAParticlesInJets(false),
  m_doResolution(false),
  m_doFJVT(true),
  m_qgtagging(false),
  m_saveqgvariables(false),
  m_applyJetTileCorrection(false),
  m_enableLegacyJetCleaning(false)
{

  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind(&JetHandler::passLooseJet, this, _1));

  // set jet algorithm name
  m_config.getif<std::string>(name+"AlgoName",m_jetAlgoName);
  m_isPFlow = (m_jetAlgoName.find("PFlow")!=std::string::npos);
  m_config.getif<std::string>("fjvtSF",m_fjvtSF);

  // decide if to study semileptonic jet decays
  m_config.getif<bool>("applyJetSemileptonic",m_applyJetSemileptonic);
  m_config.getif<bool>("countElectronInJet",m_countElectronInJet);
  m_config.getif<bool>("correctMuonInJet",m_correctMuonInJet);
  m_config.getif<bool>("saveSemileptonicInfoInJet",m_saveSemileptonicInfoInJet);
  m_config.getif<bool>("applyJetRegression",m_applyJetRegression);
  m_config.getif<bool>("writeRegVars",m_writeRegVars);
  m_config.getif<bool>("matchTruthJet",m_matchTruthJet);
  m_config.getif<bool>("storeGAParticlesInJets",m_storeGAParticlesInJets);
  m_config.getif<bool>("doResolution",m_doResolution);
  m_config.getif<bool>("doFJVT",m_doFJVT);
  m_config.getif<bool>("doqgtagging",m_qgtagging);
  m_config.getif<bool>("saveqgvariables",m_saveqgvariables);
  

  m_config.getif<bool>("applyJetTileCorrection",m_applyJetTileCorrection);
  m_config.getif<bool>("enableLegacyJetCleaning",m_enableLegacyJetCleaning);
  m_doCopyBeforeSyst = true;
}

JetHandler::~JetHandler() {

  //delete tools
  delete m_jetCalibration;
  delete m_jetCalibToGetRes;
  delete m_jetCleaning;
  //delete m_jetResolution;
  //delete m_jetResolutionSmearing;
  //delete m_jesProvider;
  for (auto & kv: m_jesProviders)
    delete kv.second;
  delete m_fjvtTool;
  delete m_jvtCalib;
  delete m_fjvtCalib;
  delete m_jetTrackFilterTool;
  delete m_trackFakeTool;
  delete m_trackSelectionTool;
  delete m_trackOriginTool;
}

EL::StatusCode JetHandler::initializeTools()
{
  m_isMC = m_eventInfoHandler.get_isMC();

  //initialize jet calibration tool - this hopefully will become less messy...
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ApplyJetCalibration2016
  //-----------------------------------------------------------------------------

  std::string calibSeq = "JetArea_Residual_EtaJES_GSC";
  std::string configFile =  "";
  bool isFullSim = !m_eventInfoHandler.get_isAFII();

  std::string configFile_afii,configFile_fullsim;
  if (m_jetAlgoName.find("EMPFlow")!=std::string::npos) configFile_fullsim = "JES_MC16Recommendation_Consolidated_PFlow_Apr2019_Rel21.config";
  else configFile_fullsim = "JES_MC16Recommendation_Consolidated_EMTopo_Apr2019_Rel21.config";
  if (m_jetAlgoName.find("EMPFlow")!=std::string::npos) configFile_afii = "JES_MC16Recommendation_AFII_PFlow_Apr2019_Rel21.config";
  else configFile_afii = "JES_MC16Recommendation_AFII_EMTopo_Apr2019_Rel21.config";

  configFile = (isFullSim ? configFile_fullsim : configFile_afii);

  if(!m_isMC) calibSeq += "_Insitu";
  else calibSeq += "_Smear";
  std::string calibArea =  "00-04-82"; // new propertiy to be set from 21.2.10 onwards
  Info("JetHandler::initializeTools()","m_containerName = %s, m_jetAlgoName = %s, m_isMC = %s",m_containerName.c_str(), m_jetAlgoName.c_str(), m_isMC ? "true" : "false");
  std::string calibToolName = "JetCorrectionTool_" + m_jetAlgoName;
  m_jetCalibration = new JetCalibrationTool(calibToolName);
  m_jetCalibration->msg().setLevel( m_msgLevel );

  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibration->setProperty("JetCollection", m_jetAlgoName));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibration->setProperty("ConfigFile", configFile));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibration->setProperty("CalibSequence", calibSeq));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibration->setProperty("CalibArea", calibArea));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibration->setProperty("IsData", !m_isMC));
  TOOL_CHECK("JetHandler()::initialize()",m_jetCalibration->initializeTool(calibToolName));

  m_jetCalibToGetRes = new JetCalibrationTool("jetCalibToGetRes");
  m_jetCalibToGetRes->msg().setLevel( m_msgLevel );

  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibToGetRes->setProperty("JetCollection", m_jetAlgoName));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibToGetRes->setProperty("ConfigFile", configFile_fullsim));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibToGetRes->setProperty("CalibSequence", "JetArea_Residual_EtaJES_GSC_Smear"));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibToGetRes->setProperty("CalibArea", calibArea));
  TOOL_CHECK("JetHandler()::initializeTools()", m_jetCalibToGetRes->setProperty("IsData", false));
  TOOL_CHECK("JetHandler()::initialize()",m_jetCalibToGetRes->initializeTool("jetCalibToGetRes"));  

  // initialize the JES uncertainty tool
  //https://gitlab.cern.ch/atlas/athena/tree/21.2/Reconstruction/Jet/JetUncertainties
  // ===> Patch for running mutiple scale schemes simultaneously
  std::string jesConfig_base = "rel21/Summer2019/R4_";
  m_config.getif<std::string>("jesConfig_base", jesConfig_base);
  std::vector<std::string> jesPrefixes;
  m_config.getif<std::vector<std::string> >("jesPrefixes", jesPrefixes);
  std::vector<std::string> jesConfigs;
  m_config.getif<std::vector<std::string> >("jesConfigs", jesConfigs);
  std::vector<std::string> jerConfigs;
  m_config.getif<std::vector<std::string> >("jerConfigs", jerConfigs);
//  std::string jesCalibArea =  "CalibArea-06"; // automatically set from 21.2.51
//  m_config.getif<std::string>("jesCalibArea", jesCalibArea);
  // ignore these two settings if not correctly set up
  if (jesPrefixes.size() != jesConfigs.size()) {
    jesConfigs.clear();
    jesPrefixes.clear();
  }
  // if jesConfigs or jerConfigs empty, then try to resolve which config file to use
  if (jesConfigs.empty() || jerConfigs.empty()) {
    std::string jesConfig;
    m_config.getif<std::string>("jesConfigFile", jesConfig);
    // user didn't supply the value, hard-coded one to be used
    if (jesConfig.empty()) {
      jesConfig_base = "rel21/Summer2019/R4_"; //Moriond 2017 recommendation
      jesConfig = "SR_Scenario1_SimpleJER"; // strongly reduced configuration scenario 1
    }
    else {
      // to avoid crash when jesConfig_base not sepcified or not consistent
      // with jesConfig (e.g. jesConfig_base = JES_2015/Moriond2016/JES2015_
      // and jesConfig = JES_2015/ICHEP2016/JES2015_SR_Scenario1.config)
      if (jesConfig.find(jesConfig_base) != 0)
        jesConfig_base = jesConfig.substr(0, jesConfig.rfind("/")+1);
      // shorten jesConfig to whatever is between jesConfig_base and ".config", e.g. SR_Scenario1
      jesConfig = jesConfig.substr(jesConfig.find(jesConfig_base)+jesConfig_base.length(),
          jesConfig.find(".config")-jesConfig_base.length());
    }
    jesConfigs.push_back(jesConfig);
  }
  // initializations of JES uncertainty tools
  for (unsigned idx = 0; idx < jesConfigs.size(); ++idx) {
  for (unsigned idxjer = 0; idxjer < jerConfigs.size(); ++idxjer) {
    std::string jesConfig = jesConfigs[idx];
    std::string jerConfig = jerConfigs[idxjer];
    // assign a dummy prefix if jesPrefixes empty, better refrain from using "DUMMYKEY" in the actual config to avoid confusion.
    std::string jesPrefix = jesPrefixes.empty() ? "DUMMYKEY" : jesPrefixes[idx];
    Info("JetHandler::initializeTools()","JES uncertainty configuration discovered: %s", jesConfig.c_str());
    m_jesProviders[jesPrefix] = new JetUncertaintiesTool(("JESProvider_"+jesConfig+"_"+jerConfig+"_"+m_jetAlgoName).c_str());
    m_jesProviders[jesPrefix]->msg().setLevel( m_msgLevel );
    TOOL_CHECK("JetHandler()::initializeTools()", m_jesProviders[jesPrefix]->setProperty("JetDefinition", m_jetAlgoName ));
    TOOL_CHECK("JetHandler()::initializeTools()", m_jesProviders[jesPrefix]->setProperty("IsData",!m_isMC));
    TOOL_CHECK("JetHandler()::initializeTools()", m_jesProviders[jesPrefix]->setProperty("MCType",isFullSim?"MC16":"AFII"));
    TOOL_CHECK("JetHandler()::initializeTools()", m_jesProviders[jesPrefix]->setProperty("ConfigFile", jesConfig_base+jesConfig+"_"+jerConfig+".config"));
//    TOOL_CHECK("JetHandler()::initializeTools()", m_jesProviders[jesPrefix]->setProperty("CalibArea", jesCalibArea)); // automatically set from 21.2.51
    TOOL_CHECK("JetHandler()::initializeTools()", m_jesProviders[jesPrefix]->initialize());

    // add in CP::SystematicSet manually since the names of the systematics in the
    // CxAOD will be different from the "official names" in the CP tool.
    std::vector<std::string> pulls = {"__1down", "__1up"};
    CP::SystematicSet jes_sysSet = m_jesProviders[jesPrefix]->recommendedSystematics();
    const std::set<std::string> sysNames = jes_sysSet.getBaseNames();
    for (auto sysName: sysNames) {
      for (auto pull : pulls) {
        std::string sysPulled = sysName + pull;
        // for easier retrieval of CP::SystematicSet based on the new name of systematic
        m_jes_sysSets.insert({jesPrefix+sysPulled, CP::SystematicSet(sysPulled)});
        // for easier retrieval of jesPrefix and thus corresponding JES uncertainty tool
        // based on the new name of systematic
        m_jes_prefixes.insert({jesPrefix+sysPulled, jesPrefix});
      }
    }
  }
  }

  // initialize and configure the jet cleaning tool
  //------------------------------------------------
  m_jetCleaning = new JetCleaningTool(("JetCleaning"+m_jetAlgoName).c_str());
  m_jetCleaning->msg().setLevel( m_msgLevel );
  TOOL_CHECK("JetHandler()::initializeTools()",m_jetCleaning->setProperty( "CutLevel", "LooseBad"));
  TOOL_CHECK("JetHandler()::initializeTools()",m_jetCleaning->setProperty("DoUgly", false));
  TOOL_CHECK("JetHandler()::initializeTools()",m_jetCleaning->initialize());
  // initialize and configure the JER tools
  //JER : note that the JER correction is done together with the JES. This JERTool is for 2015 prerecom
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetEnergyResolutionXAODTools
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetResolution2015Prerecom
  //------------------------------------------------
  //----
//  m_jetResolution = new JERTool(("JERTool"+m_jetAlgoName).c_str());
//  m_jetResolution->msg().setLevel( m_msgLevel );
//  TOOL_CHECK("JetHandler()::initializeTools()",m_jetResolution->setProperty("PlotFileName", "JetResolution/Prerec2015_xCalib_2012JER_ReducedTo9NP_Plots_v2.root"));
//  TOOL_CHECK("JetHandler()::initializeTools()",m_jetResolution->setProperty("CollectionName", m_containerName));
//
//  //JER smearing
//  //-------------
//  m_jetResolutionSmearing = new JERSmearingTool(("JERSmearingTool"+m_jetAlgoName).c_str());
//  m_jetResolutionSmearing->msg().setLevel( m_msgLevel );
//
//  ToolHandle<IJERTool> jerHandle(m_jetResolution->name());
//  TOOL_CHECK("JetHandler()::initializeTools()", m_jetResolutionSmearing->setProperty("JERTool", jerHandle));
//  TOOL_CHECK("JetHandler()::initializeTools()", m_jetResolutionSmearing->setProperty("ApplyNominalSmearing", false));
//  TOOL_CHECK("JetHandler()::initializeTools()", m_jetResolutionSmearing->setProperty("isMC", m_isMC));
//  TOOL_CHECK("JetHandler()::initializeTools()", m_jetResolutionSmearing->setProperty("SystematicMode", "Simple"));//TODO: cross check that the simple model is what we really want!
//
//  //These two tools are linked, the example shows them initialized together
//  TOOL_CHECK("JetHandler()::initializeTools()",m_jetResolution->initialize());
//  TOOL_CHECK("JetHandler()::initializeTools()",m_jetResolutionSmearing->initialize());

  //for central jet: JVT recalculation tool
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetVertexTaggerTool
  //------------------------
  TOOL_CHECK("JetHandler()::initializeTools()", m_JVTUpdateTool_handle.setProperty("OutputLevel", m_msgLevel ));
  TOOL_CHECK("JetHandler()::initializeTools()",m_JVTUpdateTool_handle.setProperty("JVTFileName",PathResolverFindCalibFile("JetMomentTools/JVTlikelihood_20140805.root"))); //might be made configurable
  TOOL_CHECK("JetHandler()::initializeTools()",m_JVTUpdateTool_handle.initialize());

  //for forward jet: JVT recalculation tool (fJVT tool)
  //https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ForwardJVT
  if(m_doFJVT){
    m_fjvtTool = new JetForwardJvtTool("fjvtTool");
    m_fjvtTool->msg().setLevel( m_msgLevel );
    // setup for MET
    TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtTool->setProperty("OutputDec", "passFJVTTight"));
    TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtTool->setProperty("UseTightOP", true));
    //if(m_fjvtSF=="DefaultMET") TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtTool->setProperty("CentralMaxPt",60e3)); // no longer need to set this parameter.
    TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtTool->initialize());
  }
  //for central jet: JVT calibration tool
  //https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JVTCalibration
  m_jvtCalib = new CP::JetJvtEfficiency("jvtCalib");
  m_jvtCalib->msg().setLevel( m_msgLevel );
  std::string jetType="";
  if (m_jetAlgoName.find("EMPFlow")!=std::string::npos) {
    jetType="EMPFlow";
  } else if (m_jetAlgoName.find("EM")!=std::string::npos) {
    jetType="EMTopoJets";
  } else if (m_jetAlgoName.find("LC")!=std::string::npos) {
    jetType="LC";
  } else {
    Error("JetHandler::initializeTools()","m_jetAlgoName %s is neither of type EM nor of type LC. This info is needed to initialize JvtJetEfficiency tool. Exiting", m_jetAlgoName.c_str());
    return EL::StatusCode::FAILURE;
  }
  TOOL_CHECK("JetHandler()::initializeTools()",m_jvtCalib->setProperty("SFFile","JetJvtEfficiency/Moriond2018/JvtSFFile_"+jetType+".root")); // update to new Jvt recommendation 2018/09/18
  TOOL_CHECK("JetHandler()::initializeTools()",m_jvtCalib->initialize());

  //for forward jet: JVT calibration tool
  m_fjvtCalib = new CP::JetJvtEfficiency("fjvtCalib");
  m_fjvtCalib->msg().setLevel( m_msgLevel );
  std::string fjvt_working_point = m_fjvtSF;
  if(m_fjvtSF=="DefaultMET") fjvt_working_point = "Medium";
  TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtCalib->setProperty("WorkingPoint",fjvt_working_point));
  TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtCalib->setProperty("SFFile","JetJvtEfficiency/Moriond2018/fJvtSFFile.root")); //update to new Jvt recommendation 2018/09/18
  TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtCalib->setProperty("ScaleFactorDecorationName","fJVTSF"));
  TOOL_CHECK("JetHandler()::initializeTools()",m_fjvtCalib->initialize());

  //JetQGTagger tool
  //https://twiki.cern.ch/twiki/bin/view/AtlasProtected/QuarkGluonTagging
  if(m_qgtagging){
    int NTrackCut (-1);
    m_config.getif<int>("NTrackCut", NTrackCut);
    m_qgtagger = std::unique_ptr<CP::JetQGTagger>( new CP::JetQGTagger( "JetQGTagger" ) );
    TOOL_CHECK("JetHandler()::initializeTools()",m_qgtagger->setProperty("NTrackCut", NTrackCut));
    m_qgtagger->msg().setLevel( m_msgLevel );
    TOOL_CHECK("JetHandler()::initializeTools()",m_qgtagger->initialize());
  }

  // Track systematics tool: InDetTrackTruthOriginTool
  m_trackOriginTool = new InDet::InDetTrackTruthOriginTool("InDet::InDetTrackTruthOriginTool");
  m_trackOriginTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("JetHandler()::initializeTools()",m_trackOriginTool->initialize());

  // Track systematics tool: JetTrackFilterTool
  // For variations: TRK_EFF_LOOSE_TIDE
  // See: https://gitlab.cern.ch/JSSTools/BoostedJetTaggers/blob/master/Root/JetQGTagger.cxx#L145
  m_jetTrackFilterTool = new InDet::JetTrackFilterTool("TrackTool");
  m_jetTrackFilterTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("JetHandler()::initializeTools()",m_jetTrackFilterTool->setProperty("Seed",1243));
  TOOL_CHECK("JetHandler()::initializeTools()",m_jetTrackFilterTool->initialize());

  // Track systematics tool: InDetTrackTruthFilterTool
  // For variations: TRK_FAKE_RATE_LOOSE
  // See: https://gitlab.cern.ch/JSSTools/BoostedJetTaggers/blob/master/Root/JetQGTagger.cxx#L137
  m_trackFakeTool = new InDet::InDetTrackTruthFilterTool("TrackTruthTool");
  m_trackFakeTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("JetHandler()::initializeTools()",m_trackFakeTool->setProperty("Seed",1243));
  TOOL_CHECK("JetHandler()::initializeTools()",m_trackFakeTool->initialize());

  // Track selection tool: InDetTrackSelectionTool
  m_trackSelectionTool = new InDet::InDetTrackSelectionTool("TrackSelector");
  m_trackSelectionTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("JetHandler()::initializeTools()",m_trackSelectionTool->setProperty("CutLevel","Loose"));
  TOOL_CHECK("JetHandler()::initializeTools()",m_trackSelectionTool->initialize());


  //JetTileCorrectionTool apply tile dead module energy (pt and m) correction but ONLY for data
  //https://indico.cern.ch/event/568686/contributions/2298880/attachments/1334007/2005868/JetTileCorrectionTool.pdf
  //https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetTileCorrectionTool
  //https://svnweb.cern.ch/trac/atlasoff/browser/Reconstruction/Jet/JetAnalysisTools/JetTileCorrection
  //https://cds.cern.ch/record/2226554?
  //https://its.cern.ch/jira/browse/ATLASG-800
  //https://its.cern.ch/jira/browse/CXAOD-342
  if (m_applyJetTileCorrection) {
    m_jetTileCorrectionTool = new CP::JetTileCorrectionTool("jetTileCorrectionTool");
    TOOL_CHECK("JetHandler()::initializeTools()", m_jetTileCorrectionTool->setProperty("isMC",(int)m_isMC));
    TOOL_CHECK("JetHandler()::initializeTools()", m_jetTileCorrectionTool->initialize());
  }

  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear();
  //m_sysToolList.push_back(m_jesProvider);
  for (const auto& kv: m_jesProviders)
    m_sysToolList.push_back(kv.second);
  //m_sysToolList.push_back(m_jetResolutionSmearing);
  m_sysToolList.push_back(m_jvtCalib);
  m_sysToolList.push_back(m_fjvtCalib);
  if (m_qgtagging){
    m_sysToolList.push_back(m_qgtagger.get());
  }
  m_sysToolList.push_back(m_jetTrackFilterTool);
  m_sysToolList.push_back(m_trackFakeTool);

  // b-jet trigger matching
  m_trigDecTool = m_eventInfoHandler.get_TrigDecTool();

  std::unordered_map<std::string, PROP<int>*> triggersToMatch {
      {"HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700", &Props::matchBJetHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490", &Props::matchBJetHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490},
      {"HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700", &Props::matchBJetHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700", &Props::matchBJetHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490", &Props::matchBJetHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490},
      {"HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490", &Props::matchBJetHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490},
      {"HLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500", &Props::matchBJetHLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500},
      {"HLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500", &Props::matchBJetHLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500}
  };
  m_triggersToMatch = triggersToMatch;

  return EL::StatusCode::SUCCESS;

}

bool JetHandler::passLooseJet(xAOD::Jet* jet) {
  bool passSel = true;

  passSel &= (Props::PassJvtMedium.get(jet));
  passSel &= (fabs(jet->eta()) < 4.5);
  passSel &= (jet->pt() > 20000.);

  Props::passPreSel.set(jet, passSel);

  return passSel;
}

EL::StatusCode JetHandler::decorate(xAOD::Jet * jet)
{
  // add decoration to jet
  bool passJet = false;
  if(m_isPFlow){
      passJet = true;
  }else if(!m_enableLegacyJetCleaning){
    if ( ! Props::DFCommonJets_jetClean_LooseBad.exists(jet) ) {
        Error("JetHandler::decorate()", "Jet cleaning variable is not stored in your DxAOD. Please turn on the config enableLegacyJetCleaning to use customised jet cleaning.");
        return EL::StatusCode::FAILURE;
    } 
      passJet = Props::DFCommonJets_jetClean_LooseBad.get(jet); //read directly from new variable from DAOD, as suggested in twiki https://twiki.cern.ch/twiki/bin/view/AtlasProtected/HowToCleanJets2017
  // have only Loose and Tight WP, the standard prescription is the Loose one
  }else{
      passJet = m_jetCleaning->accept( *jet ); // keep the availability for customisation
  }
  Props::goodJet.set(jet, (passJet) );

  int passJvt = m_jvtCalib->passesJvtCut(*jet);
  Props::PassJvtMedium.set(jet, passJvt);
  float JvtSF = 1.;
  if(m_isMC) {
    CP_CHECK("JetHandler::decorate()", m_jvtCalib->getEfficiencyScaleFactor(*jet,JvtSF), m_debug);
  }
  Props::JvtSF.set(jet, JvtSF);

  //QG tagging
  if(m_qgtagging){
    float tagger = -1;
    float tagweight = 1;
    int tag_qg_result = -1;
    m_config.getif<bool>("doqgtagging",m_qgtagging);
    if(m_qgtagger->tag(*jet,NULL)>0)  tag_qg_result = 1;
    else if(m_qgtagger->tag(*jet,NULL)==0)  tag_qg_result = 0;
    tagger = jet->auxdata<float>("qgTagger");
    tagweight = jet->auxdata<float>("qgTaggerWeight");
    Props::QGTagger.set(jet, tagger);
    Props::QGTaggerWeight.set(jet, tagweight);
    Props::QGTag_result.set(jet, tag_qg_result);
  }
  
  //JetTileCorrectionTool apply tile dead module energy (pt and m) correction but ONLY for data
  if(m_applyJetTileCorrection && !m_isMC) {
    CP_CHECK("JetHandler::decorate()", m_jetTileCorrectionTool->applyCorrection(*jet), m_debug);
  }

  if(m_doResolution){
    double resData=-1;
    double resMC=-1;
    CP_CHECK("JetHandler::decorate()", m_jetCalibToGetRes->getNominalResolutionData((*jet), resData), m_debug);
    CP_CHECK("JetHandler::decorate()", m_jetCalibToGetRes->getNominalResolutionMC((*jet), resMC), m_debug);
    if ( resData > resMC ) {
      Props::resolution.set(jet, resData);
    } else {
      Props::resolution.set(jet, resMC);
    }
  }

  //Set jet Width related parameters with respect to primary vertex
  std::vector<float> TrackWidthPt500;
  std::vector<int> NumTrkPt500, NumTrkPt1000;
  bool status(false);

  int indexPV =  m_eventInfoHandler.get_indexPV();

  status=jet->getAttribute("TrackWidthPt500",TrackWidthPt500);
  if (!status) {
    Props::TrackWidthPt500PV.set(jet, -100000.);
  } else {
    if ((int)TrackWidthPt500.size()<=indexPV) {
      Props::TrackWidthPt500PV.set(jet, -100000.);
    } else {
      Props::TrackWidthPt500PV.set(jet, TrackWidthPt500[indexPV]);
    }
  }

  jet->getAttribute("NumTrkPt500",NumTrkPt500);
  jet->getAttribute("NumTrkPt1000",NumTrkPt1000);
  Props::NumTrkPt1000PV.set(jet, NumTrkPt1000[indexPV]);
  Props::NumTrkPt500PV.set(jet, NumTrkPt500[indexPV]);

  // set btagging discriminants in the output
  EL_CHECK("JetHandler::decorate()",retrieveBTaggingDiscriminants(jet));

  // trigger matching: match offline central jets to HLT BJets
  BJetTriggerMatch(jet);

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode JetHandler::decorateFJVT(xAOD::Jet * jet)
{
  //forward JVT
  if(m_doFJVT){
    float fjvt = m_fjvtTool->getFJVT(jet)/jet->pt();
    Props::PassFJvtTight.set(jet, fjvt<0.4);
    Props::PassFJvtLoose.set(jet, fjvt<0.5);
    Props::PassFJVTMET.set(jet, fabs(jet->eta())>2.4 ? fjvt<0.5 : true);
    Props::FJvt.set(jet,fjvt);
    float FJvtSF = 1.;
    if(m_isMC) {
      CP_CHECK("JetHandler::decorate()", m_fjvtCalib->getEfficiencyScaleFactor(*jet,FJvtSF), m_debug);
    }
    Props::FJvtSF.set(jet, FJvtSF);
    Props::FJvt.set(jet,fjvt);
  }else{
    Props::PassFJvtTight.set(jet, true);
    Props::PassFJvtLoose.set(jet, true);
    Props::FJvtSF.set(jet, 1.0);
  }
  return EL::StatusCode::SUCCESS;

}

EL::StatusCode JetHandler::countElectronInJet(JetSemileptonic* m_jetSemileptonic, const xAOD::ElectronContainer* electrons) {
  if(m_debug) { std::cout<<"JetHandler::countElectronInJet m_countElectronInJet="<<m_countElectronInJet<<std::endl; }
  if (!m_countElectronInJet) { return EL::StatusCode::SUCCESS; }
  if (!electrons) {
    Warning("JetHandler::countElectronInJet","The pointer to electron container is nullptr (no electrons).");
    return EL::StatusCode::SUCCESS;
  }
  // loop over all jet variations
  for(auto& x: *getInParticles()) { //std::map<TString, partContainer*>* getInParticles();
    const TString& variation=x.first;
    if (variation!="Nominal") continue;
    if(m_debug) { std::cout<<"JetHandler::countElectronInJet variation="<<variation<<std::endl;}
    // loop over the jets in the container belonging to this variation
    for (xAOD::Jet * jet : *x.second) {
      EL_CHECK("countElectronInJet", m_jetSemileptonic->countElectronInJet(jet,variation,electrons));
    }//end loop over jets in the container for this variation
  }// end loop over variations
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::correctMuonInJet(JetSemileptonic* m_jetSemileptonic, const xAOD::MuonContainer* muons) {
  if(m_debug) { std::cout<<"JetHandler::correctMuonInJet m_correctMuonInJet="<<m_correctMuonInJet<<std::endl; }
  if (!m_correctMuonInJet) { return EL::StatusCode::SUCCESS;}
  if (!muons) {
    Warning("JetHandler::correctMuonInJet","The pointer to muon container is nullptr (no muons).");
    return EL::StatusCode::SUCCESS;
  }
  // loop over all jet variations
  for(auto& x: *getInParticles()) { //std::map<TString, partContainer*>* getInParticles();
    const TString& variation=x.first;
    if (variation!="Nominal") continue;
    if(m_debug) { std::cout<<"JetHandler::correctMuonInJet  variation="<<x.first<<std::endl; }
    //loop over the jets in the container belonging to this variation
    for (xAOD::Jet * jet : *x.second) {
      EL_CHECK("correctMuonInJet",m_jetSemileptonic->correctMuonInJet(jet,variation,muons,true));//true, as apply PtReco
    }//end loop over jets in the container for this variation
  }// end loop over variations
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::correctRegressionForJet() {
  if(m_debug) { std::cout<<"JetHandler::correctRegressionForJet m_applyJetRegression="<<m_applyJetRegression<<std::endl; }
  if (!m_applyJetRegression) { return EL::StatusCode::SUCCESS;}
  // loop over all jet variations
  for(auto& x: *getInParticles()) { //std::map<TString, partContainer*>* getInParticles();
    const TString& variation=x.first;
    if (variation!="Nominal") continue;
    if(m_debug) { std::cout<<"JetHandler::correctRegressionForJet variation="<<x.first<<std::endl; }
    //loop over the jets in the container belonging to this variation
    for (xAOD::Jet * jet : *x.second) {
      //apply the regression, scale the 4-vector by the factor
      TLorentzVector j=jet->p4();
      double factorRegression=(fabs(jet->pt())>0.001) ? Props::ptCorr.get(jet)/jet->pt() : 0.0;
      Props::factorRegression.set(jet,factorRegression);
      if (m_debug) {
	TLorentzVector jRegression=j*factorRegression;
        std::cout<<"CORR Nominal   jet:"<<std::endl;
        j.Print();
        double isBTag=Props::MV2c10.get(jet)>-0.8244;
        std::cout<<"ptCorr="<<Props::ptCorr.get(jet)<<" jetPt="<<jet->pt()<<" factorRegression="<<factorRegression<<" isBTag="<<isBTag<<std::endl;
        std::cout<<"CORR Regression jet:"<<std::endl;
        jRegression.Print();
      }
    }//end loop over jets in the container for this variation
  }// end loop over variations
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::matchTruthJet(JetSemileptonic* m_jetSemileptonic, const xAOD::JetContainer* truthjets, const std::string& name, double DRCut) {
  if(m_debug) { std::cout<<"JetHandler::matchTruthJet m_matchTruthJet="<<m_matchTruthJet<<std::endl; }
  if (!m_matchTruthJet) { return EL::StatusCode::SUCCESS;}
  // loop over all jet variations
  for(auto& x: *getInParticles()) { //std::map<TString, partContainer*>* getInParticles();
    //const TString& variation=x.first;
    if(m_debug) { std::cout<<"JetHandler::matchTruthJet variation="<<x.first<<std::endl; }
    //loop over the jets in the container belonging to this variation
    for (xAOD::Jet * jet : *x.second) {
      if(m_debug) {
        std::cout<<"JetHandler::matchTruthJet xAOD::Jet jet"
          <<" pt="<<jet->pt()<<" eta="<<jet->eta()<<" phi="<<jet->phi()<<" e="<<jet->e()<<" m="<<jet->m()
          <<std::endl;
      }
      unsigned counter=0;
      unsigned index_smallestDR=99;
      double smallestDR=99.9;
      const xAOD::Jet* truthjetbest=nullptr;
      //loop over all truth jets to do the dR matching
      for (unsigned i=0; i!=truthjets->size(); i++) {
        const xAOD::Jet* truthjet = (*truthjets)[i];
        double DR=jet->p4().DeltaR(truthjet->p4());
        if(!(DR<DRCut)) continue;
        if(m_debug) {
          std::cout<<"JetHandler::matchTruthJet xAOD::Jet truth jet"
            <<" pt="<<truthjet->pt()<<" eta="<<truthjet->eta()<<" phi="<<truthjet->phi()<<" e="<<truthjet->e()<<" m="<<truthjet->m()
            <<" DR="<<DR
            <<std::endl;
        }
        counter++;
        if(DR<smallestDR) {
          index_smallestDR=i;
          smallestDR=DR;
        }
      }//end loop over truth jets
      if(m_debug) {
        std::cout<<"JetHandler::matchTruthJet"<<" smallestDR="<<smallestDR<<" index_smalllestDR="<<index_smallestDR<<" counter="<<counter<<std::endl;
      }
      TLorentzVector truthjettlv;
      if (counter>0) {
        //we will set the real value of truthjettlv
        truthjetbest = (*truthjets)[index_smallestDR];
        if(m_debug) {
          std::cout<<"JetHandler::matchTruthJet truthjetbest"
		   <<" pt="<<truthjetbest->pt()<<" eta="<<truthjetbest->eta()<<" phi="<<truthjetbest->phi()<<" e="<<truthjetbest->e()<<" m="<<truthjetbest->m()
		   <<std::endl;
        }
        truthjettlv.SetPtEtaPhiE(truthjetbest->pt(),truthjetbest->eta(),truthjetbest->phi(),truthjetbest->e());

      }//end if counter>0
      //if not it remains with its dummy value, and Props::TruthPartonLabelID is done in the same way
      if(m_debug) {
	std::cout<<"JetHandler::matchTruthJet truthjettlv"
		 <<" pt="<<truthjettlv.Pt()<<" eta="<<truthjettlv.Eta()<<" phi="<<truthjettlv.Phi()<<" e="<<truthjettlv.E()<<" m="<<truthjettlv.M()
		 <<std::endl;
	truthjettlv.Print();
      }
      if(m_debug) {
	std::cout<<"JetHandler::matchTruthJet m_jetSemileptonic="<<m_jetSemileptonic<<" jet="<<m_jetSemileptonic<<std::endl;
      }
      EL_CHECK("JetHandler::matchTruthJet",m_jetSemileptonic->setJetTLV(jet,name,truthjettlv));

    }//end loop over jets in the container for this variation
  }// end loop over variations
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::IdentifyVBFJets(xAOD::JetContainer * jets, const xAOD::TruthParticleContainer* particles){
  std::vector<const xAOD::TruthParticle*> q_vbf;
  if(m_isMC){

    for (const xAOD::TruthParticle*particle : *particles){
      if (particle->parent(0))
        if(particle->status()==23 && abs(particle->pdgId())<5 && fabs((particle->parent(0))->status())==21) q_vbf.push_back(particle);
    }

  }
  xAOD::Jet*jet1_vbf=0;
  xAOD::Jet*jet2_vbf=0;
  float dR1=999.;
  float dR2=999.;
  for(xAOD::Jet*jet : *jets){
     if(q_vbf.size()<2) Props::isVBFTruth.set(jet,0);
     else if(q_vbf.at(0)->p4().DeltaR(jet->p4())<0.4 && q_vbf.at(0)->p4().DeltaR(jet->p4())<dR1){
       dR1= q_vbf.at(0)->p4().DeltaR(jet->p4());
       if(jet1_vbf) Props::isVBFTruth.set(jet1_vbf,0);
       jet1_vbf=jet;
     }
     else if(q_vbf.at(1)->p4().DeltaR(jet->p4())<0.4 && q_vbf.at(1)->p4().DeltaR(jet->p4())<dR2){
       dR1= q_vbf.at(1)->p4().DeltaR(jet->p4());
       if(jet2_vbf) Props::isVBFTruth.set(jet2_vbf,0);
       jet2_vbf=jet;
     }
     else Props::isVBFTruth.set(jet,0);
  }

  if(jet1_vbf) Props::isVBFTruth.set(jet1_vbf,1);
  if(jet2_vbf) Props::isVBFTruth.set(jet2_vbf,1);


  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::calibrateCopies(xAOD::JetContainer * particles, const CP::SystematicSet & sysSet)
{

  std::string sysName = sysSet.name();
  m_sysName = sysSet.name();//in order to use TString::BeginsWith() in decorate() function.
  if(m_sysName=="") m_sysName = "Nominal";

  std::string jesKey;

  if (m_jes_prefixes.find(sysName) != m_jes_prefixes.end()) {
    // sysName consistent with the new names of JES uncertainties
    jesKey = m_jes_prefixes.at(sysName);
    CP_CHECK("JetHandler::calibrateCopies()",m_jesProviders.at(jesKey)->applySystematicVariation(m_jes_sysSets.at(sysName)),m_debug);
    // CP_CHECK("JetHandler::calibrateCopies()",m_jetResolutionSmearing->applySystematicVariation(m_jes_sysSets.at(sysName)),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_jvtCalib->applySystematicVariation(m_jes_sysSets.at(sysName)),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_fjvtCalib->applySystematicVariation(m_jes_sysSets.at(sysName)),m_debug);
    //CP_CHECK("JetHandler::calibrateCopies()",m_qgtagger->applySystematicVariation(m_jes_sysSets.at(sysName)),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_trackFakeTool->applySystematicVariation(m_jes_sysSets.at(sysName)),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_jetTrackFilterTool->applySystematicVariation(m_jes_sysSets.at(sysName)),m_debug);
  } else {
    // A few things to note:
    // 1. Other systematics or nominal case will be considered here
    // 2. In cases where jesPrefixes and jesConfigs are empty or have been cleared, if the "official"/original names of
    //    JES uncertainties are present in the "variations" vector and also correspond to the JES config file being used,
    //    then the variations will be applied here. For example, JET_GroupedNP_1 in SR_Scenario1.
    // 3. A slightly tricky thing could also occur, which is that if, say, the "begin" of the unordered_map corresponds to
    //    SR_Scenario1 and the user also provided the orignal names (e.g. JET_GroupedNP_1) of the scale uncertainties, then
    //    those will be treated here as well...

    jesKey = m_jesProviders.begin()->first;
    CP_CHECK("JetHandler::calibrateCopies()",m_jesProviders.at(jesKey)->applySystematicVariation(sysSet),m_debug);
    //CP_CHECK("JetHandler::calibrateCopies()",m_jetResolutionSmearing->applySystematicVariation(sysSet),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_jvtCalib->applySystematicVariation(sysSet),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_fjvtCalib->applySystematicVariation(sysSet),m_debug);
    //CP_CHECK("JetHandler::calibrateCopies()",m_qgtagger->applySystematicVariation(sysSet),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_trackFakeTool->applySystematicVariation(sysSet),m_debug);
    CP_CHECK("JetHandler::calibrateCopies()",m_jetTrackFilterTool->applySystematicVariation(sysSet),m_debug);
  }

  // CP_CHECK("JetHandler::calibrateCopies()",m_jetResolutionSmearing->applySystematicVariation(sysSet),m_debug);
  // CP_CHECK("JetHandler::calibrateCopies()",m_jesProvider->applySystematicVariation(sysSet),m_debug);
  // CP_CHECK("JetHandler::calibrateCopies()",m_jvtCalib->applySystematicVariation(sysSet),m_debug);

  // loop over the particles in the container
  for (xAOD::Jet * jet : *particles) {
    // JER treatment: Only apply for MC, the tool does not know about it yet
    if(m_isMC){
      // decorate jet with IsBjet to get correct treatment by tool
      // (only needed for 'globally reduced set' of JES uncertainties (19 NPs),
      // but let's just decorate in any case instead of introducing a conditional
      // statement to check which scenario we're in...)
      // NM 17-02-11: Jet/Etmiss recommends c-jets be treated like b-jets
      static SG::AuxElement::Accessor<char> accIsBjet( "IsBjet" );
      accIsBjet(*jet) = ((Props::HadronConeExclTruthLabelID.get(jet) == 5) || (Props::HadronConeExclTruthLabelID.get(jet) == 4));
//      CP_CHECK("JetHandler::calibrateCopies()",m_jetResolutionSmearing->applyCorrection(*jet),m_debug);
      CP_CHECK("JetHandler::calibrateCopies()",m_jesProviders.at(jesKey)->applyCorrection(*jet),m_debug);
      //CP_CHECK("JetHandler::calibrateCopies()",m_jesProvider->applyCorrection(*jet),m_debug);
    }

    // decorate jet
    if ( decorate( jet ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;

  }

  // loop over the particles in the container
  for (xAOD::Jet * jet : *particles)
    // decorate jet with FJVT
    if ( decorateFJVT( jet ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode JetHandler::retrieveBTaggingDiscriminants(xAOD::Jet * part)
{
  const xAOD::BTagging * tagInfo = part->btagging();

  //double discriminant_mv2c00=0;
  double discriminant_mv2c10=0;
  //double discriminant_mv2c20=0;
  //double discriminant_mv2c100=0;
  //double discriminant_mv2cl100=0;

  //double discriminant_mv2c10mu=0;
  //double discriminant_mv2c10rnn=0;
 
  double jet_dl1_pb=0;
  double jet_dl1_pc=0;
  double jet_dl1_pu=0;
  //double jet_dl1mu_pb=0;
  //double jet_dl1mu_pc=0;
  //double jet_dl1mu_pu=0;
  //double jet_dl1rnn_pb=0;
  //double jet_dl1rnn_pc=0;
  //double jet_dl1rnn_pu=0;

  double jet_dl1r_pb=0;
  double jet_dl1r_pc=0;
  double jet_dl1r_pu=0;

  if(tagInfo) {
    //tagInfo->MVx_discriminant("MV2c00",discriminant_mv2c00);
    tagInfo->MVx_discriminant("MV2c10",discriminant_mv2c10);
    //tagInfo->MVx_discriminant("MV2c20",discriminant_mv2c20);
    //tagInfo->MVx_discriminant("MV2c100",discriminant_mv2c100);
    //tagInfo->MVx_discriminant("MV2cl100",discriminant_mv2cl100);

    //try {
    //  tagInfo->MVx_discriminant("MV2c10mu",discriminant_mv2c10mu);
    //  tagInfo->MVx_discriminant("MV2c10rnn",discriminant_mv2c10rnn);
    //} catch(...) {}

    // DL1
    try {
      tagInfo->pb("DL1",jet_dl1_pb);
      tagInfo->pc("DL1",jet_dl1_pc);
      tagInfo->pu("DL1",jet_dl1_pu);
    } catch(...) {}
    //try {
    //  tagInfo->pb("DL1mu",jet_dl1mu_pb);
    //  tagInfo->pc("DL1mu",jet_dl1mu_pc);
    //  tagInfo->pu("DL1mu",jet_dl1mu_pu);
    //} catch(...) {}
    //try {
    //  tagInfo->pb("DL1rnn",jet_dl1rnn_pb);
    //  tagInfo->pc("DL1rnn",jet_dl1rnn_pc);
    //  tagInfo->pu("DL1rnn",jet_dl1rnn_pu);
    //} catch(...) {}

    //DL1r
    try {
      tagInfo->pb("DL1r",jet_dl1r_pb);
      tagInfo->pc("DL1r",jet_dl1r_pc);
      tagInfo->pu("DL1r",jet_dl1r_pu);
    } catch(...) {}
  }
  else{
    Error("JetHandler::retrieveBTaggingDiscriminants","Couldn't retrieve b-tag info!");
    return EL::StatusCode::FAILURE;
  }

  //Props::MV2c00.set(part, discriminant_mv2c00);
  Props::MV2c10.set(part, discriminant_mv2c10);
  //Props::MV2c20.set(part, discriminant_mv2c20);
  //Props::MV2c100.set(part, discriminant_mv2c100);
  //Props::MV2cl100.set(part, discriminant_mv2cl100);
  //Props::MV2c10mu.set(part, discriminant_mv2c10mu);
  //Props::MV2c10rnn.set(part, discriminant_mv2c10rnn);

  Props::DL1_pb.set(part, jet_dl1_pb);
  Props::DL1_pc.set(part, jet_dl1_pc);
  Props::DL1_pu.set(part, jet_dl1_pu);
  //Props::DL1mu_pb.set(part, jet_dl1mu_pb);
  //Props::DL1mu_pc.set(part, jet_dl1mu_pc);
  //Props::DL1mu_pu.set(part, jet_dl1mu_pu);
  //Props::DL1rnn_pb.set(part, jet_dl1rnn_pb);
  //Props::DL1rnn_pc.set(part, jet_dl1rnn_pc);
  //Props::DL1rnn_pu.set(part, jet_dl1rnn_pu);

  Props::DL1r_pb.set(part, jet_dl1r_pb);
  Props::DL1r_pc.set(part, jet_dl1r_pc);
  Props::DL1r_pu.set(part, jet_dl1r_pu);

  return EL::StatusCode::SUCCESS;

}

void JetHandler::BJetTriggerMatch(xAOD::Jet * jet){
  // get trigger list from config file
  const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
  std::string trig_name;


  for (auto& trig : m_triggersToMatch) {
	 if(trigDecorators.count(trig.first)) {
		trig_name = trig.first;

		bool is_matched = false;

		if (jet->pt()>20000 && fabs(jet->eta())<2.5){
		   Trig::FeatureContainer fc = m_trigDecTool->features(trig_name);
		   Trig::FeatureContainer::combination_const_iterator comb   (fc.getCombinations().begin());
		   Trig::FeatureContainer::combination_const_iterator combEnd(fc.getCombinations().end());
		   double eta1 = jet->eta();
		   double phi1 = jet->phi();
		   double deltaR = 99;

		   for( ; comb!=combEnd ; ++comb) {
			  std::string jet_name = "EFJet";
                          if (trig_name.find("split")!=std::string::npos) jet_name = "SplitJet";
			  std::vector< Trig::Feature<xAOD::JetContainer> >            jetCollections  = comb->containerFeature<xAOD::JetContainer>(jet_name);
			  std::vector< Trig::Feature<xAOD::BTaggingContainer> >       bjetCollections = comb->containerFeature<xAOD::BTaggingContainer>("HLTBjetFex");


                          // @Bo, currently, there is some issue of retrieving HLT bjet  and jet container for mc16e and data18 in HIGG5D3, the size is not match. Comment out temporarily 
                          //https://its.cern.ch/jira/browse/ATR-19385
                          // if(jetCollections.size() != bjetCollections.size()){
                                 // Error("JetHandler::BJetTriggerMatch()","Problem in container size: HLT bjet and HLT btag object size does not match. Skip b-jet trigger matching.");
                                 // continue;
                          // }

                          // @Bo, looping over online bjet and get link to corresponding online jets
			  for(auto  jcont : bjetCollections) {
				 if(is_matched) break;
				 for (const xAOD::BTagging*  hlt_bjet_btag : *jcont.cptr()) {


                                   const auto &jetLinks = hlt_bjet_btag->auxdataConst< std::vector< ElementLink< xAOD::IParticleContainer> > >("BTagBtagToJetAssociator");
                                   
                                   // for(unsigned int ijet=0; ijet<jetLink.size(); ijet++)
                                   for(auto jetlink : jetLinks )
                                   {

                                     const xAOD::IParticle *hlt_bjet=*jetlink;

                                     if(hlt_bjet)
                                     {
                                       double eta2 = hlt_bjet->eta();
                                       double phi2 = hlt_bjet->phi();
                                       double deta = fabs(eta1 - eta2);
                                       double dphi = fabs(phi1 - phi2) < TMath::Pi() ? fabs(phi1 - phi2) : 2*TMath::Pi() - fabs(phi1 - phi2);
                                       deltaR = sqrt(deta*deta + dphi*dphi);
                                       if(deltaR<0.4){
                                         is_matched = true;
                                         break;
                                       }
                                     }
                                   }
				 }
			  } // loop over bjet features




			  // for(auto  jcont : jetCollections) {
			  //        if(is_matched) break;
			  //        for (const xAOD::Jet*  hlt_bjet : *jcont.cptr()) {
                          //
                          //               const xAOD::BTagging *hltbtag=hlt_bjet->btagging();
                          //
                          //               double hlt_bscore_mv2c20=-999;
                          //               double hlt_bscore_mv2c10=-999;
                          //               if(hltbtag) {
                          //                 hltbtag->MVx_discriminant("MV2c20",hlt_bscore_mv2c20);
                          //                 hltbtag->MVx_discriminant("MV2c10",hlt_bscore_mv2c10);
                          //               }
                          //
                          //               // std::cout<<"HLT Btagging: MV2c10 --> " << hlt_bscore_mv2c10 << std::endl;
                          //               // std::cout<<"HLT Btagging: MV2c20 --> " << hlt_bscore_mv2c20 << std::endl;
                          //
                          //
			  //               double eta2 = hlt_bjet->eta();
			  //               double phi2 = hlt_bjet->phi();
			  //               double deta = fabs(eta1 - eta2);
			  //               double dphi = fabs(phi1 - phi2) < TMath::Pi() ? fabs(phi1 - phi2) : 2*TMath::Pi() - fabs(phi1 - phi2);
			  //               deltaR = sqrt(deta*deta + dphi*dphi);
			  //               if(deltaR<0.4){
			  //                 is_matched = true;
			  //                 break;
			  //               }
			  //        }
			  // } // loop over bjet features
                          //

		   } // loop over combinations
		} // central jets

		trig.second->set(jet, is_matched);

	 }
  } // loop over trigger list


}

EL::StatusCode JetHandler::writeOutputVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool isKinVar, bool isWeightVar, const TString& sysName)
{

  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    EL_CHECK("JetHandler::writeOutputVariables",writeAllVariations(inJet, outJet, sysName)); // turned off because of qgtagger issue.
  }
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    EL_CHECK("JetHandler::writeOutputVariables",writeKinematicVariations(inJet, outJet, sysName));
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes
    EL_CHECK("JetHandler::writeOutputVariables",writeWeightVariations(inJet, outJet, sysName));
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited
    EL_CHECK("JetHandler::writeOutputVariables",writeAllVariations(inJet, outJet, sysName));  // turned off because of qgtagger issue.
    EL_CHECK("JetHandler::writeOutputVariables",writeKinematicVariations(inJet, outJet, sysName));
    EL_CHECK("JetHandler::writeOutputVariables",writeWeightVariations(inJet, outJet, sysName));
    EL_CHECK("JetHandler::writeOutputVariables",writeNominal(inJet, outJet, sysName));
  } else assert(false);

  return writeCustomVariables(inJet, outJet, isKinVar, isWeightVar, sysName);
}

EL::StatusCode JetHandler::writeAllVariations(xAOD::Jet* /*inJet*/, xAOD::Jet* /*outJet*/, const TString& /*sysName*/)
 {
   if(m_saveNonCrucialVariables){
   }

   return EL::StatusCode::SUCCESS;
 }

EL::StatusCode JetHandler::writeKinematicVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName)
{

  //'if' block below prevents increasing CxAOD size by restricting effect of TRK variations.
  // Please leave at the beginning of writeKinematicVariations()
  bool isTRKVar = (sysName == "Nominal" || sysName.BeginsWith("TRK_"));
  if(isTRKVar){
    //line below makes sure, TRK variations are only applied on track multiplicity (as intended by MonoHbb analysis).
    if(sysName.BeginsWith("TRK_")) return EL::StatusCode::SUCCESS;
  }

  // set four momentum -> only pt/m are affected by systs
  Props::pt.copy(inJet, outJet);
  Props::m.copy(inJet, outJet);

  bool isJERVar = (sysName == "Nominal" || sysName.BeginsWith("JET_JER_"));
  if(m_doResolution && isJERVar){
    Props::resolution.copy(inJet, outJet);
  }

  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::writeWeightVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString&)
{
  if(m_saveNonCrucialVariables){
    Props::JvtSF.copy(inJet, outJet);
    Props::FJvtSF.copy(inJet, outJet);
  }
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::writeNominal(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& /*sysName*/)
{

  //QG Tagger variables only stored for nominal
  if(m_qgtagging){
    Props::QGTagger.copy(inJet, outJet);
    Props::QGTaggerWeight.copy(inJet, outJet);
    Props::QGTag_result.copy(inJet, outJet);
  }

  // set four momentum -> eta/phi are not affected by systs
  Props::eta.copy(inJet, outJet);
  Props::phi.copy(inJet, outJet);

  // jvt/goodJet
  //Detector Eta (same as EMScaleEta) needed for Jvt recalculation by hand for Reader in tag 20 CxAOD when the PassJvtMedium was not yet stored
  //Props::DetectorEta.copy(inJet, outJet);
  Props::goodJet.copy(inJet, outJet);
  if (!m_isPFlow) {
      Props::DFCommonJets_jetClean_LooseBad.copyIfExists(inJet, outJet);
      Props::DFCommonJets_jetClean_TightBad.copyIfExists(inJet, outJet); // request by DBL group
      Props::Timing.copyIfExists(inJet, outJet); // request by DBL group
  }

  // jvt
  Props::PassJvtMedium.copy(inJet, outJet);
  Props::Jvt.copy(inJet, outJet);
  if(m_saveNonCrucialVariables){
    Props::PassFJvtTight.copy(inJet, outJet);
    Props::PassFJvtLoose.copy(inJet, outJet);
    Props::FJvt.copy(inJet, outJet);
  }

  //
  if(m_isMC) {
    Props::HadronConeExclTruthLabelID.copyIfExists(inJet, outJet);
    Props::TruthPartonLabelID.copyIfExists(inJet,outJet);
    Props::isVBFTruth.copyIfExists(inJet, outJet);
  }

  // b-tagging
  Props::MV2c10.copy(inJet, outJet);
  Props::DL1_pb.copyIfExists(inJet, outJet);
  Props::DL1_pc.copyIfExists(inJet, outJet);
  Props::DL1_pu.copyIfExists(inJet, outJet);
  Props::DL1r_pb.copyIfExists(inJet, outJet);
  Props::DL1r_pc.copyIfExists(inJet, outJet);
  Props::DL1r_pu.copyIfExists(inJet, outJet);    

  if(m_saveNonCrucialVariables){
    //Props::MV2c00.copy(inJet, outJet);
    //Props::MV2c20.copy(inJet, outJet);
    //Props::MV2c100.copy(inJet, outJet);
    //Props::MV2cl100.copy(inJet, outJet);
    //Props::MV2c10mu.copyIfExists(inJet, outJet);
    //Props::MV2c10rnn.copyIfExists(inJet, outJet);

    //Props::DL1mu_pb.copyIfExists(inJet, outJet);
    //Props::DL1mu_pc.copyIfExists(inJet, outJet);
    //Props::DL1mu_pu.copyIfExists(inJet, outJet);
    //Props::DL1rnn_pb.copyIfExists(inJet, outJet);
    //Props::DL1rnn_pc.copyIfExists(inJet, outJet);
    //Props::DL1rnn_pu.copyIfExists(inJet, outJet);
    
    //Props::isBJet.copyIfExists(inJet, outJet);
  }

  // jet shape related variables
  if(m_saveNonCrucialVariables){
    //Props::TrackWidthPt1000PV.copy(inJet, outJet);
    //Props::TrackWidthPt500PV.copy(inJet, outJet);
    //Props::SumPtTrkPt500PV.copy(inJet, outJet);
    //Props::ConstPx.copy(inJet, outJet);
    //Props::ConstPy.copy(inJet, outJet);
    //Props::ConstPz.copy(inJet, outJet);
    //Props::Width.copy(inJet, outJet);
  }

  if(m_saveqgvariables){
    Props::Width.copy(inJet, outJet);
    Props::TrackWidthPt500PV.copy(inJet, outJet);
    Props::NumTrkPt500PV.copy(inJet, outJet);
    Props::NumTrkPt1000PV.copy(inJet, outJet);
    Props::DFCommonJets_QGTagger_NTracks.copyIfExists(inJet,outJet);
    Props::DFCommonJets_QGTagger_TracksWidth.copyIfExists(inJet,outJet);
    Props::DFCommonJets_QGTagger_TracksC1.copyIfExists(inJet,outJet);
  }


  // regression info
  Props::factorRegression.copyIfExists(inJet,outJet);
  if(m_writeRegVars){
    //Props::RegInputEmscalePt.copyIfExists(inJet, outJet);
    //Props::RegInputNTrks.copyIfExists(inJet, outJet);
    //Props::RegInputLeadTrkPt.copyIfExists(inJet, outJet);
    Props::RegInputSumTrkPt.copyIfExists(inJet, outJet);
    //Props::RegInputEtaWidthTrks.copyIfExists(inJet, outJet);
    //Props::RegInputPhiWidthTrks.copyIfExists(inJet, outJet);
    Props::RegInputMuInJetPt.copyIfExists(inJet, outJet);
    Props::RegInputSumPtLeps.copyIfExists(inJet, outJet);
    //Props::RegInputElInJetPt.copyIfExists(inJet, outJet);
    //Props::RegInputDRJetLepInJet.copyIfExists(inJet, outJet);
    Props::RegInputSumPtCloseByJets.copyIfExists(inJet, outJet);
    Props::RegInputSecVtxMass.copyIfExists(inJet, outJet);
    Props::RegInputSecVtxNormDist.copyIfExists(inJet, outJet);
  }

  // electron-in-jet info
  if(m_applyJetSemileptonic && m_countElectronInJet) {
    Props::nrElectronInJet.copy(inJet, outJet);
    if(m_saveSemileptonicInfoInJet){
      //Props::PtRatioOneElectronInJet.copy(inJet, outJet);
    }
  }

  // muon-in-jet info
  if(m_applyJetSemileptonic && m_correctMuonInJet) {
    Props::nrMuonInJet.copy(inJet, outJet);
    if(m_saveSemileptonicInfoInJet){
      //Props::PtRatioOneMuonInJet.copy(inJet, outJet);
    }
    setP4( inJet , "OneMu", outJet );
    Props::factorPtReco.copy(inJet,outJet);
  }

  // truth jets as signal states
  if(m_isMC && m_matchTruthJet) {
    //setP4( inJet , "Truth", outJet );
    setP4( inJet , "TruthWZ", outJet );
  }

  // copy first nTrk for OR
  //std::vector<int> nTrkVecIn;
  //inJet->getAttribute(xAOD::JetAttribute::NumTrkPt500, nTrkVecIn);
  //std::vector<int> nTrkVecOut;
  //nTrkVecOut.push_back(nTrkVecIn.at(0));
  //outJet->setAttribute(xAOD::JetAttribute::NumTrkPt500, nTrkVecOut);

  // jetEtMiss base Parton jet labelling
  Props::PartonTruthLabelID.copyIfExists(inJet, outJet);

  // ghost associated hadron labels
  if ( m_isMC && m_storeGAParticlesInJets ) {
    const std::string labelGhostB  = "GhostBHadronsFinal";
    const std::string labelGhostC  = "GhostCHadronsFinal";
    const std::string labelConeB   = "ConeExclBHadronsFinal";
    const std::string labelConeC   = "ConeExclCHadronsFinal";
    const std::string labelTau = "GhostTausFinal";
    const std::string labelH   = "GhostHBosons";
    std::vector<const xAOD::TruthParticle *> ghostB;
    std::vector<const xAOD::TruthParticle *> ghostC;
    std::vector<const xAOD::TruthParticle *> coneB;
    std::vector<const xAOD::TruthParticle *> coneC;
    std::vector<const xAOD::TruthParticle *> ghostTau;
    std::vector<const xAOD::TruthParticle *> ghostH;
    // get input vectors from ghost-associated parent jet element link
    if ( ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelGhostB, ghostB   ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelGhostC, ghostC   ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelConeB , coneB   ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelConeC , coneC   ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelTau , ghostTau ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelH ,   ghostH ) )
    {
      Error("JetHandler::writeNominal()","Couldn't access ghost associated heavy flavour hadrons! (You can turn off storing these hadrons via 'storeGAParticlesInJets = false' in the config file)");
      return EL::StatusCode::FAILURE;
    }
    // make sure all IParticle pointers are valid (i.e. remove invalid pointers)
    // (are the invalid pointers a bug?)
    for (const xAOD::TruthParticle * part : ghostB  ) if ( ! part ) ghostB.erase( std::remove( std::begin(ghostB), std::end(ghostB), part ), std::end(ghostB) );
    for (const xAOD::TruthParticle * part : ghostC  ) if ( ! part ) ghostC.erase( std::remove( std::begin(ghostC), std::end(ghostC), part ), std::end(ghostC) );
    for (const xAOD::TruthParticle * part : coneB  ) if ( ! part ) coneB.erase( std::remove( std::begin(coneB), std::end(coneB), part ), std::end(coneB) );
    for (const xAOD::TruthParticle * part : coneC  ) if ( ! part ) coneC.erase( std::remove( std::begin(coneC), std::end(coneC), part ), std::end(coneC) );
    for (const xAOD::TruthParticle * part : ghostTau) if ( ! part ) ghostTau.erase( std::remove( std::begin(ghostTau), std::end(ghostTau), part ), std::end(ghostTau) );
    for (const xAOD::TruthParticle * part : ghostH  ) if ( ! part ) ghostH.erase( std::remove( std::begin(ghostH), std::end(ghostH), part ), std::end(ghostH) );

    // set output vectors
    // element links are broken in output; see JIRA CXAOD-211; removing for the moment
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelB   , ghostB   );
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelC   , ghostC   );
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelTau , ghostTau );
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelH   , ghostH   );

    // count number of B- and C-Hadrons, excluding non-prompt decays
    int nGhostMatchedBHadrons = ghostB.size();
    int nConeMatchedBHadrons = coneB.size();
    int nGhostMatchedCHadrons = ghostC.size();
    // only counting prompt C-Hadrons
    int nGhostMatchedPromptCHadrons = 0,  nConeMatchedPromptCHadrons =0;
    for (const xAOD::TruthParticle * part : ghostC ) {
      if (!IsChild(part, ghostB)) nGhostMatchedPromptCHadrons++;
    }
    for (const xAOD::TruthParticle * part : coneC ) {
      if (!IsChild(part, coneB)) nConeMatchedPromptCHadrons++;
    }
    int nGhostMatchedTaus = ghostTau.size();
    int nGhostMatchedHBosons = ghostH.size();

    Props::nGhostMatchedBHadrons.set(outJet, nGhostMatchedBHadrons);
    Props::nGhostMatchedCHadrons.set(outJet, nGhostMatchedCHadrons);
    Props::nGhostMatchedPromptCHadrons.set(outJet, nGhostMatchedPromptCHadrons);
    Props::nConeMatchedBHadrons.set(outJet, nConeMatchedBHadrons);
    Props::nConeMatchedPromptCHadrons.set(outJet, nConeMatchedPromptCHadrons);
    Props::nGhostMatchedTaus.set(outJet, nGhostMatchedTaus);
    Props::nGhostMatchedHBosons.set(outJet, nGhostMatchedHBosons);

  }

  // b-jet trigger matching
  const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
  for(auto& trig : m_triggersToMatch) {
	 if(trigDecorators.count(trig.first)) {
		trig.second->copy(inJet, outJet);
	 }
  }

  //
  return EL::StatusCode::SUCCESS;
}

bool JetHandler::IsChild(const xAOD::TruthParticle* child, std::vector<const xAOD::TruthParticle *> parents, std::vector<const xAOD::TruthParticle*> ancestors) {
  // child:     particle to be checked; starting point
  // parents:   list of parent particles
  // ancestors: lineage, starting from child, to avoid loops in some W+b samples
  // TODO:      move to CxAODTools / look if something like this exists in RootCore truth utils
  if (parents.size() == 0) return false;
  if (std::count(ancestors.begin(), ancestors.end(), child) > 0) return false;
  if (std::find(parents.begin(), parents.end(), child) != parents.end()) return true;
  ancestors.push_back(child);
  if (!child) return false;
  for (unsigned int j = 0; j < child->nParents(); j++) {
    if (IsChild(child->parent(j), parents, ancestors)) return true;
  }
  return false;
}

EL::StatusCode JetHandler::writeCustomVariables(xAOD::Jet*, xAOD::Jet*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::clearEvent() {

  //TStore::clear() needed if rho was computed for jet calibration
  //could this interfere with other Handlers?
  if( ! m_eventInfoHandler.get_isDerivation()  )  xAOD::TActiveStore::store()->clear();

  EL_CHECK("JetHandler::clearEvent()",ObjectHandler::clearEvent());
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::getJVTeventSF(const CP::SystematicSet& sysSet, float& JVTeventSF) {
  const xAOD::JetContainer* recoJets = getInParticleVariation("Nominal");
  if(m_debug) { std::cout<<"JetHandler::getJVTeventSF for a collection of "<<recoJets->size()<<" size."<<std::endl;}
  if(m_jvtCalib->sysApplySystematicVariation(sysSet) != CP::SystematicCode::Ok) {
    Error("JetHandler::getJVTeventSF", "m_jvtCalib->sysApplySystematicVariation(systSet) != CP::SystematicCode::Ok Exiting.");
  }
  if( m_jvtCalib->applyAllEfficiencyScaleFactor(recoJets,JVTeventSF) != CP::CorrectionCode::Ok) {
    Error("JetHandler::getJVTeventSF", "m_jvtCalib->applyAllEfficiencyScaleFactor(recoJets,sf) != CP::CorrectionCode::Ok Exiting.");
    return EL::StatusCode::FAILURE;
  }//end compute JvtSF for the current systematic uncertainty (variation)
  if(m_debug) {std::cout<<"JetHandler::getJVTeventSF systematic variation="<<sysSet.begin()->name()<<" JVTeventSF="<<JVTeventSF<<std::endl;}
  //return
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetHandler::decorateOriginParticles(xAOD::JetContainer * jets)
{
  for (xAOD::Jet * jet : *jets) {
    CP_CHECK("JetHandler::decorateOriginParticles()",m_jetCalibration->applyCorrection(*jet),m_debug);
    float jvt = m_JVTUpdateTool_handle->updateJvt(*jet);
    Props::Jvt.set(jet, jvt);
  }

  if(m_doFJVT) m_fjvtTool->modify(*jets);

  const xAOD::JetContainer *truthJets = nullptr;
  if(m_isMC) {
    CP_CHECK("JetHandler::decorateOriginParticles()",m_event->retrieve(truthJets, "AntiKt4TruthJets"),m_debug);
    CP_CHECK("JetHandler::decorateOriginParticles()",m_fjvtCalib->tagTruth(jets,truthJets),m_debug);
  }
  const xAOD::MuonContainer *muons = nullptr;
  CP_CHECK("JetHandler::decorateOriginParticles()",m_event->retrieve(muons, "Muons"),m_debug);
  met::addGhostMuonsToJets(*muons, *jets);

  return EL::StatusCode::SUCCESS;
}
