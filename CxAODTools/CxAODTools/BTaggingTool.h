#ifndef CxAODTools_BTaggingTool_H
#define CxAODTools_BTaggingTool_H

// TODO: obsolete
/**
 * Usage:
 * ======
 *
 * - declare tool in header:
 *   BTaggingTool m_btagtool;
 *
 * - in event loop, pass xAOD::Jets to the tool and get efficiency / scale
 * factor through e.g. float btag_eff = btagtool.getEfficiency( *jet ); float
 * btag_sf  = btagtool.getScaleFactor( *jet );
 *
 *   ==> returns -1 in both cases if the result shouldn't be used in further
 * analysis
 *
 * - you can also pass a jet container and a JetDecorator to get the scale
 * factors for all jets in it m_btagtool.getScaleFactor( jets, deco ); // "jets"
 * is an xAOD:::JetContainer, "deco" is a JetDecorator
 *
 * - Get list of systematic variations
 *   CP::SystematicsSet syst = m_btagtool.affectingSystematics();
 *
 * - Apply systematic variations
 *   m_btagtool.applySystematicVariation(const CP::SystematicSet & sysSet);
 *
 *
 * - final comment: This header should not be included in places visible to
 * CINT. In some cases, it may therefore be better to declare a pointer to the
 * tool and simply make a forward declaration in order to hide this header from
 * CINT.
 */

// Infra structure includes
// - have to include the header (instead of forward declaration of xAOD::Jet)
//   since xAOD::Jet is a typedef, i.e. not a class, hence cannot forward
//   declare...
#include "xAODBTagging/BTagging.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

// External tool (having only a forward declaration here makes linking fail on
// the grid, apparently)
#include "PATInterfaces/SystematicSet.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingTruthTaggingTool.h"

#include "CxAODTools/ConfigStore.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODTools/XSectionProvider.h"
#include "EventLoop/StatusCode.h"

class BTaggingTool {
 public:
  using Config_t = std::map<std::string, std::string>;

  // constructor
  BTaggingTool() = default;

  // destructor
  ~BTaggingTool() = default;

  // initialize external tool
  StatusCode initialize(const Config_t &, const bool &use2DbTagCut = false,
                        const bool &uncorrelate_run1_to_run2 = false,
                        bool useQuntile = false, int maxTruthTag = 2,
                        const std::string &fileNameCDI =
                            "2016-20_7-13TeV-MC15-CDI-2017-01-31_v1");

  void setWeightVar(const bool &flag = true) { m_doVar = flag; }
  BTaggingTool &setExcludedBTagEV(const std::string &list);
  BTaggingTool &setTagger(const std::string &tagger);
  BTaggingTool &setJetAuthor(const std::string &author);
  BTaggingTool &setTaggingScheme(const std::string &scheme);
  BTaggingTool &setTaggerEfficiency(const int &eff);
  BTaggingTool &setMCIndex(const char *fileName,
                           XSectionProvider &xSectionProvider);
  BTaggingTool &setMCIndex(const int &mcChannel,
                           XSectionProvider &xSectionProvider);
  bool doWeightVar() const noexcept { return m_doVar; }
  std::string getTagger() const noexcept { return m_tagger; }
  std::string getJetAuthor() const noexcept { return m_jetAuthor; }
  std::string getBtaCutEff() const noexcept { return m_btagcuteff; }
  std::string getTaggingScheme() const noexcept { return m_scheme; }
  std::string getWorkingPoint() const noexcept { return m_workingPoint; }
  int getTaggerEfficiency() const noexcept { return std::stoi(m_btagcuteff); }
  unsigned getMCIndex() const noexcept { return m_MCIndex; }

  // get efficiency and scale factors for single jet
  float getTaggerWeight(const xAOD::Jet &jet);

  float getEfficiency(const xAOD::Jet &jet);
  float getInefficiency(const xAOD::Jet &jet);
  float getMCEfficiency(const xAOD::Jet &jet);
  void getEfficiency(const xAOD::Jet &jet, float &eff) {
    eff = getEfficiency(jet);
  }
  void getInefficiency(const xAOD::Jet &jet, float &eff) {
    eff = getInefficiency(jet);
  }
  void getMCEfficiency(const xAOD::Jet &jet, float &eff) {
    eff = getMCEfficiency(jet);
  }

