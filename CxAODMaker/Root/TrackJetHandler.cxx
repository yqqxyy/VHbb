#include <iostream>
//#include "xAODRootAccess/TStore.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODBTagging/BTagging.h"

#include "CxAODMaker/TrackJetHandler.h"
#include "CxAODMaker/EventInfoHandler.h"

TrackJetHandler::TrackJetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                             EventInfoHandler & eventInfoHandler) : 
  ObjectHandler(name, config, event, eventInfoHandler),
  m_isMC(0),
  m_storeGAParticlesInTrackJets(true)
{
  m_config.getif<bool>("TrackJet::UsedForLink", m_usedForElementLinks);
  m_config.getif<bool>("TrackJet::AddPreselJets", m_addPreselJets);
  bool m_useLowPt = false;
  m_config.getif<bool>("TrackJet::UseLowPt", m_useLowPt);
  
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind( &TrackJetHandler::passTrackJet, this, _1));

  // low pT track jets for HT_soft
  // selectFcns needs to be added second to override decisions from passTrackJet
  if( m_useLowPt )
    m_selectFcns.push_back(std::bind( &TrackJetHandler::passLowPtTrackJet, this, _1));
  //

  // set switch for storing Ghost associated heavy flavour hadrons
  m_config.getif<bool>("storeGAParticlesInTrackJets", m_storeGAParticlesInTrackJets);

}


TrackJetHandler::~TrackJetHandler() 
{

}

