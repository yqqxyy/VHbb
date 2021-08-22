#ifndef CxAODReader_AnalysisReader_H
#define CxAODReader_AnalysisReader_H

#include <algorithm>
#include <utility>
#include <vector>

#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

#ifndef __MAKECINT__
#include "CxAODReader/HistSvc.h"
#include "CxAODTools/BTaggingTool.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/EventSelection.h"
#include "CxAODTools/EvtWeightVariations.h"
#include "CxAODTools/FakeFactor.h"
#include "CxAODTools/OverlapRegisterAccessor.h"
#include "CxAODTools/PUReweightingTool.h"
#include "CxAODTools/PtRecoTool.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODTools/XSectionProvider.h"
#include "CxAODTools/sumOfWeightsProvider.h"
#endif  // not __MAKECINT__

#include <TH1D.h>
#include <TH1F.h>
#include "CxAODTools/ConfigStore.h"

#include "CxAODReader/ObjectReader.h"
#include "xAODRootAccess/TStore.h"

class TriggerTool;
class PtRecoTool;

class AnalysisReader : public EL::Algorithm {
 protected:
  xAOD::TStore
      m_TStore;  // An instance of TStore is needed if any CP::tool is usd in the class!

  xAOD::TEvent *m_event;  // !

  virtual EL::StatusCode initializeCxAODTag();
  virtual EL::StatusCode initializeEvent();
  virtual EL::StatusCode initializeReader();
  virtual EL::StatusCode initializeSelection();
  virtual EL::StatusCode initializeTools();
  virtual EL::StatusCode finalizeTools();
  virtual EL::StatusCode initializeIsMC();
  virtual EL::StatusCode initializeChannel();
  virtual EL::StatusCode initializeSumOfWeights();
  virtual EL::StatusCode initializeVariations();

  virtual EL::StatusCode initializeValidationSelection();
  virtual EL::StatusCode fill_validation();

  virtual EL::StatusCode applyVariationToTools(TString sysName);
  virtual EL::StatusCode setEventWeight();
  virtual EL::StatusCode doOverlapRemoval(std::string varName);

  EL::StatusCode clearEmptyContainersWithNonZeroSize();

  // this function pointer has to be set to some
  // fill function in initializeSelection()
  std::function<EL::StatusCode()> m_fillFunction;            // !
  std::function<EL::StatusCode()> m_validationFillFunction;  // !

  // example fill function
  EL::StatusCode fill_example();

  EL::StatusCode checkEventDuplicates();
  EL::StatusCode checkSherpaTruthPt(bool &pass, float &sherpaTruthPt);
  EL::StatusCode checkPowhegTruthMtt(bool &pass, float &powhegTruthMtt);
  EL::StatusCode checkSherpaVZqqZbb(bool &pass, std::string m_varName);
  EL::StatusCode applyMCEventWeight();
  EL::StatusCode applyExtension0LepTTbarWeight();
  EL::StatusCode applyExtension0LepZnunuWeight();
  EL::StatusCode applyLumiWeight();
  EL::StatusCode applyPUWeight();
  EL::StatusCode applyVZqqZbbWeight(int DSID);

  EL::StatusCode initializePtRecoHistograms();
  EL::StatusCode getTLVFromJet(const xAOD::Jet *jet, TLorentzVector &tlv,
                               const std::string &name4Vec = "OneMu");
  EL::StatusCode getBJetEnergyCorrTLV(
      const xAOD::Jet *jet, TLorentzVector &veccor, bool isFatJet,
      const std::string &bJetEnergyCorr = "OneMu");
  EL::StatusCode applyRtrkUncertFix(const xAOD::Jet *jet);

  float computeBTagSFWeight(const std::vector<const xAOD::Jet *> &signalJets,
                            const std::string &authorName = "");

