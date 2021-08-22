#ifndef CxAODTools_VHbb_TriggerTool_VHbb0lep_H
#define CxAODTools_VHbb_TriggerTool_VHbb0lep_H

#include "CxAODTools_VHbb/TriggerTool_VHbb.h"

class TriggerTool_VHbb0lep : public TriggerTool_VHbb {

public:

  TriggerTool_VHbb0lep(ConfigStore& config);
  virtual ~TriggerTool_VHbb0lep() = default;
  
  virtual EL::StatusCode initTriggers() override;

};

#endif // ifndef CxAODTools_VHbb_TriggerTool_VHbb0lep_H
