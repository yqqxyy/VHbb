#ifndef CxAODReader_AnalysisReader_VHcc1Lep_H
#define CxAODReader_AnalysisReader_VHcc1Lep_H

#include "../CorrsAndSysts/CorrsAndSysts/BDTSyst.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ1Lep.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

class AnalysisReader_VHcc1Lep : public AnalysisReader_VHQQ1Lep {
 public:
  AnalysisReader_VHcc1Lep();
  ~AnalysisReader_VHcc1Lep();

  std::vector<std::pair<TString, Float_t>> ApplyBDTSyst(
      int channel, std::vector<const xAOD::Jet *> selectedJets,
      TLorentzVector WVecT, TLorentzVector lepVec, float Mtop,
      TLorentzVector bbjVecCorr, TLorentzVector j3VecCorr, float dYWH,
      float btagWeight, int nJet);

  EL::StatusCode run_1Lep_analysis() override;
  EL::StatusCode initializeSelection() override;
  EL::StatusCode fill_VBF() override;
  ClassDefOverride(AnalysisReader_VHcc1Lep, 1);

 private:
  bool m_ApplyFatJetMuOR;
  bool m_BDTSyst_debug;
  bool m_GenerateSTXS;
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
  bool m_writeObjectsInEasyTree;
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
  std::vector<std::string> m_BDTSystVariations;
};

#endif  // ifndef CxAODReader_AnalysisReader_VHcc1Lep_H
