// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_PhotonHandler_VHbb_H
#define CxAODMaker_PhotonHandler_VHbb_H

#include "CxAODMaker/PhotonHandler.h"

class PhotonHandler_VHbb : public PhotonHandler {

public:
  
  PhotonHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);
  
protected:

  // selection functions
  bool passVBFLoosePhoton(xAOD::Photon * photon);
  bool passVBFSignalPhoton(xAOD::Photon * photon);

  virtual EL::StatusCode writeCustomVariables(xAOD::Photon * inPart, xAOD::Photon * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) override;


};//class PhotonHandler_VHbb

#endif