  float getScaleFactor(const xAOD::Jet &jet);
  float getInefficiencyScaleFactor(const xAOD::Jet &jet);
  void getScaleFactor(const xAOD::Jet &jet, float &sf) {
    sf = getScaleFactor(jet);
  }
  void getInefficiencyScaleFactor(const xAOD::Jet &jet, float &sf) {
    sf = getInefficiencyScaleFactor(jet);
  }

  float getFullEfficiency(const xAOD::Jet &jet, bool tagged);
  float getFullEfficiency(const xAOD::Jet &jet) {
    return getFullEfficiency(jet, isTagged(jet));
  }
  float getFullScaleFactor(const xAOD::Jet &jet, bool tagged);
  float getFullScaleFactor(const xAOD::Jet &jet) {
    return getFullScaleFactor(jet, isTagged(jet));
  }

  // get scale factors for a vector of jets
  // - attaches the scale factor as a decoration to each jet
  void getScaleFactor(const xAOD::JetContainer &jets);

  // is b-jet?
  bool isTagged(const xAOD::Jet &jet);
  // continuous tagging
  int getQuantile(const xAOD::Jet &jet);

  // get list of systematic variations
  CP::SystematicSet affectingSystematics();

  // apply systematic variation
  bool applySystematicVariation(const CP::SystematicSet &sysSet);

  // set nominal (switch off systematics)
  bool applyNominal();

 private:
  bool isJetTagged(const xAOD::Jet &jet)
      const;  // an internal version of the non-const isTagged
  float getJetTaggerWeight(const xAOD::Jet &jet, bool Get_vetoTagger = false)
      const;  // an internal version of the non-const getTaggerWeight
 public:
  // returns the a map of the nominal and all systematic
  // event weights keyed by the systematic name including "Nominal" for the
  // nominal
  std::map<std::string, float> computeEventWeight(
      const std::vector<const xAOD::Jet *> &xaodjets);
  std::map<std::string, float> computeEventWeight(xAOD::JetContainer &xaodjets);

  std::map<std::string, float> computeEventWeight_truthTag(ConfigStore *m_config);



  // used in VHcc only ///////////////////////////////////
  void tagjet_selection_vhcc(std::vector<const xAOD::Jet *> &signalJets,
                        std::vector<const xAOD::Jet *> &selectedJets,
                        int &tagcatExcl, int &nSubleadingTags);

  void tagjet_selection_vhcc(std::vector<const xAOD::Jet *> &signalJets, std::vector<const xAOD::Jet *> &fwdJets,
                        std::vector<const xAOD::Jet *> &selectedJets,
                        int &tagcatExcl, int &nSubleadingTags);

  std::map<std::string,std::map<int,int>> m_sample_to_map_index;
  void FillVHccIndexMap();
  void setMCIndex_VHcc(const int &mcChannel,std::string period);
  //this function computes the event weights (direct tag and truth tag) for the VHcc style of doing truth tagging 
  //(filling each event to multiple histograms, 0, 1 and 2 tag, and whatever direct tag histograms )
  //it fill the map eventweights with event weights for 0,1,2 truth tags, and the -1 entry is for the direct tag histgoram (whatever that is)
  std::map< std::string, std::map<int, float> > compute_TruthTag_EventWeight_VHcc(std::vector<const xAOD::Jet*> &xaodjets,bool doSyst=false, const float& correction_dR_jet1 = 1, const float& correction_dR_jet2 = 1);
  //helper function for VHcc event weight computation
  void GetVhccEffs(std::vector<float> &jet_effs, std::vector<float> &jet_ineffs,std::shared_ptr<BTaggingEfficiencyTool> efftool_nominal, 
                                    std::shared_ptr<BTaggingEfficiencyTool> efftool_veto,std::vector<const xAOD::Jet*> &xaodjets,
                                    float &veto_ineff_sf, float &direct_tag_weight);
  ///////////////////////////////////////////////////////


  
  unsigned indexMCEfficiencyFromFileName(const std::string &name);
  unsigned indexMCEfficiencyFromChannel(const int &mcChannel,
                                        XSectionProvider &xSectionProvider);
  unsigned get_MC_index_from_string(const std::string &name,
                                    const unsigned &defaultMCIndex);

