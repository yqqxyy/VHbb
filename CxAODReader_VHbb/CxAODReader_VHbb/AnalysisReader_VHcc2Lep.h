#ifndef CxAODReader_AnalysisReader_VHcc2Lep_H
#define CxAODReader_AnalysisReader_VHcc2Lep_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ2Lep.h"

class AnalysisReader_VHcc2Lep : public AnalysisReader_VHQQ2Lep {
 public:
  AnalysisReader_VHcc2Lep();
  ~AnalysisReader_VHcc2Lep();

  EL::StatusCode run_2Lep_analysis() override;

  EL::StatusCode fill_2lepCutFlow(unsigned long int eventFlag, const int nJets,
                                  const int nTags, const int nFatJetTags,
                                  bool isMu = false, bool isE = false) override;
  // function to compute the deltaR correction for TT based on the minimum
  // deltaR of the tagged jet and any other jet in the event
  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHcc2Lep, 1);
};
#endif  // ifndef CxAODReader_AnalysisReader_VHcc2Lep_H
