// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_JetSemileptonic_H
#define CxAODMaker_JetSemileptonic_H

#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"

class TH1F;
class ISystematicsTool;
class PtRecoTool;
namespace CP {
  class MuonSelectionTool;
}

class JetSemileptonic {

 public:
  JetSemileptonic();
  ~JetSemileptonic();

  virtual EL::StatusCode initialize(bool debug);

  EL::StatusCode test();
  EL::StatusCode countElectronInJet(xAOD::Jet* jet, const TString& variation, const xAOD::ElectronContainer* electrons);
  EL::StatusCode correctMuonInJet(xAOD::Jet* jet, const TString& variation, const xAOD::MuonContainer* muons, bool doPtReco);
  EL::StatusCode setJetTLV(xAOD::Jet* jet, const std::string& name, const TLorentzVector& tlv); 

  bool passForJetElectron(const xAOD::Electron* electron);
  bool passForJetMuon(const xAOD::Muon* muon);
  TLorentzVector getELossTLV(const xAOD::Muon* muon);

  private:

  typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> >  JetFourMom_t;

  PtRecoTool* m_PtRecoTool;
  CP::MuonSelectionTool* m_muonSelectionTool;
  bool m_debug;
  MSG::Level m_msgLevel;
};

#endif
