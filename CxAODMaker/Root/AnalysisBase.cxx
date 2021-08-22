#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

#include "EventLoop/Job.h"
#include "EventLoop/StatusCode.h"
#include "EventLoop/IWorker.h"
#include "EventLoop/OutputStream.h"
#include "xAODCutFlow/CutBookkeeper.h"
#include "xAODCutFlow/CutBookkeeperContainer.h"

#include "CxAODMaker/AnalysisBase.h"
#include "CxAODMaker/MuonHandler.h"
#include "CxAODMaker/JetHandler.h"
#include "CxAODMaker/ElectronHandler.h"
#include "CxAODMaker/ForwardElectronHandler.h"
#include "CxAODMaker/PhotonHandler.h"
#include "CxAODMaker/TauHandler.h"
#include "CxAODMaker/DiTauJetHandler.h"
#include "CxAODMaker/FatJetHandler.h"
#include "CxAODMaker/TrackJetHandler.h"
#include "CxAODMaker/TruthJetHandler.h"
#include "CxAODMaker/METHandler.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "CxAODMaker/EventSelector.h"
#include "CxAODMaker/JetRegression.h"
#include "CxAODMaker/JetSemileptonic.h"
#include "CxAODMaker/TruthEventHandler.h"

#include "CxAODMaker/TruthProcessor.h"

// AnalysisContext
#include "CxAODTools/OvCompat.h"
#include "CxAODTools/OverlapRegisterAccessor.h"
#include "CxAODTools/LumiMetaDataTool.h"

#include "xAODRootAccess/tools/TFileAccessTracer.h"

#include "TFile.h"
#include "TTree.h"
#include "TPRegexp.h"

#include <fstream>

// prw file check
#include "xAODMetaData/FileMetaData.h"
#include <TSystem.h>
#include <TKey.h>

// this is needed to distribute the algorithm to the workers
ClassImp(AnalysisBase)

AnalysisBase::AnalysisBase() :
  m_jetHandler(nullptr),
  m_fatjetHandler(nullptr),
  m_fatjetAltHandler(nullptr),
  m_trackjetHandler(nullptr),
  m_vrtrackjetHandler(nullptr),
  m_comtrackjetHandler(nullptr),  //CMVI
  m_truthjetHandler(nullptr),
  m_truthfatjetHandler(nullptr),
  m_truthWZjetHandler(nullptr),
  m_truthWZfatjetHandler(nullptr),
  m_muonHandler(nullptr),
  m_electronHandler(nullptr),
  m_forwardElectronHandler(nullptr),
  m_photonHandler(nullptr),
  m_tauHandler(nullptr),
  m_ditauHandler(nullptr),
  m_eventInfoHandler(nullptr),
  m_truthEventHandler(nullptr),
  m_linker(),
  m_METHandler(nullptr),
  m_METHandlerMJTight(nullptr),
  m_METHandlerFJVT(nullptr),
  m_METHandlerMJMUTight(nullptr),
  m_METHandlerMJMiddle(nullptr),
  m_METHandlerMJLoose(nullptr),
  m_METTrackHandler(nullptr),
  m_selector(nullptr),
  m_regression(nullptr),
  m_semileptonic(nullptr),
  m_lumiMetaData(nullptr),
  m_analysisContext(nullptr),
  m_config(nullptr),
  m_debug(false),
  m_isMC(false),
  m_isDerivation(false),
  m_derivationName(""),
  m_isAFII(false),
  m_pTag(""),
  m_hasNoFJDetectorEta(false),
  m_mcChanNr(-1),
  m_applyJetRegression(false),
  m_applyJetSemileptonic(false),
  m_countElectronInJet(false),
  m_correctMuonInJet(false),
  m_applyMETAfterSelection(false),
  m_event(nullptr),
  m_maxEvent(-999),
  m_eventCounter(0),
  m_eventWeightCounter(0),
  m_histEventCount(0),
  m_derivation20_7(0),
  m_overlapRegAcc(nullptr),
  m_truthProcessor(nullptr)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}

AnalysisBase::~AnalysisBase() {
  delete m_analysisContext;
  if(m_lumiMetaData != nullptr) {
    delete m_lumiMetaData;
  }

}

