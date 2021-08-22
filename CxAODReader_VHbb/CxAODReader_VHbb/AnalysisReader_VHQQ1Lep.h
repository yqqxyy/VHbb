#ifndef CxAODReader_AnalysisReader_VHQQ1Lep_H
#define CxAODReader_AnalysisReader_VHQQ1Lep_H

//#include "../../CorrsAndSysts/CorrsAndSysts/BDTSyst.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

#include "CxAODTools_VHbb/VBFHbbJetAssignment.h"

// class BDTSyst;

class AnalysisReader_VHQQ1Lep : public AnalysisReader_VHQQ {
 protected:
  ////Used for MODEL systematics on V+Jets and TTbar
  // CAS::EventType m_evtType; //!
  // CAS::DetailEventType m_detailevtType; //!
  // CAS::Systematic m_Systematic; //!

  // --------- Level of systematics in histogram output ----------
  enum class SystLevel { None = 0, CombinedOnly, All };
  //---------------------------------------------------------------

  //---------- Lepton Flavour info about event --------------
  // Enum for info encoding
  // Records the flavour of the lepton
  enum class lepFlav { Combined = 0, El, Mu };

  // Protected member that stored result
  lepFlav m_leptonFlavour;
  //---------------------------------------------------------

  // New Booking hisotgram for the 1-lepton analysis only. Hence protected.
  void BookFillHist_VHQQ1Lep(const string &name, int nbinsx, float xlow,
                             float xup, float value, float weight,
                             SystLevel = SystLevel::All,
                             bool PrintSplitFlav = true);
  void BookFillHist_SMCut(const string &name, int nbinsx, float xlow, float xup,
                          float value, float weight, SystLevel = SystLevel::All,
                          bool PrintSplitFlav = true);
  void selectRegime(double WpT, int tagcat,
                    const std::vector<const xAOD::Jet *> &fatJets,
                    bool passResolved, bool passMerged, bool passResolvedSR,
                    bool passMergedSR, bool passResolvedCR, bool passMergedCR);

  bool m_doIsoInv;
  bool m_doMbbRescaling;
  bool m_doSplitLepFlavour;
  std::string m_selection;
  std::string m_JetAssignmentStrategy;
  //std::string m_analysisType;           // !
  //std::string m_analysisStrategy;       // !
  std::string m_fJVTCut;
  jetAssignmentTool *m_jetAssigner; //!

  EL::StatusCode fillMVATreeVHbbResolved1Lep(
      TLorentzVector &b1, TLorentzVector &b2, TLorentzVector &j3,
      const std::vector<const xAOD::Jet *> &selectedJets,
      const std::vector<const xAOD::Jet *> &signalJets,
      const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &HVec,
      TLorentzVector &WVec, TLorentzVector &metVec, const xAOD::MissingET *&met,
      TLorentzVector &lep, int nTaus = 0, float bTagWeight = 1);

 public:
  AnalysisReader_VHQQ1Lep();
  ~AnalysisReader_VHQQ1Lep();

  virtual EL::StatusCode initializeSelection() override;
  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode run_1Lep_analysis() = 0;
  virtual EL::StatusCode fill_VBF () = 0;
  EL::StatusCode fill_1Lep();

  //  BDTSyst *m_BDTSyst = NULL;

  // EL::StatusCode SetupCorrsAndSyst(std::string currentVar, bool isCR);
  // void fillTTbarSystslep(int nTag, bool isCR, float mVH);
  // void fillVjetsSystslep(double Mbb, float mVH);
  EL::StatusCode applyCS_MJ(const TLorentzVector &lepVec);
  EL::StatusCode applyCS_SMVHMJEl(const xAOD::Electron *el, bool pass,
                                  bool isTop, bool isW, int nJet);
  EL::StatusCode applyCS_SMVHMJMu(const xAOD::Muon *mu, bool isTop, bool isW,
                                  int nJet);
  EL::StatusCode fill_1lepResolvedCutFlow(unsigned long int eventFlag);
  EL::StatusCode fill_1lepMergedCutFlow(unsigned long int eventFlag);
  EL::StatusCode fill1LepHistBothMergedResolved(SystLevel ApplySysts,
                                                const TLorentzVector &met_vec,
                                                const TLorentzVector &lep_vec,
                                                const TLorentzVector &W_vecT,
                                                const TLorentzVector &nu_vec,
                                                const xAOD::Electron *el,
                                                const xAOD::Muon *mu);

