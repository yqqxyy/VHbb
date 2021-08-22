// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_FatJetHandler_H
#define CxAODMaker_FatJetHandler_H

#include "TRandom3.h"

#include "CxAODMaker/ObjectHandler.h"

#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "MuonSelectorTools/MuonSelectionTool.h"
#include "MuonMomentumCorrections/MuonCalibrationPeriodTool.h"

//#include "BoostedJetTaggers/IJSSTagger.h"
#include "AsgTools/AnaToolHandle.h"
#include "JetAnalysisInterfaces/IJetSelectorTool.h"
#include "BoostedJetTaggers/SmoothedWZTagger.h"
#include "BoostedJetTaggers/JSSWTopTaggerBDT.h"

class JetSemileptonic;
class MuonHandler;
class JERTool;
class JERSmearingTool;
class JetUncertaintiesTool;
class JetCalibrationTool;

class FatJetHandler : public ObjectHandler<xAOD::JetContainer> {

public:

  FatJetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);

  ~FatJetHandler();

  EL::StatusCode initializeTools() override;
  EL::StatusCode clearEvent() override;

  virtual EL::StatusCode addCPVariations(const std::vector<TString> &variations,
                                         const bool filterByTools = true, const bool skipWeightVar = false) override;

  void setP4(const xAOD::Jet * in, xAOD::Jet * out) override { out->setJetP4( in->jetP4() ); }
  void setP4(const xAOD::Jet * in,  const std::string& name, xAOD::Jet * out) { out->setJetP4(name,in->jetP4(name) ); }

  // selection functions
  bool passPreSelection(xAOD::Jet* jet);

  virtual EL::StatusCode matchTruthJet(JetSemileptonic* m_jetSemileptonic, const xAOD::JetContainer* truthjets, const std::string& name, double DRCut);

  //truth matching for FJ flavor
  EL::StatusCode match_truth(const xAOD::JetContainer* truthjets);

protected:
  //Cut parameters
  float m_ptcut;
  float m_etacut;
  bool m_bosonTag;
  bool m_useR1Tagger;
  bool m_matchTruthJet;
  bool m_storeGAParticlesInFatJets;
  bool m_saveqgvariables;
  std::string m_algName;
  bool m_isMC;
  bool m_hasNoFJDetectorEta;
  bool m_isTCC;

  //Static accessors
  static SG::AuxElement::ConstAccessor<int> GhostTrackCount;
  static SG::AuxElement::Accessor<UInt_t> parentIndex;
  static SG::AuxElement::Accessor<std::vector<float>> TrackWidthPt500;
  static SG::AuxElement::ConstAccessor<int> GhostMuonSegmentCount;
  static SG::AuxElement::ConstAccessor<int> NSubjet;
  static SG::AuxElement::ConstAccessor<TLorentzVector> CorrectedJetP4;
  static SG::AuxElement::ConstAccessor<float> JetConstitScaleMomentum_eta;
  static SG::AuxElement::ConstAccessor<int> GhostHBosonsCount;
  static SG::AuxElement::ConstAccessor<int> GhostTQuarksFinalCount;
  static SG::AuxElement::ConstAccessor<int> GhostWBosonsCount;
  static SG::AuxElement::ConstAccessor<int> GhostZBosonsCount;

  void setParameters();

  // Tagging
  std::unique_ptr<SmoothedWZTagger> m_smoothedWTagger50;    //!
  std::unique_ptr<SmoothedWZTagger> m_smoothedWTagger80;    //!
  std::unique_ptr<SmoothedWZTagger> m_smoothedZTagger50;    //!
  std::unique_ptr<SmoothedWZTagger> m_smoothedZTagger80;    //!
  asg::AnaToolHandle<IJetSelectorTool> m_BDTWTagger50; //!


  std::unique_ptr<SmoothedWZTagger> m_TCC_WTagger_2var;    //!
  std::unique_ptr<SmoothedWZTagger> m_TCC_WTagger_3var;    //!

  std::unique_ptr<SmoothedWZTagger> m_TCC_ZTagger_2var;    //!
  std::unique_ptr<SmoothedWZTagger> m_TCC_ZTagger_3var;    //!

  std::unique_ptr<SmoothedWZTagger> m_TaggerForJES; //!

  bool m_doCalib;
  bool m_doMassCalib;
  JetCalibrationTool* m_jetCalibTool;

  std::unordered_map<std::string, JetUncertaintiesTool*> m_jetUncTools;
  std::unordered_map<std::string, CP::SystematicSet> m_jetUnc_sysSets;
  std::unordered_map<std::string, std::string> m_jetUnc_prefixes;

  //This is a tmp. hack
  JetUncertaintiesTool* m_jetUncC2Old;

  // custom resolution smearing methods
  // TODO: move to tool?
  TRandom3 m_rand;
  double getSmearFactor(double sigma, long int seed);
  CP::CorrectionCode applySmearingJER(xAOD::Jet& jet);
  CP::CorrectionCode applySmearingJMR(xAOD::Jet& jet);
  CP::CorrectionCode applySmearingSubR(xAOD::Jet& jet);

  //Helper function for binned substructure selection
  virtual void doVTag(xAOD::Jet* jet);
  int getpTBin(const xAOD::Jet* jet);
  bool IsChild(const xAOD::TruthParticle* child, std::vector<const xAOD::TruthParticle *> parents, std::vector<const xAOD::TruthParticle *> ancestors = {});

  virtual EL::StatusCode decorateOriginParticle(const xAOD::Jet * jet) override;
  virtual EL::StatusCode decorate(xAOD::Jet * jet) override;

  virtual EL::StatusCode fillOutputLinks() override;
  virtual EL::StatusCode flagOutputLinks() override;
  virtual EL::StatusCode calibrateCopies(xAOD::JetContainer * particles, const CP::SystematicSet & sysSet) override;

  virtual EL::StatusCode writeOutputVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeAllVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeKinematicVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::Jet* inJet, xAOD::Jet* outJet, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool isKinVar, bool isWeightVar, const TString& sysName) override;

  typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> >  JetFourMom_t;

  // for fatjet truth labelling
  int getTruthLabel(xAOD::Jet *recoJet, const xAOD::Jet *truthJet, std::string labelName);

  //for Hbb muon-in-jet correction
  EL::StatusCode getHbbCorrectedVector(const xAOD::Jet& jet, TLorentzVector& correctedVector, const bool doVR);
  EL::StatusCode decorateWithMuons(const xAOD::Jet& jet, const bool doVR) const;
  SG::AuxElement::Decorator<std::vector<ElementLink<xAOD::IParticleContainer>>> m_decMuonsInTrackJetLink;
  enum MuonCorrectionScheme {
    Combined = 0, // call Calorimeter and TrackAssisted scheme and then update the combined mass
    Calorimeter, // add muon 4-vector to the Calo jet after subtracting energy loss in the calorimeter
    TrackAssisted // call Calorimeter shceme and update TA jet using corrected Calo jet
  };
  const xAOD::JetFourMom_t getMuonCorrectedJetFourMom(const xAOD::Jet& jet, std::vector<const xAOD::Muon*> muons, MuonCorrectionScheme scheme, bool useJMSScale = false) const;
  CP::MuonSelectionTool* m_muonSelectionTool;
  CP::MuonCalibrationPeriodTool * m_muonCalibrationPeriodTool;
  float m_trackJetPtMin;
  float m_trackJetEtaMax;
  float m_trackJetNConst;
  float m_muonPtMin;
  float m_muonEtaMax;
  float m_muonDrMax;

};

#endif
