#include "CxAODTools_VHbb/VBFHbb2centralEvtSelection.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"
#include <TLorentzVector.h>

// Constructor
VBFHbb2centralEvtSelection::VBFHbb2centralEvtSelection(bool applybtag, TString WP, bool isMC)
  : m_applybtag(m_applybtag),
    m_WP(WP),
    m_isMC(isMC) {}
VBFHbb2centralEvtSelection::VBFHbb2centralEvtSelection(const VBFHbb2centralEvtSelection &other)
  : m_applybtag(other.m_applybtag),
    m_WP(other.m_WP),
    m_isMC(other.m_isMC) {}
// Destructor
VBFHbb2centralEvtSelection::~VBFHbb2centralEvtSelection() noexcept {}


void VBFHbb2centralEvtSelection::clearResult(){
  m_result.pass = false;
  m_result.jets.clear();
  m_result.forwardJets.clear();
  m_result.jets_etaSort.clear();
}

bool VBFHbb2centralEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code

  return VBFHbbEvtSelection<ResultVBFHbb2central>::passSelection(containers, isKinVar);
}

bool VBFHbb2centralEvtSelection::passJetSelection(const xAOD::JetContainer* jets){
  m_result.jets.clear();
  m_result.jets_etaSort.clear();

  double btagweight = 0;
  double btagweight_lead = 0;
  double btagweight_sub = 0;

  if (m_WP == "85"){
    btagweight_lead = m_b_85_weight;
    btagweight_sub  = m_b_85_weight;
  }
  if (m_WP == "77"){
    btagweight_lead = m_b_77_weight;
    btagweight_sub  = m_b_77_weight;
  }
  if (m_WP == "70"){
    btagweight_lead = m_b_70_weight;
    btagweight_sub  = m_b_70_weight;
  }
  if (m_WP == "asym"){
    btagweight_lead = m_b_70_weight;
    btagweight_sub  = m_b_85_weight;
  }


  int njets(0);
  int njets_80_0eta240(0);
  int njets_60_0eta320(0); 
  int njets_45_230eta490(0);
  int njets_25_gsc45_b70_0eta320(0);
  std::vector<int> HLTMatchID_J25_GSC45;

  for (unsigned int iJet(0) ; iJet < jets->size(); ++iJet) {
    const xAOD::Jet * jet = jets->at(iJet);
    if (!Props::passOR.get(jet)) continue;
    if (!(jet->pt() > 20e3)) continue;

    // all the jets above 20 GeV is used for jet veto in the following
    njets++;
    m_result.jets.push_back(jet);

    if ( jet->pt() > 80e3 && fabs(jet->eta()) < 2.4 ) njets_80_0eta240++;
    if ( jet->pt() > 60e3 && fabs(jet->eta()) < 3.2 ) njets_60_0eta320++;
    if ( jet->pt() > 45e3 && fabs(jet->eta()) > 2.3 && fabs(jet->eta()) < 4.9 ) njets_45_230eta490++;
    if ( jet->pt() > 45e3 && fabs(jet->eta()) < 3.2 && Props::MV2c10.get( jet ) > m_b_70_weight ) njets_25_gsc45_b70_0eta320++;
  }

  // Jets appear to be pt ordered but just in case
  std::sort(m_result.jets.begin(), m_result.jets.end(), sort_pt);

  const std::vector<const xAOD::Jet*>& selected_jets = m_result.jets;
  //Info("passJetSelection ()", "jet selection was passed");
  return true;
}


bool VBFHbb2centralEvtSelection::passTriggerSelection(const xAOD::EventInfo* evtinfo){
  m_cutFlow.count("AllEvents");

  bool passesL1  = Props::pass_L1_J40_0ETA25_2J25_J20_31ETA49.get( evtinfo ) > 0;
  bool passesHLT = Props::pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split.get( evtinfo ) > 0;

  if (passesL1)  Info("passJetSelection ()", "pass L1");
  else return false;
  m_cutFlow.count("L1_J40.0ETA25_2J25_J20.31ETA49");

  if (passesHLT) Info("passJetSelection ()", "pass HLT");
  m_cutFlow.count("HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split");

  return true; 
}

bool VBFHbb2centralEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {
  
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VBFHbbEvtSelection<ResultVBFHbb2central>::passPreSelection(containers, isKinVar);
}

EL::StatusCode VBFHbb2centralEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                           xAOD::EventInfo* /*eventInfoOut*/,
                                                           bool /*isKinVar*/,
                                                           bool /*isWeightVar*/,
                                                           std::string /*sysName*/,
                                                           int /*rdm_RunNumber*/,
                                                           CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  return EL::StatusCode::SUCCESS;
}

bool VBFHbb2centralEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT
  return true;
}


// b-tagging bench mark numbers
// https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTaggingBenchmarks
// recommendation is MV2c10 for 20.7
// eff 60 wp 0.934906
// eff 70 wp 0.8244273
// eff 77 wp 0.645925
// eff 85 wp 0.1758475
