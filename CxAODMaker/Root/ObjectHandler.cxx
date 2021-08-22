// Dear emacs, this is -*-c++-*-

#include "CxAODMaker/ObjectHandler.h"

#include "xAODRootAccess/TActiveStore.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODRootAccess/TStore.h"

template <class partContainer> 
ObjectHandler<partContainer>::ObjectHandler(const std::string& name, ConfigStore & config,
                                                     xAOD::TEvent * event, EventInfoHandler & eventInfoHandler) :
  ObjectHandlerBase(name, config, event),
  m_doCopyBeforeSyst(false),
  m_eventInfoHandler(eventInfoHandler),
  m_passProperty(&Props::passSel),
  m_selectFcns()
{
  using std::placeholders::_1;
  m_selectFcns.push_back(std::bind(&ObjectHandler< partContainer>::passPreSel, this, _1));
}

template <class partContainer> 
EL::StatusCode ObjectHandler<partContainer>::setObjects() 
{

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::setObjects()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  //retrieve input object container
  const partContainer * constParticles;
  if (!m_event->retrieve(constParticles, m_containerName).isSuccess()) {
    Error("ObjectHandler::setObjects()", ("Failed to retrieve particle container '" + m_containerName + "'").c_str());
    return EL::StatusCode::FAILURE;
  }
  // clear containers
  m_inContainer.clear();
  m_outContainer.clear();

  // check the initla object only when EventNumberToInvestigate is set
  if ( m_EventNumberToInvestigate > 0 ){
    if ( m_containerName != "TruthParticles" ){ // skip the TruthParticles container
      Info("ObjectHandler::setObjects()", "Check the particle container '%s'.", m_containerName.c_str());
      std::cout << std::setw(30) << "----"
        << std::setw(15) << "pt"
        << std::setw(16) << "eta"
        << std::setw(15) << "phi"
        << std::setw(15) << ""
        << std::endl;
      for (const partType * part : *(constParticles)) {
        std::cout << std::setw(30) << ""
          << std::setw(15) << part->pt()
          << std::setw(16) << part->eta()
          << std::setw(15) << part->phi()
          << std::setw(15) << ""
          << std::endl;
      }
    }
  }

  if (m_doCopyBeforeSyst) {
    std::pair<partContainer*, xAOD::ShallowAuxContainer *> shallowCopyCont = xAOD::shallowCopyContainer(*constParticles);

    xAOD::TStore * store = xAOD::TActiveStore::store();
    TString shallowCopyName = "ShallowInputInitial" + m_containerName;
    EL_CHECK("ObjectHandler::setObjects()", store->record( shallowCopyCont.first , (shallowCopyName).Data()));
    EL_CHECK("ObjectHandler::setObjects()", store->record( shallowCopyCont.second, (shallowCopyName + "Aux.").Data()));

    if (m_createElementLinks) {
	if(m_containerName.find("_BTagging")!= std::string::npos){
	    std::string containerName_noBTag = m_containerName.substr(0,m_containerName.find("_BTagging"));
	    const partContainer * constParticles_noBTag;
	    if (!m_event->retrieve(constParticles_noBTag, containerName_noBTag).isSuccess()) {
		Error("ObjectHandler::setObjects()", ("Failed to retrieve particle container '" + containerName_noBTag + "'").c_str());
		return EL::StatusCode::FAILURE;
	    }
	    xAOD::setOriginalObjectLink(*constParticles_noBTag,*(shallowCopyCont.first));
	} else {
	    xAOD::setOriginalObjectLink(*constParticles,*(shallowCopyCont.first));
	}
    }

    if ( decorateOriginParticles( shallowCopyCont.first ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
    constParticles = shallowCopyCont.first;
  }

  // here one can do things on the original container that will be independent on any systematic
  // they cannot *change* the contents of the objects, only augment them (xAOD EDM feature)
  // -------------------------------------------------------------------------------------------
  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::setObjects()", "decorateOriginParticle for handler '%s'.", m_handlerName.c_str());
  }

  for (const partType * part : *(constParticles)) {
    if ( decorateOriginParticle( part ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
  }

  // create shallow copies for all systematic variations including "Nominal" and "Original"
  // --------------------------------------------------------------------------------------
  // make list of containers
  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::setObjects()", "Get All Variations for handler '%s'.", m_handlerName.c_str());
  }

  std::vector<TString> listOfVariations = getAllVariations(true);
  
  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::setObjects()", "Finish Getting All Variations for handler '%s'.", m_handlerName.c_str());
  }

  // record containers in TEvent
  for (const TString & sysName : listOfVariations) {

    TString shallowCopyName = "ShallowInput" + sysName + m_containerName;

    if (m_msgLevel == MSG::DEBUG) {
      Info("ObjectHandler::setObjects()", "Variation '%s'.", sysName.Data());
    }
    
    std::pair<partContainer*, xAOD::ShallowAuxContainer *> shallowCopyCont = xAOD::shallowCopyContainer(*constParticles);
    m_inContainer[sysName] = shallowCopyCont.first;

    //Need this for MET which relies on element links
    if (m_createElementLinks) {
      xAOD::setOriginalObjectLink(*constParticles,*(shallowCopyCont.first));
    }

    // Record containers in active TStore.
    // We set the active TStore to point to EventLoop's TStore just before calling this method.
    // This means that the memory is managed for us and we shouldn't try and clean up ourselves.
    // clearEvent() below is disabled for this reason. 
    xAOD::TStore * store = xAOD::TActiveStore::store();
    EL_CHECK("ObjectHandler::setObjects()", store->record( shallowCopyCont.first , (shallowCopyName).Data()));
    EL_CHECK("ObjectHandler::setObjects()", store->record( shallowCopyCont.second, (shallowCopyName + "Aux.").Data()));
  }
  return EL::StatusCode::SUCCESS;
}

template <class partContainer> 
partContainer* ObjectHandler<partContainer>::getParticleVariation(
          std::map<TString, partContainer *> &container,
          const TString &variation)
{

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::getParticleVariation()", "Called for handler '%s' and variation '%s'.",
            m_handlerName.c_str(), variation.Data());
  }
  
    if (container.count(variation)) {
      return container[variation];
    }
    if (container.count("Nominal")) {
      return container["Nominal"];
    }
    return nullptr;
}

