// Framework includes
#include "CxAODMaker/TrigObjects.h"
#include "CxAODTools/CommonProperties.h"

// EDM includes
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTrigMissingET/TrigMissingETContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigEgamma/TrigPhotonContainer.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODTrigger/JetRoIContainer.h"

bool TrigObjects::copy(const xAOD::EventInfo * in, xAOD::EventInfo * out, bool allowTrigObjFail) 
{

  // copy properties
  for (PROP<std::vector<float> > * prop : m_props) {
    bool status = prop->copyIfExists(in, out);
    if ( ! allowTrigObjFail && ! status ) {
      Error("TrigObjects::copy()","Accessor for '%s' not available on input! Called from %s", (prop->Scope() + "::" + prop->Name() + "<" + prop->Type() + ">").c_str(), m_name.c_str());
      return false;
    } 
  }

  return true;
  
}


bool HLTJet::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::JetContainer * container = 0;
  if ( ! retrieve<xAOD::JetContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;

  // fill vectors
  const unsigned int N = container->size(); 
  std::vector<float> pt(N), phi(N), eta(N);
  for (unsigned int i = 0; i < N; ++i) {
    const xAOD::Jet * jet = container->at(i);
    pt[i]  = jet->pt();
    phi[i] = jet->phi();
    eta[i] = jet->eta();
  }

  // decorate EventInfo 
  Props::HLT_JetPt.set(evtInfo, pt);
  Props::HLT_JetPhi.set(evtInfo, phi);
  Props::HLT_JetEta.set(evtInfo, eta);

  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::HLT_JetPt  );
    m_props.push_back( &Props::HLT_JetPhi );
    m_props.push_back( &Props::HLT_JetEta );
  }

  // all okay
  return true;
}


bool HLTMuon::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::MuonContainer * container = 0;
  if ( ! retrieve<xAOD::MuonContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;

  // fill vectors
  const unsigned int N = container->size(); 
  std::vector<float> pt(N), phi(N), eta(N);
  for (unsigned int i = 0; i < N; ++i) {
    const xAOD::Muon * muon = container->at(i);
    pt[i]  = muon->pt();
    phi[i] = muon->phi();
    eta[i] = muon->eta();
  }
  
  // decorate EventInfo 
  Props::HLT_MuonPt.set(evtInfo, pt);
  Props::HLT_MuonPhi.set(evtInfo, phi);
  Props::HLT_MuonEta.set(evtInfo, eta);

  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::HLT_MuonPt  );
    m_props.push_back( &Props::HLT_MuonPhi );
    m_props.push_back( &Props::HLT_MuonEta );
  }

  // all okay
  return true;

}


bool HLTElectron::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::ElectronContainer * container = 0;
  if ( ! retrieve<xAOD::ElectronContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;

  // fill vectors
  const unsigned int N = container->size(); 
  std::vector<float> pt(N), phi(N), eta(N);
  for (unsigned int i = 0; i < N; ++i) {
    const xAOD::Electron * electron = container->at(i);
    pt[i]  = electron->pt();
    phi[i] = electron->phi();
    eta[i] = electron->eta();
  }
  
  // decorate EventInfo 
  Props::HLT_ElectronPt.set(evtInfo, pt);
  Props::HLT_ElectronPhi.set(evtInfo, phi);
  Props::HLT_ElectronEta.set(evtInfo, eta);

  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::HLT_ElectronPt  );
    m_props.push_back( &Props::HLT_ElectronPhi );
    m_props.push_back( &Props::HLT_ElectronEta );
  }

  // all okay
  return true;

}


bool HLTPhoton::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::PhotonContainer * container = 0;
  if ( ! retrieve<xAOD::PhotonContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;

  // fill vectors
  const unsigned int N = container->size(); 
  std::vector<float> pt(N), phi(N), eta(N);
  for (unsigned int i = 0; i < N; ++i) {
    const xAOD::Photon * photon = container->at(i);
    pt[i]  = photon->pt();
    phi[i] = photon->phi();
    eta[i] = photon->eta();
  }
  
  // decorate EventInfo 
  Props::HLT_PhotonPt.set(evtInfo, pt);
  Props::HLT_PhotonPhi.set(evtInfo, phi);
  Props::HLT_PhotonEta.set(evtInfo, eta);

  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::HLT_PhotonPt  );
    m_props.push_back( &Props::HLT_PhotonPhi );
    m_props.push_back( &Props::HLT_PhotonEta );
  }

  // all okay
  return true;

}


