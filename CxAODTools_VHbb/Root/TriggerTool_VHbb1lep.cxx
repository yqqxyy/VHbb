#include "CxAODTools_VHbb/TriggerTool_VHbb1lep.h"
#include "CxAODTools/CommonProperties.h"


TriggerTool_VHbb1lep::TriggerTool_VHbb1lep(ConfigStore& config) :
  TriggerTool_VHbb(config){
}


EL::StatusCode TriggerTool_VHbb1lep::initTools() {
  EL_CHECK("TriggerTool_VHbb1lep::initTools()", initMuonSFTool("Medium"));
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TriggerTool_VHbb1lep::initProperties() {
  m_electronEffProp = &Props::trigEFFtightLHIsoFixedCutTight;
  m_electronSFProp  = &Props::trigSFtightLHIsoFixedCutTight;
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TriggerTool_VHbb1lep::initTriggers() {
  bool doIsoInv = false;
  m_config.getif<bool>("doIsoInv", doIsoInv);
  std::string model = m_config.get<std::string>("modelType");
  if (doIsoInv && model == "HVT") {
    // isolation inversion MJ estimate has its own trigger menu
    addIsoInvElectron();
  } else {
    addLowestUnprescaledElectron();
  }
  addLowestUnprescaledMuon();
  addLowestUnprescaledMET();
  return EL::StatusCode::SUCCESS;
}

void TriggerTool_VHbb1lep::addIsoInvElectron() {
    ADD_TRIG_MATCH(HLT_e24_lhmedium_L1EM18VH, MC,   data15, data15);
    ADD_TRIG_MATCH(HLT_e24_lhmedium_L1EM20VH, data, data15, data15);
    ADD_TRIG_MATCH(HLT_e24_lhtight_nod0_ivarloose, any,  data16A, data16BD3);
    ADD_TRIG_MATCH(HLT_e26_lhtight_nod0_ivarloose, any,  data16D4E3, data16F2L11);
}
