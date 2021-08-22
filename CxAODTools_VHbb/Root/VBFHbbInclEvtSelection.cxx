#include "CxAODTools_VHbb/VBFHbbInclEvtSelection.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"
#include <TLorentzVector.h>

void VBFHbbInclEvtSelection::clearResult(){
  m_result.pass = false;
  m_result.jets.clear();
  m_result.forwardJets.clear();
  m_result.jets_etaSort.clear();
}

bool VBFHbbInclEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code  
  return VBFHbbEvtSelection<ResultVBFHbbIncl>::passSelection(containers, isKinVar);
}

bool VBFHbbInclEvtSelection::passJetSelection(const xAOD::JetContainer* jets) {  
  m_result.jets.clear();
  m_result.jets_etaSort.clear();
  
  int njets(0);
  for (unsigned int iJet(0) ; iJet < jets->size(); ++iJet) {
    const xAOD::Jet * jet = jets->at(iJet);
    if (!Props::passOR.get(jet)) continue; 
    if (!(jet->pt() > 20e3)) continue; 
    // all the jets above 20 GeV is used for jet veto in the following
    njets++;
    m_result.jets.push_back(jet);
  }

  // Jets appear to be pt ordered but just in case
  std::sort(m_result.jets.begin(), m_result.jets.end(), sort_pt);
  const std::vector<const xAOD::Jet*>& selected_jets = m_result.jets;
  return true;
}


bool VBFHbbInclEvtSelection::passTriggerSelection(const xAOD::EventInfo* evtinfo) { return true; }
bool VBFHbbInclEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VBFHbbEvtSelection<ResultVBFHbbIncl>::passPreSelection(containers, isKinVar);
}

EL::StatusCode VBFHbbInclEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                           xAOD::EventInfo* /*eventInfoOut*/,
                                                           bool /*isKinVar*/,
                                                           bool /*isWeightVar*/,
                                                           std::string /*sysName*/,
                                                           int /*rdm_RunNumber*/,
                                                           CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {
  return EL::StatusCode::SUCCESS;
}

bool VBFHbbInclEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT
  return true;
}