template <class partContainer> 
EL::StatusCode ObjectHandler<partContainer>::calibrate() 
{

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::calibrate()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  // TODO wrap systematic set + container into struct
  for (CP::SystematicSet sysSet : m_sysList) {
    TString sysName = sysSet.name();
    if (sysName == "") sysName = "Nominal";
    if (calibrateCopies(m_inContainer[sysName], sysSet) != EL::StatusCode::SUCCESS) {
      Error("ObjectHandler::calibrate()", "Could not successfully calibrate shallow copies for variation '" + sysName + "'!");
      return EL::StatusCode::FAILURE;
    }
  }
  
  // set four momentum and add decorations for original (non-calibrated) container
  if ( m_storeOrig ) {
    partContainer * container = m_inContainer["Original"];
    
    if ( ! container ) {
      Error("ObjectHandler::calibrate()","Couldn't find Original container among input containers!");
      return EL::StatusCode::FAILURE;
    }
    for (partType * part : *(container)) {
      setP4( part , part );
      decorate( part );
    }
  }
  
  return EL::StatusCode::SUCCESS;

}


template <class partContainer> 
bool ObjectHandler<partContainer>::passAny(std::map<TString, partContainer *> partContList,
						    int index,
						    std::function<bool(partType*)> passFunction)
{
  bool passAny = false;
  for (std::pair<TString,partContainer*> particles : partContList) {
    // TODO cutflow should be set somewhere else
    m_cutflow = m_cutflowMap[particles.first];
    passAny |= passFunction(particles.second->at(index));
  }
  return passAny;
}

template <class partContainer> 
bool ObjectHandler<partContainer>::passAny(std::map<TString, partContainer *> partContList,
						    int index,
						    PROP<int>& passProperty )
{
  bool passAny = false;
  for (std::pair<TString,partContainer*> particles : partContList) {
    passAny |= passProperty.get(particles.second->at(index));
  }
  return passAny;
}

