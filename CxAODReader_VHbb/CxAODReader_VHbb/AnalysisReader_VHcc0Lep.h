#ifndef CxAODReader_AnalysisReader_VHcc0Lep_H
#define CxAODReader_AnalysisReader_VHcc0Lep_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ0Lep.h"

struct ResultVHbb0lep;

class AnalysisReader_VHcc0Lep : public AnalysisReader_VHQQ0Lep {
 public:
  AnalysisReader_VHcc0Lep() = default;
  ~AnalysisReader_VHcc0Lep() = default;

  EL::StatusCode run_0Lep_analysis() override;

  EL::StatusCode fill0LepCutFlow(unsigned long int eventFlag,
                                 std::vector<unsigned long int> cuts,
                                 std::string cutsNames[], int length_cutsNames,
                                 int bin_ntag_njet);

  EL::StatusCode fillETree();

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHcc0Lep, 1);
};

#endif  // ifndef CxAODReader_AnalysisReader_VHcc0Lep_H
