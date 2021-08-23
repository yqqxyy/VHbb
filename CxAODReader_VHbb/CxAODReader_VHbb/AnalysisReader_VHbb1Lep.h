#ifndef CxAODReader_AnalysisReader_VHbb1Lep_H
#define CxAODReader_AnalysisReader_VHbb1Lep_H

#include "../CorrsAndSysts/CorrsAndSysts/BDTSyst.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ1Lep.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

class AnalysisReader_VHbb1Lep : public AnalysisReader_VHQQ1Lep {
 public:
  AnalysisReader_VHbb1Lep();
  ~AnalysisReader_VHbb1Lep();

  EL::StatusCode run_1Lep_analysis() override;
  EL::StatusCode initializeSelection() override;
  EL::StatusCode fill_VBF() override;
  ClassDefOverride(AnalysisReader_VHbb1Lep, 1);

  void DefineVarBDTReweight_1lep(
      std::vector<const xAOD::Jet *> selectedJets, TLorentzVector &j1_corr,
      TLorentzVector &j2_corr, TLorentzVector &j3_corr,
      const TLorentzVector &leptonVec, const TLorentzVector &WvecT,
      const TLorentzVector &metVec, float Mtop, float dYWH);


 protected:
  virtual EL::StatusCode finalize() override;
  long   m_eventCountPassed_VBFHbb; // !
  EL::StatusCode fill_VBFCutFlow (std::string label);
  EL::StatusCode save_meta_info (std::vector<const xAOD::Jet*>  const& Jets,
				 std::vector<const xAOD::Jet*>  const& signalJets); // NEW METHOD                  
  EL::StatusCode save_trigger_info (); // NEW METHOD                  
  EL::StatusCode save_jet_info (
				std::vector<const xAOD::Jet*>  const& inputJets,
				std::vector<const xAOD::Jet*>  const& signalJets,
				std::vector<const xAOD::Jet*>  const& vbfJets,
				std::vector<const xAOD::Jet*>  const& trackJets); // NEW METHOD   



 private:
  bool m_ApplyFatJetMuOR;
  bool m_BDTSyst_debug;
  bool m_applyPUWeight;
  bool m_doBJetEnergyCorr;
  bool m_doBlindingData;
  bool m_doBlindingMC;
  bool m_doInputPlots;
  bool m_doIsoInv;
  bool m_doMJEtree;
  bool m_doMbbRejHist;
  bool m_doMbbRescaling;
  bool m_doMergeCR;
  bool m_doRemoveDiLepOverlap;
  bool m_doSplitLepCharge;
  bool m_doSplitLepFlavour;
  bool m_doSplitWhfCR;
  bool m_doTLVPlots;
  bool m_mVHvsPtvCut;
  bool m_nominalOnly;
  bool m_printEventWeightsForCutFlow;
  bool m_reduceHistograms;
  bool m_useTTBarPTWFiltered1LepSamples;
  bool m_writeEasyTree;
  bool m_writeMVATree;
  bool m_writeOSTree;
  bool m_doCoMtagging;
  bool m_forceTrackJetDRmatching;
  bool m_writeObjectsInEasyTree;
  bool m_doDYWFJCut;
  double m_dataOneOverSF;
  double m_mbbMergedHighEdge;
  double m_mbbMergedLowEdge;
  double m_mbbMergedLowerBound;
  double m_mbbMergedUpperBound;
  double m_mbbResolvedHighEdge;
  double m_mbbResolvedLowEdge;
  double m_mbbRestrictHighEdge;
  double m_mbbRestrictLowEdge;
  double m_minElMET;
  double m_minMuMET;

  std::string m_truthLabeling;
  std::string m_RecoToTruthLinkName;
};

#endif  // ifndef CxAODReader_AnalysisReader_VHbb1Lep_H
