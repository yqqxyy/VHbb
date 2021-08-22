#ifndef CxAODReader_AnalysisReader_VHQQ2Lep_H
#define CxAODReader_AnalysisReader_VHQQ2Lep_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
#include "TLorentzRotation.h"

struct ResultVHbb2lep;

class AnalysisReader_VHQQ2Lep : public AnalysisReader_VHQQ {
 public:
  AnalysisReader_VHQQ2Lep();
  ~AnalysisReader_VHQQ2Lep();

  virtual EL::StatusCode initializeSelection() override;
  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode run_2Lep_analysis() = 0;
  virtual EL::StatusCode finalizeTools() override;

 protected:
  void setLeptonVariables(Lepton &l1, Lepton &l2, const xAOD::Electron *el1,
                          const xAOD::Electron *el2, const xAOD::Muon *mu1,
                          const xAOD::Muon *mu2);

  void setTwoLeptonFlavour(const Lepton &l1, const Lepton &l2);

  bool pass2LepTrigger(double &triggerSF_nominal,
                       ResultVHbb2lep selectionResult);
  void checkMllCut(const TLorentzVector &ZVec,
                   const TLorentzVector &VHVec_resolved,
                   const TLorentzVector &VHVec_merged, bool &passMllResolved,
                   bool &passMllMerged);
  void checkMbbWindow(const TLorentzVector &Hvec_resolved,
                      const TLorentzVector &Hvec_merged, bool &passMHResolved,
                      bool &passMHMerged, bool &inMHSideBandResolved,
                      bool &inMHSideBandMerged);

  void checkPTVCut(const TLorentzVector &ZVec,
                   const TLorentzVector &VHVec_resolved,
                   const TLorentzVector &VHVec_merged, bool &passPtvResolved,
                   bool &passPtvMerged);

  float lowMllCut(const float mVH);  // mVH in MeV; returns cut value in GeV
  float upMllCut(const float mVH);   // mVH in MeV; returns cut value in GeV
  float METHTcut(const float mVH);   // mVH in MeV; returns cut value in GeV
  float pTVCut(const float mVH);     // mVH in MeV; returns cut value in GeV

  void selectRegime(unsigned long int eventFlag, double ZpTV,
                    std::vector<unsigned long int> cuts_SR_resolved,
                    std::vector<unsigned long int> cuts_SR_merged,
                    std::vector<unsigned long int> cuts_mBBcr_resolved,
                    std::vector<unsigned long int> cuts_mBBcr_merged,
                    std::vector<unsigned long int> cuts_top_resolved,
                    std::vector<unsigned long int> cuts_top_merged);

  EL::StatusCode getKFResolved(const int nSelJets, Jet &j1, Jet &j2, Jet &j3,
                               Lepton &l1, Lepton &l2);
  EL::StatusCode getKFMerged(const int njets, Jet &fj1, Lepton &l1, Lepton &l2,
                             std::vector<const xAOD::Jet *> &signalJets,
                             std::vector<const xAOD::Jet *> &forwardJets);

  void setVHCandidate(VHCand &resolvedVH, VHCand &mergedVH, Lepton l1,
                      Lepton l2, Jet j1, Jet j2,
                      Jet fj1);  // use higgs struct...

  virtual EL::StatusCode fill_2lepCutFlow(unsigned long int eventFlag,
                                          const int nJets, const int nTags,
                                          const int nFatJetTags,
                                          bool isMu = false, bool isE = false);
  // ETree filling
  EL::StatusCode fillETreeCommon(float triggerSF, float JVTweight,
                                 float btagWeight);
  EL::StatusCode fillETreeSelectedJets(
      const std::vector<const xAOD::Jet *> &selectedJets);
  EL::StatusCode fillETreeLepton(Lepton l1, Lepton l2);
  EL::StatusCode fillETreeResolved(Jet j1, Jet j2, double HTsmallR,
                                   double METHTsmallR);
  EL::StatusCode fillETreeMerged(VHCand &mergedVH, Higgs &mergedH, int nTrkJets,
                                 int nTaggedTrkJets, int nTrkjetsInFJ,
                                 double HTlargeR, double METHTlargeR);
  EL::StatusCode fillETreeMbbMonitor(const Higgs &h1, const Lepton &l1,
                                     const Lepton &l2, const Jet &j1,
                                     const Jet &j2, const Jet &j3);

  // Histograms filling
  EL::StatusCode fill_2LepHistos(bool isMu = false, bool isE = false);
  EL::StatusCode fill_MVAVariablesHistos2Lep(bool isMu = false,
                                             bool isE = false);
  EL::StatusCode fill_EasyTreeHistos2Lep(bool isMu = false, bool isE = false);
  EL::StatusCode fill_EasyTreeHistos2LepMbbMonitor(const string &eventType);
  EL::StatusCode fill_jetHistos2Lep(std::vector<const xAOD::Jet *> signalJets,
                                    std::vector<const xAOD::Jet *> forwardJets);

  EL::StatusCode fillMVATreeVHbbResolved2Lep(
      TLorentzVector &b1, TLorentzVector &b2, TLorentzVector &j3,
      const std::vector<const xAOD::Jet *> &selectedJets,
      const std::vector<const xAOD::Jet *> &signalJets,
      const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &HVec,
      TLorentzVector &ZVec, TLorentzVector &metVec, const xAOD::MissingET *&met,
      const Lepton &lep1, const Lepton &lep2, int nTaus = 0,
      float bTagWeight = 1);

  void printMVA();

  std::vector<TString> mGetExpressions(TString xmlFile);
  void computePolarizationVariable(const Lepton &lep1, const Lepton &lep2,
                                   float *cosThetaLep);
  TLorentzRotation getTrafoToHelicityFrame(const TLorentzVector &neglep,
                                           const TLorentzVector &poslep);
  std::string components(TLorentzVector v);

  bool m_doMergeModel;
  std::string m_modelType;
  bool m_isSLEvent;

  float m_KFPrefitLHD;
  float m_KFPostfitLHD;

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHQQ2Lep, 1);
};

// A namespace to be used for keeping track of the cut decisions
namespace DiLeptonCuts {

enum Cuts {
  AllCxAOD = 0,
  Trigger,
  LeadLepPt27,
  SFLeptons,
  OSLeptons,
  pTBothLeptons25,
  tauVeto,
  MllZwindowResolved,
  METHTResolved,
  AtLeast2Jets,
  AtLeast2SigJets,
  AtLeast1B,
  Exactly2B,
  // Veto3B,
  // LooseMll,
  PtB145,
  mHCorrResolved,
  mHNoCorrResolved,
  mHCorrInsideSideBandResolved,
  PtZLT500,
  PtZGT500,
  PtZResolved,
  Exactly2Jets,
  Exactly3Jets,
  Exactly4Jets,
  AtLeast5Jets,
  MuonEtaLT2p5,
  MllZwindowMerged,
  METHTMerged,
  AtLeast1FJ,
  passVRJetOR,
  mHCorrMerged,
  mHNoCorrMerged,
  mHCorrInsideSideBandMerged,
  PtZMerged,
  LeadFJ2TJ,
  LeadFJ1pTJ,
  LeadFJ2b,
  MLeadFJ50,
  MllAbove40,
  dRCut,
  DFLeptons,  // the inverse of SFLeptons
  DLepPt,     // Z-polarization cut
  DYVH,
  Veto3pJets  // b-tag veto in 3+ jet in VHcc 2-lepton
};
}

#endif  // ifndef CxAODReader_AnalysisReader_VHQQ2Lep_H
