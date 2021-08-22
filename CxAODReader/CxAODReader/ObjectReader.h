// Dear emacs, this is -*-c++-*-
#ifndef CxAODReader_ObjectReader_H
#define CxAODReader_ObjectReader_H

class TTree;
#include "CxAODReader/ObjectReaderBase.h"

#ifndef __MAKECINT__

class EventInfo;
#include "xAODCore/ShallowCopy.h"

#endif  // not __MAKECINT__

template <class partContainer>
class ObjectReader : public ObjectReaderBase {
 protected:
   // cache the most useful container to speed up reading the CxAODs.
   const partContainer *m_nominalContainer;

#ifndef __MAKECINT__
  std::pair<partContainer*, xAOD::ShallowAuxContainer*> createShallowCopy(
      const partContainer* container);
#endif  // not __MAKECINT__

 public:
  ObjectReader(const std::string& name, xAOD::TEvent* event);
  virtual ~ObjectReader() {}
  virtual void discoverVariations(TTree* collectionTree,
                                  bool checkContainer = true) override;
  virtual void clearEvent() override;

#ifndef __MAKECINT__
  const partContainer* getObjects(std::string variation);
#endif  // not __MAKECINT__
};

template <>
std::pair<xAOD::EventInfo*, xAOD::ShallowAuxContainer*>
ObjectReader<xAOD::EventInfo>::createShallowCopy(
    const xAOD::EventInfo* container);

//=============================================================================
// Define the implementation of the methods here in the header file.
// This is done since we are dealing with a templated base class!
//=============================================================================

#ifndef __MAKECINT__
#include "CxAODReader/ObjectReader.icc"
#endif  // not __MAKECINT__

#endif
