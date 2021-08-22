#ifndef CxAOD__VBFHbbInclEvtSelection_H
#define CxAOD__VBFHbbInclEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VBFHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

struct ResultVBFHbbIncl
{
  bool pass;
  std::vector<const xAOD::Jet*> jets;
  std::vector<const xAOD::Jet*> forwardJets;
  std::vector<const xAOD::Jet*> jets_etaSort;
  std::vector<const xAOD::Jet*> trackJets;
};

class VBFHbbInclEvtSelection : public VBFHbbEvtSelection<ResultVBFHbbIncl>
{
 public:
  VBFHbbInclEvtSelection() = default;
  virtual ~VBFHbbInclEvtSelection() noexcept {}
  
  
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
  virtual  bool passJetSelection(const xAOD::JetContainer* jets) override;
  
  virtual bool passTriggerSelection(const xAOD::EventInfo* evtinfo) override;


  virtual bool passKinematics() override;
  
  virtual void clearResult() override;
  
};

#endif
