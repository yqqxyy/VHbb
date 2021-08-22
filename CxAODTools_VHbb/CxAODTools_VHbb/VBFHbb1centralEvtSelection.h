#ifndef CxAOD__VBFHbb1centralEvtSelection_H
#define CxAOD__VBFHbb1centralEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VBFHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

struct ResultVBFHbb1central {
  bool pass;
  std::vector<const xAOD::Jet*> jets;
  std::vector<const xAOD::Jet*> forwardJets;
  std::vector<const xAOD::Jet*> jets_etaSort;
  std::vector<const xAOD::Jet*> trackJets;
};

class VBFHbb1centralEvtSelection : public VBFHbbEvtSelection<ResultVBFHbb1central> {
 public:
  // Constructor
  VBFHbb1centralEvtSelection(bool applybtag=true, TString WP = "77", bool isMC = false);
  VBFHbb1centralEvtSelection(const VBFHbb1centralEvtSelection&);
  // Destructor
  virtual ~VBFHbb1centralEvtSelection() noexcept;

  //  virtual void setIsMC(bool isMC=false) {m_isMC = isMC;};  
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
};

#endif
