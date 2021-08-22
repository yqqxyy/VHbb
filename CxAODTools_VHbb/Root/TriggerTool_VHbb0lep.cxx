#include "CxAODTools_VHbb/TriggerTool_VHbb0lep.h"

#include "CxAODTools/CommonProperties.h"

TriggerTool_VHbb0lep::TriggerTool_VHbb0lep(ConfigStore& config) :
    TriggerTool_VHbb(config) {
}

EL::StatusCode TriggerTool_VHbb0lep::initTriggers() {
  addLowestUnprescaledMET();
  return EL::StatusCode::SUCCESS;
}
