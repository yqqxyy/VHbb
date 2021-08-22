#ifndef CxAODTools_VHbb_TriggerTool_VHbb2lep_H
#define CxAODTools_VHbb_TriggerTool_VHbb2lep_H

#include "CxAODTools_VHbb/TriggerTool_VHbb.h"

class TriggerTool_VHbb2lep : public TriggerTool_VHbb {

public:

  TriggerTool_VHbb2lep(ConfigStore& config);
  virtual ~TriggerTool_VHbb2lep() = default;
  
  virtual EL::StatusCode initTools() override;
  virtual EL::StatusCode initProperties() override;
  virtual EL::StatusCode initTriggers() override;
  
};

#endif // ifndef CxAODTools_VHbb_TriggerTool_VHbb2lep_H
