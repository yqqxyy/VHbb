#ifndef CxAODReader_AnalysisReader_VHQQ_H
#define CxAODReader_AnalysisReader_VHQQ_H

#include "CorrsAndSysts/BDTSyst.h"
#include "CorrsAndSysts/CorrsAndSysts.h"
#include "CxAODReader/AnalysisReader.h"
#include "CxAODReader_VHbb/MVAApplication_TMVA.h"
#include "TRandom3.h"

class EasyTree;
class MVATree_VHbb;
class MVATree_VBFHbb;
class MVATree_BoostedVHbb;
class OSTree;
namespace TMVA {
class Reader;
}
namespace KF {
class KinematicFit;
}
class ITMVAApplicationTool;
#include "CxAODReader/HistFastSvc.h"
#include "CxAODReader_VHbb/VHResRegions.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/OvCompat.h"

PROPERTY(Props, int,
         nBTagsAll)  // number of b-tagged track jets in leading fat jet
PROPERTY(Props, int, isMatchedToLeadFatJet)  // ghost-matched between track-jet
                                             // and leading fat-jet

namespace Ov {
class IAnalysisMain;
}

class sampleCS {
 public:
  sampleCS(std::string sampleName) { m_sampleName = sampleName; }

  void setup_sample(std::string sampleName, std::string detailsampleName) {
    if ((sampleName == "WlvH125") || (sampleName == "qqWlvH125") ||
        (sampleName == "qqWminuseveHNLO125") ||
        (sampleName == "qqWminusmuvmuHNLO125") ||
        (sampleName == "qqWpluseveHNLO125") ||
        (sampleName == "qqWplusmuvmuHNLO125"))
      m_sampleName = "WHlvbb";
    else if ((sampleName == "Zll125") || (sampleName == "qqZllH125"))
      m_sampleName = "qqZHllbb";
    else if ((sampleName == "ZvvH125") || (sampleName == "qqZvvH125"))
      m_sampleName = "qqZHvvbb";
    else if ((sampleName == "ggZeeH125") || (sampleName == "ggZmmH125") ||
             (sampleName == "ggZttH125") || (sampleName == "ggZllH125"))
      m_sampleName = "ggZHllbb";
    else if ((sampleName == "ggZveveH125") || (sampleName == "ggZvmvmH125") ||
             (sampleName == "ggZvtvtH125") || (sampleName == "ggZvvH125"))
      m_sampleName = "ggZHvvbb";

    else if (sampleName == "W")
      m_sampleName = "W";
    else if (sampleName == "Z")
      m_sampleName = "Z";
    else if (sampleName == "Wv22")
      m_sampleName = "W";
    else if (sampleName == "Zv22")
      m_sampleName = "Z";
    else if (sampleName == "ttbar")
      m_sampleName = "ttbar";
    else if (sampleName == "stops")
      m_sampleName =
          "stop_s";  // consistent with the name in the XSections_13TeV.txt
    else if (sampleName == "stopt")
      m_sampleName = "stop_t";
    else if (sampleName == "stopWt")
      m_sampleName = "stop_Wt";

    else if (sampleName == "WW")
      m_sampleName = "WW";
    else if (sampleName == "WZ")
      m_sampleName = "WZ";
    else if (sampleName == "ZZ")
      m_sampleName = "ZZ";

    else if ((sampleName == "dijetJZW") || (sampleName == "dijetZJ"))
      m_sampleName = "multijet";

    else
      m_sampleName = "NONAME";

    if ((sampleName == "WW") || (sampleName == "WZ") || (sampleName == "ZZ"))
      m_detailsampleName = detailsampleName;
    else
      m_detailsampleName = "NODETAILNAME";
  }  // setup_sample

  void set_sampleName(std::string sampleName) { m_sampleName = sampleName; }

  void set_detailsampleName(std::string detailsampleName) {
    m_detailsampleName = detailsampleName;
  }

  std::string get_sampleName() { return m_sampleName; }

  std::string get_detailsampleName() { return m_detailsampleName; }

 private:
  std::string m_sampleName;
  std::string m_detailsampleName;
};

enum lepFlav { mu, el };

enum class Model { undefined, AZh, HVT, CUT, MVA };

struct Lepton {
  TLorentzVector vec;
  TLorentzVector vec_corr;
  TLorentzVector vec_resc;

  TLorentzVector vec_kf;

