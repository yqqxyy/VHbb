// Dear emacs, this is -*-c++-*-
#include "xAODEventInfo/EventInfo.h"
//#include "xAODCore/ShallowCopy.h"
#include "CxAODReader/ObjectReader.h"

template <>
std::pair<xAOD::EventInfo*, xAOD::ShallowAuxContainer*>
ObjectReader<xAOD::EventInfo>::createShallowCopy(
    const xAOD::EventInfo* container) {
  return xAOD::shallowCopyObject(*container);
}
