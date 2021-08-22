// Framework includes
#include "CxAODTools/OverlapRegisterAccessor.h"
#include "CxAODTools/ReturnCheck.h"

// EDM includes
#include "xAODRootAccess/TEvent.h"


OverlapRegisterAccessor::OverlapRegisterAccessor(AccessMode mode) : 
  m_mode                ( mode    ), 
  m_outOverlapRegCont   ( nullptr ),
  m_outOverlapRegAuxCont( nullptr ),
  m_inOverlapRegCont    ( nullptr )
{

}

EL::StatusCode OverlapRegisterAccessor::prepareRegister()
{

  // check mode
  if ( m_mode == READ ) {
    Error("OverlapRegisterAccessor::prepareRegister()","Cannot prepare register in READ mode");
    return EL::StatusCode::FAILURE;
  }

  // setup containers
  m_outOverlapRegCont    = new xAOD::OverlapRegisterContainer;
  m_outOverlapRegAuxCont = new xAOD::OverlapRegisterAuxContainer;
  m_outOverlapRegCont->setStore( m_outOverlapRegAuxCont );

  // all okay
  return EL::StatusCode::SUCCESS;

}


EL::StatusCode OverlapRegisterAccessor::fillRegister(const Containers & containers)
{

  // get containers
  const std::string             & variation = containers.variation;
  const xAOD::JetContainer      * jets      = containers.jets;
  const xAOD::JetContainer      * fatjets   = containers.fatjets;
  const xAOD::MuonContainer     * muons     = containers.muons;
  const xAOD::ElectronContainer * electrons = containers.electrons;
  const xAOD::TauJetContainer   * taus      = containers.taus;
  const xAOD::PhotonContainer   * photons   = containers.photons;

  // check mode
  if ( m_mode == READ ) {
    Error("OverlapRegisterAccessor::fillRegister()","Cannot fill register in READ mode");
    return EL::StatusCode::FAILURE;
  }

  // check if container is setup
  if ( ! m_outOverlapRegCont || ! m_outOverlapRegAuxCont ) {
    Error("OverlapRegisterAccessor::fillRegister()","OverlapRegister containers not setup");
    return EL::StatusCode::FAILURE;
  }
 
  // fill indices
  std::vector<unsigned int> goodJets;
  std::vector<unsigned int> goodFatJets;
  std::vector<unsigned int> goodMuons;
  std::vector<unsigned int> goodElectrons;
  std::vector<unsigned int> goodTaus;
  std::vector<unsigned int> goodPhotons;
  EL_CHECK("OverlapRegisterAccessor::fillRegister()", fill<xAOD::JetContainer>     ( jets      , goodJets      ));
  EL_CHECK("OverlapRegisterAccessor::fillRegister()", fill<xAOD::JetContainer>     ( fatjets   , goodFatJets   ));
  EL_CHECK("OverlapRegisterAccessor::fillRegister()", fill<xAOD::MuonContainer>    ( muons     , goodMuons     ));
  EL_CHECK("OverlapRegisterAccessor::fillRegister()", fill<xAOD::ElectronContainer>( electrons , goodElectrons ));
  EL_CHECK("OverlapRegisterAccessor::fillRegister()", fill<xAOD::TauJetContainer>  ( taus      , goodTaus      ));
  EL_CHECK("OverlapRegisterAccessor::fillRegister()", fill<xAOD::PhotonContainer>  ( photons   , goodPhotons   ));
  
  // create overlap register for this set of objects
  xAOD::OverlapRegister * overlapRegister = new xAOD::OverlapRegister;
  m_outOverlapRegCont->push_back( overlapRegister );
  overlapRegister->set_jets      ( goodJets      );
  overlapRegister->set_fatjets   ( goodFatJets   );
  overlapRegister->set_muons     ( goodMuons     );
  overlapRegister->set_electrons ( goodElectrons );
  overlapRegister->set_taus      ( goodTaus      );
  overlapRegister->set_photons   ( goodPhotons   );
  overlapRegister->set_systematic( variation     );

  // all okay
  return EL::StatusCode::SUCCESS;

} 


EL::StatusCode OverlapRegisterAccessor::recordRegister(xAOD::TEvent * event)
{

  // check mode
  if ( m_mode == READ ) {
    Error("OverlapRegisterAccessor::recordRegister()","Cannot write register in READ mode");
    return EL::StatusCode::FAILURE;
  }
  
  // check if container is setup
  if ( ! m_outOverlapRegCont || ! m_outOverlapRegAuxCont ) {
    Error("OverlapRegisterAccessor::recordRegister()","OverlapRegister containers not setup");
    return EL::StatusCode::FAILURE;
  }

  // record container
  if ( ! event->record( m_outOverlapRegCont , "OverlapRegister" ).isSuccess() ) {
    delete m_outOverlapRegCont;
    delete m_outOverlapRegAuxCont;
    m_outOverlapRegCont    = nullptr;
    m_outOverlapRegAuxCont = nullptr;
    return EL::StatusCode::FAILURE;
  }
  
  // record aux container
  if ( ! event->record( m_outOverlapRegAuxCont , "OverlapRegisterAux." ).isSuccess() ) {
    delete m_outOverlapRegAuxCont;
    m_outOverlapRegAuxCont = nullptr;
    return EL::StatusCode::FAILURE;
  }
 
  // TEvent has ownership, reset pointers
  m_outOverlapRegCont    = nullptr;
  m_outOverlapRegAuxCont = nullptr;

  // all okay
  return EL::StatusCode::SUCCESS;

}


