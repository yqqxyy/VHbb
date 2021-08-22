#ifndef CxAODTools_VHbb_TriggerTool_VHbb_H
#define CxAODTools_VHbb_TriggerTool_VHbb_H

#include "CxAODTools/TriggerTool.h"

class TriggerTool_VHbb : public TriggerTool {

public:
  
  TriggerTool_VHbb(ConfigStore& config);
  virtual ~TriggerTool_VHbb() = default;

  bool getDecisionAndSFwithMET(double& triggerSF);

  //Needed to add MET Trigger to 1L and 2L Analysis
  bool m_doMuonTriggerin1L;
  bool m_doMETTriggerin2L;
  bool m_doMETMuonTrigger; 
  std::string m_analysisType;

private:

  double m_pTWcutVal;
  double m_pTZcutVal;  

};

#endif // ifndef CxAODTools_VHbb_TriggerTool_VHbb_H
