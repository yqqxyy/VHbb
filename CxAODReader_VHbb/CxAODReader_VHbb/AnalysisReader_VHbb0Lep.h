#ifndef CxAODReader_AnalysisReader_VHbb0Lep_H
#define CxAODReader_AnalysisReader_VHbb0Lep_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ0Lep.h"

struct ResultVHbb0lep;

class AnalysisReader_VHbb0Lep : public AnalysisReader_VHQQ0Lep {
 public:
  AnalysisReader_VHbb0Lep();
  ~AnalysisReader_VHbb0Lep();

  EL::StatusCode run_0Lep_analysis() override;
  void DefineVarBDTReweight_0lep(
      TLorentzVector &j1, TLorentzVector &j2, TLorentzVector &j3,
      std::vector<const xAOD::Jet *> &selectedJets,
      const std::vector<const xAOD::Jet *> &signalJets,
      const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &metVec,
      TLorentzVector &VVec, TLorentzVector &HVec);

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHbb0Lep, 1);
};

#endif  // ifndef CxAODReader_AnalysisReader_VHbb0Lep_H
