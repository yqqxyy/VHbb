#ifndef CxAODReader_AnalysisReader_VHQQ0Lep_H
#define CxAODReader_AnalysisReader_VHQQ0Lep_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

struct ResultVHbb0lep;

class AnalysisReader_VHQQ0Lep : public AnalysisReader_VHQQ {
 public:
  AnalysisReader_VHQQ0Lep();
  ~AnalysisReader_VHQQ0Lep();

  virtual EL::StatusCode initializeSelection() override;
  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode finalizeTools() override;
  virtual EL::StatusCode run_0Lep_analysis() = 0;

  // METSig
  bool m_doMETSig_PU;  //!

 protected:
  void setVHCandidate(VHCand &resolvedVH, VHCand &mergedVH,
                      TLorentzVector &metVec, Jet &j1, Jet &j2, Jet &fj1);

  bool pass0LepTrigger(double &triggerSF_nominal,
                       ResultVHbb0lep selectionResult);

  void checkMbbWindow(const TLorentzVector &HVecResolved,
                      const TLorentzVector &HVecMerged, bool &passMHResolved,
                      bool &passMHMerged, bool &passMHResolvedSideBand,
                      bool &passMHMergedSideBand);

  void selectRegime(unsigned long int eventFlag,
                    std::vector<unsigned long int> cuts_SR_resolved,
                    std::vector<unsigned long int> cuts_SR_merged,
                    std::vector<unsigned long int> cuts_common_resolved,
                    std::vector<unsigned long int> cuts_common_merged);

  std::vector<const xAOD::Jet *> set_JetsForAntiQCD(
      std::vector<const xAOD::Jet *> fatJets,
      std::vector<const xAOD::Jet *> signalJets,
      std::vector<const xAOD::Jet *> forwardJets, unsigned int pTthreshold);

  EL::StatusCode fill0LepCutFlow(unsigned long int eventFlag);

  EL::StatusCode fillMVATreeVHbbResolved0Lep(
      TLorentzVector &b1, TLorentzVector &b2, TLorentzVector &j3,
      const std::vector<const xAOD::Jet *> &selectedJets,
      const std::vector<const xAOD::Jet *> &signalJets,
      const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &HVec,
      TLorentzVector &metVec, const xAOD::MissingET *&met, int nTaus = 0,
      float bTagWeight = 1);

  EL::StatusCode fillMVATreeBoosted(
      const std::vector<const xAOD::Jet *> &signalJets,
      const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &mptVec,
      TLorentzVector &metVec, const int nGoodFatJet, const int nMatchedTrackJet,
      TLorentzVector &HVec, double dPhiMETMPT, double mindPhiMerg,
      double mindPhiMerg_3LeadJets, VHCand mergedVH, TLorentzVector &VHVec);

  EL::StatusCode fillMVATreeBoosted_C2(
      const std::vector<const xAOD::Jet *> &fatJets);

  EL::StatusCode fill0LepHistMerged_C2();

  EL::StatusCode fillETreeResolved(
      TLorentzVector &HVec, TLorentzVector &bbjVec, TLorentzVector &metVec,
      const std::vector<const xAOD::Jet *> &selectedJets, Jet &j1_sel,
      Jet &j2_sel, const std::vector<const xAOD::Jet *> &fatJets,
      const std::vector<const xAOD::Jet *> &trackJets);

  EL::StatusCode fillETreeCommon();

  EL::StatusCode fillETreeMerged(
      TLorentzVector &metVec, TLorentzVector &mptVec,
      const std::vector<const xAOD::Jet *> &trackJetsInLeadFJ);

  EL::StatusCode fillETreeWeights(float triggerSF, float JVTweight,
                                  float btagWeight);

  EL::StatusCode fill0LepHistResolved();
  EL::StatusCode fill0LepHistMerged();

  bool m_doMergeModel;
  bool m_mbbwindow;
  bool m_met_JETcorrection;
  bool m_met_MZcorrection;
  bool m_doCutflow;
  std::string m_modelType;

  EL::StatusCode getTruthInfo_0Lep(int nTag,
                                   std::vector<const xAOD::Jet *> jets);
  EL::StatusCode fill0LepHistWithModulableCuts(
      const unsigned long int eventFlag);

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHQQ0Lep, 1);
};

// A namespace to be used for keeping track of the cut decisions
namespace ZeroLeptonCuts {

enum Cuts {

  AllCxAOD = 0,
  Trigger,
  TauVeto,
  AtLeast2Jets,
  AtLeast2SigJets,
  AtLeast1FatJet,
  AtLeast1MatchedTrackJet,
  AtLeast2MatchedTrackJets,
  passVRJetOR,
  Exactly2FatJetBTags,
  METResolved,
  METMerged,
  mJ,
  DPhiMETMPT,
  MinDPhiMETJetsResolved,
  MinDPhiMETJetsMerged,
  SumJetPt,
  MinMPTBJet0or1Resolved,
  MinMPTMerged,
  ThirdBJetVeto,
  LeadJetPt,
  DPhiBB,
  DPhiMETDijetResolved,
  DPhiMETDijetMerged,
  DRB1B2,  // SM cut only
  METLT500,
  METGT500,
  MHCorrResolved,
  MHCorrMerged,
  MHCorrResolvedSideBand,
  MHCorrMergedSideBand,
  NonOverlappingMatchedTrackJets,
  METSignificanceResolved,  // mVH will be calculated from resolved VH candidate
  METSignificanceMerged     // mVH will be calculated from merged VH candidate

};
}

#endif  // ifndef CxAODReader_AnalysisReader_VHQQ0Lep_H