  TLorentzVector vecT;  // For 1lepton W
  int charge;
  lepFlav flav;
  bool isZHTight = true;
  float resolution;
};

struct Jet {
  TLorentzVector vec;
  TLorentzVector vec_corr;  // jets are corrected for PtReco and Muon in jet
  TLorentzVector vec_resc;

  TLorentzVector vec_gsc;
  TLorentzVector vec_onemu;
  TLorentzVector vec_ptreco;
  TLorentzVector vec_fsr;
  TLorentzVector vec_kf;
  TLorentzVector vec_truthwz;

  int nrMuonInJet04 = 0;
  int nrMuonInJet = 0;
  float ptMuonInJet04 = 0.0;
  float ptMuonInJet = 0.0;
  float dRMuonInJet04 = 0.0;
  float dRMuonInJet = 0.0;
  float PtRatioOneMuonInJet = 0.0;
  int nrElectronInJet04 = 0;
  int nrElectronInJet = 0;
  float PtRatioOneElectronInJet = 0.0;
  float ptGreyElectronInJet = 0.0;
  float dRGreyElectronInJet = 0.0;

  bool isTagged = false;
  bool isMu04 = false;
  bool isMu = false;
  bool isEl04 = false;
  bool isEl = false;
  bool isSL04 = false;
  bool isSL = false;
  int nBTags =
      0;  // for fat-jets this would be # of ghost-matched b-tagged track-jets
  float resolution = 0.0;
};

struct MET {
  TLorentzVector vec;
  TLorentzVector vec_corr;  // MET is calculated using jets corrected by PtReco,
                            // Muon in jet
  TLorentzVector vec_resc;  // MET is calculated using the mBB re-weighted jets
};

struct Higgs {
  TLorentzVector vec;
  TLorentzVector vec_corr;
  TLorentzVector vec_resc;

  TLorentzVector vec_gsc;
  TLorentzVector vec_onemu;
  TLorentzVector vec_ptreco;
  TLorentzVector vec_fsr;
  TLorentzVector vec_kf;
  TLorentzVector vec_truthwz;
};

struct VCand {
  TLorentzVector vecT;      // lep.vec + met.vec
  TLorentzVector vec;       // lep.vec + nu.vec
  TLorentzVector vec_corr;  // lep.vec + nu.vec_corr
  TLorentzVector vec_resc;  // lep.vec + nu.vec_resc
};

struct bbjSystem {
  TLorentzVector vecT;
  TLorentzVector vec;
  TLorentzVector vec_corr;
  TLorentzVector vec_resc;
};

struct VHCand {
  TLorentzVector vec;
  TLorentzVector vec_corr;
  TLorentzVector vec_resc;

  TLorentzVector vec_gsc;
  TLorentzVector vec_onemu;
  TLorentzVector vec_ptreco;
  TLorentzVector vec_fsr;
  TLorentzVector vec_kf;

  TLorentzVector Vvec;
  TLorentzVector VvecT;
};

struct PhysicsMetadata {
  enum class Analysis {
    undefined,
    cutbased,
    MVA
  } analysis = Analysis::undefined;
  enum class Channel {
    undefined,
    TwoLep,
    OneLep,
    ZeroLep
  } channel = Channel::undefined;
  enum class Flavor {
    undefined,
    MuMu,
    ElEl,
    MuEl,
    ElMu,
    Mu,
    El
  } flavor = Flavor::undefined;
  enum class Regime {
    undefined,
    SMVH,
    resolved,
    merged
  } regime = Regime::undefined;
  enum class Region {
    undefined,
    SR,
    WCR,
    topCR,
    mbbCR
  } region = Region::undefined;
  enum class MbbSideBandResolved {
    undefined,
    Outer,
    Low,
    High
  } mbbSideBandResolved = MbbSideBandResolved::undefined;
  enum class MbbSideBandMerged {
    undefined,
    Outer,
    Low,
    High
  } mbbSideBandMerged = MbbSideBandMerged::undefined;
  int nJets = 0;
  int nSigJet = 0;
  int nForwardJet = 0;
  int nTags = 0;  // number of tagged small-R jets
  int b1Flav = 0;
  int b2Flav = 0;
  int j3Flav = 0;
  int nTagsInFJ = 0;  // number of lead / sublead b-tagged track jets associated
                      // to leading fat jet
  int nAddBTrkJets =
      0;  // number of b-tagged track jets not associated to leading fat jet
  int nTrackJet = 0;
  int nFatJet = 0;
};

