#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"
#include "CxAODTools/ReturnCheck.h"

#include "xAODEgamma/Electron.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"

#include "TVector2.h"
#include <iostream>

VHbb1lepEvtSelection::VHbb1lepEvtSelection(ConfigStore* config) :
  VHbbEvtSelection(config)
{}

void VHbb1lepEvtSelection::clearResult() {
  m_result.pass = false;
  m_result.signalJets.clear();
  m_result.forwardJets.clear();
  m_result.fatJets.clear();
  m_result.trackJets.clear();
  m_result.subJets.clear();
  m_result.taus.clear();
  m_result.el = nullptr;
  m_result.mu = nullptr;
  m_result.met = nullptr;
  m_result.truthTaus.clear();
  m_result.truthMuons.clear();
  m_result.truthElectrons.clear();
  m_result.truthSignalJets.clear();
  m_result.truthForwardJets.clear();
  m_result.truthFatJets.clear();
  m_result.truthParticles.clear();
}

bool VHbb1lepEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VHbbEvtSelection<ResultVHbb1lep>::passSelection(containers, isKinVar);
}

bool VHbb1lepEvtSelection::passLeptonSelection(const xAOD::ElectronContainer* electrons,
                                               const xAOD::MuonContainer* muons,
                                               const xAOD::MissingET* met) {

  const xAOD::Electron* el_junk=nullptr;
  const xAOD::Muon* mu_junk=nullptr;
  int res = doVHLeptonSelection(electrons, muons, m_result.el, el_junk, m_result.mu, mu_junk);
  //Check for a single tightLH lepton, etc...
  bool passLepSel = false;
  if(res == 1) {
    passLepSel = true;
  }
  
  m_result.met = met;
  return passLepSel;

}