template <class partContainer> 
void ObjectHandler<partContainer>::fillOutputContainer(partContainer * outPartCont, 
						    TString sysName)
{
  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::copyContainer()", "Called for handler '%s' and variation '%s'.",
            m_handlerName.c_str(), sysName.Data());
  }
  
  bool isKinVar = (sysName != "Nominal");
  bool isWeightVar = false;
  if( m_weightVariations.find(sysName) != m_weightVariations.end()) {
    isWeightVar = true;
    isKinVar = false;
  }
  
  unsigned int nParticles = getNInputParticles();
  // TODO w/o counter?
  int countOutParticles = 0;
  for (unsigned int partIndex = 0; partIndex < nParticles; partIndex++) {

    // check if particle passed the selection for any variation (nominal or systematic)
    bool passAnyVar = true;
    if (m_applyObjectSelection) {
      passAnyVar = passAny( m_inContainer, partIndex, *m_passProperty);
    }

    if (passAnyVar) {

      partType * inParticle = m_inContainer[sysName]->at(partIndex);

      partType * outParticle = 0;
      if (!(isKinVar || isWeightVar)) {
	// new particle for nominal
        outParticle = new partType();
        outPartCont->push_back(outParticle);
      } else {
	// particle exists already for shallow copies
	outParticle = outPartCont->at(countOutParticles);
      }
      
      //here it is decided which variables will be written to the output
      if(writeOutputVariables(inParticle, outParticle, isKinVar, isWeightVar, sysName) != EL::StatusCode::SUCCESS) {
	Error("ObjectHandler::copyContainer()","Failed to set output variables!");
      }

      // add particle indices to output
      if (!(isKinVar || isWeightVar)) {
        //m_indexDecorator.setPartIndex(outParticle,m_indexDecorator.getPartIndex(inParticle));
	Props::partIndex.set(outParticle, partIndex);
      }
      
      
      countOutParticles++;
    }
  }
}

template <class partContainer> 
unsigned int ObjectHandler< partContainer>::getNInputParticles() {
  unsigned int nParticles = 0;
  if(m_inContainer.size() > 0) {
    nParticles = (*m_inContainer.begin()).second->size();
  }
  return nParticles;
}

template <class partContainer> 
EL::StatusCode ObjectHandler< partContainer>::fillOutputContainer() 
{

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::fillOutputContainerCheck()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  // fill all nominal and syst particles that pass selection
  //-------------------------------------------------------------------------------

  // fill nominal container
  partContainer * outContainerNominal = new partContainer();
  xAOD::AuxContainerBase * outContainerNominalAux = new xAOD::AuxContainerBase();
  outContainerNominal->setStore(outContainerNominalAux);

  fillOutputContainer(outContainerNominal, "Nominal");
  // add container to output objects
  m_outContainer["Nominal"] = outContainerNominal;

  //necessary even if we do not want to write an xAOD!
  //because otherwise we cannot create shallow copies in the following
  //using a TStore for this purpose does not work currently (?)
  EL_CHECK("ObjectHandler::setObjects()", m_event->record(outContainerNominal, m_containerName + "___Nominal"));
  EL_CHECK("ObjectHandler::setObjects()", m_event->record(outContainerNominalAux, m_containerName + "___NominalAux."));

  // TODO move to separate method:
  if(m_usedForElementLinks) {
    if (m_msgLevel == MSG::DEBUG) {
      Info("ObjectHandler::fillOutputContainerCheck()", "Creating particle links for handler '%s'.", m_handlerName.c_str());
    }
    partContainer* inCont = m_inContainer["Nominal"];
    partContainer* outCont = m_outContainer["Nominal"];
    for(unsigned int i = 0; i < outCont->size(); ++i) {
      partType* outPart = outCont->at(i);
      int inIndex = Props::partIndex.get(outPart);
      partType* inPart = inCont->at(inIndex);
      m_linker->createParticleLink(*inPart, *outPart, *outCont);
    }
  }

  // create shallow copies for each syst variation + Original
  //---------------------------------------------------------
  std::vector<TString> listOfVariations = getAllVariations(false);
  
  // record containers in TEvent
  for (const TString & sysName : listOfVariations) {
    std::pair<partContainer *, xAOD::ShallowAuxContainer *> outContainerSC = xAOD::shallowCopyContainer(*outContainerNominal);

    fillOutputContainer(outContainerSC.first, sysName);
    // add container to output objects
    m_outContainer[sysName] = outContainerSC.first;

    //only necessary if we want to write an output xAOD
    EL_CHECK("ObjectHandler::setObjects()", m_event->record(outContainerSC.first, m_containerName + "___" + sysName.Data()));
    EL_CHECK("ObjectHandler::setObjects()", m_event->record(outContainerSC.second, m_containerName + "___" + sysName.Data() + "Aux."));
  }

  return EL::StatusCode::SUCCESS;

}

