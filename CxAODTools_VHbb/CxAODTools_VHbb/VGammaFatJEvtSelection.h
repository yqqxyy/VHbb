#ifndef CxAODTools__VGammaFatJEvtSelection_H
#define CxAODTools__VGammaFatJEvtSelection_H

#include "CxAODTools/EventSelection.h"
#include "xAODEgamma/Photon.h"

namespace xAOD {
#ifndef XAODEGAMMA_PHOTON_H
  class Photon;
#endif
#ifndef XAODJET_JET_H
  class Jet;
#endif
}

template <typename T>
class VGammaFatJEvtSelection : public EventSelection
{
  protected:
    T m_result;

  public:
    VGammaFatJEvtSelection() noexcept;

    virtual ~VGammaFatJEvtSelection() noexcept {}

    virtual T& result() { return m_result; } // probably should be const ?

    static bool sortPhotons_pt(const xAOD::Photon* phA, const xAOD::Photon* phB) {
      return phA->pt() > phB->pt();
    }

    virtual bool passPreSelection(SelectionContainers & containers, bool isKinVar) override;

    virtual bool passSelection(SelectionContainers & containers, bool isKinVar) override;

  protected:
    //common to Incl and 1ph analysis
    virtual bool passFatJetPreSelection     (const xAOD::JetContainer* jets);
    virtual bool passJetPreSelection     (const xAOD::JetContainer* jets);
    virtual bool passJetSelection     (const xAOD::JetContainer* jets);
    virtual bool passTrackJetPreSelection(const xAOD::JetContainer* jets);
    virtual bool passTrackJetSelection(const xAOD::JetContainer* jets);
    virtual bool passPhotonPreSelection  (const xAOD::PhotonContainer* photons);
    //virtual bool passTriggerPreSelection (const xAOD::EventInfo* evtinfo);

    int doVGammaFatJPhotonSelection            (const xAOD::PhotonContainer* photons);
    int doVGammaFatJPhotonPreSelection         (const xAOD::PhotonContainer* photons);
    virtual bool passKinematics         ();



    //specific to each analysis
    virtual bool passFatJetSelection     (const xAOD::JetContainer* jets) = 0;
    virtual bool passPhotonSelection  (const xAOD::PhotonContainer* photons) = 0;
    //virtual bool passTriggerSelection (const xAOD::EventInfo* evtinfo) = 0;
    virtual void clearResult          () = 0;


};

#include "CxAODTools_VHbb/VGammaFatJEvtSelection.icc"

#endif