  void truth_tag_jets(unsigned long long eventNumber,
                      const xAOD::JetContainer &m_signalJets,
                      ConfigStore *m_config,
		      const xAOD::JetContainer &signalJetsDT = xAOD::JetContainer()); // TT for boosted VHbb, jets that we want to tag with direct tagging even when running with TT
  void truth_tag_jets(unsigned long long eventNumber,
                      const std::vector<const xAOD::Jet *> &m_signalJets,
                      ConfigStore *m_config,
		      const std::vector<const xAOD::Jet *> &signalJetsDT = {} // TT for boosted VHbb, jets that we want to tag with direct tagging even when running with TT  
		      );
 
  inline BTaggingEfficiencyTool &getBTaggingEfficiencyTool() {
    if (auto tool = m_currentEfficiencyTool.lock())
      return *tool;
    else {
      Error("BTaggingTool::getBTaggingEfficiencyTool()",
            "tool is not initalized!");
      return *m_currentEfficiencyTool.lock();
    }
  }
  inline BTaggingSelectionTool &getBTaggingSelectionTool() {
    if (auto tool = m_currentSelectionTool.lock())
      return *tool;
    else {
      Error("BTaggingTool::getBTaggingSelectionTool()",
            "tool is not initalized!");
      return *m_currentSelectionTool.lock();
    }
  }

  void PrintConfiguration() {
    if (!m_currentEfficiencyTool.expired() && !m_currentSelectionTool.expired())
      Info("setTaggerEfficiency",
           "BTaggingTool configuration: Algorithm: %s, "
           "Author: %s, Efficiency: %s, Scheme: %s",
           m_tagger.c_str(), m_jetAuthor.c_str(), m_btagcuteff.c_str(),
           m_scheme.c_str());
  }
  float get_jet_truthtag_btagscore(const xAOD::Jet &jet,
                                   unsigned long long eventNumber,
                                   bool is_tagged, int ratio = -1);
  void getBinFromEfficiency(int &effbin) const;

 private:
 
  StatusCode initializeTruthTagTool(
      std::shared_ptr<BTaggingTruthTaggingTool> &tt_ptr, const std::string &alg,
      const std::string &author, const std::string &wp);
  StatusCode initializeEffTool(std::shared_ptr<BTaggingEfficiencyTool> &eff_ptr,
                               const std::string &alg,
                               const std::string &author,
                               const std::string &wp);
  StatusCode initializeSelectTool(
      std::shared_ptr<BTaggingSelectionTool> &select_ptr,
      const std::string &alg, const std::string &author, const std::string &wp);
  std::string getFlavorLabel(const xAOD::Jet &jet) const;

  int getFlavor(const xAOD::Jet &jet) const;
  void setTaggerType();
  void selectTools();

  enum class CDI_FileType {
    UNKNOWN,
    OFFICIAL_MORIOND_2018,
    OFFICIAL_MAY_2018,
    OFFICIAL_MAY_2018_CustomVHbbMaps,
    OFFICIAL_JUN_2018,
    OFFICIAL_OCT_2018,
    OFFICIAL_JUL_2019,
    VHcc_Custom_CDI,
  };
  enum class TaggerType { UNKNOWN, MV2c10, DL1, DL1r };

  //If you want to EXCLUDE some B-tagging variations to speed up things, you can hard code them here and enable "useReducedSetOfBtagEV" in the config file.
  //By default the code will EXCLUDE these variation and for the others it will store just the UP-variations (they are symmetric in the CDI).
  std::string m_ExcludeEV = ""; //There is a variable in the config file

  CDI_FileType m_reference_file_type = CDI_FileType::UNKNOWN;
  TaggerType m_taggerEnum = TaggerType::UNKNOWN;
  TaggerType m_Veto_taggerEnum = TaggerType::UNKNOWN;
  // external tools
  // TAGGER, JET_AUTHOR, WORKING_POINT
  std::unordered_map<
      std::string,
      std::unordered_map<
          std::string,
          std::unordered_map<std::string,
                             std::shared_ptr<BTaggingEfficiencyTool>>>>
      m_efficiencyTools;
  std::unordered_map<
      std::string,
      std::unordered_map<
          std::string,
          std::unordered_map<std::string,
                             std::shared_ptr<BTaggingSelectionTool>>>>
      m_selectionTools;
  std::unordered_map<
      std::string,
      std::unordered_map<
          std::string,
          std::unordered_map<std::string,
                             std::shared_ptr<BTaggingTruthTaggingTool>>>>
      m_TruthTaggingTools;