template<class partContainer>
bool ObjectHandler< partContainer>::passPreSel(partType* particle)
{
  Props::passPreSel.set(particle, true);
  return true;
}

template <class partContainer> 
bool ObjectHandler<partContainer>::checkPassSel(const partType* particle)
{
  bool pass = false;
  
  if (m_usedInOR) {
    // user global overlap removal flag
    if (Props::passORGlob.exists(particle)) {
      pass = Props::passORGlob.get(particle);
    } else {
      Warning("ObjectHandler::checkPassSel",
              "Accessor 'passORGlob' not available in handler '%s'", m_handlerName.c_str());
    }
  } else {
    // use preselection flag
    if (Props::passPreSel.exists(particle)) {
      pass = Props::passPreSel.get(particle);
    } else {
      Warning("ObjectHandler::checkPassSel",
              "Accessor 'passPreSel' not available in handler '%s'", m_handlerName.c_str());
    }
  }
  
  //This is pointless at this stage it seems becuase the m_links map is empty. 
  if (m_usedForElementLinks) {
    if (m_addPreselJets) {
      // do an or with particle being linked to
      pass |= m_linker->isLinkUsed(*particle, m_handlerName);
    }// else {       //With the changes to isLinkUsed and checkPassLinked, this is now redundant and results in all jets being set to pass = 0
     // // replace decision by particle being linked to
     // pass = m_linker->isLinkUsed(*particle, m_handlerName, true);
     //}
  }
  
  // decorate result to particle
  Props::passSel.set(particle, pass);
  
  return pass;
}

template <class partContainer> 
bool ObjectHandler<partContainer>::checkPassLinked(const partType* particle)
{
  // the passSel flag is required:
  bool pass = Props::passSel.get(particle);
  
  //Three possibilities for keep AntiKt2PV0, AntiKtVr30Max, and CoM subjets
  //    i)  Pass selection criteria irrelevant of whether the jet is linked to a large-R jet (fatjet)
  //   ii)  Linked and non-linked jets must pass selection criteria, but if linked jet fails then 
  //        remove from m_links std::map
  //  iii)  Non-linked jets must pass selection criteria, but linked do not.
  //
  //Implementation however only cares about two things:
  //      *) Whether to check each jet for a link
  //      *) Whether the jet passes selection criteria
  //
  //Because we save element links we need to correct m_links std::map in ParticleLinker.h
  //when removing a jet from its collection, as the link in the fatjet handler will be saved
  //but will point to an empty memory address.
  //
  //Solution:   Give ParticleLinker::isLinkUsed() the boolean decision if the jet passed the selection
  //            criteria. If not then the jet is failed and removed from m_links std::map.
  //            However, if m_usedForElementLinks is set, then keep the jet regardless.
  
  if( m_usedForElementLinks && m_addPreselJets ){
    //                     !!! OPTION iii) !!!
    pass |= m_linker->isLinkUsed(*particle, m_handlerName);
    
  }else if( m_usedForElementLinks && !m_addPreselJets ){
    //                     !!! OPTION ii) !!!
    pass = m_linker->isValid(*particle, m_handlerName, pass);
  }else{
    //                     !!! OPTION i) !!!
    /* !!!MUST STILL ALTER LINKS INCASE JET IS REMOVED!!! */
    bool dump;
    dump = m_linker->isValid(*particle, m_handlerName, pass);
    (void)dump;//Dummy cast to suppress compiler warnings. (No operation performed).
  }
  
  // decorate result to particle
  Props::passSel.set(particle, pass);
  
  //Return modified pass result
  return pass;
}