  void getNeutrinoPz(double MET, double MET_phi, const TLorentzVector &lepVec,
                     double &nu_pz1, double &nu_pz2);
  double getNeutrinoPz(double MET, double MET_phi, const TLorentzVector &lepVec,
                       bool min = true);
  TLorentzVector getNeutrinoTLV(const TLorentzVector &metVec,
                                const TLorentzVector &lepVec, bool min);

  void getGAHeavyFlavHadronLabels_PtSort(
      const xAOD::Jet *jet, std::vector<std::pair<int, float> > &label) const;
  void getGAHeavyFlavHadronLabels_PtSort(const xAOD::Jet *jet, int &jetflav1,
                                         int &jetflav2) const;
  int getGAHeavyFlavHadronLabels_leadPt(const xAOD::Jet *jet) const;

  bool checkMCEventWeight(); // Method to discard events with large MCEventWeights seen in ggZllHbb (MC16d), ggZvvHbb (MC16e), ggZllHcc (MC16e) and ggZvvHcc (MC16e) [to be removed when xAOD prodcution fixed]

 public:
  static bool sortPtGAHeavyFlavHadrons(std::pair<int, float> had1,
                                       std::pair<int, float> had2);

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)

 public:
  // this is a standard constructor
  AnalysisReader();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob(EL::Job &job) override;
  virtual EL::StatusCode fileExecute() override;
  virtual EL::StatusCode histInitialize() override;
  virtual EL::StatusCode changeInput(bool firstFile) override;
  virtual EL::StatusCode initialize() override;
  virtual EL::StatusCode execute() override;
  virtual EL::StatusCode postExecute() override;
  virtual EL::StatusCode finalize() override;
  virtual EL::StatusCode histFinalize() override;
  virtual EL::StatusCode executePreEvtSel() { return EL::StatusCode::SUCCESS; };

  void setConfig(ConfigStore *config) { m_config = config; }

  void setSumOfWeightsFile(const std::string sumOfWeightsFile) {
    m_sumOfWeightsFile = sumOfWeightsFile;
  }

 protected:
  // list of all object reader
  std::vector<ObjectReaderBase *> m_objectReader;  // !

  // object reader to handle all variations of one object container
  ObjectReader<xAOD::EventInfo> *m_eventInfoReader;                   // !
  ObjectReader<xAOD::MissingETContainer> *m_METReader;                // !
  ObjectReader<xAOD::MissingETContainer> *m_METMJTightReader;         // !
  ObjectReader<xAOD::MissingETContainer> *m_METMJMuTightReader;       // !
  ObjectReader<xAOD::MissingETContainer> *m_MPTReader;                // !
  ObjectReader<xAOD::MissingETContainer> *m_truthMETReader;           // !
  ObjectReader<xAOD::ElectronContainer> *m_electronReader;            // !
  ObjectReader<xAOD::ElectronContainer> *m_forwardElectronReader;     // !
  ObjectReader<xAOD::PhotonContainer> *m_photonReader;                // !
  ObjectReader<xAOD::MuonContainer> *m_muonReader;                    // !
  ObjectReader<xAOD::TauJetContainer> *m_tauReader;                   // !
  ObjectReader<xAOD::DiTauJetContainer> *m_ditauReader;               // !
  ObjectReader<xAOD::JetContainer> *m_jetReader;                      // !
  ObjectReader<xAOD::JetContainer> *m_fatJetReader;                   // !
  ObjectReader<xAOD::JetContainer> *m_fatJetAltReader;                // !
  ObjectReader<xAOD::JetContainer> *m_trackJetReader;                 // !
  ObjectReader<xAOD::JetContainer>    *m_subJetReader;                // !
  ObjectReader<xAOD::TruthEventContainer> *m_truthEventReader;        // !
  ObjectReader<xAOD::TruthParticleContainer> *m_truthParticleReader;  // !
  ObjectReader<xAOD::TruthParticleContainer> *m_truthTauReader;       // !
  ObjectReader<xAOD::TruthParticleContainer> *m_truthMuonReader;      // !
  ObjectReader<xAOD::TruthParticleContainer> *m_truthElectronReader;  // !
  ObjectReader<xAOD::TruthParticleContainer> *m_truthNeutrinoReader;  // !
  ObjectReader<xAOD::JetContainer> *m_truthWZJetReader;               // !
  ObjectReader<xAOD::JetContainer>         *m_truthWZFatJetReader;    // !
  ObjectReader<xAOD::EventInfo> *m_mmcReader;                         //!

  // pointers to the current variation of objects
  // can be directly accessed from fill functions,
  // but it is safer to use the result of a selection class
  // (for consistent pre-selection, pT-sorting, ...)
