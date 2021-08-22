#include "EventLoop/Job.h"
#include "EventLoop/StatusCode.h"
#include "EventLoop/IWorker.h"
#include "EventLoop/OutputStream.h"

#include "CxAODMaker_VHbb/AnalysisBase_VHbb.h"

#include "CxAODMaker_VHbb/ElectronHandler_VHbb.h"
#include "CxAODMaker_VHbb/MuonHandler_VHbb.h"
#include "CxAODMaker_VHbb/FatJetHandler_VHbb.h"
#include "CxAODMaker_VHbb/JetHandler_VHbb.h"
#include "CxAODMaker/TauHandler.h"
#include "CxAODMaker_VHbb/PhotonHandler_VHbb.h"
#include "CxAODMaker_VHbb/PhotonHandler_VGamma.h"
#include "CxAODMaker/TrackJetHandler.h"
#include "CxAODMaker/TruthJetHandler.h"
#include "CxAODMaker/METHandler.h"
#include "CxAODMaker/TruthProcessor.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "CxAODMaker/EventSelector.h"
#include "CxAODMaker/TruthEventHandler.h"


#include "CxAODTools_VHbb/VHbb0lepEvtSelection.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"
#include "CxAODTools_VHbb/VHbb2lepEvtSelection.h"
#include "CxAODTools_VHbb/VBFHbb1phEvtSelection.h"
#include "CxAODTools_VHbb/VGammaEvtSelection.h"

#include "CxAODTools/OverlapRemoval.h"

// this is needed to distribute the algorithm to the workers
ClassImp(AnalysisBase_VHbb)

