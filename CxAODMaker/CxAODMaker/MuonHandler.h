// Dear emacs, this is -*-c++-*-

#ifndef CxAODMaker_MuonHandler_H
#define CxAODMaker_MuonHandler_H

#include "CxAODMaker/ObjectHandler.h"

#include "xAODMuon/MuonContainer.h"

#include "AsgTools/AnaToolHandle.h"

#include <unordered_map>

namespace CP {
  class MuonCalibrationPeriodTool;
  class MuonSelectionTool;
  class MuonEfficiencyScaleFactors;
  class IsolationSelectionTool;
}
namespace Trig {
  class TrigDecisionTool;
  class IMatchingTool;
}

class MuonHandler : public ObjectHandler<xAOD::MuonContainer> {

public:

  MuonHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
              EventInfoHandler & eventInfoHandler);
  
  virtual ~MuonHandler();
  
  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode clearEvent() override;

  void setP4(const xAOD::Muon * in, xAOD::Muon * out) override { out->setP4(in->pt(), in->eta(), in->phi()); }

  EL::StatusCode calibrate() override;
  virtual EL::StatusCode fillOutputLinks();

protected:

  // calibration tools
  CP::MuonCalibrationPeriodTool * m_muonCalibrationPeriodTool;
  CP::MuonSelectionTool  *m_muonSelectionToolLoose;
  CP::MuonSelectionTool  *m_muonSelectionToolMedium;
  CP::MuonSelectionTool  *m_muonSelectionToolTight;
  CP::MuonSelectionTool  *m_muonSelectionToolHighPt;
  CP::MuonSelectionTool  *m_muonSelectionToolLowPtEfficiency;
  CP::MuonSelectionTool  *m_muonSelectionToolVeryLoose;

  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsTTVA;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsLoose;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsMedium;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsTight;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsHighPt;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsLowPt;

  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFixedCutLooseIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFixedCutTightIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFixedCutPflowLooseIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFixedCutPflowTightIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFixedCutHighPtTrackOnlyIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFixedCutTightTrackOnlyIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFCLoose_FixedRadIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFCTight_FixedRadIso;
  CP::MuonEfficiencyScaleFactors * m_muonEfficiencyScaleFactorsFCTightTrackOnly_FixedRadIso;

  CP::IsolationSelectionTool           * m_isIso; 

  Trig::TrigDecisionTool *m_trigDecTool; 

  // selection functions
  bool passLooseMuon(xAOD::Muon * muon);

  virtual EL::StatusCode decorateOriginParticle(const xAOD::Muon * muon) override;
  virtual EL::StatusCode decorate(xAOD::Muon * muon) override; 
  
  virtual EL::StatusCode calibrateCopies(xAOD::MuonContainer * particles, const CP::SystematicSet & sysSet) override;

  virtual EL::StatusCode writeOutputVariables(xAOD::Muon * inMuon, xAOD::Muon * outMuon, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeAllVariations(xAOD::Muon* inMuon, xAOD::Muon* outMuon, const TString& sysName);
  virtual EL::StatusCode writeKinematicVariations(xAOD::Muon* inMuon, xAOD::Muon* outMuon, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::Muon* inMuon, xAOD::Muon* outMuon, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::Muon* inMuon, xAOD::Muon* outMuon, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::Muon * inMuon, xAOD::Muon * outMuon, bool isKinVar, bool isWeightVar, const TString& sysName) override;

  bool m_doTrigMatch;
  bool m_doResolution;
  bool m_doSagittaCorrection;
  bool m_turnOffMuonTRTRequirement;

  std::unordered_map<std::string, PROP<int>*> m_triggersToMatch;

#ifndef __MAKECINT__
  //Trig::MatchingTool* m_match_Tool; //!
  asg::AnaToolHandle<Trig::IMatchingTool> m_match_Tool; //!
#endif // not __MAKECINT__

};

#endif