bool HLTMet::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::TrigMissingETContainer * container = 0;
  if ( ! retrieve<xAOD::TrigMissingETContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;
  
  float METx  = 0.;
  float METy  = 0.;
  if ( container->size() > 0 ) {
    const xAOD::TrigMissingET * met = container->at(0);
    METx = met->ex();
    METy = met->ey();
  }
  std::vector<float> METet;
  std::vector<float> METphi;
  METet.push_back( sqrt(METx*METx + METy*METy) );
  if ( METx != 0. ) METphi.push_back( atan(METy/METx) );
  else              METphi.push_back( -5. );

  // decorate EventInfo
  Props::HLT_METet.set(evtInfo, METet);
  Props::HLT_METphi.set(evtInfo, METphi);
  
  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::HLT_METet  );
    m_props.push_back( &Props::HLT_METphi );
  }

  // all okay
  return true;

}


bool L1EM::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::EmTauRoIContainer * container = 0;
  if ( ! retrieve<xAOD::EmTauRoIContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;

  // fill vectors
  const unsigned int N = container->size(); 
  std::vector<float> et(N), phi(N), eta(N);
  for (unsigned int i = 0; i < N; ++i) {
    const xAOD::EmTauRoI * roi = container->at(i);
    et[i]  = roi->eT();
    phi[i] = roi->phi();
    eta[i] = roi->eta();
  }
  
  // decorate EventInfo 
  Props::L1_EMEt.set(evtInfo, et);
  Props::L1_EMPhi.set(evtInfo, phi);
  Props::L1_EMEta.set(evtInfo, eta);

  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::L1_EMEt  );
    m_props.push_back( &Props::L1_EMPhi );
    m_props.push_back( &Props::L1_EMEta );
  }

  // all okay
  return true;

}


bool L2Photon::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::TrigPhotonContainer * container = 0;
  if ( ! retrieve<xAOD::TrigPhotonContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;

  // fill vectors
  const unsigned int N = container->size(); 
  std::vector<float> pt(N), phi(N), eta(N);
  for (unsigned int i = 0; i < N; ++i) {
    const xAOD::TrigPhoton * photon = container->at(i);
    pt[i]  = photon->pt();
    phi[i] = photon->phi();
    eta[i] = photon->eta();
  }
  
  // decorate EventInfo 
  Props::L2_PhotonPt.set(evtInfo, pt);
  Props::L2_PhotonPhi.set(evtInfo, phi);
  Props::L2_PhotonEta.set(evtInfo, eta);

  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::L2_PhotonPt  );
    m_props.push_back( &Props::L2_PhotonPhi );
    m_props.push_back( &Props::L2_PhotonEta );
  }

  // all okay
  return true;

}


bool L1Met::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::EnergySumRoI * container = 0;
  if ( ! retrieve<xAOD::EnergySumRoI>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;
  
  float METx  = container->exMiss();
  float METy  = container->eyMiss();
  std::vector<float> METet;
  std::vector<float> METphi;
  METet.push_back( sqrt(METx*METx + METy*METy) );
  if ( METx != 0. ) METphi.push_back( atan(METy/METx) );
  else              METphi.push_back( -5. );

  // decorate EventInfo
  Props::L1_METet.set(evtInfo, METet);
  Props::L1_METphi.set(evtInfo, METphi);
  
  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::L1_METet  );
    m_props.push_back( &Props::L1_METphi );
  }

  // all okay
  return true;

}


bool L1Jet::process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail) 
{

  // retrieve container from TEvent
  const xAOD::JetRoIContainer * container = 0;
  if ( ! retrieve<xAOD::JetRoIContainer>( event , container , allowTrigObjFail ) ) return false;
  else if ( ! container ) return true;

  // fill vectors
  const unsigned int N = container->size(); 
  std::vector<float> et8x8(N);
  std::vector<float> phi(N);
  std::vector<float> eta(N);
  for (unsigned int i = 0; i < N; ++i) {
    const xAOD::JetRoI * roi = container->at(i);
    et8x8[i] = roi->et8x8();
    phi[i]   = roi->phi();
    eta[i]   = roi->eta();
  }
  
  // decorate EventInfo 
  Props::L1_JetPt.set(evtInfo, et8x8);
  Props::L1_JetPhi.set(evtInfo, phi);
  Props::L1_JetEta.set(evtInfo, eta);

  // cashe pointers to decorators (for copying to output)
  static bool prepareCopy = true;
  if ( prepareCopy ) {
    prepareCopy = false;
    m_props.push_back( &Props::L1_JetPt  );
    m_props.push_back( &Props::L1_JetPhi );
    m_props.push_back( &Props::L1_JetEta );
  }

  // all okay
  return true;

}

