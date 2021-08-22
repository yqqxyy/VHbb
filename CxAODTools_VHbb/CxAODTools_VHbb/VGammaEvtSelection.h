#ifndef CxAOD__VGammaEvtSelection_H
#define CxAOD__VGammaEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VGammaFatJEvtSelection.h"
#include "EventLoop/StatusCode.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

// structure holding kinematics of the result
struct ResultVGamma
{
  bool pass;
  std::vector<const xAOD::Photon*> ph;
  std::vector<const xAOD::Jet*> fatJets;
  std::vector<const xAOD::Jet*> jets4;
  std::vector<const xAOD::Jet*> trackJets;
};

class VGammaEvtSelection : public VGammaFatJEvtSelection<ResultVGamma>
{
  public:
    VGammaEvtSelection() = default;
    virtual ~VGammaEvtSelection() noexcept {}

    virtual bool passSelection(SelectionContainers & containers, bool isKinVar) override;

    virtual bool passPreSelection(SelectionContainers & containers, bool isKinVar) override;

    virtual EL::StatusCode writeEventVariables(const xAOD::EventInfo* eventInfoIn,
                                               xAOD::EventInfo* eventInfoOut,
                                               bool isKinVar,
                                               bool isWeightVar,
                                               std::string sysName,
                                               int rdm_RunNumber,
                                               CP::MuonTriggerScaleFactors* trig_sfmuon) override;

    using EventSelection::writeEventVariables;

    std::string m_model;
    std::string m_analysisType; 
    void SetModel(std::string model) { m_model = model;}
    void SetAnalysisType(std::string analysisType) { m_analysisType = analysisType;}

  protected:
  virtual  bool passPhotonSelection(const xAOD::PhotonContainer* photons) override;

  //virtual  bool passPhotonPreSelection(const xAOD::PhotonContainer* photons);

  virtual bool passFatJetSelection(const xAOD::JetContainer* jets) override;
  //virtual bool passTriggerSelection(const xAOD::EventInfo* evtinfo) override;
  
  virtual bool passKinematics() override;
  
  virtual void clearResult() override;

};

#endif