class AnalysisReader_VHQQ : public AnalysisReader {
 protected:
  virtual EL::StatusCode initializeSelection() override;
  virtual EL::StatusCode initializeVariations() override;
  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode finalizeTools() override;
  EL::StatusCode SetupCorrsAndSyst(std::string currentVar, bool isCR);
  void fillTTbarSystslep(int nTag, bool isCR, float mVH);
  void fillTopSystslep_PRSR(std::string Regime, int nTag, float mBB, float mVH,
                            bool isEMu = false);
  void fillVjetsSystslep(double Mbb, float mVH);
  void fillVjetsSystslep_PRSR(std::string Regime, double Mbb, float mVH,
                              int nAddBTags = 0);

  EL::StatusCode initializeCorrsAndSysts();

  // used for histfastsvc
  virtual EL::StatusCode histFinalize() override;

  EL::StatusCode fill_MVAVariablesHistos();
  EL::StatusCode fill_bTaggerHists(const xAOD::Jet *jet);
  EL::StatusCode fill_FtagEff_Hists(const xAOD::Jet *jet, float eventweight,
                                    std::string hist_type);
  EL::StatusCode fill_nJetHistos(const std::vector<const xAOD::Jet *> &jets,
                                 const string &jetType);
  EL::StatusCode fill_fatJetHistos(
      const std::vector<const xAOD::Jet *> &fatJets);
  EL::StatusCode fill_jetSelectedHistos();
  EL::StatusCode compute_jetSelectedQuantities(
      const std::vector<const xAOD::Jet *> &selectedJets);
  EL::StatusCode fill_TLV(std::string name, const TLorentzVector &vec,
                          double weight, bool isE = false, bool isMu = false);
  EL::StatusCode fillStage1Bin_RecoCategory();
  EL::StatusCode fillStage1Bin_RecoCategory_EPS();
  EL::StatusCode fillStage1Bin_RecoCategory_Simp();
  EL::StatusCode fillStage1Bin_RecoCategory_1lep();
  int RetrieveStage1Bin();
  int RetrieveHTXSNJET();
  float RetrieveHTXSPTV();
  std::string TranslateStage1Bin();

  std::tuple<TLorentzVector, TLorentzVector> calculateFSR(
      TLorentzVector B1, TLorentzVector B2, TLorentzVector J3,
      double ptv_for_FSR = -999.);
  void migrateCategoryFSR(std::vector<const xAOD::Jet *> &signalJets,
                          std::vector<const xAOD::Jet *> &forwardJets);

  // EL::StatusCode fill_VBF0ph ();
  // EL::StatusCode fill_VBF1ph ();
  // EL::StatusCode fill_VBFIncl ();
  // EL::StatusCode fill_vbf1ph ();
  // EL::StatusCode fill_VBFInclCutFlow (std::string label);
  EL::StatusCode STXS_FillYields();
  EL::StatusCode STXS_FillYields_Boosted();
  int STXS_GetBinFromEvtInfo();
  int STXS_GetNJetFromEvtInfo();
  float STXS_GetPtVFromEvtInfo();
  std::string STXS_ParseBin();
  // ---
  float compute_JVTSF(const std::vector<const xAOD::Jet *> &signalJets);
  void compute_btagging();
  void compute_TRF_tagging(const std::vector<const xAOD::Jet *> &signalJets,
                           const std::vector<const xAOD::Jet *> &signalJetsDT =
                               {},                    // added for boosted TT
                           bool useTrackJets = false  // added for boosted TT
  );
  void compute_fatjetTags(const std::vector<const xAOD::Jet *> &trackJets,
                          const std::vector<const xAOD::Jet *> &fatJets,
                          std::vector<const xAOD::Jet *> *tagTrackJetsInLeadFJ,
                          std::vector<const xAOD::Jet *> *trackJetsNotInLeadFJ);
  void matchTrackJetstoFatJets(
      const std::vector<const xAOD::Jet *> &trackJets, const xAOD::Jet *fatjet,
      std::vector<const xAOD::Jet *> *trackJetsInFatJet,
      std::vector<const xAOD::Jet *> *trackJetsNotInFatJet);
  void CoMtagging_LeadFJ(const std::vector<const xAOD::Jet *> subJets,
                         const xAOD::Jet *fatJets,
                         std::vector<const xAOD::Jet *> &trackJetsNotInFatJet,
                         int &nBtags);
  void setevent_flavour(const std::vector<const xAOD::Jet *> &selectedJets);
  // set flavour using GhostMatching
  void setevent_flavourGhost(
      const std::vector<const xAOD::Jet *> &selectedJets);

