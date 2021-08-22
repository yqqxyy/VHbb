// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_PhotonHandler_H
#define CxAODMaker_PhotonHandler_H

#include "CxAODMaker/ObjectHandler.h"

#include "xAODEgamma/PhotonContainer.h"

#include "AsgTools/AnaToolHandle.h"
#include <unordered_map>

namespace CP {
  class EgammaCalibrationAndSmearingTool;
  class IsolationSelectionTool;
  class IsolationCorrectionTool;
}

namespace Trig {
   class IMatchingTool;
   class TrigDecisionTool;
}

class TruthParticleHandler;
class AsgPhotonIsEMSelector;
class ElectronPhotonShowerShapeFudgeTool;
class AsgPhotonEfficiencyCorrectionTool;
class IEGammaAmbiguityTool;

class PhotonHandler : public ObjectHandler< xAOD::PhotonContainer> {

public:
  
  PhotonHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);
  
  virtual ~PhotonHandler();

  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode clearEvent() override;
  virtual void setTruthHandler(TruthParticleHandler* truthhandler ) { m_truthhandler=truthhandler; }
  
  void setP4(const xAOD::Photon * in, xAOD::Photon * out) override {out->setP4(in->pt(), in->eta(), in->phi(), in->m()); }
  
  
protected:
  // calibration tools
  CP::EgammaCalibrationAndSmearingTool * m_EgammaCalibrationAndSmearingTool;
  AsgPhotonIsEMSelector                * m_photonLooseIsEMSelector;
  AsgPhotonIsEMSelector                * m_photonMediumIsEMSelector;
  AsgPhotonIsEMSelector                * m_photonTightIsEMSelector;
  ElectronPhotonShowerShapeFudgeTool   * m_photonFudgeMCTool;
  AsgPhotonEfficiencyCorrectionTool    * m_photonLooseEffTool;
  AsgPhotonEfficiencyCorrectionTool    * m_photonMediumEffTool;
  AsgPhotonEfficiencyCorrectionTool    * m_photonTightEffTool;
  AsgPhotonEfficiencyCorrectionTool    * m_photonIsoTightEffTool;
  // Trigger matching tool
  Trig::TrigDecisionTool *m_trigDecTool;

#ifndef __MAKECINT__
  asg::AnaToolHandle<Trig::IMatchingTool> m_match_Tool; //!
#endif // not __MAKECINT__



  std::unordered_map<std::string, PROP<int>*> m_triggersToMatch;
  
  //Ambiguity tool
  IEGammaAmbiguityTool *m_egammaAmbiguityTool;

  CP::IsolationSelectionTool           * m_isIsoPh;
  CP::IsolationCorrectionTool          * m_isoCorr_tool;

  TruthParticleHandler* m_truthhandler;

  // selection functions
  bool passLoosePhoton(xAOD::Photon * photon);

  virtual EL::StatusCode decorateOriginParticle(const xAOD::Photon * photon) override;
  virtual EL::StatusCode decorate(xAOD::Photon * photon) override;  
  virtual EL::StatusCode calibrateCopies(xAOD::PhotonContainer * particles, const CP::SystematicSet & sysSet) override;
  virtual EL::StatusCode writeOutputVariables(xAOD::Photon * inPhoton, xAOD::Photon * outPhoton, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeAllVariations(xAOD::Photon* inPhoton, xAOD::Photon* outPhoton, const TString& sysName);
  virtual EL::StatusCode writeKinematicVariations(xAOD::Photon* inPhoton, xAOD::Photon* outPhoton, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::Photon* inPhoton, xAOD::Photon* outPhoton, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::Photon* inPhoton, xAOD::Photon* outPhoton, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::Photon * inPhoton, xAOD::Photon * outPhoton, bool isKinVar, bool isWeightVar, const TString& sysName) override;

};//class PhotonHandler

#endif
