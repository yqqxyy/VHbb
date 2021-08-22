#include <iostream>

//muon specific includes
#include "CxAODMaker_VHbb/MuonHandler_VHbb.h"

MuonHandler_VHbb::MuonHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent *event,
                         EventInfoHandler & eventInfoHandler) :
  MuonHandler(name, config, event, eventInfoHandler),
  m_isMJ(false)
{
  m_config.getif<bool>("isMJ", m_isMJ);

  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind( &MuonHandler_VHbb::passVHLooseMuon, this, _1));
  m_selectFcns.push_back(std::bind( &MuonHandler_VHbb::passVHSignalMuon, this, _1));
  m_selectFcns.push_back(std::bind( &MuonHandler_VHbb::passZHSignalMuon, this, _1));
  m_selectFcns.push_back(std::bind( &MuonHandler_VHbb::passWHSignalMuon, this, _1));
}

bool MuonHandler_VHbb::passVHLooseMuon(xAOD::Muon * muon) 
{
  bool passSel = true;
  if (passSel) m_cutflow->count("VHLooseMuon input", 100);

  //apply selection for combined or segment tagged muons for now

  if (!(Props::acceptedMuonTool.get(muon)) ) passSel = false;
  if (passSel) m_cutflow->count("IsAtLeastLooseMuon");//This is the starting quality we require in the tool
  if (!(fabs(Props::d0sigBL.get(muon)) < 3)) passSel = false;
  if (passSel) m_cutflow->count("|d0sigBL|");
  if (!(fabs(Props::z0sinTheta.get(muon)) < 0.5)) passSel = false;
  if (passSel) m_cutflow->count("|z0sinTheta|");
  if (!(muon->pt() > 7000.)) passSel = false;
  if (passSel) m_cutflow->count("pt loose");

  if( m_isMJ ) {
    if(!(Props::ptvarcone30.get(muon)/muon->pt() < 0.15 )) passSel = false;
    if (passSel) m_cutflow->count("Pass FixedCutLoose Iso");
  }
  else{
    bool m_useLooseTrackOnlyLeptonIso = false;
    bool m_useVVSemileptonicMuonIso = false;
    m_config.getif<bool>("useLooseTrackOnlyLeptonIso", m_useLooseTrackOnlyLeptonIso);
    m_config.getif<bool>("useVVSemileptonicMuonIso", m_useVVSemileptonicMuonIso);
    if(!m_useVVSemileptonicMuonIso){
      // VHbb definition
      if(m_useLooseTrackOnlyLeptonIso){
        if (!(Props::isLooseTrackOnlyIso.get(muon)) ) passSel = false;
      }else{
        if (!(Props::isFixedCutLooseIso.get(muon)) ) passSel = false;
      }
      if (passSel) m_cutflow->count("Pass FixedCutLoose Iso");
    }else{
      // DBL definition
      if ( !( muon->pt()>100e+3 || (Props::isFixedCutLooseIso.get(muon) || Props::isFixedCutTightTrackOnlyIso.get(muon)) ) ) passSel = false;
      if (passSel) m_cutflow->count("Pass DBL Lepton Iso");
    }
  }

  Props::isVHLooseMuon.set(muon, passSel);
  Props::passPreSel.set(muon, passSel);
  Props::forMETRebuild.set(muon, passSel);
  if (passSel) m_cutflow->count("VHLooseMuon selected");
  return passSel;
}

bool MuonHandler_VHbb::passVHSignalMuon(xAOD::Muon* muon){
  
  bool passSel = true;
  
  //check loose selection is passed
  if(!(Props::isVHLooseMuon.get(muon))) passSel = false;
  if (passSel) m_cutflow->count("VHSignalMuon input",200);
  //common cuts
  if (!(fabs(muon->eta()) < 2.5)) passSel = false;
  if (passSel) m_cutflow->count("|eta|<2.5 signal");
  if (!(muon->pt() > 25000.)) passSel = false;
  if (passSel) m_cutflow->count("pt>25 GeV signal");
  Props::isVHSignalMuon.set(muon, passSel);
  if (passSel) m_cutflow->count("VHSignalMuon selected");
  return passSel;
}

bool MuonHandler_VHbb::passZHSignalMuon(xAOD::Muon* muon){
  
  bool passSel = true;
 
  //check VH selection is passed
  if(!(Props::isVHSignalMuon.get(muon))) passSel = false;
  if (passSel) m_cutflow->count("ZHSignalMuon input", 300);

  Props::isZHSignalMuon.set(muon, passSel);
  if (passSel) m_cutflow->count("ZHSignalMuon selected");

  return passSel;
}

bool MuonHandler_VHbb::passWHSignalMuon(xAOD::Muon* muon){
  bool passSel = true;
 
  //check VH selection is passed
  if(!(Props::isVHSignalMuon.get(muon))) passSel = false;
  if (passSel) m_cutflow->count("WHSignalMuon input", 400);

  if (!(Props::passedIDCuts.get(muon))) passSel = false;
  if (passSel) m_cutflow->count ("WHSignal Muon passed IDCuts");

  if (!(Props::muonQuality.get(muon) < 2)) passSel = false; // 0 == tight, 1 == medium. Selects medium & tight muons
  if (passSel) m_cutflow->count ("WHSignalMuon quality");

  if ( !(MuonHandler_VHbb::passWHSignalMuonIso(muon)) ) passSel = false;
  if (passSel) m_cutflow->count("Pass TightIso");

  Props::isWHSignalMuon.set(muon, passSel);
  if (passSel) m_cutflow->count("WHSignalMuon selected");
  return passSel;
}

bool MuonHandler_VHbb::passWHSignalMuonIso(xAOD::Muon *muon)
{
  // NM Temporary measure, for r28. To be revisited once SM VHbb has converged on a WP
  bool passSel = true;
  if( !(Props::isFixedCutHighPtTrackOnlyIso.get(muon)) ) passSel = false;

  passSel = true;
  return passSel;
  //return true;

  //if( m_isMJ ) {
  ////if(!(Props::isFixedCutTightTrackOnlyIso.get(muon)) && (Props::isLooseTrackOnlyIso.get(muon)) ) { return true; }
  //if(Props::ptvarcone30.get(muon)/muon->pt() < 0.15 ) { return true; }
  //}
  //else {
  //if( (Props::isFixedCutTightTrackOnlyIso.get(muon)) ) { return true; }
  //}
  //return false;
}

EL::StatusCode MuonHandler_VHbb::writeCustomVariables(xAOD::Muon * inMuon, xAOD::Muon * outMuon, bool /*isKinVar*/, bool isWeightVar, const TString& /*sysName*/) 
{
  // those are independent of weight systematics
  if(!isWeightVar) {
    Props::isVHLooseMuon.copy(inMuon, outMuon);
    Props::isZHSignalMuon.copy(inMuon, outMuon);
    Props::isWHSignalMuon.copy(inMuon, outMuon);
    Props::muonQuality.copy(inMuon, outMuon);
  }
  
  return EL::StatusCode::SUCCESS;

}