  std::weak_ptr<BTaggingEfficiencyTool> m_currentEfficiencyTool;
  std::weak_ptr<BTaggingSelectionTool> m_currentSelectionTool;

  std::weak_ptr<BTaggingTruthTaggingTool> m_currentTruthTaggingTool;

  mutable bool m_state_change = false;
  bool m_doVar = false, m_useContinuous = false, m_printOutOfRangeWarnings,
    m_uncorrelate_run1_to_run2 = false, m_useReducedSetOfBTagEV = false;
  std::string m_tagger, m_jetAuthor, m_workingPoint, m_scheme, m_rScheme,
      m_fileNameCDI, m_btagcuteff, m_taggerName_Veto, m_OP_Veto;
  unsigned m_MCIndex = 9999;

  // used in VHcc truth tagging only
  unsigned m_MCIndex_TruthTag = 9999;

  const unsigned m_defaultMCIndex = 99;
  std::map<std::string, std::string> m_hbbname, m_flavorDSID;
  std::vector<float> m_binEdges;  // the bin edges of the fixed cut WP, for
                                  // calculating the quantile of the jet
  std::map<unsigned, unsigned> m_actualMC2MCIndex;
  std::set<unsigned> m_validMCIndicies;

  CP::SystematicSet m_affectingSystematics;

  // when truth tagging, if we want to require a particular set of jets to be
  // tagged, we can split into two groups and enforce different requirments on
  // each group. the final truth tagging event weight is the product of the two
  // weights

  struct TruthTagJets {
    std::vector<double> pt;
    std::vector<double> eta;
    std::vector<int> flav;
    std::vector<double> tagw;
    std::vector<int> signaljet_index;
    int n_requiredTags;
    Analysis::TruthTagResults tt_results;

    void clear() {
      pt.clear();
      eta.clear();
      flav.clear();
      tagw.clear();
      signaljet_index.clear();
      n_requiredTags = 0;
      tt_results.clear();
    }

    int size() { return pt.size(); }

    bool hasJetIndex(int indx) {
      for (unsigned int i = 0; i < signaljet_index.size(); i++) {
        if (indx == signaljet_index[i]) {
          return true;
        }
      }
      return false;
    }

    bool istagged(int ijet, bool exclusive) {
      for (unsigned int i = 0; i < signaljet_index.size(); i++) {
        if (ijet == signaljet_index[i]) {
          std::vector<bool> chosen_permuation =
              tt_results.getEventPermutation(n_requiredTags, exclusive);
          return chosen_permuation[i];
        }
      }
      return false;
    }
    float tagweight(int ijet, bool exclusive) {
      for (unsigned int i = 0; i < signaljet_index.size(); i++) {
        if (ijet == signaljet_index[i]) {
          std::vector<double> randomTaggerScores =
              tt_results.getRandomTaggerScores(n_requiredTags, exclusive);
          return randomTaggerScores[i];
        }
      }
      return -99;
    }
    float quantile(int ijet, bool exclusive) {
      for (unsigned int i = 0; i < signaljet_index.size(); i++) {
        if (ijet == signaljet_index[i]) {
          std::vector<int> quantiles =
              tt_results.getEventQuantiles(n_requiredTags, exclusive);
          return quantiles[i];
        }
      }
      return -99;
    }
    float getEventWeight(bool exclusive, std::string syst_name = "Nominal") {
      if (pt.size() == 0) {
        return 1.0;
      }
      return tt_results.getEventWeight(n_requiredTags, exclusive, syst_name);
    }
  };

  std::map<std::string, TruthTagJets> m_truthtag_jets;
  std::map<std::string, TruthTagJets> m_truthtag_jets_groupB;
  // a vector to store b-jets that are used in hybrid truth tagging.
  std::map<std::string, std::vector<const xAOD::Jet *> > m_hybrid_jets;


  bool m_UseQuantile;
  int m_maxTruthTag;

  void SelectJetsForTruthTagging(
      int n_RequiredTruthTags, TruthTagJets &tt_jets,
      TruthTagJets &tt_jets_groupB,
      const std::vector<const xAOD::Jet *> &signalJets, std::string strategy,
      bool doHybridTruthTagging, bool exclusive) const;
};

#endif  // ifndef CxAODTools_BTaggingTool_H
