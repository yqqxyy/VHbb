#include <iostream>
#include "stdlib.h"

#include "CxAODMaker/PhotonHandler.h"
#include "CxAODMaker/EventInfoHandler.h"

#include "IsolationSelection/IsolationSelectionTool.h"
#include "IsolationCorrections/IsolationCorrectionTool.h"
#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"
#include "ElectronPhotonSelectorTools/egammaPIDdefs.h"
#include "ElectronPhotonSelectorTools/AsgPhotonIsEMSelector.h"
#include "ElectronPhotonShowerShapeFudgeTool/ElectronPhotonShowerShapeFudgeTool.h"
#include "PathResolver/PathResolver.h"
#include "PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h"
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "ElectronPhotonSelectorTools/EGammaAmbiguityTool.h"
#include "TriggerMatchingTool/MatchingTool.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
 #include "ElectronPhotonSelectorTools/egammaPIDdefs.h"

#include "xAODTruth/xAODTruthHelpers.h"

PhotonHandler::PhotonHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                             EventInfoHandler & eventInfoHandler) :
  ObjectHandler(name, config, event, eventInfoHandler),
  m_EgammaCalibrationAndSmearingTool(nullptr),
  m_photonLooseIsEMSelector(nullptr),
  m_photonMediumIsEMSelector(nullptr),
  m_photonTightIsEMSelector(nullptr),
  m_photonFudgeMCTool(nullptr),
  m_photonLooseEffTool(nullptr),
  m_photonMediumEffTool(nullptr),
  m_photonTightEffTool(nullptr),
  m_photonIsoTightEffTool(nullptr),
  m_trigDecTool(nullptr),
  m_triggersToMatch(),
  m_egammaAmbiguityTool(nullptr),
  m_isIsoPh(nullptr),
  m_isoCorr_tool(nullptr),
  m_truthhandler(nullptr)
{
}

PhotonHandler::~PhotonHandler()
{
  delete m_EgammaCalibrationAndSmearingTool;
  delete m_photonLooseIsEMSelector;
  delete m_photonMediumIsEMSelector;
  delete m_photonTightIsEMSelector;
  delete m_photonFudgeMCTool;
  delete m_photonLooseEffTool;
  delete m_photonMediumEffTool;
  delete m_photonTightEffTool;
  delete m_photonIsoTightEffTool;
  delete m_isIsoPh;
  delete m_isoCorr_tool;
  delete m_egammaAmbiguityTool;
}

