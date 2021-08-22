#ifndef CxAOD__VBFHbb2centralEvtSelection_H
#define CxAOD__VBFHbb2centralEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VBFHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

struct ResultVBFHbb2central
{
  bool pass;
  std::vector<const xAOD::Jet*> jets;
  std::vector<const xAOD::Jet*> forwardJets;
  std::vector<const xAOD::Jet*> jets_etaSort;
  std::vector<const xAOD::Jet*> trackJets;
};

class VBFHbb2centralEvtSelection : public VBFHbbEvtSelection<ResultVBFHbb2central>
{
 public:
  // Constructor
  VBFHbb2centralEvtSelection(bool applybtag, TString WP = "77", bool isMC = false);
  VBFHbb2centralEvtSelection(const VBFHbb2centralEvtSelection& other);
  // Destructor
  virtual ~VBFHbb2centralEvtSelection() noexcept;

  //  virtual void setIsMC(bool isMC) {m_isMC = isMC;}
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
  

 private:
  bool m_applybtag;
  bool m_isMC;
  TString m_WP;
  //bool m_DoBJetTriggerOfflineScale;

};

#endif