template <class partContainer> 
EL::StatusCode ObjectHandler<partContainer>::setPassSelFlags() {
  using std::placeholders::_1;
  std::function<bool(partType*)> passFunc = std::bind(
        &ObjectHandler<partContainer>::checkPassSel, this, _1);
  for (unsigned int partIndex = 0; partIndex < getNInputParticles(); partIndex++) {
    passAny(m_inContainer, partIndex, passFunc);
  }
  return EL::StatusCode::SUCCESS;
}

template <class partContainer> 
EL::StatusCode ObjectHandler<partContainer>::setPassSelForLinked() {
  
  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::setPassSelForLinked()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  using std::placeholders::_1;
  std::function<bool(partType*)> passFunc = std::bind(
        &ObjectHandler<partContainer>::checkPassLinked, this, _1);
  for (unsigned int partIndex = 0; partIndex < getNInputParticles(); partIndex++) {
    passAny(m_inContainer, partIndex, passFunc);
  }
  return EL::StatusCode::SUCCESS;
}

template <class partContainer>
EL::StatusCode ObjectHandler<partContainer>::decorateOriginParticle(const partType * /*particle*/)
{
  return EL::StatusCode::SUCCESS;
}

template <class partContainer>
EL::StatusCode ObjectHandler<partContainer>::decorateOriginParticles(partContainer * /*particle*/)
{
  return EL::StatusCode::SUCCESS;
}

template <class partContainer> 
EL::StatusCode ObjectHandler<partContainer>::clearEvent() {
  
  // Nothing to do here - memory handled by EventLoop's TStore
  return EL::StatusCode::SUCCESS;

}

template <class partContainer> 
void ObjectHandler<partContainer>::countObjects() 
{

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::countObjects()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  // count the number of objects in the input containers
  for (std::pair<TString, partContainer *> particleSet : m_inContainer) {

    // inclusive container size
    m_inObjectsCount[particleSet.first] += particleSet.second->size();

    // individual objects
    if ( m_EventNumberToInvestigate > 0 ){
      Info("ObjectHandler::countObjects()", "Check the passSel particle in '%s'.", m_containerName.c_str());
      std::cout << std::setw(30) << "----"
        << std::setw(15) << "pt"
        << std::setw(16) << "eta"
        << std::setw(15) << "phi"
        << std::setw(15) << ""
        << std::endl;
    }
    for (const partType* part: *particleSet.second) {
      // passPreSel
      if (Props::passPreSel.get(part)) {
        m_inObjectsPreSel[particleSet.first]++;
      }
      // passSel
      if (Props::passSel.get(part)) {
        m_inObjectsSelected[particleSet.first]++;
        if ( m_EventNumberToInvestigate > 0 ){
          std::cout << std::setw(30) << ""
                    << std::setw(15) << part->pt()
                    << std::setw(16) << part->eta()
                    << std::setw(15) << part->phi()
                    << std::setw(15) << ""
                    << std::endl;
        }
      }
    }
  }

  // count the number of objects in the output containers
  for (std::pair<TString, partContainer *> particleSet : m_outContainer) {
    // inclusive container size
    m_outObjectsCount[particleSet.first] += particleSet.second->size();
  }

}

template <class partContainer>
EL::StatusCode ObjectHandler< partContainer>::select() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::select()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  // get number of objects
  unsigned int nObjects = (*m_inContainer.begin()).second->size();

  // loop on objects in container
  for (unsigned int objIndex = 0; objIndex < nObjects; ++objIndex) {
    // loop over all containers / systematic variations
    //return value is ignored
    for(auto& fcn : m_selectFcns) {
      passAny(m_inContainer, objIndex, fcn);
    }
  }

  return EL::StatusCode::SUCCESS;

}

#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

template class ObjectHandler<xAOD::JetContainer>;
template class ObjectHandler<xAOD::MuonContainer>;
template class ObjectHandler<xAOD::ElectronContainer>;
template class ObjectHandler<xAOD::PhotonContainer>;
template class ObjectHandler<xAOD::TauJetContainer>;
template class ObjectHandler<xAOD::DiTauJetContainer>;
template class ObjectHandler<xAOD::TruthParticleContainer>;