EL::StatusCode PhotonHandler::initializeTools()
{

  // todo : fullsim / af2
  bool isFullSim = !m_eventInfoHandler.get_isAFII();
  int FullSim_AFII_flag = isFullSim? 0 :1 ;
  // todo : 0 for data, 1 for fullsum, 3 for AF2 <-- Jake: Twiki recommends this
  int dataType = 1;
  //int dataType = PATCore::ParticleDataType::Full;

  //calibration tool
  //----------------
  m_EgammaCalibrationAndSmearingTool = new CP::EgammaCalibrationAndSmearingTool("EgammaCalibrationAndSmearingTool_Photons");
  m_EgammaCalibrationAndSmearingTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("PhotonHandler::initializeTools()",m_EgammaCalibrationAndSmearingTool->setProperty("ESModel", "es2017_R21_v1")); //latest recommendation: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ElectronPhotonFourMomentumCorrection#August_2018_release_of_recommend
  TOOL_CHECK("PhotonHandler::initializeTools()",m_EgammaCalibrationAndSmearingTool->setProperty("decorrelationModel", "1NP_v1")); // Jake: Do we want to make this a config?
  TOOL_CHECK("PhotonHandler::initializeTools()",m_EgammaCalibrationAndSmearingTool->setProperty("useAFII", FullSim_AFII_flag));
  TOOL_CHECK("PhotonHandler::initializeTools()",m_EgammaCalibrationAndSmearingTool->initialize());


  //isEM selector tools
  //------------------
  //create the selectors
  m_photonLooseIsEMSelector = new AsgPhotonIsEMSelector ( "PhotonLooseIsEMSelector" );
  m_photonLooseIsEMSelector->msg().setLevel( m_msgLevel );
  m_photonMediumIsEMSelector = new AsgPhotonIsEMSelector ( "PhotonMediumIsEMSelector" );
  m_photonMediumIsEMSelector->msg().setLevel( m_msgLevel );
  m_photonTightIsEMSelector = new AsgPhotonIsEMSelector ( "PhotonTightIsEMSelector" );
  m_photonTightIsEMSelector->msg().setLevel( m_msgLevel );



  //set the type of selection
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseIsEMSelector->setProperty("isEMMask", egammaPID::PhotonLoose));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumIsEMSelector->setProperty("isEMMask", egammaPID::PhotonMedium));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightIsEMSelector->setProperty("isEMMask", egammaPID::PhotonTight));

  //set the configuration file
  // todo : monitor the config files!
  TOOL_CHECK("PhotonHandler::initializeTools()",
             m_photonLooseIsEMSelector->setProperty("ConfigFile","ElectronPhotonSelectorTools/offline/mc15_20150712/PhotonIsEMLooseSelectorCutDefs.conf"));
  TOOL_CHECK("PhotonHandler::initializeTools()",
             m_photonMediumIsEMSelector->setProperty("ConfigFile","ElectronPhotonSelectorTools/offline/mc15_20150712/PhotonIsEMMediumSelectorCutDefs.conf"));
  TOOL_CHECK("PhotonHandler::initializeTools()",
	     m_photonTightIsEMSelector->setProperty("ConfigFile","ElectronPhotonSelectorTools/offline/20180825/PhotonIsEMTightSelectorCutDefs.conf"));

  //initialize the tools
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseIsEMSelector->initialize());
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumIsEMSelector->initialize());
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightIsEMSelector->initialize());

  // fudge MC tool
  //--------------
  m_photonFudgeMCTool = new ElectronPhotonShowerShapeFudgeTool( "PhotonFudgeMCTool");
  int FFset = 22; // for Moriond 18 rel21 MC15 samples, which are based on a geometry derived from GEO-21 from 2015+2016 data
  TOOL_CHECK("PhotonHandler::initializeTools()",
          m_photonFudgeMCTool->setProperty("Preselection", FFset));
  m_photonFudgeMCTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonFudgeMCTool->initialize());


  // photon ID efficiency correction tool
  //----------------------------------
  //create the tools
  m_photonLooseEffTool = new AsgPhotonEfficiencyCorrectionTool("PhotonLooseEfficiencyCorrectionTool");
  m_photonLooseEffTool->msg().setLevel( m_msgLevel );
  m_photonMediumEffTool = new AsgPhotonEfficiencyCorrectionTool("PhotonMediumEfficiencyCorrectionTool");
  m_photonMediumEffTool->msg().setLevel( m_msgLevel );
  m_photonTightEffTool = new AsgPhotonEfficiencyCorrectionTool( "PhotonTightEfficiencyCorrectionTool");
  m_photonTightEffTool->msg().setLevel( m_msgLevel );
  m_photonIsoTightEffTool = new AsgPhotonEfficiencyCorrectionTool( "PhotonIsoTightEfficiencyCorrectionTool");
  m_photonIsoTightEffTool->msg().setLevel( m_msgLevel );

  // recommended files here: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PhotonEfficiencyRun2#Recommended_input_files
