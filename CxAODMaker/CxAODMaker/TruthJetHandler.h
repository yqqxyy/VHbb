// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_TruthJetHandler_H
#define CxAODMaker_TruthJetHandler_H

#include "CxAODMaker/ObjectHandler.h"
#include "CxAODMaker/MatchJet.h"

#include "xAODJet/JetContainer.h"

class TruthJetHandler : public ObjectHandler<xAOD::JetContainer> {

public:

  TruthJetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);

  ~TruthJetHandler();

  EL::StatusCode initializeTools() override;
  EL::StatusCode clearEvent() override;  
  
  void setP4(const xAOD::Jet * in, xAOD::Jet * out) override { out->setJetP4( in->jetP4() ); }

protected:
  bool m_isMC;
  bool m_storeTruthJet4Vector;

  // selection functions
  
  bool passTruthJet(xAOD::Jet* jet);

  //void retrieveBTaggingDiscriminants(xAOD::Jet * jet);
  
  EL::StatusCode decorate(xAOD::Jet * jet) override;

  EL::StatusCode calibrateCopies(xAOD::JetContainer * particles, const CP::SystematicSet & sysSet) override;
  
  virtual EL::StatusCode writeOutputVariables(xAOD::Jet * inPart, xAOD::Jet * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeCustomVariables(xAOD::Jet * inPart, xAOD::Jet * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) override;

  MatchJet m_matchJet;
};

#endif
