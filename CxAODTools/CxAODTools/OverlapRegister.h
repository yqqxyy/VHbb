#ifndef CxAODTools__OverlapRegister_H
#define CxAODTools__OverlapRegister_H


// EDM includes
#include "AthContainers/DataVector.h"
#include "AthContainers/AuxElement.h"
#include "xAODCore/AuxContainerBase.h"

// stl includes
#include <iostream>
#include <vector>
#include <string>


namespace xAOD {
  

  //--------------------------------------
  //           Aux. element
  //--------------------------------------
  class OverlapRegister : public SG::AuxElement {
    
  public:
    
    // default constructor
    OverlapRegister() : SG::AuxElement() {}
    
    // default destructor
    virtual ~OverlapRegister() {}
    
    // get objects
    const std::vector<unsigned int> & jets       () const; 
    const std::vector<unsigned int> & fatjets    () const; 
    const std::vector<unsigned int> & muons      () const; 
    const std::vector<unsigned int> & electrons  () const; 
    const std::vector<unsigned int> & taus       () const;
    const std::vector<unsigned int> & ditaus     () const; 
    const std::vector<unsigned int> & photons    () const; 
    const std::string               & systematic () const; 

    // set objects
    void set_jets       (const std::vector<unsigned int> & jets     );
    void set_fatjets    (const std::vector<unsigned int> & fatjets  );
    void set_muons      (const std::vector<unsigned int> & muons    ); 
    void set_electrons  (const std::vector<unsigned int> & electrons); 
    void set_taus       (const std::vector<unsigned int> & taus     );
    void set_ditaus     (const std::vector<unsigned int> & ditaus   );
    void set_photons    (const std::vector<unsigned int> & photons  ); 
    void set_systematic (const std::string               & systematic);

    // pretty print contents
    std::ostream & print(std::ostream & os) const;
  };
  

  //--------------------------------------
  //             Container
  //--------------------------------------
  typedef DataVector<xAOD::OverlapRegister> OverlapRegisterContainer;


  //--------------------------------------
  //            Aux container
  //--------------------------------------
  class OverlapRegisterAuxContainer : public AuxContainerBase {
    
  public:
    
    // default constructor
    OverlapRegisterAuxContainer();
    
  private:
    
    // indices to "good" objects (i.e. not overlap removed).
    // this is basically a look-up table, i.e a vector per
    // systematic, and for each systematic a vector of indices
    // to surviving objects, where the index is "partIndex"
    std::vector<std::vector<unsigned int> > jets;
    std::vector<std::vector<unsigned int> > fatjets;
    std::vector<std::vector<unsigned int> > muons;
    std::vector<std::vector<unsigned int> > electrons;
    std::vector<std::vector<unsigned int> > taus;
    std::vector<std::vector<unsigned int> > ditaus;
    std::vector<std::vector<unsigned int> > photons;
    std::vector<std::string>                systematic;

  };

}

inline std::ostream & operator<<(std::ostream & os, const xAOD::OverlapRegister & OR) {
  return OR.print(os);
}

// Dictonaries
// generated with 'clid -m MyClassName'
CLASS_DEF( xAOD::OverlapRegister , 175584459 , 1 )
CLASS_DEF( xAOD::OverlapRegisterContainer , 1131101725 , 1 )
CLASS_DEF( xAOD::OverlapRegisterAuxContainer , 1252586608 , 1 )


#endif