EL::StatusCode AnalysisBase::setupJob(EL::Job& job) {
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  Info("setupJob()", "Setting up job.");

  // check if ConfigStore is set
  if ( ! m_config ) {
    Error("setupJob()","ConfigStore not setup! Remember to set it via setConfig()!");
    return EL::StatusCode::FAILURE;
  }

  Info("setupJob()","maxEvents = %d",m_config->get<int>("maxEvents"));

  job.useXAOD();

  // let's initialize the algorithm to use the xAODRootAccess package
  xAOD::Init("AnalysisBase").ignore(); // call before opening first file

  // tell EventLoop about our output xAOD:
  EL::OutputStream out("CxAOD","xAODNoMeta");
  out.options()->setString(EL::OutputStream::optMergeCmd, "xAODMerge -m LumiMetaDataTool");
  job.outputAdd (out);

  //std::cout << "DEBUG delete old analysis context." << std::endl;
  //delete m_analysisContext;
  //std::cout << "DEBUG delete new analysis context." << std::endl;


  m_config->getif<bool>("applyJetRegression",m_applyJetRegression);
  bool tmp_doTraining;
  m_config->getif<bool>("JetRegression.doTraining",tmp_doTraining);

  // hardcode output stream for JetRegression
  if(m_applyJetRegression && tmp_doTraining){
    EL::OutputStream outreg("RegTrees");
    job.outputAdd (outreg);
  }

  m_config->getif<bool>("applyMETAfterSelection",m_applyMETAfterSelection);

  m_skipNoPV = true; // default to true
  m_config->getif<bool>("skipNoPV",m_skipNoPV);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::histInitialize() {
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  if (m_histEventCount) {
    // TODO why is histInitialize() called after fileExecute() ??
    Warning("histInitialize()", "Histograms already initialized! Skipping histInitialize()...");
    return EL::StatusCode::SUCCESS;
  }

  // event counts from meta data
  m_histEventCount = new TH1D("MetaData_EventCount", "MetaData_EventCount", 8, 0.5, 8.5);
  m_histEventCount -> GetXaxis() -> SetBinLabel(1, "nEvents initial");
  m_histEventCount -> GetXaxis() -> SetBinLabel(2, "nEvents selected in");
  m_histEventCount -> GetXaxis() -> SetBinLabel(3, "nEvents selected out");
  m_histEventCount -> GetXaxis() -> SetBinLabel(4, "sumOfWeights initial");
  m_histEventCount -> GetXaxis() -> SetBinLabel(5, "sumOfWeights selected in");
  m_histEventCount -> GetXaxis() -> SetBinLabel(6, "sumOfWeights selected out");
  m_histEventCount -> GetXaxis() -> SetBinLabel(7, "nEvents in this job");
  m_histEventCount -> GetXaxis() -> SetBinLabel(8, "sumOfWeights in this job");
  wk() -> addOutput(m_histEventCount);

  Info("histInitialize()", "Histograms initialized.");

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeMetadata() {
  TString fileName = wk()->inputFile()->GetName();
  Info("fileExecute()", "Processing file '" + fileName + "'");

  // get the MetaData tree once a new file is opened, with
  TTree *MetaData = dynamic_cast<TTree*>(wk()->inputFile()->Get("MetaData"));
  if (!MetaData) {
    Error("fileExecute()", "MetaData not found! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  MetaData->LoadTree(0);

  //check if file is data or MC
  if (!m_derivation20_7) {
    m_isMC =  !( MetaData->GetBranch("LumiBlocks") || MetaData->GetBranch("IncompleteLumiBlocks") ) ;
  }
  else {
    //m_isMC =  !( MetaData->GetBranch("LumiBlocks") || MetaData->GetBranch("IncompleteLumiBlocks") ) ;
    m_isMC =  !( MetaData->GetBranch("ByteStreamMetadata") ) ;
  }

  //check if file is from a DxAOD
  m_isDerivation = !MetaData->GetBranch("StreamAOD");

  //retrieve DxAOD name
  if(m_isDerivation){
    for (auto branch : *MetaData->GetListOfBranches()){
      std::string brName =  branch->GetName();
      if(brName.find("StreamDAOD") != std::string::npos){
        m_derivationName = brName.substr(brName.find("_")+1);
        break;
      }
    }
    if(m_derivationName == ""){
      Error("fileExecute()","Failed to retrieve derivation name from MetaData! Exiting.");
      return EL::StatusCode::FAILURE;
    }
    if(!fileName.Contains(m_derivationName)){
      Error("fileExecute()","Mismatch between derivation in Metadata and sample name! Exiting.");
      return EL::StatusCode::FAILURE;
    }
    Info("fileExecute()","Running on derivation (%s)!",m_derivationName.c_str());
  }
  else Info("fileExecute()","Running on xAOD!");

  //check if file is AFII or FullSim - should be replaced by MetaData check!!
  //note: local and grid only work with either file or sample name!
  TString sampleName = wk()->metaData()->castString("sample_name");
  if (m_isMC) {
    TPRegexp re("_a\\d\\d\\d");
    if (fileName.Contains(re)|| sampleName.Contains(re)) {
      Info("fileExecute()","Running on fast-simulation sample!");
      m_isAFII = true;
    }else{
      Info("fileExecute()","Running on full-simulation sample!");
    }
  }

  //save the pTag of the sample, at least temporarily needed for the FatJetHandler
  TPRegexp ptagpattern("_p\\d{4}/");
  if(ptagpattern.MatchB(sampleName)){
    m_pTag = sampleName(ptagpattern);
  }
  if(ptagpattern.MatchB(fileName)){
    // use / to catch when there are two p-tags and a file after it
    m_pTag = fileName(ptagpattern);
    m_pTag.erase(0,2);
    m_pTag.erase(m_pTag.length()-1);
  } else {
    //a sample, pick the last 4 characters, but ignore / if present
    m_pTag=sampleName.Data();
    if (m_pTag[m_pTag.length()-1]=='/') {
      m_pTag=m_pTag.substr(m_pTag.length()-5,m_pTag.length()-1);
    } else {
      m_pTag=m_pTag.substr(m_pTag.length()-4,m_pTag.length());
    }
  }
  Info("fileExecute()","Retrieved from sample (%s) the last p-tag as %s!",sampleName.Data(),m_pTag.c_str());
  if (m_pTag=="") {
    Error("fileExecute()","Failed to retrieve pTag name from sampleName or fileName! Exiting.");
    return EL::StatusCode::FAILURE;
  }

  //Detector Eta fix
  //temporary hack: check if the sample has the detector eta variable for the fatjets or not
  //In derivations up to cache 21.2.18 (pTags 3503/3504 and p3506) there is no DetectorEta information available -> replace it with JetConstituentEta later in the FatJetHandler
  //For newer derivations (see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationProductionTeam for pTag-Cache matching) the DetectorEta has been added again
  if ( m_pTag=="3372" || m_pTag=="3371" || m_pTag=="3374" || m_pTag=="3402"          
    || m_pTag=="3557" || m_pTag=="3560" || m_pTag=="3571" || m_pTag=="3572" || m_pTag=="3474"){
      m_hasNoFJDetectorEta = true;
  }

  //extract MC channel number
  if(m_isMC){
    if (!wk()->tree()) {
      Warning("AnalysisBase::initializeMetadata()", "CollectionTree does not exist, MC channel number cannot be determined.");
    } else {
      m_event = wk()->xaodEvent();
      const xAOD::EventInfo * eventInfo = 0;
      if ( ! m_event->retrieve(eventInfo, "EventInfo").isSuccess() ) {
        Error("AnalysisBase::initializeMetadata()", "Failed to retrieve event info collection for determination of MC channel number. Exiting.");
        return EL::StatusCode::FAILURE;
      }
      m_mcChanNr = eventInfo->mcChannelNumber();
    }
  }

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode AnalysisBase::fileExecute() {
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed

  if (!m_histEventCount) {
    // TODO why is histInitialize() called after fileExecute() ??
    Warning("fileExecute()", "Histograms not initialized! Calling histInitialize()...");
    histInitialize();
  }

  //----------------------------
  // check the derivation tag
  //----------------------------
  // assume that AnalysisBase 2.3.X is old 20.1 derivation based
  std::string tag_rootcore(std::getenv("ROOTCORE_RELEASE_SERIES"));
  if (tag_rootcore == "24" || tag_rootcore == "25") m_derivation20_7 = 1;

  //----------------------------
  // Meta data
  //---------------------------
  if(m_lumiMetaData == nullptr) {
    Info("fileExecute()", "Creating the LumiMetaDataTool");
    m_lumiMetaData = new LumiMetaDataTool();
  }

  EL_CHECK("fileExecute()", initializeMetadata() );

  m_event = wk()->xaodEvent();
  if(m_isDerivation){
    // First, check that we have no incomplete bookkeepers.
    // This would indicate that an input file was not completely processed.
    // -> should be disregarded, since it is not trustworthy!

    //don't do this check on data derivations for now!

    const xAOD::CutBookkeeperContainer* incompleteCBC = nullptr;
    TOOL_CHECK("fileExecute()",m_event->retrieveMetaInput(incompleteCBC, "IncompleteCutBookkeepers"));
    if ( incompleteCBC->size() != 0 ) {
      if (!m_isMC) {
        for(auto cbk : *incompleteCBC) {
          if (cbk->inputStream() != "unknownStream"){
	    Warning("fileExecute()","Found incomplete Bookkeepers in data (inputStream = '%s')! This appears to be okay (apparently just a renaming of the stream)", (cbk->inputStream()).c_str());
            // Error("fileExecute()","Found incomplete Bookkeepers in data! Check file for corruption. Exiting!");
            // return EL::StatusCode::FAILURE;
          }
          else{
            Error("fileExecute()","Found incomplete Bookkeepers in data! Check file for corruption. NOT exiting coming from RAW->ESD step...");
          }
        }
      }else{
        if (!m_derivation20_7) {
          Error("fileExecute()","Found incomplete Bookkeepers in MC! Check file for corruption. Exiting!");
          return EL::StatusCode::FAILURE;
        }
        else {
          Warning("fileExecute()","Found incomplete Bookkeepers in MC! Just a print here as it can happen.");
        }
      }
    }
    // Now, let's find the actual information
    const xAOD::CutBookkeeperContainer* completeCBC = 0;
    TOOL_CHECK("fileExecute()",m_event->retrieveMetaInput(completeCBC, "CutBookkeepers") );

    //AllExecutedEvents
    //number of events in DxAOD: HIGGxxKernel
    const xAOD::CutBookkeeper* DxAODEventsCBK=0;
    std::string kernelName = m_derivationName;
    kernelName+="Kernel";

    // Now, let's actually find the right one that contains all the needed info...
    // Directly from Karsten, see JIRA CXAOD-109
    const xAOD::CutBookkeeper* allEventsCBK = 0;
    int maxCycle = -1;
    for (const auto& cbk: *completeCBC) {
      if (cbk->cycle() > maxCycle && cbk->name() == "AllExecutedEvents" && cbk->inputStream() == "StreamAOD")
      { allEventsCBK = cbk; maxCycle = cbk->cycle(); }
      if ( cbk->name() == kernelName ){
        DxAODEventsCBK = cbk;
      }
    }

    uint64_t nEventsProcessed  = 0;
    //double sumOfWeightsSquared = 0;
    double sumOfWeightsProcessed       = 0;

    uint64_t nEventsDxAOD           = 0;
    //double sumOfWeightsSquaredDxAOD = 0;
    double sumOfWeightsDxAOD        = 0;

    if(!allEventsCBK){
      Error("fileExecute()","Failed to retrieve CutBookkeeper AllExecutedEvents - needed for normalization! Exiting.");
      return EL::StatusCode::FAILURE;
    }
    else{
      nEventsProcessed      = allEventsCBK->nAcceptedEvents();
      //sumOfWeightsSquared   = allEventsCBK->sumOfEventWeightsSquared();
      sumOfWeightsProcessed = allEventsCBK->sumOfEventWeights();
    }

    if(!DxAODEventsCBK){
      Error("fileExecute()","Failed to retrieve CutBookkeeper %s! Keep going, not crucial...", m_derivationName.c_str());
    }
    else{
      nEventsDxAOD             = DxAODEventsCBK->nAcceptedEvents();
      //sumOfWeightsSquaredDxAOD = DxAODEventsCBK->sumOfEventWeightsSquared();
      sumOfWeightsDxAOD        = DxAODEventsCBK->sumOfEventWeights();
    }

    // read meta data
    Info("fileExecute()", "Meta data from this file:");
    Info("fileExecute()", "Initial  events         = %lu", nEventsProcessed);
    Info("fileExecute()", "Selected events         = %lu", nEventsDxAOD );
    Info("fileExecute()", "Initial  sum of weights = %.2f", sumOfWeightsProcessed);
    Info("fileExecute()", "Selected sum of weights = %.2f", sumOfWeightsDxAOD);
    m_histEventCount -> Fill(1, nEventsProcessed);// nEvents initial
    m_histEventCount -> Fill(2, nEventsDxAOD);// nEvents selected in
    m_histEventCount -> Fill(4, sumOfWeightsProcessed);// sumOfWeights initial
    m_histEventCount -> Fill(5, sumOfWeightsDxAOD);// sumOfWeights selected in

  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::changeInput(bool /*firstFile*/) {
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initialize() {
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  Info("initialize()", "Initialize.");

  Info("initialize()", "Dumping config...");
  m_config->print();


  // turn off sending xAOD summary access report (to be discussed)
  bool m_enableDataSubmission = true;
  m_config->getif<bool>("enableDataSubmission", m_enableDataSubmission);
  if( !m_enableDataSubmission ){
    Info("initialize()", "Turning off xAOD access report.");
    xAOD::TFileAccessTracer::enableDataSubmission(false);
  }

  // modular initialization for easier derivation (initializes m_isMC)
  EL_CHECK("initialize()",initializeMetadata() );
  EL_CHECK("initialize()",initializeEvent()     );
  EL_CHECK("initialize()",initializeSampleInfo());
  EL_CHECK("initialize()",initializeVariations());
  EL_CHECK("initialize()",initializeHandlers()  );

  EL_CHECK("initialize()",initializeSelector() );
  EL_CHECK("initialize()",initializeTools());
  EL_CHECK("initialize()",initializeSelection());

  m_config->getif<bool>("applyJetRegression",m_applyJetRegression);
  if ( m_applyJetRegression ) {
    EL_CHECK("initialize()",initializeRegression());
  }

  m_config->getif<bool>("applyJetSemileptonic",m_applyJetSemileptonic);
  m_config->getif<bool>("countElectronInJet",m_countElectronInJet);
  m_config->getif<bool>("correctMuonInJet",m_correctMuonInJet);

  //we need to initialize jet semileptonic even when we do not want to calculate
  //jet semileptonic, as we need the function of set a 4-vector to the jet
  //signal state also for Truth and TruthWZ and we want to be able to run this
  //even when not running the jet semileptonic quantities
  //if ( m_applyJetSemileptonic ) {
  EL_CHECK("initialize()",initializeSemileptonic());
  //}

  // initialize overlap register accessor (in write mode)
  bool applyOverlapRemoval = false;
  m_config->getif<bool>("applyOverlapRemoval",applyOverlapRemoval);
  bool writeOverlapRegister = false;
  m_config->getif<bool>("writeOverlapRegister",writeOverlapRegister);
  m_overlapRegAcc = nullptr;
  if ( applyOverlapRemoval && writeOverlapRegister ) {
    Info("initialize()","Initialized to write OverlapRegister to output");
    m_overlapRegAcc = new OverlapRegisterAccessor(OverlapRegisterAccessor::WRITE);
    m_selector->setOverlapRegisterAccessor( m_overlapRegAcc );
  }

  bool printVariations;
  m_config->getif<bool>("printKnownVariations", printVariations);
  if (printVariations) printKnownVariations();

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeEvent() {
  Info("initializeEvent()", "Initialize event.");

  // get TEvent
  m_event = wk()->xaodEvent();

  std::cout << "DEBUG set config store for analysis context." << std::endl;
  if (!m_analysisContext) {
    m_analysisContext=new Ov::AnalysisContext;
  }
  m_analysisContext->setWorker(wk());
  m_analysisContext->setConfigStore(m_config);

  // flag for debug output
  m_config->getif<bool>("debug",m_debug);

  //Pass debug to m_linker
  //m_linker.SetDebug(m_debug);

  // sample name
  TString sampleName = wk()->metaData()->castString("sample_name");
  Info("initialize()", "Sample name = %s", sampleName.Data());
  // as a check, let's see the number of events in our xAOD (long long int)
  Info("initialize()", "Number of events in file   = %lli", m_event->getEntries());
  if(m_maxEvent < 0) m_maxEvent = m_event->getEntries() ;
  Info("initialize()", "Number of events to run on = %li", m_maxEvent);


  // count number of events
  m_eventCounter = 0;
  m_eventWeightCounter = 0;

  // output xAOD
  TFile * file = wk()->getOutputFile ("CxAOD");
  if(!m_event->writeTo(file).isSuccess()){
    Error("initialize()", "Failed to write event to output file!");
    return EL::StatusCode::FAILURE;
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeVariations() {
  Info("initializeVariations()", "Initialize variations.");

  m_variations.clear();
  // Nominal is always present
  m_variations.push_back("Nominal");
  // Original (not calibrated)
  bool storeOrig;
  m_config->getif<bool>("storeOriginal", storeOrig);
  if (storeOrig) {
    m_variations.push_back("Original");
  }

  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);
  // if users forget to update the sample_info.txt before production
  // and the production is done with systematics,
  // assume these missed samples are nominal and run with systematics
  // at worse it runs longer, but then you do not have to run again with systematics later
  bool storeOnlyNominalNoSyst=false;
  if (nominalOnly) {
    // all samples are stored only for nominal and no systematics
    storeOnlyNominalNoSyst=true;
  } else {
    // we asked to run with systematics, but we do not run all samples with systematics
    // we check if the sample DSID (m_mcChanNr) exists in the file sample_info.txt
    bool existsInMap=m_NominalDSIDs.count(m_mcChanNr);
    if (existsInMap) {
      // if it exists in the file, we trust the value written there and follow the instructions
      bool isNominalSample=(m_NominalDSIDs[m_mcChanNr]==true);
      if (isNominalSample) {
	// it is a nominal sample, meaning we want to run and store nominal with systematics
	storeOnlyNominalNoSyst=false;
      } else {
	// it is an alternative sample, meaning we want to run and store only nominal without systematics
	storeOnlyNominalNoSyst=true;
      } // done if isNominalSample
    } else {
      // if it does not exist because we forgot to add, be safe and assume we want to run with systematics
      // so that we do not discover we want to run with syst and then have to run again on the grid
      storeOnlyNominalNoSyst=false;
    } // done if existsInMap
  } // done if nominalOnly
  if (storeOnlyNominalNoSyst) {
    return EL::StatusCode::SUCCESS;
  }

  // variations
  std::vector<std::string> variations;
  m_config->getif< std::vector<std::string> >("variations", variations);
  std::vector<std::string> pulls = {"1down", "1up"};
  for (std::string name : variations) {
    for (std::string pull : pulls) {
      m_variations.push_back((name + "__" + pull).c_str());
    }
  }
  std::vector<std::string> weightVariations;
  m_config->getif< std::vector<std::string> >("weightVariations", weightVariations);
  // Add weight variations to variations vector
  for (std::string name : weightVariations) {
    for (std::string pull : pulls) {
      m_variations.push_back((name + "__" + pull).c_str());
    }
  }

  std::vector<std::string> oneSideVariations;
  m_config->getif< std::vector<std::string> >("oneSideVariations", oneSideVariations);
  // Add variations with are just defined in one flavor
  for (std::string name : oneSideVariations) {
    m_variations.push_back(name);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeHandlers() {
  Info("initializeHandlers()", "Initialize handlers.");

  // initialize EventInfoHandler
  m_eventInfoHandler = new EventInfoHandler(*m_config, m_event);
  //pass information if running on derivation and MC / data to EventInfoHandler in order to have it available in the other object handlers
  m_eventInfoHandler->set_isMC(m_isMC);
  m_eventInfoHandler->set_isDerivation(m_isDerivation);
  m_eventInfoHandler->set_derivationName(m_derivationName);
  m_eventInfoHandler->set_isAFII(m_isAFII);
  m_eventInfoHandler->set_pTag(m_pTag);
  m_eventInfoHandler->set_hasNoFJDetectorEta(m_hasNoFJDetectorEta);
  m_eventInfoHandler->set_mcChanNr(m_mcChanNr);

  // check pile up reweight Files contain MC dataset before initializing PU reweight in EventInfoHandler
  if(m_isMC) EL_CHECK("AnalysisBase::initializeHandlers()", checkPileupRwFiles());

  EL_CHECK("initializeHandlers()",m_eventInfoHandler->initialize());

  if (m_isMC) {
    m_truthProcessor = new TruthProcessor("truthProcessor", *m_config, m_event, *m_eventInfoHandler);
  }

  // note: the names of the handlers determine which collections are
  //       read, see also framework-run.cfg

  // these have global pointer to be used e.g. in event selection
  if (m_isMC) {
    m_truthEventHandler = registerHandler<TruthEventHandler>("truthEvent");
    m_truthjetHandler      = registerHandler<TruthJetHandler>("truthJet");
    m_truthfatjetHandler      = registerHandler<TruthJetHandler>("truthfatJet");
    m_truthWZjetHandler    = registerHandler<TruthJetHandler>("truthWZJet");
    m_truthWZfatjetHandler    = registerHandler<TruthJetHandler>("truthWZfatJet");

  }
  m_muonHandler     = registerHandler<MuonHandler>("muon");
  m_tauHandler      = registerHandler<TauHandler>("tau");
  m_ditauHandler      = registerHandler<DiTauJetHandler>("diTauJet");
  m_electronHandler = registerHandler<ElectronHandler>("electron");
  m_forwardElectronHandler = registerHandler<ForwardElectronHandler>("forwardElectron");
  m_photonHandler   = registerHandler<PhotonHandler>("photon");
  m_jetHandler      = registerHandler<JetHandler>("jet");
  m_fatjetHandler   = registerHandler<FatJetHandler>("fatJet");
  m_fatjetAltHandler   = registerHandler<FatJetHandler>("fatJetAlt");
  m_trackjetHandler = registerHandler<TrackJetHandler>("trackJet");
  m_vrtrackjetHandler = registerHandler<TrackJetHandler>("vrtrackJet");
  if(m_event->contains<xAOD::JetContainer>("AntiKt10LCTopoTrimmedPtFrac5SmallR20ExCoM2SubJets")) m_comtrackjetHandler = registerHandler<TrackJetHandler>("comtrackJet");
  m_METHandler      = registerHandler<METHandler>("MET");
  m_METHandlerMJTight = registerHandler<METHandler>("METMJTight");
  m_METHandlerFJVT = registerHandler<METHandler>("METFJVT");
  m_METHandlerMJMUTight = registerHandler<METHandler>("METMJMUTight");
  m_METHandlerMJMiddle = registerHandler<METHandler>("METMJMiddle");
  m_METHandlerMJLoose = registerHandler<METHandler>("METMJLoose");

  // these are spectators: they are calibrated and written to output,
  //                       but are not used in the event selection

  registerHandler<JetHandler>("jetSpectator");
  m_METTrackHandler = registerHandler<METHandler>("METTrack");
  if (m_isMC) {
    registerHandler<METHandler>("METTruth");
  }

  // for tau truth matching
  if (m_tauHandler) {
    m_tauHandler->setTruthProcessor(m_truthProcessor);
  }


  // Set trigger stream info if data
  // need to catch embedding once the time has come
  bool m_isEmbedding=false;
  if( !m_isMC && !m_isEmbedding ){
    TString sampleName = wk()->metaData()->castString("sample_name");
    if(sampleName.Contains("Egamma")) m_eventInfoHandler->set_TriggerStream(1);
    if(sampleName.Contains("Muons")) m_eventInfoHandler->set_TriggerStream(2);
    if(sampleName.Contains("Jet")) m_eventInfoHandler->set_TriggerStream(3);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeSelector() {
  Info("initializeSelector()", "Initialize selector.");

  // initialize EventSelector
  m_selector = new EventSelector(*m_config);

  m_selector->setJets(m_jetHandler);
  m_selector->setFatJets(m_fatjetHandler);
  m_selector->setFatJetsAlt(m_fatjetAltHandler);
  m_selector->setTrackJets(m_trackjetHandler);
  m_selector->setCoMSubJets(m_comtrackjetHandler);
  m_selector->setMuons(m_muonHandler);
  m_selector->setTaus(m_tauHandler);
  m_selector->setDiTaus(m_ditauHandler);
  m_selector->setElectrons(m_electronHandler);
  m_selector->setForwardElectrons(m_forwardElectronHandler);
  m_selector->setPhotons(m_photonHandler);
  m_selector->setMET(m_METHandler);
  m_selector->setMETMJTight(m_METHandlerMJTight);
  m_selector->setMETFJVT(m_METHandlerFJVT);
  m_selector->setMETMJMUTight(m_METHandlerMJMUTight);
  m_selector->setMETMJMiddle(m_METHandlerMJMiddle);
  m_selector->setMETMJLoose(m_METHandlerMJLoose);
  m_selector->setMETTrack(m_METTrackHandler);
  m_selector->setEventInfo(m_eventInfoHandler);
  m_selector->setTruthEvent(m_truthEventHandler);
  m_selector->setTruthElectronHandler(m_truthProcessor);
  m_selector->setTruthMuonHandler(m_truthProcessor);

  EL_CHECK("EventSelector::initialize",m_selector->initialize());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeRegression() {

  bool tmp_muInJet;
  m_config->getif<bool>("applyMuonInJetCorrection",tmp_muInJet);
  if(m_applyJetRegression && tmp_muInJet) Info("initializeRegression()", "You try to run the regression and the muInJet correction. The regression is not designed for that.");
  Info("initializeRegression()", "Initialize jet regression.");

  m_regression = new JetRegression(*m_config);

  m_regression->setJets(m_jetHandler);
  m_regression->setMuons(m_muonHandler);
  m_regression->setElectrons(m_electronHandler);
  m_regression->setTruthProcessor(m_truthProcessor);
  m_regression->setEventInfo(m_eventInfoHandler);
  m_regression->setTrackJets(m_trackjetHandler);
  m_regression->setTruthJets(m_truthjetHandler);

  if (!m_analysisContext) return EL::StatusCode::FAILURE;
  std::cout << "DEBUG inititalize jet regression." << std::endl;
  m_regression->initialize(*m_analysisContext);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeSemileptonic() {
  Info("initializeSemileptonic()", "Initialize jet semileptonic corrections (muon-in-jet, PtReco, neutrino corrections).");
  m_semileptonic = new JetSemileptonic();
  Info("initializeSemileptonic()","before m_semileptonic->initialize();");
  EL_CHECK("initializeSemileptonic()",m_semileptonic->initialize(m_debug));
  Info("initializeSemileptonic()","after m_semileptonic->initialize();");
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase::initializeTools() {
  Info("initializeTools", "Initialize CP tools and systematics.");

  //---------------------------
  // initialize tools and set systematics
  //---------------------------

  if (m_isMC) {
    m_eventInfoHandler->setJets(m_jetHandler);
    EL_CHECK("initializeTools()",m_eventInfoHandler->addVariations(m_variations));
  }

  for (ObjectHandlerBase * handler : m_objectHandler) {
    EL_CHECK("initializeTools()",handler->initializeTools());
    if (m_isMC) {
      EL_CHECK("initializeTools()",handler->addCPVariations(m_variations));
    }
    EL_CHECK("initializeTools()",handler->addCutflows());
  }

  if (m_METHandler) {
    m_METHandler->setJets(m_jetHandler);
    m_METHandler->setMuons(m_muonHandler);
    m_METHandler->setTaus(m_tauHandler);
    m_METHandler->setElectrons(m_electronHandler);
    m_METHandler->setPhotons(m_photonHandler);


    if (m_isMC) {
      EL_CHECK("initializeTools()", m_METHandler->addParticleVariations());
    }
  }

  if (m_METTrackHandler) {
    m_METTrackHandler->setJets(m_jetHandler);
    m_METTrackHandler->setMuons(m_muonHandler);
    m_METTrackHandler->setTaus(m_tauHandler);
    m_METTrackHandler->setElectrons(m_electronHandler);
    m_METTrackHandler->setPhotons(m_photonHandler);


    if (m_isMC) {
      EL_CHECK("initializeTools()", m_METTrackHandler->addParticleVariations());
    }
  }

  if (m_METHandlerMJTight) {
    m_METHandlerMJTight->setJets(m_jetHandler);
    m_METHandlerMJTight->setMuons(m_muonHandler);
    m_METHandlerMJTight->setTaus(m_tauHandler);
    m_METHandlerMJTight->setElectrons(m_electronHandler);
    m_METHandlerMJTight->setPhotons(m_photonHandler);

    if (m_isMC) {
      EL_CHECK("initializeTools()", m_METHandlerMJTight->addParticleVariations());
    }
  }
  if (m_METHandlerFJVT) {
    m_METHandlerFJVT->setJets(m_jetHandler);
    m_METHandlerFJVT->setMuons(m_muonHandler);
    m_METHandlerFJVT->setTaus(m_tauHandler);
    m_METHandlerFJVT->setElectrons(m_electronHandler);
    m_METHandlerFJVT->setPhotons(m_photonHandler);

    if (m_isMC) {
      EL_CHECK("initializeTools()", m_METHandlerFJVT->addParticleVariations());
    }
  }
  if (m_METHandlerMJMUTight) {
    m_METHandlerMJMUTight->setJets(m_jetHandler);
    m_METHandlerMJMUTight->setMuons(m_muonHandler);
    m_METHandlerMJMUTight->setTaus(m_tauHandler);
    m_METHandlerMJMUTight->setElectrons(m_electronHandler);
    m_METHandlerMJMUTight->setPhotons(m_photonHandler);

    if (m_isMC) {
      EL_CHECK("initializeTools()", m_METHandlerMJMUTight->addParticleVariations());
    }
  }
  if (m_METHandlerMJMiddle) {
    m_METHandlerMJMiddle->setJets(m_jetHandler);
    m_METHandlerMJMiddle->setMuons(m_muonHandler);
    m_METHandlerMJMiddle->setTaus(m_tauHandler);
    m_METHandlerMJMiddle->setElectrons(m_electronHandler);
    m_METHandlerMJMiddle->setPhotons(m_photonHandler);

    if (m_isMC) {
      EL_CHECK("initializeTools()", m_METHandlerMJMiddle->addParticleVariations());
    }
  }
  if (m_METHandlerMJLoose) {
    m_METHandlerMJLoose->setJets(m_jetHandler);
    m_METHandlerMJLoose->setMuons(m_muonHandler);
    m_METHandlerMJLoose->setTaus(m_tauHandler);
    m_METHandlerMJLoose->setElectrons(m_electronHandler);
    m_METHandlerMJLoose->setPhotons(m_photonHandler);

    if (m_isMC) {
      EL_CHECK("initializeTools()", m_METHandlerMJLoose->addParticleVariations());
    }
  }
  return EL::StatusCode::SUCCESS;
}


EL::StatusCode AnalysisBase::execute() {

  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.
  
  if (m_debug) {
    Info("execute()", "Called.");
  }

  // print every 100 events, so we know where we are:
  if ( (m_eventCounter % 100) == 0 || m_debug ) {
    Info("execute()", "Event number = %li", m_eventCounter);
  }
  ++m_eventCounter;


  //----------------------------
  // Event information
  //---------------------------
  EL_CHECK("execute()",m_eventInfoHandler->executeEvent());

  //---------------------------
  // check the the event to be investigated
  //---------------------------
  int EventNumberToInvestigate = -1;
  m_config->getif<int>("EventNumberToInvestigate", EventNumberToInvestigate); // the event number to investigate
  if ( EventNumberToInvestigate > 0 ) {
    if ( !InvestigateThisEvent( EventNumberToInvestigate ) ) { // it is not the event to investigate
      cleanUpEvent(false);
      return EL::StatusCode::SUCCESS;
    }
  }


  if(m_isMC){
    // sum the weights of this job
    m_eventWeightCounter += m_eventInfoHandler->get_MCEventWeight();
  }

  //---------------------------
  // Primary vertex check
  //---------------------------

  bool hasPV = Props::hasPV.get(m_eventInfoHandler->getEventInfo());
  if(m_skipNoPV && !hasPV) {
    cleanUpEvent(false);
    Warning("execute()","No Primary vertex found - skipping event");
    return EL::StatusCode::SUCCESS;
  }

  //---------------------------
  // perform object calibrations
  //---------------------------
  
  // before any calibration call the truth processer
  // this reads the truth information so that later it can be used
  if (m_truthProcessor && m_isMC) {
    wk()->xaodStore()->setActive();
    EL_CHECK("execute()", m_truthProcessor->setObjects());
  }

  // now calibrate the reco objects
  for (ObjectHandlerBase * handler : m_objectHandler) {
    // set EventLoops TStore to be the active one
    wk()->xaodStore()->setActive();

    EL_CHECK("execute()",handler->setObjects());
    EL_CHECK("execute()",handler->calibrate());
  }

  if (m_eventInfoHandler) {
    if (m_isMC) {
      if(m_jetHandler)            EL_CHECK("set_JvtSF",m_eventInfoHandler->set_JvtSF());
    }
  }

  //---------------------------
  // perform object selections
  //---------------------------
  for (ObjectHandlerBase * handler : m_objectHandler) {
    EL_CHECK("execute()",handler->select());
  }


  //------------------------------
  //perform jet energy regression
  //------------------------------

  if(m_regression && m_applyJetRegression) {
    m_regression->applyRegression();
  }

  //--------------------------------------------------------
  // count semileptonic jet decays
  // and apply jet corrections, also ptCorr from regression
  // set the number of TruthWZ jets for Sherpa correction
  //--------------------------------------------------------
  if (m_jetHandler) {
    if (m_applyJetSemileptonic) {
      if (m_countElectronInJet && m_electronHandler)      EL_CHECK("execute()",m_jetHandler->countElectronInJet   (m_semileptonic, m_electronHandler->getInParticleVariation("Nominal")));
      if (m_correctMuonInJet   && m_muonHandler)          EL_CHECK("execute()",m_jetHandler->correctMuonInJet     (m_semileptonic, m_muonHandler     ->getInParticleVariation("Nominal")));
    }
    if (m_regression)             EL_CHECK("execute()",m_jetHandler->correctRegressionForJet());
    if (m_isMC) {
     if (m_truthProcessor) {
       if (!m_truthProcessor->isNewStyle()) {
         EL_CHECK("execute()",m_jetHandler->IdentifyVBFJets(m_jetHandler->getInParticleVariation("Nominal"),m_truthProcessor->getInParticle("TruthParticles")));
       } else {
         EL_CHECK("execute()",m_jetHandler->IdentifyVBFJets(m_jetHandler->getInParticleVariation("Nominal"),m_truthProcessor->getInParticle("HardScatterParticles")));
       }
     }
     if (m_truthjetHandler)      EL_CHECK("execute()",m_jetHandler->matchTruthJet   (m_semileptonic,m_truthjetHandler->getInParticleVariation("Nominal"),"Truth",0.4));
      if (m_truthWZjetHandler)    EL_CHECK("execute()",m_jetHandler->matchTruthJet   (m_semileptonic,m_truthWZjetHandler->getInParticleVariation("Nominal"),"TruthWZ",0.4));
    }
  }

  if (m_fatjetHandler) {
    if (m_isMC) {
      if (m_truthfatjetHandler)   EL_CHECK("execute()",m_fatjetHandler->matchTruthJet (m_semileptonic, m_truthfatjetHandler->getInParticleVariation("Nominal"),"Truth",1.0));
      if (m_truthWZfatjetHandler) EL_CHECK("execute()",m_fatjetHandler->matchTruthJet (m_semileptonic, m_truthWZfatjetHandler->getInParticleVariation("Nominal"),"TruthWZ",1.0));
    }
  }

  if (m_fatjetAltHandler) {
    if (m_isMC) {
      if (m_truthfatjetHandler)   EL_CHECK("execute()",m_fatjetAltHandler->matchTruthJet (m_semileptonic, m_truthfatjetHandler->getInParticleVariation("Nominal"),"Truth",1.0));
      if (m_truthWZfatjetHandler) EL_CHECK("execute()",m_fatjetAltHandler->matchTruthJet (m_semileptonic, m_truthWZfatjetHandler->getInParticleVariation("Nominal"),"TruthWZ",1.0));
    }
  }

  if (m_eventInfoHandler) {
    if (m_isMC) {
      if(m_truthWZjetHandler)     EL_CHECK("set_NTruthWZJets",m_eventInfoHandler->set_NTruthWZJets(m_truthWZjetHandler->getInParticleVariation("Nominal")));
    }
  }

  //------------------------
  // perform MET calculation
  //------------------------
  if(m_debug) Info("execute()", "checking METTrack");
  if (!m_applyMETAfterSelection) {
    if( m_METHandler ) {
      if(m_debug) Info("execute()", "checking METTrack reg - Run");
      EL_CHECK("execute()",m_METHandler->setMET());
    }
    if( m_METTrackHandler ) {
      if(m_debug) Info("execute()", "checking METTrack - Run");
      EL_CHECK("execute()",m_METTrackHandler->setMET());
    }
    if( m_METHandlerMJTight ) {
      if(m_debug) Info("execute()", "checking METTrack  - MJTight - Run");
      EL_CHECK("execute()",m_METHandlerMJTight->setMET());
    }
    if( m_METHandlerFJVT ) {
      if(m_debug) Info("execute()", "checking METTrack  - FJVT - Run");
      EL_CHECK("execute()",m_METHandlerFJVT->setMET());
    }
    if( m_METHandlerMJMUTight ) {
      if(m_debug) Info("execute()", "checking METTrack  - MJMUTight - Run");
      EL_CHECK("execute()",m_METHandlerMJMUTight->setMET());
    }
    if( m_METHandlerMJLoose ) {
      if(m_debug) Info("execute()", "checking METTrack  - MJLoose - Run");
      EL_CHECK("execute()",m_METHandlerMJLoose->setMET());
    }
    if( m_METHandlerMJMiddle ) {
      if(m_debug) Info("execute()", "checking METTrack  - MJMiddle - Run");
      EL_CHECK("execute()",m_METHandlerMJMiddle->setMET());
    }
  }

  //------------------------
  // do event selection
  //------------------------


  bool eventPassed = false;
  EL_CHECK("execute()",m_selector->performSelection(eventPassed));

  if (m_debug) {
    Info("execute()", "eventPassed = %i", eventPassed);
  }

  //------------------------
  // perform MET calculation alternatively after event selection
  //------------------------
  
  if (m_applyMETAfterSelection) {
    if( m_METHandler ) {
      EL_CHECK("execute()",m_METHandler->setMET());
    }
    if( m_METTrackHandler ) {
      EL_CHECK("execute()",m_METTrackHandler->setMET());
    }
    if( m_METHandlerMJTight ) {
      EL_CHECK("execute()",m_METHandlerMJTight->setMET());
    }
    if( m_METHandlerFJVT ) {
      EL_CHECK("execute()",m_METHandlerFJVT->setMET());
    }
    if( m_METHandlerMJMUTight ) {
      EL_CHECK("execute()",m_METHandlerMJMUTight->setMET());
    }
    if( m_METHandlerMJMiddle ) {
      EL_CHECK("execute()",m_METHandlerMJMiddle->setMET());
    }
    if( m_METHandlerMJLoose ) {
      EL_CHECK("execute()",m_METHandlerMJLoose->setMET());
    }
  }
  
  if (m_isMC && m_truthProcessor) {
    int codeTTBarDecay = -999;
    float final_pTV = -999;
    float final_pTL = -999;
    float final_EtaL = -999;
    bool computeCodeTTBarDecay = true;
    m_config->getif<bool>("computeCodeTTBarDecay", computeCodeTTBarDecay); // computing this variable can cause crashes with some derivations (e.g. HIGG4D1 ttbar)
    if (computeCodeTTBarDecay) {
      EL_CHECK("execute()", m_truthProcessor->getCodeTTBarDecay(codeTTBarDecay, final_pTV,final_pTL,final_EtaL));
    }
    EL_CHECK("execute()", m_eventInfoHandler->setCodeTTBarDecay(codeTTBarDecay));
    EL_CHECK("execute()", m_eventInfoHandler->setTTbarpTW( final_pTV ));
    EL_CHECK("execute()", m_eventInfoHandler->setTTbarpTL( final_pTL ));
    EL_CHECK("execute()", m_eventInfoHandler->setTTbarEtaL( final_EtaL ));

    // when this is activated, the muons from hadron decays are saved in the CxAOD and they are decorated with a flag for easy access
    bool findMuonsFromHadronDecays = true;
    m_config->getif<bool>("findMuonsFromHadronDecays", findMuonsFromHadronDecays); // computing this variable can cause crashes with some derivations (e.g. HIGG4D1 ttbar)
    if (findMuonsFromHadronDecays) {
      EL_CHECK("execute()", m_truthProcessor->findMuonsFromHadronDecay());
    }
  }

  if (m_truthProcessor)
    m_truthProcessor->decorate();
    
  //------------------------
  // compute Sherpa pTV
  //------------------------
  if (m_isMC && m_truthProcessor) {
    EL_CHECK("execute()", m_eventInfoHandler->setSherpapTV( m_truthProcessor->ComputeSherpapTV() ));
  }

  //------------------------
  // save SUSYMET
  //------------------------
  m_setSUSYMET = true;
  m_config->getif<bool>("setSUSYMET",m_setSUSYMET);
  if (m_isMC && m_setSUSYMET) {
    EL_CHECK("execute()", m_eventInfoHandler->setSUSYMET());
  }
  
  //------------------------
  // Diboson isVZbb
  //------------------------
  if (m_isMC && m_truthProcessor && m_truthProcessor->isNewStyle()) {
    EL_CHECK("execute()", m_eventInfoHandler->isVZbb( m_truthProcessor->checkSherpaVZqqZbb() ));
  }
  // first select which truth particles to be saved in the output
  // This needs to be done only after the calls to codeTTbarDecays, because this function call determines the
  // particles in the ttbar decay to be saved in the select() function
  if (m_isMC && m_truthProcessor) {
    EL_CHECK("execute()", m_truthProcessor->select());
  }

  //----------------------
  // fill output container
  //----------------------
  if (eventPassed) {

    for (ObjectHandlerBase * handler : m_objectHandler) {
      // first round of passSel flag setting (required by flagOutputLinks()):
      EL_CHECK("execute()", handler->setPassSelFlags());
    }

    for (ObjectHandlerBase * handler : m_objectHandler) {
      // flag links and second round of passSel flag setting (adding linked objects):
      // note: this procedure supports only one layer of links
      EL_CHECK("execute()", handler->flagOutputLinks());
      EL_CHECK("execute()", handler->setPassSelForLinked());
      EL_CHECK("execute()", handler->fillOutputContainer());
      handler->countObjects();
    }

    for (ObjectHandlerBase * handler : m_objectHandler) {
      // fill output links (requires fillOutputContainer() called for all handlers)
      EL_CHECK("execute()", handler->fillOutputLinks());
    }

    m_histEventCount -> Fill(3); // nEvents selected out
    if (m_isMC)
      m_histEventCount -> Fill(6, m_eventInfoHandler->get_MCEventWeight()); // sumOfWeights selected out

    // initialize event info output container for writing variations
    EL_CHECK("execute()",m_eventInfoHandler->setOutputContainer());
    EL_CHECK("execute()",m_selector->fillEventVariables());
    EL_CHECK("execute()",m_eventInfoHandler->fillOutputContainer());
    if (m_isMC && m_truthProcessor) {
      EL_CHECK("execute()", m_truthProcessor->fillOutputContainer());
    }
    // record overlap register
    if ( m_overlapRegAcc ) {
      EL_CHECK("execute()",m_overlapRegAcc->recordRegister(m_event));
    }

  }

  // TEST of alternative ntuple dumper -- don't remove this hook!
  EL_CHECK("execute()",executeCustom(eventPassed));

  EL_CHECK("execute()",cleanUpEvent(eventPassed));
  return EL::StatusCode::SUCCESS;
}

  //-------------------------
  // clean memory allocations
  //-------------------------
EL::StatusCode AnalysisBase::cleanUpEvent(bool eventPassed) {
  for (ObjectHandlerBase * handler : m_objectHandler) {
    EL_CHECK("execute()",handler->clearEvent());
  }

  // skip all further execute() and postExecute() if !eventPassed
  if (!eventPassed) {
    // event info is cleared here or in postExecute()
    EL_CHECK("execute()",m_eventInfoHandler->clearEvent());
    wk()->skipEvent();
  }

  // clear overlap register (memory management - has no effect if register was recorded)
  if ( m_overlapRegAcc ) {
    EL_CHECK("execute()",m_overlapRegAcc->clearRegister());
  }

  m_linker.clear();
  return EL::StatusCode::SUCCESS;

}

EL::StatusCode AnalysisBase::postExecute() {
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.

  if (m_debug) {
    Info("postExecute()", "Called.");
  }

  //--------------------------
  //save the event in the xAOD
  //--------------------------
  if(m_event->fill() < 0){
    Error("execute()", "Failed writing event to output file!");
  };

  EL_CHECK("postExecute()",m_eventInfoHandler->clearEvent());

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode AnalysisBase::finalize() {
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events

  Info("finalize()", "Finalize.");

  // finalize and close our output xAOD file ( and write MetaData tree)
  TFile * file = wk()->getOutputFile("CxAOD");
  TOOL_CHECK("finalize()",m_event->finishWritingTo( file ));

  if ( m_applyJetRegression ) m_regression->finalize();


  // print object counters
  bool printCounts;
  m_config->getif<bool>("printObjectCounts", printCounts);
  if (printCounts) {
    Info("finalize()", "Printing number of objects for the selected events:");
    for (ObjectHandlerBase * handler : m_objectHandler) {
      // print the size of the arrays
      handler->printObjectCounts();
    }
  }

  // write cut flow counters
  for (ObjectHandlerBase * handler : m_objectHandler) {
    std::map<TString, CutFlowCounter*> counter = handler->getCutFlowCounter();
    for (auto cf : counter) {
      TH1D* cutflowHisto = cf.second->getCutFlow(std::string("CutFlow_") + cf.first + std::string("/"));
      wk()->addOutput(cutflowHisto);
    }
  }
  bool applySelection = false;
  m_config->getif<bool>("applyEventSelection", applySelection);
  if (applySelection) {
    CutFlowCounter counter_preselection = m_selector->getCutFlowCounter();
    TH1D* cutflowHisto_preselection = counter_preselection.getCutFlow("CutFlow/");
    wk()->addOutput(cutflowHisto_preselection);
  }

  // delete handlers
  delete m_eventInfoHandler;
  for (ObjectHandlerBase * handler : m_objectHandler) {
    delete handler;
    handler = 0;
  }

  // delete overlap register accessor
  delete m_overlapRegAcc;
  m_overlapRegAcc = nullptr;

  Info("finalize()", "Number of events:");
  Info("finalize()", "  in input file (meta data) = %li", (long) m_histEventCount -> GetBinContent(2));
  Info("finalize()", "  limit (config)            = %li", m_maxEvent);
  Info("finalize()", "  processed                 = %li", m_eventCounter);
  Info("finalize()", "  written to output         = %li", (long) m_histEventCount -> GetBinContent(3));

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode AnalysisBase::histFinalize() {
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.

  Info("histFinalize()", "Finalize histograms.");

  // write meta-data histograms to CxAOD file directly,
  TFile * file = wk()->getOutputFile("CxAOD");
  if (file && m_histEventCount) {
    m_histEventCount -> Fill(7, m_eventCounter); // events in this job
    m_histEventCount -> Fill(8, m_eventWeightCounter); // sumOfWeights in this job
    file->cd();
    m_histEventCount->Write();
  }

  EL_CHECK("histFinalize()", writeMetaDataTree());

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode AnalysisBase::writeMetaDataTree() {

  TFile * file = wk()->getOutputFile("CxAOD");
  if (!file) {
    return EL::StatusCode::SUCCESS;
  }

  std::string tag;
  m_config->getif<std::string>("vtag", tag);
  if (tag == "") {
    return EL::StatusCode::SUCCESS;
  }

  TTree* tree = new TTree ("MetaData_CxAOD", "MetaData_CxAOD");
  tree->SetDirectory(file);
  tree->Branch ("tag", &tag);
  tree->Fill();

  return EL::StatusCode::SUCCESS;
}


void AnalysisBase::printKnownVariations() {
  Info("printKnownVariations()", "Printing all systematic variations to be run in this job:");
  for (TString variation: m_variations) {
    std::cout << variation.Data() << std::endl;
  }
  for (ObjectHandlerBase * handler : m_objectHandler) {
    handler -> printAllVariations();
  }
}

EL::StatusCode AnalysisBase::checkPileupRwFiles() {
  // Check that the prwFile contains the MC dataset
  // - if not use a default dataset:
  //   Note this copies prwFile to the current directory, modifies it and
  //   then overwrites value in prwFiles config passed to the CxAOD PUReweightingTool

  // we need the dataset id of the present file - get from the EventInfoHandler
  TString mcchannel("");
  int datasetid = m_eventInfoHandler->get_mcChanNr();

  if (datasetid<0) {
    Error("checkPileupRwFiles","datasetid set incorrectly (%i) -> prw unreliable",datasetid);
    return EL::StatusCode::FAILURE;
  }
  mcchannel+=datasetid;

  Info("checkPileupRwFiles","Checking pile-up reweight file for dataset %s",mcchannel.Data());

  // Open prwFile and see if dataset id is present
  std::vector<std::string> prwInFiles = m_config->get< std::vector<std::string> >("configFiles");
  std::vector<std::string> prwOutFiles;
  bool prwfilesModified=false;
  bool dsInFile=false;
  std::map< TString, bool> dsInFiles; 
  TFile* prwFileIn=nullptr;
  TString prwIn;
  for (auto prwf : prwInFiles) {
    Info("checkPileupRwFiles","prwf %s",prwf.c_str());

    //Expand path to file
    prwIn = gSystem->ExpandPathName(prwf.c_str());
    Info("checkPileupRwFiles","prwIn %s",prwIn.Data());

    //For each configuration file, initialise the presence of the necessary DSID info as false
    dsInFiles.insert( std::pair<TString, bool>(prwIn, false) );

    //the PRW and actualMu config files contain different naming schemes and directory structure
    //we will skip the actualMu ones when checking if the dataset is in any of those
    if( prwIn.Contains("actualMu") ) {
      Info("checkPileupRwFiles","prwIn %s contains ActualMu, so we skip it",prwIn.Data());
      dsInFiles[prwIn] = true;
      continue;
    }

    // check if prw histogram exists in file for this dataset
    prwFileIn = TFile::Open(prwIn);
    Info("checkPileupRwFiles","prwFileIn %i",bool(prwFileIn));

    if (prwFileIn == nullptr) {
      Error("checkPileupRwFiles","prwIn=%s could not be opened by ROOT, so we ABORT!!!",prwIn.Data());
      return EL::StatusCode::FAILURE;
    }

    prwFileIn->cd("PileupReweighting");  
    TIter nextkey(gDirectory->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) nextkey())) {
      
      //Extract name of key
      TString name = key -> GetName();

      //Check if key contains DSID number of MC
      if (name.Contains(mcchannel)) {
	if(!dsInFile) dsInFile=true;
	dsInFiles[prwIn] = true;
	break;
      }
    }
    
    //Break once we have a match
    //if (dsInFile) break;

  } // end of loop over prwFiles - use first prwFile with match

  //Check that all config files contain the relevant DSID info
  std::map<TString,bool>::iterator DsIter1 = dsInFiles.begin();
  std::map<TString,bool>::iterator endDsIter1 = dsInFiles.end();
  for( ; DsIter1 != endDsIter1; DsIter1++){
    if( !DsIter1->second ){
      Info("checkPileupRwFiles", "Configuration file %s does not contain dataset %s info", DsIter1->first.Data(), mcchannel.Data());
      dsInFile = false;
    }
  }
  
  Info("checkPileupRwFiles", "dsInFile %i", dsInFile);
  Info("checkPileupRwFiles", "prwFileIn %i", bool(prwFileIn));

  if(dsInFile && prwFileIn) {
    Info("checkPileupRwFiles","PileupReweight file %s \n contains prw info for dataset %s\n",prwIn.Data(),mcchannel.Data());
    prwOutFiles.push_back(prwIn.Data()); // for overwriting prwFiles in config steering if needed
  } else {
    prwfilesModified=true;
    Info("checkPileupRwFiles","The provided prw config files do not contain prw info for dataset %s - using a default sample\n", mcchannel.Data());

    // determine from input string if mc16a (data15+data16), mc16c or mc16d (data17), or mc16e (data18)
    TString mcprod(""); // for print out
    TString defchannel(""); // default DSID
    TString defstartrun(""); //default run number
    if (prwIn.Contains("mc16a")){
      mcprod="mc16a";
      defchannel="410470"; // large sample ttbar
      defstartrun="284500";
    } else if (prwIn.Contains("mc16c")){
      mcprod="mc16c";
      defchannel="410470"; // large sample ttbar
      defstartrun="300000";
    } else if (prwIn.Contains("mc16d") || (prwIn.Contains("data17") && prwIn.Contains("actualMu"))){
      mcprod="mc16d";
      defchannel="410470"; // large sample ttbar
      defstartrun="300000";
    } else if (prwIn.Contains("mc16e") || (prwIn.Contains("data18") && prwIn.Contains("actualMu"))){
      mcprod="mc16e";
      defchannel="410470"; // large sample ttbar
      defstartrun="310000";
    } else {
      Error("checkPileupRwFiles","prwIn fileName %s does not contain neither mc16a, mc16c, mc16d, nor mc16e.",prwIn.Data());
      return EL::StatusCode::FAILURE;
    }

    Info("checkPileupRwFiles","Given prw file %s \n corresponds to mcprod  %s - using pilup profile from channel %s for this period \n",prwIn.Data(),mcprod.Data(),defchannel.Data());

    //Loop through dsInFiles map and either copy prwInFiles config file name or create a new one
    DsIter1 = dsInFiles.begin();
    for( ; DsIter1 != endDsIter1; DsIter1++){

      if(DsIter1->second){
	//Then simply copy the configuration file entered by user
	prwOutFiles.push_back(DsIter1->first.Data()); // for overwriting prwFiles in config steering if needed
      }else{

	//Need to take default pileup info for defchannel
	// get the default template histogram
	TString defhist_str = "PileupReweighting/pileup_chan"+defchannel+"_run"+defstartrun;
	TH1D* defpileup=nullptr;

	//Open file and extract default mc channel info
	prwFileIn = TFile::Open(DsIter1->first);
	defpileup = (TH1D*)prwFileIn->Get(defhist_str);
	
	//Check that it was opened and extract succesfully
	if (!defpileup) {
	  Error("checkPileupRwFiles","Unable to find default pileup histogram %s in file %s",defhist_str.Data(),DsIter1->first.Data());
	  return EL::StatusCode::FAILURE;
	}
	
	//Info for user
	Info("checkPileupRwFiles","%s PRW file In: %s for channel %s and start run %s",mcprod.Data(),DsIter1->first.Data(),mcchannel.Data(),defstartrun.Data());
	
	// Output file - in current directory
	TString prwOut=gSystem->BaseName(DsIter1->first);
	if (!prwOut) {
	  Error("checkPileupRwFiles","Unable to get basename of %s",DsIter1->first.Data());
	  return EL::StatusCode::FAILURE;
	}
	prwOut.ReplaceAll(".root","_modified_"+mcchannel+".root");
	Info("checkPileupRwFiles","%s PRW file Out: %s for channel %s and start run %s",mcprod.Data(),prwOut.Data(),mcchannel.Data(),defstartrun.Data());
	
	// Copy to output file;
	// https://root.cern.ch/root/html602/TSystem.html#TSystem:CopyFile
	// returns 0 when successfull 
	// returns -2 if file already exists and overwrite is set to false, so not copying, which is what we want
	bool overwrite=true;
	int result=gSystem->CopyFile(DsIter1->first.Data(),prwOut.Data(),overwrite);
	if (m_debug) {
	  Info("checkPileupRwFiles","result=%i",result);
	}
	if (overwrite==true) {
	  if(result!=0) {
	    Error("checkPileupRwFiles","Unable to copy PURW reweight file %s, though you asked to overwrite",DsIter1->first.Data());
	    return EL::StatusCode::FAILURE;
	  }
	} else {
	  if (result!=0 && result!=-2) {
	    Error("checkPileupRwFiles","You asked the PURW reweight file %s not to overwrite and if already exists, so we should not overwrite, and if it exists we should copy it, but there was some problem, as the result of the copy was %i.",DsIter1->first.Data(),result);
	    return EL::StatusCode::FAILURE;
	  }
	} //done if overwrite

	//different copy for luminosity and actualMu based configuration files
	if(DsIter1->first.Contains("actualMu")){
	  //do nothing for now. Keep for possible further implementations of actualMu PRW config file modification
	}else{
	  // open copy
	  TFile* prwFileOut = TFile::Open(prwOut,"UPDATE");
	  prwFileOut->cd("PileupReweighting"); // for writing histogram
	  // copy def prw histogram and set name correctly
	  TString outhist_str("pileup_chan"+mcchannel+"_run"+defstartrun);
	  TH1D* outpileup=(TH1D*)defpileup->Clone(outhist_str);
	  outpileup->SetTitle(outhist_str);
	  // copy/add mchannel to prw Ntuple tree
	  TTree* outTreeMC = (TTree*)prwFileOut->Get("PileupReweighting/MCPileupReweighting");
	  Int_t channel = 0;UInt_t runNumber = 0;
	  std::vector<UInt_t> pStarts;
	  std::vector<UInt_t> pEnds;
	  std::vector<UInt_t>* pStartsPtr = &pStarts;
	  std::vector<UInt_t>* pEndsPtr = &pEnds;
	  Char_t histName[150];
	  outTreeMC->SetBranchAddress("Channel",&channel);
	  outTreeMC->SetBranchAddress("RunNumber",&runNumber);
	  outTreeMC->SetBranchAddress("PeriodStarts",&pStartsPtr);
	  outTreeMC->SetBranchAddress("PeriodEnds",&pEndsPtr);
	  outTreeMC->SetBranchAddress("HistName",&histName);
	  // std::cout << "Entries before " << outTreeMC->GetEntries() << std::endl;
	  // fill first value
	  outTreeMC->GetEntry(0);
	  // overwrite what data we need
	  channel=datasetid;
	  TString histName2=outhist_str;
	  strncpy(histName,histName2,sizeof(histName)-1);
	  
	  outTreeMC->Fill();
	  //std::cout << "Entries after " << outTreeMC->GetEntries() << std::endl;
	  prwFileOut->Write();
	  prwOutFiles.push_back(prwOut.Data()); // for overwriting prwFiles in config steering
	  
	}

      }

    }
    
  }
  
  // overwrite the prwFiles config steering if a prwFile needed to be copied
  if (prwfilesModified) {
    bool overwriteconfig=true;

    //Diagnostic
    std::cout << "<AnalysisBase::checkPileupRwFiles()>::   prwOutFiles : ";
    for( auto prwOutString : prwOutFiles){
      std::cout << "  " << prwOutString ;
    }
    std::cout << std::endl;
    
    m_config->put("configFiles",prwOutFiles,overwriteconfig);
  }

  return EL::StatusCode::SUCCESS;

}

bool AnalysisBase::InvestigateThisEvent( int eventNumber_check ) {
  int eventNumber_this = m_eventInfoHandler->getEventInfo()->eventNumber();
  if ( eventNumber_this == eventNumber_check ) {
    Info("InvestigateThisEvent()", "The event to be investigated is found: %d",eventNumber_this);
    return true;
  }
  else return false;
}
