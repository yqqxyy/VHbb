#ifndef CxAODTools_VHbb_TriggerTool_VHbb1lep_H
#define CxAODTools_VHbb_TriggerTool_VHbb1lep_H

#include "CxAODTools_VHbb/TriggerTool_VHbb.h"

class TriggerTool_VHbb1lep : public TriggerTool_VHbb {

public:
  TriggerTool_VHbb1lep(ConfigStore& config);
  virtual ~TriggerTool_VHbb1lep() = default;

  virtual EL::StatusCode initTools() override;
  virtual EL::StatusCode initProperties() override;
  virtual EL::StatusCode initTriggers() override;

protected:
  // isolation inversion MJ estimate has its own trigger menu
  void addIsoInvElectron();

};

#endif // ifndef CxAODTools_VHbb_TriggerTool_VHbb1lep_H
