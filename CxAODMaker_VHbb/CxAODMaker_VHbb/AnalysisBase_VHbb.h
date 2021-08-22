#ifndef CxAODMaker_AnalysisBase_VHbb_H
#define CxAODMaker_AnalysisBase_VHbb_H

#include "CxAODMaker/AnalysisBase.h"

class AnalysisBase_VHbb : public AnalysisBase {

protected:
  virtual EL::StatusCode initializeSampleInfo() override;
  virtual EL::StatusCode initializeHandlers() override;
  virtual EL::StatusCode initializeSelection() override;
  
public: 
  AnalysisBase_VHbb()
    : AnalysisBase() {}

  ~AnalysisBase_VHbb() {}

  ClassDefOverride(AnalysisBase_VHbb, 1);
};

#endif
