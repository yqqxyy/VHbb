#ifndef CxAODReader_AnalysisReader_VHreso2Lep_H
#define CxAODReader_AnalysisReader_VHreso2Lep_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ2Lep.h"

class AnalysisReader_VHreso2Lep : public AnalysisReader_VHQQ2Lep {
 public:
  AnalysisReader_VHreso2Lep();
  ~AnalysisReader_VHreso2Lep();

  EL::StatusCode run_2Lep_analysis() override;

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHreso2Lep, 1);
};
#endif  // ifndef CxAODReader_AnalysisReader_VHreso2Lep_H