  void setevent_nJets(const std::vector<const xAOD::Jet *> &signalJets,
                      const std::vector<const xAOD::Jet *> &forwardJets);

  void setevent_nJets(const std::vector<const xAOD::Jet *> &signalJets,
                      const std::vector<const xAOD::Jet *> &forwardJets,
                      const std::vector<const xAOD::Jet *> &fatJets);

  std::string determine_regime();

  void truthVariablesCS(float &truthPt, float &avgTopPt);

  EL::StatusCode applyCS(float VpT, float MET, float nJet, float nTag,
                         const TLorentzVector &jet1, const TLorentzVector &jet2,
                         bool passCutBased = false, float Mtop = 0);

  EL::StatusCode applyCS(float VpT, float Mbb, float truthPt, float DeltaPhi,
                         float DeltaR, float pTB1, float pTB2, float MET,
                         float avgTopPt, int njet, int ntag,
                         bool passCutBased = false, float Mtop = 0);

  EL::StatusCode applyCS(float VpT, float mJ, int naddcalojet);

  EL::StatusCode applyCorrections();

  EL::StatusCode applySystematics(float VpT, float Mbb, float truthPt,
                                  float DeltaPhi, float DeltaR, float pTB1,
                                  float pTB2, float MET, float avgTopPt,
                                  int njet, int ntag, bool passCutBased = false,
                                  float Mtop = 0);

  EL::StatusCode applySystematics(float VpT, float MET, float nJet, float nTag,
                                  const TLorentzVector &jet1,
                                  const TLorentzVector &jet2,
                                  bool passCutBased = false, float Mtop = 0);

  std::vector<std::pair<TString, Float_t>> ComputeBDTSystematics(
      int channel, float btagWeight, int nJet, int nTag);

  void tagjet_selection(std::vector<const xAOD::Jet *> &signalJets,
                        std::vector<const xAOD::Jet *> &forwardJets,
                        std::vector<const xAOD::Jet *> &selectedJets,
                        int &tagcatExcl);

  void tagTrackjet_selection(
      const xAOD::Jet *fatJet, std::vector<const xAOD::Jet *> trackJetsInFatJet,
      std::vector<const xAOD::Jet *> trackJetsNotInFatJet,
      std::vector<const xAOD::Jet *> &selectedtrackJetsInFatJet,
      std::vector<const xAOD::Jet *> &bTaggedUnmatchedTrackJets,
      std::vector<const xAOD::Jet *> &trackJetsForBTagSF, int &tagcatExcl,
      int &nAddBTags);

  bool passVRJetOR(std::vector<const xAOD::Jet *> trackJetsForBTagSF);

  void set_BtagProperties_fatJets(
      const xAOD::Jet *fatJet, std::vector<const xAOD::Jet *> trackJetsInFatJet,
      std::vector<const xAOD::Jet *> trackJetsNotInFatJet);

  // Multiple event weight application function
  // NOTE:: m_EvtWeightVar is defined and initialised in
  // CxAODReader/AnalysisReader.cxx,
  //       this function simply applies variations that are defined within the
  //       config
  EL::StatusCode apply_EvtWeights();
  EL::StatusCode apply_TTbarPwPy8VariationWeightProtection(
      float &variationWeight, float nominalWeight,
      float relativeThreshold = 10.0);

  // returns true if dilep event
  // needed for merging dilepton-filtered and non-all had / inclusive ttbar and
  // single top Wt events to be called in channel specific readers
  int vetoDilepTtbarEvents();
  int vetoDilepWtEvents();

  // a function to check whether an event passes
  // all the cuts up to "cut" - possibility to exclude some cuts
  bool passAllCutsUpTo(const unsigned long int flag,
                       const unsigned long int cut,
                       const std::vector<unsigned long int> &excludeCuts = {});

  // a function to check whether an event passes a specific set of cuts
  bool passSpecificCuts(const unsigned long int flag,
                        const std::vector<unsigned long int> &cuts);

  // a function to create the bit mask
  unsigned long int bitmask(
      const unsigned long int cut,
      const std::vector<unsigned long int> &excludeCuts = {});