EL::StatusCode OverlapRegisterAccessor::clearRegister() 
{
  
  // clear memory (if any)
  delete m_outOverlapRegCont;
  delete m_outOverlapRegAuxCont;
  m_outOverlapRegCont    = nullptr;
  m_outOverlapRegAuxCont = nullptr;
  
  // all okay
  return EL::StatusCode::SUCCESS;
  
}


EL::StatusCode OverlapRegisterAccessor::loadRegister(xAOD::TEvent * event)
{
      
  // check mode
  if ( m_mode == WRITE ) {
    Error("OverlapRegisterAccessor::loadRegister()","Cannot load register in WRITE mode");
    return EL::StatusCode::FAILURE;
  }
  
  // retrieve container from TEvent
  if ( ! event->retrieve( m_inOverlapRegCont , "OverlapRegister" ).isSuccess() ) {
    Error("OverlapRegisterAccessor::loadRegister()","Cannot retrieve container 'OverlapRegister' from TEvent");
    return EL::StatusCode::FAILURE;  
  } 

  // all okay
  return EL::StatusCode::SUCCESS;

}


EL::StatusCode OverlapRegisterAccessor::decorateObjects(const Containers & containers) const
{
      
  // get containers
  const std::string             & variation = containers.variation;
  const xAOD::JetContainer      * jets      = containers.jets;
  const xAOD::JetContainer      * fatjets   = containers.fatjets;
  const xAOD::MuonContainer     * muons     = containers.muons;
  const xAOD::ElectronContainer * electrons = containers.electrons;
  const xAOD::TauJetContainer   * taus      = containers.taus;
  const xAOD::PhotonContainer   * photons   = containers.photons;

  // check mode
  if ( m_mode == WRITE ) {
    Error("OverlapRegisterAccessor::decorateObjects()","Cannot decorate objects in WRITE mode");
    return EL::StatusCode::FAILURE;
  }
  
  // check if input container is loaded
  if ( ! m_inOverlapRegCont ) {
    Error("OverlapRegisterAccessor::decorateObjects()","Input container is not loaded");
    return EL::StatusCode::FAILURE;
  } 

  // find variation
  const xAOD::OverlapRegister * overlapReg = nullptr;
  for (const xAOD::OverlapRegister * reg : *m_inOverlapRegCont) {
    if ( reg->systematic() == variation ) {
      overlapReg = reg;
      break;
    }
  }
  
  // check if variation was found
  if ( ! overlapReg ) {
    Error("OverlapRegisterAccessor::decorateObjects()","Cannot find variation = %s", variation.c_str());
    return EL::StatusCode::SUCCESS;
  }

  // decorate objects
  const std::vector<unsigned int> & goodJets      = overlapReg->jets();
  const std::vector<unsigned int> & goodFatJets   = overlapReg->fatjets();
  const std::vector<unsigned int> & goodMuons     = overlapReg->muons();
  const std::vector<unsigned int> & goodElectrons = overlapReg->electrons();
  const std::vector<unsigned int> & goodTaus      = overlapReg->taus();
  const std::vector<unsigned int> & goodPhotons   = overlapReg->photons();
  EL_CHECK("OverlapRegisterAccessor::decorateObjects()", decorate<xAOD::JetContainer>     ( jets      , goodJets      ));
  EL_CHECK("OverlapRegisterAccessor::decorateObjects()", decorate<xAOD::JetContainer>     ( fatjets   , goodFatJets   ));
  EL_CHECK("OverlapRegisterAccessor::decorateObjects()", decorate<xAOD::MuonContainer>    ( muons     , goodMuons     ));
  EL_CHECK("OverlapRegisterAccessor::decorateObjects()", decorate<xAOD::ElectronContainer>( electrons , goodElectrons ));
  EL_CHECK("OverlapRegisterAccessor::decorateObjects()", decorate<xAOD::TauJetContainer>  ( taus      , goodTaus      ));
  EL_CHECK("OverlapRegisterAccessor::decorateObjects()", decorate<xAOD::PhotonContainer>  ( photons   , goodPhotons   ));

  // all okay
  return EL::StatusCode::SUCCESS;

}
  
std::ostream & OverlapRegisterAccessor::print(std::ostream & os) const 
{
  for (const xAOD::OverlapRegister * reg : *m_inOverlapRegCont) {
    os << *reg; 
  }
  os << std::endl;
  return os;
}
