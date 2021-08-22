// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools__xAODContainerStore_H
#define CxAODTools__xAODContainerStore_H

#include "CxAODTools/xAODBaseContainerStore.h"

//xAOD Container includes
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETContainer.h"

//C++ 11 library includes
#include <typeinfo>
#include <iostream>


//Storage class of xAOD:: Reco object containers in order to allow the storage of multiple
//types within a single map
template<typename RecoType> 
class xAODContainerStore : public xAODBaseContainerStore
{
  
public:
  
  //Class constructor
  xAODContainerStore();

  //Class deconstructor
  ~xAODContainerStore() = default;
  
  //Method (Master) : Accessor function
  //RecoType GetContainer(){ return m_container; } 
  std::string GetContainerType(){ return typeid(m_container).name(); } 
  
  //===================== Data Members ==================
  RecoType m_container;
  //====================================================
  
};

//************************** Specialisation : JetContainer ********************
//Explicit instantiations of used template classes - Force compilation of templated xAOD::JetContainer class
template class xAODContainerStore<xAOD::ElectronContainer>;
template class xAODContainerStore<xAOD::MuonContainer>;
template class xAODContainerStore<xAOD::TauJetContainer>;
template class xAODContainerStore<xAOD::PhotonContainer>;
template class xAODContainerStore<xAOD::MissingET>;
template class xAODContainerStore<xAOD::MissingETContainer>;
template class xAODContainerStore<xAOD::JetContainer>;
//*****************************************************************************

#include "CxAODTools/xAODContainerStore.icc"

#endif