  // a function to update the bit flag
  void updateFlag(unsigned long int &flag, const unsigned long int cutPosition,
                  const unsigned long int passCut = 1);

  void rescale_jets(double mBB, TLorentzVector &j1Vec, TLorentzVector &j2Vec);

  void rescale_fatjet(double mBB, TLorentzVector &fatjet);

  void rescale_leptons(double mLL, TLorentzVector &l1Vec,
                       TLorentzVector &l2Vec);

  // Remove fatjets that overlap with muon
  std::vector<const xAOD::Jet *> ApplyMuonFatJetOR(
      std::vector<const xAOD::Muon *> Muons,
      std::vector<const xAOD::Jet *> fatJets);

  bool checkTightLeptons(const Lepton &l1, const Lepton &l2);

  void computeHT(double &HTsmallR, double &HTlargeR, const Lepton &l1,
                 const Lepton &l2,
                 const std::vector<const xAOD::Jet *> &signalJets,
                 const std::vector<const xAOD::Jet *> &forwardJets,
                 const std::vector<const xAOD::Jet *> &fatJets);

  TLorentzVector getMETCorrTLV(
      const xAOD::MissingET *met,
      std::vector<const TLorentzVector *> removeObjects,
      std::vector<const TLorentzVector *> addObjects);

  EL::StatusCode setJetVariables(
      Jet &j1, Jet &j2, Jet &j3,
      const std::vector<const xAOD::Jet *> &selectedJets,
      double ptv_for_FSR = -999.);

  EL::StatusCode fillMVATreeVHbbResolved(
      TLorentzVector &b1, TLorentzVector &b2, TLorentzVector &j3,
      const std::vector<const xAOD::Jet *> &selectedJets,
      const std::vector<const xAOD::Jet *> &signalJets,
      const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &HVec,
      TLorentzVector &VVec, TLorentzVector &metVec, int nTaus);

  EL::StatusCode fillOSTree();  // sgargiul 11.07.2019

  EL::StatusCode setFatJetVariables(
      Jet &fj1, Jet &fj2, Jet &fj3,
      const std::vector<const xAOD::Jet *> &fatJets);
  EL::StatusCode setFatJetVariablesSingle(Jet &fj, const xAOD::Jet *fatJet);

  unsigned int countAdditionalCaloJets(
      std::vector<const xAOD::Jet *> &signalJets,
      std::vector<const xAOD::Jet *> &forwardJets, Jet fj1);

  void setHiggsCandidate(Higgs &resolvedH, Higgs &mergedH, Jet j1, Jet j2,
                         Jet fj1);

  TLorentzVector defineMinDRLepBSyst(const Lepton &l1, const Lepton &l2,
                                     const Jet &j1, const Jet &j2);

  bool isBlindedRegion(const unsigned long int eventFlag,
                       bool isMerged = false);

  EL::StatusCode fill_jetHistos(std::vector<const xAOD::Jet *> signalJets,
                                std::vector<const xAOD::Jet *> forwardJets);
  EL::StatusCode fill_jetHistos (std::vector<const xAOD::Jet*> jets, std::vector<const xAOD::Jet*> signalJets, std::vector<const xAOD::Jet*> forwardJets);

  double getSMSigVPTSlice();
  // float computeBTagSFWeight (std::vector<const xAOD::Jet*> &signalJets);
  //

  std::string m_analysis;                    // !
  std::string m_analysisType;                // !
  std::string m_analysisStrategy;            // !
  bool m_doTruthTagging;                     // !
  bool m_doMbbWindow = true;                 // !
  bool m_doFillCR = false;                   // !
  bool m_doOnlyInputs = false;               // !
  bool m_doNewRegions = false;               // !
  bool m_doBootstrap = false;                // !
  bool m_doReduceFillHistos = false;         // !
  bool m_GenerateSTXS = false;               // !
  MVATree_VHbb *m_tree = nullptr;            // !
  MVATree_VBFHbb *m_tree_vbf = nullptr;      // !
  EasyTree *m_etree = nullptr;               // !
  OSTree *m_OStree = nullptr;                // !
  PhysicsMetadata m_physicsMeta;             // !
  bool m_doBlinding = false;                 //!
  bool m_doMergeJetBins = false;             //!
  bool m_doMergePtVBins = false;             //!
  std::string m_jetCorrType;                 //!
  std::string m_fatJetCorrType;              //!
  bool m_doFSRrecovery;                      //!
  float m_hasFSR;                            //!
  std::string m_tagStrategy;                 //!
  std::string m_boostedTagStrategy;          //!
  std::vector<std::string> m_csCorrections;  //!
  std::vector<std::string> m_csVariations;   //!
  std::string m_kfConfig;                    //!

