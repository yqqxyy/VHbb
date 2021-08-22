// HEADER INCLUDE
#include "CxAODTools/EvtSelTruthAllocator.h"
#include "CxAODTools/xAODBaseContainerStore.h"
#include "CxAODTools/xAODContainerStore.h"

//xAOD CONTAINER INCLUDES
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"

//Class Constructor
EvtSelTruthAllocator::EvtSelTruthAllocator()
{
  //Construct the pairings of truth particle PDF IDs and Reco. containers
  m_RecoTruthIDMapping[11] = new xAODContainerStore<xAOD::ElectronContainer>();// Option 1b)
  m_RecoTruthIDMapping[13] = new xAODContainerStore<xAOD::MuonContainer>();      // Option 1b)
  m_RecoTruthIDMapping[15] = new xAODContainerStore<xAOD::TauJetContainer>();    // Option 1b)
  m_RecoTruthIDMapping[22] = new xAODContainerStore<xAOD::PhotonContainer>();      // Option 1b)
  m_RecoTruthIDMapping[12] = new xAODContainerStore<xAOD::MissingET>();          // Option 1b)
  m_RecoTruthIDMapping[14] = new xAODContainerStore<xAOD::MissingET>();          // Option 1b)
  m_RecoTruthIDMapping[16] = new xAODContainerStore<xAOD::MissingET>();          // Option 1b)
  m_RecoTruthIDMapping[0] = new xAODContainerStore<xAOD::JetContainer>();       // Option 1b)

  //Construct the dR cone definitions for dR matching between reco object and truth object
  m_dRRecoTruthConeWidth.insert(std::make_pair<xAODContainerStore<xAOD::ElectronContainer>*, float>( new xAODContainerStore<xAOD::ElectronContainer>(), 0.2 ) );
  m_dRRecoTruthConeWidth.insert(std::make_pair<xAODContainerStore<xAOD::MuonContainer>*, float>( new xAODContainerStore<xAOD::MuonContainer>(), 0.2 ) );
  m_dRRecoTruthConeWidth.insert(std::make_pair<xAODContainerStore<xAOD::TauJetContainer>*, float>( new xAODContainerStore<xAOD::TauJetContainer>(), 0.2 ) );

}

//Member Method (Master) : Explicit definition of TLV extraction for jet containers
TLorentzVector EvtSelTruthAllocator::GetTLV( const xAOD::Jet *jet ){ 
  //Check for attribute before performing extraction
  //if( !jet->getAttribute("pt") ){
  //  TLorentzVector TLV;
  //  TLV.SetPtEtaPhiM(0.0, 0.0, 0.0, 0.0);
  //  return TLV;
  //}else{
    xAOD::JetFourMom_t Jet4Vec = jet->jetP4();
    TLorentzVector TLV;
    TLV.SetPtEtaPhiM(Jet4Vec.Pt(), Jet4Vec.Eta(), Jet4Vec.Phi(), Jet4Vec.M());
    return TLV;
    //}
}
