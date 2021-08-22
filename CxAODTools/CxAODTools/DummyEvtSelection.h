#ifndef CxAODTools__DummyEvtSelection_H
#define CxAODTools__DummyEvtSelection_H

#include "CxAODTools/EventSelection.h"
#include "CxAODTools/CutFlowCounter.h"
#include "EventLoop/StatusCode.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

namespace xAOD {
#ifndef XAODMUON_MUON_H
  class Muon;
#endif
#ifndef XAODJET_JET_H
  class Jet;
#endif
}

// structure holding kinematics of the result
struct ResultDummy
{
  bool pass;
  const xAOD::Muon* muon;
  const xAOD::MissingET* met;
  std::vector<const xAOD::Jet*> jets;
};

class DummyEvtSelection : public EventSelection {
protected:
  ResultDummy m_result;
  void clearResult();

public:
  DummyEvtSelection() noexcept;
  virtual ~DummyEvtSelection() noexcept {
  }
  virtual ResultDummy result() { return m_result; }
  virtual bool passPreSelection(SelectionContainers & containers, bool isKinVar);
  virtual bool passSelection(SelectionContainers & containers, bool isKinVar);
  virtual EL::StatusCode writeEventVariables(
          const xAOD::EventInfo* eventInfoIn,
          xAOD::EventInfo* eventInfoOut,
          bool isKinVar,
          bool isWeightVar,
          std::string sysName,
          int rdm_RunNumber,
          CP::MuonTriggerScaleFactors* m_trig_sfmuon) override;

  virtual void writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                   xAOD::EventInfo* /*eventInfoOut*/,
                                   bool /*isKinVar*/,
                                   bool /*isWeightVar*/) override;

};

#endif
