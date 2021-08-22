#include "CxAODTools_VHbb/VBFHbb1centralEvtSelection.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"
#include <TLorentzVector.h>


// =================================== 
// Constructor
VBFHbb1centralEvtSelection::VBFHbb1centralEvtSelection(bool applybtag, TString WP, bool isMC)
  : m_applybtag(applybtag),
    m_isMC(isMC) {}
VBFHbb1centralEvtSelection::VBFHbb1centralEvtSelection(const VBFHbb1centralEvtSelection &other)
  : m_applybtag(other.m_applybtag),
    m_isMC(other.m_isMC) {}
// Destructor                                                                                                                                                                           
VBFHbb1centralEvtSelection::~VBFHbb1centralEvtSelection() noexcept {}
// =================================== 

void VBFHbb1centralEvtSelection::clearResult(){
  m_result.pass = false;
  m_result.jets.clear();
  m_result.forwardJets.clear();
  m_result.jets_etaSort.clear();
}

bool VBFHbb1centralEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {  
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VBFHbbEvtSelection<ResultVBFHbb1central>::passSelection(containers, isKinVar);
}

bool VBFHbb1centralEvtSelection::passJetSelection(const xAOD::JetContainer* jets) {
  //Info("passJetSelection ()", "in passJetSelection");
  m_result.jets.clear();
  m_result.jets_etaSort.clear();
  
  for (unsigned int iJet(0) ; iJet < jets->size(); ++iJet) {
    const xAOD::Jet * jet = jets->at(iJet);
    if (!Props::passOR.get(jet)) continue;
    if (!(jet->pt() > 20e3)) continue;
    //std::cout << "jet pt " << jet->pt()*0.001 << "\n";
    // all the jets above 20 GeV is used for jet veto in the following
    //    njets++;
    m_result.jets.push_back(jet);
  }

  // Jets appear to be pt ordered but just in case
  std::sort(m_result.jets.begin(), m_result.jets.end(), sort_pt);

  const std::vector<const xAOD::Jet*>& selected_jets = m_result.jets;
  //Info("passJetSelection ()", "jet selection was passed");
  return true;
}


bool VBFHbb1centralEvtSelection::passTriggerSelection(const xAOD::EventInfo* evtinfo){
  m_cutFlow.count("AllEvents");

  bool passesL1 = Props::pass_L1_J25_0ETA23_2J15_31ETA49.get( evtinfo ) > 0;
  bool passesHLT = Props::pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49.get( evtinfo ) > 0;

  if (passesL1)  Info("passJetSelection ()", "pass L1");
  else return false;
  m_cutFlow.count("L1_J25.0ETA23_2J15.31ETA49");

  if (passesHLT) Info("passJetSelection ()", "pass HLT");
  else return false;
  m_cutFlow.count("HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25.0ETA23_2J15.31ETA49");

  return true;
}

bool VBFHbb1centralEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {  
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VBFHbbEvtSelection<ResultVBFHbb1central>::passPreSelection(containers, isKinVar);
}

EL::StatusCode VBFHbb1centralEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                           xAOD::EventInfo* /*eventInfoOut*/,
                                                           bool /*isKinVar*/,
                                                           bool /*isWeightVar*/,
                                                           std::string /*sysName*/,
                                                           int /*rdm_RunNumber*/,
                                                           CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  return EL::StatusCode::SUCCESS;

}

bool VBFHbb1centralEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT
  return true;
}
