#ifndef CxAODMaker_AnalysisBase_H
#define CxAODMaker_AnalysisBase_H

#include "EventLoop/Algorithm.h"

// Infrastructure include(s):
#include "CxAODTools/ConfigStore.h"
#include "CxAODTools/ParticleLinker.h"

// Forward declarations
class Init;
class TEvent;
class ObjectHandlerBase;
class JetHandler;
class FatJetHandler;
class TrackJetHandler;
class TruthJetHandler;
class MuonHandler;
class TauHandler;
class DiTauJetHandler;
class ElectronHandler;
class ForwardElectronHandler;
class PhotonHandler;
class METHandler;
class EventInfoHandler;
class TruthProcessor;
class TruthEventHandler;
class EventSelector;
class JetRegression;
class JetSemileptonic;
class OverlapRegisterAccessor;
class LumiMetaDataTool;

namespace Ov {
 class IAnalysisMain;
 class AnalysisContext;
}


class AnalysisBase : public EL::Algorithm {

protected:

  // vector of all ObjectHandlers, including defaults
  std::vector<ObjectHandlerBase *> m_objectHandler; //!

  // default ObjectHandlers used for MET, OR and selection
  JetHandler* m_jetHandler; //!
  FatJetHandler* m_fatjetHandler; //!
  FatJetHandler* m_fatjetAltHandler; //!
  TrackJetHandler* m_trackjetHandler; //!
  //VRTrackJetHandler* m_vrtrackjetHandler;//!
  TrackJetHandler* m_vrtrackjetHandler;//!
  TrackJetHandler* m_comtrackjetHandler;//!
  TruthJetHandler* m_truthjetHandler; //!
  TruthJetHandler* m_truthfatjetHandler; //!
  TruthJetHandler* m_truthWZjetHandler; //!
  TruthJetHandler* m_truthWZfatjetHandler; //!
  MuonHandler* m_muonHandler; //!
  ElectronHandler* m_electronHandler; //!
  ForwardElectronHandler* m_forwardElectronHandler; //!
  PhotonHandler* m_photonHandler; //!
  TauHandler* m_tauHandler; //!
  DiTauJetHandler* m_ditauHandler; //!
  EventInfoHandler * m_eventInfoHandler; //!
  TruthEventHandler * m_truthEventHandler; //!
  FatJetHandler* m_fatjetSpectatorHandler; //!
  TruthJetHandler*  m_truthSpectatorFatJetHandler; //!


  std::vector<TString> m_variations; //!
  std::map<int, bool> m_NominalDSIDs; //!

#ifndef __MAKECINT__
  ParticleLinker m_linker; //!
  METHandler* m_METHandler; //!
  METHandler* m_METHandlerMJTight; //!
  METHandler* m_METHandlerFJVT; //!
  METHandler* m_METHandlerMJMUTight; //!
  METHandler* m_METHandlerMJMiddle; //!
  METHandler* m_METHandlerMJLoose; //!
  METHandler* m_METTrackHandler; //!
  EventSelector* m_selector; //!
  JetRegression* m_regression; //!
  JetSemileptonic* m_semileptonic; //!
  LumiMetaDataTool* m_lumiMetaData; //!
  Ov::IAnalysisMain* m_analysisContext; //!
#endif // not __MAKECINT__

  ConfigStore * m_config;
  bool m_debug; //!
  bool m_isMC; //!
  bool m_isDerivation; //!
  std::string m_derivationName; //!
  bool m_isAFII; //!
  std::string m_pTag; //!
  bool m_hasNoFJDetectorEta; //!
  int m_mcChanNr; //!

  bool m_skipNoPV;
  bool m_applyJetRegression;
  bool m_applyJetSemileptonic;
  bool m_countElectronInJet;
  bool m_correctMuonInJet;
  bool m_applyMETAfterSelection;
  bool m_setSUSYMET;

  // some helper functions
  template <class handlerType>
  handlerType* registerHandler(std::string name);
  void printKnownVariations();


  // these are called from initialize() in fixed order
  virtual EL::StatusCode initializeMetadata();
  virtual EL::StatusCode initializeEvent();
  virtual EL::StatusCode initializeSampleInfo() = 0;
  virtual EL::StatusCode initializeVariations();
  virtual EL::StatusCode initializeHandlers() = 0;
  virtual EL::StatusCode initializeSelector();
  virtual EL::StatusCode initializeTools();
  virtual EL::StatusCode initializeSelection() = 0;
  virtual EL::StatusCode initializeRegression();
  virtual EL::StatusCode initializeSemileptonic();

  // hook at the end of execute()
  // (for testing alternative ntuple dumper...)
  virtual EL::StatusCode executeCustom(bool /*eventPassed*/) { return EL::StatusCode::SUCCESS; }

  EL::StatusCode checkPileupRwFiles(); // check prw histogram is present for MC
  bool InvestigateThisEvent( int event_number ); // only process the event with the event_number

public:
  // Tree *myTree; //!
  // TH1 *myHist; //!
  xAOD::TEvent * m_event; //!
  long m_maxEvent;
  long m_eventCounter; //!
  double m_eventWeightCounter; //!
  TH1* m_histEventCount; //!
  bool m_derivation20_7; //!

  // overlap register accessor
  OverlapRegisterAccessor * m_overlapRegAcc; //!

  // this is a standard constructor
  AnalysisBase();
  ~AnalysisBase();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob(EL::Job& job) override;
  virtual EL::StatusCode fileExecute() override;
  virtual EL::StatusCode histInitialize() override;
  virtual EL::StatusCode changeInput(bool firstFile) override;
  virtual EL::StatusCode initialize() override;
  virtual EL::StatusCode execute() override;
  virtual EL::StatusCode postExecute() override;
  virtual EL::StatusCode finalize() override;
  virtual EL::StatusCode histFinalize() override;

  void setConfig(ConfigStore * config) { m_config = config; }

  virtual EL::StatusCode cleanUpEvent(bool eventPassed);
  virtual EL::StatusCode writeMetaDataTree();

protected:
  TruthProcessor *m_truthProcessor; //!

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisBase, 1);
};

#include "CxAODMaker/AnalysisBase.icc"

#endif
