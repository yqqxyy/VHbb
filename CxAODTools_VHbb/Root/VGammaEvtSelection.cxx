#include "CxAODTools_VHbb/VGammaEvtSelection.h"
#include "CxAODTools/ReturnCheck.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODTau/TauJet.h"
#include "xAODEventInfo/EventInfo.h"

#include "TVector2.h"
#include <iostream>

void VGammaEvtSelection::clearResult() {
  m_result.pass = false;
  m_result.fatJets.clear();
  m_result.jets4.clear();
  m_result.ph.clear();
}

bool VGammaEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VGammaFatJEvtSelection<ResultVGamma>::passSelection(containers, isKinVar);
}

bool VGammaEvtSelection::passPhotonSelection(const xAOD::PhotonContainer* photons){
  
  return doVGammaFatJPhotonSelection(photons);
}

bool VGammaEvtSelection::passFatJetSelection(const xAOD::JetContainer* fatjets){
  return VGammaFatJEvtSelection::passFatJetPreSelection(fatjets);
}

//
bool VGammaEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code

  bool passpreselection = VGammaFatJEvtSelection<ResultVGamma>::passPreSelection(containers, isKinVar);

  return passpreselection;
}

EL::StatusCode VGammaEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                       xAOD::EventInfo* /*eventInfoOut*/,
                                                       bool /*isKinVar*/,
                                                       bool /*isWeightVar*/,
                                                       std::string /*sysName*/,
                                                       int /*rdm_RunNumber*/,
                                                       CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  return EL::StatusCode::SUCCESS;

}

bool VGammaEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT
  return true;
}
