// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_ForwardElectronHandler_H
#define CxAODMaker_ForwardElectronHandler_H

#include "CxAODMaker/ObjectHandler.h"

#include "xAODEgamma/ElectronContainer.h"

#include "AsgTools/AnaToolHandle.h"

#include <unordered_map>

namespace CP {
  class EgammaCalibrationAndSmearingTool;
  class IsolationSelectionTool;
  class IsolationCorrectionTool;
}

namespace Trig {
  class WebBunchCrossingTool;
}

class TruthParticleHandler;
class AsgElectronEfficiencyCorrectionTool;
class AsgForwardElectronLikelihoodTool;

class ForwardElectronHandler : public ObjectHandler<xAOD::ElectronContainer> {

public:

  ForwardElectronHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                  EventInfoHandler & eventInfoHandler);

  virtual ~ForwardElectronHandler();

  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode clearEvent() override;
  virtual void setTruthHandler(TruthParticleHandler* truthhandler ) { m_truthhandler=truthhandler; }
  virtual EL::StatusCode fillOutputLinks();

  void setP4(const xAOD::Electron * in, xAOD::Electron * out) override { out->setP4(in->pt(), in->eta(), in->phi(), in->m()); }

  EL::StatusCode calibrate() override;

protected:

  // calibration tools
  CP::EgammaCalibrationAndSmearingTool * m_EgammaCalibrationAndSmearingTool;

  AsgElectronEfficiencyCorrectionTool  * m_effToolLooseLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolMediumLH;
  AsgElectronEfficiencyCorrectionTool  * m_effToolTightLH;

  AsgForwardElectronLikelihoodTool            * m_checkLooseLH;
  AsgForwardElectronLikelihoodTool            * m_checkMediumLH;
  AsgForwardElectronLikelihoodTool            * m_checkTightLH;
  CP::IsolationSelectionTool           * m_isIso; 

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

  Trig::WebBunchCrossingTool *m_bct;

  bool m_doResolution;


};

#endif
