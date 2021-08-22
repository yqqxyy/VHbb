
// electron specific includes
#include "CxAODMaker_VHbb/ElectronHandler_VHbb.h"
#include "CxAODTools/CommonProperties.h"


ElectronHandler_VHbb::ElectronHandler_VHbb(const std::string& name, ConfigStore& config,
                                           xAOD::TEvent* event,
                                           EventInfoHandler& eventInfoHandler)
  : ElectronHandler(name, config, event, eventInfoHandler),
    m_isMJ(false)
{
  m_config.getif<bool>("isMJ", m_isMJ);

  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind( &ElectronHandler_VHbb::passVHLooseElectron, this, _1));
  m_selectFcns.push_back(std::bind( &ElectronHandler_VHbb::passVHSignalElectron, this, _1));
  m_selectFcns.push_back(std::bind( &ElectronHandler_VHbb::passZHSignalElectron, this, _1));
  m_selectFcns.push_back(std::bind( &ElectronHandler_VHbb::passWHSignalElectron, this, _1));
}

bool ElectronHandler_VHbb::passVHLooseElectron(xAOD::Electron * electron) 
{

  bool passSel = true;

  // set priority of first cut to some value, which should be different for
  // other selection functions
  // cut names have to be unique (no check currently!)
  if (passSel) m_cutflow->count("VHLooseElectron input", 100);
  
  if (!(Props::isLooseLH.get(electron))) passSel = false;
  if (passSel) m_cutflow->count("isLooseLH");
  
  // TODO do we want to exclude the crack region?
  if(electron->caloCluster()){ 
    if (!(fabs(electron->caloCluster()->etaBE(2)) < 2.47)) passSel = false;
  }
  else{
    if (!(fabs(electron->eta()) < 2.47)) passSel = false;
    Warning("ElectronHandler_VHbb::passVHLooseElectron","Did not find caloCluster, use eta() instead of caloCluster()->etaBE(2) to check eta range!");
  }
  
  if (passSel) m_cutflow->count("|eta|");
  
  if (!(electron->pt() > 7000.)) passSel = false;
  if (passSel) m_cutflow->count("pt loose");
  if (!(fabs(Props::d0sigBL.get(electron)) < 5)) passSel = false;
  if (passSel) m_cutflow->count("|d0sigBL|");
  if (!(fabs(Props::z0sinTheta.get(electron)) < 0.5)) passSel = false;
  if (passSel) m_cutflow->count("|z0sinTheta|");

  if (!(electron->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON))) passSel = false;
  if (passSel) m_cutflow->count("isGoodOQ");

  // Isolation
  bool m_useLooseTrackOnlyLeptonIso = false;
  bool m_useVVSemileptonicElectronIso = false;
  m_config.getif<bool>("useLooseTrackOnlyLeptonIso", m_useLooseTrackOnlyLeptonIso);
  m_config.getif<bool>("useVVSemileptonicElectronIso", m_useVVSemileptonicElectronIso);
  if(!m_useVVSemileptonicElectronIso){
    // VHbb definition
    if(m_useLooseTrackOnlyLeptonIso){
      if (!(Props::isLooseTrackOnlyIso.get(electron)) ) passSel = false;}
    else{
      if (!(Props::isFixedCutLooseIso.get(electron)) ) passSel = false;}
    if (passSel) m_cutflow->count("Pass LooseTrackIso");
  }else{
    // DBL definition
    if ( !( electron->pt()>100e+3 || (Props::isFixedCutLooseIso.get(electron) || Props::isFixedCutHighPtCaloOnlyIso.get(electron)) ) ) passSel = false;
    if (passSel) m_cutflow->count("Pass DBL LeptonIso");
  }

  Props::isVHLooseElectron.set(electron, passSel);
  Props::passPreSel.set(electron, passSel);
  Props::forMETRebuild.set(electron, passSel);
  if (passSel) m_cutflow->count("VHLooseElectron selected");

  return passSel;

}

bool ElectronHandler_VHbb::passVHSignalElectron(xAOD::Electron * electron) 
{

  bool passSel = true;

  if (!Props::isVHLooseElectron.get(electron)) passSel = false;
  if (passSel) m_cutflow->count("VHSignalElectron input", 200);

  if (!(electron->pt() > 25000.)) passSel = false;
  if (passSel) m_cutflow->count("pt signal");
 
  Props::isVHSignalElectron.set(electron, passSel);
  if (passSel) m_cutflow->count("VHSignalElectron selected");

  return passSel;

}

bool ElectronHandler_VHbb::passZHSignalElectron(xAOD::Electron * electron) 
{

  bool passSel = true;

  if (!Props::isVHSignalElectron.get(electron)) passSel = false;
  if (passSel) m_cutflow->count("ZHSignalElectron input", 300);
 
  Props::isZHSignalElectron.set(electron, passSel);
  if (passSel) m_cutflow->count("ZHSignalElectron selected");

  return passSel;

}

bool ElectronHandler_VHbb::passWHSignalElectron(xAOD::Electron * electron) 
{
  bool passSel = true;

  if (!Props::isVHSignalElectron.get(electron)) passSel = false;
  if (passSel) m_cutflow->count("WHSignalElectron input", 400);

  if(        ( m_isMJ && !(Props::isMediumLH.get(electron)))
	  || (!m_isMJ && !(Props::isTightLH.get(electron))))
    passSel = false;
  if (passSel) m_cutflow->count("isTightLH");
  
  if ( !(ElectronHandler_VHbb::passWHSignalElectronIso(electron)) ) passSel = false;
  if (passSel) m_cutflow->count("Pass TightIso");

  Props::isWHSignalElectron.set(electron, passSel);
  if (passSel) m_cutflow->count("WHSignalElectron selected");

  return passSel;

}

bool ElectronHandler_VHbb::passWHSignalElectronIso(xAOD::Electron * /*electron*/)
{
  // NM Temporary measure, for r28. To be revisited once SM VHbb has converged on a WP

  return true;

  //if( m_isMJ ) {
  //return true;
  //}
  //else {
  //if( (Props::isFixedCutTightIso.get(electron)) ) { return true; }
  //}
  //return false;
}

EL::StatusCode ElectronHandler_VHbb::writeCustomVariables(xAOD::Electron * inElectron, xAOD::Electron * outElectron, bool /*isKinVar*/, bool isWeightVar, const TString& /*sysName*/) 
{

  if(!isWeightVar) {
    Props::isVHLooseElectron.copy(inElectron, outElectron);
    Props::isZHSignalElectron.copy(inElectron, outElectron);
    Props::isWHSignalElectron.copy(inElectron, outElectron);
    if(inElectron->isAvailable< float >("topoetcone30")){
      outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::topoetcone30),xAOD::Iso::topoetcone30);}
    if(inElectron->isAvailable< float >("topoetcone40")){
      outElectron->setIsolationValue(inElectron->isolationValue(xAOD::Iso::topoetcone40),xAOD::Iso::topoetcone40);}
  }

  return EL::StatusCode::SUCCESS;
}