  // VHbb MVA applications
  MVAApplication_TMVA *m_mvaVHbbFullRun2 = nullptr;              //!
  MVAApplication_TMVA *m_mvaVHbbFullRun2VZ = nullptr;            //!
  MVAApplication_TMVA *m_mvaVHbbFullRun2PCBT = nullptr;          //!
  MVAApplication_TMVA *m_mvaVHbbFullRun2oldDefault = nullptr;    //!
  MVAApplication_TMVA *m_mvaVHbbFullRun2oldDefaultVZ = nullptr;  //!
  std::map<std::string, MVAApplication_TMVA *> m_mvaVHbbApps;    //!

  std::vector<std::string> m_BDTSystVariations;               //!
  std::vector<std::pair<TString, Float_t>> m_BDTSyst_scores;  //!

  BDTSyst *m_BDTSyst;  //!

  // Function for 2/3jet defined from 10%on WZ and 95%/85% of the signal
  TF1 *m_fit_func_2j10;
  TF1 *m_fit_func_3j10;
  TF1 *m_fit_func_2j95;
  TF1 *m_fit_func_3j85;

  // Merged Only V+Jets and TTbar systematics
  // Used for MODEL systematics on V+Jets and TTbar
  CAS::EventType m_evtType;              //!
  CAS::DetailEventType m_detailevtType;  //!
  CAS::Systematic m_Systematic;          //!

  KF::KinematicFit *m_KF;  //!

  Ov::IAnalysisMain *m_analysisContext;  //!

  bool m_use2DbTagCut;
  bool m_doMETTriggerin2L = false;  //!
  bool m_doMETMuonTrigger = false;  //!

  // for boosted analysis
  bool m_doFillHistograms;  //!
  bool m_useFastHistSvc;    //!
  bool m_doMergeCR;         //!
  bool m_doCutflow;         //!
  bool m_BDTSyst_debug;     //!
  bool m_doMbbMonitor;      //!
  bool m_doKF;              //!
  bool m_doBinnedSyst;      //!
  bool m_doSherpaSyst;      //!
  bool m_doVRJetOR;
  // remove overlap between dilep and inclusive / non-allhad top samples;
  bool m_SplitBoostedInputsAddJets;
  bool m_doRemoveDilepOverlap;              //!
  MVATree_BoostedVHbb *m_boostedtree;       //!
  HistFastSvc<VHResRegions> m_histFastSvc;  //!
  HistoVec m_fatJetHistos;                  //!
  bool m_isStoreMJwC2;                      //!
  std::map<std::string, float> m_c2Cuts;    //!

  Model m_model;

  static bool sort_pt(const xAOD::Jet *jetA, const xAOD::Jet *jetB) {
    return jetA->pt() > jetB->pt();
  }

  /// Beginning of functions and branches required for object vector saving
  EL::StatusCode initialiseObjectBranches();
  EL::StatusCode finaliseObjectBranches();
  EL::StatusCode defineObjectBranches();
  EL::StatusCode resetObjectBranches();
  EL::StatusCode fillObjectBranches(
      const std::vector<const xAOD::Jet *> &jets,
      // const std::vector<const xAOD::Electron*> &electrons,
      // const std::vector<const xAOD::Muon*> &muons,
      // const std::vector<const xAOD::TauJet*> &taus,
      // const xAOD::JetContainer *jets,
      const xAOD::ElectronContainer *electrons,
      const xAOD::MuonContainer *muons, const xAOD::TauJetContainer *taous,
      const xAOD::MissingET *met);

  bool m_writeObjectsInEasyTree;
  std::vector<float> *vIn_jet_pt;
  std::vector<float> *vIn_jet_eta;
  std::vector<float> *vIn_jet_phi;
  std::vector<float> *vIn_jet_e;
  std::vector<float> *vIn_jet_mv2c10;
  std::vector<float> *vIn_jet_mv2c100;
  std::vector<float> *vIn_jet_mv2cl100;
  std::vector<float> *vIn_jet_DL1_pb;
  std::vector<float> *vIn_jet_DL1_pc;
  std::vector<float> *vIn_jet_DL1_pu;
  std::vector<float> *vIn_jet_jvt;
  std::vector<int> *vIn_jet_truthflav;

