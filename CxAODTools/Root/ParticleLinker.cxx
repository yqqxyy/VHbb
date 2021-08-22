#include "CxAODTools/ParticleLinker.h"
#include "xAODBase/IParticleHelpers.h"
#include <unordered_map>

//Default Constructor
ParticleLinker::ParticleLinker():
  m_debug(false)
{}

void ParticleLinker::createParticleLink(const xAOD::IParticle& inPart, ElementLink<DataVector<xAOD::IParticle>>& outPart)
{
  
  if(m_debug) Info("ParticleLinker::createParticleLink(inPart, Link)", "Linking Particles" );
  const xAOD::IParticle* orig = xAOD::getOriginalObject(inPart);
  m_links[orig] = outPart;
  if(m_debug) Info("ParticleLinker::createParticleLink(inPart, Link)", "         m_links.size() = %lu", m_links.size() );
}

void ParticleLinker::createParticleLink(const xAOD::IParticle& inPart, xAOD::IParticle& outPart, DataVector<xAOD::IParticle>& partCont)
{

  if(m_debug) Info("ParticleLinker::createParticleLink(inPart, outPart, partCont)", "Linking Particles");
  const xAOD::IParticle* orig = xAOD::getOriginalObject(inPart);
  m_links[orig] = ElementLink<DataVector<xAOD::IParticle>>(&outPart, partCont);

  if(m_debug) Info("ParticleLinker::createParticleLink(inPart, outPart, partCont)", "         m_links.size() = %lu", m_links.size());
}

void ParticleLinker::clear()
{
  m_links.clear();
}

bool ParticleLinker::isLinkUsed(const xAOD::IParticle& part, std::string handlerName)
{

  if(m_debug){
    Info("ParticleLinker::isLinkUsed()", "Checking Particle Link Utilisation on object %s",  handlerName.c_str());
  }

  //Find link between particle and another object to be stored in CxAOD.
  bool isLinked = ParticleLinker::findLink(part, true);
  
  if(m_debug){
    Info("ParticleLinker::isLinkUsed()", "            -> isLink %d", isLinked);
    Info("ParticleLinker::isLinkUsed()", "            -> pass %d", true);
  }
  
  return isLinked;
}

bool ParticleLinker::isValid(const xAOD::IParticle& part, std::string handlerName, bool pass)
{

  if(m_debug){
    Info("ParticleLinker::isLinkUsedAndValid()", "Checking Particle Link Utilisation on object %s",  handlerName.c_str());
  }
  //Find link between particle and another object to be stored in CxAOD.
  bool isLinkedAndValid = ParticleLinker::findLink(part, pass);
  if(m_debug){
    Info("ParticleLinker::isLinkUsedAndUsed()", "            -> isLink %d", isLinkedAndValid);
    Info("ParticleLinker::isLinkUsedAndUsed()", "            -> pass %d", pass);
  }

  //Spcial requirement for CoM jets - CoM jet must be linked at all costs.
  if( handlerName == "comtrackJet"){
    return isLinkedAndValid;
  }else{
    //Otherwise particle must either pass object selection or is a valid link 
    //(if user defines as requirements)
    return pass || isLinkedAndValid;
  }
}

bool ParticleLinker::findLink(const xAOD::IParticle& part, bool pass)
{
  //Extract original particle/object
  const xAOD::IParticle* orig = xAOD::getOriginalObject(part);
  //Check link storage map as to whether object/particle is linked to another object in event
  //E.g. trackjets to large-R jets.
  LinkMap::iterator itr = m_links.find(orig);
  
  //Check if the jet is linked to another object
  if (itr != m_links.end()) {
    if(m_debug){
      Info("ParticleLinker::findLink()", "            -> Link Used [m_links.size() = %lu]", m_links.size());
    }
    //Erase element if the link does not pass pre-selection
    if( !pass ){
      m_links.erase(itr);
    }
    if(m_debug){
      Info("ParticleLinker::findLink()", "            -> Link Used (post pass check) [m_links.size() = %lu]",
	   m_links.size());
    }
    return pass;
  }else{
    if(m_debug){
      Info("ParticleLinker::findLink()", "            -> Link Not Used [m_links.size() = %lu]", m_links.size());
    }
    return false;
  } 
  
}

//Add links to the outPart that refer to the same objects as the link in the inPart 
void ParticleLinker::copyParticleLinks(
        const xAOD::IParticle& inPart,
        xAOD::IParticle& outPart,
        const std::string& attrib_id,
	const std::string& handlerName) const {

  if(m_debug){
    Info("ParticleLinker::copyParticleLinks()", "Copying Particle Links" );
  }
  
  SG::AuxElement::Accessor<std::vector<ElementLink<DataVector<xAOD::IParticle>>>> acc(attrib_id);
  std::vector<ElementLink<DataVector<xAOD::IParticle>>> output;
  if (acc.isAvailable(inPart)) {
    const std::vector<ElementLink<DataVector<xAOD::IParticle>>>& input = acc(inPart);
    
    for(const auto& in_el : input) {
      //Get actual data pointer
      const xAOD::IParticle* inPartLinked = *(in_el.cptr());
      auto itr = m_links.find(inPartLinked);
      
      //Check if we found the particle and make new element link of type T (since it's stored as type IParticle)
      if (itr != m_links.end()) { 
        const ElementLink<DataVector<xAOD::IParticle>>& el = itr->second;
        auto out_el = ElementLink<DataVector<xAOD::IParticle>>(el.key(), el.index());
        output.push_back(out_el);
      } else {
	//Do not run for 'fatJet' handler, due to expected warnings from removal of linked
	//trackjets that do not pass object pre-selection
	if( handlerName != "fatJet"){
	  Warning("copyParticleLinks()", "Could not find matching particle.");
	}
      }
    }
  }
  acc(outPart) = output;
}

//Flag links which are used
void ParticleLinker::flagParticleLinks(
        const xAOD::IParticle& inPart,
        const std::string& attrib_id) {
  
  //Info("ParticleLinker::flagParticleLinks()", "Flag Particle Links" );
  SG::AuxElement::Accessor<std::vector<ElementLink<DataVector<xAOD::IParticle>>>> acc (attrib_id);
  if (acc.isAvailable(inPart)) {
    for(const auto& in_el : acc(inPart)) {
      m_links[*(in_el.cptr())] = nullptr;
    }
  }
  
  if(m_debug){
    Info("ParticleLinker::flagParticleLinks()", "         m_links.size() = %lu", m_links.size() );
  }
}
