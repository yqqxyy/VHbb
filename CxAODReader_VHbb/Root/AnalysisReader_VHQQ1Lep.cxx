#include <cfloat>
#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"

#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb1lep.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

#include "CxAODTools_VHbb/VBFHbbInclEvtSelection.h"
#include "CxAODTools_VHbb/VBFHbb1centralEvtSelection.h"
#include "CxAODTools_VHbb/VBFHbb2centralEvtSelection.h"
#include "CxAODTools_VHbb/VBFHbb4centralEvtSelection.h"

#include <CxAODReader_VHbb/AnalysisReader_VHQQ1Lep.h>
#include "../../CorrsAndSysts/Root/BDTSyst.cxx"

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHQQ1Lep::AnalysisReader_VHQQ1Lep()
    : AnalysisReader_VHQQ(),
      m_doIsoInv(false),
      m_doMbbRescaling(true),
      m_selection("none"),
      m_jetAssigner(nullptr),
      m_doSplitLepFlavour(false) {}

AnalysisReader_VHQQ1Lep::~AnalysisReader_VHQQ1Lep() {}

EL::StatusCode AnalysisReader_VHQQ1Lep::initializeSelection() {
  EL_CHECK("AnalysisReader_VHQQ1Lep::initializeSelection()",
           AnalysisReader_VHQQ::initializeSelection());

  initializeCuts();

  // BDTSyst::test_function();
  m_BDTSyst = new BDTSyst();
  m_BDTSyst->Initialise(1);


  //added by wym
  //m_analysisType = "vbf";
  //m_analysisStrategy = "Resolved";
  m_config->getif<string>("JetAssignmentStrategy", m_JetAssignmentStrategy); // BTAG_XX
  //m_config->getif<string>("fJVTCut", m_fJVTCut);
  m_config->getif<string>("selection", m_selection); // 1 or 2 or 4 central


  // Check inputs
  std::string btag_selection = "0";
  if (m_JetAssignmentStrategy.find("BTAG_60")      != std::string::npos) btag_selection = "60";
  else if (m_JetAssignmentStrategy.find("BTAG_70") != std::string::npos) btag_selection = "70";
  else if (m_JetAssignmentStrategy.find("BTAG_77") != std::string::npos) btag_selection = "77";
  else if (m_JetAssignmentStrategy.find("BTAG_85") != std::string::npos) btag_selection = "85";
  else if (m_JetAssignmentStrategy.find("asym")    != std::string::npos) btag_selection = "asym";
  else if (m_JetAssignmentStrategy.find("nobtag")  != std::string::npos) btag_selection = "0"; // remains unchanged
  else {
    Error("initializeSelection()", "Invalid b-tag selection for jet assignment %s", btag_selection.c_str());
    return EL::StatusCode::FAILURE;
  }

  if (m_selection == "none") 
    m_eventSelection = new VBFHbbInclEvtSelection();
  else if (m_selection.find("4central")!=std::string::npos) 
    m_eventSelection = new VBFHbb4centralEvtSelection( m_selection.find("nobtag")==std::string::npos ,btag_selection, m_isMC);
  else if (m_selection.find("2central")!=std::string::npos) 
    m_eventSelection = new VBFHbb2centralEvtSelection( m_selection.find("nobtag")==std::string::npos ,btag_selection, m_isMC);
  else if (m_selection.find("1central")!=std::string::npos)
    m_eventSelection = new VBFHbb1centralEvtSelection( m_selection.find("nobtag")==std::string::npos ,btag_selection, m_isMC);
  else {
    Error("initializeSelection()", "Invalid selection %s", m_selection.c_str());
    return EL::StatusCode::FAILURE;
  }
  Info("initializeSelection()", "Selection is '%s'.", m_selection.c_str());

  //wym

  m_fillFunction   = std::bind( &AnalysisReader_VHQQ1Lep::fill_VBF, this );
  //m_eventSelection = new VHbb1lepEvtSelection(m_config);
  // m_fillFunction   = std::bind(&AnalysisReader_VHQQ1Lep::fill_1Lep, this);

  //m_fillFunction = std::bind(&AnalysisReader_VHQQ1Lep::run_1Lep_analysis, this);


  if (m_model == Model::HVT)
    ((VHbb1lepEvtSelection *)m_eventSelection)->SetModel("HVT");
  ((VHbb1lepEvtSelection *)m_eventSelection)->SetAnalysisType(m_analysisType);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ1Lep::initializeTools() {
  EL_CHECK("AnalysisReader_VHQQ1Lep::initializeTools()",
           AnalysisReader_VHQQ::initializeTools());



  // Jet Assignment Tool
  // -------------
  m_jetAssigner = new jetAssignmentTool();
  EL_CHECK( "AnalysisReader::jetAssignmentTool()",m_jetAssigner->setSelection(m_selection));
  //  EL_CHECK( "AnalysisReader::jetAssignmentTool()",m_jetAssigner->setDataPeriod(m_Period)); // TO BE REMOVED
  EL_CHECK( "AnalysisReader::jetAssignmentTool()",m_jetAssigner->setIsMC(m_isMC));

  if (m_selection.find("nobtag") != std::string::npos) {
    EL_CHECK( "AnalysisReader::jetAssignmentTool()",m_jetAssigner->setAssignmentType("0tag") );
  }
  else { 
    EL_CHECK( "AnalysisReader::jetAssignmentTool()",m_jetAssigner->setAssignmentType( m_JetAssignmentStrategy ) );
  }


  m_triggerTool = new TriggerTool_VHbb1lep(*m_config);
  EL_CHECK("AnalysisReader_VHQQ1Lep::initializeTools()",
           m_triggerTool->initialize());

  // default full run 2 MVA (ICHEP style training settings, full truth tagging,
  // ptV>150 GeV inclusive training)
  /*m_mvaVHbbFullRun2oldDefault = new MVAApplication_TMVA(
      m_analysisType, MVAType::ichepStyleIncl1LepMedium, "mvaOldDefault");
  m_mvaVHbbFullRun2oldDefault->Initialise(
      "$WorkDir_DIR/data/CxAODReader_VHbb/BDT_fullTruthTag_ICHEPstyle/",
      m_tree);
  m_mvaVHbbApps["mvaOldDefault"] = m_mvaVHbbFullRun2oldDefault;

  m_mvaVHbbFullRun2oldDefaultVZ =
      new MVAApplication_TMVA(m_analysisType, MVAType::ichepStyleIncl1LepMedium,
                              "mvadibosonOldDefault");
  m_mvaVHbbFullRun2oldDefaultVZ->Initialise(
      "$WorkDir_DIR/data/CxAODReader_VHbb/BDT_fullTruthTag_ICHEPstyle_diboson/"
      "1L/",
      m_tree);
  m_mvaVHbbApps["mvadibosonOldDefault"] = m_mvaVHbbFullRun2oldDefaultVZ;

  if (m_useContinuous) {
    m_mvaVHbbFullRun2 = new MVAApplication_TMVA(
        m_analysisType, MVAType::ichepStyleIncl1LepMedium, "mva");
    m_mvaVHbbFullRun2->Initialise(
        "$WorkDir_DIR/data/CxAODReader_VHbb/"
        "BDT_fullTruthTag_PCBToptimised_SRCR/",
        m_tree);
    m_mvaVHbbApps["mva"] = m_mvaVHbbFullRun2;

    m_mvaVHbbFullRun2VZ = new MVAApplication_TMVA(
        m_analysisType, MVAType::ichepStyleIncl1LepMedium, "mvadiboson");
    m_mvaVHbbFullRun2VZ->Initialise(
        "$WorkDir_DIR/data/CxAODReader_VHbb/"
        "BDT_fullTruthTag_PCBToptimised_SRCR_diboson/",
        m_tree);
    m_mvaVHbbApps["mvadiboson"] = m_mvaVHbbFullRun2VZ;

    m_mvaVHbbFullRun2PCBT = new MVAApplication_TMVA(
        m_analysisType, MVAType::ichepStyleIncl1LepMedium, "mvaPCBT");
    m_mvaVHbbFullRun2PCBT->Initialise(
        "$WorkDir_DIR/data/CxAODReader_VHbb/BDT_fullTruthTag_PCBT_SRCR/",
        m_tree);
    // m_mvaVHbbApps["mvaPCBT"] = m_mvaVHbbFullRun2PCBT; // include this line if
    // you want mva with MV2c10 in input variables and old hyperparameter
    // settings
  }*/

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ1Lep::fill_1lepResolvedCutFlow(
    unsigned long int eventFlag) {
  // systematics variations are skipped via histNameSvc
  std::string dir = "CutFlow/Nominal/";

  static std::string cuts[24] = {
      "AllCxAOD",      "Trigger",      "LooseLeptons",
      "SignalLeptons", "AtLeast2Jets", "AtLeast2SigJets",
      "Pt45",          "MET",          "mTW",
      "Veto3bjet",     "mbbRestrict",  "mbbCorr",
      "pTV",           "dRBB",         "mTW_add",
      "SR_0tag_2jet",  "SR_0tag_3jet", "SR_0tag_4pjet",
      "SR_1tag_2jet",  "SR_1tag_3jet", "SR_1tag_4pjet",
      "SR_2tag_2jet",  "SR_2tag_3jet", "SR_2tag_4pjet"};

  // Cuts to exclude from cutflow
  std::vector<unsigned long int> excludeCuts = {
      OneLeptonResolvedCuts::dRBB,
      OneLeptonResolvedCuts::mTW_add};  // OneLeptonResolvedCuts::tauVeto};

  // Loop over cuts
  for (unsigned long int i = OneLeptonResolvedCuts::AllCxAOD;
       i <= OneLeptonResolvedCuts::SR_2tag_4pjet; ++i) {
    // Skip excluded cuts
    if (std::find(excludeCuts.begin(), excludeCuts.end(), i) !=
        excludeCuts.end())
      continue;

    // all cuts up to Pt45 should be applied sequentially
    if (i <= OneLeptonResolvedCuts::pTV) {
      if (!passAllCutsUpTo(eventFlag, i, {})) continue;
    } else {
      if (!(passAllCutsUpTo(eventFlag, OneLeptonResolvedCuts::pTV, {}) &&
            passSpecificCuts(eventFlag, {i})))
        continue;
    }
    std::string label = cuts[i];
    m_histSvc->BookFillCutHist(dir + "CutsResolved", length(cuts), cuts, label,
                               m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsResolvedNoWeight", length(cuts), cuts,
                               label, 1);
    // if(m_leptonFlavour == lepFlav::El){
    //  m_histSvc->BookFillCutHist(dir + "CutsResolved_el", length(cuts), cuts,
    //  label, m_weight); m_histSvc->BookFillCutHist(dir +
    //  "CutsResolvedNoWeight_el", length(cuts), cuts, label, 1);
    //}
    // if(m_leptonFlavour == lepFlav::Mu){
    //  m_histSvc->BookFillCutHist(dir + "CutsResolved_mu", length(cuts), cuts,
    //  label, m_weight); m_histSvc->BookFillCutHist(dir +
    //  "CutsResolvedNoWeight_mu", length(cuts), cuts, label, 1);
    //}
  }

  return EL::StatusCode::SUCCESS;
}  // fill_1lepResolvedCutFlow

EL::StatusCode AnalysisReader_VHQQ1Lep::fill_1lepMergedCutFlow(
    unsigned long int eventFlag) {
  // systematics variations are skipped via histNameSvc
  std::string dir = "CutFlow/Nominal/";

  static std::string cuts[17] = {
      "AllCxAOD",
      "Trigger",
      "Leptons",
      "MET",
      "AtLeast1FatJet",
      "mbbCorr",
      "AtLeast2TrackJets",
      "passVRJetOR",
      "mTW",
      "pTV",
      "mJ",
      "dYWFJ",
      "mbbRestrict",
      "SR_0tag_1pfat0pjets",
      "SR_1tag_1pfat0pjets",
      "SR_2tag_1pfat0pjets",
      "SR_3ptag_1pfat0pjets",
  };

  // Cuts to exclude from cutflow
  std::vector<unsigned long int> excludeCuts = {};

  // Loop over cuts
  for (unsigned long int i = OneLeptonMergedCuts::AllCxAOD;
       i <= OneLeptonMergedCuts::SR_3ptag_1pfat0pjets; ++i) {
    // Skip excluded cuts
    if (std::find(excludeCuts.begin(), excludeCuts.end(), i) !=
        excludeCuts.end())
      continue;

    if (i <= OneLeptonMergedCuts::pTV) {
      if (!passAllCutsUpTo(eventFlag, i, {})) continue;
    } else {
      if (!(passAllCutsUpTo(eventFlag, OneLeptonMergedCuts::pTV, {}) &&
            passSpecificCuts(eventFlag, {i})))
        continue;
    }
    std::string label = cuts[i];
    m_histSvc->BookFillCutHist(dir + "CutsMerged", length(cuts), cuts, label,
                               m_weight);
  }

  return EL::StatusCode::SUCCESS;
}  // fill_1lepMergedCutFlow

// Common histogram filling functions
// Using three levels of histogram output, harmonized with 0L and 2L
// - doOnlyInputs: Do only the input histograms for the fit (mBB, pTV for CUT
// and  mva scores, pTV for MVA)
// - GenerateSTXS: Generate the histograms used for the STXS splitting
// - doReduceFillHistos: Reduce the number of filled histograms to a more
// restricted collection (mostly disabling the filling of histograms for full
// 4-vector information)

EL::StatusCode AnalysisReader_VHQQ1Lep::fill1LepHistBothMergedResolved(
    SystLevel ApplySysts, const TLorentzVector &met_vec,
    const TLorentzVector &lep_vec, const TLorentzVector &W_vecT,
    const TLorentzVector &nu_vec, const xAOD::Electron *el,
    const xAOD::Muon *mu) {
  if (!m_doOnlyInputs && !m_GenerateSTXS) {
    SystLevel ApplyMETSysts = m_doIsoInv ? SystLevel::All : SystLevel::None;

    BookFillHist_VHQQ1Lep("MET", 100, 0, 1000, met_vec.Pt() / 1e3, m_weight,
                          ApplyMETSysts);
    BookFillHist_VHQQ1Lep("mTW", 100, 0, 500, W_vecT.M() / 1e3, m_weight);

    BookFillHist_VHQQ1Lep("pTL", 500, 0, 500, lep_vec.Pt() * 0.001, m_weight);
    BookFillHist_VHQQ1Lep("pTLeta", 130, -6.5, 6.5, lep_vec.Eta(),
                          m_weight);  // Need to be named like this?

    if (!m_doReduceFillHistos) {
      // Plot the average number of  interactions per crossing for PU.
      BookFillHist_VHQQ1Lep("AverageMu", 100, 0, 100, m_averageMu, m_weight);
      BookFillHist_VHQQ1Lep("ActualMu", 100, 0, 100, m_actualMu, m_weight);
      BookFillHist_VHQQ1Lep("AverageMuScaled", 100, 0, 100, m_averageMuScaled,
                            m_weight);
      BookFillHist_VHQQ1Lep("ActualMuScaled", 100, 0, 100, m_actualMuScaled,
                            m_weight);

      // TLV of leptonic system
      fill_TLV("lepVec", lep_vec, m_weight, m_leptonFlavour == lepFlav::El,
               m_leptonFlavour == lepFlav::Mu);
      fill_TLV("nuVec", nu_vec, m_weight, m_leptonFlavour == lepFlav::El,
               m_leptonFlavour == lepFlav::Mu);
      fill_TLV("WVec", W_vecT, m_weight, m_leptonFlavour == lepFlav::El,
               m_leptonFlavour == lepFlav::Mu);

      // Lepton and MET
      BookFillHist_VHQQ1Lep("DeltaPhiLepMet", 160, -4, 4,
                            lep_vec.DeltaPhi(met_vec), m_weight, ApplySysts,
                            false);
    }  //! doReduceFillHistos

    if (!m_doReduceFillHistos ||
        m_doIsoInv) {  // In normal run, these are extended, if doIsoInv for MJ
                       // estimate, these are part of the standard plots
      // Plot the ptvarcone and topetcone values for the isolation of the
      // leptons
      if (m_leptonFlavour == lepFlav::El) {
        BookFillHist_VHQQ1Lep("ptvarcone20_pT", 100, 0, 0.5,
                              Props::ptvarcone20.get(el) / lep_vec.Pt(),
                              m_weight, SystLevel::None);
        BookFillHist_VHQQ1Lep("ptvarcone20", 100, 0, 5000,
                              Props::ptvarcone20.get(el), m_weight,
                              SystLevel::None);
        BookFillHist_VHQQ1Lep("topoetcone20_pT", 100, 0, 0.5,
                              Props::topoetcone20.get(el) / lep_vec.Pt(),
                              m_weight, SystLevel::None);
        BookFillHist_VHQQ1Lep("topoetcone20", 100, 0, 5000,
                              Props::topoetcone20.get(el), m_weight,
                              SystLevel::None);
      }
      if (m_leptonFlavour == lepFlav::Mu) {
        BookFillHist_VHQQ1Lep("ptvarcone30_pT", 100, 0, 0.5,
                              Props::ptvarcone30.get(mu) / lep_vec.Pt(),
                              m_weight, SystLevel::None);
        BookFillHist_VHQQ1Lep("ptvarcone30", 100, 0, 5000,
                              Props::ptvarcone30.get(mu), m_weight,
                              SystLevel::None);
        BookFillHist_VHQQ1Lep("topoetcone20_pT", 100, 0, 0.5,
                              Props::topoetcone20.get(mu) / lep_vec.Pt(),
                              m_weight, SystLevel::None);
        BookFillHist_VHQQ1Lep("topoetcone20", 100, 0, 5000,
                              Props::topoetcone20.get(mu), m_weight,
                              SystLevel::None);
      }

    }  //! m_doReduceFillHistos || m_doIsoInv
  }    // m_doOnlyInputs && !generateSTXS

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ1Lep::fill1LepHistResolved(
    SystLevel ApplySysts, int tag, const TLorentzVector &W_vecT,
    const TLorentzVector &H_vec_corr, const TLorentzVector &j1,
    const TLorentzVector &j2, const TLorentzVector &j3, int nSelectedJets,
    const TLorentzVector &lep_vec, std::vector<const xAOD::Jet *> &signalJets,
    std::vector<const xAOD::Jet *> &forwardJets,
    std::vector<const xAOD::Jet *> &selectedJets, float Mtop, float dYWH,
    double metSig, double metSig_PU, double metSig_soft, double metSig_hard,
    double metOverSqrtSumET, double metOverSqrtHT,
    const TLorentzVector &met_vec) {
  //=========== Fill Resolved Regime Input Plots ============

  // first use common functions
  EL_CHECK("AnalysisReader_VHQQ1Lep::fill1LepHistResolved",
           fill_jetHistos(signalJets, forwardJets));
  // this also fills mBB in case of CUT && doOnlyInputs
  EL_CHECK("AnalysisReader_VHQQ1Lep::fill1LepHistResolved",
           fill_jetSelectedHistos());

  if (m_doOnlyInputs) {
    BookFillHist_VHQQ1Lep("pTV", 200, 0, 2000, W_vecT.Pt() / 1e3, m_weight,
                          ApplySysts);

    BookFillHist_VHQQ1Lep("mBB", 100, 0, 500, H_vec_corr.M() / 1e3, m_weight,
                          ApplySysts);
  }

  // now the additional variables
  if (!m_doOnlyInputs && !m_GenerateSTXS) {
    // resolved specific lepton variables

    // MET-related variables
    if (!m_doReduceFillHistos || m_doSplitLepFlavour) {
      BookFillHist_VHQQ1Lep("METSig", 300, 0., 30., metSig, m_weight,
                            ApplySysts);
      if (metSig_PU != -1) {
        BookFillHist_VHQQ1Lep("METSig_PU", 300, 0., 30., metSig_PU, m_weight,
                              ApplySysts);
      } else {
        BookFillHist_VHQQ1Lep("METSig_soft", 300, 0., 30., metSig_soft,
                              m_weight, ApplySysts);
        BookFillHist_VHQQ1Lep("METSig_hard", 300, 0., 30., metSig_hard,
                              m_weight, ApplySysts);
      }
      BookFillHist_VHQQ1Lep("METOverSqrtSumET", 300, 0., 30., metOverSqrtSumET,
                            m_weight, ApplySysts);
      BookFillHist_VHQQ1Lep("METOverSqrtHT", 300, 0., 30., metOverSqrtHT,
                            m_weight, ApplySysts);
    }

    double Ht = 0;
    for (auto SigJet : signalJets) {
      Ht += SigJet->p4().Pt();
    }
    for (auto ForwardJet : forwardJets) {
      Ht += ForwardJet->p4().Pt();
    }
    BookFillHist_VHQQ1Lep("Ht", 500, 0, 5000, Ht / 1e3, m_weight);

    BookFillHist_VHQQ1Lep("dPhiVBB", 315, 0., 3.15,
                          fabs(W_vecT.DeltaPhi(H_vec_corr)), m_weight);

    BookFillHist_VHQQ1Lep(
        "dPhiLBmin", 100, 0., 6.,
        std::min(fabs(lep_vec.DeltaPhi(j1)), fabs(lep_vec.DeltaPhi(j2))),
        m_weight);

    if (m_model == Model::MVA || m_model == Model::CUT) {
      BookFillHist_VHQQ1Lep("Mtop", 500, 0, 500, Mtop / 1e3, m_weight);
      BookFillHist_VHQQ1Lep("dYWH", 100, 0, 6, dYWH, m_weight);
    }

    // only for extended plots
    if (!m_doReduceFillHistos) {
      // full 4-vectors of Higgs and jets
      fill_TLV("HVecJetCorr", H_vec_corr, m_weight,
               m_leptonFlavour == lepFlav::El, m_leptonFlavour == lepFlav::Mu);
      fill_TLV("j1VecCorr", j1, m_weight, m_leptonFlavour == lepFlav::El,
               m_leptonFlavour == lepFlav::Mu);
      fill_TLV("j2VecCorr", j2, m_weight, m_leptonFlavour == lepFlav::El,
               m_leptonFlavour == lepFlav::Mu);
      BookFillHist_VHQQ1Lep("binMV2c10B1", 6, 0, 6,
                            BTagProps::Quantile.get(selectedJets.at(0)),
                            m_weight, ApplySysts);
      BookFillHist_VHQQ1Lep("binMV2c10B2", 6, 0, 6,
                            BTagProps::Quantile.get(selectedJets.at(1)),
                            m_weight, ApplySysts);

      if (nSelectedJets > 2) {
        fill_TLV("j3VecCorr", j3, m_weight, m_leptonFlavour == lepFlav::El,
                 m_leptonFlavour == lepFlav::Mu);
      }
      // 2D Histos
      m_histSvc->BookFillHist("dPhiBB_dEtaBB", 100, 0., 3.15, 100, 0., 6.,
                              fabs(j1.DeltaPhi(j2)), fabs(j1.Eta() - j2.Eta()),
                              m_weight);
      m_histSvc->BookFillHist("dPhiBB_dRBB", 100, 0., 3.15, 100, 0., 6.,
                              fabs(j1.DeltaPhi(j2)), j1.DeltaR(j2), m_weight);
      m_histSvc->BookFillHist("dEtaBB_dRBB", 100, 0., 6., 100, 0., 6.,
                              fabs(j1.Eta() - j2.Eta()), j1.DeltaR(j2),
                              m_weight);

    }  // !doReduceFillHistos
  }    //! doOnlyInputs && !GenerateSTXS

  // VHbb resolved MVA specific
  if (m_model == Model::MVA) {
    // MVA scores and STXS filling
    for (std::map<std::string, MVAApplication_TMVA *>::iterator mva_iter =
             m_mvaVHbbApps.begin();
         mva_iter != m_mvaVHbbApps.end(); mva_iter++) {
      std::string mvaName = mva_iter->first;
      float mvaScore = mva_iter->second->Evaluate(
          m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
      m_histSvc->BookFillHist(mvaName, 500, -1, 1, mvaScore,
                              m_weight);  // new mva discriminant
    }

    if (m_GenerateSTXS && tag == 2 && m_physicsMeta.nJets < 4) {
      // start inputs filling for each STXS signal truth bin
      if (STXS_GetBinFromEvtInfo() != -1) {
        STXS_FillYields();
        std::string temp_name = m_histNameSvc->get_sample();
        STXS_ReplaceName();  // replace the signal name with the STXS
        // bin
        BookFillHist_VHQQ1Lep("pTV", 200, 0, 2000, W_vecT.Pt() / 1e3, m_weight);
        BookFillHist_VHQQ1Lep("mBB", 100, 0, 500, H_vec_corr.M() / 1e3,
                              m_weight);

        for (std::map<std::string, MVAApplication_TMVA *>::iterator mva_iter =
                 m_mvaVHbbApps.begin();
             mva_iter != m_mvaVHbbApps.end(); mva_iter++) {
          std::string mvaName = mva_iter->first;
          float mvaScore = mva_iter->second->Evaluate(
              m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
          m_histSvc->BookFillHist(mvaName, 500, -1, 1, mvaScore,
                                  m_weight);  // new mva discriminant
        }
        m_histNameSvc->set_sample(temp_name);  // set the histogram name back
      }
    }  // STXS

    // Multijet estimate - special naming convention for variables
    if (m_doIsoInv && !m_GenerateSTXS) {
      m_histSvc->BookFillHist("mva_pTV", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT, W_vecT.Pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("mva_mBB", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT, H_vec_corr.M() / 1e3, m_weight);
      m_histSvc->BookFillHist("mva_MET", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT, met_vec.Pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("mva_mTW", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT, W_vecT.M() / 1e3, m_weight);
      m_histSvc->BookFillHist("mvadiboson_pTV", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT_VZ, W_vecT.Pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("mvadiboson_mBB", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT_VZ, H_vec_corr.M() / 1e3, m_weight);
      m_histSvc->BookFillHist("mvadiboson_MET", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT_VZ, met_vec.Pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("mvadiboson_mTW", 1000, -1., 1, 1000, 0., 5000.,
                              m_tree->BDT_VZ, W_vecT.M() / 1e3, m_weight);
    }
  }  // m_model MVA

  return EL::StatusCode::SUCCESS;
}  // fill1LepHistResolved

EL::StatusCode AnalysisReader_VHQQ1Lep::fill1LepHistMerged(
    VHCand &mergedVH, const TLorentzVector &fatJet,
    std::vector<const xAOD::Jet *> &trackjetsInLeadFJ,
    const TLorentzVector &lep_vec, const TLorentzVector &W_vecT,
    std::vector<const xAOD::Jet *> fatJets, float dYWFJ) {
  //=========== Fill Merged Regime Input Plots ============

  if (m_GenerateSTXS) {
    // start inputs filling for each STXS signal truth bin
    if (STXS_GetBinFromEvtInfo() != -1) {
      STXS_FillYields_Boosted();
      std::string temp_name = m_histNameSvc->get_sample();
      STXS_ReplaceName();  // replace the signal name with the STXS
      BookFillHist_VHQQ1Lep("mJ", 100, 0, 500, fatJet.M() / 1e3, m_weight,
                            SystLevel::CombinedOnly);
      m_histNameSvc->set_sample(temp_name);  // set the histogram name back
    }
  }  // end STXS

  // mJ with C2 cuts applied
  float fj1c2 = Props::C2.get(fatJets.at(0));
  for (auto cut_info : m_c2Cuts) {
    if (fj1c2 < cut_info.second)
      BookFillHist_VHQQ1Lep(("mJ" + cut_info.first).c_str(), 100, 0, 500,
                            fatJet.M() / 1e3, m_weight,
                            SystLevel::CombinedOnly);
  }

  if (!m_GenerateSTXS) {
    BookFillHist_VHQQ1Lep("pTV", 200, 0, 2000, W_vecT.Pt() / 1e3, m_weight,
                          SystLevel::CombinedOnly);
    BookFillHist_VHQQ1Lep("mJ", 100, 0, 500, fatJet.M() / 1e3, m_weight,
                          SystLevel::CombinedOnly);
    BookFillHist_VHQQ1Lep("mTW", 100, 0, 500, W_vecT.M() / 1e3, m_weight,
                          SystLevel::CombinedOnly);
  }

  if (!m_doOnlyInputs && !m_GenerateSTXS) {
    BookFillHist_VHQQ1Lep("mVH", 600, 0, 6000, mergedVH.vec_corr.M() / 1e3,
                          m_weight, SystLevel::CombinedOnly);

    // BM: ToDo, check what this common function does
    EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
             fill_fatJetHistos(fatJets));

    BookFillHist_VHQQ1Lep("dPhiVFatCorr", 100, 0., 3.15,
                          fabs(W_vecT.DeltaPhi(fatJet)), m_weight);
    BookFillHist_VHQQ1Lep("dPhiLFatCorr", 100, 0., 3.15,
                          fabs(lep_vec.DeltaPhi(fatJet)), m_weight);

    BookFillHist_VHQQ1Lep("dYWFJ", 100, 0, 6, dYWFJ, m_weight);

    if (!m_doReduceFillHistos) {
      fill_TLV("HVecFatCorr", fatJet, m_weight, m_leptonFlavour == lepFlav::El,
               m_leptonFlavour == lepFlav::Mu);
      if (trackjetsInLeadFJ.size() >= 1)
        fill_TLV("TrkJet1", trackjetsInLeadFJ.at(0)->p4(), m_weight,
                 m_leptonFlavour == lepFlav::El,
                 m_leptonFlavour == lepFlav::Mu);
      if (trackjetsInLeadFJ.size() >= 2)
        fill_TLV("TrkJet2", trackjetsInLeadFJ.at(1)->p4(), m_weight,
                 m_leptonFlavour == lepFlav::El,
                 m_leptonFlavour == lepFlav::Mu);

      // 1D histogram of MV2c10 for each trackjet
      if (trackjetsInLeadFJ.size() >= 1) {
        BookFillHist_VHQQ1Lep("TrkMV2c10B1", 100, -1, 1,
                              Props::MV2c10.get(trackjetsInLeadFJ.at(0)),
                              m_weight);
      }
      if (trackjetsInLeadFJ.size() >= 2) {
        BookFillHist_VHQQ1Lep("TrkMV2c10B2", 100, -1, 1,
                              Props::MV2c10.get(trackjetsInLeadFJ.at(1)),
                              m_weight);
      }

    }  //! m_doReduceFillHistos
  }    //! m_doOnlyInputs && !m_GenerateSTXS

  return EL::StatusCode::SUCCESS;
}  // fill1LepHistMerged

bool AnalysisReader_VHQQ1Lep::pass1LepTrigger(double &triggerSF_nominal,
                                              ResultVHbb1lep selectionResult) {
  const xAOD::Electron *el = selectionResult.el;
  const xAOD::Muon *mu = selectionResult.mu;
  const xAOD::MissingET *met = selectionResult.met;

  CP::SystematicSet sysSet;
  m_triggerTool->applySystematicVariation(sysSet);
  m_triggerTool->setEventInfo(m_eventInfo, m_randomRunNumber);
  m_triggerTool->setElectrons({el});
  m_triggerTool->setMuons({mu});
  m_triggerTool->setMET(met);

  bool triggerDec = ((TriggerTool_VHbb1lep *)m_triggerTool)
                        ->getDecisionAndSFwithMET(triggerSF_nominal);

  if (!triggerDec) return false;

  if (m_isMC) m_weight *= triggerSF_nominal;

  // handle systematics
  if (m_isMC && (m_currentVar == "Nominal")) {
    for (size_t i = 0; i < m_triggerSystList.size(); i++) {
      // not computing useless systematics
      if (el && (m_triggerSystList.at(i).find("MUON_EFF_Trig") !=
                 std::string::npos)) {
        m_weightSysts.push_back({m_triggerSystList.at(i), 1.0});
        continue;
      }
      if (mu &&
          (m_triggerSystList.at(i).find("EL_EFF_Trig") != std::string::npos)) {
        m_weightSysts.push_back({m_triggerSystList.at(i), 1.0});
        continue;
      }

      // syst of SF(sumpt) only for 3 jet case
      if (m_triggerSystList.at(i).find("METTrigSumpt") != std::string::npos &&
          !((m_physicsMeta.nSigJet >= 3) ||
            (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1)))
        continue;

      // get decision + weight
      double triggerSF = 1.;
      CP::SystematicSet sysSet(m_triggerSystList.at(i));
      m_triggerTool->applySystematicVariation(sysSet);
      ((TriggerTool_VHbb1lep *)m_triggerTool)
          ->getDecisionAndSFwithMET(triggerSF);

      if (triggerSF_nominal > 0)
        m_weightSysts.push_back(
            {m_triggerSystList.at(i), (float)(triggerSF / triggerSF_nominal)});
      else
        Error("pass1LepTrigger()",
              "Nominal trigger SF=0!, The systematics will not be generated.");
    }
  }
  return true;
}  // pass1LepTrigger

bool AnalysisReader_VHQQ1Lep::passLeptonPt27(ResultVHbb1lep selectionResult,
                                             const TLorentzVector &leptonVec,
                                             const TLorentzVector &WVecT) {
  const xAOD::Electron *el = selectionResult.el;
  const xAOD::Muon *mu = selectionResult.mu;

  bool passPtLep27 = false;

  if (m_model == Model::HVT || (el && !mu)) {
    if (leptonVec.Pt() > 27e3) passPtLep27 = true;
  } else if (WVecT.Pt() <= 150e3) {
    if (leptonVec.Pt() > 27e3) passPtLep27 = true;
  } else
    passPtLep27 = true;
  return passPtLep27;
}  // passLeptonPt27

bool AnalysisReader_VHQQ1Lep::passIsoInverted(ResultVHbb1lep selectionResult,
                                              const TLorentzVector &leptonVec) {
  const xAOD::Electron *el = selectionResult.el;
  const xAOD::Muon *mu = selectionResult.mu;
  float caloIso = -1.0;
  float trackIso = -1.0;

  bool passIsoInv = false;

  if (el) {
    caloIso = Props::topoetcone20.get(el) / leptonVec.Pt();
    trackIso = Props::ptvarcone20.get(el) / leptonVec.Pt();
    if (m_model == Model::HVT)
      passIsoInv = caloIso > 0.06 and trackIso < 0.06;
    else if (m_model == Model::CUT || m_model == Model::MVA)
      passIsoInv =
          Props::topoetcone20.get(el) > max(0.015 * leptonVec.Pt(), 3.5e3);
  } else if (mu) {
    caloIso = Props::ptvarcone30.get(mu) / leptonVec.Pt();
    if (m_model == Model::HVT)
      passIsoInv = caloIso > 0.06;
    else if (m_model == Model::CUT || m_model == Model::MVA) {
      if (Props::ptcone20.exists(
              mu))  // ptcone20 is not available for CxAODs before Tag28.
        passIsoInv = Props::ptcone20.get(mu) > 1.25e3;
      else
        passIsoInv = caloIso > 0.06;
    }
  }  // mu
  return passIsoInv;
}  // passIsoInverted

bool AnalysisReader_VHQQ1Lep::passNomIsolation(
    ResultVHbb1lep selectionResult, const TLorentzVector &leptonVec) {
  const xAOD::Electron *el = selectionResult.el;
  const xAOD::Muon *mu = selectionResult.mu;
  float caloIso = -1.0;
  float trackIso = -1.0;
  bool passNom = false;

  if (el) {
    caloIso = Props::topoetcone20.get(el) / leptonVec.Pt();
    trackIso = Props::ptvarcone20.get(el) / leptonVec.Pt();
    if (m_model == Model::HVT)
      passNom = caloIso < 0.06 and trackIso < 0.06;
    else if (m_model == Model::CUT || m_model == Model::MVA)
      passNom =
          Props::topoetcone20.get(el) < max(0.015 * leptonVec.Pt(), 3.5e3);
  } else if (mu) {
    caloIso = Props::ptvarcone30.get(mu) / leptonVec.Pt();
    if (m_model == Model::HVT)
      passNom = caloIso < 0.06;
    else if (m_model == Model::CUT || m_model == Model::MVA) {
      if (Props::ptcone20.exists(
              mu))  // ptcone20 is not available for CxAODs before Tag28.
        passNom = Props::ptcone20.get(mu) < 1.25e3;
      else
        passNom = caloIso < 0.06;
    }
  }  // mu
  return passNom;
}  // passNomIsolation

bool AnalysisReader_VHQQ1Lep::passLeptonQuality(
    ResultVHbb1lep selectionResult) {
  const xAOD::Electron *el = selectionResult.el;
  const xAOD::Muon *mu = selectionResult.mu;

  bool passLepQuality = false;
  if (el) {
    passLepQuality = Props::isTightLH.get(el) == 1;
  } else if (mu) {
    passLepQuality = Props::isWHSignalMuon.get(mu) == 1;
  }

  return passLepQuality;
}  // passLeptonQuality

bool AnalysisReader_VHQQ1Lep::passmTW(const TLorentzVector &WVecT) {
  bool passMTWcut = false;

  if (WVecT.Pt() > 150e3)
    passMTWcut = (m_model == Model::HVT)
                     ? (WVecT.M() > 0e3 && WVecT.M() < 300e3)
                     : WVecT.M() > 0e3;  // No mTW cut at high pTV
  else
    passMTWcut = WVecT.M() > 20e3;

  return passMTWcut;
}

bool AnalysisReader_VHQQ1Lep::passMET(const TLorentzVector &WVecT,
                                      const TLorentzVector &metVec,
                                      int nSignalEl, int nSignalMu,
                                      double m_minElMET, double m_minMuMET) {
  bool passMETCut = false;

  if (WVecT.Pt() > 150e3) {
    if (nSignalEl == 1 && nSignalMu == 0)
      passMETCut = metVec.Pt() > m_minElMET;
    else if (nSignalEl == 0 && nSignalMu == 1)
      passMETCut = metVec.Pt() > m_minMuMET;
  } else if (WVecT.Pt() <= 150e3) {
    if (nSignalEl == 1 && nSignalMu == 0)
      passMETCut = metVec.Pt() > 30e3;
    else if (nSignalEl == 0 && nSignalMu == 1)
      passMETCut = metVec.Pt() > 0e3;
  }

  return passMETCut;
}

float AnalysisReader_VHQQ1Lep::pTWFiltering(const TLorentzVector &lepton) {
  // merge the filtered ttbar events (mc 345951 and mc 346031) with the events
  // from the inclusive sample (410470) events in the inclusive samples are
  // weighted depending on the pt of the leptonically decaying W 410470: PP8
  // non-all-had ttbar ; 345951: pTW between 100 and 200 GeV 346031: ptW
  // higher than 200 GeV

  float pTWFilteringWeight = 1;

  if (m_mcChannel == 410470) {  // inclusive sample of nonallhad
    // pT of the truth W
    float pTW = -99.0;
    // pT and Eta of the truth lepton
    float pTL = -99.0;
    float EtaL = -99.0;
    //
    if (m_CxAODTag == "CxAODTag31") {
      // PDGID of the first W and and of the truth lepton, used to get the
      // charge sign
      int PDGIDW1 = -99, PDGIDL = -99;
      // transverse momentum of the two Ws
      double pTW1 = -99, pTW2 = -99;
      // Delta R between the reconstructed lepton (e or mu) and the truth lepton
      double DRMin = 10e5;
      // number of W bosons and number of tau leptons
      unsigned int nW = 0, nTau = 0;
      // check codeTTBarDecay
      if (!Props::codeTTBarDecay.exists(m_eventInfo)) {
        Error("run_1Lep_analysis()", "Props::codeTTBarDecay doesn't exist!");
        return EL::StatusCode::FAILURE;
      }
      // First W-->tau nu decay. Add weight only for events with tau undergo
      // leptonic decay
      if (Props::codeTTBarDecay.get(m_eventInfo) == 3) {
        // loop on truth particles
        for (auto *part : *m_truthParts) {
          if (part->isW() == true) {
            nW += 1;
            if (nW == 1) {
              pTW1 = part->pt();
              PDGIDW1 = part->pdgId();
            }
            if (nW == 2) {
              pTW2 = part->pt();
            }
            if (nW > 2) {
              Error("run_1Lep_analysis()", "Should have only 2W");
              return EL::StatusCode::FAILURE;
            }
          } else if (part->isTau()) {
            nTau += 1;
            if (nTau == 1) PDGIDL = part->pdgId();
          } else if (part->isElectron() || part->isMuon()) {
            TLorentzVector truthL = part->p4();
            if (lepton.DeltaR(truthL) < DRMin) {
              DRMin = lepton.DeltaR(truthL);
              pTL = part->pt();
              EtaL = part->eta();
            }
          }
        }
      } else if (Props::codeTTBarDecay.get(m_eventInfo) == 1 ||
                 Props::codeTTBarDecay.get(m_eventInfo) ==
                     2) {                   // only e+jet or mu+jet
        for (auto *part : *m_truthParts) {  // so should have 2 W and 1 lepton
          if (part->isW() == true) {
            nW += 1;
            if (nW == 1) {
              pTW1 = part->pt();
              PDGIDW1 = part->pdgId();
            }
            if (nW == 2) {
              pTW2 = part->pt();
            }
            if (nW > 2) {
              Error("run_1Lep_analysis()", "Should have only 2W");
              return EL::StatusCode::FAILURE;
            }
          } else if (part->isElectron() || part->isMuon()) {
            TLorentzVector truthL = part->p4();
            if (lepton.DeltaR(truthL) < DRMin) {
              DRMin = lepton.DeltaR(truthL);
              pTL = part->pt();
              EtaL = part->eta();
              PDGIDL = part->pdgId();
            }
          }
        }
      }
      //
      if (PDGIDL < 0)
        pTW = (PDGIDW1 > 0) ? pTW1 : pTW2;  //+Lepton +W
      else if (PDGIDL > 0)
        pTW = (PDGIDW1 < 0) ? pTW1 : pTW2;  //-Lepton -W
      //

    } else if (m_CxAODTag >= "CxAODTag32") {
      pTW = Props::ttbarpTW.get(m_eventInfo);
      pTL = Props::ttbarpTL.get(m_eventInfo);
      EtaL = Props::ttbarEtaL.get(m_eventInfo);
    } else {
      Error("run_1Lep_analysis()",
            "m_CxAODTag=%s is not supported. Only CxAODTag31 and CxAODTag32. "
            "Will ABORT!!!",
            m_CxAODTag.c_str());
      return EL::StatusCode::FAILURE;
    }
    // apply the reweighting for 410470
    bool isGoodTTBarDecay = (Props::codeTTBarDecay.get(m_eventInfo) == 1) ||
                            (Props::codeTTBarDecay.get(m_eventInfo) == 2) ||
                            (Props::codeTTBarDecay.get(m_eventInfo) == 3);
    if (isGoodTTBarDecay && pTL > 20e3 && fabs(EtaL) < 3) {
      if (pTW > 100e3 && pTW < 200e3) pTWFilteringWeight = 0.5;
      if (pTW > 200e3) pTWFilteringWeight = 0.25;
    }
  } else if (m_mcChannel == 345951) {
    pTWFilteringWeight = 0.5;
  } else if (m_mcChannel == 346031) {
    pTWFilteringWeight = 0.75;
  } else {
    pTWFilteringWeight = 1.0;
  }

  return pTWFilteringWeight;
}
EL::StatusCode AnalysisReader_VHQQ1Lep::fillMVATreeVHbbResolved1Lep(
    TLorentzVector &b1, TLorentzVector &b2, TLorentzVector &j3,
    const std::vector<const xAOD::Jet *> &selectedJets,
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &HVec,
    TLorentzVector &WVec, TLorentzVector &metVec, const xAOD::MissingET *&met,
    TLorentzVector &lep, int nTaus, float bTagWeight) {
  // m_tree->SetVariation(m_currentVar);

  // DO NOT ENTER NEW VARIABLES HERE IF NOT ABSOLUTELY NECESSARY FOR MVA
  // please use the EasyTree functionality for studies

  // first fill the common variables
  fillMVATreeVHbbResolved(b1, b2, j3, selectedJets, signalJets, forwardJets,
                          HVec, WVec, metVec, nTaus);

  // only take the first three jets into account
  float MEff = (b1.Pt() + b2.Pt() + j3.Pt() + lep.Pt() + metVec.Pt()) / 1e3;
  float Mtop = calculateMtop(lep, metVec, b1, b2);
  float dYWH = calculatedYWH(lep, metVec, b1, b2);

  m_tree->METSig = Props::metSig.get(met);
  m_tree->dYWH = dYWH;
  m_tree->dPhiLBmin = std::min(fabs(lep.DeltaPhi(b1)), fabs(lep.DeltaPhi(b2)));

  m_tree->bTagWeight = bTagWeight;
  m_tree->Mtop = Mtop / 1e3;
  m_tree->mTW = WVec.M() / 1e3;
  m_tree->etaL = lep.Eta();
  m_tree->phiL = lep.Phi();
  m_tree->pTL = lep.Pt() / 1e3;
  m_tree->MEff = MEff;

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHQQ1Lep::selectRegime(
    double WpT, int tagcat, const std::vector<const xAOD::Jet *> &fatJets,
    bool passResolved, bool passMerged, bool passResolvedSR, bool passMergedSR,
    bool passResolvedCR, bool passMergedCR) {
  // choose between
  // m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  // m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;

  // Possibility to turn off merged analysis
  if (m_analysisStrategy == "Resolved") {
    if (passResolved) m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "Merged") {
    if (passMerged) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "RecyclePtV") {
    // Give priority to merged above 500 GeV
    if (WpT / 1.e3 > 500.) {
      if (passMerged)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      else if (passResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    } else {
      if (passResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      else if (passMerged)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
  } else if (m_analysisStrategy == "SimpleMerge500") {
    // Only merged above 500 GeV
    if (WpT / 1.e3 > 500.) {
      if (passMerged) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    } else {
      if (passResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
  } else if (m_analysisStrategy == "PriorityResolved") {
    if (passResolved)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMerged)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "PriorityResolvedSR") {
    if (passResolvedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "PriorityResolvedSRbtag") {
    if (passResolvedSR && (tagcat == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR && (Props::nBTags.get(fatJets.at(0)) == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR && (tagcat == 1))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR && (Props::nBTags.get(fatJets.at(0)) == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "PriorityMerged") {
    if (passMerged)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolved)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "PriorityMergedSR") {
    if (passMergedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "PriorityMergedSRbtag") {
    if (passMergedSR && (Props::nBTags.get(fatJets.at(0)) == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR && (tagcat == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR && (Props::nBTags.get(fatJets.at(0)) == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR && (tagcat == 1))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  }

  return;
}

float AnalysisReader_VHQQ1Lep::calculateMtop(const TLorentzVector &lepton,
                                             const TLorentzVector &MET,
                                             const TLorentzVector &b_jet1,
                                             const TLorentzVector &b_jet2) {
  //  bool NeuPz_im;
  float min_mtop, NeuPz_1, NeuPz_2;
  int METShift = 1;
  TLorentzVector neu_tlv_1, neu_tlv_2;
  float mw = 80385;
  double tmp =
      mw * mw + 2. * lepton.Px() * MET.Px() + 2. * lepton.Py() * MET.Py();
  double METcorr = MET.Pt();
  if (tmp * tmp - pow(2. * lepton.Pt() * MET.Pt(), 2) < 0) {
    //    NeuPz_im = true;
    if (METShift > 0) {
      METcorr = 0.5 * mw * mw /
                (lepton.Pt() - lepton.Px() * cos(MET.Phi()) -
                 lepton.Py() * sin(MET.Phi()));
      double newtmp = mw * mw + 2. * lepton.Px() * METcorr * cos(MET.Phi()) +
                      2. * lepton.Py() * METcorr * sin(MET.Phi());
      NeuPz_1 = lepton.Pz() * newtmp / 2. / lepton.Pt() / lepton.Pt();
      NeuPz_2 = NeuPz_1;
    } else {
      NeuPz_1 = (lepton.Pz() * tmp) / 2. / lepton.Pt() / lepton.Pt();
      NeuPz_2 = NeuPz_1;
    }
  } else {
    //    NeuPz_im = false;
    NeuPz_1 =
        (lepton.Pz() * tmp +
         lepton.E() * sqrt(tmp * tmp - pow(2. * lepton.Pt() * MET.Pt(), 2))) /
        2. / lepton.Pt() / lepton.Pt();
    NeuPz_2 =
        (lepton.Pz() * tmp -
         lepton.E() * sqrt(tmp * tmp - pow(2. * lepton.Pt() * MET.Pt(), 2))) /
        2. / lepton.Pt() / lepton.Pt();
  }

  neu_tlv_1.SetPxPyPzE(METcorr * cos(MET.Phi()), METcorr * sin(MET.Phi()),
                       NeuPz_1, sqrt(pow(METcorr, 2) + pow(NeuPz_1, 2)));
  neu_tlv_2.SetPxPyPzE(METcorr * cos(MET.Phi()), METcorr * sin(MET.Phi()),
                       NeuPz_2, sqrt(pow(METcorr, 2) + pow(NeuPz_2, 2)));

  float mtop11 = (b_jet1 + lepton + neu_tlv_1).M();
  float mtop12 = (b_jet1 + lepton + neu_tlv_2).M();
  float mtop21 = (b_jet2 + lepton + neu_tlv_1).M();
  float mtop22 = (b_jet2 + lepton + neu_tlv_2).M();
  min_mtop = mtop11;
  if (min_mtop > mtop12) min_mtop = mtop12;
  if (min_mtop > mtop21) min_mtop = mtop21;
  if (min_mtop > mtop22) min_mtop = mtop22;

  return min_mtop;
}

float AnalysisReader_VHQQ1Lep::calculatedYWH(const TLorentzVector &lepton,
                                             const TLorentzVector &MET,
                                             const TLorentzVector &b_jet1,
                                             const TLorentzVector &b_jet2) {
  float NeuPz_1, NeuPz_2;
  float mw = 80385;
  TLorentzVector neu_tlv, Wz1, Wz2, Hz;
  double tmp =
      mw * mw + 2. * lepton.Px() * MET.Px() + 2. * lepton.Py() * MET.Py();

  if (tmp * tmp - pow(2. * lepton.Pt() * MET.Pt(), 2) < 0) {
    NeuPz_1 = (lepton.Pz() * tmp) / 2. / lepton.Pt() / lepton.Pt();
    NeuPz_2 = NeuPz_1;
  } else {
    NeuPz_1 =
        (lepton.Pz() * tmp +
         lepton.E() * sqrt(tmp * tmp - pow(2. * lepton.Pt() * MET.Pt(), 2))) /
        2. / lepton.Pt() / lepton.Pt();
    NeuPz_2 =
        (lepton.Pz() * tmp -
         lepton.E() * sqrt(tmp * tmp - pow(2. * lepton.Pt() * MET.Pt(), 2))) /
        2. / lepton.Pt() / lepton.Pt();
  }

  // TODO: gives the same results as
  // double NeuPz_1, NeuPz_2;
  // getNeutrinoPz(MET.Pt(), MET.Phi(), lepton, NeuPz_1, NeuPz_2);

  Wz1.SetPxPyPzE(0, 0, lepton.Pz() + NeuPz_1,
                 std::sqrt(mw * mw + std::pow(lepton.Pz() + NeuPz_1, 2)));
  Wz2.SetPxPyPzE(0, 0, lepton.Pz() + NeuPz_2,
                 std::sqrt(mw * mw + std::pow(lepton.Pz() + NeuPz_2, 2)));
  Hz.SetPxPyPzE(0, 0, b_jet1.Pz() + b_jet2.Pz(),
                std::sqrt((b_jet1 + b_jet2).M2() +
                          std::pow(b_jet1.Pz() + b_jet2.Pz(), 2)));

  float dBeta1 = std::fabs(Wz1.Beta() - Hz.Beta());
  float dBeta2 = std::fabs(Wz2.Beta() - Hz.Beta());

  if (dBeta1 < dBeta2)
    neu_tlv.SetPxPyPzE(
        MET.Px(), MET.Py(), NeuPz_1,
        sqrt(pow(MET.Px(), 2) + pow(MET.Py(), 2) + pow(NeuPz_1, 2)));
  else
    neu_tlv.SetPxPyPzE(
        MET.Px(), MET.Py(), NeuPz_2,
        sqrt(pow(MET.Px(), 2) + pow(MET.Py(), 2) + pow(NeuPz_2, 2)));

  float dYWH =
      std::fabs((lepton + neu_tlv).Rapidity() - (b_jet1 + b_jet2).Rapidity());

  return dYWH;
}

float AnalysisReader_VHQQ1Lep::calculatedYWFJ(const TLorentzVector &lepton,
                                              const TLorentzVector &MET,
                                              const TLorentzVector &fatjet1) {
  double NeuPz_1, NeuPz_2;  // two possible values for the neutrino pZ
  float mw = 80385.0;
  TLorentzVector neu_tlv, Wz1, Wz2, Hz;

  getNeutrinoPz(MET.Pt(), MET.Phi(), lepton, NeuPz_1, NeuPz_2);

  Wz1.SetPxPyPzE(0, 0, lepton.Pz() + NeuPz_1,
                 std::sqrt(mw * mw + std::pow(lepton.Pz() + NeuPz_1, 2)));
  Wz2.SetPxPyPzE(0, 0, lepton.Pz() + NeuPz_2,
                 std::sqrt(mw * mw + std::pow(lepton.Pz() + NeuPz_2, 2)));
  Hz.SetPxPyPzE(0, 0, fatjet1.Pz(),
                std::sqrt(fatjet1.M2() + std::pow(fatjet1.Pz(), 2)));

  float dBeta1 = std::fabs(Wz1.Beta() - Hz.Beta());
  float dBeta2 = std::fabs(Wz2.Beta() - Hz.Beta());

  if (dBeta1 < dBeta2)
    neu_tlv.SetPxPyPzE(
        MET.Px(), MET.Py(), NeuPz_1,
        sqrt(pow(MET.Px(), 2) + pow(MET.Py(), 2) + pow(NeuPz_1, 2)));
  else
    neu_tlv.SetPxPyPzE(
        MET.Px(), MET.Py(), NeuPz_2,
        sqrt(pow(MET.Px(), 2) + pow(MET.Py(), 2) + pow(NeuPz_2, 2)));

  float dYWFJ = std::fabs((lepton + neu_tlv).Rapidity() - fatjet1.Rapidity());

  return dYWFJ;
}

EL::StatusCode AnalysisReader_VHQQ1Lep::applyFFweight(
    const TLorentzVector &WVecT, const TLorentzVector &metVec,
    const xAOD::Electron *el) {
  if (!el) {
    return EL::StatusCode::FAILURE;
  }
  double ff_weight = 1.0;
  double iso = Props::topoetcone20.get(el);
  double ptcone = Props::ptvarcone20.get(el);
  double d0sigbl = Props::d0sigBL.get(el);
  bool mediumid = Props::isMediumLH.get(el);
  bool tightid = Props::isTightLH.get(el);
  m_FakeFactor_el->set_parameters(
      WVecT.Pt() / 1000., el->pt() / 1000., fabs(el->eta()),
      fabs(el->p4().DeltaPhi(metVec)), metVec.Pt() / 1000., iso / 1000.,
      d0sigbl, mediumid, tightid);
  if (m_currentVar == "MJ_Mu_METstr")
    ff_weight = m_FakeFactor_el->get_weight("Nominal");
  else
    ff_weight = m_FakeFactor_el->get_weight(m_currentVar);
  if (!((ptcone / el->pt()) < 0.06)) m_weight = 0.0;
  m_weight *= ff_weight;
  if (m_isMC) m_weight *= -1;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ1Lep::applyFFweight(
    const TLorentzVector &WVecT, const TLorentzVector &metVec,
    const xAOD::Muon *mu) {
  if (!mu) {
    return EL::StatusCode::FAILURE;
  }

  double ff_weight = 1.0;
  double iso = Props::ptvarcone30.get(mu);
  double d0sigbl = Props::d0sigBL.get(mu);
  bool mediumid = Props::isMedium.get(mu);
  bool tightid = Props::isTight.get(mu);
  m_FakeFactor_mu->set_parameters(
      WVecT.Pt() / 1000., mu->pt() / 1000., fabs(mu->eta()),
      fabs(mu->p4().DeltaPhi(metVec)), metVec.Pt() / 1000., iso / 1000.,
      d0sigbl, mediumid, tightid);
  if (m_currentVar == "MJ_El_EWK")
    ff_weight = m_FakeFactor_mu->get_weight("MJ_El_EWK");
  else
    ff_weight = m_FakeFactor_mu->get_weight("Nominal");
  m_weight *= ff_weight;
  if (m_currentVar == "MJ_Mu_METstr")
    m_weight *= get_sys_metstrmu(WVecT.M() / 1000.);
  if (WVecT.Pt() / 1000. < 150 && iso / mu->pt() > 0.07)
    ff_weight = 0;  // for lowpTV, denominator is 0.06 <ptcone/pt < 0.07
  if (m_isMC) m_weight *= -1;

  return EL::StatusCode::SUCCESS;
}

float AnalysisReader_VHQQ1Lep::get_sys_metstrmu(float mTW) {
  return 0.513 + 0.00318 * mTW;
}

EL::StatusCode AnalysisReader_VHQQ1Lep::applyElMuCS(
    string samplename, int nJet, ResultVHbb1lep selectionResult,
    const TLorentzVector &lepVec) {
  const xAOD::Electron *el = selectionResult.el;
  const xAOD::Muon *mu = selectionResult.mu;

  if (m_model == Model::HVT) {
    EL_CHECK("AnalysisReader_VHbb1Lep::applyCS_MJ()", applyCS_MJ(lepVec));
  } else if (m_model == Model::MVA || m_model == Model::CUT) {
    bool isTop = (samplename == "ttbar" || samplename == "stopWt" ||
                  samplename == "stopt" || samplename == "stops");
    bool isW = (samplename == "W");
    if (el) {
      TriggerInfo::DataPeriod period =
          TriggerInfo::getDataPeriodFromRun(m_eventInfo->runNumber());
      bool pass = false;
      if (period <= TriggerInfo::DataPeriod::data15)
        pass = Props::passHLT_e24_lhmedium_L1EM20VH.get(m_eventInfo);
      else if (period >= TriggerInfo::DataPeriod::data16A)
        pass = Props::passHLT_e26_lhtight_nod0_ivarloose.get(m_eventInfo);
      EL_CHECK("AnalysisReader_VHbb1Lep::applyCS_SMVHMJEl()",
               applyCS_SMVHMJEl(el, pass, isTop, isW, nJet));
    } else if (mu) {
      EL_CHECK("AnalysisReader_VHbb1Lep::applyCS_SMVHMJMu()",
               applyCS_SMVHMJMu(mu, isTop, isW, nJet));
    }
  }
  return EL::StatusCode::SUCCESS;
}  // applyCS

EL::StatusCode AnalysisReader_VHQQ1Lep::applyCS_SMVHMJEl(
    const xAOD::Electron *el, bool pass, bool isTop, bool isW, int nJet) {
  if (!el) {
    return EL::StatusCode::FAILURE;
  }
  float WeightReduced = 1;
  float WeightTrigger = 1;
  float WeightSFsCR = 1;
  for (auto s_csVariations : m_csVariations) {
    if (s_csVariations == "MJReduced") {
      if (Props::topoetcone20.get(el) > 11e3) {
        WeightReduced = .0;
      }
    } else if (s_csVariations == "MJTrigger") {
      if (!pass) {
        WeightTrigger = .0;
      }
    } else if (s_csVariations == "MJSFsCR") {
      if (isTop) {
        if (nJet == 2)
          WeightSFsCR = 0.937;
        else if (nJet == 3)
          WeightSFsCR = 0.928;
      } else if (isW) {
        if (nJet == 2)
          WeightSFsCR = 1.265;
        else if (nJet == 3)
          WeightSFsCR = 1.316;
      }
    }
    bool nominalOnly = false;
    m_config->getif<bool>("nominalOnly", nominalOnly);
    if ((m_csVariations.size() != 0) && !nominalOnly) {
      if (s_csVariations == "MJReduced") {
        m_weightSysts.push_back({(std::string)s_csVariations, WeightReduced});
      } else if (s_csVariations == "MJTrigger") {
        m_weightSysts.push_back({(std::string)s_csVariations, WeightTrigger});
      } else if (s_csVariations == "MJ2Tag") {
        m_weightSysts.push_back({(std::string)s_csVariations, 1});
      } else if (s_csVariations == "MJWOCorrection") {
        m_weightSysts.push_back({(std::string)s_csVariations, 1});
      } else if (s_csVariations == "MJSFsCR") {
        m_weightSysts.push_back({(std::string)s_csVariations, WeightSFsCR});
      } else {
        std::cout << "Unknown csVariation \"" << s_csVariations << "\""
                  << std::endl;
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}
EL::StatusCode AnalysisReader_VHQQ1Lep::applyCS_SMVHMJMu(const xAOD::Muon *mu,
                                                         bool isTop, bool isW,
                                                         int nJet) {
  if (!mu) {
    return EL::StatusCode::FAILURE;
  }
  float WeightReduced = 1;
  float WeightSFsCR = 1;
  for (auto s_csVariations : m_csVariations) {
    if (s_csVariations == "MJTrigger") {  // Only for el channel
      continue;
    } else if (s_csVariations == "MJReduced") {
      if (Props::ptcone20.get(mu) > 2.25e3) {
        WeightReduced = .0;
      }
    } else if (s_csVariations == "MJSFsCR") {
      if (isTop) {
        if (nJet == 2)
          WeightSFsCR = 0.937;
        else if (nJet == 3)
          WeightSFsCR = 0.928;
      } else if (isW) {
        if (nJet == 2)
          WeightSFsCR = 1.265;
        else if (nJet == 3)
          WeightSFsCR = 1.316;
      }
    }
    bool nominalOnly = false;
    m_config->getif<bool>("nominalOnly", nominalOnly);
    if ((m_csVariations.size() != 0) && !nominalOnly) {
      if (s_csVariations == "MJReduced") {
        m_weightSysts.push_back({(std::string)s_csVariations, WeightReduced});
      } else if (s_csVariations == "MJ2Tag") {
        m_weightSysts.push_back({(std::string)s_csVariations, 1});
      } else if (s_csVariations == "MJWOCorrection") {
        m_weightSysts.push_back({(std::string)s_csVariations, 1});
      } else if (s_csVariations == "MJSFsCR") {
        m_weightSysts.push_back({(std::string)s_csVariations, WeightSFsCR});
      } else {
        std::cout << "Unknown csVariation \"" << s_csVariations << "\""
                  << std::endl;
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ1Lep::applyCS_MJ(
    const TLorentzVector &lepVec) {
  // Apply a scale factor to account for trigger inefficiencies in the IsoInv
  // region Inefficiency only affects electron trigger

  // For the MJ_TrigEff uncertainty we have to determine in this function if
  // it's an electron or muon event.
  // Simplest thing to do is to check m_leptonFlavour, which is however only
  // set when running with doSplitLepFlavour option. Currently, this is always
  // the case! If this is changed later -> Error
  if (m_leptonFlavour == lepFlav::Combined) {
    Error("applyCS_MJ()",
          "applyCS_MJ not compatible with m_leptonFlavour == "
          "lepFlav::Combined (usually this means that doSplitLepFlavour is not "
          "set to true in the config).");
    return EL::StatusCode::FAILURE;
  }

  TriggerInfo::DataPeriod period =
      TriggerInfo::getDataPeriodFromRun(m_eventInfo->runNumber());
  double ScaleFactor = 1;
  double slope = 0;
  double offset = 1;
  for (auto s_csCorrections : m_csCorrections) {
    if (s_csCorrections == "MJ_TrigEff") {
      if (m_leptonFlavour != lepFlav::El) continue;
      if (period <= TriggerInfo::DataPeriod::data15) {
        slope = 0.000542;
        offset = 1.576;
      } else if (period >= TriggerInfo::DataPeriod::data16A) {
        slope = 0.0141;
        offset = 1.003;
      }
      ScaleFactor = slope * lepVec.Pt() / 1e3 + offset;
      m_weight *= ScaleFactor;
    }
  }
  // Do MJ (IsoInv) related Systematics
  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);

  if ((m_csVariations.size() != 0) && !nominalOnly) {
    for (auto s_csVariations : m_csVariations) {
      if (s_csVariations == "MJ_TrigEff") {
        if (m_leptonFlavour != lepFlav::El) continue;
        double slope_corr = 0;
        double offset_corr = 1;
        if (period == TriggerInfo::DataPeriod::data15) {
          slope_corr = 0.002331;
          offset_corr = 0.580;
        } else if (period == TriggerInfo::DataPeriod::data16A) {
          slope_corr = 0.0032;
          offset_corr = 0.670;
        }
        double weightUp = 1;
        double weightDo = 1;

        weightUp = ((slope + slope_corr) * lepVec.Pt() / 1e3 +
                    (offset - offset_corr)) /
                   ScaleFactor;
        weightDo = ((slope - slope_corr) * lepVec.Pt() / 1e3 +
                    (offset + offset_corr)) /
                   ScaleFactor;

        if (fabs(weightUp - 1) > 1.e-6 || fabs(weightDo - 1) > 1.e-6) {
          m_weightSysts.push_back({(std::string)s_csVariations + "__1up",
                                   static_cast<float>(weightUp)});
          m_weightSysts.push_back({(std::string)s_csVariations + "__1down",
                                   static_cast<float>(weightDo)});
        }
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}

// Interface member function designed to simply produce multiple histograms
// for lepton flavour
void AnalysisReader_VHQQ1Lep::BookFillHist_VHQQ1Lep(
    const string &name, int nbinsx, float xlow, float xup, float value,
    float weight, SystLevel level, bool PrintSplitFlav) {
  // Determine if we want to write systematics for the variable or not
  // HistSvc::doSysts::no = 0, HistSvc::doSysts::yes = 1
  HistSvc::doSysts ds_Split = HistSvc::doSysts::yes,
                   ds_Combined = HistSvc::doSysts::yes;
  if (level == SystLevel::None) {
    ds_Split = HistSvc::doSysts::no;
    ds_Combined = HistSvc::doSysts::no;
  } else if (level == SystLevel::CombinedOnly) {
    ds_Split = HistSvc::doSysts::no;
    ds_Combined = HistSvc::doSysts::yes;
  }

  // Check the member variable for the "Nominal" string if we do not want
  // systematics. I.e. do not print the systematic versions of the histograms
  if (level == SystLevel::None && m_currentVar != "Nominal") {
    return;
  }

  // Fill the user defined histogram
  m_histSvc->BookFillHist(name, nbinsx, xlow, xup, value, weight, ds_Combined);
  // If set then fill the lepton flavour specific
  if (m_leptonFlavour == lepFlav::El && PrintSplitFlav) {
    m_histSvc->BookFillHist("El_" + name, nbinsx, xlow, xup, value, weight,
                            ds_Split);
  } else if (m_leptonFlavour == lepFlav::Mu && PrintSplitFlav) {
    m_histSvc->BookFillHist("Mu_" + name, nbinsx, xlow, xup, value, weight,
                            ds_Split);
  }
}

// Plotting function to produced cut based histograms at same time as running
// inputs for MVA
void AnalysisReader_VHQQ1Lep::BookFillHist_SMCut(const string &name, int nbinsx,
                                                 float xlow, float xup,
                                                 float value, float weight,
                                                 SystLevel level,
                                                 bool PrintSplitFlav) {
  m_histNameSvc->set_analysisType(
      HistNameSvc::AnalysisType::CUT);  // Have to switch to cut based
                                        // analysis to get right pTV splitting
  BookFillHist_VHQQ1Lep(name + "CutBased", nbinsx, xlow, xup, value, weight,
                        level, PrintSplitFlav);
  m_histNameSvc->set_analysisType(
      HistNameSvc::AnalysisType::MVA);  // And now switch back to MVA
                                        // (assuming that this function is
                                        // only being called by MVA analysis
}

EL::StatusCode AnalysisReader_VHQQ1Lep::initializeCuts() {
  ///////////////////////////////////////////////////
  // DEFINITION OF CUTS FOR DIFFERENT REGIONS     ///
  ///////////////////////////////////////////////////

  cuts_SR_resolved = {OneLeptonResolvedCuts::AllCxAOD,
                      OneLeptonResolvedCuts::Trigger,
                      OneLeptonResolvedCuts::LooseLeptons,
                      OneLeptonResolvedCuts::SignalLeptons,
                      OneLeptonResolvedCuts::AtLeast2Jets,
                      OneLeptonResolvedCuts::AtLeast2SigJets,
                      OneLeptonResolvedCuts::Pt45,
                      OneLeptonResolvedCuts::MET,
                      OneLeptonResolvedCuts::mTW,
                      OneLeptonResolvedCuts::Veto3bjet,
                      OneLeptonResolvedCuts::mbbRestrict,
                      OneLeptonResolvedCuts::mbbCorr,
                      OneLeptonResolvedCuts::pTV};

  cuts_CR_resolved = {OneLeptonResolvedCuts::AllCxAOD,
                      OneLeptonResolvedCuts::Trigger,
                      OneLeptonResolvedCuts::LooseLeptons,
                      OneLeptonResolvedCuts::SignalLeptons,
                      OneLeptonResolvedCuts::AtLeast2Jets,
                      OneLeptonResolvedCuts::AtLeast2SigJets,
                      OneLeptonResolvedCuts::Pt45,
                      OneLeptonResolvedCuts::MET,
                      OneLeptonResolvedCuts::mTW,
                      OneLeptonResolvedCuts::Veto3bjet,
                      OneLeptonResolvedCuts::mbbRestrict,
                      OneLeptonResolvedCuts::pTV};

  cuts_model_resolved = {OneLeptonResolvedCuts::AllCxAOD,
                         OneLeptonResolvedCuts::Trigger,
                         OneLeptonResolvedCuts::LooseLeptons,
                         OneLeptonResolvedCuts::SignalLeptons,
                         OneLeptonResolvedCuts::AtLeast2Jets,
                         OneLeptonResolvedCuts::AtLeast2SigJets,
                         OneLeptonResolvedCuts::Pt45,
                         OneLeptonResolvedCuts::MET,
                         OneLeptonResolvedCuts::mTW,
                         OneLeptonResolvedCuts::Veto3bjet,
                         OneLeptonResolvedCuts::mbbCorr,
                         OneLeptonResolvedCuts::pTV,
                         OneLeptonResolvedCuts::dRBB,
                         OneLeptonResolvedCuts::mTW_add};

  cuts_SR_merged = {OneLeptonMergedCuts::AllCxAOD,
                    OneLeptonMergedCuts::Trigger,
                    OneLeptonMergedCuts::Leptons,
                    OneLeptonMergedCuts::MET,
                    OneLeptonMergedCuts::AtLeast1FatJet,
                    OneLeptonMergedCuts::mbbCorr,
                    OneLeptonMergedCuts::AtLeast2TrackJets,
                    OneLeptonMergedCuts::passVRJetOR,
                    OneLeptonMergedCuts::mTW,
                    OneLeptonMergedCuts::pTV,
                    OneLeptonMergedCuts::mJ,
                    OneLeptonMergedCuts::dYWFJ};

  cuts_CR_merged = {OneLeptonMergedCuts::AllCxAOD,
                    OneLeptonMergedCuts::Trigger,
                    OneLeptonMergedCuts::Leptons,
                    OneLeptonMergedCuts::MET,
                    OneLeptonMergedCuts::AtLeast1FatJet,
                    OneLeptonMergedCuts::AtLeast2TrackJets,
                    OneLeptonMergedCuts::mTW,
                    OneLeptonMergedCuts::pTV,
                    OneLeptonMergedCuts::mbbRestrict};

  cuts_resolved_CUT = {OneLeptonResolvedCuts::AllCxAOD,
                       OneLeptonResolvedCuts::Trigger,
                       OneLeptonResolvedCuts::LooseLeptons,
                       OneLeptonResolvedCuts::SignalLeptons,
                       OneLeptonResolvedCuts::AtLeast2Jets,
                       OneLeptonResolvedCuts::AtLeast2SigJets,
                       OneLeptonResolvedCuts::Pt45,
                       OneLeptonResolvedCuts::MET,
                       OneLeptonResolvedCuts::mTW,
                       OneLeptonResolvedCuts::Veto3bjet,
                       OneLeptonResolvedCuts::mbbRestrict,
                       OneLeptonResolvedCuts::pTV,
                       OneLeptonResolvedCuts::dRBB,
                       OneLeptonResolvedCuts::mTW_add};

  cuts_resolved = {OneLeptonResolvedCuts::AllCxAOD,
                   OneLeptonResolvedCuts::Trigger,
                   OneLeptonResolvedCuts::LooseLeptons,
                   OneLeptonResolvedCuts::SignalLeptons,
                   OneLeptonResolvedCuts::AtLeast2Jets,
                   OneLeptonResolvedCuts::AtLeast2SigJets,
                   OneLeptonResolvedCuts::Pt45,
                   OneLeptonResolvedCuts::MET,
                   OneLeptonResolvedCuts::mTW,
                   OneLeptonResolvedCuts::Veto3bjet,
                   OneLeptonResolvedCuts::mbbRestrict,
                   OneLeptonResolvedCuts::pTV};

  cuts_merged = {OneLeptonMergedCuts::AllCxAOD,
                 OneLeptonMergedCuts::Trigger,
                 OneLeptonMergedCuts::Leptons,
                 OneLeptonMergedCuts::MET,
                 OneLeptonMergedCuts::AtLeast1FatJet,
                 OneLeptonMergedCuts::AtLeast2TrackJets,
                 OneLeptonMergedCuts::passVRJetOR,
                 OneLeptonMergedCuts::mTW,
                 OneLeptonMergedCuts::pTV,
                 OneLeptonMergedCuts::dYWFJ};

  cuts_easytree_merged = {OneLeptonMergedCuts::AllCxAOD,
                          OneLeptonMergedCuts::Trigger,
                          OneLeptonMergedCuts::Leptons,
                          OneLeptonMergedCuts::MET,
                          OneLeptonMergedCuts::AtLeast1FatJet,
                          OneLeptonMergedCuts::AtLeast2TrackJets,
                          OneLeptonMergedCuts::passVRJetOR,
                          OneLeptonMergedCuts::mTW};

  cuts_easytree_resolved = {OneLeptonResolvedCuts::AllCxAOD,
                            OneLeptonResolvedCuts::LooseLeptons,
                            OneLeptonResolvedCuts::SignalLeptons,
                            OneLeptonResolvedCuts::AtLeast2Jets,
                            OneLeptonResolvedCuts::AtLeast2SigJets,
                            OneLeptonResolvedCuts::Pt45,
                            OneLeptonResolvedCuts::MET,
                            OneLeptonResolvedCuts::mTW,
                            OneLeptonResolvedCuts::Veto3bjet,
                            OneLeptonResolvedCuts::mbbRestrict};

  cuts_cutflow_resolved = {OneLeptonResolvedCuts::AllCxAOD,
                           OneLeptonResolvedCuts::LooseLeptons,
                           OneLeptonResolvedCuts::SignalLeptons,
                           OneLeptonResolvedCuts::AtLeast2Jets,
                           OneLeptonResolvedCuts::AtLeast2SigJets,
                           OneLeptonResolvedCuts::Pt45};

  return EL::StatusCode::SUCCESS;
}