EL::StatusCode TrackJetHandler::initializeTools()
{

  m_isMC = m_eventInfoHandler.get_isMC();
  
  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear();

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode TrackJetHandler::decorate(xAOD::Jet * jet) 
{

  if(m_handlerName == "vrtrackJet") Props::TrkJetnumConstituents.set(jet,jet->numConstituents());

  //Props::passTaggingReq.set( jet , ( (jet->pt()>10000.0) && (fabs(jet->eta())<2.5) && (jet->numConstituents()>=2) ) ? 1 : 0 );

  // set btagging discriminants in the output
  EL_CHECK("TrackJetHandler::decorate()",retrieveBTaggingDiscriminants(jet));
  
  return EL::StatusCode::SUCCESS;
  
}


EL::StatusCode TrackJetHandler::calibrateCopies(xAOD::JetContainer * particles, const CP::SystematicSet & /*sysSet*/) 
{

  // loop over the particles in the container
  for (xAOD::Jet * jet : *particles) {
    //this ensures that the DynAux container has the same number of entries as the original one
    setP4( jet , jet );

    // print particle (pt,eta,phi,e,m)
    //std::cout << "===> Fat jet <===" << std::endl;
    //printParticle(jet);
    
    // apply calibrations

    // decorate jet
    if ( decorate( jet ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
    
  }

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode TrackJetHandler::retrieveBTaggingDiscriminants(xAOD::Jet * part) 
{
  const xAOD::BTagging * tagInfo = part->btagging();
  
  //double discriminant_mv1 = 0;
  //double discriminant_sv1_ip3d = 0;
  double discriminant_mv2c00=0;
  double discriminant_mv2c10=0;
  double discriminant_mv2c20=0;
  //double discriminant_mvb=0;
  double discriminant_mv2c100=0;
  double discriminant_mv2cl100=0;
  
  double discriminant_mv2c10mu=0;
  double discriminant_mv2c10rnn=0;
  
  double jet_dl1_pb=0;
  double jet_dl1_pc=0;
  double jet_dl1_pu=0;
  double jet_dl1mu_pb=0;
  double jet_dl1mu_pc=0;
  double jet_dl1mu_pu=0;
  double jet_dl1rnn_pb=0;
  double jet_dl1rnn_pc=0;
  double jet_dl1rnn_pu=0;
  double jet_dl1r_pb=0;
  double jet_dl1r_pc=0;
  double jet_dl1r_pu=0;
  
  if(tagInfo) {
    tagInfo->MVx_discriminant("MV2c00",discriminant_mv2c00);
    tagInfo->MVx_discriminant("MV2c10",discriminant_mv2c10);
    tagInfo->MVx_discriminant("MV2c20",discriminant_mv2c20);
    tagInfo->MVx_discriminant("MV2c100",discriminant_mv2c100);
    tagInfo->MVx_discriminant("MV2cl100",discriminant_mv2cl100);
    
    try {
      tagInfo->MVx_discriminant("MV2c10mu",discriminant_mv2c10mu);
      tagInfo->MVx_discriminant("MV2c10rnn",discriminant_mv2c10rnn);
    } catch(...) {}
    
    try {
      tagInfo->pb("DL1",jet_dl1_pb);
      tagInfo->pc("DL1",jet_dl1_pc);
      tagInfo->pu("DL1",jet_dl1_pu);
    } catch(...) {}
    try {
      tagInfo->pb("DL1mu",jet_dl1mu_pb);
      tagInfo->pc("DL1mu",jet_dl1mu_pc);
      tagInfo->pu("DL1mu",jet_dl1mu_pu);
    } catch(...) {}
    try {
      tagInfo->pb("DL1rnn",jet_dl1rnn_pb);
      tagInfo->pc("DL1rnn",jet_dl1rnn_pc);
      tagInfo->pu("DL1rnn",jet_dl1rnn_pu);
    } catch(...) {}      
    try {
      tagInfo->pb("DL1r",jet_dl1r_pb);
      tagInfo->pc("DL1r",jet_dl1r_pc);
      tagInfo->pu("DL1r",jet_dl1r_pu);
    } catch(...) {}


  }
  else{
    TString pTag = m_eventInfoHandler.get_pTag();
    if(m_eventInfoHandler.get_derivationName()=="HIGG5D2" && (pTag=="3372" || pTag=="3371" || pTag=="3374" || pTag=="3402" || pTag=="3358" || pTag=="3557" || pTag=="3560" || pTag=="3572" || pTag=="3571" || pTag=="3574")){
      Warning("TrackJetHandler::retrieveBTaggingDiscriminants","Couldn't retrieve b-tag info!");
    }else{
      Error("TrackJetHandler::retrieveBTaggingDiscriminants","Couldn't retrieve b-tag info!");
      return EL::StatusCode::FAILURE;    
    }
  }
  //Props::MV2c00.set(part, discriminant_mv2c00);
  Props::MV2c10.set(part, discriminant_mv2c10);
  //Props::MV2c20.set(part, discriminant_mv2c20);
  Props::MV2c100.set(part, discriminant_mv2c100);
  Props::MV2cl100.set(part, discriminant_mv2cl100);
  
  //Props::MV2c10mu.set(part, discriminant_mv2c10mu);
  //Props::MV2c10rnn.set(part, discriminant_mv2c10rnn);
  
  Props::DL1_pb.set(part, jet_dl1_pb);
  Props::DL1_pc.set(part, jet_dl1_pc);
  Props::DL1_pu.set(part, jet_dl1_pu);
  //Props::DL1mu_pb.set(part, jet_dl1mu_pb);
  //Props::DL1mu_pc.set(part, jet_dl1mu_pc);
  //Props::DL1mu_pu.set(part, jet_dl1mu_pu);
  //Props::DL1rnn_pb.set(part, jet_dl1rnn_pb);
  //Props::DL1rnn_pc.set(part, jet_dl1rnn_pc);
  //Props::DL1rnn_pu.set(part, jet_dl1rnn_pu);

  Props::DL1r_pb.set(part, jet_dl1r_pb);
  Props::DL1r_pc.set(part, jet_dl1r_pc);
  Props::DL1r_pu.set(part, jet_dl1r_pu);

  return EL::StatusCode::SUCCESS;    
}

bool TrackJetHandler::passTrackJet(xAOD::Jet* jet)
{
  bool passTrackCut=false;
  if(jet->pt()>7000){
    if(fabs(jet->eta()) <2.5){
      if(jet->numConstituents() >= 2){
	passTrackCut=true;
      }
    }
  }
  Props::isTrackJet.set(jet, passTrackCut);
  Props::passPreSel.set(jet, passTrackCut);

 return passTrackCut;
}

bool TrackJetHandler::passLowPtTrackJet(xAOD::Jet * jet)
{

  bool passLowPtTrackCut=false;
  if(jet->pt()>2000){
    if(fabs(jet->eta()) < 2.5){
      passLowPtTrackCut = true;
    }
  }
  Props::passPreSel.set(jet, passLowPtTrackCut);

  return passLowPtTrackCut;
}

EL::StatusCode TrackJetHandler::writeOutputVariables(xAOD::Jet* inJet, xAOD::Jet* outJet, bool isKinVar, bool isWeightVar, const TString& sysName)
{
  
  if (isKinVar || isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Yes
    EL_CHECK("TrackJetHandler::writeOutputVariables",writeAllVariations(inJet, outJet, sysName));
  }
  if (isKinVar && !isWeightVar) {
    // Nominal Not, Kinematic Yes, Weight Not
    EL_CHECK("TrackJetHandler::writeOutputVariables",writeKinematicVariations(inJet, outJet, sysName));
  } else if (!isKinVar && isWeightVar) {
    // Nominal Not, Kinematic Not, Weight Yes  
    EL_CHECK("TrackJetHandler::writeOutputVariables",writeWeightVariations(inJet, outJet, sysName));
  } else if (!isKinVar && !isWeightVar) {
    // Nominal Yes, Kinematic Not, Weight Not
    // for nominal we save all of them
    // in order to not copy paste code in both up and here in Nominal
    // and later to update one and forget the other
    // the functions were created to be easily read by name and edited 
    EL_CHECK("TrackJetHandler::writeOutputVariables",writeAllVariations(inJet, outJet, sysName));
    EL_CHECK("TrackJetHandler::writeOutputVariables",writeKinematicVariations(inJet, outJet, sysName));
    EL_CHECK("TrackJetHandler::writeOutputVariables",writeWeightVariations(inJet, outJet, sysName));
    EL_CHECK("TrackJetHandler::writeOutputVariables",writeNominal(inJet, outJet, sysName));
  } else assert(false);
  
  return writeCustomVariables(inJet, outJet, isKinVar, isWeightVar, sysName);
  
}

EL::StatusCode TrackJetHandler::writeAllVariations(xAOD::Jet*, xAOD::Jet*, const TString&)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TrackJetHandler::writeKinematicVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& /*sysName*/)
{
  // set four momentum
  outJet->setJetP4(inJet->jetP4());
  
  //
  Props::isTrackJet.copy(inJet, outJet);
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TrackJetHandler::writeWeightVariations(xAOD::Jet*, xAOD::Jet*, const TString&)
{
  //
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TrackJetHandler::writeNominal(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& /*sysName*/)
{
  //
  if(m_isMC) {
    Props::HadronConeExclTruthLabelID.copyIfExists(inJet, outJet);
  }
  
  //
  //Props::MV2c00.copy(inJet, outJet);
  Props::MV2c10.copy(inJet, outJet);
  //Props::MV2c10mu.copyIfExists(inJet, outJet);
  //Props::MV2c10rnn.copyIfExists(inJet, outJet);
  //if(m_saveNonCrucialVariables){
  //  Props::MV2c20.copy(inJet, outJet);
  //}
  Props::MV2c100.copy(inJet, outJet);
  Props::MV2cl100.copy(inJet, outJet);

  Props::DL1_pb.copyIfExists(inJet, outJet);
  Props::DL1_pc.copyIfExists(inJet, outJet);
  Props::DL1_pu.copyIfExists(inJet, outJet);
  //Props::DL1mu_pb.copyIfExists(inJet, outJet);
  //Props::DL1mu_pc.copyIfExists(inJet, outJet);
  //Props::DL1mu_pu.copyIfExists(inJet, outJet);
  //Props::DL1rnn_pb.copyIfExists(inJet, outJet);
  //Props::DL1rnn_pc.copyIfExists(inJet, outJet);
  //Props::DL1rnn_pu.copyIfExists(inJet, outJet);
  
  Props::DL1r_pb.copyIfExists(inJet, outJet);
  Props::DL1r_pc.copyIfExists(inJet, outJet);
  Props::DL1r_pu.copyIfExists(inJet, outJet);

  if(m_handlerName == "vrtrackJet"){
    Props::TrkJetnumConstituents.copy(inJet, outJet);
    Props::VRJetOverlap.copy(inJet, outJet);
  }
  //Props::passTaggingReq.copy(inJet, outJet);

  // ghost associated hadron labels
  if ( m_isMC && m_storeGAParticlesInTrackJets ) {
    const std::string labelB   = "ConeExclBHadronsFinal";//"GhostBHadronsFinal";
    const std::string labelC   = "ConeExclCHadronsFinal";//"GhostCHadronsFinal";
    const std::string labelTau = "GhostTausFinal";
    const std::string labelH   = "GhostHBosons";
    std::vector<const xAOD::TruthParticle *> ghostB;
    std::vector<const xAOD::TruthParticle *> ghostC;
    std::vector<const xAOD::TruthParticle *> ghostTau;
    std::vector<const xAOD::TruthParticle *> ghostH;
    // get input vectors from ghost-associated parent jet element link
    if ( ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelB   , ghostB   ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelC   , ghostC   ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelTau , ghostTau ) ||
         ! inJet->getAssociatedObjects<xAOD::TruthParticle>( labelH ,   ghostH ) )
      {
         Error("TrackJetHandler::writeNominal()","Couldn't access ghost associated heavy flavour hadrons! (You can turn off storing these hadrons via 'storeGAParticlesInTrackJets = false' in the config file)");
  return EL::StatusCode::FAILURE;
      }
    // make sure all IParticle pointers are valid (i.e. remove invalid pointers)
    // (are the invalid pointers a bug?)
    for (const xAOD::TruthParticle * part : ghostB  ) if ( ! part ) ghostB.erase( std::remove( std::begin(ghostB), std::end(ghostB), part ), std::end(ghostB) );
    for (const xAOD::TruthParticle * part : ghostC  ) if ( ! part ) ghostC.erase( std::remove( std::begin(ghostC), std::end(ghostC), part ), std::end(ghostC) );
    for (const xAOD::TruthParticle * part : ghostTau) if ( ! part ) ghostTau.erase( std::remove( std::begin(ghostTau), std::end(ghostTau), part ), std::end(ghostTau) );
    for (const xAOD::TruthParticle * part : ghostH  ) if ( ! part ) ghostH.erase( std::remove( std::begin(ghostH), std::end(ghostH), part ), std::end(ghostH) );

    // set output vectors
    // element links are broken in output; see JIRA CXAOD-211; removing for the moment
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelB   , ghostB   );
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelC   , ghostC   );
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelTau , ghostTau );
    // outJet->setAssociatedObjects<xAOD::TruthParticle>( labelH   , ghostH   );

    // count number of B- and C-Hadrons, excluding non-prompt decays
    int nGhostMatchedHBosons = ghostH.size();
    int nGhostMatchedBHadrons = ghostB.size();
    int nGhostMatchedPromptCHadrons = 0;
    // only counting prompt C-Hadrons
    for (const xAOD::TruthParticle * part : ghostC ) {
      if (!IsChild(part, ghostB)) nGhostMatchedPromptCHadrons++;
    }
    int nGhostMatchedTaus = ghostTau.size();


    Props::nGhostMatchedBHadrons.set(outJet, nGhostMatchedBHadrons);
    Props::nGhostMatchedPromptCHadrons.set(outJet, nGhostMatchedPromptCHadrons);
    if(m_saveNonCrucialVariables){
      Props::nGhostMatchedHBosons.set(outJet, nGhostMatchedHBosons);
      Props::nGhostMatchedTaus.set(outJet, nGhostMatchedTaus);
    }
  }

  //
  return EL::StatusCode::SUCCESS;
}

bool TrackJetHandler::IsChild(const xAOD::TruthParticle* child, std::vector<const xAOD::TruthParticle *> parents, std::vector<const xAOD::TruthParticle*> ancestors) {
  // child:     particle to be checked; starting point
  // parents:   list of parent particles
  // ancestors: lineage, starting from child, to avoid loops in some W+b samples
  // TODO:      move to CxAODTools / look if something like this exists in RootCore truth utils
  if (parents.size() == 0) return false;
  if (std::count(ancestors.begin(), ancestors.end(), child) > 0) return false;
  if (std::find(parents.begin(), parents.end(), child) != parents.end()) return true;
  ancestors.push_back(child);
  if (!child) return false;
  for (unsigned int j = 0; j < child->nParents(); j++) {
      if (IsChild(child->parent(j), parents, ancestors)) return true;
  }
  return false;
}

EL::StatusCode TrackJetHandler::writeCustomVariables(xAOD::Jet*, xAOD::Jet*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TrackJetHandler::clearEvent() 
{
  EL_CHECK("TrackJetHandler::clearEvent()",ObjectHandler::clearEvent());
  return EL::StatusCode::SUCCESS;

}

//Overload of base ObjectHandler::select() function
EL::StatusCode TrackJetHandler::select() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("TrackJetHandler::select()", "Called for handler '%s' (derived overload).", m_handlerName.c_str());
  }
  
  //--------------- Identical to ObjectHandler::select() -----------------
  // Get number of objects
  unsigned int nObjects = (*m_inContainer.begin()).second->size();

  // Loop on objects in container
  for (unsigned int objIndex = 0; objIndex < nObjects; ++objIndex) {
    // loop over all containers / systematic variations
    //return value is ignored
    for(auto& fcn : m_selectFcns) {
      passAny(m_inContainer, objIndex, fcn);
    }
  }
  //-----------------------------------------------------------------------

  //---------------------- VR Overlap -------------------------------
  if( m_handlerName == "vrtrackJet") EL_CHECK("TrackJetHandler::select()", FlagVRTrkJetOverlap());
  //-----------------------------------------------------------------

  return EL::StatusCode::SUCCESS;

}

