#include "CxAODTools_VHbb/VHbb0lepEvtSelection.h"

#include "xAODEgamma/Electron.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"

#include<iostream>

VHbb0lepEvtSelection::VHbb0lepEvtSelection(ConfigStore* config) :
   VHbbEvtSelection(config)
{}

void VHbb0lepEvtSelection::clearResult() {
  m_result.pass = false;
  m_result.signalJets.clear();
  m_result.forwardJets.clear();
  m_result.fatJets.clear();
  m_result.trackJets.clear();
  m_result.subJets.clear();
  m_result.taus.clear();
  m_result.met = nullptr;
  m_result.truthElectrons.clear();
  m_result.truthMuons.clear();
  m_result.truthTaus.clear();
  m_result.truthSignalJets.clear();
  m_result.truthForwardJets.clear();
  m_result.truthFatJets.clear();
}

bool VHbb0lepEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VHbbEvtSelection<ResultVHbb0lep>::passSelection(containers, isKinVar);
}

bool VHbb0lepEvtSelection::passLeptonSelection(const xAOD::ElectronContainer* electrons,
                                               const xAOD::MuonContainer* muons,
                                               const xAOD::MissingET* met) {
  const xAOD::Electron *el1=nullptr, *el2=nullptr;
  const xAOD::Muon *mu1=nullptr, *mu2=nullptr;
  int res = doVHLeptonSelection(electrons, muons, el1, el2, mu1, mu2);
  if(res != 0) {
    return false;
  }
  m_result.met = met;
  return true;

}

// preselection versions
bool VHbb0lepEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  bool passpreselection = VHbbEvtSelection<ResultVHbb0lep>::passPreSelection(containers, isKinVar);
  const xAOD::MissingET * met = containers.met;
  if ( met ) {
    m_result.met = met;
    if(met->met() < 140000) {
      passpreselection = false;
    }

    //cut on nJet < 5 for SM (this flag has to be set both in the Reader and Maker config file)
    bool doNJetCut0L1L = false;
    m_config->getif<bool>("doNJetCut0L1L", doNJetCut0L1L);
    if(doNJetCut0L1L){
      int n_signalJ = m_result.signalJets.size();
      int n_forwardJ = m_result.forwardJets.size();
      int n_fatJet = m_result.fatJets.size();
    
      if((n_signalJ+n_forwardJ >= 5) && n_fatJet < 1){
	m_result.pass = false; 
	return false;
      }
    }

    if(passpreselection&&!isKinVar) {
      m_cutFlow.count("nJ res. < 5",200);

      m_cutFlow.count("MET > 100 GeV",201);
      double MetValue = m_result.met->met();
      if( MetValue/1000 <= 90){
        m_cutFlow.count("MET < 90GeV",202);
      }
      if(MetValue/1000 > 90 && MetValue/1000 <= 120){
        m_cutFlow.count("90GeV < MET < 120GeV",203);
      }
      if(MetValue/1000 > 120 && MetValue/1000 <= 160){
        m_cutFlow.count("120GeV < MET < 160GeV",204);
      }
      if(MetValue/1000 > 160 && MetValue/1000 <= 200){
        m_cutFlow.count("160GeV < MET < 200GeV",205);
      }
      if( MetValue/1000 > 200){
        m_cutFlow.count("MET > 200GeV",206);
      }
    }
  }
  return passpreselection;
}

bool VHbb0lepEvtSelection::passLeptonPreSelection(const xAOD::ElectronContainer* electrons,
                                                  const xAOD::MuonContainer* muons,
                                                  const xAOD::MissingET* /*met*/) {
  const xAOD::Electron *el1=nullptr, *el2=nullptr;
  const xAOD::Muon *mu1=nullptr, *mu2=nullptr;
  int res = doVHLeptonPreSelection(electrons, muons, el1, el2, mu1, mu2);
  if(res != 0) {
    return false;
  }
  return true;

}

EL::StatusCode VHbb0lepEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                         xAOD::EventInfo* /*eventInfoOut*/,
                                                         bool /*isKinVar*/,
                                                         bool /*isWeightVar*/,
                                                         std::string /*sysName*/,
                                                         int /*rdm_RunNumber*/,
                                                         CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  return EL::StatusCode::SUCCESS;

}

bool VHbb0lepEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT
  return true;
}
