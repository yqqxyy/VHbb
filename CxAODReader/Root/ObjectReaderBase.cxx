// Dear emacs, this is -*-c++-*-
#include "CxAODReader/ObjectReaderBase.h"

ObjectReaderBase::ObjectReaderBase(const std::string& containerName,
                                   xAOD::TEvent* event)
    : m_containerName(containerName),
      m_event(event),
      m_createShallowCopies(false),
      m_haveNominal(true),
      m_nominalWasRead(false) {}

void ObjectReaderBase::clearEvent() { m_nominalWasRead = false; }
