// (c) ATLAS 2018

#ifndef CxAODReader_AnalysisReader_VHreso0Lep_H
#define CxAODReader_AnalysisReader_VHreso0Lep_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ0Lep.h"

class TLorentzVector;

struct Jet;     // from AnalysisReader_VHQQ.h
struct Higgs;   // from AnalysisReader_VHQQ.h
struct VHCand;  // from AnalysisReader_VHQQ.h

class AnalysisReader_VHreso0Lep : public AnalysisReader_VHQQ0Lep {
  // This class implements the event selection for the search for Vh resonances

  // public methods
 public:
  AnalysisReader_VHreso0Lep();
  ~AnalysisReader_VHreso0Lep();

  EL::StatusCode run_0Lep_analysis() override;

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHreso0Lep, 1);

  // Apply b tagging weight
  EL::StatusCode ApplyBTaggingWeight();

  // Evaluate jet selection and do b tagging based on selected jets
  EL::StatusCode DoJetsAndTagging();

  // Evaluate which cuts the event passes, this is stored in the event flag
  EL::StatusCode DoEventFlag();

  // Write the histograms for number of jets
  EL::StatusCode DoJetHistos();

  // Write resolved histograms and trees
  EL::StatusCode DoResolvedOutput();

  // Write merged histograms and trees
  EL::StatusCode DoMergedOutput();

  // Write EasyTree for all events independent of their Regime
  EL::StatusCode FillEasyTree();

  // ttbar variations
  EL::StatusCode DoTTbarVariations();

  // V+jets variations
  EL::StatusCode DoVJetsVariations();

  // Initialize event - overloads (but calls) AnalysisReader::initializeEvent()
  // Reads all VHreso specific settings from the config
  EL::StatusCode initializeEvent() override;

  // Get the Lorentz vectors that we are using for kinematic information
  EL::StatusCode SelectCandidateVectors();

  // Get objects from event selection and store in member variables
  EL::StatusCode SelectObjects();

  // Set the cuts that are used in resolved and merged
  EL::StatusCode SetCuts();

  // Set the event flavor that determines the histogram names
  EL::StatusCode SetEventFlavour();

  // Set MET and MET significance
  EL::StatusCode SetMetAndMetSignificance();

  // Configure the histogram naming service for the current region
  EL::StatusCode SetHistNameSvcDescription();

  // Set MPT vector
  EL::StatusCode SetMpt();

  // Set Sumpt
  EL::StatusCode SetSumpt();

  // Update all settings that depend on member variables
  EL::StatusCode UpdateSettings() noexcept;

  // overloads that are only intended for this specific implementation
  // usually exact same functionality as the method in the base class, except
  // that is uses member variables and has the same interface as all the other
  // methods in the VHreso class to make the run_0Lep_analysis method easier to
  // read
 protected:
  EL::StatusCode setJetVariables();
  EL::StatusCode setFatJetVariables();
  EL::StatusCode setVHCandidate();
  EL::StatusCode setHiggsCandidate();
  EL::StatusCode fill0LepCutFlow();
  EL::StatusCode fillMVATreeResolved();
  EL::StatusCode fillMVATreeBoosted();

 protected:
  // Use by this class for filling shared resolved/merged variables in the easy
  // tree
  EL::StatusCode SetCommonBranchesAndValues();

  // private members, mainly additional variables that used to be in the body of
  // the large run_0Lep_analysis method
 private:
  // variables from the config are read during initializeEvent (only once), thus
  // they are persistent for the whole class and not part of the EventVariables
  bool m_nominalOnly;
  bool m_alwaysFillEasyTree;  //! fill easytree for every event
  bool m_applyPUWeight;  //! store w/ & w/o PU weight as systematic variation
  bool m_applyTauVeto;   //! tau veto in event selection
  bool m_doTrackJetOverlapVeto;  //! veto events with overlapping VRs
  bool m_doMETSignificanceCut;   //! do the MET significance cut
  bool m_generateSTXSsignals;    //! generate STXS signals (?)
  bool m_writeEasyTree;          //! write easy tree to output

  // all the other event variables that are reset at every call of
  // run_0Lep_analysis
  struct EventVariables {
    EventVariables();
    // xAOD objects from event selection
    const xAOD::MissingET *met;
    std::vector<const xAOD::Jet *> signalJets;
    std::vector<const xAOD::Jet *> forwardJets;
    std::vector<const xAOD::Jet *> fatJets;
    std::vector<const xAOD::Jet *> trackJets;
    std::vector<const xAOD::Jet *> selectedJets;
    std::vector<const xAOD::TauJet *> taus;
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
    TLorentzVector HVec;    // Higgs candidate
    TLorentzVector VHVec;   // VH candidate
    TLorentzVector bbjVec;  // signal jets plus third selected jet
    // Sumpt
    float sumpt;
    // Variables determined during cutflow
    float dPhiMETMPT;
    float dPhiMETdijetResolved;
    float dPhiMETdijetMerged;
    float dRb1b2;
    float mindPhi;
    float mindPhiMerged;
    float mindPhiMerged_3LeadJets;  // TODO
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

#endif  // ifned CxAODReader_AnalysisReader_VHreso0Lep_H
