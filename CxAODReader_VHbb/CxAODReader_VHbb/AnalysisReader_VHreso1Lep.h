#ifndef CxAODReader_AnalysisReader_VHreso1Lep_H
#define CxAODReader_AnalysisReader_VHreso1Lep_H

#include "../CorrsAndSysts/CorrsAndSysts/BDTSyst.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ1Lep.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

class AnalysisReader_VHreso1Lep : public AnalysisReader_VHQQ1Lep {
 public:
  AnalysisReader_VHreso1Lep();
  ~AnalysisReader_VHreso1Lep();

  EL::StatusCode run_1Lep_analysis() override;
  EL::StatusCode fill_VBF() override;
  ClassDefOverride(AnalysisReader_VHreso1Lep, 1);

  // Initialize event - overloads (but calls) AnalysisReader::initializeEvent()
  // Reads all VHreso specific settings from the config
  EL::StatusCode initializeEvent() override;
  EL::StatusCode selectObjects();
  bool ttbarDiLeptonVeto();
  EL::StatusCode mergepTWFilteredttbar();
  void fatJetMuOR();
  EL::StatusCode computeJetsAndTagging();
  EL::StatusCode setEventFlavour(std::vector<const xAOD::Jet *> jets);
  EL::StatusCode setJetVariables();
  EL::StatusCode setFatJetVariables();
  EL::StatusCode setHiggsCandidate();
  // EL::StatusCode selectedEventJets();
  EL::StatusCode inmBBWindow();
  EL::StatusCode setLeptonVariables();
  EL::StatusCode setMetAndMetSignificance();
  void setevent_nJets(const std::vector<const xAOD::Jet *> &signal,
                      const std::vector<const xAOD::Jet *> &forward,
                      const std::vector<const xAOD::Jet *> &track,
                      const std::vector<const xAOD::Jet *> &fat);
  EL::StatusCode rescale_jets();
  EL::StatusCode
  setVHCandidate();  // Would be nice to still keep this two seperate
  EL::StatusCode computeVHCandidate();  // Might be better way to do it though
  EL::StatusCode setSumpt();
  EL::StatusCode doEventFlag();
  bool signalLepton();
  bool passMETCut(std::string regime);
  bool passmTWCut(std::string regime);
  bool passpTVCut(std::string regime);
  bool passdRBBCut();
  EL::StatusCode setCuts();
  EL::StatusCode selectCandidateVectors();
  EL::StatusCode setHistNameSvcDescription();
  void select_regime();
  EL::StatusCode updateWeights();
  EL::StatusCode doSystematics();
  void doBlinding();
  EL::StatusCode printEventWeights();
  bool passSpecificCuts(const unsigned long int flag,
                        const std::vector<unsigned long int> &cuts,
                        const std::vector<unsigned long int> &excludeCuts);
  EL::StatusCode fillEasyTree();
  EL::StatusCode fill_1lepCutFlow(std::string regime);
  EL::StatusCode doInputPlots();
  EL::StatusCode doBasicPlots();
  EL::StatusCode doExtendedPlots();
  EL::StatusCode doTLVPlots();
  EL::StatusCode doSystPlots();
  EL::StatusCode printCuts();
  EL::StatusCode printStats();
  std::string printVector(TLorentzVector v);
  EL::StatusCode initializeSelection() override;

 private:
  // cuts and corrections
  bool m_doBJetEnergyCorr;
  bool m_doMbbRescaling;
  bool m_doIsoInv;
  bool m_doBlindingData;
  bool m_doBlindingMC;
  bool m_applyFatJetMuOR;
  bool m_mVHvsPtvCut;

  // plot/tree saving options
  bool m_doBasicPlots;
  bool m_doInputPlots;
  bool m_doMJEtree;
  bool m_doExtendedPlots;
  bool m_doTLVPlots;
  bool m_doMbbRejHist;
  bool m_reduceHistograms;

  bool m_writeMVATree;
  bool m_writeEasyTree;

  // misc options
  bool m_printEventWeightsForCutFlow;
  bool m_doCutflow;
  bool m_doMergeCR;
  bool m_nominalOnly;

  bool m_useTTBarPTWFiltered1LepSamples;

  bool m_applyPUWeight;
  bool m_writeObjectsInEasyTree;

  struct EventVariables {
    EventVariables();
    // xAOD objects from event selection
    const xAOD::Electron *el;
    const xAOD::Muon *mu;
    const xAOD::MissingET *met;
    std::vector<const xAOD::Jet *> signalJets;
    std::vector<const xAOD::Jet *> forwardJets;
    std::vector<const xAOD::Jet *> fatJets;
    std::vector<const xAOD::Jet *> trackJets;
    std::vector<const xAOD::Jet *> selectedJets;
    std::vector<const xAOD::TauJet *> taus;
    Lepton lepton;
    double triggerSF;
    bool isTriggered;

    // For define signal region / control regions
    bool inmBBresolvedWindow;
    bool inmBBmergedWindow;
    bool inmBBresolvedrejWindow;
    bool inmBBmergedrejWindow;

    int nLooseEl;
    int nSigEl;
    int nLooseMu;
    int nSigMu;
    // xAOD track jet objects
    std::vector<const xAOD::Jet *> matchedLeadTrackJets;
    std::vector<const xAOD::Jet *> unmatchedTrackJets;
    std::vector<const xAOD::Jet *> trackJetsForLabelling;
    std::vector<const xAOD::Jet *> trackJetsForBTagging;
    // jets with additional information like e.g. muons in jets
    std::vector<Jet> selectedEventJets;
    std::vector<Jet> selectedEventFatJets;
    // MET significance variables
    float metSig;
    float metSig_PU;
    float metSig_soft;
    float metSig_hard;
    float metOverSqrtSumET;
    float metOverSqrtHT;
    // MET vector
    MET resolvedMET;
    MET mergedMET;
    TLorentzVector metVec;
    // MPT vector
    TLorentzVector mptVec;
    // resolved and merged higgs candidates
    Higgs resolvedH;
    Higgs mergedH;
    // VH candidates
    VHCand resolvedVH;
    VHCand mergedVH;
    // Some Lorentz vectors that are used for kinematic information
    // Gives you the option to choose nominal, corrected, rescaled 4-momenta
    TLorentzVector HVec;         // Higgs candidate
    TLorentzVector VHVec;        // VH candidate
    TLorentzVector bbjVec;       // signal jets plus third selected jet
    TLorentzVector bbjVec_Corr;  // signal jets plus third selected jet
    TLorentzVector WVec;         // W
    int isResolved;
    int isMerged;
    double leptonSF;    // TODO are these needed?
    double btagWeight;  // TODO
    float Mtop;         // TODO
    // Sumpt
    float sumpt;
    // Variables determined during cutflow
    float dPhiMETMPT;
    float dPhiMETdijetResolved;
    unsigned long int eventFlag;
    // specific cuts that define signal and control regions in resolved and
    // merged
    std::vector<unsigned long int> cuts_common_resolved;
    std::vector<unsigned long int> cuts_SR_resolved;
    std::vector<unsigned long int> cuts_jet_hists_resolved;
    std::vector<unsigned long int> cuts_common_merged;
    std::vector<unsigned long int> cuts_SR_merged;
  } m_vars;  //!
};

#endif  // ifndef CxAODReader_AnalysisReader_VHreso1Lep_H
