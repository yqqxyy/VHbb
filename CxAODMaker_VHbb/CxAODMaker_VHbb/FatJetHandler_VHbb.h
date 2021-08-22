// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_FatJetHandler_VHbb_H
#define CxAODMaker_FatJetHandler_VHbb_H

#include "CxAODMaker/FatJetHandler.h"

class FatJetHandler_VHbb : public FatJetHandler {

public:

  FatJetHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);

private:
  bool m_writePartonTruthLabel;
  
  // selection functions
  bool passFatJet(xAOD::Jet* jet);

  virtual EL::StatusCode decorate(xAOD::Jet* jet) override;
  
  virtual EL::StatusCode writeCustomVariables(xAOD::Jet * inPart, xAOD::Jet * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) override;
};

#endif