#ifndef __MAKECINT__
  const xAOD::EventInfo *m_eventInfo;          // !
  const xAOD::EventInfo *m_mmc;                // !
  const xAOD::ElectronContainer *m_electrons;  // !
  const xAOD::ElectronContainer *m_forwardElectrons;  // !
  const xAOD::PhotonContainer *m_photons;      // !
  const xAOD::MuonContainer *m_muons;          // !
  const xAOD::TauJetContainer *m_taus;         // !
  const xAOD::DiTauJetContainer *m_ditaus;     // !
  const xAOD::JetContainer *m_jets;            // !
  const xAOD::JetContainer *m_fatJets;         // !
  const xAOD::JetContainer *m_fatJetsAlt;      // !
  const xAOD::JetContainer *m_trackJets;       // !
  const xAOD::JetContainer *m_subJets;       // !
  const xAOD::MissingETContainer *m_metCont;   // !
  const xAOD::MissingET *m_met;                // !
  const xAOD::MissingET *m_met_soft;           // !
  const xAOD::MissingETContainer *m_mptCont;   // !
  // const xAOD::MissingETContainer *m_truthCont;        // !
  const xAOD::MissingET *m_mpt;                          // !
  const xAOD::MissingET *m_truthMET;                        // !
  const xAOD::TruthEventContainer *m_truthEvent;         // !
  const xAOD::TruthParticleContainer *m_truthParts;      // !
  const xAOD::TruthParticleContainer *m_truthMuons;      // !
  const xAOD::TruthParticleContainer *m_truthTaus;       // !
  const xAOD::TruthParticleContainer *m_truthElectrons;  // !
  const xAOD::TruthParticleContainer *m_truthNeutrinos;  // !
  const xAOD::JetContainer *m_truthWZJets;               // !
  const xAOD::JetContainer *m_truthWZFatJets;            // ! 
#endif   // not __MAKECINT__

#ifndef __MAKECINT__
  template <class containerType>
  ObjectReader<containerType> *registerReader(std::string name);
