#include "CxAODTools/DummyEvtSelection.h"

#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"

DummyEvtSelection::DummyEvtSelection() noexcept
{}

void DummyEvtSelection::clearResult() {
  m_result.pass = false;
  m_result.jets.clear();
  m_result.muon = nullptr;
}

bool DummyEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {
  // passSelection should use tighter cuts than passPreSection.
  // One can just call passPreSelection to ensure this.
  return passPreSelection(containers, isKinVar);
}

bool DummyEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {

  clearResult();
  bool passSel = true;
  if(!isKinVar && passSel) m_cutFlow.count("Preselection initial", 100);

  // select jets
  for (const xAOD::Jet* jet : *containers.jets) {
    if (!Props::passOR.get(jet)) continue;
    m_result.jets.push_back(jet);
  }
  // cannot rely on jets being pt sorted since all variations have same order in CxAOD
  std::sort(m_result.jets.begin(), m_result.jets.end(), sort_pt);

  // require at least one jet
  passSel &= (m_result.jets.size() >= 1);
  if(!isKinVar && passSel) m_cutFlow.count("At least one jet");

  // select muon
  for (const xAOD::Muon* muon : *containers.muons) {
    if (!Props::passOR.get(muon)) continue;
    m_result.muon = muon;
    break;
  }

  // require to have a muon in the event
  passSel &= (m_result.muon != nullptr);
  if(!isKinVar && passSel) m_cutFlow.count("Found a muon");
  
  // set met
  m_result.met = containers.met;

  return passSel;
}


EL::StatusCode DummyEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                      xAOD::EventInfo* eventInfoOut,
                                                      bool /*isKinVar*/,
                                                      bool /*isWeightVar*/,
                                                      std::string /*sysName*/,
                                                      int /*rdm_RunNumber*/,
                                                      CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  // ensure to write all variables for all events
  Props::leptonSF.set(eventInfoOut, 1);
  
  if (!m_result.pass) return EL::StatusCode::SUCCESS;
  
  // write lepton SF
  float leptonSF = 1;
  leptonSF *= Props::effSF.get(m_result.muon);
  Props::leptonSF.set(eventInfoOut, leptonSF);

  return EL::StatusCode::SUCCESS;
}

void DummyEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                            xAOD::EventInfo* /*eventInfoOut*/,
                                            bool /*isKinVar*/,
                                            bool /*isWeightVar*/) { 
  return;
}
