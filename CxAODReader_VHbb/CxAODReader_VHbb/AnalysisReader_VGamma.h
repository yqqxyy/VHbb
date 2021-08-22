#ifndef CxAODReader_AnalysisReader_VGamma_H
#define CxAODReader_AnalysisReader_VGamma_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
#include "CxAODTools_VHbb/VGammaEvtSelection.h"

#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1F.h>

class AnalysisReader_VGamma : public AnalysisReader_VHQQ {
 protected:
  // Used for MODEL systematics on V+Jets and TTbar
  // CAS::EventType m_evtType; //!
  // CAS::DetailEventType m_detailevtType; //!
  // CAS::Systematic m_Systematic; //!

 public:
  AnalysisReader_VGamma();
  ~AnalysisReader_VGamma();

  virtual EL::StatusCode initializeSelection() override;
  bool m_doCutBased;
  bool m_doMVAPreSel;
  bool m_doTrigStudy;
  bool m_doBlinding;
  float m_bTagCut;

  static bool sortPhotons_pt(const xAOD::Photon *phA, const xAOD::Photon *phB) {
    return phA->pt() > phB->pt();
  }

  EL::StatusCode fill_vgamma();
  EL::StatusCode run_vgamma_analysis();
  std::vector<float> getTrkJetMV2C10Vector(
      const std::vector<const xAOD::Jet *> FatJets, const int n = 0,
      std::string trackLinkName = "GhostAntiKt2TrackJet");
  std::vector<int> getTrkJetTruthIDVector(
      const std::vector<const xAOD::Jet *> FatJets, const int n = 0,
      std::string trackLinkName = "GhostAntiKt2TrackJet");
  EL::StatusCode initializeJetResolutions();

  ClassDefOverride(AnalysisReader_VGamma, 1);
};

#endif  // ifndef CxAODReader_AnalysisReader_VGamma_H
