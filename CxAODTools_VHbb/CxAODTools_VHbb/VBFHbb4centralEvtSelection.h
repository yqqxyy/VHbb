#ifndef CxAOD__VBFHbb4centralEvtSelection_H
#define CxAOD__VBFHbb4centralEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VBFHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

struct ResultVBFHbb4central
{
  bool pass;
  std::vector<const xAOD::Jet*> jets;
  std::vector<const xAOD::Jet*> forwardJets;
  std::vector<const xAOD::Jet*> jets_etaSort;
  std::vector<const xAOD::Jet*> trackJets;
};

class VBFHbb4centralEvtSelection : public VBFHbbEvtSelection<ResultVBFHbb4central>
{
 public:
  // Constructor 
  VBFHbb4centralEvtSelection(bool applybtag, TString WP = "77", bool isMC = false);
  VBFHbb4centralEvtSelection(const VBFHbb4centralEvtSelection& other);
  // Destructor
  virtual ~VBFHbb4centralEvtSelection() noexcept;

  // ================

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
  std::string m_Period;
  TString m_WP;
};

#endif
