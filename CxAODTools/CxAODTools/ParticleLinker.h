#ifndef CXAODTOOLS_PARTICLELINKER_H
#define CXAODTOOLS_PARTICLELINKER_H

#ifndef __MAKECINT__

#include <unordered_map>
#include <map>
#include "xAODBase/IParticle.h"
#include "AthLinks/ElementLink.h"
#include "AthContainers/DataVector.h"

/** ParticleLinker class 
 *  This class stores a map between input xAOD particles and an element linkto the corresponding 
 *  output CxAOD particles. It can then copy a link or vector of links from an input to an output
 *  particle, swapping the original linked particles for there CxAOD equivalents **/


class ParticleLinker
{
public:

  //Default Constructor
  ParticleLinker();

  //Typedef of map for linking particle and element link
  typedef std::unordered_map<const xAOD::IParticle*, ElementLink<DataVector<xAOD::IParticle>>> LinkMap;
  
  //Store link between the two particles
  void createParticleLink(const xAOD::IParticle& inPart, ElementLink<DataVector<xAOD::IParticle>>& outPart);
  void createParticleLink(const xAOD::IParticle& inPart, xAOD::IParticle& outPart, DataVector<xAOD::IParticle>& partCont);
  
  //Copy element links stored with id attrib_id from inPart to outPart. Works for ElementLink<DataVector<T>> or some vector<ElementLink<DataVector<T>>> 
  //to arbitrary depth of vectors
  void copyParticleLinks(const xAOD::IParticle& inPart, xAOD::IParticle& outPart, const std::string& attrib_id, const std::string& handlerName) const;
  
  //Clear all stored links (used at end of each event)
  void clear();
  
  //Functions used to determine if links exist and depending on the call either let m_links contain element
  //link or remove it when object is removed due to a failed pre-selection
  bool findLink(const xAOD::IParticle& part, bool pass);
  bool isValid(const xAOD::IParticle& part, std::string handlerName = "NoHandlerAssigned", bool pass = true);
  bool isLinkUsed(const xAOD::IParticle& part, std::string handlerName = "NoHandlerAssigned");
  
  void flagParticleLinks(const xAOD::IParticle& part, const std::string& attrib_id);
  void SetDebug(bool debug){ m_debug = debug; }
  
 private:
  LinkMap m_links;
  bool m_debug;
};

#endif

#endif