  EL::StatusCode fill1LepHistResolved(
      SystLevel ApplySysts, int tag, const TLorentzVector &W_vecT,
      const TLorentzVector &H_vec_corr, const TLorentzVector &j1,
      const TLorentzVector &j2, const TLorentzVector &j3, int nSelectedJets,
      const TLorentzVector &lep_vec, std::vector<const xAOD::Jet *> &signalJets,
      std::vector<const xAOD::Jet *> &forwardJets,
      std::vector<const xAOD::Jet *> &selectedJets, float Mtop, float dYWH,
      double metSig, double metSig_PU, double metSig_soft, double metSig_hard,
      double metOverSqrtSumET, double metOverSqrtHT,
      const TLorentzVector &met_vec);

  EL::StatusCode fill1LepHistMerged(
      VHCand &mergedVH, const TLorentzVector &fatJet,
      std::vector<const xAOD::Jet *> &trackjetsInLeadFJ,
      const TLorentzVector &lep_vec, const TLorentzVector &W_vecT,
      std::vector<const xAOD::Jet *> fatJets, float dYWFJ);
  EL::StatusCode applyElMuCS(string samplename, int nJet,
                             ResultVHbb1lep selectionResult,
                             const TLorentzVector &lepVec);
  bool pass1LepTrigger(double &triggerSF_nominal,
                       ResultVHbb1lep selectionResult);
  bool passLeptonPt27(ResultVHbb1lep selectionResult,
                      const TLorentzVector &leptonVec,
                      const TLorentzVector &WVecT);
  bool passIsoInverted(ResultVHbb1lep selectionResult,
                       const TLorentzVector &leptonVec);
  bool passNomIsolation(ResultVHbb1lep selectionResult,
                        const TLorentzVector &leptonVec);
  bool passLeptonQuality(ResultVHbb1lep selectionResult);
  bool passmTW(const TLorentzVector &WVecT);
  bool passMET(const TLorentzVector &WVecT, const TLorentzVector &metVec,
               int nSignalEl, int nSignalMu, double m_minElMET,
               double m_minMuMET);
  float pTWFiltering(const TLorentzVector &lepton);
  float calculateMtop(const TLorentzVector &lepton, const TLorentzVector &MET,
                      const TLorentzVector &b_jet1,
                      const TLorentzVector &b_jet2);
  float calculatedYWH(const TLorentzVector &lepton, const TLorentzVector &MET,
                      const TLorentzVector &b_jet1,
                      const TLorentzVector &b_jet2);
  float calculatedYWFJ(const TLorentzVector &lepton, const TLorentzVector &MET,
                       const TLorentzVector &fatjet1);
  float get_sys_metstrmu(float mTW);
  EL::StatusCode applyFFweight(const TLorentzVector &WVecT,
                               const TLorentzVector &metVec,
                               const xAOD::Electron *el);
  EL::StatusCode applyFFweight(const TLorentzVector &WVecT,
                               const TLorentzVector &metVec,
                               const xAOD::Muon *mu);

  std::vector<unsigned long int> cuts_SR_resolved;
  std::vector<unsigned long int> cuts_CR_resolved;
  std::vector<unsigned long int> cuts_model_resolved;
  std::vector<unsigned long int> cuts_SR_merged;
  std::vector<unsigned long int> cuts_CR_merged;
  std::vector<unsigned long int> cuts_resolved_CUT;
  std::vector<unsigned long int> cuts_resolved;
  std::vector<unsigned long int> cuts_merged;
  std::vector<unsigned long int> cuts_easytree_merged;
  std::vector<unsigned long int> cuts_easytree_resolved;
  std::vector<unsigned long int> cuts_cutflow_resolved;

