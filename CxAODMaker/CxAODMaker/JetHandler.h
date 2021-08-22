// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_JetHandler_H
#define CxAODMaker_JetHandler_H

#include "CxAODMaker/ObjectHandler.h"
#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "CxAODMaker/TruthJetHandler.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "AsgTools/AnaToolHandle.h"

class JetSemileptonic;
class JetCalibrationTool;
class JetCleaningTool;
class JERTool;
class JERSmearingTool;
class JetUncertaintiesTool;
//class JetVertexTaggerTool;
class JetForwardJvtTool;
namespace CP {
  class JetJvtEfficiency;
  class JetTileCorrectionTool;
  class JetQGTagger;
}

namespace Trig {
   class TrigDecisionTool;
}

namespace InDet {
   class InDetTrackTruthFilterTool;
   class InDetTrackTruthOriginTool;
   class JetTrackFilterTool;
   class InDetTrackSelectionTool;
}

class JetHandler : public ObjectHandler<xAOD::JetContainer> {

public:

  JetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
             EventInfoHandler & eventInfoHandler);

  virtual ~JetHandler();

  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode clearEvent() override;

  void setP4(const xAOD::Jet * in, xAOD::Jet * out) override { out->setJetP4( in->jetP4() ); }
  void setP4(const xAOD::Jet * in,  const std::string& name, xAOD::Jet * out) { out->setJetP4(name,in->jetP4(name) ); }

  //semileptonic jet decays
  EL::StatusCode countElectronInJet(JetSemileptonic* m_jetSemileptonic, const xAOD::ElectronContainer* electrons);
  EL::StatusCode correctMuonInJet(JetSemileptonic* m_jetSemileptonic, const xAOD::MuonContainer* muons);
  EL::StatusCode correctRegressionForJet();
  EL::StatusCode matchTruthJet(JetSemileptonic* m_jetSemileptonic, const xAOD::JetContainer* truthjets, const std::string& name, double DRCut);
  EL::StatusCode IdentifyVBFJets(xAOD::JetContainer * jets, const xAOD::TruthParticleContainer* particles);
  //per event JVT SF
  EL::StatusCode getJVTeventSF(const CP::SystematicSet& sysSet, float& JVTeventSF);

  EL::StatusCode decorateOriginParticles(xAOD::JetContainer * jets) override;

protected:

  // tools
  JetCalibrationTool * m_jetCalibration;
  JetCalibrationTool * m_jetCalibToGetRes;
  JetCleaningTool    * m_jetCleaning;
  JERTool            * m_jetResolution;
  JERSmearingTool    * m_jetResolutionSmearing;

  //JetUncertaintiesTool* m_jesProvider;
  std::unordered_map<std::string, JetUncertaintiesTool*> m_jesProviders;
  std::unordered_map<std::string, CP::SystematicSet> m_jes_sysSets;
  std::unordered_map<std::string, std::string> m_jes_prefixes;
  JetForwardJvtTool *m_fjvtTool;
  CP::JetJvtEfficiency *m_jvtCalib;
  CP::JetJvtEfficiency *m_fjvtCalib;
  std::unique_ptr<CP::JetQGTagger> m_qgtagger;
  CP::JetTileCorrectionTool *m_jetTileCorrectionTool;
  InDet::InDetTrackSelectionTool *m_trackSelectionTool;
  InDet::InDetTrackTruthFilterTool *m_trackFakeTool;//TRK_FAKE_RATE_LOOSE
  InDet::InDetTrackTruthOriginTool *m_trackOriginTool;
  InDet::JetTrackFilterTool *m_jetTrackFilterTool;//TRK_EFF_LOOSE_TIDE
  Trig::TrigDecisionTool *m_trigDecTool;
  std::unordered_map<std::string, PROP<int>*> m_triggersToMatch;
  asg::AnaToolHandle<IJetUpdateJvt>  m_JVTUpdateTool_handle{"JetVertexTaggerTool"};//!

  std::string m_jetAlgoName;
  std::string m_fjvtSF;
  TString m_sysName;
  bool m_isMC;
  bool m_applyJetSemileptonic;
  bool m_countElectronInJet;
  bool m_correctMuonInJet;
  bool m_saveSemileptonicInfoInJet;
  bool m_applyJetRegression;
  bool m_writeRegVars;
  bool m_matchTruthJet;
  bool m_storeGAParticlesInJets;
  bool m_doResolution;
  bool m_doFJVT;
  bool m_qgtagging;
  bool m_saveqgvariables;
  bool m_applyJetTileCorrection;
  bool m_isPFlow;
  bool m_enableLegacyJetCleaning;

  std::vector<std::string> m_btaggerNames;

  // selection functions
  bool passLooseJet(xAOD::Jet* jet);

  EL::StatusCode retrieveBTaggingDiscriminants(xAOD::Jet * jet);

  void BJetTriggerMatch(xAOD::Jet * jet);

  virtual EL::StatusCode decorate(xAOD::Jet * jet) override;
  virtual EL::StatusCode decorateFJVT(xAOD::Jet * jet);

  virtual EL::StatusCode calibrateCopies(xAOD::JetContainer * particles, const CP::SystematicSet & sysSet) override;

  virtual EL::StatusCode writeOutputVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool isKinVar, bool isWeightVar, const TString& sysName);
  virtual EL::StatusCode writeAllVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);  // turned off because of qgtagger issue.
  virtual EL::StatusCode writeKinematicVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool isKinVar, bool isWeightVar, const TString& sysName) override;

  typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> >  JetFourMom_t;

  bool IsChild(const xAOD::TruthParticle* child, std::vector<const xAOD::TruthParticle *> parents, std::vector<const xAOD::TruthParticle *> ancestors = {});

};

#endif