  // electrons
  std::vector<float> *vIn_el_pt;
  std::vector<float> *vIn_el_eta;
  std::vector<float> *vIn_el_phi;
  std::vector<float> *vIn_el_e;
  std::vector<float> *vIn_el_charge;

  // muons
  std::vector<float> *vIn_mu_pt;
  std::vector<float> *vIn_mu_eta;
  std::vector<float> *vIn_mu_phi;
  std::vector<float> *vIn_mu_e;
  std::vector<float> *vIn_mu_charge;

  // taus
  std::vector<float> *vIn_tau_pt;
  std::vector<float> *vIn_tau_eta;
  std::vector<float> *vIn_tau_phi;
  std::vector<float> *vIn_tau_e;
  std::vector<float> *vIn_tau_charge;

  // met
  float m_met_met;
  float m_met_phi;

 public:
  AnalysisReader_VHQQ();

  std::string STXS_ParseBin(int stage1, int DSID, int baseline, float ptv,
                            int njet);
  std::string STXS_ParseBin_Stage1PP(int stage1, int DSID, int baseline,
                                     float ptv, int njet);
  EL::StatusCode STXS_FillYields(HistSvc *histSvc, string sample_name,
                                 string stage1_name, string category_name,
                                 double weight, int baseline,
                                 bool isMerged = false);
  // EL::StatusCode STXS_ParseBin_Stage1PP ( HistSvc *histSvc, string
  // sample_name, string stage1_name, string category_name, double weight, int
  // baseline );
  EL::StatusCode STXS_ReplaceName();

  std::string TranslateStage1Bin(int stage1, int DSID, bool baseline, float ptv,
                                 int njet);

  EL::StatusCode fillStage1Bin_RecoCategory(HistSvc *histSvc,
                                            string sample_name,
                                            string stage1_name,
                                            string category_name,
                                            double weight);

  EL::StatusCode fillStage1Bin_RecoCategory_EPS(HistSvc *histSvc,
                                                string sample_name,
                                                string stage1_name,
                                                string category_name,
                                                double weight, bool baseline);

  EL::StatusCode ReplaceSigNameWithSTXS_Stage1();

  // emulating c-tag systematics for VHcc
  EL::StatusCode emulateCTagSystematics(
      std::vector<const xAOD::Jet *> selectedJets);
  float getCTagSF(int flav, int syst = 1);
  float getCTagSFWeight(const xAOD::Jet *jet, int syst);
  std::vector<std::pair<std::string, int>> m_cTagSystVec = {
      make_pair("CTag_C__1up", 1), make_pair("CTag_C__1down", -1),
      make_pair("CTag_L__1up", 2), make_pair("CTag_L__1down", -2),
      make_pair("CTag_B__1up", 3), make_pair("CTag_B__1down", -3),
  };

  // compute min dR reweighting scale factor based on flavJ1, flavJ2 and minimum
  // dR(J1,J2)
  EL::StatusCode compute_TT_min_deltaR_scale_factor(const string &filename,
                                                    const int &flav_j1,
                                                    const int &flav_j2,
                                                    const float &min_dR,
                                                    float &scale_factor);

  EL::StatusCode compute_TT_min_deltaR_correction(
      std::vector<const xAOD::Jet *> selectedJets,
      std::map<std::string, std::map<int, float>> &btag_TT_weights,
      const bool &do_btag_Syst);

  virtual ~AnalysisReader_VHQQ();

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHQQ, 1);

 private:
  // combMass begin

  virtual EL::StatusCode combMassTmp1(bool &isCombMassSyst,
                                      std::string &varName) override;
  virtual EL::StatusCode combMassTmp2(bool &isCombMassSyst) override;

  double getSmearFactor(double sigma, long int seed);
  CP::CorrectionCode applySmearingJER(xAOD::Jet &jet);
  CP::CorrectionCode applySmearingJMR(xAOD::Jet &jet);

  std::vector<std::string> m_combMassSyst;  //!
  TRandom3 m_rand;                          //!
  CorrsAndSysts *m_corrsAndSysts;           //!

 public:
  std::vector<const xAOD::Jet *> m_allLeadFJMatchedTrackJets;
  // combMass end
};

#endif  // ifndef CxAODReader_AnalysisReader_VHQQ_H
