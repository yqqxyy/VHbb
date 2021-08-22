#ifndef CxAOD__VHbb2lepEvtSelection_H
#define CxAOD__VHbb2lepEvtSelection_H

#include <vector>

#include "CxAODTools_VHbb/VHbbEvtSelection.h"
#include "EventLoop/StatusCode.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

// structure holding kinematics of the result
struct ResultVHbb2lep
{
  bool pass;
  const xAOD::Electron* el1;
  const xAOD::Electron* el2;
  const xAOD::Muon* mu1;
  const xAOD::Muon* mu2;
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

  enum class Type { undefined, ee, mm, em };
  Type type() {
    if(el1!=nullptr && el2!=nullptr) return Type::ee;
    if(mu1!=nullptr && mu2!=nullptr) return Type::mm;
    if(mu1!=nullptr && el1!=nullptr) return Type::em;
    return Type::undefined;
  }
};

class VHbb2lepEvtSelection : public VHbbEvtSelection<ResultVHbb2lep>
{
  public:
  VHbb2lepEvtSelection() = default;
  VHbb2lepEvtSelection(ConfigStore *config);
    virtual ~VHbb2lepEvtSelection() noexcept {}

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
