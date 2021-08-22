#include "EventLoop/StatusCode.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODCore/AuxInfoBase.h"
#include "xAODCore/ShallowAuxInfo.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODTracking/Vertex.h"
#include "xAODEventShape/EventShape.h"

#include "CxAODMaker/OverlapRemover.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "GoodRunsLists/GoodRunsListSelectionTool.h"

#include "CxAODTools/ConfigStore.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODTools/PUReweightingTool.h"
#include "CxAODMaker/TrigObjects.h"
#include "CxAODMaker/JetHandler.h"

#include "xAODTrigMissingET/TrigMissingETContainer.h"
#include "xAODTrigger/EnergySumRoI.h"

#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "TSystem.h"

EventInfoHandler::EventInfoHandler(ConfigStore & config, xAOD::TEvent * event) :
  m_config(config),
  m_debug(false),
  m_msgLevel(MSG::WARNING),
  m_event(event),
  m_eventInfoIn(nullptr),
  m_grlSelectionTool(nullptr),
  m_puReweightingTool(nullptr),
  m_trigConfigTool(nullptr),
  m_trigDecTool(nullptr),
  m_useTrigObj(false),
  m_allowTrigObjFail(true),
  m_METtrigEmul(false),
  m_jetAlgoName("none"),
  m_firstEvent(0),
  m_isMC(-1),
  m_triggerStream(-1),
  m_indexPV(-1),
  m_isDerivation(false),
  m_derivationName(""),
  m_isAFII(false),
  m_pTag(""),
  m_hasNoFJDetectorEta(false),
  m_mcChanNr(-1)
{
  // set jet algorithm name                                                                                                                          
  m_config.getif<std::string>("jetAlgoName",m_jetAlgoName);
  m_isPFlow = (m_jetAlgoName.find("PFlow")!=std::string::npos);
  m_config.getif<bool>("debug", m_debug);
  m_grl.clear();
}


EventInfoHandler::~EventInfoHandler(){
  delete m_grlSelectionTool;
  delete m_puReweightingTool;
  delete m_trigConfigTool;
  delete m_trigDecTool;
  for (TrigObjects * trigObj : m_trigObjects) {
    delete trigObj;
  }
}


