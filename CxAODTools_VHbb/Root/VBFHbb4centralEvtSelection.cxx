#include "CxAODTools_VHbb/VBFHbb4centralEvtSelection.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"
#include <TLorentzVector.h>

// ===================================
// Constructor
VBFHbb4centralEvtSelection::VBFHbb4centralEvtSelection(bool applybtag, TString WP, bool isMC) 
  : m_applybtag(applybtag),
    m_Period("preICHEP"),
    m_WP(WP),
    m_isMC(isMC) {}
VBFHbb4centralEvtSelection::VBFHbb4centralEvtSelection(const VBFHbb4centralEvtSelection& other)
  : m_applybtag(other.m_applybtag),
    m_Period(other.m_Period),
    m_WP(other.m_WP),
    m_isMC(other.m_isMC) {}
// Destructor
VBFHbb4centralEvtSelection::~VBFHbb4centralEvtSelection() noexcept {}
// ===================================

void VBFHbb4centralEvtSelection::clearResult(){
  m_result.pass = false;
  m_result.jets.clear();
  m_result.forwardJets.clear();
  m_result.jets_etaSort.clear();
}

bool VBFHbb4centralEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code  
  return VBFHbbEvtSelection<ResultVBFHbb4central>::passSelection(containers, isKinVar);
}

bool VBFHbb4centralEvtSelection::passJetSelection(const xAOD::JetContainer* jets){
  m_result.jets.clear();
  m_result.jets_etaSort.clear();

  double btagweight = 0;
  if (m_WP == "77"){
    btagweight = m_b_77_weight;
  }
  if (m_WP == "70"){
    btagweight = m_b_70_weight;
  }

  for (unsigned int iJet(0) ; iJet < jets->size(); ++iJet) {
    const xAOD::Jet * jet = jets->at(iJet);
    if (!Props::passOR.get(jet)) continue; 
    if (!(jet->pt() > 20e3)) continue; 
    m_result.jets.push_back(jet);
  }

  // Jets appear to be pt ordered but just in case
  std::sort(m_result.jets.begin(), m_result.jets.end(), sort_pt);
  
  const std::vector<const xAOD::Jet*>& selected_jets = m_result.jets;  
  //  Info("passJetSelection ()", "jet selection was passed");
  return true;
}


bool VBFHbb4centralEvtSelection::passTriggerSelection(const xAOD::EventInfo* evtinfo){
  m_cutFlow.count("AllEvents");

  bool passesL1  = true;//Props::pass_L1_HT150_J20s5_ETA31_MJJ_400_CF.get( evtinfo ) > 0;
  bool passesHLT = true;//Props::pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split.get( evtinfo ) > 0; 

  if (passesL1) Info("passJetSelection ()", "pass L1");
  else return false;
  m_cutFlow.count("L1_HT150-J20s5.ETA31_MJJ-400-CF");
  
  if (passesHLT) Info("passJetSelection ()", "pass HLT");
  else return false;
  m_cutFlow.count("HLT_ht300_2j40_0eta490_invm700_L1HT150-J20s5.ETA31_MJJ-400-CF_AND_2j25_gsc45_bmv2c1070_split");

  return true;
}

bool VBFHbb4centralEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {
  
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VBFHbbEvtSelection<ResultVBFHbb4central>::passPreSelection(containers, isKinVar);
}

EL::StatusCode VBFHbb4centralEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                           xAOD::EventInfo* /*eventInfoOut*/,
                                                           bool /*isKinVar*/,
                                                           bool /*isWeightVar*/,
                                                           std::string /*sysName*/,
                                                           int /*rdm_RunNumber*/,
                                                           CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  return EL::StatusCode::SUCCESS;

}

bool VBFHbb4centralEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT
  return true;
}