/* THIS IS AN OLD MENU
  //std::string file_unc = PathResolverFindCalibFile("PhotonEfficiencyCorrection/efficiencySF.offline.Tight.2015.13TeV.rel20.unc.v02.root");
  //std::string file_con = PathResolverFindCalibFile("PhotonEfficiencyCorrection/efficiencySF.offline.Tight.2015.13TeV.rel20.con.v02.root");
  std::string file_unc = PathResolverFindCalibFile("PhotonEfficiencyCorrection/2015_2016/rel20.7/Moriond2017_v1/PIDSF/efficiencySF.offline.Tight.2016.13TeV.rel20.7.25ns.unc.v00.root");
  std::string file_con = PathResolverFindCalibFile("PhotonEfficiencyCorrection/2015_2016/rel20.7/Moriond2017_v1/PIDSF/efficiencySF.offline.Tight.2016.13TeV.rel20.7.25ns.con.v00.root");
  if(!isFullSim){
    file_unc = PathResolverFindCalibFile("PhotonEfficiencyCorrection/efficiencySF.offline.Tight.2015.13TeV.rel20.AFII.unc.v01.root");
    file_con = PathResolverFindCalibFile("PhotonEfficiencyCorrection/efficiencySF.offline.Tight.2015.13TeV.rel20.AFII.con.v01.root");
  }

  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightEffTool->setProperty("CorrectionFileNameConv",file_con));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightEffTool->setProperty("CorrectionFileNameUnconv",file_unc));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumEffTool->setProperty("CorrectionFileNameConv",file_con));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumEffTool->setProperty("CorrectionFileNameUnconv",file_unc));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseEffTool->setProperty("CorrectionFileNameConv",file_con));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseEffTool->setProperty("CorrectionFileNameUnconv",file_unc));
*/
  //TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightEffTool->setProperty ("MapFilePath", "PhotonEfficiencyCorrection/2015_2017/rel21.2/Winter2018_Prerec_v1/map0.txt"));//PhotonEfficiencyCorrection/map0.txt"));
  //TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumEffTool->setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2017/rel21.2/Winter2018_Prerec_v1/map0.txt"));//PhotonEfficiencyCorrection/map0.txt"));
  //TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseEffTool->setProperty ("MapFilePath", "PhotonEfficiencyCorrection/2015_2017/rel21.2/Winter2018_Prerec_v1/map0.txt"));//PhotonEfficiencyCorrection/map0.txt"));

  // new recommendation for photon isolation efficiency scale factor https://twiki.cern.ch/twiki/bin/view/AtlasProtected/IsolationOctober2018RecomRel21#Photon_isolation , only for tight
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseEffTool->setProperty ("MapFilePath", "PhotonEfficiencyCorrection/2015_2017/rel21.2/Summer2018_Rec_v1/map4.txt"));//PhotonEfficiencyCorrection/map0.txt"));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumEffTool->setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2017/rel21.2/Summer2018_Rec_v1/map4.txt"));//PhotonEfficiencyCorrection/map0.txt"));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightEffTool->setProperty ("MapFilePath", "PhotonEfficiencyCorrection/2015_2017/rel21.2/Summer2018_Rec_v1/map4.txt"));//PhotonEfficiencyCorrection/map0.txt"));

  // set data type
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseEffTool->setProperty("ForceDataType", dataType));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumEffTool->setProperty("ForceDataType", dataType));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightEffTool->setProperty("ForceDataType", dataType));

  //initialize
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonLooseEffTool->initialize());
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonMediumEffTool->initialize());
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonTightEffTool->initialize());

  // isolation correction tool
  //--------------------------
  m_isoCorr_tool = new CP::IsolationCorrectionTool("isoCorr_tool_photon"); // latest recommendation https://twiki.cern.ch/twiki/bin/view/AtlasProtected/IsolationLeakageCorrections
  TOOL_CHECK("PhotonHandler::initializeTools()", m_isoCorr_tool->setProperty("CorrFile_ddshift", "IsolationCorrections/v3/isolation_ddcorrection_shift.root")); // DD corrections
  TOOL_CHECK("PhotonHandler::initializeTools()", m_isoCorr_tool->setProperty("IsMC", m_eventInfoHandler.get_isMC() ? true : false));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_isoCorr_tool->setProperty("AFII_corr", FullSim_AFII_flag) );
  TOOL_CHECK("PhotonHandler::initializeTools()", m_isoCorr_tool->initialize());

  // isolation tools
  //----------------
  m_isIsoPh = new CP::IsolationSelectionTool("PhotonIso");
  TOOL_CHECK("PhotonHandler::initializeTools()", m_isIsoPh->setProperty("PhotonWP", "FixedCutTightCaloOnly"));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_isIsoPh->initialize());


  // isolation scale factor tool 

  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonIsoTightEffTool->setProperty ("MapFilePath", "PhotonEfficiencyCorrection/2015_2017/rel21.2/Summer2018_Rec_v1/map4.txt"));//PhotonEfficiencyCorrection/map0.txt"));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonIsoTightEffTool->setProperty("ForceDataType", dataType));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonIsoTightEffTool->setProperty("IsoKey", "TightCaloOnly" ));
  TOOL_CHECK("PhotonHandler::initializeTools()", m_photonIsoTightEffTool->initialize());

  // Ambiguity Tool
  // --------------
  m_egammaAmbiguityTool = new EGammaAmbiguityTool("PhotonEGammaAmbiguityTool");
  TOOL_CHECK("PhotonHandler::initializeTools()", m_egammaAmbiguityTool->initialize());

  //register ISystematicsTools
  //--------------------------
  m_sysToolList.clear();
  m_sysToolList.push_back(m_EgammaCalibrationAndSmearingTool);
  m_sysToolList.push_back(m_photonTightEffTool);
  m_sysToolList.push_back(m_photonMediumEffTool);
  m_sysToolList.push_back(m_photonLooseEffTool);
  m_sysToolList.push_back(m_isoCorr_tool);
  m_sysToolList.push_back(m_photonIsoTightEffTool);
  //m_sysToolList.push_back(m_egammaAmbiguityTool);  No Instructions found for adding to systematics

  //egamma trigger matching tool