#endif  // not __MAKECINT__

  std::string
      m_currentVar;  //! need this to pass the variation name to the derived classes

  BTaggingTool *m_bTagTool;             // !
  EvtWeightVariations *m_EvtWeightVar;  //!
  long m_eventCounter;                  // !
  long m_eventCountPassed;              // !
  bool m_isMC;                          // !
  std::string m_period;                 // !
  int m_mcChannel;                      // !
  int m_mcChannelFromInputFile;         // !
  //QG Tagging
  typedef std::map<float,std::pair<TLorentzVector,int>> TJ_jetsinevent_t; //!
  typedef std::map<int,TJ_jetsinevent_t> TJ_eventinrun_t; //!
  typedef std::map<int,TJ_eventinrun_t> TJ_dsid_t; //!
  TJ_dsid_t m_truthjetsmap; //!
  bool m_qgtruthmatch; //!
  int m_qg_truth_map_status; //!
  int computeQGTruthMap(std::string inputFile);

  double m_weight;                      // !
  int m_maxEvents;  // ! do we have a max number of events to run on?
  float
      m_luminosity;  // ! for rescaling the MC to a particular value of the luminosity (default is 1 pb-1)
  double m_dataOneOverSF;   // ! for plotting mu properly
  bool m_isSherpaVJets;     // ! is Sherpa file
  bool m_isSherpaPt0WJets;  // ! is present sample SherpaWJets Pt0
  bool m_isSherpaPt0ZJets;  // ! is present sample SherpaZJets Pt0
  bool m_isSherpa; // !
  bool m_lowerShMCWeightThreshold; // !
  bool m_isHighWeight; // !
  std::vector<std::string> m_triggerSystList;  // !
  int m_randomRunNumber;                       // !
  float m_pileupReweight;                      // !
  float m_averageMu;                           // !
  float m_actualMu;                            // !
  float m_averageMuScaled;                     // !
  float m_actualMuScaled;                      // !

  int m_metMJCalc;
  float m_jetPtCut;
  float m_jetRapidityCut;
  float m_lepPtCut;
  ConfigStore *m_config;

  // list of weight variations for HistSvc
  std::vector<HistSvc::WeightSyst> m_weightSysts;    //!
  std::vector<HistSvc::WeightSyst> m_weightSystsCS;  //!

  bool m_debug;                           // !
  bool m_validation;                      // !
  std::vector<std::string> m_variations;  // !
  bool m_applyEventPreSelection;          // !
  bool m_allowORwithTruthSelection;       // !
  bool m_applySherpaTruthPtCut;           // !
  bool m_applyPowhegTruthMttCut;          // !
  float m_usePowhegInclFraction;          // !
  bool m_computePileupReweight;           // !
  bool m_recomputePileupReweight;         // !
  std::string m_CxAODTag;                 // !
  bool m_doICHEP = false;                 // ! 

  bool m_checkEventDuplicates;                                          //!
  bool m_failEventDuplicates;                                           //!
  std::map<long int, std::map<long int, int> > m_eventCountDuplicates;  //!
  bool m_putAllSysInOneDir;                                             //!
  bool m_recomputeMuTrigSF;                                             //!
  bool m_doQCD;                                                         //!
  bool m_doFakeFactor;                                                  //!
  bool m_useContinuous;                                                 //!
  //add VZbb filtered samples
  std::string m_applyVZbbWeight;  //!

  //combMass begin
  bool m_doCombMass;      //!
  bool m_doCombMassMuon;  //!
  virtual EL::StatusCode combMassTmp1(bool &isCombMassSyst,
                                      std::string &varName);
  virtual EL::StatusCode combMassTmp2(bool &isCombMassSyst);
  //combMass end

  // yield file can be set from executable
  std::string m_sumOfWeightsFile;


#ifndef __MAKECINT__
  HistSvc *m_histSvc;                            // !
  HistSvc *m_truthHistSvc;                       // !
  HistNameSvc *m_histNameSvc;                    // !
  HistNameSvc *m_truthHistNameSvc;               // !
  EventSelection *m_eventSelection;              // !
  EventSelection *m_eventPostSelection;          // !
  XSectionProvider *m_xSectionProvider;          // !
  sumOfWeightsProvider *m_sumOfWeightsProvider;  // !
  TriggerTool *m_triggerTool;                    // !
  PUReweightingTool *m_puReweightingTool;        // !
  PtRecoTool *m_PtRecoTool;                      // !
  OverlapRegisterAccessor *m_overlapRegAcc;      // !
  FakeFactor *m_FakeFactor_el;                   // !
  FakeFactor *m_FakeFactor_mu;                   // !
#endif      // not __MAKECINT_


  // Add switch to discard events with large MC weights, samples concerned are ggZllHbb (MC16d), ggZvvHbb (MC16e), ggZllHcc (MC16e) and ggZvvHcc (MC16e) 
  // Switch should be removed once the bug is fixed from xAODs
  bool m_doLargeMCEventWeightsRemoval;     // !

  int m_evtWeightVarMode; // !

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader, 1);
};

#ifndef __MAKECINT__
#include "CxAODReader/AnalysisReader.icc"
#endif  // not __MAKECINT__

#endif  // ifndef CxAODReader_AnalysisReader_H