EL::StatusCode AnalysisBase_VHbb::initializeSampleInfo() {
  std::cout << "Inside initializeSampleInfo" << std::endl;

  std::ifstream file;
  std::string sampleInfo_File = gSystem->Getenv("WorkDir_DIR");
  sampleInfo_File += "/data/CxAODOperations_VHbb/DxAOD/info/sample_info.txt";

  std::cout << sampleInfo_File << std::endl;

  file.open(sampleInfo_File.c_str());

  while (!file.eof()) {
    // read line
    std::string lineString;
    getline(file, lineString);
    // std::cout << lineString << std::endl;

    // skip empty lines
    // TODO - is there a better way to implement this check?
    // if (lineString.find(".") > 1000) {
    //   continue;
    // }

    // skip lines starting with #
    if (lineString.find("#") == 0) {
      continue;
    }

    // store in map
    std::stringstream line(lineString);
    int dsid;
    std::string shortname;
    std::string longname;
    std::string nominal;
    std::string veto;

    line >> dsid >> shortname >> longname >> nominal >> veto;

    if (m_NominalDSIDs.count(dsid) != 0) {
      Warning("initializeSampleInfo()", "Skipping duplicated mc_channel_number for line '%s'.", lineString.c_str());
      continue;
    }
    m_NominalDSIDs[dsid] = (nominal == "nominal");
  }
  file.close();
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisBase_VHbb::initializeHandlers() {
  Info("AnalysisBase_VHbb::initializeHandlers()","Initialize handlers.");

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
  if(m_isMC) EL_CHECK("AnalysisBase_VHbb::initializeHandlers()", checkPileupRwFiles());

  EL_CHECK("AnalysisBase_VHbb::initializeHandlers()",m_eventInfoHandler->initialize());

  std::string selectionName = "";
  m_config->getif<std::string>("selectionName", selectionName);

  // note: the names of the handlers determine which collections are
  //       read, see also CxAODMaker-job.cfg
  
  // these have global pointer to be used e.g. in event selection
  if (m_isMC) {
    m_truthProcessor = new TruthProcessor("truthProcessor", *m_config, m_event, *m_eventInfoHandler);
    m_truthjetHandler = registerHandler<TruthJetHandler>("truthJet");
    m_truthfatjetHandler = registerHandler<TruthJetHandler>("truthfatJet");
    m_truthWZjetHandler = registerHandler<TruthJetHandler>("truthWZJet");
    m_truthWZfatjetHandler = registerHandler<TruthJetHandler>("truthWZfatJet");
  }
  m_muonHandler     = registerHandler<MuonHandler_VHbb>("muon");
  m_tauHandler      = registerHandler<TauHandler>("tau");
  m_electronHandler = registerHandler<ElectronHandler_VHbb>("electron");
  if (selectionName == "vgamma") m_photonHandler   = registerHandler<PhotonHandler_VGamma>("photon");
  else m_photonHandler   = registerHandler<PhotonHandler_VHbb>("photon");
  m_jetHandler      = registerHandler<JetHandler_VHbb>("jet");
  m_fatjetHandler   = registerHandler<FatJetHandler_VHbb>("fatJet");
  m_trackjetHandler = registerHandler<TrackJetHandler>("trackJet");
  m_vrtrackjetHandler = registerHandler<TrackJetHandler>("vrtrackJet");
  if(m_event->contains<xAOD::JetContainer>("AntiKt10LCTopoTrimmedPtFrac5SmallR20ExCoM2SubJets")) m_comtrackjetHandler = registerHandler<TrackJetHandler>("comtrackJet"); 
  m_METHandler      = registerHandler<METHandler>("MET");
  m_METHandlerMJTight= registerHandler<METHandler>("METMJTight"); 
  m_METHandlerMJMUTight= registerHandler<METHandler>("METMJMUTight"); 
  m_METHandlerMJMiddle= registerHandler<METHandler>("METMJMiddle"); 
  m_METHandlerMJLoose= registerHandler<METHandler>("METMJLoose"); 
  
  // these are spectators: they are calibrated and written to output,
  //                       but are not used in the event selection
   
  registerHandler<JetHandler_VHbb>("jetSpectator");
  m_METTrackHandler = registerHandler<METHandler>("METTrack");

  if (m_isMC) {
    registerHandler<METHandler>("METTruth");
    registerHandler<TruthEventHandler>("truthEvent");
  }
   
  // alternative: manual handler initialization (e.g. with different constructor)
  //  std::string containerName;
  //  std::string name = "muon";
  //  m_config->getif<std::string>(name + "Container", containerName);
  //  if ( ! containerName.empty() ) {
  //    m_muonHandler = new MuonHandler(name, *m_config, m_event, *m_eventInfoHandler);
  //    m_objectHandler.push_back( m_muonHandler );
  //  }

  // for tau truth matching
  if (m_tauHandler) {
    // changed this to use the TruthProcessor
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

EL::StatusCode AnalysisBase_VHbb::initializeSelection() {

  Info("initializeSelection()","...");

  // determine selection name
  
  std::string selectionName = "";
  bool autoDetermineSelection;
  m_config->getif<bool>("autoDetermineSelection", autoDetermineSelection);
  if (!autoDetermineSelection) {
    m_config->getif<std::string>("selectionName", selectionName);
  } else {
    TString sampleName = wk()->metaData()->castString("sample_name");
    if      (sampleName.Contains("HIGG5D1")) selectionName = "0lep";
    else if (sampleName.Contains("HIGG5D2")) selectionName = "1lep";
    else if (sampleName.Contains("HIGG2D4")) selectionName = "2lep";
    else if (sampleName.Contains("HIGG5D3")) selectionName = "vbf";
    //else if (sampleName.Contains("EXOT3")) selectionName = "vgamma";
    else {
      Error("initialize()", "Could not auto determine selection!");
      return EL::StatusCode::FAILURE;
    }
  }

  // initialize event selection

  //new VGammaEvtSelection();

  bool applySelection = false;
  m_config->getif<bool>("applyEventSelection", applySelection);
  if (applySelection) {
    Info("initialize()", "Applying selection: %s", selectionName.c_str());
    if      (selectionName == "0lep") m_selector->setSelection(new VHbb0lepEvtSelection(m_config));
    else if (selectionName == "1lep") m_selector->setSelection(new VHbb1lepEvtSelection(m_config));
    else if (selectionName == "2lep") m_selector->setSelection(new VHbb2lepEvtSelection(m_config));
    //else if (selectionName == "vbf" ) m_selector->setSelection(new VBFHbb1phEvtSelection());
    //else if (selectionName == "vbfa") m_selector->setSelection(new VBFHbb1phEvtSelection());
    else if (selectionName == "vgamma") m_selector->setSelection(new VGammaEvtSelection());
    else {
      Error("initialize()", "Unknown selection requested!");
      return EL::StatusCode::FAILURE;
    }
    
  }

  // initialize overlap removal (possibly analysis specific)

  OverlapRemoval* overlapRemoval = new OverlapRemoval(*m_config);
  EL_CHECK("initializeSelection()",overlapRemoval->initialize());
  m_selector -> setOverlapRemoval(overlapRemoval);

  if (selectionName == "vvjj") {
    if (applySelection && (!m_fatjetHandler)) { 
      Error("initialize()", "Not all collections for event selection are defined!");
      return EL::StatusCode::FAILURE;
    }
  }
  else if (selectionName == "vgamma") {
    if (applySelection && (!m_fatjetHandler || !m_photonHandler)) {
      Error("initialize()", "Not all collections for event selection are defined!");
      return EL::StatusCode::FAILURE;
    }
  }
  else {
    if (applySelection && (!m_muonHandler || !m_electronHandler || !m_jetHandler || !m_METHandler)) {
      Error("initialize()", "Not all collections for event selection are defined!");
      return EL::StatusCode::FAILURE;
    }
  }

  return EL::StatusCode::SUCCESS;
}
