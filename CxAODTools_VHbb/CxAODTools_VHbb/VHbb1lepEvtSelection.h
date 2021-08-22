#ifndef CxAOD__VHbb1lepEvtSelection_H
#define CxAOD__VHbb1lepEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

// structure holding kinematics of the result
struct ResultVHbb1lep
{
  bool pass;
  const xAOD::Electron* el;
  const xAOD::Muon* mu;
  const xAOD::MissingET* met;
  std::vector<const xAOD::Jet*> signalJets;
  std::vector<const xAOD::Jet*> forwardJets;
  std::vector<const xAOD::Jet*> fatJets;
  std::vector<const xAOD::Jet*> trackJets;
  std::vector<const xAOD::Jet*> subJets;
  std::vector<const xAOD::TauJet*> taus;
  std::vector<const xAOD::TruthParticle*> truthElectrons;
  std::vector<const xAOD::TruthParticle*> truthMuons;
  std::vector<const xAOD::TruthParticle*> truthTaus;
  std::vector<const xAOD::Jet*> truthSignalJets;
  std::vector<const xAOD::Jet*> truthForwardJets;
  std::vector<const xAOD::Jet*> truthFatJets;
  std::vector<const xAOD::TruthParticle*> truthParticles;

  bool isEl() { return el!=nullptr; }    //Why is this here!!!!
};

class VHbb1lepEvtSelection : public VHbbEvtSelection<ResultVHbb1lep>
{
  public:
  VHbb1lepEvtSelection() = default;
  VHbb1lepEvtSelection(ConfigStore *config);
    virtual ~VHbb1lepEvtSelection() noexcept {}


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
    virtual bool passLeptonSelection(const xAOD::ElectronContainer* electrons,
                                     const xAOD::MuonContainer* muons,
                                     const xAOD::MissingET* met) override;

    virtual bool passLeptonPreSelection(const xAOD::ElectronContainer* electrons,
                                        const xAOD::MuonContainer* muons,
                                        const xAOD::MissingET* met) override;

    virtual bool passKinematics() override;

    virtual void clearResult() override;


};

#endif