EL::StatusCode EventInfoHandler::initialize() {

  //m_isDerivation is set in AnalysisBase.cxx initializeHandlers()
  
  //Derivation Name: required for storing m_trigObjects (breaks without)
  m_config.getif<std::string>("DerivationName", m_derivationName);
  
  // GRL
  //-------
  m_config.getif<std::vector<std::string>>("grl",m_grl);

  // GRL selection tool
  //--------------------
  m_grlSelectionTool = new GoodRunsListSelectionTool("GoodRunsListSelectionTool");
  // TODO retrieve message level from config
  m_grlSelectionTool->msg().setLevel( MSG::WARNING );
  // if true (default) will ignore result of GRL and will just pass all events
  bool passThrough = false;
  std::cout << "Number of GRL Files: " << m_grl.size() << std::endl;
  if(!m_grl.size()){
    passThrough = true;
    TOOL_CHECK("EventInfoHandler::initialize()",m_grlSelectionTool->setProperty("PassThrough", passThrough));
    Warning("EventInfoHandler::initialize()","No GRL is set in config. Set GoodRunsListSelectionTool to pass through mode!");
  }
  else{
    for ( unsigned int i=0; i<m_grl.size(); i++ ){
      std::cout << "The GRL: " << m_grl[i] << " is installed." << std::endl;
    }
    TOOL_CHECK("EventInfoHandler::initialize()",m_grlSelectionTool->setProperty("GoodRunsListVec", m_grl));
  }
  TOOL_CHECK("EventInfoHandler::initialize()",m_grlSelectionTool->initialize());
  
  //pileup reweighting tool
  //---------------------------
  
  m_puReweightingTool = new PUReweightingTool(m_config);
  EL_CHECK("EventInfoHandler::initialize()",m_puReweightingTool->initialize());
  
 
  //trigger tool
  //

  //----------------
  //get list of triggers from config
  std::vector<std::string> triggerList0;
  triggerList0.clear();
  std::string triggerListPath="/data/CxAODMaker/";
  m_config.getif< std::vector<std::string> >("triggerList", triggerList0);
  m_config.getif< std::string >("triggerListPath", triggerListPath);
  m_triggerList.clear();
  // check if txt file or real list
  if(!triggerList0.size()) Warning("EventInfoHandler::initialize()","No trigger defined in config file!");
  else{
    // we face a txt file
    if(triggerList0.size()==1) {
      // get file name
      std::string triggerfile = gSystem->Getenv("WorkDir_DIR");
      triggerfile += triggerListPath;
      triggerfile += triggerList0.at(0);
      Info("EventInfoHandler::initialize()", "Open trigger file: %s",triggerfile.c_str());
      // open the file
      std::ifstream infile;
      infile.open(triggerfile);
      if ( infile.fail() ) {
        Error("EventInfoHandler::initialize()","Trigger list file '%s' not found!", triggerfile.c_str());
        return EL::StatusCode::FAILURE;
      }
      std::string line;
      while (!infile.eof()) {
        getline(infile, line);
        // remove all spaces from line
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        // skip empty lines to avoid crash
        if (line.length() == 0) continue;
        // don't run over commented lines
        if (line.find("#") != std::string::npos) continue;
        // add sample name to vector
        m_triggerList.push_back(line);
      }
      infile.close();
    }
    // we have vector of string in config file
    else {
      m_config.getif< std::vector<std::string> >("triggerList", m_triggerList);
    }
  }
  // get switch for turning on trigger objects (off by default)
  m_config.getif<bool >("useTrigObj", m_useTrigObj);
  // get switch for allowing trigger objects in list to fail processing, e.g. if container is not in input (off by default)
  m_config.getif<bool>("allowTrigObjFail", m_allowTrigObjFail);
  // work on triggers in the list
  if(!m_triggerList.size()) Warning("EventInfoHandler::initialize()","No trigger defined when form back the trigger list!");
  else{
    // setup vector of decorator pointers
    // TODO NM seriously, this is UGLY and SLOOOOOOOOOOOOOOOOOWWWWWWWWWWW
    // not critical because it is at initialization, but still...
    for (const std::string & trigger : m_triggerList) {
      std::cout<<"Trigger used : "<<trigger<<std::endl;
      // trigger flags
      if      ( trigger == "L1_XE60" ) m_trigDecorators["L1_XE60"] = &Props::passL1_XE60;
      else if ( trigger == "HLT_xe80_tc_lcw_L1XE50" ) m_trigDecorators["HLT_xe80_tc_lcw_L1XE50"] = &Props::passHLT_xe80_tc_lcw_L1XE50;
      else if ( trigger == "HLT_xe90_mht_L1XE50" ) m_trigDecorators["HLT_xe90_mht_L1XE50"] = &Props::passHLT_xe90_mht_L1XE50;
      else if ( trigger == "HLT_xe90_tc_lcw_wEFMu_L1XE50" ) m_trigDecorators["HLT_xe90_tc_lcw_wEFMu_L1XE50"] = &Props::passHLT_xe90_tc_lcw_wEFMu_L1XE50;
      else if ( trigger == "HLT_xe90_mht_wEFMu_L1XE50" ) m_trigDecorators["HLT_xe90_mht_wEFMu_L1XE50"] = &Props::passHLT_xe90_mht_wEFMu_L1XE50;
      else if ( trigger == "HLT_xe100_L1XE50" ) m_trigDecorators["HLT_xe100_L1XE50"] = &Props::passHLT_xe100_L1XE50;
      else if ( trigger == "HLT_xe100_tc_em_L1XE50" ) m_trigDecorators["HLT_xe100_tc_em_L1XE50"] = &Props::passHLT_xe100_tc_em_L1XE50;
      else if ( trigger == "HLT_xe120_pueta" ) m_trigDecorators["HLT_xe120_pueta"] = &Props::passHLT_xe120_pueta;
      else if ( trigger == "HLT_xe120_pufit" ) m_trigDecorators["HLT_xe120_pufit"] = &Props::passHLT_xe120_pufit;
      else if ( trigger == "HLT_xe90_tc_lcw_L1XE50" ) m_trigDecorators["HLT_xe90_tc_lcw_L1XE50"] = &Props::passHLT_xe90_tc_lcw_L1XE50 ;
      else if ( trigger == "HLT_xe100_tc_lcw_L1XE50" ) m_trigDecorators["HLT_xe100_tc_lcw_L1XE50"] = &Props::passHLT_xe100_tc_lcw_L1XE50;
      else if ( trigger == "HLT_xe110_tc_lcw_L1XE60" ) m_trigDecorators["HLT_xe110_tc_lcw_L1XE60"] = &Props::passHLT_xe110_tc_lcw_L1XE60;
      else if ( trigger == "HLT_xe80_mht_L1XE50" ) m_trigDecorators["HLT_xe80_mht_L1XE50"] = &Props::passHLT_xe80_mht_L1XE50    ;
      else if ( trigger == "HLT_xe100_mht_L1XE50" ) m_trigDecorators["HLT_xe100_mht_L1XE50"] = &Props::passHLT_xe100_mht_L1XE50   ;
      else if ( trigger == "HLT_xe100_mht_L1XE60" ) m_trigDecorators["HLT_xe100_mht_L1XE60"] = &Props::passHLT_xe100_mht_L1XE60   ;
      else if ( trigger == "HLT_xe110_mht_L1XE50" ) m_trigDecorators["HLT_xe110_mht_L1XE50"] = &Props::passHLT_xe110_mht_L1XE50   ;
      else if ( trigger == "HLT_xe110_mht_L1XE50_AND_xe70_L1XE50" ) m_trigDecorators["HLT_xe110_mht_L1XE50_AND_xe70_L1XE50"] = &Props::passHLT_xe110_mht_L1XE50_AND_xe70_L1XE50   ;
      else if ( trigger == "HLT_xe110_mht_L1XE50_AND_xe70_L1XE55" ) m_trigDecorators["HLT_xe110_mht_L1XE50_AND_xe70_L1XE55"] = &Props::passHLT_xe110_mht_L1XE50_AND_xe70_L1XE55   ;
      else if ( trigger == "HLT_xe130_mht_L1XE50" ) m_trigDecorators["HLT_xe130_mht_L1XE50"] = &Props::passHLT_xe130_mht_L1XE50   ;
      else if ( trigger == "HLT_xe80_L1XE50" ) m_trigDecorators["HLT_xe80_L1XE50"] = &Props::passHLT_xe80_L1XE50        ;
      else if ( trigger == "HLT_xe90_L1XE50" ) m_trigDecorators["HLT_xe90_L1XE50"] = &Props::passHLT_xe90_L1XE50        ;
      else if ( trigger == "HLT_xe110_L1XE60" ) m_trigDecorators["HLT_xe110_L1XE60"] = &Props::passHLT_xe110_L1XE60       ;
      else if ( trigger == "HLT_xe80_tc_em_L1XE50" ) m_trigDecorators["HLT_xe80_tc_em_L1XE50"] = &Props::passHLT_xe80_tc_em_L1XE50  ;
      else if ( trigger == "HLT_xe90_tc_em_L1XE50" ) m_trigDecorators["HLT_xe90_tc_em_L1XE50"] = &Props::passHLT_xe90_tc_em_L1XE50  ;
      else if ( trigger == "L1_XE70" ) m_trigDecorators["L1_XE70"] = &Props::passL1_XE70;
      else if ( trigger == "HLT_xe70" ) m_trigDecorators["HLT_xe70"] = &Props::passHLT_xe70;
      else if ( trigger == "HLT_xe80" ) m_trigDecorators["HLT_xe80"] = &Props::passHLT_xe80;
      else if ( trigger == "HLT_xe100" ) m_trigDecorators["HLT_xe100"] = &Props::passHLT_xe100;
      else if ( trigger == "HLT_xe100_tc_lcw_L1XE60" ) m_trigDecorators["HLT_xe100_tc_lcw_L1XE60"] = &Props::passHLT_xe100_tc_lcw_L1XE60;
      else if ( trigger == "HLT_xe110_tc_em_L1XE50" ) m_trigDecorators["HLT_xe110_tc_em_L1XE50"] = &Props::passHLT_xe110_tc_em_L1XE50;
      else if ( trigger == "HLT_xe110_tc_em_wEFMu_L1XE50" ) m_trigDecorators["HLT_xe110_tc_em_wEFMu_L1XE50"] = &Props::passHLT_xe110_tc_em_wEFMu_L1XE50;
      else if ( trigger == "HLT_xe120_pueta_wEFMu" ) m_trigDecorators["HLT_xe120_pueta_wEFMu"] = &Props::passHLT_xe120_pueta_wEFMu;
      else if ( trigger == "HLT_xe120_mht" ) m_trigDecorators["HLT_xe120_mht"] = &Props::passHLT_xe120_mht;
      else if ( trigger == "HLT_xe120_tc_lcw" ) m_trigDecorators["HLT_xe120_tc_lcw"] = &Props::passHLT_xe120_tc_lcw;
      else if ( trigger == "HLT_xe120_mht_wEFMu" ) m_trigDecorators["HLT_xe120_mht_wEFMu"] = &Props::passHLT_xe120_mht_wEFMu;
      else if ( trigger == "HLT_xe110_L1XE50" ) m_trigDecorators["HLT_xe110_L1XE50"] = &Props::passHLT_xe110_L1XE50;
      else if ( trigger == "HLT_xe100_L1XE60" ) m_trigDecorators["HLT_xe100_L1XE60"] = &Props::passHLT_xe100_L1XE60;
      else if ( trigger == "HLT_xe90_pufit_L1XE50" ) m_trigDecorators["HLT_xe90_pufit_L1XE50"] = &Props::passHLT_xe90_pufit_L1XE50;
      else if ( trigger == "HLT_xe90_pufit_L1XE55" ) m_trigDecorators["HLT_xe90_pufit_L1XE55"] = &Props::passHLT_xe90_pufit_L1XE55;
      else if ( trigger == "HLT_xe100_pufit_L1XE50" ) m_trigDecorators["HLT_xe100_pufit_L1XE50"] = &Props::passHLT_xe100_pufit_L1XE50;
      else if ( trigger == "HLT_xe100_pufit_L1XE55" ) m_trigDecorators["HLT_xe100_pufit_L1XE55"] = &Props::passHLT_xe100_pufit_L1XE55;
      else if ( trigger == "HLT_xe110_pufit_L1XE50" ) m_trigDecorators["HLT_xe110_pufit_L1XE50"] = &Props::passHLT_xe110_pufit_L1XE50;
      else if ( trigger == "HLT_xe110_pufit_L1XE55" ) m_trigDecorators["HLT_xe110_pufit_L1XE55"] = &Props::passHLT_xe110_pufit_L1XE55;
      else if ( trigger == "HLT_xe110_pufit_L1XE60" ) m_trigDecorators["HLT_xe110_pufit_L1XE60"] = &Props::passHLT_xe110_pufit_L1XE60;
      else if ( trigger == "HLT_xe110_pufit_xe65_L1XE50" ) m_trigDecorators["HLT_xe110_pufit_xe65_L1XE50"] = &Props::passHLT_xe110_pufit_xe65_L1XE50;
      else if ( trigger == "HLT_xe110_pufit_xe70_L1XE50" ) m_trigDecorators["HLT_xe110_pufit_xe70_L1XE50"] = &Props::passHLT_xe110_pufit_xe70_L1XE50;
      else if ( trigger == "HLT_xe120_pufit_L1XE50" ) m_trigDecorators["HLT_xe120_pufit_L1XE50"] = &Props::passHLT_xe120_pufit_L1XE50;
      else if ( trigger == "HLT_xe120_pufit_L1XE55" ) m_trigDecorators["HLT_xe120_pufit_L1XE55"] = &Props::passHLT_xe120_pufit_L1XE55;
      else if ( trigger == "HLT_xe120_pufit_L1XE60" ) m_trigDecorators["HLT_xe120_pufit_L1XE60"] = &Props::passHLT_xe120_pufit_L1XE60;
      else if ( trigger == "HLT_xe120_pufit_wEFMu" ) m_trigDecorators["HLT_xe120_pufit_wEFMu"] = &Props::passHLT_xe120_pufit_wEFMu;
      else if ( trigger == "HLT_xe120_tc_lcw_wEFMu" ) m_trigDecorators["HLT_xe120_tc_lcw_wEFMu"] = &Props::passHLT_xe120_tc_lcw_wEFMu;
      else if ( trigger == "HLT_xe120_tc_em" ) m_trigDecorators["HLT_xe120_tc_em"] = &Props::passHLT_xe120_tc_em;
      else if ( trigger == "L1_J40_DPHI-J20XE50" ) m_trigDecorators["L1_J40_DPHI-J20XE50"] = &Props::passL1_J40_DPHI_J20XE50;
      else if ( trigger == "HLT_e15_lhtight_ivarloose_3j20_L1EM13VH_3J20") m_trigDecorators["HLT_e15_lhtight_ivarloose_3j20_L1EM13VH_3J20"] = &Props::passHLT_e15_lhtight_ivarloose_3j20_L1EM13VH_3J20;
      else if (trigger == "HLT_mu14_ivarloose_3j20_L1MU10_3J20") m_trigDecorators["HLT_mu14_ivarloose_3j20_L1MU10_3J20"] = &Props::passHLT_mu14_ivarloose_3j20_L1MU10_3J20;
      
      else if ( trigger == "L1_J40_DPHI-J20s2XE50" ) m_trigDecorators["L1_J40_DPHI-J20s2XE50"] = &Props::passL1_J40_DPHI_J20s2XE50;
      else if ( trigger == "HLT_j80_xe80_dphi1_L1J40_DPHI-J20XE50" ) m_trigDecorators["HLT_j80_xe80_dphi1_L1J40_DPHI-J20XE50"] = &Props::passHLT_j80_xe80_dphi1_L1J40_DPHI_J20XE50;
      else if ( trigger == "HLT_j80_xe80_dphi1_L1J40_DPHI-J20s2XE50" ) m_trigDecorators["HLT_j80_xe80_dphi1_L1J40_DPHI-J20s2XE50"] = &Props::passHLT_j80_xe80_dphi1_L1J40_DPHI_J20s2XE50;
      else if ( trigger == "HLT_j100_xe80_L1J40_DPHI-J20XE50" ) m_trigDecorators["HLT_j100_xe80_L1J40_DPHI-J20XE50"] = &Props::passHLT_j100_xe80_L1J40_DPHI_J20XE50;
      else if ( trigger == "HLT_j100_xe80_L1J40_DPHI-J20s2XE50" ) m_trigDecorators["HLT_j100_xe80_L1J40_DPHI-J20s2XE50"] = &Props::passHLT_j100_xe80_L1J40_DPHI_J20s2XE50;
      else if ( trigger == "L1_EM13VH" ) m_trigDecorators["L1_EM13VH"] = &Props::passL1_EM13VH;
      else if ( trigger == "L1_EM15VH" ) m_trigDecorators["L1_EM15VH"] = &Props::passL1_EM15VH;
      else if ( trigger == "L1_EM18VH" ) m_trigDecorators["L1_EM18VH"] = &Props::passL1_EM18VH;
      else if ( trigger == "L1_EM20VH" ) m_trigDecorators["L1_EM20VH"] = &Props::passL1_EM20VH;
      else if ( trigger == "L1_EM22VHI" ) m_trigDecorators["L1_EM22VHI"] = &Props::passL1_EM22VHI;
      else if ( trigger == "L1_2EM15VH" ) m_trigDecorators["L1_2EM15VH"] = &Props::passL1_2EM15VH;
      else if ( trigger == "HLT_e17_lhloose_L1EM15" ) m_trigDecorators["HLT_e17_lhloose_L1EM15"] = &Props::passHLT_e17_lhloose_L1EM15;
      else if ( trigger == "HLT_e22_lhvloose_nod0_e12_lhvloose_nod0_e10_lhvloose_nod0" ) m_trigDecorators["HLT_e22_lhvloose_nod0_e12_lhvloose_nod0_e10_lhvloose_nod0"] = &Props::passHLT_e22_lhvloose_nod0_e12_lhvloose_nod0_e10_lhvloose_nod0;
      else if ( trigger == "HLT_e24_lhvloose_L1EM18VH" ) m_trigDecorators["HLT_e24_lhvloose_L1EM18VH"] = &Props::passHLT_e24_lhvloose_L1EM18VH;
      else if ( trigger == "HLT_e24_lhvloose_L1EM20VH" ) m_trigDecorators["HLT_e24_lhvloose_L1EM20VH"] = &Props::passHLT_e24_lhvloose_L1EM20VH;
      else if ( trigger == "HLT_e24_lhvloose_nod0_L1EM20VH" ) m_trigDecorators["HLT_e24_lhvloose_nod0_L1EM20VH"] = &Props::passHLT_e24_lhvloose_nod0_L1EM20VH;
      else if ( trigger == "HLT_e24_lhvloose_nod0_2e12_lhvloose_nod0_L1EM20VH_3EM10VH" ) m_trigDecorators["HLT_e24_lhvloose_nod0_2e12_lhvloose_nod0_L1EM20VH_3EM10VH"] = &Props::passHLT_e24_lhvloose_nod0_2e12_lhvloose_nod0_L1EM20VH_3EM10VH;
      else if ( trigger == "HLT_e24_medium_iloose_L1EM18VH" ) m_trigDecorators["HLT_e24_medium_iloose_L1EM18VH"] = &Props::passHLT_e24_medium_iloose_L1EM18VH;
      else if ( trigger == "HLT_e24_medium_iloose_L1EM20VH" ) m_trigDecorators["HLT_e24_medium_iloose_L1EM20VH"] = &Props::passHLT_e24_medium_iloose_L1EM20VH;
      else if ( trigger == "HLT_e24_lhmedium_iloose_L1EM18VH" ) m_trigDecorators["HLT_e24_lhmedium_iloose_L1EM18VH"] = &Props::passHLT_e24_lhmedium_iloose_L1EM18VH;
      else if ( trigger == "HLT_e24_lhmedium_iloose_L1EM20VH" ) m_trigDecorators["HLT_e24_lhmedium_iloose_L1EM20VH"] = &Props::passHLT_e24_lhmedium_iloose_L1EM20VH;

      else if ( trigger == "HLT_e24_lhmedium_L1EM18VH" ) m_trigDecorators["HLT_e24_lhmedium_L1EM18VH"] = &Props::passHLT_e24_lhmedium_L1EM18VH;
      else if ( trigger == "HLT_e24_lhmedium_L1EM20VH" ) m_trigDecorators["HLT_e24_lhmedium_L1EM20VH"] = &Props::passHLT_e24_lhmedium_L1EM20VH;
      else if ( trigger == "HLT_e24_lhtight_iloose" ) m_trigDecorators["HLT_e24_lhtight_iloose"] = &Props::passHLT_e24_lhtight_iloose;
      else if ( trigger == "HLT_e24_tight_iloose" ) m_trigDecorators["HLT_e24_tight_iloose"] = &Props::passHLT_e24_tight_iloose;
      else if ( trigger == "HLT_e26_lhvloose_nod0_L1EM20VH" ) m_trigDecorators["HLT_e26_lhvloose_nod0_L1EM20VH"] = &Props::passHLT_e26_lhvloose_nod0_L1EM20VH;
      else if ( trigger == "HLT_e26_lhmedium_nod0_L1EM20VH" ) m_trigDecorators["HLT_e26_lhmedium_nod0_L1EM20VH"] = &Props::passHLT_e26_lhmedium_nod0_L1EM20VH;
      else if ( trigger == "HLT_e26_lhmedium_nod0_ivarloose" ) m_trigDecorators["HLT_e26_lhmedium_nod0_ivarloose"] = &Props::passHLT_e26_lhmedium_nod0_ivarloose;
      else if ( trigger == "HLT_e26_tight_iloose" ) m_trigDecorators["HLT_e26_tight_iloose"] = &Props::passHLT_e26_tight_iloose;
      else if ( trigger == "HLT_e26_tight1_iloose" ) m_trigDecorators["HLT_e26_tight1_iloose"] = &Props::passHLT_e26_tight1_iloose;
      else if ( trigger == "HLT_e26_lhtight_iloose" ) m_trigDecorators["HLT_e26_lhtight_iloose"] = &Props::passHLT_e26_lhtight_iloose;
      else if ( trigger == "HLT_e60_medium" ) m_trigDecorators["HLT_e60_medium"] = &Props::passHLT_e60_medium;
      else if ( trigger == "HLT_e60_medium1" ) m_trigDecorators["HLT_e60_medium1"] = &Props::passHLT_e60_medium1;
      else if ( trigger == "HLT_e60_lhmedium" ) m_trigDecorators["HLT_e60_lhmedium"] = &Props::passHLT_e60_lhmedium;
      else if ( trigger == "HLT_e60_lhmedium_nod0" ) m_trigDecorators["HLT_e60_lhmedium_nod0"] = &Props::passHLT_e60_lhmedium_nod0;
      else if ( trigger == "HLT_e60_lhmedium_nod0_L1EM24VHI" ) m_trigDecorators["HLT_e60_lhmedium_nod0_L1EM24VHI"] = &Props::passHLT_e60_lhmedium_nod0_L1EM24VHI;
      else if ( trigger == "HLT_e80_lhmedium_nod0" ) m_trigDecorators["HLT_e80_lhmedium_nod0"] = &Props::passHLT_e80_lhmedium_nod0;
      else if ( trigger == "HLT_e120_lhloose" ) m_trigDecorators["HLT_e120_lhloose"] = &Props::passHLT_e120_lhloose;
      else if ( trigger == "HLT_e120_lhloose_nod0" ) m_trigDecorators["HLT_e120_lhloose_nod0"] = &Props::passHLT_e120_lhloose_nod0;
      else if ( trigger == "HLT_e140_lhloose" ) m_trigDecorators["HLT_e140_lhloose"] = &Props::passHLT_e140_lhloose;
      else if ( trigger == "HLT_e140_lhloose_nod0" ) m_trigDecorators["HLT_e140_lhloose_nod0"] = &Props::passHLT_e140_lhloose_nod0;
      else if ( trigger == "HLT_e140_lhloose_nod0_L1EM24VHI" ) m_trigDecorators["HLT_e140_lhloose_nod0_L1EM24VHI"] = &Props::passHLT_e140_lhloose_nod0_L1EM24VHI;
      else if ( trigger == "HLT_e300_etcut" ) m_trigDecorators["HLT_e300_etcut"] = &Props::passHLT_e300_etcut;
      else if ( trigger == "HLT_e300_etcut_L1EM24VHI" ) m_trigDecorators["HLT_e300_etcut_L1EM24VHI"] = &Props::passHLT_e300_etcut_L1EM24VHI;
      else if ( trigger == "HLT_2e17_loose" ) m_trigDecorators["HLT_2e17_loose"] = &Props::passHLT_2e17_loose;
      else if ( trigger == "HLT_2e17_loose1" ) m_trigDecorators["HLT_2e17_loose1"] = &Props::passHLT_2e17_loose1;
      else if ( trigger == "HLT_2e17_lhloose" ) m_trigDecorators["HLT_2e17_lhloose"] = &Props::passHLT_2e17_lhloose;
      else if ( trigger == "HLT_2e17_lhvloose_nod0" ) m_trigDecorators["HLT_2e17_lhvloose_nod0"] = &Props::passHLT_2e17_lhvloose_nod0;
      else if ( trigger == "HLT_2e17_lhvloose_nod0_L12EM15VHI" ) m_trigDecorators["HLT_2e17_lhvloose_nod0_L12EM15VHI"] = &Props::passHLT_2e17_lhvloose_nod0_L12EM15VHI;
      else if ( trigger == "HLT_2e24_lhvloose_nod0" ) m_trigDecorators["HLT_2e24_lhvloose_nod0"] = &Props::passHLT_2e24_lhvloose_nod0;
      else if ( trigger == "HLT_e17_lhloose_mu14" ) m_trigDecorators["HLT_e17_lhloose_mu14"] = &Props::passHLT_e17_lhloose_mu14;
      else if ( trigger == "HLT_e17_lhloose_nod0_mu14" ) m_trigDecorators["HLT_e17_lhloose_nod0_mu14"] = &Props::passHLT_e17_lhloose_nod0_mu14;
      else if ( trigger == "HLT_e26_lhmedium_nod0_mu8noL1" ) m_trigDecorators["HLT_e26_lhmedium_nod0_mu8noL1"] = &Props::passHLT_e26_lhmedium_nod0_mu8noL1;
      else if ( trigger == "HLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1" ) m_trigDecorators["HLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1"] = &Props::passHLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1;
      else if ( trigger == "HLT_e7_lhmedium_mu24" ) m_trigDecorators["HLT_e7_lhmedium_mu24"] = &Props::passHLT_e7_lhmedium_mu24;
      else if ( trigger == "HLT_e7_lhmedium_nod0_mu24" ) m_trigDecorators["HLT_e7_lhmedium_nod0_mu24"] = &Props::passHLT_e7_lhmedium_nod0_mu24;
      else if ( trigger == "L1_MU20" ) m_trigDecorators["L1_MU20"] = &Props::passL1_MU20;
      else if ( trigger == "L1_2MU10" ) m_trigDecorators["L1_2MU10"] = &Props::passL1_2MU10;
      else if ( trigger == "HLT_mu20_iloose_L1MU15" ) m_trigDecorators["HLT_mu20_iloose_L1MU15"] = &Props::passHLT_mu20_iloose_L1MU15;
      else if ( trigger == "HLT_mu14" ) m_trigDecorators["HLT_mu14"] = &Props::passHLT_mu14;
      else if ( trigger == "HLT_mu20_2mu4noL1" ) m_trigDecorators["HLT_mu20_2mu4noL1"] = &Props::passHLT_mu20_2mu4noL1;
      else if ( trigger == "HLT_mu24_iloose_L1MU15" ) m_trigDecorators["HLT_mu24_iloose_L1MU15"] = &Props::passHLT_mu24_iloose_L1MU15;
      else if ( trigger == "HLT_mu24_imedium" ) m_trigDecorators["HLT_mu24_imedium"] = &Props::passHLT_mu24_imedium;
      else if ( trigger == "HLT_mu26_imedium" ) m_trigDecorators["HLT_mu26_imedium"] = &Props::passHLT_mu26_imedium;
      else if ( trigger == "HLT_mu40" ) m_trigDecorators["HLT_mu40"] = &Props::passHLT_mu40;
      else if ( trigger == "HLT_mu50" ) m_trigDecorators["HLT_mu50"] = &Props::passHLT_mu50;
      else if ( trigger == "HLT_mu50_0eta105_msonly" ) m_trigDecorators["HLT_mu50_0eta105_msonly"] = &Props::passHLT_mu50_0eta105_msonly;
      else if ( trigger == "HLT_mu60" ) m_trigDecorators["HLT_mu60"] = &Props::passHLT_mu60;
      else if ( trigger == "HLT_mu60_0eta105_msonly" ) m_trigDecorators["HLT_mu60_0eta105_msonly"] = &Props::passHLT_mu60_0eta105_msonly;
      else if ( trigger == "HLT_mu80" ) m_trigDecorators["HLT_mu80"] = &Props::passHLT_mu80;
      else if ( trigger == "HLT_2mu10" ) m_trigDecorators["HLT_2mu10"] = &Props::passHLT_2mu10;
      else if ( trigger == "HLT_2mu14" ) m_trigDecorators["HLT_2mu14"] = &Props::passHLT_2mu14;
      else if ( trigger == "HLT_mu18_mu8noL1" ) m_trigDecorators["HLT_mu18_mu8noL1"] = &Props::passHLT_mu18_mu8noL1;
      else if ( trigger == "HLT_mu24_mu8noL1" ) m_trigDecorators["HLT_mu24_mu8noL1"] = &Props::passHLT_mu24_mu8noL1;
      else if ( trigger == "HLT_mu24_mu8noL1_calotag_0eta010" ) m_trigDecorators["HLT_mu24_mu8noL1_calotag_0eta010"] = &Props::passHLT_mu24_mu8noL1_calotag_0eta010;
      else if ( trigger == "HLT_e28_lhtight_nod0_ivarloose" ) m_trigDecorators["HLT_e28_lhtight_nod0_ivarloose"] = &Props::passHLT_e28_lhtight_nod0_ivarloose;
      else if ( trigger == "HLT_e28_lhtight_nod0_ivarloose_L1EM24VHIM" ) m_trigDecorators["HLT_e28_lhtight_nod0_ivarloose_L1EM24VHIM"] = &Props::passHLT_e28_lhtight_nod0_ivarloose_L1EM24VHIM;
      else if ( trigger == "HLT_e26_lhtight_ivarloose" ) m_trigDecorators["HLT_e26_lhtight_ivarloose"] = &Props::passHLT_e26_lhtight_ivarloose;
      else if ( trigger == "HLT_e26_lhtight_nod0_ivarloose" ) m_trigDecorators["HLT_e26_lhtight_nod0_ivarloose"] = &Props::passHLT_e26_lhtight_nod0_ivarloose;
      else if ( trigger == "HLT_e26_lhtight_nod0_ivarloose_L1EM22VHIM" ) m_trigDecorators["HLT_e26_lhtight_nod0_ivarloose_L1EM22VHIM"] = &Props::passHLT_e26_lhtight_nod0_ivarloose_L1EM22VHIM;
      else if ( trigger == "HLT_e26_lhtight_smooth_ivarloose" ) m_trigDecorators["HLT_e26_lhtight_smooth_ivarloose"] = &Props::passHLT_e26_lhtight_smooth_ivarloose;
      else if ( trigger == "HLT_e24_lhtight_ivarloose" ) m_trigDecorators["HLT_e24_lhtight_ivarloose"] = &Props::passHLT_e24_lhtight_ivarloose;
      else if ( trigger == "HLT_e24_lhtight_nod0_ivarloose" ) m_trigDecorators["HLT_e24_lhtight_nod0_ivarloose"] = &Props::passHLT_e24_lhtight_nod0_ivarloose;
      else if ( trigger == "HLT_e24_lhmedium_ivarloose" ) m_trigDecorators["HLT_e24_lhmedium_ivarloose"] = &Props::passHLT_e24_lhmedium_ivarloose;
      else if ( trigger == "HLT_e24_lhmedium_nod0_ivarloose" ) m_trigDecorators["HLT_e24_lhmedium_nod0_ivarloose"] = &Props::passHLT_e24_lhmedium_nod0_ivarloose;
      else if ( trigger == "HLT_e24_lhmedium_nod0_L1EM20VH" ) m_trigDecorators["HLT_e24_lhmedium_nod0_L1EM20VH"] = &Props::passHLT_e24_lhmedium_nod0_L1EM20VH;
      else if ( trigger == "HLT_2e17_lhloose_nod0" ) m_trigDecorators["HLT_2e17_lhloose_nod0"] = &Props::passHLT_2e17_lhloose_nod0;
      else if ( trigger == "HLT_2e15_lhloose_L12EM13VH" ) m_trigDecorators["HLT_2e15_lhloose_L12EM13VH"] = &Props::passHLT_2e15_lhloose_L12EM13VH;
      else if ( trigger == "HLT_2e15_lhloose_nod0_L12EM13VH" ) m_trigDecorators["HLT_2e15_lhloose_nod0_L12EM13VH"] = &Props::passHLT_2e15_lhloose_nod0_L12EM13VH;
      else if ( trigger == "HLT_2e12_lhloose_L12EM10VH" ) m_trigDecorators["HLT_2e12_lhloose_L12EM10VH"] = &Props::passHLT_2e12_lhloose_L12EM10VH;
      else if ( trigger == "HLT_2e12_lhloose_nod0_L12EM10VH" ) m_trigDecorators["HLT_2e12_lhloose_nod0_L12EM10VH"] = &Props::passHLT_2e12_lhloose_nod0_L12EM10VH;
      else if ( trigger == "HLT_mu28_ivarmedium" ) m_trigDecorators["HLT_mu28_ivarmedium"] = &Props::passHLT_mu28_ivarmedium;
      else if ( trigger == "HLT_mu26_ivarmedium" ) m_trigDecorators["HLT_mu26_ivarmedium"] = &Props::passHLT_mu26_ivarmedium;
      else if ( trigger == "HLT_mu24_ivarmedium" ) m_trigDecorators["HLT_mu24_ivarmedium"] = &Props::passHLT_mu24_ivarmedium;
      else if ( trigger == "HLT_mu24_ivarloose" ) m_trigDecorators["HLT_mu24_ivarloose"] = &Props::passHLT_mu24_ivarloose;
      else if ( trigger == "HLT_mu24_ivarloose_L1MU15" ) m_trigDecorators["HLT_mu24_ivarloose_L1MU15"] = &Props::passHLT_mu24_ivarloose_L1MU15;
      else if ( trigger == "HLT_mu24_iloose" ) m_trigDecorators["HLT_mu24_iloose"] = &Props::passHLT_mu24_iloose;
      else if ( trigger == "HLT_mu22_mu8noL1" ) m_trigDecorators["HLT_mu22_mu8noL1"] = &Props::passHLT_mu22_mu8noL1;
      else if ( trigger == "HLT_mu22_mu8noL1_calotag_0eta010" ) m_trigDecorators["HLT_mu22_mu8noL1_calotag_0eta010"] = &Props::passHLT_mu22_mu8noL1_calotag_0eta010;
      else if ( trigger == "HLT_mu20_mu8noL1" ) m_trigDecorators["HLT_mu20_mu8noL1"] = &Props::passHLT_mu20_mu8noL1;
      else if ( trigger == "HLT_mu4_j15_dr05"  ) m_trigDecorators["HLT_mu4_j15_dr05" ] = &Props::passHLT_mu4_j15_dr05;
      else if ( trigger == "HLT_mu4_j25_dr05"  ) m_trigDecorators["HLT_mu4_j25_dr05" ] = &Props::passHLT_mu4_j25_dr05;
      else if ( trigger == "HLT_mu4_j35_dr05"  ) m_trigDecorators["HLT_mu4_j35_dr05" ] = &Props::passHLT_mu4_j35_dr05;
      else if ( trigger == "HLT_mu4_j55_dr05"  ) m_trigDecorators["HLT_mu4_j55_dr05" ] = &Props::passHLT_mu4_j55_dr05;
      else if ( trigger == "HLT_mu6_j85_dr05"  ) m_trigDecorators["HLT_mu6_j85_dr05" ] = &Props::passHLT_mu6_j85_dr05;
      else if ( trigger == "HLT_mu6_j110_dr05"  ) m_trigDecorators["HLT_mu6_j110_dr05" ] = &Props::passHLT_mu6_j110_dr05;
      else if ( trigger == "HLT_mu6_j150_dr05"  ) m_trigDecorators["HLT_mu6_j150_dr05" ] = &Props::passHLT_mu6_j150_dr05;
      else if ( trigger == "HLT_mu6_j175_dr05"  ) m_trigDecorators["HLT_mu6_j175_dr05" ] = &Props::passHLT_mu6_j175_dr05;
      else if ( trigger == "HLT_mu6_j260_dr05"  ) m_trigDecorators["HLT_mu6_j260_dr05" ] = &Props::passHLT_mu6_j260_dr05;
      else if ( trigger == "HLT_mu6_j320_dr05"  ) m_trigDecorators["HLT_mu6_j320_dr05" ] = &Props::passHLT_mu6_j320_dr05;
      else if ( trigger == "HLT_mu6_j400_dr05"  ) m_trigDecorators["HLT_mu6_j400_dr05" ] = &Props::passHLT_mu6_j400_dr05;
      else if ( trigger == "HLT_mu4_j15_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu4_j15_bperf_split_dr05_dz02" ] = &Props::passHLT_mu4_j15_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu4_j25_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu4_j25_bperf_split_dr05_dz02" ] = &Props::passHLT_mu4_j25_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu4_j35_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu4_j35_bperf_split_dr05_dz02" ] = &Props::passHLT_mu4_j35_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu4_j55_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu4_j55_bperf_split_dr05_dz02" ] = &Props::passHLT_mu4_j55_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu6_j85_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu6_j85_bperf_split_dr05_dz02" ] = &Props::passHLT_mu6_j85_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu6_j110_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu6_j110_bperf_split_dr05_dz02" ] = &Props::passHLT_mu6_j110_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu6_j150_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu6_j150_bperf_split_dr05_dz02" ] = &Props::passHLT_mu6_j150_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu6_j175_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu6_j175_bperf_split_dr05_dz02" ] = &Props::passHLT_mu6_j175_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu6_j260_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu6_j260_bperf_split_dr05_dz02" ] = &Props::passHLT_mu6_j260_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu6_j320_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu6_j320_bperf_split_dr05_dz02" ] = &Props::passHLT_mu6_j320_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_mu6_j400_bperf_split_dr05_dz02"  ) m_trigDecorators["HLT_mu6_j400_bperf_split_dr05_dz02" ] = &Props::passHLT_mu6_j400_bperf_split_dr05_dz02;
      else if ( trigger == "HLT_g10_loose" ) m_trigDecorators["HLT_g10_loose"] = &Props::passHLT_g10_loose;
      else if ( trigger == "HLT_g15_loose_L1EM7" ) m_trigDecorators["HLT_g15_loose_L1EM7"] = &Props::passHLT_g15_loose_L1EM7;
      else if ( trigger == "HLT_g20_loose_L1EM12" ) m_trigDecorators["HLT_g20_loose_L1EM12"] = &Props::passHLT_g20_loose_L1EM12;
      else if ( trigger == "HLT_g25_loose_L1EM15" ) m_trigDecorators["HLT_g25_loose_L1EM15"] = &Props::passHLT_g25_loose_L1EM15;
      else if ( trigger == "HLT_g35_loose_L1EM15" ) m_trigDecorators["HLT_g35_loose_L1EM15"] = &Props::passHLT_g35_loose_L1EM15;
      else if ( trigger == "HLT_g40_loose_L1EM15" ) m_trigDecorators["HLT_g40_loose_L1EM15"] = &Props::passHLT_g40_loose_L1EM15;
      else if ( trigger == "HLT_g45_loose_L1EM15" ) m_trigDecorators["HLT_g45_loose_L1EM15"] = &Props::passHLT_g45_loose_L1EM15;
      else if ( trigger == "HLT_g50_loose_L1EM15" ) m_trigDecorators["HLT_g50_loose_L1EM15"] = &Props::passHLT_g50_loose_L1EM15;
      else if ( trigger == "HLT_g60_loose" ) m_trigDecorators["HLT_g60_loose"] = &Props::passHLT_g60_loose;
      else if ( trigger == "HLT_g70_loose" ) m_trigDecorators["HLT_g70_loose"] = &Props::passHLT_g70_loose;
      else if ( trigger == "HLT_g80_loose" ) m_trigDecorators["HLT_g80_loose"] = &Props::passHLT_g80_loose;
      else if ( trigger == "HLT_g100_loose" ) m_trigDecorators["HLT_g100_loose"] = &Props::passHLT_g100_loose;
      else if ( trigger == "HLT_g120_loose" ) m_trigDecorators["HLT_g120_loose"] = &Props::passHLT_g120_loose;
      else if ( trigger == "HLT_g140_loose" ) m_trigDecorators["HLT_g140_loose"] = &Props::passHLT_g140_loose;
      else if ( trigger == "HLT_g140_tight" ) m_trigDecorators["HLT_g140_tight"] = &Props::passHLT_g140_tight;
      else if ( trigger == "HLT_g200_loose" ) m_trigDecorators["HLT_g200_loose"] = &Props::passHLT_g200_loose;
      else if ( trigger == "HLT_g300_etcut_L1EM24VHI" ) m_trigDecorators["HLT_g300_etcut_L1EM24VHI"] = &Props::passHLT_g300_etcut_L1EM24VHI;
      else if ( trigger == "HLT_g300_etcut" ) m_trigDecorators["HLT_g300_etcut"] = &Props::passHLT_g300_etcut;
      else if ( trigger == "L1_MJJ-400" ) m_trigDecorators["L1_MJJ-400"] = &Props::passL1_MJJ_400;
      else if ( trigger == "L1_MJJ-700" ) m_trigDecorators["L1_MJJ-700"] = &Props::passL1_MJJ_700;
      else if ( trigger == "L1_MJJ-800" ) m_trigDecorators["L1_MJJ-800"] = &Props::passL1_MJJ_800;
      else if ( trigger == "L1_MJJ-900" ) m_trigDecorators["L1_MJJ-900"] = &Props::passL1_MJJ_900;
      else if ( trigger == "L1_J30_2J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["L1_J30_2J20_4J20.0ETA49_MJJ-400"] = &Props::passL1_J30_2J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "L1_J30_2J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["L1_J30_2J20_4J20.0ETA49_MJJ-700"] = &Props::passL1_J30_2J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "L1_J30_2J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["L1_J30_2J20_4J20.0ETA49_MJJ-800"] = &Props::passL1_J30_2J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "L1_J30_2J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["L1_J30_2J20_4J20.0ETA49_MJJ-900"] = &Props::passL1_J30_2J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "L1_3J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["L1_3J20_4J20.0ETA49_MJJ-400"] = &Props::passL1_3J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "L1_3J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["L1_3J20_4J20.0ETA49_MJJ-700"] = &Props::passL1_3J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "L1_3J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["L1_3J20_4J20.0ETA49_MJJ-800"] = &Props::passL1_3J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "L1_3J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["L1_3J20_4J20.0ETA49_MJJ-900"] = &Props::passL1_3J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "L1_4J20.0ETA49" ) m_trigDecorators["L1_4J20.0ETA49"] = &Props::passL1_4J20_0ETA49;
      else if ( trigger == "HLT_g15_loose_2j40_0eta490_3j25_0eta490" ) m_trigDecorators["HLT_g15_loose_2j40_0eta490_3j25_0eta490"] = &Props::passHLT_g15_loose_2j40_0eta490_3j25_0eta490;
      else if ( trigger == "HLT_g20_loose_2j40_0eta490_3j25_0eta490" ) m_trigDecorators["HLT_g20_loose_2j40_0eta490_3j25_0eta490"] = &Props::passHLT_g20_loose_2j40_0eta490_3j25_0eta490;
      else if ( trigger == "HLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ-700" ) m_trigDecorators["HLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ-700"] = &Props::passHLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ_700;
      else if ( trigger == "HLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ-900" ) m_trigDecorators["HLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ-900"] = &Props::passHLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ_900;
      else if ( trigger == "HLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490" ) m_trigDecorators["HLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490"] = &Props::passHLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490;
      else if ( trigger == "HLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700" ) m_trigDecorators["HLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700"] = &Props::passHLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700;
      else if ( trigger == "HLT_g25_loose_L1EM20VH_2j40_0eta490_3j25_0eta490_invm700" ) m_trigDecorators["HLT_g25_loose_L1EM20VH_2j40_0eta490_3j25_0eta490_invm700"] = &Props::passHLT_g25_loose_L1EM20VH_2j40_0eta490_3j25_0eta490_invm700;
      else if ( trigger == "HLT_g25_loose_2j40_0eta490_3j25_0eta490_invm700" ) m_trigDecorators["HLT_g25_loose_2j40_0eta490_3j25_0eta490_invm700"] = &Props::passHLT_g25_loose_2j40_0eta490_3j25_0eta490_invm700;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700"] = &Props::passHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000"] = &Props::passHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700"] = &Props::passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490"] = &Props::passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700"] = &Props::passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700"] = &Props::passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490"] = &Props::passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490"] = &Props::passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490;
      else if ( trigger == "HLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500" ) m_trigDecorators["HLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500"] = &Props::passHLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500;
      else if ( trigger == "HLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500" ) m_trigDecorators["HLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500"] = &Props::passHLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_j35_0eta490_boffperf_split_3j35_0eta490_invm700" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_j35_0eta490_boffperf_split_3j35_0eta490_invm700"] = &Props::passHLT_g25_medium_L1EM22VHI_j35_0eta490_boffperf_split_3j35_0eta490_invm700;
      else if ( trigger == "HLT_g25_medium_L1EM22VHI_2j35_0eta490_boffperf_split_2j35_0eta490" ) m_trigDecorators["HLT_g25_medium_L1EM22VHI_2j35_0eta490_boffperf_split_2j35_0eta490"] = &Props::passHLT_g25_medium_L1EM22VHI_2j35_0eta490_boffperf_split_2j35_0eta490;
      else if ( trigger == "HLT_g25_loose_L1EM20VH_4j35_0eta490" ) m_trigDecorators["HLT_g25_loose_L1EM20VH_4j35_0eta490"] = &Props::passHLT_g25_loose_L1EM20VH_4j35_0eta490;
      else if ( trigger == "HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-400"] = &Props::passHLT_2j55_bperf_L1J30_2J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-700"] = &Props::passHLT_2j55_bperf_L1J30_2J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-800"] = &Props::passHLT_2j55_bperf_L1J30_2J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["HLT_2j55_bperf_L1J30_2J20_4J20.0ETA49_MJJ-900"] = &Props::passHLT_2j55_bperf_L1J30_2J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-400"] = &Props::passHLT_2j55_bperf_L13J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-700"] = &Props::passHLT_2j55_bperf_L13J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-800"] = &Props::passHLT_2j55_bperf_L13J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["HLT_2j55_bperf_L13J20_4J20.0ETA49_MJJ-900"] = &Props::passHLT_2j55_bperf_L13J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "HLT_2j55_bperf_L14J20.0ETA49" ) m_trigDecorators["HLT_2j55_bperf_L14J20.0ETA49"] = &Props::passHLT_2j55_bperf_L14J20_0ETA49;
      else if ( trigger == "HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-400"] = &Props::passHLT_2j55_bloose_L1J30_2J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-700"] = &Props::passHLT_2j55_bloose_L1J30_2J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-800"] = &Props::passHLT_2j55_bloose_L1J30_2J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["HLT_2j55_bloose_L1J30_2J20_4J20.0ETA49_MJJ-900"] = &Props::passHLT_2j55_bloose_L1J30_2J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-400"] = &Props::passHLT_2j55_bloose_L13J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-700"] = &Props::passHLT_2j55_bloose_L13J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-800"] = &Props::passHLT_2j55_bloose_L13J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["HLT_2j55_bloose_L13J20_4J20.0ETA49_MJJ-900"] = &Props::passHLT_2j55_bloose_L13J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "HLT_2j55_bloose_L14J20.0ETA49" ) m_trigDecorators["HLT_2j55_bloose_L14J20.0ETA49"] = &Props::passHLT_2j55_bloose_L14J20_0ETA49;
      else if ( trigger == "HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-400"] = &Props::passHLT_2j55_bmedium_L13J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-700"] = &Props::passHLT_2j55_bmedium_L13J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-800"] = &Props::passHLT_2j55_bmedium_L13J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["HLT_2j55_bmedium_L13J20_4J20.0ETA49_MJJ-900"] = &Props::passHLT_2j55_bmedium_L13J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-400" ) m_trigDecorators["HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-400"] = &Props::passHLT_2j55_bmedium_L1J30_2J20_4J20_0ETA49_MJJ_400;
      else if ( trigger == "HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-700" ) m_trigDecorators["HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-700"] = &Props::passHLT_2j55_bmedium_L1J30_2J20_4J20_0ETA49_MJJ_700;
      else if ( trigger == "HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-800" ) m_trigDecorators["HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-800"] = &Props::passHLT_2j55_bmedium_L1J30_2J20_4J20_0ETA49_MJJ_800;
      else if ( trigger == "HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-900" ) m_trigDecorators["HLT_2j55_bmedium_L1J30_2J20_4J20.0ETA49_MJJ-900"] = &Props::passHLT_2j55_bmedium_L1J30_2J20_4J20_0ETA49_MJJ_900;
      else if ( trigger == "HLT_2j55_bmedium_L14J20.0ETA49" ) m_trigDecorators["HLT_2j55_bmedium_L14J20.0ETA49"] = &Props::passHLT_2j55_bmedium_L14J20_0ETA49;
      else if ( trigger == "L1_EM20VHI" ) m_trigDecorators["L1_EM20VHI"] = &Props::passL1_EM20VHI;
      else if ( trigger == "L1_MU15" ) m_trigDecorators["L1_MU15"] = &Props::passL1_MU15;
      else if ( trigger == "L1_XE50" ) m_trigDecorators["L1_XE50"] = &Props::passL1_XE50;
      else if ( trigger == "L1_XE35" ) m_trigDecorators["L1_XE35"] = &Props::passL1_XE35;
      else if ( trigger == "HLT_xe35" ) m_trigDecorators["HLT_xe35"] = &Props::passHLT_xe35;
      else if ( trigger == "HLT_xe50" ) m_trigDecorators["HLT_xe50"] = &Props::passHLT_xe50;
      else if ( trigger == "HLT_xe60" ) m_trigDecorators["HLT_xe60"] = &Props::passHLT_xe60;
      else if ( trigger == "L1_XE40" ) m_trigDecorators["L1_XE40"] = &Props::passL1_XE40;
      //tauTriggers
      else if ( trigger == "HLT_tau25_medium1_tracktwo") m_trigDecorators["HLT_tau25_medium1_tracktwo"] = &Props::passHLT_tau25_medium1_tracktwo ;
      else if ( trigger == "HLT_tau25_medium1_tracktwo_L1TAU12") m_trigDecorators["HLT_tau25_medium1_tracktwo_L1TAU12"] = &Props::passHLT_tau25_medium1_tracktwo_L1TAU12 ;
      else if ( trigger == "HLT_tau25_medium1_tracktwo_L1TAU12IL") m_trigDecorators["HLT_tau25_medium1_tracktwo_L1TAU12IL"] = &Props::passHLT_tau25_medium1_tracktwo_L1TAU12IL ;
      else if ( trigger == "HLT_tau25_medium1_tracktwo_L1TAU12IT") m_trigDecorators["HLT_tau25_medium1_tracktwo_L1TAU12IT"] = &Props::passHLT_tau25_medium1_tracktwo_L1TAU12IT ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo") m_trigDecorators["HLT_tau35_medium1_tracktwo"] = &Props::passHLT_tau35_medium1_tracktwo ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_L1TAU20") m_trigDecorators["HLT_tau35_medium1_tracktwo_L1TAU20"] = &Props::passHLT_tau35_medium1_tracktwo_L1TAU20 ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_L1TAU20IL") m_trigDecorators["HLT_tau35_medium1_tracktwo_L1TAU20IL"] = &Props::passHLT_tau35_medium1_tracktwo_L1TAU20IL ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_L1TAU20IT") m_trigDecorators["HLT_tau35_medium1_tracktwo_L1TAU20IT"] = &Props::passHLT_tau35_medium1_tracktwo_L1TAU20IT;
      else if ( trigger == "HLT_tau50_medium1_tracktwo_L1TAU12") m_trigDecorators["HLT_tau50_medium1_tracktwo_L1TAU12"] = &Props::passHLT_tau50_medium1_tracktwo_L1TAU12 ;
      else if ( trigger == "HLT_tau80_medium1_tracktwo") m_trigDecorators["HLT_tau80_medium1_tracktwo"] = &Props::passHLT_tau80_medium1_tracktwo ;
      else if ( trigger == "HLT_tau80_medium1_tracktwo_L1TAU60") m_trigDecorators["HLT_tau80_medium1_tracktwo_L1TAU60"] = &Props::passHLT_tau80_medium1_tracktwo_L1TAU60 ;
      else if ( trigger == "HLT_tau125_medium1_tracktwo") m_trigDecorators["HLT_tau125_medium1_tracktwo"] = &Props::passHLT_tau125_medium1_tracktwo ;
      else if ( trigger == "HLT_tau160_medium1_tracktwo") m_trigDecorators["HLT_tau160_medium1_tracktwo"] = &Props::passHLT_tau160_medium1_tracktwo ;
      else if ( trigger == "HLT_tau160_medium1_tracktwo_L1TAU100") m_trigDecorators["HLT_tau160_medium1_tracktwo_L1TAU100"] = &Props::passHLT_tau160_medium1_tracktwo_L1TAU100 ;
      else if ( trigger == "HLT_tau160_medium1_tracktwoEF_L1TAU100") m_trigDecorators["HLT_tau160_medium1_tracktwoEF_L1TAU100"] = &Props::passHLT_tau160_medium1_tracktwoEF_L1TAU100 ;
      else if ( trigger == "HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100") m_trigDecorators["HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"] = &Props::passHLT_tau160_mediumRNN_tracktwoMVA_L1TAU100;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR-TAU20ITAU12I-J25") m_trigDecorators["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR-TAU20ITAU12I-J25"]=&Props::passHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR_TAU20ITAU12I_J25 ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_03dR30_L1DR-TAU20ITAU12I-J25") m_trigDecorators["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_03dR30_L1DR-TAU20ITAU12I-J25"]=&Props::passHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_03dR30_L1DR_TAU20ITAU12I_J25 ;
      else if ( trigger == "HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_03dR30_L1DR-TAU20ITAU12I-J25") m_trigDecorators["HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_03dR30_L1DR-TAU20ITAU12I-J25"]=&Props::passHLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_03dR30_L1DR_TAU20ITAU12I_J25;
      else if ( trigger == "HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dR30_L1DR-TAU20ITAU12I-J25") m_trigDecorators["HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dR30_L1DR-TAU20ITAU12I-J25"]=&Props::passHLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dR30_L1DR_TAU20ITAU12I_J25;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR-TAU20ITAU12I") m_trigDecorators["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR-TAU20ITAU12I"]=&Props::passHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR_TAU20ITAU12I ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20ITAU12I-J25") m_trigDecorators["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20ITAU12I-J25"] = &Props::passHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20ITAU12I_J25;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo") m_trigDecorators["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"] = &Props::passHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM") m_trigDecorators["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM"] = &Props::passHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM ;
      else if ( trigger == "HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1DR-TAU20ITAU12I-J25") m_trigDecorators["HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1DR-TAU20ITAU12I-J25"] = &Props::passHLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1DR_TAU20ITAU12I_J25 ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM_4J12") m_trigDecorators["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM_4J12"] = &Props::passHLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM_4J12 ;
      else if ( trigger == "HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1TAU20IM_2TAU12IM_4J12.0ETA23") m_trigDecorators["HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1TAU20IM_2TAU12IM_4J12.0ETA23"] = &Props::passHLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1TAU20IM_2TAU12IM_4J12_0ETA23 ;
      else if ( trigger == "HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1TAU20IM_2TAU12IM_4J12.0ETA23") m_trigDecorators["HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1TAU20IM_2TAU12IM_4J12.0ETA23"] = &Props::passHLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1TAU20IM_2TAU12IM_4J12_0ETA23 ;
      else if ( trigger == "HLT_tau40_medium1_tracktwoEF_tau35_medium1_tracktwoEF") m_trigDecorators["HLT_tau40_medium1_tracktwoEF_tau35_medium1_tracktwoEF"] = &Props::passHLT_tau40_medium1_tracktwoEF_tau35_medium1_tracktwoEF ;
      else if ( trigger == "HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo") m_trigDecorators["HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo"] = &Props::passHLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo ;
      else if ( trigger == "HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1DR-TAU20ITAU12I-J25") m_trigDecorators["HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1DR-TAU20ITAU12I-J25"] = &Props::passHLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1DR_TAU20ITAU12I_J25 ;
      else if ( trigger == "HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo_L1TAU20IM_2TAU12IM") m_trigDecorators["HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo_L1TAU20IM_2TAU12IM"] = &Props::passHLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo_L1TAU20IM_2TAU12IM ;
      else if ( trigger == "HLT_tau35_medium1_tracktwo_L1TAU20_tau25_medium1_tracktwo_L1TAU12") m_trigDecorators["HLT_tau35_medium1_tracktwo_L1TAU20_tau25_medium1_tracktwo_L1TAU12"] = &Props::passHLT_tau35_medium1_tracktwo_L1TAU20_tau25_medium1_tracktwo_L1TAU12 ;
      else if ( trigger == "HLT_tau40_medium1_tracktwo_tau35_medium1_tracktwo") m_trigDecorators["HLT_tau40_medium1_tracktwo_tau35_medium1_tracktwo"] = &Props::passHLT_tau40_medium1_tracktwo_tau35_medium1_tracktwo;
      else if ( trigger == "HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA") m_trigDecorators["HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA"] = &Props::passHLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA ;
      else if ( trigger == "HLT_tau80_medium1_TAU60_tau50_medium1_L1TAU12") m_trigDecorators["HLT_tau80_medium1_TAU60_tau50_medium1_L1TAU12"] = &Props::passHLT_tau80_medium1_TAU60_tau50_medium1_L1TAU12 ;
      else if ( trigger == "HLT_tau80_medium1_tracktwoEF_L1TAU60_tau35_medium1_tracktwoEF_L1TAU12IM_L1TAU60_DR-TAU20ITAU12I") m_trigDecorators["HLT_tau80_medium1_tracktwoEF_L1TAU60_tau35_medium1_tracktwoEF_L1TAU12IM_L1TAU60_DR-TAU20ITAU12I"] = &Props::passHLT_tau80_medium1_tracktwoEF_L1TAU60_tau35_medium1_tracktwoEF_L1TAU12IM_L1TAU60_DR_TAU20ITAU12I ;
      else if ( trigger == "HLT_tau80_medium1_tracktwoEF_L1TAU60_tau60_medium1_tracktwoEF_L1TAU40") m_trigDecorators["HLT_tau80_medium1_tracktwoEF_L1TAU60_tau60_medium1_tracktwoEF_L1TAU40"] = &Props::passHLT_tau80_medium1_tracktwoEF_L1TAU60_tau60_medium1_tracktwoEF_L1TAU40 ;
      else if ( trigger == "HLT_tau80_medium1_tracktwo_L1TAU60_tau50_medium1_tracktwo_L1TAU12") m_trigDecorators["HLT_tau80_medium1_tracktwo_L1TAU60_tau50_medium1_tracktwo_L1TAU12"] = &Props::passHLT_tau80_medium1_tracktwo_L1TAU60_tau50_medium1_tracktwo_L1TAU12 ;
      else if ( trigger == "HLT_tau80_medium1_tracktwo_L1TAU60_tau60_medium1_tracktwo_L1TAU40") m_trigDecorators["HLT_tau80_medium1_tracktwo_L1TAU60_tau60_medium1_tracktwo_L1TAU40"] = &Props::passHLT_tau80_medium1_tracktwo_L1TAU60_tau60_medium1_tracktwo_L1TAU40 ;
      else if ( trigger == "HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60_tau35_mediumRNN_tracktwoMVA_L1TAU12IM_L1TAU60_DR-TAU20ITAU12I") m_trigDecorators["HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60_tau35_mediumRNN_tracktwoMVA_L1TAU12IM_L1TAU60_DR-TAU20ITAU12I"] = &Props::passHLT_tau80_mediumRNN_tracktwoMVA_L1TAU60_tau35_mediumRNN_tracktwoMVA_L1TAU12IM_L1TAU60_DR_TAU20ITAU12I ;
      else if ( trigger == "HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60_tau60_mediumRNN_tracktwoMVA_L1TAU40") m_trigDecorators["HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60_tau60_mediumRNN_tracktwoMVA_L1TAU40"] = &Props::passHLT_tau80_mediumRNN_tracktwoMVA_L1TAU60_tau60_mediumRNN_tracktwoMVA_L1TAU40 ;
      else if ( trigger =="HLT_tau80_medium1_tracktwo_L1TAU60_tau35_medium1_tracktwo_L1TAU12IM_L1TAU60_DR-TAU20ITAU12I") m_trigDecorators["HLT_tau80_medium1_tracktwo_L1TAU60_tau35_medium1_tracktwo_L1TAU12IM_L1TAU60_DR-TAU20ITAU12I"]= &Props::passHLT_tau80_medium1_tracktwo_L1TAU60_tau35_medium1_tracktwo_L1TAU12IM_L1TAU60_DR_TAU20ITAU12I;
      else if ( trigger == "HLT_tau125_medium1_tracktwo_tau50_medium1_tracktwo_L1TAU12") m_trigDecorators["HLT_tau125_medium1_tracktwo_tau50_medium1_tracktwo_L1TAU12"] = &Props::passHLT_tau125_medium1_tracktwo_tau50_medium1_tracktwo_L1TAU12;
      else if ( trigger == "HLT_e17_medium_tau25_medium1_tracktwo")m_trigDecorators["HLT_e17_medium_tau25_medium1_tracktwo"] =&Props::passHLT_e17_medium_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_tau25_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_tau25_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_e17_medium_iloose_tau25_medium1_tracktwo")m_trigDecorators["HLT_e17_medium_iloose_tau25_medium1_tracktwo"] =&Props::passHLT_e17_medium_iloose_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_iloose_tau25_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_iloose_tau25_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_iloose_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_nod0_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1DR-EM15TAU12I-J25")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1DR-EM15TAU12I-J25"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1DR_EM15TAU12I_J25;
      else if ( trigger == "HLT_e17_medium_tau80_medium1_tracktwo")m_trigDecorators["HLT_e17_medium_tau80_medium1_tracktwo"] =&Props::passHLT_e17_medium_tau80_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_tau80_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_tau80_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_tau80_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_tau80_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_nod0_tau80_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_nod0_tau80_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_iloose_tau25_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_nod0_iloose_tau25_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_nod0_iloose_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_iloose_tau35_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_nod0_iloose_tau35_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_nod0_iloose_tau35_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau35_medium1_tracktwo")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau35_medium1_tracktwo"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau35_medium1_tracktwo;
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1EM15VHI_2TAU12IM_4J12")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1EM15VHI_2TAU12IM_4J12"]=&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1EM15VHI_2TAU12IM_4J12;
      //
      else if ( trigger == "HLT_e24_lhmedium_nod0_ivarloose_tau35_medium1_tracktwo")m_trigDecorators["HLT_e24_lhmedium_nod0_ivarloose_tau35_medium1_tracktwo"] =&Props::passHLT_e24_lhmedium_nod0_ivarloose_tau35_medium1_tracktwo;
      //
      else if ( trigger == "HLT_mu14_tau25_medium1_tracktwo")m_trigDecorators["HLT_mu14_tau25_medium1_tracktwo"] =&Props::passHLT_mu14_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_mu14_tau35_medium1_tracktwo_L1TAU20")m_trigDecorators["HLT_mu14_tau35_medium1_tracktwo_L1TAU20"] =&Props::passHLT_mu14_tau35_medium1_tracktwo_L1TAU20;
      else if ( trigger == "HLT_mu14_tau35_medium1_tracktwo")m_trigDecorators["HLT_mu14_tau35_medium1_tracktwo"] =&Props::passHLT_mu14_tau35_medium1_tracktwo;
      else if ( trigger == "HLT_mu14_iloose_tau25_medium1_tracktwo")m_trigDecorators["HLT_mu14_iloose_tau25_medium1_tracktwo"] =&Props::passHLT_mu14_iloose_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_mu14_iloose_tau35_medium1_tracktwo")m_trigDecorators["HLT_mu14_iloose_tau35_medium1_tracktwo"] =&Props::passHLT_mu14_iloose_tau35_medium1_tracktwo;
      else if ( trigger == "HLT_mu14_ivarloose_tau25_medium1_tracktwo")m_trigDecorators["HLT_mu14_ivarloose_tau25_medium1_tracktwo"] =&Props::passHLT_mu14_ivarloose_tau25_medium1_tracktwo;
      else if ( trigger == "HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1DR-MU10TAU12I_TAU12I-J25")m_trigDecorators["HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1DR-MU10TAU12I_TAU12I-J25"] =&Props::passHLT_mu14_ivarloose_tau25_medium1_tracktwo_L1DR_MU10TAU12I_TAU12I_J25;
      else if ( trigger == "HLT_mu14_ivarloose_tau35_medium1_tracktwo")m_trigDecorators["HLT_mu14_ivarloose_tau35_medium1_tracktwo"] =&Props::passHLT_mu14_ivarloose_tau35_medium1_tracktwo;
      else if ( trigger == "HLT_mu14_ivarloose_tau35_medium1_tracktwo_L1MU10_TAU20IM_J25_2J20")m_trigDecorators["HLT_mu14_ivarloose_tau35_medium1_tracktwo_L1MU10_TAU20IM_J25_2J20"] =&Props::passHLT_mu14_ivarloose_tau35_medium1_tracktwo_L1MU10_TAU20IM_J25_2J20;
      else if ( trigger == "HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12")m_trigDecorators["HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12"] =&Props::passHLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12;
      //
      // 2018 Lep+Tau triggers (begin)
      // tau+e+1J
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF;
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA;
      // tau+e+1J (backup)
      else if ( trigger == "HLT_e24_lhmedium_nod0_ivarloose_tau35_medium1_tracktwoEF")m_trigDecorators["HLT_e24_lhmedium_nod0_ivarloose_tau35_medium1_tracktwoEF"] =&Props::passHLT_e24_lhmedium_nod0_ivarloose_tau35_medium1_tracktwoEF;
      else if ( trigger == "HLT_e24_lhmedium_nod0_ivarloose_tau35_mediumRNN_tracktwoMVA")m_trigDecorators["HLT_e24_lhmedium_nod0_ivarloose_tau35_mediumRNN_tracktwoMVA"] =&Props::passHLT_e24_lhmedium_nod0_ivarloose_tau35_mediumRNN_tracktwoMVA;
      // tau+e+1J (L1Topo)
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1DR-EM15TAU12I-J25")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1DR-EM15TAU12I-J25"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1DR_EM15TAU12I_J25;
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1DR-EM15TAU12I-J25")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1DR-EM15TAU12I-J25"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1DR_EM15TAU12I_J25;
      // tau+e+2J
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1EM15VHI_2TAU12IM_4J12")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1EM15VHI_2TAU12IM_4J12"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1EM15VHI_2TAU12IM_4J12;
      else if ( trigger == "HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1EM15VHI_2TAU12IM_4J12")m_trigDecorators["HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1EM15VHI_2TAU12IM_4J12"] =&Props::passHLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1EM15VHI_2TAU12IM_4J12;
      // tau+mu
      else if ( trigger == "HLT_mu14_ivarloose_tau35_medium1_tracktwoEF")m_trigDecorators["HLT_mu14_ivarloose_tau35_medium1_tracktwoEF"] =&Props::passHLT_mu14_ivarloose_tau35_medium1_tracktwoEF;
      else if ( trigger == "HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA")m_trigDecorators["HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA"] =&Props::passHLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA;
      // tau+mu+1J
      else if ( trigger == "HLT_mu14_ivarloose_tau25_medium1_tracktwoEF")m_trigDecorators["HLT_mu14_ivarloose_tau25_medium1_tracktwoEF"] =&Props::passHLT_mu14_ivarloose_tau25_medium1_tracktwoEF;
      else if ( trigger == "HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA")m_trigDecorators["HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA"] =&Props::passHLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA;
      // tau+mu+1J (backup)
      else if ( trigger == "HLT_mu14_ivarloose_tau35_medium1_tracktwoEF_L1MU10_TAU20IM_J25_2J20")m_trigDecorators["HLT_mu14_ivarloose_tau35_medium1_tracktwoEF_L1MU10_TAU20IM_J25_2J20"] =&Props::passHLT_mu14_ivarloose_tau35_medium1_tracktwoEF_L1MU10_TAU20IM_J25_2J20;
      else if ( trigger == "HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_L1MU10_TAU20IM_J25_2J20")m_trigDecorators["HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_L1MU10_TAU20IM_J25_2J20"] =&Props::passHLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_L1MU10_TAU20IM_J25_2J20;
      // tau+mu+1J (L1Topo)
      else if ( trigger == "HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1DR-MU10TAU12I_TAU12I-J25")m_trigDecorators["HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1DR-MU10TAU12I_TAU12I-J25"] =&Props::passHLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1DR_MU10TAU12I_TAU12I_J25;
      else if ( trigger == "HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1DR-MU10TAU12I_TAU12I-J25")m_trigDecorators["HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1DR-MU10TAU12I_TAU12I-J25"] =&Props::passHLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1DR_MU10TAU12I_TAU12I_J25;
      // tau+mu+2J
      else if ( trigger == "HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12")m_trigDecorators["HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12"] =&Props::passHLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12;
      else if ( trigger == "HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12")m_trigDecorators["HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12"] =&Props::passHLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12;
      //
      // 2018 Lep+Tau triggers (end)
      //
      else if ( trigger == "HLT_e20_lhtight_nod0_ivarloose_3j20_L1EM18VHI_3J20")m_trigDecorators["HLT_e20_lhtight_nod0_ivarloose_3j20_L1EM18VHI_3J20"] =&Props::passHLT_e20_lhtight_nod0_ivarloose_3j20_L1EM18VHI_3J20;
      else if ( trigger == "HLT_mu20_ivarmedium_L1MU10_2J20")m_trigDecorators["HLT_mu20_ivarmedium_L1MU10_2J20"] =&Props::passHLT_mu20_ivarmedium_L1MU10_2J20;
      else if ( trigger == "HLT_mu20_ivarmedium_L1MU10_2J15_J20")m_trigDecorators["HLT_mu20_ivarmedium_L1MU10_2J15_J20"] =&Props::passHLT_mu20_ivarmedium_L1MU10_2J15_J20;
      //	
      else if ( trigger == "L1_J100"  ) m_trigDecorators["L1_J100"  ] = &Props::passL1_J100 ;
      else if ( trigger == "HLT_j15"  ) m_trigDecorators["HLT_j15"  ] = &Props::passHLT_j15 ;
      else if ( trigger == "HLT_j25"  ) m_trigDecorators["HLT_j25"  ] = &Props::passHLT_j25 ;
      else if ( trigger == "HLT_j35"  ) m_trigDecorators["HLT_j35"  ] = &Props::passHLT_j35 ;
      else if ( trigger == "HLT_j45"  ) m_trigDecorators["HLT_j45"  ] = &Props::passHLT_j45 ;
      else if ( trigger == "HLT_j55"  ) m_trigDecorators["HLT_j55"  ] = &Props::passHLT_j55 ;
      else if ( trigger == "HLT_j60"  ) m_trigDecorators["HLT_j60"  ] = &Props::passHLT_j60 ;
      else if ( trigger == "HLT_j85"  ) m_trigDecorators["HLT_j85"  ] = &Props::passHLT_j85 ;
      else if ( trigger == "HLT_j100" ) m_trigDecorators["HLT_j100" ] = &Props::passHLT_j100;
      else if ( trigger == "HLT_j110" ) m_trigDecorators["HLT_j110" ] = &Props::passHLT_j110;
      else if ( trigger == "HLT_j150" ) m_trigDecorators["HLT_j150" ] = &Props::passHLT_j150;
      else if ( trigger == "HLT_j175" ) m_trigDecorators["HLT_j175" ] = &Props::passHLT_j175;
      else if ( trigger == "HLT_j200" ) m_trigDecorators["HLT_j200" ] = &Props::passHLT_j200;
      else if ( trigger == "HLT_j260" ) m_trigDecorators["HLT_j260" ] = &Props::passHLT_j260;
      else if ( trigger == "HLT_j300" ) m_trigDecorators["HLT_j300" ] = &Props::passHLT_j300;
      else if ( trigger == "HLT_j320" ) m_trigDecorators["HLT_j320" ] = &Props::passHLT_j320;
      else if ( trigger == "HLT_j360" ) m_trigDecorators["HLT_j360" ] = &Props::passHLT_j360;
      else if ( trigger == "HLT_j380" ) m_trigDecorators["HLT_j380" ] = &Props::passHLT_j380;
      else if ( trigger == "HLT_j400" ) m_trigDecorators["HLT_j400" ] = &Props::passHLT_j400;
      else if ( trigger == "HLT_j420" ) m_trigDecorators["HLT_j420" ] = &Props::passHLT_j420;
      else if ( trigger == "HLT_j440" ) m_trigDecorators["HLT_j440" ] = &Props::passHLT_j440;
      else if ( trigger == "HLT_j450" ) m_trigDecorators["HLT_j450" ] = &Props::passHLT_j450;
      else if ( trigger == "HLT_j460" ) m_trigDecorators["HLT_j460" ] = &Props::passHLT_j460;
      else if ( trigger == "HLT_noalg_L1J400" ) m_trigDecorators["HLT_noalg_L1J400" ] = &Props::passHLT_noalg_L1J400;
      else if ( trigger == "HLT_j45_bperf" ) m_trigDecorators["HLT_j45_bperf" ] = &Props::passHLT_j45_bperf;
      else if ( trigger == "HLT_j260_a10_lcw_L1J75" ) m_trigDecorators["HLT_j260_a10_lcw_L1J75"] = &Props::passHLT_j260_a10_lcw_L1J75;
      else if ( trigger == "HLT_j260_a10_lcw_nojcalib_L1J75" ) m_trigDecorators["HLT_j260_a10_lcw_nojcalib_L1J75"] = &Props::passHLT_j260_a10_lcw_nojcalib_L1J75;
      else if ( trigger == "HLT_j260_a10_lcw_sub_L1J75" ) m_trigDecorators["HLT_j260_a10_lcw_sub_L1J75"] = &Props::passHLT_j260_a10_lcw_sub_L1J75;
      else if ( trigger == "HLT_j260_a10_nojcalib_L1J75" ) m_trigDecorators["HLT_j260_a10_nojcalib_L1J75"] = &Props::passHLT_j260_a10_nojcalib_L1J75;
      else if ( trigger == "HLT_j260_a10_sub_L1J75" ) m_trigDecorators["HLT_j260_a10_sub_L1J75"] = &Props::passHLT_j260_a10_sub_L1J75;
      else if ( trigger == "HLT_j300_a10_lcw_L1J75" ) m_trigDecorators["HLT_j300_a10_lcw_L1J75"] = &Props::passHLT_j300_a10_lcw_L1J75;
      else if ( trigger == "HLT_j300_a10_sub_L1J75" ) m_trigDecorators["HLT_j300_a10_sub_L1J75"] = &Props::passHLT_j300_a10_sub_L1J75;
      else if ( trigger == "HLT_j300_a10_lcw_L1SC85" ) m_trigDecorators["HLT_j300_a10_lcw_L1SC85"] = &Props::passHLT_j300_a10_lcw_L1SC85;
      else if ( trigger == "HLT_j300_a10_lcw_sub_L1SC85" ) m_trigDecorators["HLT_j300_a10_lcw_sub_L1SC85"] = &Props::passHLT_j300_a10_lcw_sub_L1SC85;
      else if ( trigger == "HLT_j360_a10_lcw_L1J100" ) m_trigDecorators["HLT_j360_a10_lcw_L1J100"] = &Props::passHLT_j360_a10_lcw_L1J100;
      else if ( trigger == "HLT_j380_a10_lcw_L1J100" ) m_trigDecorators["HLT_j380_a10_lcw_L1J100"] = &Props::passHLT_j380_a10_lcw_L1J100;
      else if ( trigger == "HLT_j400_a10_lcw_L1J100" ) m_trigDecorators["HLT_j400_a10_lcw_L1J100"] = &Props::passHLT_j400_a10_lcw_L1J100;
      else if ( trigger == "HLT_j420_a10_lcw_L1J100" ) m_trigDecorators["HLT_j420_a10_lcw_L1J100"] = &Props::passHLT_j420_a10_lcw_L1J100;
      else if ( trigger == "HLT_j440_a10_lcw_L1J100" ) m_trigDecorators["HLT_j440_a10_lcw_L1J100"] = &Props::passHLT_j440_a10_lcw_L1J100;
      else if ( trigger == "HLT_j460_a10_lcw_L1J100" ) m_trigDecorators["HLT_j460_a10_lcw_L1J100"] = &Props::passHLT_j460_a10_lcw_L1J100;
      else if ( trigger == "HLT_j460_a10_lcw_subjes_L1J100" ) m_trigDecorators["HLT_j460_a10_lcw_subjes_L1J100"] = &Props::passHLT_j460_a10_lcw_subjes_L1J100;
      else if ( trigger == "HLT_j460_a10_lcw_nojcalib_L1J100" ) m_trigDecorators["HLT_j460_a10_lcw_nojcalib_L1J100"] = &Props::passHLT_j460_a10_lcw_nojcalib_L1J100;
      else if ( trigger == "HLT_j460_a10_lcw_sub_L1J100" ) m_trigDecorators["HLT_j460_a10_lcw_sub_L1J100"] = &Props::passHLT_j460_a10_lcw_sub_L1J100;
      else if ( trigger == "HLT_j460_a10_nojcalib_L1J100" ) m_trigDecorators["HLT_j460_a10_nojcalib_L1J100"] = &Props::passHLT_j460_a10_nojcalib_L1J100;
      else if ( trigger == "HLT_j460_a10_sub_L1J100" ) m_trigDecorators["HLT_j460_a10_sub_L1J100"] = &Props::passHLT_j460_a10_sub_L1J100;
      else if ( trigger == "HLT_j480_a10_lcw_subjes_L1J100" ) m_trigDecorators["HLT_j480_a10_lcw_subjes_L1J100"] = &Props::passHLT_j480_a10_lcw_subjes_L1J100;
      else if ( trigger == "HLT_j260_a10r_L1J75" ) m_trigDecorators["HLT_j260_a10r_L1J75"] = &Props::passHLT_j260_a10r_L1J75;
      else if ( trigger == "HLT_j300_a10r_L1J75" ) m_trigDecorators["HLT_j300_a10r_L1J75"] = &Props::passHLT_j300_a10r_L1J75;
      else if ( trigger == "HLT_j360_a10r_L1J100" ) m_trigDecorators["HLT_j360_a10r_L1J100"] = &Props::passHLT_j360_a10r_L1J100;
      else if ( trigger == "HLT_j380_a10r_L1J100" ) m_trigDecorators["HLT_j380_a10r_L1J100"] = &Props::passHLT_j380_a10r_L1J100;
      else if ( trigger == "HLT_j400_a10r_L1J100" ) m_trigDecorators["HLT_j400_a10r_L1J100"] = &Props::passHLT_j400_a10r_L1J100;
      else if ( trigger == "HLT_j420_a10r_L1J100" ) m_trigDecorators["HLT_j420_a10r_L1J100"] = &Props::passHLT_j420_a10r_L1J100;
      else if ( trigger == "HLT_j440_a10r_L1J100" ) m_trigDecorators["HLT_j440_a10r_L1J100"] = &Props::passHLT_j440_a10r_L1J100;
      else if ( trigger == "HLT_j460_a10r_L1J100" ) m_trigDecorators["HLT_j460_a10r_L1J100"] = &Props::passHLT_j460_a10r_L1J100;
      else if ( trigger == "HLT_j480_a10r_L1J100" ) m_trigDecorators["HLT_j480_a10r_L1J100"] = &Props::passHLT_j480_a10r_L1J100;
      // trigger objects
      //jet and photon only for HIGG5D1/3
      else if ( trigger == "HLT_Jet"      ) { if ( m_useTrigObj && (m_derivationName == "HIGG5D1" || m_derivationName == "HIGG5D3" || m_derivationName == "FTAG1")) m_trigObjects.push_back( new HLTJet      ("HLT_xAOD__JetContainer_a4tcemsubjesFS"           ) ); }
      else if ( trigger == "HLT_Photon"   ) { if ( m_useTrigObj && (m_derivationName == "HIGG5D1" || m_derivationName == "HIGG5D3")) m_trigObjects.push_back( new HLTPhoton   ("HLT_xAOD__PhotonContainer_egamma_Photons"        ) ); }
      else if ( trigger == "HLT_Muon"     ) { if ( m_useTrigObj ) m_trigObjects.push_back( new HLTMuon     ("HLT_xAOD__MuonContainer_MuonEFInfo"              ) ); }
      else if ( trigger == "HLT_Electron" ) { if ( m_useTrigObj ) m_trigObjects.push_back( new HLTElectron ("HLT_xAOD__ElectronContainer_egamma_Electrons"    ) ); }
      else if ( trigger == "HLT_MET"      ) { if ( m_useTrigObj ) m_trigObjects.push_back( new HLTMet      ("HLT_xAOD__TrigMissingETContainer_TrigEFMissingET") ); }
      else if ( trigger == "L1_EM"        ) { if ( m_useTrigObj ) m_trigObjects.push_back( new L1EM        ("LVL1EmTauRoIs"                                   ) ); } 
      else if ( trigger == "L2_Photon"    ) { if ( m_useTrigObj ) m_trigObjects.push_back( new L2Photon    ("HLT_xAOD__TrigPhotonContainer_L2PhotonFex"       ) ); }
      else if ( trigger == "L1_MET"       ) { if ( m_useTrigObj ) m_trigObjects.push_back( new L1Met       ("LVL1EnergySumRoI"                                ) ); } 
      else if ( trigger == "L1_Jet"       ) { if ( m_useTrigObj ) m_trigObjects.push_back( new L1Jet       ("LVL1JetRoIs"                                     ) ); }
      else {
        Error("EventInfoHandler::decorateTrigDec()","No decorator for trigger %s defined! Need to create one.", trigger.c_str());
        return EL::StatusCode::FAILURE;
      }
    }    
    //xAODConfigTool
    m_trigConfigTool = new TrigConf::xAODConfigTool ("xAODConfigTool");
    m_trigConfigTool->msg().setLevel( m_msgLevel );
    TOOL_CHECK("EventInfoHandler::initialize()",m_trigConfigTool->initialize());
    ToolHandle<TrigConf::ITrigConfigTool> handle(m_trigConfigTool);
    TOOL_CHECK("EventInfoHandler::initialize()",handle->initialize());
    //TrigDecisionTool
    m_trigDecTool = new Trig::TrigDecisionTool("TrigDecisionTool");
    m_trigDecTool->msg().setLevel( m_msgLevel );
    TOOL_CHECK("EventInfoHandler::initialize()",m_trigDecTool->setProperty("ConfigTool",handle));
    TOOL_CHECK("EventInfoHandler::initialize()",m_trigDecTool->setProperty("TrigDecisionKey","xTrigDecision"));
    TOOL_CHECK("EventInfoHandler::initialize()",m_trigDecTool->setProperty("OutputLevel", m_msgLevel));
    TOOL_CHECK("EventInfoHandler::initialize()",m_trigDecTool->initialize());
  }

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode EventInfoHandler::addVariations(const std::vector<TString> &variations)
{

  // here we filter the variations that affect the event info
  for (TString variation : variations) {
    // TODO: check for duplications? maybe use set...    
    if (variation.EqualTo("Nominal")) continue;
    if (variation.EqualTo("Original")) continue;
    // allow scale factors
    bool allow = false;
    allow |= variation.Contains("MUON_EFF");
    allow |= variation.Contains("EL_EFF");
    allow |= variation.Contains("TAUS_TRUEHADTAU_EFF");
    allow |= variation.Contains("MUON_TTVA");
    allow |= variation.Contains("MUON_ISO");
    allow |= variation.Contains("PRW_DATASF");
    allow |= variation.BeginsWith("JET_JvtEfficiency");
    if (!allow) continue;
    m_variations.push_back(variation);
  }

  return EL::StatusCode::SUCCESS;
}

