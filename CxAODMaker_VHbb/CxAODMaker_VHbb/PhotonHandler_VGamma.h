// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_PhotonHandler_VGamma_H
#define CxAODMaker_PhotonHandler_VGamma_H

#include "CxAODMaker/PhotonHandler.h"

class PhotonHandler_VGamma : public PhotonHandler {

public:
  
  PhotonHandler_VGamma(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);
  
protected:

  //Cut parameters
  double m_ptcut;
  double m_etacut;

  // selection functions
  bool passVBFLoosePhoton(xAOD::Photon * photon);
  bool passVBFSignalPhoton(xAOD::Photon * photon);

  virtual EL::StatusCode writeCustomVariables(xAOD::Photon * inPart, xAOD::Photon * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) override;


};//class PhotonHandler_VGamma

#endif
