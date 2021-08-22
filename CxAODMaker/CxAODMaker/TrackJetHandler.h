// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_TrackJetHandler_H
#define CxAODMaker_TrackJetHandler_H

#include "CxAODMaker/ObjectHandler.h"

#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"


class TrackJetHandler : public ObjectHandler<xAOD::JetContainer> {

public:

  TrackJetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);

  ~TrackJetHandler();

  EL::StatusCode initializeTools() override;
  EL::StatusCode clearEvent() override;  

  void setP4(const xAOD::Jet * in, xAOD::Jet * out) override { out->setJetP4( in->jetP4() ); }
  
protected:
  bool m_isMC;
  bool m_storeGAParticlesInTrackJets;
  
  // selection functions
  
  bool passTrackJet(xAOD::Jet* jet);
  bool passLowPtTrackJet(xAOD::Jet* jet);

  EL::StatusCode retrieveBTaggingDiscriminants(xAOD::Jet * jet);

  bool IsChild(const xAOD::TruthParticle* child, std::vector<const xAOD::TruthParticle *> parents, std::vector<const xAOD::TruthParticle *> ancestors = {});
  
  EL::StatusCode decorate(xAOD::Jet * jet) override;

  EL::StatusCode calibrateCopies(xAOD::JetContainer * particles, const CP::SystematicSet & sysSet) override;
  
  EL::StatusCode FlagVRTrkJetOverlap();

  virtual EL::StatusCode writeOutputVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeAllVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeKinematicVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode select() override;
  

};

#endif
