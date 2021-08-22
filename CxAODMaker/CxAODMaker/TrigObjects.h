#ifndef CxAODMaker_TrigObjects_h
#define CxAODMaker_TrigObjects_h

// Framework includes
#include "CxAODTools/ObjectDecorator.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODRootAccess/TEvent.h"

// DESCRIPTION
// ...


// Base class to provide shared functionality
class TrigObjects {
    
public:

  // constructor
  TrigObjects(const std::string & containerName, const std::string & name) : m_containerName(containerName), m_name(name) {}

  // destructor
  virtual ~TrigObjects() {}

  // This method does the heavy lifting... needs to be implemented by derived class!
  virtual bool process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail = false) = 0;

  // copy properties from input to output
  bool copy(const xAOD::EventInfo * in, xAOD::EventInfo * out, bool allowTrigObjFail = false);

  // get class name
  const std::string & name() const { return m_name; }
  

protected:

  // retrieve container from TEvent
  template <typename containerType>
  bool retrieve(xAOD::TEvent * event, const containerType *& container, bool allowTrigObjFail = false);
  
  // vector of pointers to decorators - filled in process
  std::vector<PROP<std::vector<float> > *> m_props;

  // name of xAOD container
  const std::string m_containerName;

  // class identifyer
  const std::string m_name;
  
};


// template definitions
#include "CxAODMaker/TrigObjects.icc"


// preprocessor macro to define trigger object classes
#define TRIGOBJECTS(className) \
  class className : public TrigObjects { \
  public: \
  className(const std::string & containerName) : TrigObjects(containerName, #className) { Info("TrigObjects::TrigObjects()", "Instantiating '%s'", name().c_str()); } \
  virtual bool process(xAOD::TEvent * event, const xAOD::EventInfo * evtInfo, bool allowTrigObjFail); \
  }; 


// define trigger object classes
TRIGOBJECTS( HLTJet ) 
TRIGOBJECTS( HLTElectron ) 
TRIGOBJECTS( HLTMuon ) 
TRIGOBJECTS( HLTPhoton ) 
TRIGOBJECTS( HLTMet ) 
TRIGOBJECTS( L2Photon ) 
TRIGOBJECTS( L1EM ) 
TRIGOBJECTS( L1Met ) 
TRIGOBJECTS( L1Jet ) 


#endif
