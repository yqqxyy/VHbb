// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_JetHandler_VHbb_H
#define CxAODMaker_JetHandler_VHbb_H

#include "CxAODMaker/JetHandler.h"

class JetHandler_VHbb : public JetHandler {

public:

  JetHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
             EventInfoHandler & eventInfoHandler);

protected:
  
  // selection functions
  bool passVetoJet(xAOD::Jet* jet);
  bool passSignalJet(xAOD::Jet* jet);
  bool checkCentralJet(xAOD::Jet* jet,bool isInCutFlow=0);

  float m_btagCut;

  virtual EL::StatusCode writeCustomVariables(xAOD::Jet * inPart, xAOD::Jet * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) override;
};

#endif
