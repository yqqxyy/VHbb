 #ifndef CxAOD__VHbb0lepEvtSelection_H
#define CxAOD__VHbb0lepEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

// structure holding kinematics of the result
struct ResultVHbb0lep
{
  bool pass;
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
};

class VHbb0lepEvtSelection : public VHbbEvtSelection<ResultVHbb0lep>
{
  public:
  VHbb0lepEvtSelection() = default;
  VHbb0lepEvtSelection(ConfigStore *config);
    virtual ~VHbb0lepEvtSelection() noexcept {}


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