//Overlap flag function for VR trackjets
EL::StatusCode TrackJetHandler::FlagVRTrkJetOverlap(){

  //Loop through VR Trackjets and check dR separation
  xAOD::JetContainer::iterator Iter1 = (*m_inContainer.begin()).second->begin();
  xAOD::JetContainer::iterator EndIter1 = (*m_inContainer.begin()).second->end();
  for( ;  Iter1 != EndIter1; ++Iter1 ){

    //Initialise all VR overlap flags if not set
    if( !Props::VRJetOverlap.exists(*Iter1) ){
      Props::VRJetOverlap.set((*Iter1), 0);
    }
    
    //Check jet 1 pT
    if( (*Iter1)->pt() < 5e3 ) { continue; }
    //Check jet 1 TrkJetNumConstituents
    if( (*Iter1)->numConstituents() < 2 ) { continue; }

    //Setup second iterator
    xAOD::JetContainer::iterator Iter2 = Iter1+1;
    xAOD::JetContainer::iterator EndIter2 = (*m_inContainer.begin()).second->end();
    for( ; Iter2 != EndIter2; ++Iter2){

      //Check jet 2 pT
      if( (*Iter2)->pt() < 5e3 ) { continue; }
      //Check jet 2 TrkJetNumConstituents
      if( (*Iter2)->numConstituents() < 2 ) { continue; }
      //Now calculate spatial dR value
      double jetR1 = std::max(0.02, std::min(0.4, 30. / ((*Iter1)->pt() / 1e3)));
      double jetR2 = std::max(0.02, std::min(0.4, 30. / ((*Iter2)->pt() / 1e3)));
      double deltaR = (*Iter1)->p4().DeltaR((*Iter2)->p4());
      if (deltaR < std::min(jetR1, jetR2)) {
	Props::VRJetOverlap.set((*Iter1), 1);
	Props::VRJetOverlap.set((*Iter2), 1);
      }


    }

  }
  
  //Return success - all good !!
  return EL::StatusCode::SUCCESS;
}