//  m_trigConfigTool = new TrigConf::xAODConfigTool ("xAODConfigTool");
//  ToolHandle<TrigConf::ITrigConfigTool> handle(m_trigConfigTool);
//  TOOL_CHECK("PhotonHandler::initializeTools()",handle->initialize());
//
//  m_trigDecTool = new Trig::TrigDecisionTool("TrigDecisionTool");
//  TOOL_CHECK("PhotonHandler::initializeTools()",m_trigDecTool->setProperty("ConfigTool",handle));
//  TOOL_CHECK("PhotonHandler::initializeTools()",m_trigDecTool->setProperty("TrigDecisionKey","xTrigDecision"));
//  TOOL_CHECK("PhotonHandler::initializeTools()",m_trigDecTool->initialize());
  m_trigDecTool = m_eventInfoHandler.get_TrigDecTool();

  if (m_trigDecTool){
	 ToolHandle<Trig::TrigDecisionTool> m_trigDec(m_trigDecTool);
	 TOOL_CHECK("PhotonHandler::initializeTools()",m_trigDec.retrieve());
	 // m_match_Tool = new Trig::MatchingTool("TrigMatchTool");

         m_match_Tool.setTypeAndName("Trig::MatchingTool/PhotonMatchingTool");
         TOOL_CHECK("PhotonHandler::initializeTools()",m_match_Tool.retrieve());
  }



  std::unordered_map<std::string, PROP<int>*> triggersToMatch {
      {"HLT_g20_loose", &Props::matchHLT_g20_loose},
      {"HLT_g15_loose_2j40_0eta490_3j25_0eta490", &Props::matchHLT_g15_loose_2j40_0eta490_3j25_0eta490},
      {"HLT_g20_loose_2j40_0eta490_3j25_0eta490", &Props::matchHLT_g20_loose_2j40_0eta490_3j25_0eta490},
      {"HLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ-700", &Props::matchHLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ700},
      {"HLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ-900", &Props::matchHLT_g20_loose_2j40_0eta490_3j25_0eta490_L1MJJ900},
      {"HLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700", &Props::matchHLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700},
      {"HLT_g25_loose_L1EM20VH_2j40_0eta490_3j25_0eta490_invm700", &Props::matchHLT_g25_loose_L1EM20VH_2j40_0eta490_3j25_0eta490_invm700},
      {"HLT_g25_loose_2j40_0eta490_3j25_0eta490_invm700", &Props::matchHLT_g25_loose_2j40_0eta490_3j25_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700", &Props::matchHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000", &Props::matchHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000},
      {"HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700", &Props::matchPhHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490", &Props::matchPhHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490},
      {"HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700", &Props::matchPhHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490", &Props::matchPhHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490},
      {"HLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700", &Props::matchPhHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700},
      {"HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490", &Props::matchPhHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490},
      {"HLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500", &Props::matchPhHLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500},
      {"HLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500", &Props::matchPhHLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500},
      {"HLT_g25_loose_L1EM20VH_4j35_0eta490", &Props::matchHLT_g25_loose_L1EM20VH_4j35_0eta490},
      {"HLT_g140_loose", &Props::matchHLT_g140_loose}
  };
  m_triggersToMatch = triggersToMatch;

  return EL::StatusCode::SUCCESS;
}

bool PhotonHandler::passLoosePhoton(xAOD::Photon * photon) {
  bool passSel = true;

  passSel &= (Props::isLoose.get(photon));
  passSel &= (photon->eta() < 2.37);
  passSel &= (photon->pt() > 15000.);

  Props::passPreSel.set(photon, passSel);
  return passSel;
}

EL::StatusCode PhotonHandler::decorateOriginParticle(const xAOD::Photon *)
{

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode PhotonHandler::decorate(xAOD::Photon * photon)
{
  //selection tools
  //---------------
  //retrieve decision from tools and decorate photon with it
  //perform actual selection later

  int isFullSim = 1;
  if(isFullSim){

    // check the availability of the shower shape variables
    bool allFound = true;

    allFound = allFound && photon->isAvailable<float>("e277");
    allFound = allFound && photon->isAvailable<float>("Reta");
    allFound = allFound && photon->isAvailable<float>("Rphi");
    allFound = allFound && photon->isAvailable<float>("weta2");
    allFound = allFound && photon->isAvailable<float>("f1");
    allFound = allFound && photon->isAvailable<float>("wtot");
    allFound = allFound && photon->isAvailable<float>("w1");
    allFound = allFound && photon->isAvailable<float>("Rhad1");
    allFound = allFound && photon->isAvailable<float>("Rhad");
    allFound = allFound && photon->isAvailable<float>("fracs1");
    allFound = allFound && photon->isAvailable<float>("DeltaE");
    allFound = allFound && photon->isAvailable<float>("Eratio");


    if(allFound)
    {
      if(m_photonFudgeMCTool->applyCorrection(*photon) == CP::CorrectionCode::Error){
        Error("PhotonHandler::decorate()", "photonFudgeMCTool->applyCorrection(*photon) returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
    }
  }

  int convType = photon->conversionType();
  Props::convType.set(photon, convType);

  // truth information
  if(m_eventInfoHandler.get_isMC()){

	 // MC classifier
	 int truthType = 0;
	 int truthOrigin = 0;
	 truthType = xAOD::TruthHelpers::getParticleTruthType(*photon);
	 truthOrigin = xAOD::TruthHelpers::getParticleTruthOrigin(*photon);
	 Props::truthType.set(photon, truthType);
	 Props::truthOrigin.set(photon, truthOrigin);

	 // pdgID
//	 int mc_pdgId = 0;
//	 int mc_parentId = 0;
//	 auto truthPart = xAOD::TruthHelpers::getTruthParticle(*photon);
//	 if(truthPart){
//		mc_pdgId = truthPart->pdgId();
//		if(truthPart->nParents()) mc_parentId = truthPart->parent(0)->pdgId();
//	 }
//	 Props::mc_pdgId.set(photon, mc_pdgId);
//	 Props::mc_parentId.set(photon, mc_parentId);
  }

  uint32_t OQ = photon->OQ();
  int Author = photon->author();
  Props::OQ.set(photon, OQ);
  Props::Author.set(photon, Author);

  int isLoose = 0;
  int isMedium = 0;
  int isTight = 0;
  unsigned int isEM_Loose = 0;
  unsigned int isEM_Medium = 0;
  unsigned int isEM_Tight = 0;


  int ptag=atof((m_eventInfoHandler.get_pTag()).c_str());

  isLoose = photon->isAvailable<char>("DFCommonPhotonsIsEMLoose") ? static_cast<int>(photon->auxdata<char>("DFCommonPhotonsIsEMLoose")) : static_cast<int>(m_photonLooseIsEMSelector->accept(photon));
  isMedium = photon->isAvailable<char>("DFCommonPhotonsIsEMMedium") ? static_cast<int>(photon->auxdata<char>("DFCommonPhotonsIsEMMedium")) : static_cast<int>(m_photonMediumIsEMSelector->accept(photon));
  isTight = photon->isAvailable<char>("DFCommonPhotonsIsEMTight") ? static_cast<int>(photon->auxdata<char>("DFCommonPhotonsIsEMTight")) : static_cast<int>(m_photonTightIsEMSelector->accept(photon));

  if(ptag>=3627) // recommendation: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaRecommendationsR21
  {
    isTight = photon->isAvailable<char>("DFCommonPhotonsIsEMTightPtIncl") ? static_cast<int>(photon->auxdata<char>("DFCommonPhotonsIsEMTightPtIncl")) : static_cast<int>(m_photonTightIsEMSelector->accept(photon));
  }

  isEM_Loose = photon->isAvailable<unsigned int>("DFCommonPhotonsIsEMLooseIsEMValue") ? photon->auxdata<unsigned int>("DFCommonPhotonsIsEMLooseIsEMValue") : m_photonLooseIsEMSelector->IsemValue();
  isEM_Medium = photon->isAvailable<unsigned int>("DFCommonPhotonsIsEMMediumIsEMValue") ? photon->auxdata<unsigned int>("DFCommonPhotonsIsEMMediumIsEMValue") : m_photonMediumIsEMSelector->IsemValue();
  isEM_Tight = photon->isAvailable<unsigned int>("DFCommonPhotonsIsEMTightIsEMValue") ? photon->auxdata<unsigned int>("DFCommonPhotonsIsEMTightIsEMValue") : m_photonTightIsEMSelector->IsemValue();


  if(ptag>=3627) // recommendation: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaRecommendationsR21
  {
    isEM_Tight = photon->isAvailable<unsigned int>("DFCommonPhotonsIsEMTightPtInclIsEMValue") ? photon->auxdata<unsigned int>("DFCommonPhotonsIsEMTightPtInclIsEMValue") : m_photonTightIsEMSelector->IsemValue();
  }





  Props::isTight.set(photon, isTight);
  Props::isMedium.set(photon, isMedium);
  Props::isLoose.set(photon, isLoose);
  Props::isEM_Tight.set(photon, isEM_Tight);
  Props::isEM_Medium.set(photon, isEM_Medium);
  Props::isEM_Loose.set(photon, isEM_Loose);

  Root::TAccept isoAccept = m_isIsoPh->accept( *photon);
  Props::isFixedCutTightCaloOnlyIso.set(photon, isoAccept.getCutResult("FixedCutTightCaloOnly"));

  // photon ID SF
  double photonTightEffSF(1.), photonMediumEffSF(1.), photonLooseEffSF(1.);

  const xAOD::CaloCluster* cluster = photon->caloCluster();
  float cluster_eta = 10;
  float cluster_et = 0;
  if (cluster) {
    cluster_eta = cluster->etaBE(2);
    if (cluster_eta != 0.0) {
      cluster_et = cluster->e() / cosh(cluster_eta);
    }
  }

  bool inCrack = fabs(cluster_eta)>1.37 && fabs(cluster_eta)<1.52;

  // configuration files not yet available for 13 TeV :(
  //sf only available after basic kinematic selection
  if(cluster_et > 15000. && fabs(cluster_eta) < 2.37 && !inCrack){
    if(isLoose) {
      // SF
      if(m_photonLooseEffTool->getEfficiencyScaleFactor(*photon, photonLooseEffSF) == CP::CorrectionCode::Error){
        Error("PhotonHandler::decorate()", "getEfficiencyScaleFactor returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
    }

    if(isMedium) {
      // SF
      if(m_photonMediumEffTool->getEfficiencyScaleFactor(*photon, photonMediumEffSF) == CP::CorrectionCode::Error){
        Error("PhotonHandler::decorate()", "getEfficiencyScaleFactor returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
    }

    if(isTight) {
      // SF
      if(m_photonTightEffTool->getEfficiencyScaleFactor(*photon, photonTightEffSF) == CP::CorrectionCode::Error){
        Error("PhotonHandler::decorate()", "getEfficiencyScaleFactor returned CP::CorrectionCode::Error");
        return EL::StatusCode::FAILURE;
      }
    }

  }

  Props::tightEffSF.set(photon, photonTightEffSF);
  Props::mediumEffSF.set(photon,photonMediumEffSF);
  Props::looseEffSF.set(photon,photonLooseEffSF);

  int isAmbiguous = 0; // Photon is ambiguous if = 1, should be skipped
  isAmbiguous = static_cast<int>(!m_egammaAmbiguityTool->accept(*photon));
  Props::isAmbiguous.set(photon,isAmbiguous);

  //------------------trigger matching------------------------

  if(m_event->contains<xAOD::PhotonContainer>("HLT_xAOD__PhotonContainer_egamma_Photons")){ // check if trigger photon info exist

    // get trigger list from config file
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    std::string trig_name;


    // only call the trigger matching for the triggers that really need it !
    // loop over the triggers we declared as match-able for photons
    for (auto& trig : m_triggersToMatch) {
      // check if it is in the trigDecorators. Maybe we don't want this trigger.
      if(trigDecorators.count(trig.first)) {
        trig_name = trig.first;

		if(m_eventInfoHandler.get_isMC() && (trig_name=="HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm1000")){ // VBFg trigger emulation
		   trig_name = "HLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700";
		}

                bool is_matched = false;

                if(isLoose)
                {
                   std::vector<const xAOD::IParticle*> myParticles;
                   myParticles.clear();
                   myParticles.push_back(photon);
                   // set dR=0.07 as it is recommended for the egamma triggers. See https://twiki.cern.ch/twiki/bin/view/Atlas/XAODMatchingTool

                   is_matched = m_match_Tool->match(myParticles,trig_name,0.07);
                   trig.second->set(photon, is_matched);
                }

        trig.second->set(photon, is_matched);

      }
    } // loop over trigger list

  } // check if EgammaTriggerContent exist

  // Isolation SF

  double photonIsoTightEffSF=1.0;
  if(isoAccept.getCutResult("FixedCutTightCaloOnly"))
  {
    if(m_photonIsoTightEffTool->getEfficiencyScaleFactor(*photon, photonIsoTightEffSF) == CP::CorrectionCode::Error)
    {
      Error("PhotonHandler::decorate()", "getEfficiencyScaleFactor returned CP::CorrectionCode::Error");
    }
  }

  Props::photonFixedCutTightEffSF.set(photon, photonIsoTightEffSF);




  Props::forMETRebuild.set(photon, false);//setting as false as default for MET rebuilding
  return EL::StatusCode::SUCCESS;

}


EL::StatusCode PhotonHandler::calibrateCopies(xAOD::PhotonContainer * particles, const CP::SystematicSet & sysSet)
{

  //calibration tool
  //----------------
  //tell tool to apply systematic variation
  CP_CHECK("PhotonHandler::calibrateCopies()",m_EgammaCalibrationAndSmearingTool->applySystematicVariation(sysSet),m_debug);

  //tell tool to apply systematic variation
  CP_CHECK("PhotonHandler::calibrateCopies()", m_photonTightEffTool->applySystematicVariation(sysSet), m_debug);
  CP_CHECK("PhotonHandler::calibrateCopies()", m_photonMediumEffTool->applySystematicVariation(sysSet), m_debug);
  CP_CHECK("PhotonHandler::calibrateCopies()", m_photonLooseEffTool->applySystematicVariation(sysSet), m_debug);

  CP_CHECK("PhotonHandler::calibrateCopies()", m_isoCorr_tool->applySystematicVariation(sysSet), m_debug);

  CP_CHECK("PhotonHandler::calibrateCopies()", m_photonIsoTightEffTool->applySystematicVariation(sysSet), m_debug);

  for(xAOD::Photon * photon : *particles) {

    //calibration tool
    //----------------

    //this ensures that the DynAux container has the same number of entries as the original one
    //not necessary in the case of photons - keep it for consistency?
    //setP4( photon, photon);
    if( (photon->author() & xAOD::EgammaParameters::AuthorPhoton)   // veto soft photons
	|| (photon->author() & xAOD::EgammaParameters::AuthorAmbiguous) ) {
      CP_CHECK("PhotonHandler::calibrateCopies()",m_EgammaCalibrationAndSmearingTool->applyCorrection(*photon),m_debug);
    }

    const xAOD::CaloCluster* cluster = photon->caloCluster();
    float cluster_et = cluster->e() / cosh(cluster->etaBE(2));

	// calo iso correction
    if(cluster_et/1000. > 10. && fabs(cluster->etaBE(2)) < 2.37){
      CP_CHECK("PhotonHandler::calibrateCopies()", m_isoCorr_tool->applyCorrection(*photon), m_debug);
    }

    // decorate photon
    if ( decorate( photon ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;

  }

  return EL::StatusCode::SUCCESS;

}



EL::StatusCode PhotonHandler::writeOutputVariables(xAOD::Photon * inPhoton, xAOD::Photon * outPhoton,  bool isKinVar, bool isWeightVar, const TString& sysName)
{

  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    EL_CHECK("PhotonHandler::writeOutputVariables",writeAllVariations(inPhoton, outPhoton, sysName));
  }
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    EL_CHECK("PhotonHandler::writeOutputVariables",writeKinematicVariations(inPhoton, outPhoton, sysName));
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes
    EL_CHECK("PhotonHandler::writeOutputVariables",writeWeightVariations(inPhoton, outPhoton, sysName));
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited
    EL_CHECK("PhotonHandler::writeOutputVariables",writeAllVariations(inPhoton, outPhoton, sysName));
    EL_CHECK("PhotonHandler::writeOutputVariables",writeKinematicVariations(inPhoton, outPhoton, sysName));
    EL_CHECK("PhotonHandler::writeOutputVariables",writeWeightVariations(inPhoton, outPhoton, sysName));
    EL_CHECK("PhotonHandler::writeOutputVariables",writeNominal(inPhoton, outPhoton, sysName));
  } else assert(false);

  return writeCustomVariables(inPhoton, outPhoton, isKinVar, isWeightVar, sysName);
}

EL::StatusCode PhotonHandler::writeAllVariations(xAOD::Photon* /*inPhoton*/, xAOD::Photon* /*outPhoton*/, const TString& /*sysName*/)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode PhotonHandler::writeKinematicVariations(xAOD::Photon* inPhoton, xAOD::Photon* outPhoton, const TString& /*sysName*/)
{
  //set four momentum
  setP4( inPhoton, outPhoton );

  Props::isTight.copy(inPhoton, outPhoton);
  Props::isMedium.copy(inPhoton, outPhoton);
  Props::isLoose.copy(inPhoton, outPhoton);

  // set isEM bits
  Props::isEM_Tight.copy(inPhoton, outPhoton);
  Props::isEM_Medium.copy(inPhoton, outPhoton);
  Props::isEM_Loose.copy(inPhoton, outPhoton);

  Props::isAmbiguous.copy(inPhoton, outPhoton);

  //set isolation
 if(inPhoton->isAvailable<float>("topoetcone40"))  outPhoton->setIsolationValue(inPhoton->isolationValue(xAOD::Iso::topoetcone40),xAOD::Iso::topoetcone40);
 if(inPhoton->isAvailable<float>("topoetcone20"))  outPhoton->setIsolationValue(inPhoton->isolationValue(xAOD::Iso::topoetcone20),xAOD::Iso::topoetcone20);

  Props::isFixedCutTightCaloOnlyIso.copy(inPhoton, outPhoton);

  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode PhotonHandler::writeWeightVariations(xAOD::Photon* inPhoton, xAOD::Photon* outPhoton, const TString& /*sysName*/)
{
  if (m_eventInfoHandler.get_isMC()) {
    Props::tightEffSF.copy(inPhoton, outPhoton);
    Props::mediumEffSF.copy(inPhoton, outPhoton);
    Props::looseEffSF.copy(inPhoton, outPhoton);


    Props::photonFixedCutTightEffSF.copy(inPhoton, outPhoton);

  }
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode PhotonHandler::writeNominal(xAOD::Photon* inPhoton, xAOD::Photon* outPhoton, const TString& /*sysName*/)
{
  Props::convType.copy(inPhoton, outPhoton);
  if (m_eventInfoHandler.get_isMC()) {
	 Props::truthType.copy(inPhoton, outPhoton);
	 Props::truthOrigin.copy(inPhoton, outPhoton);
//	 Props::mc_pdgId.copy(inPhoton, outPhoton);
//	 Props::mc_parentId.copy(inPhoton, outPhoton);
  }

  Props::OQ.copy(inPhoton, outPhoton);
  Props::Author.copy(inPhoton, outPhoton);

  // set something without having a pre-defined method
  //trigger matching
  if(m_event->contains<xAOD::PhotonContainer>("HLT_xAOD__PhotonContainer_egamma_Photons")){ // check if trigger photon info exist
    const auto& trigDecorators = m_eventInfoHandler.get_trigDecorators();
    for(auto& trig : m_triggersToMatch) {
      if(trigDecorators.count(trig.first)) {
        trig.second->copy(inPhoton, outPhoton);
      }
    }
  }
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode PhotonHandler::writeCustomVariables(xAOD::Photon*, xAOD::Photon*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode PhotonHandler::clearEvent()
{

  EL_CHECK("PhotonHandler::clearEvent()",ObjectHandler::clearEvent());
  return EL::StatusCode::SUCCESS;

}