//
bool VHbb1lepEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {

  bool passVHpreselection = VHbbEvtSelection<ResultVHbb1lep>::passPreSelection(containers, isKinVar);
  if (!passVHpreselection) {
    m_result.pass = false;
    return false;
  }
  
  const xAOD::MissingET * met = containers.met;
  if ( met ) {
    m_result.met = met ;
  } else {
    m_result.pass = false;
    return false;
  }

  TVector2 lep;
  TVector2 met_2D;
  met_2D.SetMagPhi(m_result.met->met(), m_result.met->phi());
  if (m_result.mu != 0) {
    lep.SetMagPhi(m_result.mu->pt(), m_result.mu->phi());
  }
  if (m_result.el != 0) {
    lep.SetMagPhi(m_result.el->pt(), m_result.el->phi());
  }
  TVector2 total;
  total = lep + met_2D;
  double PtValue = total.Mod();
  
  if (PtValue / 1000 < 75) {
    m_result.pass = false;
    return false;
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

  if (!isKinVar) {
    m_cutFlow.count("nJ res. < 5",200);

    if (PtValue / 1000 <= 90) {
      m_cutFlow.count("75GeV Pt < 90GeV", 201);
    }
    if (PtValue / 1000 > 90 && PtValue / 1000 <= 120) {
      m_cutFlow.count("90GeV < Pt < 120GeV", 202);
    }
    if (PtValue / 1000 > 120 && PtValue / 1000 <= 160) {
      m_cutFlow.count("120GeV < Pt < 160GeV", 203);
    }
    if (PtValue / 1000 > 160 && PtValue / 1000 <= 200) {
      m_cutFlow.count("160GeV < Pt < 200GeV", 204);
    }
    if (PtValue / 1000 > 200) {
      m_cutFlow.count("Pt > 200GeV", 205);
    }
  }
  
  m_result.pass = true;
  return true;
}

bool VHbb1lepEvtSelection::passLeptonPreSelection(const xAOD::ElectronContainer* electrons,
						  const xAOD::MuonContainer* muons,
						  const xAOD::MissingET* /*met*/) {
  const xAOD::Electron* el_junk=nullptr;
  const xAOD::Muon* mu_junk=nullptr;
  int res = doVHLeptonPreSelection(electrons, muons, m_result.el, el_junk, m_result.mu, mu_junk);
  if(res != 1) {
    return false;
  }
  return true;

}

EL::StatusCode VHbb1lepEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                         xAOD::EventInfo* eventInfoOut,
                                                         bool /*isKinVar*/,
                                                         bool /*isWeightVar*/,
                                                         std::string sysName,
                                                         int /*rdm_RunNumber*/,
                                                         CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  bool sysForLeptonSF=!(TString(sysName).BeginsWith("JET"));

  // ensure to write all variables for all events
  if (sysForLeptonSF) {
      Props::leptonSF.set(eventInfoOut, 1);
    }  

  if (!m_result.pass) return EL::StatusCode::SUCCESS;

  // lepton SF
  float leptonSF = 1;  
  if (m_result.isEl()) {
    //if (Props::isFixedCutTightTrackOnlyIso.get(m_result.el) == 1){ //Regular Isolation scale factors
    if(Props::isFixedCutHighPtCaloOnlyIso.get(m_result.el) == 1){//Regular Isolation scale factors
	leptonSF *= Props::effSFtightLH.get(m_result.el)*
	Props::effSFReco.get(m_result.el)*
	//Props::effSFIsoFixedCutTightTightLH.get(m_result.el);
    Props::effSFIsoFixedCutHighPtCaloOnlyTightLH.get(m_result.el);
	}
    else { //Inverted Isolation scale factors
      leptonSF *= Props::effSFtightLH.get(m_result.el)*
	Props::effSFReco.get(m_result.el)*
	Props::effSFIsoFixedCutLooseTightLH.get(m_result.el);
    }
  } else {
    //if (Props::isFixedCutTightTrackOnlyIso.get(m_result.mu) == 1){ //Regular Isolation scale factors
    if (Props::isFixedCutHighPtTrackOnlyIso.get(m_result.mu) == 1){ //Regular Isolation scale factors
	leptonSF *= Props::mediumEffSF.get(m_result.mu)*
    	Props::TTVAEffSF.get(m_result.mu)*
	//Props::fixedCutTightTrackOnlyIsoSF.get(m_result.mu);
    Props::fixedCutHighPtTrackOnlyIsoSF.get(m_result.mu);
	}
    else { //Inverted Isolation scale factors here
      leptonSF *= Props::mediumEffSF.get(m_result.mu)*
    	Props::TTVAEffSF.get(m_result.mu)*
	Props::fixedCutLooseIsoSF.get(m_result.mu);
    }
  }//end if (m_result.isEl())
  
  if (sysForLeptonSF) {
    Props::leptonSF.set(eventInfoOut, leptonSF);
  }
  
 // // compute and store muon trigger SF/syst weight of the event
 // std::string variation = "Nominal";
 // if (sysName.find("MUON_EFF_Trig") !=std::string::npos) variation = sysName;
 // //
 // double trigSF_mu_HLT_mu20_iloose_L1MU15 = 1.; 
 // double trigSF_mu_HLT_mu24_iloose_L1MU15 = 1.;
 // double trigSF_mu_HLT_mu24_imedium = 1.;
 // double trigSF_mu_HLT_mu26_imedium = 1.;
 // double trigSF_mu_HLT_mu50 = 1.;
 // double trigSF_mu_HLT_mu20_iloose_L1MU15_OR_HLT_mu50 = 1.;
 // double trigSF_mu_HLT_mu24_iloose_L1MU15_OR_HLT_mu50 = 1.;
 // double trigSF_mu_HLT_mu24_imedium_OR_HLT_mu50 = 1.;
 // double trigSF_mu_HLT_mu26_imedium_OR_HLT_mu50 = 1.;
 //
 // if(rdm_RunNumber>0) {
 //   // make muon container needed to get trigger SF/syst
 //   ConstDataVector<xAOD::MuonContainer> selectedMuons(SG::VIEW_ELEMENTS);
 //   if (m_result.mu) selectedMuons.push_back( m_result.mu );
 //   // setup the data period and systematics
 //   CP_CHECK("TriggerTool::getTriggerDecision, setRunNumber",trig_sfmuon->setRunNumber(rdm_RunNumber),false);
 //   CP::SystematicVariation testSys(variation);
 //   CP::SystematicSet shiftSet(testSys.name());
 //   if(trig_sfmuon->applySystematicVariation(shiftSet) != CP::SystematicCode::Ok) exit(1);
 //   // compute event weight for SF
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu20_iloose_L1MU15            ,"HLT_mu20_iloose_L1MU15"            ),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu24_iloose_L1MU15            ,"HLT_mu24_iloose_L1MU15"            ),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu24_imedium                  ,"HLT_mu24_imedium"                  ),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu26_imedium                  ,"HLT_mu26_imedium"                  ),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu50                          ,"HLT_mu50"                          ),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu20_iloose_L1MU15_OR_HLT_mu50,"HLT_mu20_iloose_L1MU15_OR_HLT_mu50"),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu24_iloose_L1MU15_OR_HLT_mu50,"HLT_mu24_iloose_L1MU15_OR_HLT_mu50"),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu24_imedium_OR_HLT_mu50      ,"HLT_mu24_imedium_OR_HLT_mu50"      ),false);
 //   CP_CHECK("TriggerTool::getTriggerDecision",trig_sfmuon->getTriggerScaleFactor(*selectedMuons.asDataVector(),trigSF_mu_HLT_mu26_imedium_OR_HLT_mu50      ,"HLT_mu26_imedium_OR_HLT_mu50"      ),false);
 // }
 // // set values
 // double trigSF_mu = trigSF_mu_HLT_mu26_imedium_OR_HLT_mu50;
 // Props::trigSF_mu.set(eventInfoOut, trigSF_mu);
 // //
 // Props::trigSF_mu_HLT_mu20_iloose_L1MU15.set(eventInfoOut, trigSF_mu_HLT_mu20_iloose_L1MU15);
 // Props::trigSF_mu_HLT_mu24_iloose_L1MU15.set(eventInfoOut, trigSF_mu_HLT_mu24_iloose_L1MU15);
 // Props::trigSF_mu_HLT_mu24_imedium.set(eventInfoOut, trigSF_mu_HLT_mu24_imedium);
 // Props::trigSF_mu_HLT_mu26_imedium.set(eventInfoOut, trigSF_mu_HLT_mu26_imedium);
 // Props::trigSF_mu_HLT_mu50.set(eventInfoOut, trigSF_mu_HLT_mu50);
 // Props::trigSF_mu_HLT_mu20_iloose_L1MU15_OR_HLT_mu50.set(eventInfoOut, trigSF_mu_HLT_mu20_iloose_L1MU15_OR_HLT_mu50);
 // Props::trigSF_mu_HLT_mu24_iloose_L1MU15_OR_HLT_mu50.set(eventInfoOut, trigSF_mu_HLT_mu24_iloose_L1MU15_OR_HLT_mu50);
 // Props::trigSF_mu_HLT_mu24_imedium_OR_HLT_mu50.set(eventInfoOut, trigSF_mu_HLT_mu24_imedium_OR_HLT_mu50);
 // Props::trigSF_mu_HLT_mu26_imedium_OR_HLT_mu50.set(eventInfoOut, trigSF_mu_HLT_mu26_imedium_OR_HLT_mu50);
 //
  return EL::StatusCode::SUCCESS;

}

bool VHbb1lepEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT

  return true;
}
