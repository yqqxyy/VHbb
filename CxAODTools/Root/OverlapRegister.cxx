// Framework includes
#include "CxAODTools/OverlapRegister.h"

// EDM includes
#include "xAODCore/AuxStoreAccessorMacros.h"

#include "xAODCore/AddDVProxy.h"

#include <iomanip>

namespace xAOD {


  //--------------------------------------
  //           Aux. element
  //--------------------------------------
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::vector<unsigned int> , jets       , set_jets       )
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::vector<unsigned int> , fatjets    , set_fatjets    )
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::vector<unsigned int> , muons      , set_muons      )
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::vector<unsigned int> , electrons  , set_electrons  )
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::vector<unsigned int> , taus       , set_taus       )
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::vector<unsigned int> , ditaus     , set_ditaus     )
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::vector<unsigned int> , photons    , set_photons    )
  AUXSTORE_OBJECT_SETTER_AND_GETTER( OverlapRegister , std::string               , systematic , set_systematic )


  //--------------------------------------
  //            Aux container
  //--------------------------------------
  OverlapRegisterAuxContainer::OverlapRegisterAuxContainer() :
  AuxContainerBase()
  {
    AUX_VARIABLE( jets       );
    AUX_VARIABLE( fatjets    );
    AUX_VARIABLE( muons      );
    AUX_VARIABLE( electrons  );
    AUX_VARIABLE( taus       );
    AUX_VARIABLE( ditaus     );
    AUX_VARIABLE( photons    );
    AUX_VARIABLE( systematic );
  }

  
  std::ostream & OverlapRegister::print(std::ostream & os) const {
    os << "OverlapRegister : " << systematic() << std::endl;
    
    const std::vector<unsigned int> & jets = this->jets();
    os << "  " << "jets      : " << std::setw(3) << jets.size() << " : ";
    for( int index : jets) {
      os << " " << index;
	}
    os << std::endl;
    
    const std::vector<unsigned int> & muons = this->muons();
    os << "  " << "muons     : " << std::setw(3) << muons.size() << " : ";
    for( int index : muons) {
      os << " " << index;
    }
    os << std::endl;

    const std::vector<unsigned int> & electrons = this->electrons();
    os << "  " << "electrons : " << std::setw(3) << electrons.size() << " : ";
    for( int index : electrons) {
      os << " " << index;
    }
    os << std::endl;

    const std::vector<unsigned int> & taus = this->taus();
    os << "  " << "taus      : " << std::setw(3) << taus.size() << " : ";
    for( int index : taus) {
      os << " " << index;
    }
    os << std::endl;

    const std::vector<unsigned int> & ditaus = this->ditaus();
    os << "  " << "ditaus      : " << std::setw(3) << ditaus.size() << " : ";
    for( int index : ditaus) {
      os << " " << index;
    }
    os << std::endl;

    const std::vector<unsigned int> & photons = this->photons();
    os << "  " << "photons   : " << std::setw(3) << photons.size() << " : ";
    for( int index : photons) {
      os << " " << index;
    }
    os << std::endl;
    return os;
  }
}

