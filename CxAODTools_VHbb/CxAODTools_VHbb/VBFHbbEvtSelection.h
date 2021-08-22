#ifndef CxAODTools__VBFHbbEvtSelection_H
#define CxAODTools__VBFHbbEvtSelection_H

#include "CxAODTools/EventSelection.h"

#include "CxAODTools_VHbb/CommonProperties_VBFHbb.h"

namespace xAOD {
#ifndef XAODJET_JET_H
  class Jet;
#endif
#ifndef XAODEGAMMA_ELECTRON_H
  class Electron;
#endif
#ifndef XAODMUON_MUON_H
  class Muon;
#endif

}

template <typename T>
class VBFHbbEvtSelection : public EventSelection {
 protected:
  T m_result;

 public:
  VBFHbbEvtSelection() noexcept;
  
  virtual ~VBFHbbEvtSelection() noexcept {}
  
  virtual T& result() { return m_result; } // probably should be const ?
  
  virtual bool passPreSelection(SelectionContainers & containers, bool isKinVar) override;
  
  virtual bool passSelection(SelectionContainers & containers, bool isKinVar) override;
  
 protected:
  //common to Incl and 1ph analysis
  virtual bool passJetPreSelection     (const xAOD::JetContainer* jets);
  virtual bool passTrackJetPreSelection(const xAOD::JetContainer* jets);
  virtual bool passTrackJetSelection(const xAOD::JetContainer* jets);
  virtual bool passTriggerPreSelection (const xAOD::EventInfo* evtinfo);
  int doVBFPhotonSelection            (const xAOD::PhotonContainer* photons,
					 const xAOD::Photon*& ph); 

 
  virtual bool passKinematics         ();
  
  
  //virtual bool passPhotonSelection  (const xAOD::PhotonContainer* photons) = 0; 
  //specific to each analysis
  virtual bool passJetSelection     (const xAOD::JetContainer* jets) = 0;
  virtual bool passTriggerSelection (const xAOD::EventInfo* evtinfo) = 0;
  virtual void clearResult          () = 0;
  
  double m_b_85_weight = 0.1758475;
  double m_b_77_weight = 0.645925;
  double m_b_70_weight = 0.8244273;  
};

#include "CxAODTools_VHbb/VBFHbbEvtSelection.icc"

#endif