std::vector<TString> EventInfoHandler::getAllVariations() {
  return m_variations;
}

EL::StatusCode EventInfoHandler::executeEvent() 
{
 
  if (m_debug) {
    Info("EventInfoHandler::executeEvent()", "Called.");
  }

  // the output containers are not initialized when this function is called
  // (since the isMC flag is needed for that)
  
  // TODO should have more generic systematic handling, e.g. derive from EventObjectHandler
  CP::SystematicSet sysSetNominal;
  
  //----------------------------
  // Event information
  //--------------------------- 

  // retrieve event info from TEvent
  //---------------------------------
  const xAOD::EventInfo * eventInfoIn = 0;
  if ( ! m_event->retrieve(eventInfoIn, "EventInfo").isSuccess() ) {
    Error("EventInfoHandler::executeEvent()", "Failed to retrieve event info collection. Exiting.");
    return EL::StatusCode::FAILURE;
  }
  std::pair< xAOD::EventInfo *, xAOD::ShallowAuxInfo *> sc_eventInfo = shallowCopyObject(*eventInfoIn);
  m_eventInfoIn = sc_eventInfo.first; 
  
  // check if MC or data - agrees with information retrieved from MetaData?
  //------------------------
  //m_isMC is retrieved from the MetaData in AnalysisBase and passed to the EventInfoHandler
  int isMC = static_cast<int>(m_eventInfoIn->eventType(xAOD::EventInfo::IS_SIMULATION));
  if (isMC !=  m_isMC) {
    Error("EventInfoHandler::executeEvent()", "Mismatch: MetaData has different isMC than the EventInfo! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  Props::isMC.set(m_eventInfoIn, m_isMC);// use a custom decorator for isMC to avoid bit map

  //if (m_debug) std::cout << "HH: tjs: ismc: " << m_isMC << std::endl;

  ////pile-up weight (MC) and mu correction (Data)
  // TODO add SYS_CHECK or so to ReturnCheck.h?
  if (m_puReweightingTool -> applySystematicVariation(sysSetNominal) != CP::SystematicCode::Ok) {
    return EL::StatusCode::FAILURE;
  }
  EL_CHECK("EventInfoHandler::executeEvent()", m_puReweightingTool->decorate(m_eventInfoIn));//new
  //EL_CHECK("EventInfoHandler::executeEvent()", m_puReweightingTool->decorateWeight(m_eventInfoIn));//to remove

  if ( m_isMC ) {    
    Props::MCEventWeight.set(m_eventInfoIn, eventInfoIn->mcEventWeight(0));// use a custom decorator for mcEventWeight, since the original info is a vector
    Props::MCEventWeightSys.set(m_eventInfoIn, eventInfoIn->mcEventWeights());
    // electron efficiency tool needs the random runnumber on the original container... should be changed soon (17/6/16) ... still true (30/1/17):
    Props::RandomRunNumber.set(eventInfoIn, Props::RandomRunNumber.get(m_eventInfoIn));

  } else { 
    //TODO readout stream
    // for( const auto& st :  m_eventInfoIn->streamTags() ) {
    //   std:: string streamname = st.name();
    //   st.type();
    //   std::cout<<"EventInfoHandler Execute: Stream "<<streamname<<std::endl;
    // }

    //----------
    //GRL:         check if GRL is passed and decorate m_eventInfoIn
    //----------
    int passGRL = true;
    if (!m_grlSelectionTool->passRunLB(*eventInfoIn)) passGRL = false;
    Props::passGRL.set(m_eventInfoIn, passGRL);

    //--------------
    //Event cleaning:  check if event is clean and decorate m_eventInfoIn
    //https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PhysicsAnalysisWorkBookRel20CPRec#Event_cleaning
    //--------------
    int isCleanEvent = true;
    if ((eventInfoIn->errorState(xAOD::EventInfo::EventFlagSubDet::Tile) == xAOD::EventInfo::Error)
	|| (eventInfoIn->errorState(xAOD::EventInfo::EventFlagSubDet::LAr) == xAOD::EventInfo::Error) 
	|| (eventInfoIn->errorState(xAOD::EventInfo::EventFlagSubDet::SCT) == xAOD::EventInfo::Error)
	|| (eventInfoIn->isEventFlagBitSet(xAOD::EventInfo::Core,18))) {
      isCleanEvent = false;
    }
    Props::isCleanEvent.set(m_eventInfoIn, isCleanEvent);
  }

  //---------------
  //Trigger info:   retrieve information and decorate m_eventInfoIn      
  //---------------
  // Set TriggerStream information
  Props::TriggerStream.set(m_eventInfoIn, m_triggerStream); // What is this, always -1?

  m_config.getif<bool>("METtrigEmul",m_METtrigEmul);

  //get trigger decision of triggers defined in config
  for (auto& dec : m_trigDecorators) {
    int triggerIsPassed = static_cast<int>(m_trigDecTool->isPassed(dec.first));

    //HLT_xe100_mht_L1XE50, HLT_xe110_mht_L1XE50_AND_xe70_L1XE50, HLT_xe110_mht_L1XE50_AND_xe70_L1XE50 emulation 
    if(m_isMC && m_METtrigEmul && (dec.first.find("HLT_xe110_mht_L1XE50") != std::string::npos)){
        auto chainGroup = m_trigDecTool->getChainGroup(dec.first.c_str());
        if(chainGroup->getListOfTriggers().size() == 0) triggerIsPassed = METtrigEmulation(dec.first);
    }

	// vbfg trigger emulation: HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000 , @Bo do not need to emulate
//	if(m_isMC && (dec.first=="HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000")){
//	   const xAOD::JetContainer* jets = 0;
//	   if ( ! m_event->retrieve(jets, "HLT_xAOD__JetContainer_a4tcemsubjesFS").isSuccess() ) {
//		  Error("EventInfoHandler::executeEvent()", "VBFgGammaTriggerEmulation: Failed to retrieve HLT Jet collection. Please remove HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000 from trigger list.");
//		  return EL::StatusCode::FAILURE;
//	   } 
//	   double hlt_mjj=0;
//	   for(unsigned int i=0; i<jets->size(); ++i){
//		  for(unsigned int j=i+1; j<jets->size(); ++j){
//			 double tmp_mjj = (jets->at(i)->p4() + jets->at(j)->p4()).M();
//			 if(tmp_mjj>hlt_mjj) hlt_mjj = tmp_mjj;
//		  }
//	   }
//	   triggerIsPassed = static_cast<int>(m_trigDecTool->isPassed("HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700"));
//	   if(hlt_mjj<1000000){
//		  triggerIsPassed = false;
//		  if(m_debug) Info("EventInfoHandler::executeEvent()","VBFgTrigggerEmulation: %s failed with HLT_Mjj = %f GeV. Trigger flag updated.",dec.first.c_str(), hlt_mjj*0.001);
//	   }
//	}
	// end of vbfg trigger emulation

    dec.second->set(m_eventInfoIn, triggerIsPassed);
    if(m_debug) Info("EventInfoHandler::executeEvent()","Decision for trigger %s: %i",dec.first.c_str(),triggerIsPassed); 
  }


  if(m_debug && m_triggerList.size()){
    //print list of triggers in inpput file
    Info("EventInfoHandler::executeEvent()","list of triggers in input file :");
    auto chainGroup = m_trigDecTool->getChainGroup(".*");
    for(auto &trig : chainGroup->getListOfTriggers()){
      Info("EventInfoHandler::executeEvent()",trig.c_str());
    }
  }

  //--------------
  //Vertex info:   retrieve information and decorate m_eventInfoIn 
  //--------------

  // primary vertex
  const xAOD::Vertex * primVtx = 0;
  int nVtx2Trks = 0;
  // get the vertex container
  const xAOD::VertexContainer * primVtxCont = 0;
  if ( ! m_event->retrieve(primVtxCont, "PrimaryVertices").isSuccess() ) {
    Error("EventInfoHandler::executeEvent()", "Failed to retrieve primary vertices collection. Exiting.");
    return EL::StatusCode::FAILURE;
  } 

  // find the primary vertex and count the number of vertices with at least 2 tracks
  int indexPV = 0;
  for (const xAOD::Vertex * vtx : *primVtxCont) {
    //ask for type - no need to ask for at least two associated tracks anymore
    if ( vtx->vertexType() == xAOD::VxType::PriVtx ) {
      primVtx = vtx;
      m_indexPV = indexPV;
    }
    if ( vtx->nTrackParticles() >= 2 ) ++nVtx2Trks;
    indexPV++;
  }

  // check if primary vertex is found
  float ZPV = -9999;
  if ( ! primVtx ) {
    Warning("EventInfoHandler::executeEvent()", "Couldn't find a primary vertex in this event!");
  } 
  else {
    ZPV = primVtx->z();
  }
  Props::hasPV.set(m_eventInfoIn, primVtx != 0);
  Props::ZPV.set(m_eventInfoIn, ZPV);
  Props::NVtx2Trks.set(m_eventInfoIn, nVtx2Trks);

  // trigger objects
  for (TrigObjects * trigObjects : m_trigObjects) {
    if ( ! trigObjects->process(m_event, m_eventInfoIn, m_allowTrigObjFail) ) {
      Error("EventInfoHandler::executeEvent()","%s::process() returned 'false'! Exiting.", (trigObjects->name()).c_str());
      return EL::StatusCode::FAILURE;
    }
  }

  Props::triggerMatched.set(m_eventInfoIn, 0);

  //retrieve rho (event shape)
  const xAOD::EventShape* RhoEM = 0;
  //migration to 2.6.X
  int releaseSeries = atoi(getenv("ROOTCORE_RELEASE_SERIES"));
  if(releaseSeries>=25) {
    if( m_event->retrieve( RhoEM, m_isPFlow?"Kt4EMPFlowEventShape":"Kt4EMTopoOriginEventShape" ).isFailure() ) {
      Error("EventInfoHandler::executeEvent()","No Rho (event shape) object could be retrieved. Failure.");
	    return EL::StatusCode::FAILURE;
    }
  } else {
    if( m_event->retrieve( RhoEM, "Kt4EMTopoEventShape" ).isFailure() ) {
      Error("EventInfoHandler::executeEvent()","No Rho (event shape) object could be retrieved. Failure.");
	    return EL::StatusCode::FAILURE;
    }
  }
  float RhoEventShape = RhoEM->auxdata<float>("Density");
  if(m_debug) std::cout<<"RhoEventShape="<<RhoEventShape<<std::endl;
  Props::RhoEventShape.set(m_eventInfoIn,RhoEventShape);
  m_firstEvent++;

  return EL::StatusCode::SUCCESS;
  
}

//--------------                                                                                                                                                              
//number of Truth jets, needed to reweight Sherpa 2.2, we store it for all MC samples, though
//-------------- 
EL::StatusCode EventInfoHandler::set_NTruthWZJets(const xAOD::JetContainer* truthWZJets) {
  if(m_debug) { std::cout<<"EventInfoHandler::set_NTruthWZJets for a collection of "<<truthWZJets->size()<<" size."<<std::endl;}
  assert(m_isMC);//in AnalysisBase.cxx the function is called only for MC events
  int nTruthWZJets20 = 0;
  int nTruthWZJets30 = 0;
  //loop over all truth jets
  for (unsigned i=0; i!=truthWZJets->size(); i++) {
    const xAOD::Jet* truthJet = (*truthWZJets)[i];
    if(m_debug) {std::cout<<"Index "<<i<<" truthJet pointer = "<<truthJet<<std::endl;}
    //remove hadronic tau jets from the counting, as per twiki
    //https://svnweb.cern.ch/trac/atlasoff/browser/PhysicsAnalysis/AnalysisCommon/PMGTools/tags/PMGTools-00-00-02/Root/PMGSherpa22VJetsWeightTool.cxx#L121
    // Remove hadronic taus e.g. from Ztautau, Wtaunu decays
    static const SG::AuxElement::ConstAccessor<int> acc( "HadronConeExclTruthLabelID" );
    if(acc.isAvailable(*truthJet) && (acc(*truthJet) == 15) ) {
      continue;
    }
    //start counting in different kinematic regimes
    double Pt=truthJet->pt();
    double AbsEta=fabs(truthJet->eta());
    if (Pt>20000.0 && AbsEta<4.5) {nTruthWZJets20++;}
    if (Pt>30000.0 && AbsEta<4.5) {nTruthWZJets30++;}
  }//end loop over truth jets
  if(m_debug) {
    std::cout<<"nTruthWZJets20="<<nTruthWZJets20<<std::endl;
    std::cout<<"nTruthWZJets30="<<nTruthWZJets30<<std::endl;
  }
  //store into properties
  Props::NTruthWZJets20.set(m_eventInfoIn, nTruthWZJets20);
  Props::NTruthWZJets30.set(m_eventInfoIn, nTruthWZJets30);
  //return
  return EL::StatusCode::SUCCESS;
}

void EventInfoHandler::setJets(JetHandler* jets) {
  m_jetHandler = jets;
}

EL::StatusCode EventInfoHandler::set_JvtSF() {  
  if(m_debug) Info("EventInfoHandler::set_JvtSF()","Start method");
  if(!m_isMC) {
    Error("EventInfoHandler::set_JvtSF()", "You are running on data (isMC=false). Exiting."); 
    return EL::StatusCode::FAILURE;
  }
  if(!m_jetHandler) {
    Error("EventInfoHandler::set_JvtSF()", "jetHandler is not set in the EventInfoHandler. Exiting."); 
    return EL::StatusCode::FAILURE;
  }
  TString sysName="";
  CP::SystematicVariation sysVar(sysName.Data());
  CP::SystematicSet sysSet;
  sysSet.insert(sysVar);
  float JVTeventSF=-99.9;
  EL_CHECK("EventInfoHandler::set_JvtSF",m_jetHandler->getJVTeventSF(sysSet,JVTeventSF));
  if(m_debug) Info("EventInfoHandler::set_JvtSF()","For variation=%s JVTeventSF=%f",sysName.Data(),JVTeventSF);
  Props::JvtSF.set(m_eventInfoIn, JVTeventSF);
  if(m_debug) Info("EventInfoHandler::set_JvtSF()","End method");
  //return                       
  return EL::StatusCode::SUCCESS;
}


EL::StatusCode EventInfoHandler::setCodeTTBarDecay(int code) {
  Props::codeTTBarDecay.set(m_eventInfoIn, code);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setTTbarpTW(float ptw) {
  Props::ttbarpTW.set(m_eventInfoIn, ptw);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setTTbarpTL(float ptl) { 
  Props::ttbarpTL.set(m_eventInfoIn, ptl);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setTTbarEtaL(float etal) {
  Props::ttbarEtaL.set(m_eventInfoIn, etal);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setSherpapTV(float ptw) {
  Props::SherpapTV.set(m_eventInfoIn, ptw);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::isVZbb(bool isVZbb){
  Props::isVZbb.set(m_eventInfoIn, isVZbb);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setSUSYMET() {
  float SUSYMET = Props::GenFiltMET.exists(m_eventInfoIn) ? Props::GenFiltMET.get(m_eventInfoIn) : -999;

  if (SUSYMET){ 
    Props::SUSYMET.set(m_eventInfoIn, SUSYMET);
  }
  else{
    Props::SUSYMET.set(m_eventInfoIn, -999);
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setForHadHad(int isForHadHad) {
  Props::isForHadHad.set(m_eventInfoIn, isForHadHad);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setForFourTauOnly(int isForFourTauOnly) {
  Props::isForFourTauOnly.set(m_eventInfoIn, isForFourTauOnly);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::setOutputContainer() {
  
  if (m_debug) {
    Info("EventInfoHandler::setOutputContainer()", "Called.");
  }

  // create containers
  xAOD::EventInfo * eventInfoOutNominal = new xAOD::EventInfo();
  xAOD::AuxInfoBase * eventInfoOutNominalAux = new xAOD::AuxInfoBase();
  eventInfoOutNominal->setStore(eventInfoOutNominalAux);
  
  m_eventInfosOut["Nominal"] = eventInfoOutNominal;
  
  // record event info (yes, can be done before setting the actual values)
  if ( ! m_event->record(eventInfoOutNominal, "EventInfo___Nominal") ) {
    return EL::StatusCode::FAILURE;
  }
  if ( ! m_event->record(eventInfoOutNominalAux, "EventInfo___NominalAux.") ) {
    return EL::StatusCode::FAILURE;
  }

  // create a shallow copies e.g. for systematic variations of the event weights
  for (TString variation : m_variations) {
    std::pair< xAOD::EventInfo *, xAOD::ShallowAuxInfo *> eventInfoOutSC = shallowCopyObject(*eventInfoOutNominal);
    if ( ! m_event->record(eventInfoOutSC.first, ("EventInfo___" + variation).Data())) {
      return EL::StatusCode::FAILURE;
    }
    if ( ! m_event->record(eventInfoOutSC.second, ("EventInfo___" + variation + "Aux.").Data())) {
      return EL::StatusCode::FAILURE;
    }
    
    m_eventInfosOut[variation] = eventInfoOutSC.first;
  }
  return EL::StatusCode::SUCCESS;
}


void EventInfoHandler::writeOutputVariables(xAOD::EventInfo* eventInfoIn, xAOD::EventInfo* eventInfoOut) {
  eventInfoOut->setRunNumber(eventInfoIn->runNumber());
  eventInfoOut->setLumiBlock(eventInfoIn->lumiBlock());
  eventInfoOut->setEventTypeBitmask(eventInfoIn->eventTypeBitmask());
  eventInfoOut->setEventNumber(eventInfoIn->eventNumber());

  //Pile-up weight and mu
  Props::PileupReweight.copy(eventInfoIn, eventInfoOut);
  Props::RandomRunNumber.copy(eventInfoIn, eventInfoOut);
  Props::averageInteractionsPerCrossing.copy(eventInfoIn, eventInfoOut);
  Props::actualInteractionsPerCrossing.copy(eventInfoIn, eventInfoOut);
  Props::CorrectedAvgMu.copy(eventInfoIn, eventInfoOut);
  Props::CorrectedAndScaledAvgMu.copy(eventInfoIn, eventInfoOut);
  Props::CorrectedMu.copy(eventInfoIn, eventInfoOut);
  Props::CorrectedAndScaledMu.copy(eventInfoIn, eventInfoOut);
  Props::triggerMatched.copy(eventInfoIn, eventInfoOut);

  Props::isMC.copy(eventInfoIn, eventInfoOut);
  for (auto& dec : m_trigDecorators) {
    dec.second->copy(eventInfoIn, eventInfoOut);
  }

  // copy trigger objects to output
  for (TrigObjects * trigObjects : m_trigObjects) {
    if ( ! trigObjects->copy(eventInfoIn, eventInfoOut, m_allowTrigObjFail) ) {
      Error("EventInfoHandler::writeOutputVariables()","%s::copy() returned 'false'! Exiting.", (trigObjects->name()).c_str());
      exit(EXIT_FAILURE);
    }
  }

  if ( m_isMC ) {
    eventInfoOut->setMCEventNumber(eventInfoIn->mcEventNumber());
    eventInfoOut->setMCChannelNumber(eventInfoIn->mcChannelNumber());
    Props::MCEventWeight.copy(eventInfoIn, eventInfoOut);
    Props::MCEventWeightSys.copy(eventInfoIn, eventInfoOut);
    Props::codeTTBarDecay.copyIfExists(eventInfoIn, eventInfoOut);
    Props::ttbarpTW.copyIfExists(eventInfoIn, eventInfoOut);
    Props::ttbarpTL.copyIfExists(eventInfoIn, eventInfoOut);
    Props::ttbarEtaL.copyIfExists(eventInfoIn, eventInfoOut);
    Props::SherpapTV.copyIfExists(eventInfoIn, eventInfoOut);
    Props::isVZbb.copyIfExists(eventInfoIn, eventInfoOut);
    Props::SUSYMET.copyIfExists(eventInfoIn, eventInfoOut);
    Props::HTXS_V_jets25_pt.copyIfExists(eventInfoIn, eventInfoOut);
    Props::HTXS_V_jets25_eta.copyIfExists(eventInfoIn, eventInfoOut);
    Props::HTXS_V_jets25_phi.copyIfExists(eventInfoIn, eventInfoOut);
    Props::HTXS_V_jets25_m.copyIfExists(eventInfoIn, eventInfoOut);
  } else {
    Props::passGRL.copy(eventInfoIn, eventInfoOut);
    Props::isCleanEvent.copy(eventInfoIn, eventInfoOut);
  }

  Props::isForHadHad.copyIfExists(eventInfoIn, eventInfoOut);
  Props::isForFourTauOnly.copyIfExists(eventInfoIn, eventInfoOut);

  Props::DFCommonJets_eventClean_LooseBad.copy(eventInfoIn, eventInfoOut); // new event clean variable, see twikihttps://twiki.cern.ch/twiki/bin/view/AtlasProtected/HowToCleanJets2017
  Props::DFCommonJets_isBadBatman.copyIfExists(eventInfoIn, eventInfoOut); // Batman flag details (https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/HowToCleanJets2016#IsBadBatMan_Event_Flag_and_EMEC)
  Props::NTruthWZJets20.copyIfExists(eventInfoIn, eventInfoOut);
  Props::NTruthWZJets30.copyIfExists(eventInfoIn, eventInfoOut);
  Props::JvtSF.copyIfExists(eventInfoIn, eventInfoOut);
  Props::RhoEventShape.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_prodMode.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Stage0_Category.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Stage1_Category_pTjet25.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Stage1_Category_pTjet30.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Njets_pTjet25.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Njets_pTjet30.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Higgs_pt.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Higgs_eta.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Higgs_phi.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_Higgs_m.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_V_pt.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_V_eta.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_V_phi.copyIfExists(eventInfoIn, eventInfoOut);
  Props::HTXS_V_m.copyIfExists(eventInfoIn, eventInfoOut);
  Props::hasPV.copy(eventInfoIn, eventInfoOut);
  Props::ZPV.copy(eventInfoIn, eventInfoOut);
  Props::NVtx2Trks.copy(eventInfoIn, eventInfoOut);

}

EL::StatusCode EventInfoHandler::writeVariation(xAOD::EventInfo* eventInfoIn, xAOD::EventInfo* eventInfoOut, TString sysName) {
  if (m_debug) {
    Info("EventInfoHandler::writeVariation","sysName=%s",sysName.Data());  
  }

  CP::SystematicVariation sysVar(sysName.Data());
  CP::SystematicSet sysSet;
  sysSet.insert(sysVar);
  
  if (sysName.BeginsWith("PRW_DATASF")) {
    // TODO create shallow copies for input event info. Currently we are forced to do this:
    float nominalPUW = Props::PileupReweight.get(eventInfoIn);
    if (m_puReweightingTool -> applySystematicVariation(sysSet) != CP::SystematicCode::Ok) {
      return EL::StatusCode::FAILURE;
    }
    EL_CHECK("EventInfoHandler::writeVariation()", m_puReweightingTool -> decorate(eventInfoIn));
    Props::PileupReweight.copy(eventInfoIn, eventInfoOut);
    Props::PileupReweight.set(eventInfoIn, nominalPUW);
  }

  if(!m_isMC) {
    Error("EventInfoHandler::EventInfoHandler::writeVariation()", "You are running on data (isMC=false). Exiting."); 
    return EL::StatusCode::FAILURE;
  }
  if (sysName.BeginsWith("JET")) {
    if(m_debug) Info("EventInfoHandler::set_JvtSF()","JET systematic=%s",sysName.Data());
    if(m_debug) Info("EventInfoHandler::set_JvtSF()","m_isMC=%i",(int)m_isMC);
    float JVTeventSF=-99.9;
    EL_CHECK("EventInfoHandler::writeVariation",m_jetHandler->getJVTeventSF(sysSet,JVTeventSF));
    if(m_debug) Info("EventInfoHandler::set_JvtSF()","JVTeventSF=%f",JVTeventSF);
    Props::JvtSF.set(eventInfoIn, JVTeventSF);
    Props::JvtSF.copy(eventInfoIn, eventInfoOut);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode EventInfoHandler::fillOutputContainer() 
{
  
  // write variables for Nominal
  xAOD::EventInfo* eventInfoOut = getOutEventInfoVariation("Nominal");

  if (!eventInfoOut) {
    Error("EventInfoHandler::fillOutputContainer()", "No Nominal output container defined!");
    return EL::StatusCode::FAILURE;
  }
  
  writeOutputVariables(m_eventInfoIn, eventInfoOut);
  
  // write systematic variations
  // note: more variations/variables can be written in EventSelection::setEventVariables()
  for (TString variation : m_variations) {
    eventInfoOut = getOutEventInfoVariation(variation);
    EL_CHECK("EventInfoHandler::fillOutputContainer()",writeVariation(m_eventInfoIn, eventInfoOut, variation));
  }

  return EL::StatusCode::SUCCESS;

}

xAOD::EventInfo * EventInfoHandler::getOutEventInfoVariation(const TString &variation, bool fallbackToNominal) {

  if (m_debug) {
    Info("METHandler::getOutEventInfoVariation()", "Called for variation '%s'.", variation.Data());
  }

  if (m_eventInfosOut.count(variation)) {
    return m_eventInfosOut[variation];
  }
  if (fallbackToNominal && m_eventInfosOut.count("Nominal")) {
    return m_eventInfosOut["Nominal"];
  }
  return nullptr;
}

EL::StatusCode EventInfoHandler::clearEvent()
{
  
  delete m_eventInfoIn->getConstStore();
  delete m_eventInfoIn;
  m_eventInfoIn = nullptr;
  // clear output map
  m_eventInfosOut.clear();

  return EL::StatusCode::SUCCESS;

}

int EventInfoHandler::METtrigEmulation(const TString &triggerName) 
{

    //check L1 MET
    bool passL1XE50 = m_trigDecTool->isPassed("L1_XE50");
    bool passL1XE55 = m_trigDecTool->isPassed("L1_XE55");
    
    //retrive HLTMET_mht
    const xAOD::TrigMissingETContainer * mhtCont(0);
    if ( ! m_event->retrieve(mhtCont, "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht").isSuccess() ) {
        Error("EventInfoHandler::executeEvent()", "METtrigEmulation: Failed to retrieve HLT mht MET collection. Exiting. Turn off emulation by setting METtrigEmul = false.");
        return EL::StatusCode::FAILURE;
    } 

    float mht_ex = 0.; // xAOD information is stored in MeV, trigger thresholds are measured in GeV
    float mht_ey = 0.;

  if ( mhtCont->size() > 0 ) {
    mht_ex = mhtCont->front()->ex() * 0.001; // xAOD information is stored in MeV, trigger thresholds are measured in GeV
    mht_ey = mhtCont->front()->ey() * 0.001;
  }

    float mht_Met = sqrt(mht_ex*mht_ex + mht_ey*mht_ey);
    bool passHLT_xe110_mht = mht_Met > 110;

    //retrive HLTMET_cell
    const xAOD::TrigMissingETContainer * cellCont(0);
    if ( ! m_event->retrieve(cellCont, "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET").isSuccess() ) {
        Error("EventInfoHandler::executeEvent()", "METtrigEmulation: Failed to retrieve HLT MET collection. Exiting. Turn off emulation by setting METtrigEmul = false.");
        return EL::StatusCode::FAILURE;
    } 

    float cell_ex = 0.; 
    float cell_ey = 0.;
  if ( cellCont->size() > 0 ) {
    cell_ex = cellCont->front()->ex() * 0.001; 
    cell_ey = cellCont->front()->ey() * 0.001;
  }

    float cell_Met = sqrt(cell_ex*cell_ex + cell_ey*cell_ey);
    bool passHLT_xe70 = cell_Met > 70;

    if(triggerName =="HLT_xe110_mht_L1XE50" && passL1XE50 && passHLT_xe110_mht) return 1;
    else if(triggerName =="HLT_xe110_mht_L1XE50_AND_xe70_L1XE50" && passL1XE50 && passHLT_xe110_mht && passHLT_xe70) return 1;
    else if(triggerName =="HLT_xe110_mht_L1XE50_AND_xe70_L1XE55" && (passL1XE50 && passHLT_xe110_mht) && (passL1XE55 && passHLT_xe70)) return 1;
    else return 0;

}
