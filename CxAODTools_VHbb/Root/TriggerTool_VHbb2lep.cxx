#include "CxAODTools_VHbb/TriggerTool_VHbb2lep.h"
#include "CxAODTools/CommonProperties.h"

TriggerTool_VHbb2lep::TriggerTool_VHbb2lep(ConfigStore& config) :
   TriggerTool_VHbb(config){ 
}

EL::StatusCode TriggerTool_VHbb2lep::initTools() {
  EL_CHECK("TriggerTool_VHbb2lep::initTools()", initMuonSFTool("Loose"));
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TriggerTool_VHbb2lep::initProperties() {
  m_electronEffProp = &Props::trigEFFlooseLHIsoFixedCutLoose;
  m_electronSFProp  = &Props::trigSFlooseLHIsoFixedCutLoose;
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TriggerTool_VHbb2lep::initTriggers() {
  addLowestUnprescaledElectron();
  addLowestUnprescaledMuon();
  if (m_doMETTriggerin2L || m_doMETMuonTrigger)   addLowestUnprescaledMET();
  return EL::StatusCode::SUCCESS;
}

