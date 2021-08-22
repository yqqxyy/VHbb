#ifndef CxAODTools__OverlapRegisterAccessor_h
#define CxAODTools__OverlapRegisterAccessor_h

/**
 * DESCRIPTION :
 *
 * OverlapRegisterAccessor provides an interface for xAOD::OverlapRegister
 * (defined in CxAODTools/OverlapRegister.h). 
 *
 * For each systematic variation, the overlap removal decisions for a set 
 * of containers (jets, muons, electrons, taus and photons) are stored in
 * an xAOD::OverlapRegister. These xAOD::OverlapRegister's are put in an
 * xAOD::OverlapRegisterContainer (i.e. DataVector<xAOD::OverlapRegister>) 
 * which is recorded in TEvent along it's corresponding aux container
 * (xAOD::OverlapRegisterAuxContainer) which carries the payload. 
 *
 * The register allows the user to store/retrieve the overlap removal 
 * decision for a given object for a given systematic variation.
 *
 * OverlapRegisterAccessor is instatiated in WRITE or READ mode, depending
 * on whether the user wants to write overlap removal decisions from the 
 * objects to TEvent or read back the OverlapRegister from TEvent and 
 * decorate the objects with the decisions. In both cases the code uses
 * the decoration called 'passOR'.
 *
 * 
 * USAGE in EventLoop:
 * 
 *
 * WRITE mode:
 * 
 * - Declare the OverlapRegisterAccessor pointer in the header of the 
 *   EL::Algorithm.
 *   
 *     OverlapRegisterAccessor * m_overlapRegAcc; //!
 *
 * - In initialize(), the OverlapRegisterAccessor is instantiated in WRITE 
 *   mode
 *   
 *     m_overlapRegAcc = new OverlapRegisterAccessor(OverlapRegisterAccessor::WRITE);
 * 
 * - In execute() the register is first prepared, i.e. the output 
 *   containers are declared and connected.
 *
 *     m_overlapRegAcc->prepareRegister();
 *
 * - then, after the OverlapRemoval tool is called, the register is filled
 *   for each systematic variation
 *
 *     OverlapRegisterAccessor::Containers containers;
 *     containers.variation = variation;
 *     containers.jets      = jets;
 *     containers.fatjets   = fatjets;
 *     containers.muons     = muons;
 *     containers.eletrons  = electrons;
 *     //containers.taus    = taus;
 *     //containers.photons = photons;
 *     m_overlapRegAcc->fillRegister( containers );
 * 
 *   where 'variation' is a std::string and jets, muons, ... are xAOD
 *   containers (for which the objects have already been decorated with 
 *   integer 'passOR' by the OverlapRemoval tool). Only the indices for 
 *   surviving objects are stored in the register.
 *
 * - if the event is to be written out, the OverlapRegister containers
 *   are recorded in TEvent with
 *
 *     m_overlapRegAcc->recordRegister(m_event);
 *
 *   where m_event is the TEvent pointer.
 *
 * - at the end of execute(), the memory allocations are deleted with
 *
 *     m_overlapRegAcc->clearRegister();
 *
 *   if the containers were recorded in TEvent, this call has no effect 
 *   (so it's safe to always make the call).
 *
 *   The output CxAOD will have to containers written to it:
 *   "OverlapRegister"
 *   "OverlapRegisterAux."
 *
 *
 * READ mode:
 *
 * - Declare the OverlapRegisterAccessor pointer in the header of the 
 *   EL::Algorithm.
 *   
 *     OverlapRegisterAccessor * m_overlapRegAcc; //!
 *
 * - In initialize(), the OverlapRegisterAccessor is instantiated in READ 
 *   mode
 *   
 *     m_overlapRegAcc = new OverlapRegisterAccessor(OverlapRegisterAccessor::READ);
 * 
 * - In execute() the register is first loaded, i.e. the containers are 
 *   retrieved from TEvent.
 *
 *     m_overlapRegAcc->loadRegister(m_event);
 *
 *   where m_event is the TEvent pointer.
 *
 * - the overlap removal decision is decorated onto the objects for a
 *   given systematic variations with
 *
 *     OverlapRegisterAccessor::Containers containers;
 *     containers.variation = variation;
 *     containers.jets      = jets;
 *     containers.fatjets   = fatjets;
 *     containers.muons     = muons;
 *     containers.eletrons  = electrons;
 *     //containers.taus    = taus;
 *     //containers.photons = photons;
 *     m_overlapRegAcc->decorateObjects( containers );
 * 
 *   where 'variation' is a std::string and jets, muons, ... are xAOD
 *   containers. All objects will be decorated with integer 'passOR'.
 *
 */



// EDM includes
#include "EventLoop/StatusCode.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODEgamma/PhotonContainer.h"

// Framewok includes
#include "CxAODTools/OverlapRegister.h"

// stl includes
#include <string>
#include <vector>

// forward declarations
namespace xAOD {
  class TEvent;
}


class OverlapRegisterAccessor {
  
public:

  // access mode
  enum AccessMode {
    WRITE,
    READ
  };

  // struct for containers
  struct Containers {
  public:
    Containers() : 
      variation("NONE"),
      jets     (nullptr),
      fatjets  (nullptr),
      muons    (nullptr),
      electrons(nullptr),
      taus     (nullptr),
      ditaus   (nullptr),
      photons  (nullptr)
    {}
    std::string               variation;
    const xAOD::JetContainer      * jets;
    const xAOD::JetContainer      * fatjets;
    const xAOD::MuonContainer     * muons;
    const xAOD::ElectronContainer * electrons;
    const xAOD::TauJetContainer   * taus;
    const xAOD::DiTauJetContainer * ditaus;
    const xAOD::PhotonContainer   * photons;  
  };

  // constructor
  OverlapRegisterAccessor(AccessMode mode);

  // setup output containers 
  EL::StatusCode prepareRegister();

  // read OR decision from objects and fill register
  EL::StatusCode fillRegister(const Containers & containers);

  // record register in TEvent
  EL::StatusCode recordRegister(xAOD::TEvent * event);

  // clear memory allocations if register not recorded
  EL::StatusCode clearRegister();

  // read OverlapRegister from TEvent
  EL::StatusCode loadRegister(xAOD::TEvent * event);

  // decorate objects with overlap removal decision
  EL::StatusCode decorateObjects(const Containers & containers) const;
  
  std::ostream & print(std::ostream &) const; // pretty print
private:
 
  // access mode
  const AccessMode m_mode;

  // overlap register and aux: non-const for write mode, const for read mode
  xAOD::OverlapRegisterContainer       * m_outOverlapRegCont;
  xAOD::OverlapRegisterAuxContainer    * m_outOverlapRegAuxCont;
  const xAOD::OverlapRegisterContainer * m_inOverlapRegCont;

  // fill vector with indices to particles which pass overlap removal
  template<typename partContainer> 
  EL::StatusCode fill(const partContainer * particles, std::vector<unsigned int> & goodParticles);

  // decorate particles with 'passOR'
  template<typename partContainer> 
  EL::StatusCode decorate(const partContainer * particles, const std::vector<unsigned int> & goodParticles) const;

};

inline std::ostream & operator<<(std::ostream & os, const OverlapRegisterAccessor & ORA) {
  return ORA.print(os);
}
#include "CxAODTools/OverlapRegisterAccessor.icc"


#endif
