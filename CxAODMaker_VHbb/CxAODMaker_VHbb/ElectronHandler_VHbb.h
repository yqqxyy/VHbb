// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_ElectronHandler_VHbb_H
#define CxAODMaker_ElectronHandler_VHbb_H

#include "CxAODMaker/ElectronHandler.h"


class ElectronHandler_VHbb : public ElectronHandler {

public:

  ElectronHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                  EventInfoHandler & eventInfoHandler);

protected:
  
  // selection functions
  bool passVHLooseElectron(xAOD::Electron * electron);
  bool passVHSignalElectron(xAOD::Electron * electron);//common cuts for ZH and WH
  bool passZHSignalElectron(xAOD::Electron * electron);
  bool passWHSignalElectron(xAOD::Electron * electron);
  bool passWHSignalElectronIso(xAOD::Electron * electron);

  //Override function (calls base)
  virtual EL::StatusCode writeCustomVariables(xAOD::Electron* inPart, xAOD::Electron* outPart, 
                                      bool isKinVar, bool isWeightVar, const TString& sysName) override;
  bool m_isMJ;

};

#endif
