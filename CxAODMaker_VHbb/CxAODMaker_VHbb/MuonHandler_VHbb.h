// Dear emacs, this is -*-c++-*-

#ifndef CxAODMaker_MuonHandler_VHbb_H
#define CxAODMaker_MuonHandler_VHbb_H

#include "CxAODMaker/MuonHandler.h"

class MuonHandler_VHbb : public MuonHandler {

public:

  MuonHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
              EventInfoHandler & eventInfoHandler);

protected:
  
  // selection functions
  bool passVHLooseMuon(xAOD::Muon * part);
  bool passVHSignalMuon(xAOD::Muon * part);//common cuts for ZH and WH
  bool passZHSignalMuon(xAOD::Muon * part);
  bool passWHSignalMuon(xAOD::Muon * part);
  bool passWHSignalMuonIso(xAOD::Muon * part);
  
  virtual EL::StatusCode writeCustomVariables(xAOD::Muon * inMuon, xAOD::Muon * outMuon, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  bool m_isMJ;

};

#endif

