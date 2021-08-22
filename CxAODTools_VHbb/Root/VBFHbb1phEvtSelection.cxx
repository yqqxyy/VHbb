#include "CxAODTools_VHbb/VBFHbb1phEvtSelection.h"

#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"


void VBFHbb1phEvtSelection::clearResult(){
  m_result.pass = false;
  m_result.jets.clear();
  m_result.ph = nullptr;
}

bool VBFHbb1phEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code          
  return VBFHbbEvtSelection<ResultVBFHbb1ph>::passSelection(containers, isKinVar);
}

bool VBFHbb1phEvtSelection::passPhotonSelection(const xAOD::PhotonContainer* photons){
  
  int res = doVBFPhotonSelection(photons, m_result.ph);
  if(res < 1)
    return false;
  
  return true;

}

bool VBFHbb1phEvtSelection::passJetSelection(const xAOD::JetContainer* jets){
  return VBFHbb1phEvtSelection::passJetPreSelection(jets);
}
/*
bool VBFHbb1phEvtSelection::passTriggerSelection(const xAOD::EventInfo* evtinfo){
  return true;
}
*/

bool VBFHbb1phEvtSelection::passTriggerSelection (const xAOD::EventInfo* evtinfo){
  return true;
}

bool VBFHbb1phEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code          
  return VBFHbbEvtSelection<ResultVBFHbb1ph>::passPreSelection(containers, isKinVar);
}


/*
bool VBFHbb1phEvtSelection::passPhotonPreSelection(const xAOD::PhotonContainer* photons){
  
  int res = doVBFPhotonPreSelection(photons, m_result.ph);
  if(res != 1)
    return false;
  
  return true;

}
*/

EL::StatusCode VBFHbb1phEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                          xAOD::EventInfo* /*eventInfoOut*/,
                                                          bool /*isKinVar*/,
                                                          bool /*isWeightVar*/,
                                                          std::string /*sysName*/,
                                                          int /*rdm_RunNumber*/,
                                                          CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  return EL::StatusCode::SUCCESS;

}

bool VBFHbb1phEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that          
  // doesn't need to be put in the Result struct, like MPT
  return true;
}
