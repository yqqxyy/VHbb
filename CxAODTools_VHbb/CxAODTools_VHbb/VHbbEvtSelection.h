#ifndef CxAODTools__VHbbEvtSelection_H
#define CxAODTools__VHbbEvtSelection_H

#include "CxAODTools/EventSelection.h"
#include "CxAODTools/EvtSelTruthAllocator.h"

namespace xAOD {
#ifndef XAODEGAMMA_ELECTRON_H
  class Electron;
#endif
#ifndef XAODEGAMMA_PHOTON_H
  class Photon;
#endif
#ifndef XAODMUON_MUON_H
  class Muon;
#endif
#ifndef XAODTAU_TAUJET_H 
  class TauJet;
#endif
#ifndef XAODJET_JET_H
  class Jet;
#endif
}

template <typename T>
class VHbbEvtSelection : public EventSelection
{
  protected:
    T m_result;
    EvtSelTruthAllocator m_RecoTruthLinker;
    
  public:
    VHbbEvtSelection() noexcept;

    VHbbEvtSelection(ConfigStore *config) noexcept;

    virtual ~VHbbEvtSelection() noexcept {}

    virtual T& result() { return m_result; } // probably should be const ?

    virtual bool passPreSelection(SelectionContainers & containers, bool isKinVar) override;

   virtual bool passSelection(SelectionContainers & containers, bool isKinVar) override;
  protected:

    //Jet Selection Methods
    virtual bool passJetPreSelection(const xAOD::JetContainer* jets);
    virtual bool passJetSelection(const xAOD::JetContainer* jets);

    //Fatjet Selection Methods
    virtual bool passFatJetPreSelection(const xAOD::JetContainer* jets);
    virtual bool passFatJetSelection(const xAOD::JetContainer* jets);

    //Trackjet Selection Methods
    virtual bool passTrackJetPreSelection(const xAOD::JetContainer* jets);
    virtual bool passTrackJetSelection(const xAOD::JetContainer* jets);

    //CoM subjet Selection Methods
    virtual bool passCoMJetSelection(const xAOD::JetContainer* jets);
    
    //Lepton Selection Methods
    virtual bool passLeptonPreSelection(const xAOD::ElectronContainer* electrons,
          const xAOD::MuonContainer* muons,
          const xAOD::MissingET* met) = 0;
    virtual bool passLeptonSelection(const xAOD::ElectronContainer * electrons,
                                     const xAOD::MuonContainer* muons,
                                     const xAOD::MissingET* met) = 0;
     
    //Tau Selection Methods
    virtual bool passTauPreSelection(const xAOD::TauJetContainer* taus);
    virtual bool passTauSelection(const xAOD::TauJetContainer* taus);

    //Truth Particle Selection
    virtual bool passTruthParticlePreSelection(const xAOD::TruthParticleContainer* truth, std::vector<const xAOD::TruthParticle*> *result);

    virtual bool passKinematics();

    virtual void clearResult() = 0;

    /// ensures consistency between channels
    int doVHLeptonSelection(const xAOD::ElectronContainer* electrons,
                            const xAOD::MuonContainer* muons,
                            const xAOD::Electron*& el1, const xAOD::Electron*& el2,
                            const xAOD::Muon*& mu1, const xAOD::Muon*& mu2);

    int doVHLeptonPreSelection(const xAOD::ElectronContainer* electrons,
                               const xAOD::MuonContainer* muons,
                               const xAOD::Electron*& el1, const xAOD::Electron*& el2,
                               const xAOD::Muon*& mu1, const xAOD::Muon*& mu2);
};

#include "CxAODTools_VHbb/VHbbEvtSelection.icc"

#endif
