// Dear emacs, this is -*-c++-*-
#ifndef CxAODReader_ObjectReaderBase_H
#define CxAODReader_ObjectReaderBase_H

#include <string>
#include <unordered_set>
#include "TTree.h"

#include "xAODRootAccess/TEvent.h"

class ObjectReaderBase {
 protected:
  // name of the container
  std::string m_containerName;
  // xAOD event
  xAOD::TEvent* m_event;
  std::unordered_set<std::string> m_variations;
  bool m_createShallowCopies;
  bool m_haveNominal;
  bool m_nominalWasRead;

 public:
  ObjectReaderBase(const std::string& containerName, xAOD::TEvent* event);
  virtual ~ObjectReaderBase() { ; }
  virtual void discoverVariations(TTree* collectionTree,
                                  bool checkContainer = true) = 0;
  const std::unordered_set<std::string>& getVariations() { return m_variations; }
  const std::string& getContainerName() { return m_containerName; }
  void setVariations(const std::unordered_set<std::string>& variations) {
    m_variations = variations;
  }
  void setCreateShallowCopies(bool create) { m_createShallowCopies = create; }
  virtual void clearEvent();
};

#endif
