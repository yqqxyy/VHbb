#include "CxAODTools_VHbb/VHbb2lepEvtSelection.h"
#include "CxAODTools/ReturnCheck.h"

#include "xAODEgamma/Electron.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"

#include "TLorentzVector.h"

#include <iostream>

VHbb2lepEvtSelection::VHbb2lepEvtSelection(ConfigStore* config):
  VHbbEvtSelection(config)
{}

void VHbb2lepEvtSelection::clearResult() {
  m_result.pass = false;
  m_result.signalJets.clear();
  m_result.forwardJets.clear();
  m_result.fatJets.clear();
  m_result.trackJets.clear();
  m_result.subJets.clear();
  m_result.taus.clear();
  m_result.el1 = nullptr;
  m_result.el2 = nullptr;
  m_result.mu1 = nullptr;
  m_result.mu2 = nullptr;
  m_result.met = nullptr;
  m_result.truthElectrons.clear();
  m_result.truthMuons.clear();
  m_result.truthTaus.clear();
  m_result.truthSignalJets.clear();
  m_result.truthForwardJets.clear();
  m_result.truthFatJets.clear();
}

bool VHbb2lepEvtSelection::passSelection(SelectionContainers & containers, bool isKinVar) {

  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  return VHbbEvtSelection<ResultVHbb2lep>::passSelection(containers, isKinVar);
}

bool VHbb2lepEvtSelection::passLeptonSelection(const xAOD::ElectronContainer* electrons,
                                               const xAOD::MuonContainer* muons,
                                               const xAOD::MissingET* met) {
  int res = doVHLeptonSelection(electrons, muons, m_result.el1, m_result.el2,
                                m_result.mu1, m_result.mu2);
  if(res != 2) {
    return false;
  }
  m_result.met = met;
  return true;

}

bool VHbb2lepEvtSelection::passKinematics() {
  // MJ cuts, like MET / MPT etc...
  // my advice is to add in passKinematics() prototype all the stuff that
  // doesn't need to be put in the Result struct, like MPT

  return true;
}

//
// Pre-selection versions
bool VHbb2lepEvtSelection::passPreSelection(SelectionContainers & containers, bool isKinVar) {
  // here just as an example:
  // if a new passKinematics() function is defined with some variables in the prototype,
  // one needs to reimplement passSelection here
  // otherwise, don't need to put any code
  bool passpreselection = VHbbEvtSelection<ResultVHbb2lep>::passPreSelection(containers, isKinVar);

  if(passpreselection) {
    TLorentzVector lep1(0,0,0,0);
    TLorentzVector lep2(0,0,0,0);
    if( m_result.mu1!=0){
      lep1.SetPtEtaPhiE(m_result.mu1->pt(),m_result.mu1->eta(),m_result.mu1->phi(),m_result.mu1->e());
    }
    if( m_result.mu2!=0){
      if(lep1.Mag()==0){    
	lep1.SetPtEtaPhiE(m_result.mu2->pt(),m_result.mu2->eta(),m_result.mu2->phi(),m_result.mu2->e());
      }
      else{
	lep2.SetPtEtaPhiE(m_result.mu2->pt(),m_result.mu2->eta(),m_result.mu2->phi(),m_result.mu2->e());
      }
    }
  
    if( m_result.el1!=0){
      if(lep1.Mag()==0){
	lep1.SetPtEtaPhiE(m_result.el1->pt(),m_result.el1->eta(),m_result.el1->phi(),m_result.el1->e());
      }
      else{
	lep2.SetPtEtaPhiE(m_result.el1->pt(),m_result.el1->eta(),m_result.el1->phi(),m_result.el1->e());
      }
    }
    if( m_result.el2!=0){
      if(lep1.Mag()==0){
	lep1.SetPtEtaPhiE(m_result.el2->pt(),m_result.el2->eta(),m_result.el2->phi(),m_result.el2->e());
      }
      else{
	lep2.SetPtEtaPhiE(m_result.el2->pt(),m_result.el2->eta(),m_result.el2->phi(),m_result.el2->e());
      }
    }
    TLorentzVector dilep = lep1+lep2;
    double PtValue = dilep.Pt();

    //In SM, lower cut on pTZ (this flag has to be set both in the Maker and Reader config file)
    bool doLowerPtZ2LCut = false;
    m_config->getif<bool>("doLowerPtZ2LCut", doLowerPtZ2LCut);
    if(doLowerPtZ2LCut){
      if (PtValue / 1000 < 75) {
	m_result.pass = false;
	return false;
      }
    }

    if(!isKinVar){
      if( PtValue/1000 <= 90){
	m_cutFlow.count("75GeV < Pt < 90GeV",200);
      }
      if(PtValue/1000 > 90 && PtValue/1000 <= 120){
	m_cutFlow.count("90GeV < Pt < 120GeV",201);
      }
      if(PtValue/1000 > 120 && PtValue/1000 <= 160){
	m_cutFlow.count("120GeV < Pt < 160GeV",202);
      }
      if(PtValue/1000 > 160 && PtValue/1000 <= 200){
	m_cutFlow.count("160GeV < Pt < 200GeV",203);
      }
      if( PtValue/1000 > 200){
	m_cutFlow.count("Pt > 200GeV",204);
      }
    }
  }
  return passpreselection;
}