  EL::StatusCode initializeCuts();

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VHQQ1Lep, 1);
};

namespace OneLeptonResolvedCuts {

enum Cuts {
  AllCxAOD = 0,     // C0
  Trigger,          // C1
  LooseLeptons,     // C2
  SignalLeptons,    // C3
  AtLeast2Jets,     // C4
  AtLeast2SigJets,  // C5
  Pt45,             // C6
  MET,              // C7
  mTW,              // C8
  Veto3bjet,        // C9
  mbbRestrict,
  mbbCorr,
  pTV,
  dRBB,
  mTW_add,
  SR_0tag_2jet,   // C10
  SR_0tag_3jet,   // C11
  SR_0tag_4pjet,  // C12
  SR_1tag_2jet,   // C13
  SR_1tag_3jet,   // C14
  SR_1tag_4pjet,  // C15
  SR_2tag_2jet,   // C16
  SR_2tag_3jet,   // C17
  SR_2tag_4pjet,  // C18
};
}

namespace OneLeptonCuts {

enum Cuts {
  AllCxAOD = 0,         // 0
  Trigger,              // 1
  LooseLepton,          // 2
  SignalLepton,         // 3
  AtLeast2Jets,         // 4
  AtLeast2SigJets,      // 5
  Pt45,                 // 6
  METResolved,          // 7
  mTWResolved,          // 8
  Veto3bjet,            // 9
  mbbRestrictResolved,  // 10
  mbbRestrictMerged,    // 11
  mbbCorrResolved,      // 12
  pTVResolved,          // 13
  dRBB,                 // 14
  mTW_add,              // 15
  SR_0tag_2pjet,        // 16
  SR_1tag_2pjet,        // 17
  SR_2tag_2pjet,        // 18
  AtLeast1FatJet,       // 19
  AtLeast2TrackJets,    // 20
  METMerged,            // 21
  mTWMerged,            // 22
  pTVMerged,            // 23
  mbbCorrMerged,        // 24
  SR_0tag_1pfat0pjets,  // 25
  SR_1tag_1pfat0pjets,  // 26
  SR_2tag_1pfat0pjets,  // 27
  SR_3ptag_1pfat0pjets  // 28

};
}

namespace OneLeptonMergedCuts {
enum Cuts {
  AllCxAOD = 0,          // M0
  Trigger,               // M1
  Leptons,               // M2
  MET,                   // M3
  AtLeast1FatJet,        // M4
  mbbCorr,               // M5
  AtLeast2TrackJets,     // M6
  passVRJetOR,           // M7
  mTW,                   // M8
  pTV,                   // M9
  mJ,                    // M10
  dYWFJ,                 // M11
  mbbRestrict,           // M12
  SR_0tag_1pfat0pjets,   // M13
  SR_1tag_1pfat0pjets,   // M14
  SR_2tag_1pfat0pjets,   // M15
  SR_3ptag_1pfat0pjets,  // M16
};
}  // namespace OneLeptonMergedCuts

// Replace Line // <<<--- SJ01 Revert Point
//    LooseLeptons,               // C2
//  SignalLeptons,              // c3
//    AtLeast2Jets,               // C4
//    AtLeast2SigJets,            // C5
//    Pt45,                       // C6
//    MET,                        // C7
//    mTW,                        // C8
//    BjetVeto3rd,                // C9
//    SR_0tag_2jet,              // C10
//    SR_0tag_3jet,               // C11
//    SR_0tag_4pjet,              // C12
//    SR_1tag_2jet,              // C13
//    SR_1tag_3jet,               // C14
//    SR_1tag_4pjet,              // C15
//    SR_2tag_2jet,              // C16
//    SR_2tag_3jet,               // C17
//    SR_2tag_4pjet,              // C18
//    SR_3ptag_23jet,             // C19
//    SR_3ptag_4jet,              // C20
//    SR_3ptag_5pjet,             // C21
//};
// }
//             // <<<--- SJ01 Revert Point

#endif  // ifndef CxAODReader_AnalysisReader_VHQQ1Lep_H
