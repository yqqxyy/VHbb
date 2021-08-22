// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_ElectronHandler_H
#define CxAODMaker_ElectronHandler_H

#include "CxAODMaker/ObjectHandler.h"

#include "xAODEgamma/ElectronContainer.h"

#include "AsgTools/AnaToolHandle.h"

#include <unordered_map>

namespace CP {
  class EgammaCalibrationAndSmearingTool;
  class IsolationSelectionTool;
  class IsolationCorrectionTool;
}

namespace TrigConf{
  class xAODConfigTool;
}

namespace Trig{
  class TrigDecisionTool;
  class IMatchingTool;
}

class TruthParticleHandler;
class AsgElectronEfficiencyCorrectionTool;
class AsgElectronLikelihoodTool;

class ElectronHandler : public ObjectHandler<xAOD::ElectronContainer> {

public:

  ElectronHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                  EventInfoHandler & eventInfoHandler);

  virtual ~ElectronHandler();

  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode clearEvent() override;
  virtual void setTruthHandler(TruthParticleHandler* truthhandler ) { m_truthhandler=truthhandler; }
  virtual EL::StatusCode fillOutputLinks();

  void setP4(const xAOD::Electron * in, xAOD::Electron * out) override { out->setP4(in->pt(), in->eta(), in->phi(), in->m()); }

  EL::StatusCode calibrate() override;

protected:

  // calibration tools
  CP::EgammaCalibrationAndSmearingTool * m_EgammaCalibrationAndSmearingTool;

  AsgElectronEfficiencyCorrectionTool  * m_trigEffToolLooseLHIsoFixedCutLoose;
  AsgElectronEfficiencyCorrectionTool  * m_trigToolLooseLHIsoFixedCutLoose;

  AsgElectronEfficiencyCorrectionTool  * m_trigEffToolTightLHIsoFixedCutTight;
  AsgElectronEfficiencyCorrectionTool  * m_trigToolTightLHIsoFixedCutTight;

  AsgElectronEfficiencyCorrectionTool  * m_trigEffToolTightLHIsoFixedCutLoose;
  AsgElectronEfficiencyCorrectionTool  * m_trigToolTightLHIsoFixedCutLoose;

  AsgElectronEfficiencyCorrectionTool  * m_e17trigEffToolMediumLHIsoFixedCutLoose;
  AsgElectronEfficiencyCorrectionTool  * m_e17trigToolMediumLHIsoFixedCutLoose;

  AsgElectronEfficiencyCorrectionTool  * m_trigEffToolMediumLH_offTightIsoFixedCutLoose;
  AsgElectronEfficiencyCorrectionTool  * m_trigToolMediumLH_offTightIsoFixedCutLoose;

  AsgElectronEfficiencyCorrectionTool  * m_effToolLooseLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolMediumLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolTightLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolReco;

  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoGradientLooseLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoGradientMediumLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoGradientTightLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutLooseLooseLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutLooseMediumLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutLooseTightLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutTightLooseLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutTightMediumLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutTightTightLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutHighPtCaloOnlyLooseLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutHighPtCaloOnlyMediumLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolIsoFixedCutHighPtCaloOnlyTightLH;

  AsgElectronLikelihoodTool            * m_checkVeryLooseLH;
  AsgElectronLikelihoodTool            * m_checkLooseNoBLLH;
  AsgElectronLikelihoodTool            * m_checkLooseLH;
  AsgElectronLikelihoodTool            * m_checkMediumLH;
  AsgElectronLikelihoodTool            * m_checkTightLH;
  CP::IsolationSelectionTool           * m_isIso; 
  CP::IsolationCorrectionTool          * m_isoCorr_tool;

  Trig::TrigDecisionTool *m_trigDecTool;

  // selection functions
  bool passLooseElectron(xAOD::Electron * electron);

  virtual EL::StatusCode decorateOriginParticle(const xAOD::Electron * electron) override;
  virtual EL::StatusCode decorate(xAOD::Electron * electron) override;

  virtual EL::StatusCode calibrateCopies(xAOD::ElectronContainer * particles, const CP::SystematicSet & sysSet) override;

  virtual EL::StatusCode writeOutputVariables(xAOD::Electron * inElectron, xAOD::Electron * outElectron, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeAllVariations(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& sysName);
  virtual EL::StatusCode writeKinematicVariations(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::Electron* inElectron, xAOD::Electron* outElectron, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::Electron * inElectron, xAOD::Electron * outElectron, bool isKinVar, bool isWeightVar, const TString& sysName) override;

  TruthParticleHandler* m_truthhandler;

  bool m_doTrigMatch;
  bool m_doResolution;

  std::unordered_map<std::string, PROP<int>*> m_triggersToMatch;

#ifndef __MAKECINT__
  asg::AnaToolHandle<Trig::IMatchingTool> m_match_Tool; //!
#endif // not __MAKECINT__

};

#endif