bool VHbb2lepEvtSelection::passLeptonPreSelection(const xAOD::ElectronContainer* electrons,
                                                  const xAOD::MuonContainer* muons,
                                                  const xAOD::MissingET* /*met*/) {
  int res = doVHLeptonPreSelection(electrons, muons, m_result.el1, m_result.el2,
                                   m_result.mu1, m_result.mu2);
  if(res != 2) {
    return false;
  }
  return true;

}

EL::StatusCode VHbb2lepEvtSelection::writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                                         xAOD::EventInfo* eventInfoOut,
                                                         bool /*isKinVar*/,
                                                         bool /*isWeightVar*/,
                                                         std::string sysName,
                                                         int /*rdm_RunNumber*/,
                                                         CP::MuonTriggerScaleFactors* /*trig_sfmuon*/) {

  bool sysForLeptonSF=!(TString(sysName).BeginsWith("JET"));

  // ensure to write all variables for all events
  if(sysForLeptonSF) {
    Props::leptonSF.set(eventInfoOut, 1);
  }  

  if (!m_result.pass) return EL::StatusCode::SUCCESS;

  // lepton SF
  float leptonSF = 1;
  if (m_result.type() == ResultVHbb2lep::Type::ee) {
    leptonSF *= Props::effSFlooseLH.get(m_result.el1)*
      Props::effSFReco.get(m_result.el1)*
      Props::effSFIsoFixedCutLooseLooseLH.get(m_result.el1);
    leptonSF *= Props::effSFlooseLH.get(m_result.el2)*
      Props::effSFReco.get(m_result.el2)*
      Props::effSFIsoFixedCutLooseLooseLH.get(m_result.el2);
  } else if (m_result.type() == ResultVHbb2lep::Type::mm) {
    leptonSF *= Props::effSF.get(m_result.mu1)*
      Props::TTVAEffSF.get(m_result.mu1)*
      Props::fixedCutLooseIsoSF.get(m_result.mu1);
    leptonSF *= Props::effSF.get(m_result.mu2)*
      Props::TTVAEffSF.get(m_result.mu2)*
      Props::fixedCutLooseIsoSF.get(m_result.mu2);
  } else if (m_result.type() == ResultVHbb2lep::Type::em) {
    leptonSF *= Props::effSFlooseLH.get(m_result.el1)*
      Props::effSFReco.get(m_result.el1)*
      Props::effSFIsoFixedCutLooseLooseLH.get(m_result.el1);
    leptonSF *= Props::effSF.get(m_result.mu1)*
      Props::TTVAEffSF.get(m_result.mu1)*
      Props::fixedCutLooseIsoSF.get(m_result.mu1);
  }
  
  if(sysForLeptonSF) {
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
 //
 //   // make muon container needed to get trigger SF/syst
 //   ConstDataVector<xAOD::MuonContainer> selectedMuons(SG::VIEW_ELEMENTS);
 //   if (m_result.mu1) selectedMuons.push_back( m_result.mu1 );
 //   if (m_result.mu2) selectedMuons.push_back( m_result.mu2 );
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

  return EL::StatusCode::SUCCESS;

}

