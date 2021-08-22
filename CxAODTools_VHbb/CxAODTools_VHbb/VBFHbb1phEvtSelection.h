#ifndef CxAOD__VBFHbb1phEvtSelection_H
#define CxAOD__VBFHbb1phEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VBFHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

struct ResultVBFHbb1ph
{
  bool pass;
  const xAOD::Photon* ph;
  std::vector<const xAOD::Jet*> jets;
  std::vector<const xAOD::Jet*> trackJets;
  std::vector<const xAOD::Jet*> forwardJets;
};

class VBFHbb1phEvtSelection : public VBFHbbEvtSelection<ResultVBFHbb1ph>
{
 public:
  VBFHbb1phEvtSelection() = default;
  virtual ~VBFHbb1phEvtSelection() noexcept {}

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
 protected:
  bool passPhotonSelection(const xAOD::PhotonContainer* photons) ;

  //virtual  bool passPhotonPreSelection(const xAOD::PhotonContainer* photons);

  virtual bool passTriggerSelection (const xAOD::EventInfo* evtinfo) override;
  virtual bool passJetSelection(const xAOD::JetContainer* jets) override;
  //virtual bool passTriggerSelection(const xAOD::EventInfo* evtinfo) override;
  
  virtual bool passKinematics() override;
  
  virtual void clearResult() override;
  
};

#endif
