#include <type_traits>

#include <EventLoop/IWorker.h>

#include "TSystem.h"

#include "CxAODReader/AnalysisReader.h"
#include "CxAODReader/EasyTree.h"
#include "CxAODReader/OSTree.h"
#include "CxAODReader_VHbb/AnalysisReader_VBFHbb1Ph.h"
#include "CxAODReader_VHbb/AnalysisReader_VGamma.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
#include "CxAODReader_VHbb/AnalysisReader_VHbb0Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHbb1Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHbb2Lep.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VBFHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/VHbb0lepEvtSelection.h"
#include "KinematicFit/KinematicFit.h"

#include "CxAODTools_VHbb/VHbb2lepEvtSelection.h"

#define length(array) (sizeof(array) / sizeof(*(array)))

ClassImp(AnalysisReader_VHQQ)  //; - comment required for proper auto-formatting

    AnalysisReader_VHQQ::AnalysisReader_VHQQ()
    : AnalysisReader(),
      m_analysis(""),
      m_analysisType(""),
      m_analysisStrategy("SimpleMerge500"),
      m_doTruthTagging(false),
      m_doOnlyInputs(false),
      m_doNewRegions(false),
      m_doReduceFillHistos(false),
      m_GenerateSTXS(false),
      m_tree(nullptr),
      m_jetCorrType("PtReco"),
      m_fatJetCorrType("hbbVR"),
      m_doFSRrecovery(false),
      m_hasFSR(0),
      m_tagStrategy("AllSignalJets"),
      m_boostedTagStrategy("Leading2SignalJets"),
      m_analysisContext(nullptr),
      // boosted 0 lep, I'll try to reduce (Camilla)
      m_use2DbTagCut(false),
      m_doMETTriggerin2L(false),
      m_doMETMuonTrigger(false),
      m_doFillHistograms(false),
      m_doMergeCR(false),
      m_doCutflow(false),
      m_BDTSyst_debug(false),
      m_doMbbMonitor(false),
      m_doKF(true),
      m_doBinnedSyst(false),
      m_doSherpaSyst(false),
      m_doVRJetOR(true),
      m_SplitBoostedInputsAddJets(true),
      m_doRemoveDilepOverlap(false),
      m_histFastSvc(),
      m_fatJetHistos(),
      m_isStoreMJwC2(false),
      m_model(Model::undefined),
      m_writeObjectsInEasyTree(false),
      m_corrsAndSysts(nullptr),
      m_allLeadFJMatchedTrackJets() {
  m_csCorrections.clear();
  m_csVariations.clear();
  EL::StatusCode rc = initialiseObjectBranches();
  if (rc != EL::StatusCode::SUCCESS) {
    ATH_MSG_ERROR(
        "Cannot initialise object branches in AnalysisReader_VHQQ constructor");
    return;
  }

  // Function for 2/3jet defined from 10%on WZ and 95%/85% of the signal
  m_fit_func_2j10 = new TF1("fit_func_2j10", "[0]+exp([1]+[2]*x)", 75, 500);
  m_fit_func_3j10 = new TF1("fit_func_3j10", "[0]+exp([1]+[2]*x)", 75, 500);
  m_fit_func_2j95 = new TF1("fit_func_2j95", "[0]+exp([1]+[2]*x)", 75, 500);
  m_fit_func_3j85 = new TF1("fit_func_3j85", "[0]+exp([1]+[2]*x)", 75, 500);

  // 10% 2jet
  m_fit_func_2j10->SetParameter(0, 3.99879e-01);
  m_fit_func_2j10->SetParameter(1, 7.88279e-01);
  m_fit_func_2j10->SetParameter(2, -1.02303e-02);

  // 95% 2jet
  m_fit_func_2j95->SetParameter(0, 8.70286e-01);
  m_fit_func_2j95->SetParameter(1, 1.37867e+00);
  m_fit_func_2j95->SetParameter(2, -7.95322e-03);

  // 10% 3jet
  m_fit_func_3j10->SetParameter(0, 4.20840e-01);
  m_fit_func_3j10->SetParameter(1, 2.67742e-01);
  m_fit_func_3j10->SetParameter(2, -8.08892e-03);

  // 85% 3jet
  m_fit_func_3j85->SetParameter(0, 7.63283e-01);
  m_fit_func_3j85->SetParameter(1, 1.33179e+00);
  m_fit_func_3j85->SetParameter(2, -7.30396e-03);
}

AnalysisReader_VHQQ::~AnalysisReader_VHQQ() {}

EL::StatusCode AnalysisReader_VHQQ::initializeSelection() {
  // reset physics metadata
  m_physicsMeta = PhysicsMetadata();
  std::string model = m_config->get<std::string>("modelType");
  if (model == "AZh")
    m_model = Model::AZh;
  else if (model == "HVT")
    m_model = Model::HVT;
  else if (model == "CUT")
    m_model = Model::CUT;
  else if (model == "MVA")
    m_model = Model::MVA;
  else
    m_model = Model::undefined;

  m_config->getif<string>("analysis", m_analysis);
  m_config->getif<string>("analysisType", m_analysisType);
  m_config->getif<string>("analysisStrategy", m_analysisStrategy);
  m_config->getif<bool>("doVRJetOR", m_doVRJetOR);
  m_config->getif<bool>("SplitBoostedInputsAddJets",
                        m_SplitBoostedInputsAddJets);

  // Select analysis strategy according to model
  if (m_model == Model::MVA && m_analysisStrategy != "Resolved") {
    Error("initializeSelection()",
          "Running with MVA selecction. Only possible for resolved analysis, "
          "but %s "
          "is chosen. Exiting!",
          m_analysisStrategy.c_str());
    return EL::StatusCode::FAILURE;
  }

  // For VH-reso analyses default is to merge ptv bins
  if (m_model == Model::AZh || m_model == Model::HVT) m_doMergePtVBins = true;

  // For 2D cut only use merged
  m_config->getif<bool>("use2DbTagCut", m_use2DbTagCut);
  if (m_use2DbTagCut) {
    if (m_analysisStrategy != "Merged") {
      Error("initializeSelection()",
            "Running with 2DbTagCut. Only possible for merged analysis, but %s "
            "is chosen. Exiting!",
            m_analysisStrategy.c_str());
      return EL::StatusCode::FAILURE;
    }
    Warning("initializeSelection()",
            "Running with 2DbTagCut. b-tagging for calo jets disabled!!");
  }

  Info("initializeSelection()", "Initialize analysis '%s %s'.",
       m_analysisType.c_str(), m_analysisStrategy.c_str());

  // Trees
  //------
  // TODO move to base class?
  bool writeMVATree = false;
  bool readMVA = false;
  std::string MVAxmlFileName = "";
  m_config->getif<bool>("writeMVATree", writeMVATree);
  m_config->getif<bool>("readMVA", readMVA);
  m_config->getif<std::string>("MVAxmlFileName", MVAxmlFileName);
  bool writeEasyTree = false;
  bool writeObjectsInEasyTree = false;
  m_config->getif<bool>("writeEasyTree", writeEasyTree);
  m_config->getif<bool>("writeObjectsInEasyTree", writeObjectsInEasyTree);

  if (m_writeObjectsInEasyTree && !writeEasyTree) writeEasyTree = true;
  if (writeEasyTree && writeMVATree) writeMVATree = false;

  // initialize MVATree
  m_tree = new MVATree_VHbb(writeMVATree, readMVA, m_analysisType, wk(),
                            m_variations, false);

  if (m_analysisType == "0lep" && m_analysisStrategy != "Resolved") {
    m_boostedtree = new MVATree_BoostedVHbb(
        writeMVATree, readMVA, m_analysisType, wk(), m_variations, false);
  }

  if ((m_analysisType == "vbf") || (m_analysisType == "vbfa")) {
    delete m_tree;
    m_tree = nullptr;
    m_tree_vbf = new MVATree_VBFHbb(writeMVATree, readMVA, m_analysisType, wk(),
                                    m_variations, false, MVAxmlFileName);
  }

  // initialize EasyTree
  m_etree =
      new EasyTree(writeEasyTree, m_analysisType, wk(), m_variations, false);

  // if objects in easytree
  if (m_writeObjectsInEasyTree) {
    EL_CHECK("AnalysisReader_VHQQ::initializeSelection()",
             defineObjectBranches());
    EL_CHECK("AnalysisReader_VHQQ::initializeSelectiom()",
             resetObjectBranches());
  }

  bool writeOSTree = false;
  m_config->getif<bool>("writeOSTree", writeOSTree);
  m_OStree =
      new OSTree(writeOSTree, readMVA, m_analysisType, wk(), {"physics"}, true);

  // Fill reduced set of histograms
  m_config->getif<bool>("doReduceFillHistos", m_doReduceFillHistos);

  // Fill only histograms for fit inputs
  m_config->getif<bool>("doOnlyInputs", m_doOnlyInputs);

  // Define new SR and CRs using dRBB vs. pTV Cuts
  m_config->getif<bool>("doNewRegions", m_doNewRegions);

  // Generate STXS histograms
  m_config->getif<bool>("generateSTXSsignals", m_GenerateSTXS);

  // Run bootstrap
  m_config->getif<bool>("doBootstrap", m_doBootstrap);

  // do truth tagging?
  m_config->getif<bool>("doTruthTagging", m_doTruthTagging);

  m_config->getif<bool>("fillCr", m_doFillCR);

  m_config->getif<string>("jetCorrType", m_jetCorrType);

  m_config->getif<bool>("doFSRrecovery", m_doFSRrecovery);

  m_config->getif<string>("fatjetCorrType", m_fatJetCorrType);
  if (m_CxAODTag == "CxAODTag26") {
    m_fatJetCorrType = "XbbMu";
  } else if (m_CxAODTag == "CxAODTag28") {
    m_fatJetCorrType = "xbb";
  } else if (m_CxAODTag == "CxAODTag31") {
    m_fatJetCorrType = "xbb";
    if (m_analysisType == "1lep") {
      m_fatJetCorrType = "Nominal";
    }
  }
  Info("initializeSelection()", "%s using %s", m_CxAODTag.c_str(),
       m_fatJetCorrType.c_str());

  // 2L MET Trigger addition
  m_config->getif<bool>("doMETTriggerin2L", m_doMETTriggerin2L);

  // Adds ability to trigger on both the MET and the Muon triggers
  // simultaneously in 1L and 2L analyses
  m_config->getif<bool>("doMETMuonTrigger", m_doMETMuonTrigger);

  m_config->getif<std::string>(
      "tagStrategy",
      m_tagStrategy);  // AllSignalJets,Leading2SignalJets,LeadingSignalJets

  m_config->getif<std::string>(
      "boostedTagStrategy",
      m_boostedTagStrategy);  // AllSignalJets,Leading2SignalJets

  m_config->getif<std::vector<std::string>>("csCorrections", m_csCorrections);
  m_config->getif<std::vector<std::string>>("csVariations", m_csVariations);

  // initialise the BDT systematic part
  m_BDTSyst = NULL;
  int channel = 0;
  if (m_analysisType == "1lep") channel = 1;
  if (channel == 0 || channel == 1) {
    m_BDTSyst = new BDTSyst();
    m_BDTSyst->Initialise(channel);
  }

  m_config->getif<std::vector<std::string>>("BDTSystVariations",
                                            m_BDTSystVariations);

  m_config->getif<bool>("BDTSyst_debug", m_BDTSyst_debug);

  m_config->getif<bool>("doMergeJetBins", m_doMergeJetBins);
  m_config->getif<bool>("doMergePtVBins", m_doMergePtVBins);
  m_config->getif<bool>("doMergeCR", m_doMergeCR);
  m_config->getif<bool>("doMbbMonitor", m_doMbbMonitor);
  m_config->getif<bool>("doKF", m_doKF);
  m_config->getif<bool>("doBinnedSyst", m_doBinnedSyst);
  m_config->getif<bool>("doSherpaSyst", m_doSherpaSyst);

  m_config->getif<string>("kfConfig", m_kfConfig);

  bool doPtvSplitting250GeV = false;
  m_config->getif<bool>("doPtvSplitting250GeV", doPtvSplitting250GeV);
  m_histNameSvc->set_doPtvSplitting250GeV(doPtvSplitting250GeV);

  // Initialize c2 cut configuration
  std::vector<float> C2Cuts;

  m_config->getif<bool>("isStoreMJwC2", m_isStoreMJwC2);
  m_config->getif<std::vector<float>>("C2Cuts", C2Cuts);
  m_c2Cuts.clear();

  for (auto cut : C2Cuts) {
    std::stringstream ss;
    ss << std::fixed << setprecision(2) << cut;
    TString cut_info = "C2" + ss.str();
    cut_info.ReplaceAll(".", "P");

    m_c2Cuts[cut_info.Data()] = cut;
  }

  return EL::StatusCode::SUCCESS;
}  // initializeSelection

EL::StatusCode AnalysisReader_VHQQ::initializeCorrsAndSysts() {
  // m_doTruthTagging is retrieved BEFORE m_isMC, so we need to put it after
  // This isn't the best sytlistically, but I'm not sure where else to put this
  // cross-check
  m_doTruthTagging &= m_isMC;

  if (!m_isMC) return EL::StatusCode::SUCCESS;

  std::string comEnergy = m_config->get<std::string>("COMEnergy");

  if ((comEnergy != "8TeV") || (comEnergy != "7TeV")) comEnergy = "13TeV";
  TString csname;
  std::string debugname;

  if (m_analysisType == "0lep") {
    csname = comEnergy + "_ZeroLepton";
    debugname = comEnergy + "_ZeroLepton";
  }

  if (m_analysisType == "1lep") {
    csname = comEnergy + "_OneLepton";
    debugname = comEnergy + "_OneLepton";
  }

  if (m_analysisType == "2lep") {
    csname = comEnergy + "_TwoLepton";
    debugname = comEnergy + "_TwoLepton";
  }

  if (m_analysisType == "vbfa" || m_analysisType == "vgamma") {
    csname = comEnergy + "_TwoLepton";
    debugname = comEnergy + "_TwoLepton";
  }  // fake config for vbfa analysis. Is it possible to skip it?

  Info("initializeCorrsAndSysts()", "Initializing CorrsAndSysts for %s",
       debugname.c_str());

  m_corrsAndSysts = new CorrsAndSysts(csname);

  return EL::StatusCode::SUCCESS;
}  // initializeCorrsAndSysts

EL::StatusCode AnalysisReader_VHQQ::initializeVariations() {
  EL_CHECK("AnalysisReader_VHQQ::initializeVariations()",
           AnalysisReader::initializeVariations());

  string analysisType = "";
  bool nominalOnly = false;
  m_config->getif<string>("analysisType", analysisType);
  // TODO member n_nominalOnly would be useful
  m_config->getif<bool>("nominalOnly", nominalOnly);

  // trigger syst
  // -------------
  // TODO move triggerSystList to config?
  m_triggerSystList.clear();
  std::vector<std::string> triggerSystList;
  m_config->getif<std::vector<std::string>>("triggerSystList", triggerSystList);
  if (!nominalOnly) {
    std::string finalTriggerSyst;
    for (auto triggerSyst : triggerSystList) {
      // up
      finalTriggerSyst = triggerSyst + "__1up";
      Info("AnalysisReader_VHQQ::initializeVariations()",
           finalTriggerSyst.c_str());
      m_triggerSystList.push_back(finalTriggerSyst);
      // down
      finalTriggerSyst = triggerSyst + "__1down";
      Info("AnalysisReader_VHQQ::initializeVariations()",
           finalTriggerSyst.c_str());
      m_triggerSystList.push_back(finalTriggerSyst);
    }
  }

  // PU systematics - temporary fix to run PU syst in v26
  //---------------
  if (m_isMC && m_CxAODTag == "CxAODTag26" &&
      !m_config->get<bool>("nominalOnly")) {
    std::vector<std::string> variations;
    m_config->getif<std::vector<std::string>>("variations", variations);
    for (std::string varNameIn : variations) {
      if (varNameIn.find("PRW_DATASF") == 0) {
        // make sure to not pushback twice the same syst
        bool alreadyPresent = false;
        for (std::string varName : m_variations) {
          if (varName.find(varNameIn) == 0) alreadyPresent = true;
        }
        if (!alreadyPresent) {
          m_variations.push_back("PRW_DATASF__1up");
          m_variations.push_back("PRW_DATASF__1down");
        }
      }
    }
  }

  return EL::StatusCode::SUCCESS;
}  // initializeVariations

EL::StatusCode AnalysisReader_VHQQ::initializeTools() {
  EL_CHECK(
      "AnalysisReader_VHQQ::initializeTools() initializeTools() from "
      "CxAODReader",
      AnalysisReader::initializeTools());
  EL_CHECK("AnalysisReader_VHQQ::initializeTools() initializeCorrsAndSysts()",
           initializeCorrsAndSysts());

  // KinematicFitter
  // -----------------
  m_KF = new KF::KinematicFit();
  std::string datadir_KF = gSystem->Getenv("WorkDir_DIR");
  datadir_KF += "/data/KinematicFit/";

  m_KF->Init(datadir_KF, m_kfConfig, m_isMC);

  std::string ffin_el = m_config->get<std::string>("fakerate_file_el");
  std::string ffin_mu = m_config->get<std::string>("fakerate_file_mu");

  m_FakeFactor_el = new FakeFactor(*m_config, 0);
  m_FakeFactor_mu = new FakeFactor(*m_config, 1);
  EL_CHECK("AnalysisReader::initializeTools()",
           m_FakeFactor_el->initialize(ffin_el));
  EL_CHECK("AnalysisReader::initializeTools()",
           m_FakeFactor_mu->initialize(ffin_mu));

  if (!m_analysisContext) {
    m_analysisContext = new AnalysisContext;
  }
  m_analysisContext->setWorker(wk());
  m_analysisContext->setConfigStore(m_config);

  return EL::StatusCode::SUCCESS;
}  // initializeTools

EL::StatusCode AnalysisReader_VHQQ::finalizeTools() {
  delete m_KF;

  if (m_writeObjectsInEasyTree) {
    EL_CHECK("AnalysisReader_VHQQ::finalizeTools()", finaliseObjectBranches());
  }

  EL_CHECK("AnalysisReader_VHQQ::finalizeTools()",
           AnalysisReader::finalizeTools());

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHQQ::truthVariablesCS(float &truthPt, float &avgTopPt)

{
  truthPt = 0;
  avgTopPt = 0;

  std::string samplename = m_histNameSvc->get_sample();

  if (samplename ==
      "ttbar")  // average pT of top,antitop truth pair
                // we need 2 truth-level quantities: average top pt (to get
                // SysTopPt) and pT(ttbar) (to get SysTTbarPt) use avgTopPt for
                // SysTopPt and truthPt for SysTTbarPt
  {
    TLorentzVector p4_ttbar(0, 0, 0, 0);
    for (auto *part : *m_truthParts) {
      if (fabs(part->pdgId()) == 6) {
        p4_ttbar = p4_ttbar + part->p4();
        avgTopPt += (part->pt() / 2.0);
        // truthPt += (part->pt()/2.0);
      }
    }
    truthPt = p4_ttbar.Pt();
  }

  if (samplename == "Z" || samplename == "W" || (samplename == "Zv22") ||
      (samplename ==
       "Wv22"))  // take truth pT of truth V candidate built from truth leptons
  {
    TLorentzVector l1, l2, V;
    for (auto *part : *m_truthParts) {
      if (part->pdgId() == 11 || part->pdgId() == 12 || part->pdgId() == 13 ||
          part->pdgId() == 14 || part->pdgId() == 15 || part->pdgId() == 16)
        l1 = part->p4();
      if (part->pdgId() == -11 || part->pdgId() == -12 ||
          part->pdgId() == -13 || part->pdgId() == -14 ||
          part->pdgId() == -15 || part->pdgId() == -16)
        l2 = part->p4();
    }
    V = l1 + l2;
    truthPt = V.Pt();
  }

  if (samplename == "WW" ||
      samplename == "ZZ")  // cannot select the leptonic one -> take average pT
  {
    for (auto *part : *m_truthParts) {
      if (fabs(part->pdgId()) == 23 || fabs(part->pdgId()) == 24)
        truthPt += (part->pt() / 2.0);
    }
  }

  if (samplename == "WZ")  // can discriminate between leptonic and hadronic V
                           // -> take leptonic V truth pT
  {
    int pdgId = -1;
    if (m_mcChannel == 361083) pdgId = 24;  // WlvZqq
    if (m_mcChannel == 361084) pdgId = 23;  // WqqZll
    if (m_mcChannel == 361085) pdgId = 23;  // WqqZvv
    for (auto *part : *m_truthParts) {
      if (fabs(part->pdgId()) == pdgId) truthPt = (part->pt());
    }
  }

  if (samplename == "qqZllH125" || samplename == "qqZvvH125" ||
      samplename == "qqWlvH125" || samplename == "ggZllH125" ||
      samplename == "ggZvvH125") {
    int pdgId = 23;
    if (samplename == "qqWlvH125") pdgId = 24;
    for (auto *part : *m_truthParts) {
      if (fabs(part->pdgId()) == pdgId && part->status() == 62)
        truthPt = (part->pt());
    }
  }
}  // truthVariablesCS

EL::StatusCode AnalysisReader_VHQQ::applyCorrections() {
  float truthPt = -1;
  float cs_avgTopPt = -1;
  truthVariablesCS(truthPt, cs_avgTopPt);
  std::string detailSampleNameClean =
      m_xSectionProvider->getSampleDetailName(m_mcChannel);
  std::string sampleNameClean = m_histNameSvc->get_sample();
  sampleCS sample("temp");
  sample.setup_sample(sampleNameClean, detailSampleNameClean);
  std::string flav = "";

  if ((sampleNameClean == "Z") || (sampleNameClean == "W") ||
      (sampleNameClean == "Zv22") || (sampleNameClean == "Wv22")) {
    int flav0, flav1 = -1;
    m_histNameSvc->get_eventFlavour(flav0, flav1);

    if (flav0 < 0 || flav1 < 0) {
      // commenting out for now as it appears many times leading to huge log
      // files or several GB in total Error("applyCorrections()","Failed to
      // retrieve event flavour! Exiting!");
      return EL::StatusCode::FAILURE;
    } else if (flav0 == 5 || flav1 == 5)
      flav = "b";
    else if (flav0 == 4 || flav1 == 4) {
      flav = "c";
      if (flav0 == flav1) flav = "cc";
    } else if (flav0 < 4 && flav1 < 4)
      flav = "l";
  }
  if ((sampleNameClean == "stopWt") && (m_analysisStrategy != "Merged")) {
    int flav0, flav1 = -1;
    m_histNameSvc->get_eventFlavour(flav0, flav1);

    if (flav0 < 0 || flav1 < 0) {
      // commenting out for now as it appears many times leading to huge log
      // files or several GB in total Error("applyCorrections()","Failed to
      // retrieve event flavour! Exiting!");
      return EL::StatusCode::FAILURE;
    } else if (flav0 == 5 && flav1 == 5)
      flav = "bb";
    else
      flav = "oth";
  }
  TString sampleName = sample.get_sampleName() + flav;
  TString detailsampleName = sample.get_detailsampleName();
  CAS::EventType evtType = m_corrsAndSysts->GetEventType(sampleName);

  if (m_csCorrections.size() != 0)  // apply corrections
  {
    for (decltype(m_csCorrections.size()) i = 0; i < m_csCorrections.size();
         i++) {
      if (m_csCorrections[i] == "VpTEFTCorrectionfit_kHdvR1Lambda1ca1") {
        m_weight =
            m_weight * m_corrsAndSysts->Get_VpTEFTCorrection_kHdvR1Lambda1ca1(
                           evtType, truthPt);
      } else if (m_csCorrections[i] ==
                 "VpTEFTCorrectionfit_kSM1HdvR1Lambda1ca1") {
        m_weight = m_weight *
                   m_corrsAndSysts->Get_VpTEFTCorrection_kSM1HdvR1Lambda1ca1(
                       evtType, truthPt);
      } else if (m_csCorrections[i] == "VHNLOEWK") {
        m_weight = m_weight *
                   m_corrsAndSysts->Get_HiggsNLOEWKCorrection(evtType, truthPt);
      } else if (m_csCorrections[i] == "NNLORW") {
        // No need to do anything, as applied elsewhere
      } else {
        Error("applyCcorrections()",
              "Cannot retrieve correction from CorrsAndSysts package %s. "
              "Exiting.",
              m_csCorrections[i].c_str());
        return EL::StatusCode::FAILURE;
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}  // applyCorrections

EL::StatusCode AnalysisReader_VHQQ::applyCS(float VpT, float mJ,
                                            int naddcalojet) {
  // CS function for boosted VHbb
  EL::StatusCode statusCorrection = applyCorrections();
  if (statusCorrection == EL::StatusCode::FAILURE) {
    return EL::StatusCode::FAILURE;
  }

  float cs_dr = -99;
  float cs_dphi = -99;
  float cs_mbb = mJ;
  float cs_ptb1 = -99;
  float cs_ptb2 = -99;
  float cs_truthPt = -99;
  float cs_avgTopPt = -99;
  int nJet = naddcalojet;
  int passCutBased = false;
  float Mtop = -99;
  int nTag = -99;
  float MET = -99;

  truthVariablesCS(cs_truthPt, cs_avgTopPt);

  EL::StatusCode statusSystematics = applySystematics(
      VpT, cs_mbb, cs_truthPt, cs_dphi, cs_dr, cs_ptb1, cs_ptb2, MET,
      cs_avgTopPt, nJet, nTag, passCutBased, Mtop);

  if (statusCorrection == EL::StatusCode::SUCCESS &&
      statusSystematics == EL::StatusCode::SUCCESS)
    return EL::StatusCode::SUCCESS;
  else
    return EL::StatusCode::FAILURE;
}

EL::StatusCode AnalysisReader_VHQQ::applyCS(float VpT, float MET, float nJet,
                                            float nTag,
                                            const TLorentzVector &jet1,
                                            const TLorentzVector &jet2,
                                            bool passCutBased, float Mtop) {
  EL::StatusCode statusCorrection = applyCorrections();
  if (statusCorrection == EL::StatusCode::FAILURE) {
    return EL::StatusCode::FAILURE;
  }

  // TODO: Check quantities (truth, reco, missing, ...) --> not well defined yet
  TLorentzVector dijet = jet1 + jet2;
  float cs_dr = fabs(jet1.DeltaR(jet2));
  float cs_dphi = fabs(jet1.DeltaPhi(jet2));
  float cs_mbb = dijet.M();
  float cs_ptb1 = jet1.Pt();
  float cs_ptb2 = jet2.Pt();
  float cs_truthPt = -1;
  float cs_avgTopPt = -1;
  truthVariablesCS(cs_truthPt, cs_avgTopPt);
  EL::StatusCode statusSystematics = applySystematics(
      VpT, cs_mbb, cs_truthPt, cs_dphi, cs_dr, cs_ptb1, cs_ptb2, MET,
      cs_avgTopPt, nJet, nTag, passCutBased, Mtop);

  if (statusCorrection == EL::StatusCode::SUCCESS &&
      statusSystematics == EL::StatusCode::SUCCESS)
    return EL::StatusCode::SUCCESS;
  else
    return EL::StatusCode::FAILURE;
}

EL::StatusCode AnalysisReader_VHQQ::applyCS(float VpT, float Mbb, float truthPt,
                                            float DeltaPhi, float DeltaR,
                                            float pTB1, float pTB2, float MET,
                                            float avgTopPt, int njet, int ntag,
                                            bool passCutBased, float Mtop) {
  EL::StatusCode statusCorrection = applyCorrections();
  if (statusCorrection == EL::StatusCode::FAILURE) {
    return EL::StatusCode::FAILURE;
  }

  EL::StatusCode statusSystematics =
      applySystematics(VpT, Mbb, truthPt, DeltaPhi, DeltaR, pTB1, pTB2, MET,
                       avgTopPt, njet, ntag, passCutBased, Mtop);

  if (statusCorrection == EL::StatusCode::SUCCESS &&
      statusSystematics == EL::StatusCode::SUCCESS)
    return EL::StatusCode::SUCCESS;
  else
    return EL::StatusCode::FAILURE;
}

EL::StatusCode AnalysisReader_VHQQ::applySystematics(
    float VpT, float MET, float nJet, float nTag, const TLorentzVector &jet1,
    const TLorentzVector &jet2, bool passCutBased, float Mtop) {
  // TODO: Check quantities (truth, reco, missing, ...) --> not well defined yet
  TLorentzVector dijet = jet1 + jet2;
  float cs_dr = fabs(jet1.DeltaR(jet2));
  float cs_dphi = fabs(jet1.DeltaPhi(jet2));
  float cs_mbb = dijet.M();
  float cs_ptb1 = jet1.Pt();
  float cs_ptb2 = jet2.Pt();
  float cs_truthPt = -1;
  float cs_avgTopPt = -1;
  truthVariablesCS(cs_truthPt, cs_avgTopPt);
  return applySystematics(VpT, cs_mbb, cs_truthPt, cs_dphi, cs_dr, cs_ptb1,
                          cs_ptb2, MET, cs_avgTopPt, nJet, nTag, passCutBased,
                          Mtop);
}

EL::StatusCode AnalysisReader_VHQQ::applySystematics(
    float VpT, float Mbb, float truthPt, float DeltaPhi, float DeltaR,
    float pTB1, float pTB2, float MET, float avgTopPt, int njet, int ntag,
    bool passCutBased, float Mtop) {
  m_weightSystsCS.clear();
  std::string detailSampleNameClean =
      m_xSectionProvider->getSampleDetailName(m_mcChannel);
  std::string sampleNameClean = m_histNameSvc->get_sample();
  sampleCS sample("temp");
  sample.setup_sample(sampleNameClean, detailSampleNameClean);
  std::string flav = "";

  if ((sampleNameClean == "Z") || (sampleNameClean == "W") ||
      (sampleNameClean == "Zv22") || (sampleNameClean == "Wv22")) {
    int flav0, flav1 = -1;
    m_histNameSvc->get_eventFlavour(flav0, flav1);

    if (flav0 < 0 || flav1 < 0) {
      Error("applySystematics()", "Failed to retrieve event flavour! Exiting!");
      return EL::StatusCode::FAILURE;
    } else if (flav0 == 5 && flav1 == 5) {
      flav = "bb";
    } else if ((flav0 == 5 && flav1 == 4) || (flav0 == 4 && flav1 == 5)) {
      flav = "bc";
    } else if ((flav0 == 5 && flav1 < 4) || (flav0 < 4 && flav1 == 5)) {
      flav = "bl";
    } else if (flav0 == 4 && flav1 == 4) {
      flav = "cc";
    } else if ((flav0 == 4 && flav1 < 4) || (flav0 < 4 && flav1 == 4)) {
      flav = "cl";
    } else if (flav0 < 4 && flav1 < 4)
      flav = "l";
  }

  if ((sampleNameClean == "stopWt") && (m_analysisStrategy != "Merged")) {
    int flav0, flav1 = -1;
    m_histNameSvc->get_eventFlavour(flav0, flav1);

    if (flav0 < 0 || flav1 < 0) {
      Error("applySystematics()", "Failed to retrieve event flavour! Exiting!");
      return EL::StatusCode::FAILURE;
    } else if (flav0 == 5 && flav1 == 5)
      flav = "bb";
    else
      flav = "oth";
  }
  TString sampleName = sample.get_sampleName() + flav;
  TString detailsampleName = sample.get_detailsampleName();
  CAS::EventType evtType = m_corrsAndSysts->GetEventType(sampleName);
  CAS::EventType evtType_BDTr = m_corrsAndSysts->GetEventType_BDTr(sampleName);
  CAS::DetailEventType detailevtType =
      m_corrsAndSysts->GetDetailEventType(detailsampleName);
  m_corrsAndSysts->SetCutBased(passCutBased);
  double weightUp = 1;
  double weightDo = 1;

  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);
  if (!nominalOnly && m_csVariations.size() != 0)  // apply variations
  {
    for (TString s_csVariations : m_csVariations) {
      CAS::Systematic sysUp;
      CAS::SysVar varUp = CAS::Up;
      CAS::SysVar varDo = CAS::Do;
      CAS::SysBin bin = CAS::Any;

      m_corrsAndSysts->GetSystFromName(s_csVariations, sysUp);

      CAS::Systematic sysDo = sysUp;

      if (s_csVariations ==
          "SysTopPt")  // pass average top+antitop pT as truthPt variable
      {
        weightUp = m_corrsAndSysts->Get_SystematicWeight(
            evtType, VpT, Mbb, avgTopPt, DeltaPhi, DeltaR, pTB1, pTB2, MET,
            njet, ntag, detailevtType, sysUp, varUp, bin);
        weightDo = m_corrsAndSysts->Get_SystematicWeight(
            evtType, VpT, Mbb, avgTopPt, DeltaPhi, DeltaR, pTB1, pTB2, MET,
            njet, ntag, detailevtType, sysDo, varDo, bin);
      } else if (s_csVariations.Contains(
                     "MTOP"))  // separate functionality for Mtop
      {
        weightUp =
            m_corrsAndSysts->Get_SystematicWeight(evtType, Mtop, sysUp, varUp);
        weightDo =
            m_corrsAndSysts->Get_SystematicWeight(evtType, Mtop, sysDo, varDo);
      } else  // use truthPt variable (which for SysTTbarPt is the pT of the
              // ttbar pair))
      {
        if (s_csVariations.Contains("BDTr")) {
          weightUp = m_corrsAndSysts->Get_SystematicWeight(
              evtType_BDTr, VpT, Mbb, truthPt, DeltaPhi, DeltaR, pTB1, pTB2,
              MET, njet, ntag, detailevtType, sysUp, varUp, bin);
          weightDo = m_corrsAndSysts->Get_SystematicWeight(
              evtType_BDTr, VpT, Mbb, truthPt, DeltaPhi, DeltaR, pTB1, pTB2,
              MET, njet, ntag, detailevtType, sysDo, varDo, bin);
        } else {
          weightUp = m_corrsAndSysts->Get_SystematicWeight(
              evtType, VpT, Mbb, truthPt, DeltaPhi, DeltaR, pTB1, pTB2, MET,
              njet, ntag, detailevtType, sysUp, varUp, bin);
          weightDo = m_corrsAndSysts->Get_SystematicWeight(
              evtType, VpT, Mbb, truthPt, DeltaPhi, DeltaR, pTB1, pTB2, MET,
              njet, ntag, detailevtType, sysDo, varDo, bin);
        }
      }

      if (s_csVariations.BeginsWith("Sys"))
        s_csVariations.Remove(
            0, 3);  // remove "Sys" from name to avoid duplicating it

      std::string samplename_tmp = m_histNameSvc->get_sample();

      // Fill the histograms only if at least one side has a non-0 variation
      if (fabs(weightUp - 1) > 1.e-6 || fabs(weightDo - 1) > 1.e-6 ||
          (s_csVariations.Contains("ACC") && sampleNameClean == "stopWt")) {
        m_weightSystsCS.push_back({(std::string)s_csVariations + "__1up",
                                   static_cast<float>(weightUp)});
        m_weightSystsCS.push_back({(std::string)s_csVariations + "__1down",
                                   static_cast<float>(weightDo)});
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}  // applySystematics

std::vector<std::pair<TString, Float_t>>
AnalysisReader_VHQQ::ComputeBDTSystematics(int channel, float btagWeight,
                                           int nJet, int nTag) {
  std::vector<std::pair<TString, Float_t>> BDT_scores;

  // Compute the few variables non analysis dependent

  m_BDTSyst->m_Variables["EventNumberModNfold"] =
      m_eventInfo->eventNumber() % 2;
  m_BDTSyst->m_Variables["EventNumber"] = m_eventInfo->eventNumber();
  m_BDTSyst->m_Variables["FoldType"] = 0.0;
  m_BDTSyst->m_Variables["nJ"] = nJet;
  m_BDTSyst->m_Variables["EventWeight"] = m_weight;
  m_BDTSyst->m_Variables["btagWeight"] = btagWeight;
  m_BDTSyst->m_Variables["nTag"] = nTag;
  m_BDTSyst->m_Variables["FlavourLabel"] = m_histNameSvc->getFlavorLabel();

  if (m_BDTSyst->m_Variables["nJ"] > 1 && m_BDTSyst->m_Variables["nJ"] < 4 &&
      m_BDTSyst->m_Variables["nTag"] == 2 &&
      (m_BDTSyst->m_Variables["pTV"] > 75 ||
       m_BDTSyst->m_Variables["MET"] > 150)) {
    for (auto m_BDTSystVariation : m_BDTSystVariations) {
      if (m_BDTSyst->m_Variables["FlavourLabel"] == 0 ||
          m_BDTSyst->m_Variables["FlavourLabel"] == 1 ||
          m_BDTSyst->m_Variables["FlavourLabel"] == 2 ||
          m_BDTSyst->m_Variables["FlavourLabel"] == 3) {
        float reweight;

        if ((m_histNameSvc->get_sample() == "ttbar" &&
             m_BDTSystVariation.find("W") != std::string::npos) ||
            (m_histNameSvc->get_sample() == "W" &&
             m_BDTSystVariation.find("ttbar") != std::string::npos)) {
          continue;
        }

        reweight = m_BDTSyst->DetermineWeight(
            m_BDTSystVariation, m_histNameSvc->get_sample(), channel,
            m_BDTSyst->m_Variables["BDT_" + m_BDTSystVariation]);
        if (m_debug)
          Info("AnalysisReader_VHQQ::ComputeBDTSystematics",
               "The BDT weight is %f", reweight);

        m_weightSysts.push_back(
            {"BDTr_" + m_BDTSystVariation + "__1up", reweight});
        BDT_scores.push_back(std::make_pair(
            "BDTr_" + m_BDTSystVariation,
            m_BDTSyst->m_Variables["BDT_" + m_BDTSystVariation]));
      } else {
        m_weightSysts.push_back({"_" + m_BDTSystVariation + "__1up", 1.});
      }
    }
  }

  return BDT_scores;
}  // ComputeBDTSystematics

EL::StatusCode AnalysisReader_VHQQ::fill_bTaggerHists(const xAOD::Jet *jet) {
  // fills histograms per jet and jet flavour for a list of b-taggers
  std::string flav = "Data";

  if (m_isMC) {
    int label = Props::HadronConeExclTruthLabelID.get(jet);

    flav = "L";

    if (label == 4)
      flav = "C";
    else if (label == 5)
      flav = "B";
  }

  float MV2c10 = Props::MV2c10.get(jet);
  double BVar = BTagProps::tagWeight.get(jet);

  if (m_isMC) {
    double BEff = BTagProps::eff.get(jet);
    m_histSvc->BookFillHist("eff_" + flav, 110, -0.1, 1.1, BEff, m_weight);
    m_histSvc->BookFillHist("pT_eff_" + flav, 400, 0, 400, 110, 0.0, 1.1,
                            jet->pt() / 1e3, BEff, m_weight);
    m_histSvc->BookFillHist("eta_eff_" + flav, 100, -5, 5, 110, 0.0, 1.1,
                            jet->eta(), BEff, m_weight);
  }
  m_histSvc->BookFillHist("btag_weight_" + flav, 110, -1.1, 1.1, BVar,
                          m_weight);
  m_histSvc->BookFillHist("MV2c10_" + flav, 110, -1.1, 1.1, MV2c10, m_weight);

  return EL::StatusCode::SUCCESS;
}  // fill_bTaggerHists

EL::StatusCode AnalysisReader_VHQQ::fill_FtagEff_Hists(const xAOD::Jet *jet,
                                                       float eventweight,
                                                       std::string hist_type) {
  std::string flav;

  int label = -1;
  Props::HadronConeExclTruthLabelID.get(jet, label);

  if (fabs(label) == 5) {
    flav = "B";
  } else if (fabs(label) == 4) {
    flav = "C";
  } else if (fabs(label) == 0) {
    flav = "Light";
  } else if (fabs(label) == 15) {
    flav = "T";
  }

  float pt = jet->pt() / 1e3;
  float abseta = fabs(jet->eta());

  m_histSvc->BookFillHist(hist_type + "_" + flav + "_total", 3000, 0, 3000, 60,
                          0, 3, pt, abseta, eventweight);

  if (BTagProps::isTagged.get(jet)) {
    m_histSvc->BookFillHist(hist_type + "_" + flav + "_pass", 3000, 0, 3000, 60,
                            0, 3, pt, abseta, eventweight);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ::fill_TLV(std::string name,
                                             const TLorentzVector &vec,
                                             double weight, bool isE,
                                             bool isMu) {
  //     if ((vec.Pt() > 14e6) && (vec.E() > 14e6)) return
  //     EL::StatusCode::SUCCESS;
  m_histSvc->BookFillHist(name + "Pt", 600, 0, 3000, vec.Pt() / 1e3, weight);
  m_histSvc->BookFillHist(name + "Eta", 100, -5., 5., vec.Eta(), weight);
  m_histSvc->BookFillHist(name + "Phi", 100, -TMath::Pi(), TMath::Pi(),
                          vec.Phi(), weight);
  m_histSvc->BookFillHist(name + "M", 600, 0, 3000, vec.M() / 1e3, weight);
  if (isE) {
    m_histSvc->BookFillHist("El_" + name + "Pt", 600, 0, 3000, vec.Pt() / 1e3,
                            weight);
    m_histSvc->BookFillHist("El_" + name + "Eta", 100, -5., 5., vec.Eta(),
                            weight);
    m_histSvc->BookFillHist("El_" + name + "Phi", 100, -TMath::Pi(),
                            TMath::Pi(), vec.Phi(), weight);
    m_histSvc->BookFillHist("El_" + name + "M", 600, 0, 3000, vec.M() / 1e3,
                            weight);
  }
  if (isMu) {
    m_histSvc->BookFillHist("Mu_" + name + "Pt", 600, 0, 3000, vec.Pt() / 1e3,
                            weight);
    m_histSvc->BookFillHist("Mu_" + name + "Eta", 100, -5., 5., vec.Eta(),
                            weight);
    m_histSvc->BookFillHist("Mu_" + name + "Phi", 100, -TMath::Pi(),
                            TMath::Pi(), vec.Phi(), weight);
    m_histSvc->BookFillHist("Mu_" + name + "M", 600, 0, 3000, vec.M() / 1e3,
                            weight);
  }

  //     m_histSvc->BookFillHist(name + "E",   600, 0, 3000, vec.E() / 1e3,
  //     weight);
  return EL::StatusCode::SUCCESS;
}

/*
EL::StatusCode AnalysisReader_VHQQ::fill_VBFInclCutFlow (std::string label)
{
  std::string dir = "CutFlow/Nominal/";

  static std::string cuts[18] = {
    "All", "J4Pt", "J5Pt", "2btag", "Ptbb"
  };

  m_histSvc->BookFillCutHist(dir + "Cuts", length(cuts), cuts, label, m_weight);
  m_histSvc->BookFillCutHist(dir + "CutsNoWeight", length(cuts), cuts,
label, 1.);

  return EL::StatusCode::SUCCESS;
} // fill_VBFInclCutFlow
*/

float AnalysisReader_VHQQ::compute_JVTSF(
    const std::vector<const xAOD::Jet *> & /*signalJets*/) {
  float SF = 1.0;

  // this seems not correct following the email exchange "Applying JVT scale
  // factors" on the hn-atlas-jet-etmiss-wg (JetEtMissWG) mailing list
  // (started 16.1.17) need to wait for correct recommendations, until then
  // don't use SFs for (auto jet : signalJets) {
  //
  //  SF *= Props::JvtSF.get(jet);
  //
  //}
  if (m_CxAODTag >= "CxAODTag28") SF *= Props::JvtSF.get(m_eventInfo);

  return SF;
}

void AnalysisReader_VHQQ::compute_btagging() {
  if (m_trackJets) {
    auto previous_ja = m_bTagTool->getJetAuthor();
    auto previous_ts = m_bTagTool->getTaggingScheme();

    m_bTagTool->setJetAuthor(m_trackJetReader->getContainerName());
    m_bTagTool->setTaggingScheme("FixedCut");

    for (auto jet : *m_trackJets) {
      BTagProps::isTagged.set(
          jet, static_cast<decltype(BTagProps::isTagged.get(jet))>(
                   m_bTagTool->isTagged(*jet)));
      BTagProps::tagWeight.set(jet, m_bTagTool->getTaggerWeight(*jet));
      BTagProps::Quantile.set(jet, m_bTagTool->getQuantile(*jet));
    }

    m_bTagTool->setJetAuthor(previous_ja);
    m_bTagTool->setTaggingScheme(previous_ts);
  }

  if (m_jets) {
    auto previous_ja = m_bTagTool->getJetAuthor();
    if (!m_use2DbTagCut)
      m_bTagTool->setJetAuthor(m_jetReader->getContainerName());

    for (auto jet : *m_jets) {
      if (!m_use2DbTagCut)
        BTagProps::isTagged.set(
            jet, static_cast<decltype(BTagProps::isTagged.get(jet))>(
                     m_bTagTool->isTagged(*jet)));
      else
        BTagProps::isTagged.set(jet, false);

      BTagProps::tagWeight.set(jet, m_bTagTool->getTaggerWeight(*jet));
      BTagProps::Quantile.set(jet, m_bTagTool->getQuantile(*jet));

      if (m_isMC && !m_use2DbTagCut)
        BTagProps::eff.set(jet, m_bTagTool->getEfficiency(*jet));
      else
        BTagProps::eff.set(jet, -999.);
    }
    m_bTagTool->setJetAuthor(previous_ja);
  }

  if (m_subJets) {
    auto previous_ja = m_bTagTool->getJetAuthor();
    m_bTagTool->setJetAuthor(
        m_trackJetReader
            ->getContainerName());  // Use trackjet as default for the moment
    for (auto jet : *m_subJets) {
      BTagProps::isTagged.set(
          jet, static_cast<decltype(BTagProps::isTagged.get(jet))>(
                   m_bTagTool->isTagged(*jet)));
      BTagProps::tagWeight.set(jet, Props::MV2c10.get(jet));
      BTagProps::Quantile.set(jet, m_bTagTool->getQuantile(*jet));
    }
    m_bTagTool->setJetAuthor(previous_ja);
  }

  if (m_fatJets) {
    for (auto fatjet : *m_fatJets) {
      Props::nTrackJets.set(fatjet, 0);
      Props::nBTags.set(fatjet, 0);
      if (m_isMC)
        Props::nTrueBJets.set(fatjet, 0);
      else
        Props::nTrueBJets.set(fatjet, -1);
    }
  }

}  // compute_btagging

void AnalysisReader_VHQQ::compute_TRF_tagging(
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &signalJetsDT,
    bool useTrackJets) {  // TT for boosted
                          // need to pass also jetsDT, and bool to
                          // check if useTrackJets

  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);
  bool isNominal = {m_currentVar == "Nominal"};

  if (!isNominal)
    m_bTagTool->setWeightVar(false);
  else if (isNominal && !nominalOnly)
    m_bTagTool->setWeightVar(true);
  else
    m_bTagTool->setWeightVar(false);

  if (m_analysisStrategy == "Resolved") {  // this part not done for boosted
    bool ttag_track_jets = false;
    if (m_trackJets) {
      m_bTagTool->setJetAuthor(m_trackJetReader->getContainerName());
      if (ttag_track_jets)
        m_bTagTool->truth_tag_jets(m_eventInfo->eventNumber(), *m_trackJets,
                                   m_config);
      else {
        for (auto jet : *m_trackJets) {
          BTagProps::isTagged.set(
              jet, static_cast<decltype(BTagProps::isTagged.get(jet))>(
                       m_bTagTool->isTagged(*jet)));
          BTagProps::tagWeight.set(jet, m_bTagTool->getTaggerWeight(*jet));
        }
      }
    }
  }

  // adapted for trackjets
  if ((m_jets && !useTrackJets) ||
      (m_trackJets && useTrackJets)) {  // we assume signal jets are taken from
                                        // the larger jet container

    if (useTrackJets)
      m_bTagTool->setJetAuthor(m_trackJetReader->getContainerName());
    else
      m_bTagTool->setJetAuthor(m_jetReader->getContainerName());
    m_bTagTool->truth_tag_jets(
        m_eventInfo->eventNumber(), signalJets, m_config,
        signalJetsDT);  // TT for boosted: need to pass also jetsDT
  }

  if (m_fatJets) {
    for (auto fatjet : *m_fatJets) {
      Props::nTrackJets.set(fatjet, 0);
      Props::nBTags.set(fatjet, 0);
      if (m_isMC)
        Props::nTrueBJets.set(fatjet, 0);
      else
        Props::nTrueBJets.set(fatjet, -1);
    }
  }

}  // compute_TRF_tagging

void AnalysisReader_VHQQ::compute_fatjetTags(
    const std::vector<const xAOD::Jet *> &trackJets,
    const std::vector<const xAOD::Jet *> &fatJets,
    std::vector<const xAOD::Jet *> *trackJetsInLeadFJ = nullptr,
    std::vector<const xAOD::Jet *> *trackJetsNotInLeadFJ = nullptr) {
  // reset the track jet vector
  m_allLeadFJMatchedTrackJets.clear();

  for (unsigned int i = 0; i < fatJets.size(); ++i) {
    const xAOD::Jet *fatjet = fatJets.at(i);
    using FatJetType = typename std::remove_pointer<decltype(fatjet)>::type;
    std::string trackLinkName = "GhostAntiKt2TrackJet";
    m_config->getif<std::string>("trackLinkName", trackLinkName);
    static FatJetType::ConstAccessor<
        vector<ElementLink<DataVector<xAOD::IParticle>>>>
        GhostAccessor(trackLinkName.c_str());

    if (Props::isFatJet.get(fatjet)) {
      // determine number of b-tags in a fat jet
      //  number of tracks will be the track jets with closest DR with the ghost
      //  matched track jets
      std::vector<const xAOD::Jet *> trackJetsInFatJet;
      std::vector<const xAOD::Jet *> trackJetsNotInFatJet;

      // find the trackJets that are part of the fat jet
      // and those who are not
      for (auto trackJet : trackJets) {
        bool forceTrackJetDRmatching = false;
        m_config->getif<bool>("forceTrackJetDRmatching",
                              forceTrackJetDRmatching);

        if (forceTrackJetDRmatching) {
          if (trackJet->p4().DeltaR(fatjet->p4()) < 1.0) {
            // dR-matched
            trackJetsInFatJet.push_back(trackJet);
            if (i == 0) {
              // std::cout << "Push back track jet pointer: " << trackJet <<
              // std::endl; std::cout << "                   with pt = " <<
              // trackJet->pt() << std::endl;
              m_allLeadFJMatchedTrackJets.push_back(trackJet);
            }
            if (i == 0 and trackJetsInLeadFJ != nullptr) {
              if (m_boostedTagStrategy == "Leading2SignalJets") {
                if (trackJetsInLeadFJ->size() < 2) {
                  trackJetsInLeadFJ->push_back(trackJet);
                }
              } else if (m_boostedTagStrategy == "AllSignalJets") {
                trackJetsInLeadFJ->push_back(trackJet);
              } else {
                Error("compute_fatjetTags()",
                      "The chosen tag strategy does not exist in the boosted "
                      "analysis");
                exit(EXIT_FAILURE);
              }
            }
          } else {
            trackJetsNotInFatJet.push_back(trackJet);
            if (i == 0 and trackJetsNotInLeadFJ != nullptr) {
              trackJetsNotInLeadFJ->push_back(trackJet);
            }
          }
        } else {
          bool ghostMatched = false;
          for (auto gTrackParticle : GhostAccessor(*fatjet)) {
            if (not gTrackParticle.isValid()) {
              continue;
            }
            if (*gTrackParticle == trackJet) {
              // both pointers point to the same jet -> it's a match!
              trackJetsInFatJet.push_back(trackJet);
              if (i == 0 and trackJetsInLeadFJ != nullptr) {
                if (m_boostedTagStrategy == "Leading2SignalJets") {
                  if (trackJetsInLeadFJ->size() < 2) {
                    trackJetsInLeadFJ->push_back(trackJet);
                  }
                } else if (m_boostedTagStrategy == "AllSignalJets") {
                  trackJetsInLeadFJ->push_back(trackJet);
                } else {
                  Error("compute_fatjetTags()",
                        "The chosen tag strategy does not exist in the boosted "
                        "analysis");
                  exit(EXIT_FAILURE);
                }
              }
              ghostMatched = true;
              break;
            }
          }
          // we looped over all ghost associated particles
          // check if there was a match
          if (not ghostMatched) {
            trackJetsNotInFatJet.push_back(trackJet);
            if (i == 0 and trackJetsNotInLeadFJ != nullptr) {
              trackJetsNotInLeadFJ->push_back(trackJet);
            }
          }
        }
      }  // end loop over trackJets
      Props::nTrackJets.set(fatjet, trackJetsInFatJet.size());

      // count number of b-tagged track jets inside the fat jet
      auto nTags = 0;
      for (auto trackJet : trackJetsInFatJet) {
        if (BTagProps::isTagged.get(trackJet)) ++nTags;
      }
      Props::nBTagsAll.set(fatjet, nTags);

      // define number of b-tag of the fat jet based on the number of b-tag
      // track jets (depending on the strategy)
      nTags = 0;
      if (m_boostedTagStrategy == "AllSignalJets") {
        for (auto trackJet : trackJetsInFatJet) {
          if (BTagProps::isTagged.get(trackJet)) ++nTags;
        }
      } else if (m_boostedTagStrategy == "Leading2SignalJets") {
        if (trackJetsInFatJet.size() >= 1) {
          if (BTagProps::isTagged.get(trackJetsInFatJet.at(0))) {
            nTags++;
          }
          if (trackJetsInFatJet.size() >= 2 &&
              BTagProps::isTagged.get(trackJetsInFatJet.at(1))) {
            nTags++;
          }
        }
      }
      Props::nBTags.set(fatjet, nTags);

      // Count the number of B-Tagged jets outside the fat jet (TopCR)
      int nAddTags = 0;
      for (const xAOD::Jet *trackjet : trackJetsNotInFatJet) {
        if (BTagProps::isTagged.get(trackjet)) {
          nAddTags++;
        }
      }
      Props::nAddBTags.set(fatjet, nAddTags);

      // finished b-tagging criteria
      // determine number of true b-jets in a fat jet with simple DR
      auto nBJets = 0;
      if (m_isMC) {
        for (auto trackJet : trackJetsInFatJet) {
          auto label = Props::HadronConeExclTruthLabelID.get(trackJet);
          if (label == 5) ++nBJets;
        }
      } else
        nBJets = -1;  // Data
      Props::nTrueBJets.set(fatjet, nBJets);
    }  // finished true b-jet association criteria
  }    // end loop on fat jets
}

void AnalysisReader_VHQQ::matchTrackJetstoFatJets(
    const std::vector<const xAOD::Jet *> &trackJets, const xAOD::Jet *fatjet,
    std::vector<const xAOD::Jet *> *trackJetsInFatJet = nullptr,
    std::vector<const xAOD::Jet *> *trackJetsNotInFatJet = nullptr) {
  // This function checks the track jets which are matched or not to the fat
  // jet.
  using FatJetType = typename std::remove_pointer<decltype(fatjet)>::type;
  std::string trackLinkName = "GhostAntiKt2TrackJet";
  m_config->getif<std::string>("trackLinkName", trackLinkName);
  static FatJetType::ConstAccessor<
      vector<ElementLink<DataVector<xAOD::IParticle>>>>
      GhostAccessor(trackLinkName.c_str());

  if (Props::isFatJet.get(fatjet)) {
    // find the trackJets that are part of the fat jet
    // and those who are not
    for (auto trackJet : trackJets) {
      bool forceTrackJetDRmatching = false;
      m_config->getif<bool>("forceTrackJetDRmatching", forceTrackJetDRmatching);
      if (forceTrackJetDRmatching) {
        if (trackJet->p4().DeltaR(fatjet->p4()) < 1.0) {
          // dR-matched
          if (trackJetsInFatJet != nullptr) {
            trackJetsInFatJet->push_back(trackJet);
          }
        } else {
          if (trackJetsNotInFatJet != nullptr) {
            trackJetsNotInFatJet->push_back(trackJet);
          }
        }
      } else {
        bool ghostMatched = false;
        for (auto gTrackParticle : GhostAccessor(*fatjet)) {
          if (not gTrackParticle.isValid()) {
            continue;
          }
          if (*gTrackParticle == trackJet) {
            // both pointers point to the same jet -> it's a match!
            if (trackJetsInFatJet != nullptr) {
              trackJetsInFatJet->push_back(trackJet);
            }
            ghostMatched = true;
            break;
          }
        }
        // we looped over all ghost associated particles
        // check if there was a match
        if (not ghostMatched) {
          if (trackJetsNotInFatJet != nullptr) {
            trackJetsNotInFatJet->push_back(trackJet);
          }
        }
      }
    }  // end loop over trackJets
  }
}

void AnalysisReader_VHQQ::tagjet_selection(
    std::vector<const xAOD::Jet *> &signalJets,
    std::vector<const xAOD::Jet *> &forwardJets,
    std::vector<const xAOD::Jet *> &selectedJets, int &tagcatExcl) {
  string tagAlgorithm;

  m_config->getif<std::string>("tagAlgorithm",
                               tagAlgorithm);  // FlavLabel,FlavTag

  selectedJets.clear();
  tagcatExcl = -1;

  /////////////////////////////////////////////////////////////
  // **B-Tagging Selection**
  bool Lead1BTag = false;
  bool Lead2BTag = false;
  int Ind1BTag = -1;
  int Ind2BTag = -1;
  int nbtag = 0;
  int jetidx = 0;
  int nSignalJet = signalJets.size();
  int nForwardJet = forwardJets.size();
  const xAOD::Jet *Jet1 = nullptr;
  const xAOD::Jet *Jet2 = nullptr;
  const xAOD::Jet *Jet3 = nullptr;

  // check if the leading signal jets are b-tagged
  //----------------------------------------------
  if (m_isMC && (tagAlgorithm ==
                 "FlavLabel")) {  // use truth label to select b-jets (MC only!)

    if (signalJets.size() > 0 &&
        Props::HadronConeExclTruthLabelID.get(signalJets.at(0)) == 5) {
      Lead1BTag = true;
    }
    if (signalJets.size() > 1 &&
        Props::HadronConeExclTruthLabelID.get(signalJets.at(1)) == 5) {
      Lead2BTag = true;
    }
  } else {  // if (tagAlgorithm == "FlavTag") use b-tagging to select b-jets
            // (the only option for data)

    if (signalJets.size() > 0) {
      if (BTagProps::isTagged.get(signalJets.at(0)) == 1) Lead1BTag = true;
    }
    if (signalJets.size() > 1) {
      if (BTagProps::isTagged.get(signalJets.at(1)) == 1) Lead2BTag = true;
    }
  }

  // count number of b-tagged jets
  // find indices of leading ones
  //-----------------------------
  for (const xAOD::Jet *jet : signalJets) {
    bool isTagged = BTagProps::isTagged.get(jet);

    // if using truth label (MC only!)
    if (m_isMC && (tagAlgorithm == "FlavLabel")) {
      isTagged = (Props::HadronConeExclTruthLabelID.get(jet) == 5);
    }
    if (isTagged) {
      nbtag++;
      if (Ind1BTag < 0) {
        Ind1BTag = jetidx;
      } else if (Ind2BTag < 0) {
        Ind2BTag = jetidx;
      }
    }
    jetidx++;
  }

  /////////////////////////////////////////////////////////////
  // **Tag-Category Definition**
  if ((m_tagStrategy == "AllSignalJets") ||
      (m_tagStrategy == "LeadingSignalJets")) {
    tagcatExcl = nbtag;

    // 0tag
    //----
    // decreasing pT ordering
    if (nbtag == 0) {
      if (nSignalJet >= 1) Jet1 = signalJets.at(0);
      if (nSignalJet >= 2) {
        Jet2 = signalJets.at(1);
        if (nSignalJet >= 3)
          Jet3 = signalJets.at(2);
        else if (nForwardJet >= 1)
          Jet3 = forwardJets.at(0);
      }
    }

    // 1 tag
    //-----
    // all sig jets: first leading jet, even if not b-tagged
    if (nbtag == 1 && m_tagStrategy == "AllSignalJets") {
      // first is always the leading, whether it's b-tagged or not
      if (nSignalJet >= 1) Jet1 = signalJets.at(0);

      // second is either the sub-leading (if the 1st is tagged) or the b-tagged
      if (nSignalJet >= 2) {
        if (Lead1BTag || Lead2BTag)
          Jet2 = signalJets.at(1);
        else
          Jet2 = signalJets.at(Ind1BTag);

        // third is the third sub-leading, unless the third is b-tagged (in that
        // case it is in position 2 in the vector)
        if (nSignalJet >= 3) {
          if (Ind1BTag >= 2)
            Jet3 = signalJets.at(1);
          else
            Jet3 = signalJets.at(2);

        } else if (nForwardJet >= 1) {
          Jet3 = forwardJets.at(0);
        }
      }
    } else if (nbtag == 1 && m_tagStrategy == "LeadingSignalJets") {
      Jet1 = signalJets.at(Ind1BTag);  // * leading b-tagged signal jet
      if ((m_tagStrategy == "LeadingSignalJets") && !Lead1BTag)
        tagcatExcl = -1;  // * LeadingSignalJets: leading b-tagged jet must be
                          // leading jet

      if (nSignalJet >= 2) {
        if (Ind1BTag == 0)
          Jet2 = signalJets.at(1);
        else
          Jet2 = signalJets.at(0);

        if (nSignalJet >= 3) {
          if (Ind1BTag < 2)
            Jet3 = signalJets.at(2);
          else
            Jet3 = signalJets.at(1);
        } else if (nForwardJet >= 1)
          Jet3 = forwardJets.at(0);
      }
    }

    // 2 tag
    //-----
    int jetidx3 = 0;
    if (nbtag >= 2) {
      Jet1 = signalJets.at(Ind1BTag);
      Jet2 = signalJets.at(Ind2BTag);
      if ((m_tagStrategy == "LeadingSignalJets") && !Lead1BTag)
        tagcatExcl = -1;  // * LeadingSignalJets: leading b-tagged jet must be
                          // leading jet

      if (nSignalJet >= 3) {
        for (const xAOD::Jet *jet : signalJets) {
          if ((jetidx3 != Ind1BTag) && (jetidx3 != Ind2BTag)) {
            Jet3 = jet;
            break;
          }
          jetidx3++;
        }
      } else if (nForwardJet >= 1) {
        Jet3 = forwardJets.at(0);
        // jetidx is negative for forward jets
        jetidx3 = -1;
      }
    }

  }  // end tag strategy "AllSignalJets" or "LeadingSignalJets"

  /////////////////////////////////////////////////////////////
  if (m_tagStrategy == "Leading2SignalJets") {
    if (!Lead1BTag && !Lead2BTag) tagcatExcl = 0;
    if ((Lead1BTag && !Lead2BTag) || (!Lead1BTag && Lead2BTag)) tagcatExcl = 1;
    if (Lead1BTag && Lead2BTag && nbtag >= 2) tagcatExcl = 2;

    if (nSignalJet >= 1) Jet1 = signalJets.at(0);
    if (nSignalJet >= 2) Jet2 = signalJets.at(1);
    if (nSignalJet >= 3)
      Jet3 = signalJets.at(2);
    else if (nForwardJet >= 1)
      Jet3 = forwardJets.at(0);
  }
  // fill selected jets
  //------------------
  if (Jet1) selectedJets.push_back(Jet1);
  if (Jet2) selectedJets.push_back(Jet2);
  if (Jet3) selectedJets.push_back(Jet3);

  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
}  // tagjet_selection

void AnalysisReader_VHQQ::tagTrackjet_selection(
    const xAOD::Jet *fatJet, std::vector<const xAOD::Jet *> trackJetsInFatJet,
    std::vector<const xAOD::Jet *> trackJetsNotInFatJet,
    std::vector<const xAOD::Jet *> &selectedtrackJetsInFatJet,
    std::vector<const xAOD::Jet *> &bTaggedUnmatchedTrackJets,
    std::vector<const xAOD::Jet *> &trackJetsForBTagSF, int &tagcatExcl,
    int &nAddBTags) {
  // tagTrackjet_selection takes as input a fat and all the track jets and
  // return the track jets be used for b-tagging SF and the
  // selectedtrackJetsInFatJet. The exact composition of these subsets of track
  // jets depends on the chosen b-tag strategy. Finally the function evaluates
  // the number of b-tagged track jets inside the fat jet depending on the tag
  // strategy and the number of b-tagged track jets outside the fat jet.

  string tagAlgorithm;

  m_config->getif<std::string>("tagAlgorithm",
                               tagAlgorithm);  // FlavLabel,FlavTag

  selectedtrackJetsInFatJet.clear();
  tagcatExcl = -1;
  nAddBTags = 0;

  /////////////////////////////////////////////////////////////
  // **B-Tagging Selection**
  bool Lead1BTag = false;
  bool Lead2BTag = false;
  int Ind1BTag = -1;
  int Ind2BTag = -1;
  int nbtag = 0;
  int jetidx = 0;
  auto nTags = 0;
  int ntrackJetsInFatJet = trackJetsInFatJet.size();
  const xAOD::Jet *TrkJet1 = nullptr;
  const xAOD::Jet *TrkJet2 = nullptr;
  const xAOD::Jet *TrkJet3 = nullptr;
  std::vector<const xAOD::Jet *> btagTrackJetsInFatJet;

  // check if the leading track jets are b-tagged
  //----------------------------------------------
  if (m_isMC && (tagAlgorithm ==
                 "FlavLabel")) {  // use truth label to select b-jets (MC only!)
    if (trackJetsInFatJet.size() > 0 &&
        Props::HadronConeExclTruthLabelID.get(trackJetsInFatJet.at(0)) == 5)
      Lead1BTag = true;
    if (trackJetsInFatJet.size() > 1 &&
        Props::HadronConeExclTruthLabelID.get(trackJetsInFatJet.at(1)) == 5)
      Lead2BTag = true;
  } else {  // if (tagAlgorithm == "FlavTag") use b-tagging to select b-jets
            // (the only option for data)
    if (trackJetsInFatJet.size() > 0) {
      if (BTagProps::isTagged.get(trackJetsInFatJet.at(0)) == 1)
        Lead1BTag = true;
    }
    if (trackJetsInFatJet.size() > 1) {
      if (BTagProps::isTagged.get(trackJetsInFatJet.at(1)) == 1)
        Lead2BTag = true;
    }
  }

  // count number of b-tagged jets
  // find indices of leading ones
  //-----------------------------
  for (const xAOD::Jet *jet : trackJetsInFatJet) {
    bool isTagged = BTagProps::isTagged.get(jet);

    // if using truth label (MC only!)
    if (m_isMC && (tagAlgorithm == "FlavLabel"))
      isTagged = (Props::HadronConeExclTruthLabelID.get(jet) == 5);

    if (isTagged) {
      nbtag++;

      if (Ind1BTag < 0)
        Ind1BTag = jetidx;
      else if (Ind2BTag < 0)
        Ind2BTag = jetidx;
    }
    jetidx++;
  }

  /////////////////////////////////////////////////////////////
  // **Tag-Category Definition**
  if (m_boostedTagStrategy == "AllSignalJets") {
    tagcatExcl = nbtag;

    // 0tag
    //----
    // decreasing pT ordering
    if (nbtag == 0) {
      if (ntrackJetsInFatJet >= 1) TrkJet1 = trackJetsInFatJet.at(0);
      if (ntrackJetsInFatJet >= 2) TrkJet2 = trackJetsInFatJet.at(1);
      if (ntrackJetsInFatJet >= 3) TrkJet3 = trackJetsInFatJet.at(2);
    }

    // 1 tag
    //-----
    // all sig jets: first leading jet, even if not b-tagged
    if (nbtag == 1) {
      // first is always the leading, whether it's b-tagged or not
      if (ntrackJetsInFatJet >= 1) TrkJet1 = trackJetsInFatJet.at(0);

      // second is either the sub-leading (if the 1st is tagged) or the b-tagged
      if (ntrackJetsInFatJet >= 2) {
        if (Lead1BTag || Lead2BTag)
          TrkJet2 = trackJetsInFatJet.at(1);
        else
          TrkJet2 = trackJetsInFatJet.at(Ind1BTag);

        // third is the third sub-leading, unless the third is b-tagged (in that
        // case it is in position 2 in the vector)
        if (ntrackJetsInFatJet >= 3) {
          if (Ind1BTag >= 2)
            TrkJet3 = trackJetsInFatJet.at(1);
          else
            TrkJet3 = trackJetsInFatJet.at(2);
        }
      }
    }

    // 2 tag
    //-----
    if (nbtag >= 2) {
      TrkJet1 = trackJetsInFatJet.at(Ind1BTag);
      TrkJet2 = trackJetsInFatJet.at(Ind2BTag);

      int jetidx3 = 0;
      if (ntrackJetsInFatJet >= 3) {
        for (const xAOD::Jet *jet : trackJetsInFatJet) {
          if ((jetidx3 != Ind1BTag) && (jetidx3 != Ind2BTag)) {
            TrkJet3 = jet;
            break;
          }
          jetidx3++;
        }
      }
    }
  }

  /////////////////////////////////////////////////////////////
  if (m_boostedTagStrategy == "Leading2SignalJets") {
    if (!Lead1BTag && !Lead2BTag) tagcatExcl = 0;
    if ((Lead1BTag && !Lead2BTag) || (!Lead1BTag && Lead2BTag)) tagcatExcl = 1;
    if (Lead1BTag && Lead2BTag && nbtag >= 2) tagcatExcl = 2;

    if (ntrackJetsInFatJet >= 1) TrkJet1 = trackJetsInFatJet.at(0);
    if (ntrackJetsInFatJet >= 2) TrkJet2 = trackJetsInFatJet.at(1);
    if (ntrackJetsInFatJet >= 3) TrkJet3 = trackJetsInFatJet.at(2);
  }
  // fill selectedtrackJetsInFatJet -> it depends on the tag strategy:
  //                                   - in case of Leading2SignalJets strategy
  //                                   it is filled with the leading track jets
  //                                   - in case of AllSignalJets strategy, 0
  //                                   tag case, it is filled with the 2 leading
  //                                   track jets
  //                                   - in case of AllSignalJets strategy, 1
  //                                   tag case, it is filled with the b-tagged
  //                                   jet and the leading non-tagged track jet
  //                                   - in case of AllSignalJets strategy, 2
  //                                   tag case, it is filled with the leading 2
  //                                   b-tagged track jets
  //------------------
  if (TrkJet1) selectedtrackJetsInFatJet.push_back(TrkJet1);
  if (TrkJet2) selectedtrackJetsInFatJet.push_back(TrkJet2);
  if (TrkJet3) selectedtrackJetsInFatJet.push_back(TrkJet3);

  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////

  // fill trackJetsForBTagSF used to compute the b-tag weight -> it depends on
  // the tag strategy:
  //  - in case of Leading2SignalJets strategy it contains the two leading track
  //  jets + all the track jets outside the LFJ
  //  - in case of AllSignalJets strategy it contains all the track jets matched
  //  to the LFJ + all the track jets outside the LFJ
  //  If doCoMtagging in config file is selected, then only use tracks outside
  //  of LFJ

  bool doCoMtagging = false;
  m_config->getif<bool>("doCoMtagging", doCoMtagging);

  if (m_boostedTagStrategy == "AllSignalJets" && !doCoMtagging) {
    trackJetsForBTagSF = trackJetsInFatJet;
    trackJetsForBTagSF.insert(trackJetsForBTagSF.end(),
                              trackJetsNotInFatJet.begin(),
                              trackJetsNotInFatJet.end());
  } else if (m_boostedTagStrategy == "Leading2SignalJets" && !doCoMtagging) {
    if (TrkJet1) btagTrackJetsInFatJet.push_back(TrkJet1);
    if (TrkJet2) btagTrackJetsInFatJet.push_back(TrkJet2);
    trackJetsForBTagSF = btagTrackJetsInFatJet;
    trackJetsForBTagSF.insert(trackJetsForBTagSF.end(),
                              trackJetsNotInFatJet.begin(),
                              trackJetsNotInFatJet.end());
  } else {
    trackJetsForBTagSF = trackJetsNotInFatJet;
  }

  for (auto jet : trackJetsNotInFatJet) {
    if (BTagProps::isTagged.get(jet)) bTaggedUnmatchedTrackJets.push_back(jet);
  }

  Props::nBTags.set(
      fatJet, tagcatExcl);  // count number of b-tagged track jets
                            // inside the fat jet depending on the tag strategy

  for (const xAOD::Jet *trackjet : trackJetsNotInFatJet) {
    if (BTagProps::isTagged.get(trackjet)) nAddBTags++;
  }
  for (const xAOD::Jet *trackjet : trackJetsInFatJet) {
    if (BTagProps::isTagged.get(trackjet)) ++nTags;
  }
  Props::nAddBTags.set(fatJet, nAddBTags);  // count number of b-tagged track
                                            // jet outside the fat jet

  Props::nBTagsAll.set(
      fatJet, nTags);  // count number of b-tagged track jets
                       // inside the fat jet regardless the tag strategy
  Props::nTrackJets.set(
      fatJet,
      ntrackJetsInFatJet);  // count number of track jets inside the fat jet

  // determine number of true b-jets in a fat jet
  int nBJets = 0;
  if (m_isMC) {
    for (const xAOD::Jet *trackjet : trackJetsInFatJet) {
      if (Props::HadronConeExclTruthLabelID.get(trackjet) == 5) ++nBJets;
    }
  } else {
    nBJets = -1;  // Data
  }
  Props::nTrueBJets.set(fatJet, nBJets);
}  // tagTrackjet_selection

// verifies if there are overlapping VR track jets in jets checked for b-tagging
// (recommendation: veto the event) see
// https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/BTagCalib2017#Recommendations_for_variable_rad)
bool AnalysisReader_VHQQ::passVRJetOR(
    std::vector<const xAOD::Jet *> trackJetsForBTagSF) {
  if (trackJetsForBTagSF.size() == 0) return true;

  bool isVR = Props::VRJetOverlap.exists(trackJetsForBTagSF.at(0));
  if (isVR) {
    for (const xAOD::Jet *trackjet : trackJetsForBTagSF) {
      if (Props::VRJetOverlap.get(trackjet)) return false;
    }
    return true;
  } else {
    return true;
  }
}  // passVRJetOR

void AnalysisReader_VHQQ::set_BtagProperties_fatJets(
    const xAOD::Jet *fatJet, std::vector<const xAOD::Jet *> trackJetsInFatJet,
    std::vector<const xAOD::Jet *> trackJetsNotInFatJet) {
  // This function counts the number of b-tagged track jets outside the fat jet
  // as well as the number of b-tagged track jets inside the fat jet, regardless
  // the tag strategy

  auto nTags = 0;
  auto nAddbtag = 0;
  for (const xAOD::Jet *trackjet : trackJetsNotInFatJet) {
    if (BTagProps::isTagged.get(trackjet)) nAddbtag++;
  }
  for (const xAOD::Jet *trackjet : trackJetsInFatJet) {
    if (BTagProps::isTagged.get(trackjet)) ++nTags;
  }
  Props::nAddBTags.set(fatJet, nAddbtag);  // count number of b-tagged track
                                           // jets outside the fat jet
  Props::nBTagsAll.set(
      fatJet, nTags);  // count number of b-tagged track jets
                       // inside the fat jet regardless the tag strategy

  // count number of track jets inside the fat jet
  Props::nTrackJets.set(fatJet, trackJetsInFatJet.size());

  // determine number of true b-jets in a fat jet
  int nBJets = 0;
  if (m_isMC) {
    for (const xAOD::Jet *trackjet : trackJetsInFatJet) {
      if (Props::HadronConeExclTruthLabelID.get(trackjet) == 5) ++nBJets;
    }
  } else {
    nBJets = -1;  // Data
  }
  Props::nTrueBJets.set(fatJet, nBJets);
}  // set_BtagProperties_fatJets

void AnalysisReader_VHQQ::CoMtagging_LeadFJ(
    const std::vector<const xAOD::Jet *> subJets, const xAOD::Jet *fatJets,
    std::vector<const xAOD::Jet *> &selectedCoMSubJets, int &nBtags) {
  // Function that uses the CoM method to b-tag the leading fat jet (LFJ(
  //
  const xAOD::Jet *fatjet = fatJets;
  using FatJetType = typename std::remove_pointer<decltype(fatjet)>::type;
  std::string subjetLinkName = "ExCoM2SubJets";
  static FatJetType::ConstAccessor<
      vector<ElementLink<DataVector<xAOD::IParticle>>>>
      LinkAccessorCoM(subjetLinkName.c_str());

  nBtags = 0;
  selectedCoMSubJets.clear();
  string tagAlgorithm;
  m_config->getif<std::string>("tagAlgorithm", tagAlgorithm);

  if (Props::isFatJet.get(fatjet)) {
    for (auto subJet : subJets) {
      // matching subjets with fatjets through CoM matching
      for (auto subjetCoMParticle : LinkAccessorCoM(*fatjet)) {
        if (not subjetCoMParticle
                    .isValid()) {  // Pass if CoM association is not valid
          continue;
        }
        if (*subjetCoMParticle ==
            subJet) {  // CoM associating between CoM subjet with LFJ
          selectedCoMSubJets.push_back(subJet);
          if (m_isMC && (tagAlgorithm == "FlavLabel") &&
              (Props::HadronConeExclTruthLabelID.get(subJet) == 5))
            nBtags++;  // Truth b-tagging, if FlavLabel is selected
          else {
            // if (BTagProps::isTagged.get(subJet) == 1) //trick until we have a
            // CDI with CoM information
            if (Props::MV2c10.get(subJet) >=
                0.860019)  // Works only for 70% and fixed cut
              nBtags++;    // b-tagging of CoM subjet
          }
        }
      }
    }
  }
}

EL::StatusCode AnalysisReader_VHQQ::fill_nJetHistos(
    const std::vector<const xAOD::Jet *> &jets, const string &jetType) {
  int nJet = jets.size();
  bool isSig = (jetType == "Sig");

  if (!m_doReduceFillHistos) {
    m_histSvc->BookFillHist("N" + jetType + "Jets", 11, -0.5, 10.5, nJet,
                            m_weight);

    for (const xAOD::Jet *jet : jets) {
      m_histSvc->BookFillHist("Pt" + jetType + "Jets", 100, 0, 100,
                              jet->pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("Eta" + jetType + "Jets", 100, -5, 5, jet->eta(),
                              m_weight);
    }

    // TODO: we may want to allow track-jets to be handled by fill_bTaggerHists
    // - some changed would be needed there
    if (isSig) {
      for (const xAOD::Jet *jet : jets) {
        EL_CHECK("AnalysisReader_VHQQ::fill_nJetHistos",
                 fill_bTaggerHists(jet));
      }
    }
  }

  return EL::StatusCode::SUCCESS;
}  // fill_nJetHistos

EL::StatusCode AnalysisReader_VHQQ::fill_fatJetHistos(
    const std::vector<const xAOD::Jet *> &fatJets) {
  if (m_doOnlyInputs) return EL::StatusCode::SUCCESS;
  TLorentzVector HVec = m_etree->exists<TLorentzVector>("HVec")
                            ? (*m_etree)["HVec"]
                            : TLorentzVector();

  if (!m_doReduceFillHistos) {
    for (auto fatjet : fatJets) {
      decltype(m_weight) eventWeight = 1.0;
      auto GeV = 1.0E-3;

      m_histSvc->BookFillHist("nBTag_nTrackJets_FatJet", 5, 0, 5, 10, 0, 10,
                              Props::nBTags.get(fatjet),
                              Props::nTrackJets.get(fatjet), eventWeight);

      if (m_isMC) {
        m_histSvc->BookFillHist("nBTag_nTrueBJets_FatJet", 5, 0, 5, 6, 0, 6,
                                Props::nBTags.get(fatjet),
                                Props::nTrueBJets.get(fatjet), eventWeight);
        m_histSvc->BookFillHist("nTrueBJets_nTrackJets_FatJet", 6, 0, 6, 10, 0,
                                10, Props::nTrueBJets.get(fatjet),
                                Props::nTrackJets.get(fatjet), eventWeight);
        m_histSvc->BookFillHist("nTrueBJets_eta_FatJet", 6, 0, 6, 30, -3, 3,
                                Props::nTrueBJets.get(fatjet), fatjet->eta(),
                                eventWeight);
        m_histSvc->BookFillHist("nTrueBJets_pT_FatJet", 6, 0, 6, 90, 100, 1000,
                                Props::nTrueBJets.get(fatjet),
                                fatjet->pt() * GeV, eventWeight);
      }
      m_histSvc->BookFillHist("eta_nTrackJets_FatJet", 30, -3, 3, 10, 0, 10,
                              fatjet->eta(), Props::nTrackJets.get(fatjet),
                              eventWeight);
      m_histSvc->BookFillHist("pT_nTrackJets_FatJet", 90, 100, 1000, 10, 0, 10,
                              fatjet->pt() * GeV, Props::nTrackJets.get(fatjet),
                              eventWeight);
      m_histSvc->BookFillHist("nBTag_eta_FatJet", 5, 0, 5, 30, -3, 3,
                              Props::nBTags.get(fatjet), fatjet->eta(),
                              eventWeight);
      m_histSvc->BookFillHist("nBTag_pT_FatJet", 5, 0, 5, 90, 100, 1000,
                              Props::nBTags.get(fatjet), fatjet->pt() * GeV,
                              eventWeight);
    }
  }

  if (fatJets.size() >= 1) {
    if (!m_doReduceFillHistos) {
      m_histSvc->BookFillHist("PtFatJ1", 75, 0, 1500, HVec.Pt() / 1.e3,
                              m_weight);
      m_histSvc->BookFillHist("EtaFatJ1", 25, -2.5, 2.5, HVec.Eta(), m_weight);
      m_histSvc->BookFillHist("NtrkjetsFatJ1", 10, -0.5, 9.5,
                              Props::nTrackJets.get(fatJets.at(0)), m_weight);
      m_histSvc->BookFillHist("NbTagsFatJ1", 5, -0.5, 4.5,
                              Props::nBTags.get(fatJets.at(0)), m_weight);
      if (fatJets.size() >= 2) {
        TLorentzVector fatj2Vec = m_etree->exists<TLorentzVector>("fatj2Vec")
                                      ? (*m_etree)["fatj2Vec"]
                                      : TLorentzVector();
        m_histSvc->BookFillHist("PtFatJ2", 150, 0, 1500, fatj2Vec.Pt() / 1.e3,
                                m_weight);
        m_histSvc->BookFillHist("EtaFatJ2", 60, -3, 3, fatj2Vec.Eta(),
                                m_weight);
        m_histSvc->BookFillHist("MFatJ2", 100, 0, 500, fatj2Vec.M() / 1.e3,
                                m_weight);
        m_histSvc->BookFillHist("NtrkjetsFatJ2", 10, -0.5, 4.5,
                                Props::nTrackJets.get(fatJets.at(1)), m_weight);
        m_histSvc->BookFillHist("NbTagsFatJ2", 5, -0.5, 4.5,
                                Props::nBTags.get(fatJets.at(1)), m_weight);
      }
    }
  }

  return EL::StatusCode::SUCCESS;
}  // fill_fatJetHistos

void AnalysisReader_VHQQ::setevent_flavour(
    const std::vector<const xAOD::Jet *> &selectedJets) {
  if (m_isMC) {
    int jet0flav = -1;
    int jet1flav = -1;
    int jet2flav = -1;

    if (selectedJets.size() > 0) {
      jet0flav = Props::HadronConeExclTruthLabelID.get(selectedJets.at(0));
    }
    if (selectedJets.size() > 1) {
      jet1flav = Props::HadronConeExclTruthLabelID.get(selectedJets.at(1));
    }
    if (selectedJets.size() > 2) {
      jet2flav = Props::HadronConeExclTruthLabelID.get(selectedJets.at(2));
    }
    m_physicsMeta.b1Flav = jet0flav;
    m_physicsMeta.b2Flav = jet1flav;
    m_physicsMeta.j3Flav = jet2flav;
  }
}  // setevent_flavour

void AnalysisReader_VHQQ::setevent_flavourGhost(
    const std::vector<const xAOD::Jet *> &selectedJets) {
  //////////////////////////////
  // Get event flavour from the truth particles that are ghost matched to
  // the leading jet of selectedJets
  // Do this by checking the number of matched BHadrons.
  // If the number is >= 2 tag both jets 'b'
  // If the number is == 1 but there is a matched prompt C hadron, tag the
  // jets 'bc'
  // If the number is == 0 and there is no matched prompt C hadron, tag the
  // jets 'bl'
  // (do the corresponding thing for 'cc', 'cl' [and respect 'cb' == 'bc'])
  //
  // setevent_flavour is not suited for this as it is
  // (ahoenle, 2016-05-30)
  //////////////////////////////
  if (m_isMC) {
    int jet0flav = -1;
    int jet1flav = -1;
    if (selectedJets.size() > 0) {
      if (!Props::nGhostMatchedBHadrons.exists(selectedJets.at(0))) {
        // ghost matching not available -> Error
        Error("setevent_flavourGhost()",
              "Ghost Matching not available for "
              "this event. No labeling can be applied.");
        return;
      } else {
        int nGMBHadrons = Props::nGhostMatchedBHadrons.get(selectedJets.at(0));
        int nGMPromptCHadrons =
            Props::nGhostMatchedPromptCHadrons.get(selectedJets.at(0));
        // 1) two or more B hadrons
        if (nGMBHadrons >= 2) {
          jet0flav = 5;
          jet1flav = 5;
          // 2) exactly one B hadron
        } else if (nGMBHadrons == 1) {
          jet0flav = 5;
          if (nGMPromptCHadrons >= 1) {
            jet1flav = 4;
          } else {
            jet1flav = 0;  // != 4, != 5 makes light in HistNameSvc.cxx
          }
          // 3) zero or less B hadrons implicit
          //    -> third case two or more C Hadrons
        } else if (nGMPromptCHadrons >= 2) {
          jet0flav = 4;
          jet1flav = 4;
          // 4) exactly one C Hadron
        } else if (nGMPromptCHadrons == 1) {
          jet0flav = 4;
          jet1flav = 0;
          // 5) only light
        } else {
          jet0flav = 0;
          jet1flav = 0;
        }
        m_physicsMeta.b1Flav = jet0flav;
        m_physicsMeta.b2Flav = jet1flav;
        m_physicsMeta.j3Flav = -1;
        if (selectedJets.size() > 1)
          m_physicsMeta
              .j3Flav = Props::HadronConeExclTruthLabelID.get(selectedJets.at(
              1));  // this is supposed to be the leading non-matched track jet
      }
    }
  }
}  // setevent_flavourGhost

void AnalysisReader_VHQQ::setevent_nJets(
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets) {
  auto nSignalJet = signalJets.size();
  auto nForwardJet = forwardJets.size();
  auto nJet = nSignalJet + nForwardJet;

  m_physicsMeta.nJets = nJet;
  m_physicsMeta.nSigJet = nSignalJet;
  m_physicsMeta.nForwardJet = nForwardJet;
}  // setevent_nJets

void AnalysisReader_VHQQ::setevent_nJets(
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets,
    const std::vector<const xAOD::Jet *> &fatJets) {
  auto nSignalJet = signalJets.size();
  auto nForwardJet = forwardJets.size();
  auto nJet = nSignalJet + nForwardJet;
  auto nFatJet = fatJets.size();

  m_physicsMeta.nJets = nJet;
  m_physicsMeta.nSigJet = nSignalJet;
  m_physicsMeta.nForwardJet = nForwardJet;
  m_physicsMeta.nFatJet = nFatJet;
}  // setevent_nJets

std::string AnalysisReader_VHQQ::determine_regime() {
  return "resolved";

  // return "merged";
}

EL::StatusCode AnalysisReader_VHQQ::fill_jetSelectedHistos() {
  int nJet = m_etree->get<int>("nJets");

  if (!m_doOnlyInputs || m_model == Model::CUT) {
    if (!m_doBlinding)
      m_histSvc->BookFillHist("mBB", 100, 0, 500, m_etree->get<float>("mBB"),
                              m_weight);
  }
  if (!m_doOnlyInputs) {
    m_histSvc->BookFillHist("pTV", 100, 0, 1000, m_etree->get<float>("pTV"),
                            m_weight);
    if (m_etree->get<float>("mBB") >= 50) {
      m_histSvc->BookFillHist("MJGE50pTV", 100, 0, 1000,
                              m_etree->get<float>("pTV"), m_weight);
    }
  }
  if (!m_doOnlyInputs) {
    // Main histograms
    // Additional histograms
    if (!m_doReduceFillHistos) {
      m_histSvc->BookFillHist("pTB1", 200, 0, 2000, m_etree->get<float>("pTB1"),
                              m_weight);
      m_histSvc->BookFillHist("pTB2", 150, 0, 1500, m_etree->get<float>("pTB2"),
                              m_weight);
      m_histSvc->BookFillHist("EtaB1", 60, -3, 3, m_etree->get<float>("etaB1"),
                              m_weight);
      m_histSvc->BookFillHist("EtaB2", 60, -3, 3, m_etree->get<float>("etaB2"),
                              m_weight);
      m_histSvc->BookFillHist("dPhiBB", 50, 0, 3.15,
                              std::fabs(m_etree->get<float>("dPhiBB")),
                              m_weight);
      m_histSvc->BookFillHist("dEtaBB", 100, 0, 5,
                              m_etree->get<float>("dEtaBB"), m_weight);
      if (m_etree->exists<float>("MV2c10B1"))
        m_histSvc->BookFillHist("MV2c10B1", 100, -1, 1,
                                m_etree->get<float>("MV2c10B1"), m_weight);
      if (m_etree->exists<float>("MV2c10B2"))
        m_histSvc->BookFillHist("MV2c10B2", 100, -1, 1,
                                m_etree->get<float>("MV2c10B2"), m_weight);
      m_histSvc->BookFillHist("dRBB", 100, 0, 5, m_etree->get<float>("dRBB"),
                              m_weight);
      m_histSvc->BookFillHist("pTBB", 400, 0, 2000, m_etree->get<float>("pTBB"),
                              m_weight);

      if (nJet >= 3) {  // this should actually be if (selectedJets.size() >=
                        // 3)...see compute_jetSelectedQuantities
        m_histSvc->BookFillHist("pTJ3", 100, 0, 500,
                                m_etree->get<float>("pTJ3"), m_weight);
        m_histSvc->BookFillHist("EtaJ3", 100, -5, 5,
                                m_etree->get<float>("etaJ3"), m_weight);
        m_histSvc->BookFillHist("mBBJ", 100, 0, 750,
                                m_etree->get<float>("mBBJ"), m_weight);
        m_histSvc->BookFillHist("pTBBJ", 600, 0, 3000,
                                m_etree->get<float>("pTBBJ"), m_weight);
      }
    }
  }
  return EL::StatusCode::SUCCESS;
}  // fill_jetSelectedHistos

// For the analysis that use MVA Tree, filling all histogram from MVA tree if
// the variables are inclueded in MVA tree
EL::StatusCode AnalysisReader_VHQQ::fill_MVAVariablesHistos() {
  // m_doOnlyInputs : most important plots
  if (!m_doOnlyInputs || m_histNameSvc->get_nTag() == 2) {
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged)
      m_histSvc->BookFillHist("mJ", 100, 0., 500., m_tree->mBB, m_weight);
    else
      m_histSvc->BookFillHist("mBB", 500, 0., 500., m_tree->mBB, m_weight);
    m_histSvc->BookFillHist("pTV", 40, 0, 1000, m_tree->pTV, m_weight);
  }
  if (m_doOnlyInputs) return EL::StatusCode::SUCCESS;

  // doReduceFillHistos : basic plots
  if (m_doReduceFillHistos) return EL::StatusCode::SUCCESS;

  // "not important" plots, for debugging or other studies
  m_histSvc->BookFillHist("NSigJets", 11, -0.5, 10.5, m_tree->nSigJet,
                          m_weight);
  m_histSvc->BookFillHist("NFwdJets", 11, -0.5, 10.5, m_tree->nForwardJet,
                          m_weight);
  m_histSvc->BookFillHist("NJets", 11, -0.5, 10.5, m_tree->nJ, m_weight);
  m_histSvc->BookFillHist("NbJets", 11, -0.5, 10.5, m_tree->nTags, m_weight);
  m_histSvc->BookFillHist("NTaus", 11, -0.5, 10.5, m_tree->nTaus, m_weight);
  m_histSvc->BookFillHist("MET", 40, 0, 200, m_tree->MET, m_weight);
  m_histSvc->BookFillHist("dPhiVH", 64, 0, TMath::Pi(), m_tree->dPhiVBB,
                          m_weight);
  m_histSvc->BookFillHist("dEtaVH", 50, 0, 5, m_tree->dEtaVBB, m_weight);

  m_histSvc->BookFillHist("pTB1", 200, 0, 2000, m_tree->pTB1, m_weight);
  m_histSvc->BookFillHist("pTB2", 150, 0, 1500, m_tree->pTB2, m_weight);
  m_histSvc->BookFillHist("EtaB1", 60, -3, 3, m_tree->etaB1, m_weight);
  m_histSvc->BookFillHist("EtaB2", 60, -3, 3, m_tree->etaB2, m_weight);
  m_histSvc->BookFillHist("dEtaBB", 100, 0, 5, m_tree->dEtaBB, m_weight);
  m_histSvc->BookFillHist("dPhiBB", 50, 0, TMath::Pi(), m_tree->dPhiBB,
                          m_weight);
  m_histSvc->BookFillHist("dRBB", 100, 0, 5, m_tree->dRBB, m_weight);
  m_histSvc->BookFillHist("sumPt", 200, 0, 2000, m_tree->sumPtJets, m_weight);

  if (m_tree->nJ >= 3) {
    m_histSvc->BookFillHist("pTJ3", 100, 0, 500, m_tree->pTJ3, m_weight);
    m_histSvc->BookFillHist("EtaJ3", 100, -5, 5, m_tree->etaJ3, m_weight);
    m_histSvc->BookFillHist("mBBJ", 100, 0, 750, m_tree->mBBJ, m_weight);
  }
  return EL::StatusCode::SUCCESS;
}  // fill_MVAVariablesHistos

// Elisabeth: comment out instances where MVATree is filled
// -> MVATree is only filled using the dedicated fill function
// Some EasyTree variables are filled here
// ToDo (maybe): Review to fill EasyTree with dedicted function
// to keep better track of information filled
// ToDo: review compute_jetSelectedQuantities
// -> currently it is only used by 2 lepton
// -> use also for 0 and 1 lepton or remove?

EL::StatusCode AnalysisReader_VHQQ::compute_jetSelectedQuantities(
    const std::vector<const xAOD::Jet *> &selectedJets) {
  if (selectedJets.size() >= 1) (*m_etree)["j1Vec"] = selectedJets.at(0)->p4();
  if (selectedJets.size() >= 2) (*m_etree)["j2Vec"] = selectedJets.at(1)->p4();
  if (selectedJets.size() >= 3) (*m_etree)["j3Vec"] = selectedJets.at(2)->p4();

  int nJet = m_physicsMeta.nJets;
  int nSigJet = m_physicsMeta.nSigJet;
  int nFwdJet = m_physicsMeta.nForwardJet;
  int nbJet = m_physicsMeta.nTags;

  TLorentzVector j1Vec = m_etree->exists<TLorentzVector>("j1Vec")
                             ? (*m_etree)["j1Vec"]
                             : TLorentzVector();
  TLorentzVector j2Vec = m_etree->exists<TLorentzVector>("j2Vec")
                             ? (*m_etree)["j2Vec"]
                             : TLorentzVector();
  TLorentzVector j3Vec = m_etree->exists<TLorentzVector>("j3Vec")
                             ? (*m_etree)["j3Vec"]
                             : TLorentzVector();

  // HVec and bbj should be built using the corrected jets (e.g. mu-in-jet)
  TLorentzVector HVec = m_etree->exists<TLorentzVector>("HVec")
                            ? (*m_etree)["HVec"]
                            : TLorentzVector();
  TLorentzVector bbj = m_etree->exists<TLorentzVector>("bbjVec")
                           ? (*m_etree)["bbjVec"]
                           : TLorentzVector();

  m_etree->SetBranchAndValue<int>("nJets", nJet, -1);
  m_etree->SetBranchAndValue<int>("nSigJets", nSigJet, -1);
  m_etree->SetBranchAndValue<int>("nFwdJets", nFwdJet, -1);
  m_etree->SetBranchAndValue<int>("nbJets", nbJet, -1);
  m_etree->SetBranchAndValue<float>("dRBB", j1Vec.DeltaR(j2Vec), -99);
  m_etree->SetBranchAndValue<float>("dPhiBB", j1Vec.DeltaPhi(j2Vec), -99);
  m_etree->SetBranchAndValue<float>("dEtaBB", fabs(j1Vec.Eta() - j2Vec.Eta()),
                                    -99);
  m_etree->SetBranchAndValue<float>("mBB", HVec.M() / 1e3, -99);

  m_etree->SetBranchAndValue<float>(
      "pTBB", HVec.Pt() / 1e3,
      -99);  // using mu-in-jet-corr, is it what we want?
  m_etree->SetBranchAndValue<float>("pTB1", j1Vec.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("pTB2", j2Vec.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("etaB1", j1Vec.Eta(), -99);
  m_etree->SetBranchAndValue<float>("etaB2", j2Vec.Eta(), -99);

  float mv2c10_b1(-99);
  float mv2c10_b2(-99);

  if (selectedJets.size() >= 1)
    mv2c10_b1 = Props::MV2c10.get(selectedJets.at(0));
  if (selectedJets.size() >= 2)
    mv2c10_b2 = Props::MV2c10.get(selectedJets.at(1));

  m_etree->SetBranchAndValue<float>("MV2c10B1", mv2c10_b1, -99);
  m_etree->SetBranchAndValue<float>("MV2c10B2", mv2c10_b2, -99);

  if (m_isMC) {  // those values get overwritten if event ends up in merged
                 // regime...
    m_etree->SetBranchAndValue<int>("flavB1", m_physicsMeta.b1Flav, -99);
    m_etree->SetBranchAndValue<int>("flavB2", m_physicsMeta.b2Flav, -99);
    m_etree->SetBranchAndValue<int>("flavJ3", m_physicsMeta.j3Flav, -99);
  }

  float pTJ3(-99);
  float etaJ3(-99);
  float mBBJ(-99);
  float pTBBJ(-99);

  if (selectedJets.size() >= 3) {
    pTJ3 = j3Vec.Pt() / 1e3;
    etaJ3 = j3Vec.Eta();
    mBBJ = bbj.M() / 1e3;
    pTBBJ = bbj.Pt() / 1e3;
  }

  m_etree->SetBranchAndValue<float>("pTJ3", pTJ3, -99);
  m_etree->SetBranchAndValue<float>("etaJ3", etaJ3, -99);
  m_etree->SetBranchAndValue<float>("mBBJ", mBBJ, -99);
  m_etree->SetBranchAndValue<float>("pTBBJ", pTBBJ, -99);

  // m_tree->nJ     = nJet;
  // m_tree->dRBB   = j1Vec.DeltaR(j2Vec);
  // m_tree->dPhiBB = j1Vec.DeltaPhi(j2Vec);
  // m_tree->dEtaBB = fabs(j1Vec.Eta() - j2Vec.Eta());
  // m_tree->mBB    = HVec.M();
  // m_tree->pTB1   = j1Vec.Pt();
  // m_tree->pTB2   = j2Vec.Pt();

  if (nJet >= 3) {
    // m_tree->pTJ3 = j3Vec.Pt();
    // m_tree->etaJ3 = j3Vec.Eta();
    // m_tree->dRB1J3 = j3Vec.DeltaR(j1Vec);
    // m_tree->dRB2J3 = j3Vec.DeltaR(j2Vec);
    // m_tree->mBBJ = bbj.M();
  }

  return EL::StatusCode::SUCCESS;
}  // compute_jetSelectedQuantities

void AnalysisReader_VHQQ::rescale_jets(double mBB, TLorentzVector &j1Vec,
                                       TLorentzVector &j2Vec) {
  double mbb_weight = 125.0 / (mBB * 1e-3);

  j1Vec *= mbb_weight;
  j2Vec *= mbb_weight;
}  // rescale_jets

void AnalysisReader_VHQQ::rescale_leptons(double mLL, TLorentzVector &l1Vec,
                                          TLorentzVector &l2Vec) {
  double mll_weight = 91.188 / (mLL * 1e-3);

  l1Vec *= mll_weight;
  l2Vec *= mll_weight;
}  // rescale_leptons

std::vector<const xAOD::Jet *> AnalysisReader_VHQQ::ApplyMuonFatJetOR(
    std::vector<const xAOD::Muon *> Muons,
    std::vector<const xAOD::Jet *> fatjets) {
  /**
   * Remove fatjets if they overlap with muons
   * Overlap means the DeltaR between the 4-vectors is < 1.2
   *
   * @param Muons The Muons that are considered for OR
   * @param fatjets The fatjets that are considered for OR
   * @return A vector of fatjets where no entry overlaps with the Muons
   */
  if (m_debug) {
    Info("ApplyMuonFatJetOR()", "fatjets.size(): %lu", fatjets.size());
    Info("ApplyMuonFatJetOR()", "Muons.size():   %lu", Muons.size());
  }
  //----------------------------
  // Large R Jet Higgs candidate
  //----------------------------
  // Check muon - largeRJet OR
  if (Muons.size() > 0 && fatjets.size() > 0) {
    // Return vector
    std::vector<const xAOD::Jet *> ValidFatJets;

    // require dR(jet,muon) > 1.2
    std::vector<const xAOD::Jet *>::iterator iter = fatjets.begin();
    std::vector<const xAOD::Jet *>::iterator iterEnd = fatjets.end();
    int counter = 0;
    for (; iter != iterEnd; iter++) {
      if (!(*iter)) continue;
      if (m_debug) {
        Info("ApplyMuonFatJetOR()", "FatJet # %d", ++counter);
      }

      // Loop over the muons
      bool MuFatJetOverlap = false;
      int mcounter = 0;
      for (const xAOD::Muon *muon : Muons) {
        if (!muon) continue;
        if (m_debug) {
          Info("ApplyMuonFatJetOR()", "Muon # %d", ++mcounter);
        }
        TLorentzVector lepVec = muon->p4();
        if (m_debug) {
          Info("ApplyMuonFatJetOR()", "Checking Muon-Fat Overlap");
        }
        if (((*iter)->p4()).DeltaR(lepVec) < 1.2) {
          if (m_debug) {
            Info("ApplyMuonFatJetOR()", "Muon and FatJet are overlapping");
          }
          MuFatJetOverlap = true;
          break;
        }
      }

      // Check fatjet overlap with all muons
      if (!MuFatJetOverlap) {
        if (m_debug) {
          Info("ApplyMuonFatJetOR()", "Adding fatjet");
        }
        ValidFatJets.push_back((*iter));
      } else if (m_debug) {
        Info("ApplyMuonFatJetOR()", "Skipping fatjet as Mu-FatJet OR");
      }
    }
    if (m_debug) {
      Info("ApplyMuonFatJetOR()", "ApplyMuonFatJetOR() finished");
    }

    return ValidFatJets;
  } else {
    Info("ApplyMuonFatJetOR()", "No muons, returning original fatjets");
    return fatjets;
  }

}  // End of ApplyMuonFatJetOR()

// function to create the bit mask
unsigned long int AnalysisReader_VHQQ::bitmask(
    const unsigned long int cut,
    const std::vector<unsigned long int> &excludeCuts) {
  unsigned long int mask = 0;
  unsigned long int bit = 0;

  const unsigned long int offBit = 0;
  const unsigned long int onBit = 1;

  for (unsigned long int i = 0; i < cut + 1; ++i) {
    if (std::find(excludeCuts.begin(), excludeCuts.end(), i) !=
        excludeCuts.end()) {
      // if a cut should be excluded set the corresponding bit to 0
      mask = mask | offBit << bit++;
    } else {
      // otherwise set the bit to 1
      mask = mask | onBit << bit++;
    }
  }

  return mask;
}

// function to compare a bit flag against a bit mask
// Usage: given a flag, check if it passes all cuts up to and including "cut"
// excluding the cuts in "excludedCuts"
bool AnalysisReader_VHQQ::passAllCutsUpTo(
    const unsigned long int flag, const unsigned long int cut,
    const std::vector<unsigned long int> &excludeCuts) {
  // Get the bitmask: we want to check all cuts up to "cut" excluding the cuts
  // listed in excludeCuts
  unsigned long int mask = bitmask(cut, excludeCuts);
  // Check if the flag matches the bit mask
  return (flag & mask) == mask;
}

// a function to check specifig cuts
bool AnalysisReader_VHQQ::passSpecificCuts(
    const unsigned long int flag, const std::vector<unsigned long int> &cuts) {
  unsigned long int mask = 0;
  const unsigned long int onBit = 1;

  // Make the bit mask
  for (auto cut : cuts) mask = mask | (onBit << cut);
  // Check if the flag matches the bit mask
  return (flag & mask) == mask;
}

// a function to update the bit flag
void AnalysisReader_VHQQ::updateFlag(unsigned long int &flag,
                                     const unsigned long int cutPosition,
                                     const unsigned long int passCut) {
  // Put bit passCut (true or false) at position cutPosition
  flag = flag | passCut << cutPosition;
}

void AnalysisReader_VHQQ::computeHT(
    double &HTsmallR, double &HTlargeR, const Lepton &l1, const Lepton &l2,
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets,
    const std::vector<const xAOD::Jet *> &fatJets) {
  // leptons
  HTsmallR += l1.vec.Pt() + l2.vec.Pt();
  HTlargeR += l1.vec.Pt() + l2.vec.Pt();
  // forward jets
  for (unsigned int i = 0; i < forwardJets.size(); i++) {
    HTsmallR += forwardJets.at(i)->pt();
  }
  // signal jets
  for (unsigned int i = 0; i < signalJets.size(); i++) {
    HTsmallR += signalJets.at(i)->pt();
  }
  // fat jets
  for (unsigned int i = 0; i < fatJets.size(); i++) {
    HTlargeR += fatJets.at(i)->pt();
  }
}  // computeHT

TLorentzVector AnalysisReader_VHQQ::getMETCorrTLV(
    const xAOD::MissingET *met,
    std::vector<const TLorentzVector *> removeObjects,
    std::vector<const TLorentzVector *> addObjects) {
  TLorentzVector metVec;
  double met_x = met->mpx();
  double met_y = met->mpy();
  for (auto jet : removeObjects) {
    met_x += jet->Px();
    met_y += jet->Py();
  }
  for (auto jet : addObjects) {
    met_x -= jet->Px();
    met_y -= jet->Py();
  }
  double met_e = sqrt(met_x * met_x + met_y * met_y);
  metVec.SetPxPyPzE(met_x, met_y, 0, met_e);
  return metVec;
}  // getMETCorrTLV

EL::StatusCode AnalysisReader_VHQQ::setJetVariables(
    Jet &j1, Jet &j2, Jet &j3,
    const std::vector<const xAOD::Jet *> &selectedJets, double ptv_for_FSR) {
  int nJet = selectedJets.size();

  const xAOD::Jet *jet1 = nullptr;
  const xAOD::Jet *jet2 = nullptr;
  const xAOD::Jet *jet3 = nullptr;

  // j1
  if (nJet >= 1) {
    jet1 = selectedJets.at(0);
    j1.vec = jet1->p4();
    if (m_isMC) {
      EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
               getTLVFromJet(jet1, j1.vec_truthwz, "TruthWZ"));
    }
    j1.isTagged = BTagProps::isTagged.get(jet1);
    j1.isMu = (Props::nrMuonInJet.get(jet1) >
               0);  // without 04 means variable radious DR<VR<0.4
    j1.isMu04 = (Props::nrMuonInJet04.exists(jet1))
                    ? (Props::nrMuonInJet04.get(jet1) > 0)
                    : j1.isMu;  // 04 means fixed radius DR<FR=0.4
    j1.isEl = (Props::nrElectronInJet.exists(jet1))
                  ? (Props::nrElectronInJet.get(jet1) > 0)
                  : 0;
    j1.isEl04 = (Props::nrElectronInJet04.exists(jet1))
                    ? (Props::nrElectronInJet04.get(jet1) > 0)
                    : j1.isEl;

    j1.isSL = (j1.isMu || j1.isEl);
    j1.isSL04 = (j1.isMu04 || j1.isEl04);
    j1.nrMuonInJet = Props::nrMuonInJet.get(jet1);
  }

  // j2
  if (nJet >= 2) {
    jet2 = selectedJets.at(1);
    j2.vec = jet2->p4();
    if (m_isMC) {
      EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
               getTLVFromJet(jet2, j2.vec_truthwz, "TruthWZ"));
    }
    j2.isTagged = BTagProps::isTagged.get(jet2);
    j2.isMu = (Props::nrMuonInJet.get(jet2) >
               0);  // without 04 means variable radious DR<VR<0.4
    j2.isMu04 = (Props::nrMuonInJet04.exists(jet2))
                    ? (Props::nrMuonInJet04.get(jet2) > 0)
                    : j2.isMu;  // 04 means fixed radius DR<FR=0.4
    j2.isEl = (Props::nrElectronInJet.exists(jet2))
                  ? (Props::nrElectronInJet.get(jet2) > 0)
                  : 0;
    j2.isEl04 = (Props::nrElectronInJet04.exists(jet2))
                    ? (Props::nrElectronInJet04.get(jet2) > 0)
                    : j2.isEl;

    j2.isSL = (j2.isMu || j2.isEl);
    j2.isSL04 = (j2.isMu04 || j2.isEl04);
    j2.nrMuonInJet = Props::nrMuonInJet.get(jet2);
  }

  // j3
  if (nJet >= 3) {
    jet3 = selectedJets.at(2);
    j3.vec = jet3->p4();
    if (m_isMC) {
      EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
               getTLVFromJet(jet3, j3.vec_truthwz, "TruthWZ"));
    }
    j3.isTagged = BTagProps::isTagged.get(jet3);
    j3.isMu = (Props::nrMuonInJet.get(jet3) >
               0);  // without 04 means variable radious DR<VR<0.4
    j3.isMu04 = (Props::nrMuonInJet04.exists(jet3))
                    ? (Props::nrMuonInJet04.get(jet3) > 0)
                    : j3.isMu;  // 04 means fixed radius DR<FR=0.4
    j3.isEl = (Props::nrElectronInJet.exists(jet3))
                  ? (Props::nrElectronInJet.get(jet3) > 0)
                  : 0;
    j3.isEl04 = (Props::nrElectronInJet04.exists(jet3))
                    ? (Props::nrElectronInJet04.get(jet3) > 0)
                    : j3.isEl;
    j3.isSL = (j3.isMu || j3.isEl);
    j3.isSL04 = (j3.isMu04 || j3.isEl04);
    j3.nrMuonInJet = Props::nrMuonInJet.get(jet3);
  }

  // b-jet energy corrections

  if (nJet >= 1 && j1.isTagged) {
    EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
             getBJetEnergyCorrTLV(jet1, j1.vec_onemu, false, "OneMu"));
    EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
             getBJetEnergyCorrTLV(jet1, j1.vec_corr, false, m_jetCorrType));
  } else {
    j1.vec_onemu = j1.vec;
    j1.vec_corr = j1.vec;
  }
  if (nJet >= 2 && j2.isTagged) {
    EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
             getBJetEnergyCorrTLV(jet2, j2.vec_onemu, false, "OneMu"));
    EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
             getBJetEnergyCorrTLV(jet2, j2.vec_corr, false, m_jetCorrType));
  } else {
    j2.vec_onemu = j2.vec;
    j2.vec_corr = j2.vec;
  }
  if (nJet >= 3 && j3.isTagged) {
    EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
             getBJetEnergyCorrTLV(jet3, j3.vec_onemu, false, "OneMu"));
    EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
             getBJetEnergyCorrTLV(jet3, j3.vec_corr, false, m_jetCorrType));
  } else {
    j3.vec_onemu = j3.vec;
    j3.vec_corr = j3.vec;
  }

  // FSR recovery
  m_hasFSR = 0;
  if (m_physicsMeta.nJets == 3 && j1.isTagged && j2.isTagged && !j3.isTagged) {
    std::tie(j1.vec_fsr, j2.vec_fsr) =
        calculateFSR(j1.vec_corr, j2.vec_corr, j3.vec_corr, ptv_for_FSR);
  } else {
    j1.vec_fsr = j1.vec_corr;
    j2.vec_fsr = j2.vec_corr;
  }

  // mBB rescaling
  if (nJet >= 1) j1.vec_resc = j1.vec_corr;  // will be set properly later
  if (nJet >= 2) j2.vec_resc = j2.vec_corr;  // will be set properly later
  if (nJet >= 3) j3.vec_resc = j3.vec_corr;  // won't be rescaled

  if (nJet >= 1) j1.vec_gsc = j1.vec;
  if (nJet >= 2) j2.vec_gsc = j2.vec;
  if (nJet >= 3) j3.vec_gsc = j3.vec;

  if (nJet >= 1) j1.vec_ptreco = j1.vec_corr;
  if (nJet >= 2) j2.vec_ptreco = j2.vec_corr;
  if (nJet >= 3) j3.vec_ptreco = j3.vec_corr;

  if (nJet == 3 && m_doFSRrecovery) {
    j1.vec_corr = j1.vec_fsr;
    j2.vec_corr = j2.vec_fsr;
    if (m_hasFSR) j3.vec_corr.SetPtEtaPhiM(0, 0, 0, 0);
  }

  if (nJet >= 1) j1.vec_kf = j1.vec_corr;
  if (nJet >= 2) j2.vec_kf = j2.vec_corr;
  if (nJet >= 3) j3.vec_kf = j3.vec_corr;

  if (nJet >= 1) j1.resolution = Props::resolution.get(jet1);
  if (nJet >= 2) j2.resolution = Props::resolution.get(jet2);
  if (nJet >= 3) j3.resolution = Props::resolution.get(jet3);

  return EL::StatusCode::SUCCESS;

}  // setJetVariables

EL::StatusCode AnalysisReader_VHQQ::fillMVATreeVHbbResolved(
    TLorentzVector &b1, TLorentzVector &b2, TLorentzVector &j3,
    const std::vector<const xAOD::Jet *> &selectedJets,
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &HVec,
    TLorentzVector &VVec, TLorentzVector &metVec, int nTaus) {
  // will be called separately in analysis loop
  // m_tree->Reset();
  // m_tree->SetVariation(m_currentVar);

  // DO NOT INTRODUCE NEW VARIABLES HERE IF NOT ABSOLUTELY NECESSARY FOR MVA
  // please use the EasyTree functionality for studies

  // event info
  m_tree->sample = m_histNameSvc->getFullSample();
  m_tree->EventNumber = m_eventInfo->eventNumber();
  m_tree->EventWeight = m_weight;
  m_tree->ChannelNumber = m_mcChannel;

  // event kinematics
  m_tree->nSigJet = m_physicsMeta.nSigJet;
  m_tree->nForwardJet = m_physicsMeta.nForwardJet;
  m_tree->nJ = m_physicsMeta.nJets;
  m_tree->nTags = m_physicsMeta.nTags;
  m_tree->nTaus = nTaus;
  m_tree->MET = metVec.Pt() / 1e3;
  m_tree->dPhiVBB = fabs(VVec.DeltaPhi(HVec));
  m_tree->dEtaVBB = fabs(VVec.Eta() - HVec.Eta());

  double soft_mpx = Props::soft_mpx.get(m_met) / 1e3;
  double soft_mpy = Props::soft_mpy.get(m_met) / 1e3;
  m_tree->softMET = sqrt(soft_mpx * soft_mpx + soft_mpy * soft_mpy);

  // Attention: this is before b-jet corrections
  float sumpt = 0;
  for (unsigned int s_i = 0; s_i < signalJets.size(); s_i++) {
    sumpt += signalJets.at(s_i)->pt();
  }
  for (unsigned int f_i = 0; f_i < forwardJets.size(); f_i++) {
    sumpt += forwardJets.at(f_i)->pt();
  }
  m_tree->sumPtJets = sumpt / 1e3;

  // vector boson
  m_tree->pTV = VVec.Pt() / 1e3;
  m_tree->phiV = VVec.Phi();

  // higgs boson/jet system
  m_tree->mBB = HVec.M() / 1e3;
  m_tree->dRBB = b1.DeltaR(b2);
  m_tree->dEtaBB = fabs(b1.Eta() - b2.Eta());
  m_tree->dPhiBB = fabs(b1.DeltaPhi(b2));
  m_tree->mBBJ = (HVec + j3).M() / 1e3;

  // jets
  m_tree->pTB1 = b1.Pt() / 1e3;
  m_tree->pTB2 = b2.Pt() / 1e3;
  m_tree->pTJ3 = j3.Pt() / 1e3;
  m_tree->etaB1 = b1.Eta();
  m_tree->etaB2 = b2.Eta();
  m_tree->etaJ3 = j3.Eta();
  m_tree->phiB1 = b1.Phi();
  m_tree->phiB2 = b2.Phi();
  m_tree->phiJ3 = j3.Phi();
  m_tree->mB1 = b1.M() / 1e3;
  m_tree->mB2 = b2.M() / 1e3;
  m_tree->mJ3 = j3.M() / 1e3;
  if (selectedJets.size() > 0)
    m_tree->bin_MV2c10B1 = BTagProps::Quantile.get(selectedJets.at(0));
  if (selectedJets.size() > 1)
    m_tree->bin_MV2c10B2 = BTagProps::Quantile.get(selectedJets.at(1));
  if (selectedJets.size() > 2)
    m_tree->bin_MV2c10J3 = BTagProps::Quantile.get(selectedJets.at(2));

  m_tree->hasFSR = m_hasFSR;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ::fillOSTree() {
  m_OStree->EventNumber = m_eventInfo->eventNumber();
  m_OStree->RunNumber = m_eventInfo->runNumber();
  m_OStree->BDT = m_tree->BDT;

  if (m_analysisType == "0lep") {
    m_OStree->Category.push_back(0);
  } else if (m_analysisType == "1lep" &&
             m_physicsMeta.region == PhysicsMetadata::Region::SR) {
    //    std::cout<<"######## IS SR ############"<<std::endl;
    m_OStree->Category.push_back(1);
  } else if (m_analysisType == "1lep" &&
             m_physicsMeta.region == PhysicsMetadata::Region::WCR) {
    // std::cout<<"######## IS WCR ############"<<std::endl;
    m_OStree->Category.push_back(4);
  } else if (m_analysisType == "2lep") {
    if (m_physicsMeta.region == PhysicsMetadata::Region::topCR) {
      m_OStree->Category.push_back(3);
    } else {  // m_physicsMeta.region == PhysicsMetadata::Region::SR is not set
              // for 2L SR. Maybe it can be set.
      m_OStree->Category.push_back(2);
    }
  }

  if (m_tree->nJ == 2) {
    // std::cout<<"######## IS 2jet region ############"<<std::endl;
    m_OStree->Category.push_back(2);
  } else if (m_tree->nJ >= 3) {
    m_OStree->Category.push_back(3);
  }

  if (m_tree->pTV < 75) {
    m_OStree->Category.push_back(0);
  } else if (m_tree->pTV > 75 && m_tree->pTV < 150) {
    m_OStree->Category.push_back(1);
  } else if (m_tree->pTV > 150) {
    // std::cout<<"######## IS 150 pTV ############"<<std::endl;
    m_OStree->Category.push_back(2);
  }

  if (m_tree->nTags == 0) {
    m_OStree->Category.push_back(0);
  } else if (m_tree->nTags == 1) {
    m_OStree->Category.push_back(1);
  } else if (m_tree->nTags == 2) {
    // std::cout<<"######## IS 2tag ############"<<std::endl;
    m_OStree->Category.push_back(2);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ::setFatJetVariables(
    Jet &fj1, Jet &fj2, Jet &fj3,
    const std::vector<const xAOD::Jet *> &fatJets) {
  if (fatJets.size() >= 1) {
    EL_CHECK("setFatJetVariables",
             setFatJetVariablesSingle(fj1, fatJets.at(0)));
  }
  if (fatJets.size() >= 2) {
    EL_CHECK("setFatJetVariables",
             setFatJetVariablesSingle(fj2, fatJets.at(1)));
  }
  if (fatJets.size() >= 3) {
    EL_CHECK("setFatJetVariables",
             setFatJetVariablesSingle(fj3, fatJets.at(2)));
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ::setFatJetVariablesSingle(
    Jet &fj, const xAOD::Jet *fatJet) {
  fj.vec = fatJet->p4();
  fj.nBTags = Props::nBTagsAll.get(fatJet);
  if (m_isMC) {
    EL_CHECK("setFatJetVariablesSingle",
             getTLVFromJet(fatJet, fj.vec_truthwz, "TruthWZ"));
  }
  if (fj.nBTags > 0 && fj.vec.M() > 0 && Props::caloMass.get(fatJet) > 0 &&
      Props::TAmass.get(fatJet) > 0) {
    EL_CHECK("setFatJetVariablesSingle",
             getBJetEnergyCorrTLV(fatJet, fj.vec_corr, true, m_fatJetCorrType));
  } else {
    fj.vec_corr = fj.vec;
  }
  fj.vec_gsc = fj.vec;
  fj.vec_onemu = fj.vec_corr;
  fj.vec_ptreco = fj.vec_corr;
  fj.vec_kf = fj.vec_corr;
  fj.vec_resc = fj.vec_corr;
  fj.isSL = (fj.vec_onemu != fj.vec);
  fj.resolution = 0.10;

  return EL::StatusCode::SUCCESS;
}

unsigned int AnalysisReader_VHQQ::countAdditionalCaloJets(
    std::vector<const xAOD::Jet *> &signalJets,
    std::vector<const xAOD::Jet *> &forwardJets, Jet fj1) {
  unsigned int nAddCaloJets = 0;
  for (auto caloJet : signalJets) {
    if (caloJet->p4().DeltaR(fj1.vec) > 1.0) {
      if (caloJet->p4().Pt() > 30e3) nAddCaloJets += 1;
    }
  }
  for (auto caloJet : forwardJets) {
    if (caloJet->p4().DeltaR(fj1.vec) > 1.0) {
      if (caloJet->p4().Pt() > 30e3) nAddCaloJets += 1;
    }
  }
  return nAddCaloJets;
}

void AnalysisReader_VHQQ::setHiggsCandidate(Higgs &resolvedH, Higgs &mergedH,
                                            Jet j1, Jet j2, Jet fj1) {
  // resolved
  resolvedH.vec = j1.vec + j2.vec;
  resolvedH.vec_corr = j1.vec_corr + j2.vec_corr;

  resolvedH.vec_gsc = j1.vec_gsc + j2.vec_gsc;
  resolvedH.vec_onemu = j1.vec_onemu + j2.vec_onemu;
  resolvedH.vec_ptreco = j1.vec_ptreco + j2.vec_ptreco;
  resolvedH.vec_fsr = j1.vec_fsr + j2.vec_fsr;
  resolvedH.vec_kf = j1.vec_kf + j2.vec_kf;
  resolvedH.vec_truthwz = j1.vec_truthwz + j2.vec_truthwz;

  // merged
  mergedH.vec = fj1.vec;
  mergedH.vec_corr = fj1.vec_corr;

  mergedH.vec_gsc = fj1.vec_gsc;
  mergedH.vec_onemu = fj1.vec_onemu;
  mergedH.vec_ptreco = fj1.vec_ptreco;
  mergedH.vec_kf = fj1.vec_kf;
  mergedH.vec_truthwz = fj1.vec_truthwz;

}  // setHiggsCandidate

TLorentzVector AnalysisReader_VHQQ::defineMinDRLepBSyst(const Lepton &l1,
                                                        const Lepton &l2,
                                                        const Jet &j1,
                                                        const Jet &j2) {
  float minDR = 10000;
  TLorentzVector lbVec;
  if (j1.vec.DeltaR(l1.vec) < minDR) {
    minDR = j1.vec.DeltaR(l1.vec);
    lbVec = j1.vec + l1.vec;
  }
  if (j1.vec.DeltaR(l2.vec) < minDR) {
    minDR = j1.vec.DeltaR(l2.vec);
    lbVec = j1.vec + l2.vec;
  }
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    if (j2.vec.DeltaR(l1.vec) < minDR) {
      minDR = j2.vec.DeltaR(l1.vec);
      lbVec = j2.vec + l1.vec;
    }
    if (j2.vec.DeltaR(l2.vec) < minDR) {
      minDR = j2.vec.DeltaR(l2.vec);
      lbVec = j2.vec + l2.vec;
    }
  }
  return lbVec;
}

bool AnalysisReader_VHQQ::checkTightLeptons(const Lepton &l1,
                                            const Lepton &l2) {
  // Check that both leptons are above 25 GeV
  if (l1.vec.Pt() / 1.e3 < 25. || l2.vec.Pt() / 1.e3 < 25.) return false;

  // Check that at least one of them is ZHTight & has pt > 60
  if (!((l1.isZHTight && l1.vec.Pt() / 1.e3 > 60.) ||
        (l2.isZHTight && l2.vec.Pt() / 1.e3 > 60.)))
    return false;

  return true;
}

EL::StatusCode AnalysisReader_VHQQ::fill_jetHistos(
    std::vector<const xAOD::Jet *> signalJets,
    std::vector<const xAOD::Jet *> forwardJets) {
  if (!m_doReduceFillHistos && !m_doOnlyInputs) {
    m_histSvc->BookFillHist("NJets", 11, -0.5, 10.5, m_etree->get<int>("nJets"),
                            m_weight);
    m_histSvc->BookFillHist("NSigJets", 11, -0.5, 10.5,
                            m_etree->get<int>("nSigJets"), m_weight);
    m_histSvc->BookFillHist("NbJets", 11, -0.5, 10.5,
                            m_etree->get<int>("nbJets"), m_weight);

    m_histSvc->BookFillHist("NFwdJets", 11, -0.5, 10.5,
                            m_etree->get<int>("nFwdJets"), m_weight);
    for (const xAOD::Jet *jet : signalJets) {
      m_histSvc->BookFillHist("PtJets", 100, 0, 100, jet->pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("EtaJets", 100, -5, 5, jet->eta(), m_weight);
    }

    for (const xAOD::Jet *jet : forwardJets) {
      m_histSvc->BookFillHist("PtJets", 100, 0, 100, jet->pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("EtaJets", 100, -5, 5, jet->eta(), m_weight);
    }
  }

  if (m_doMbbMonitor) {
    for (const xAOD::Jet *jet : signalJets) {
      m_histSvc->BookFillHist("NMuonsInSigJets", 20, 0, 20,
                              Props::nrMuonInJet.get(jet), m_weight);
      if (Props::nrMuonInJet.get(jet) > 0) {
        xAOD::JetFourMom_t momOneMu = jet->jetP4("OneMu");
        TLorentzVector vecOneMu;
        vecOneMu.SetPx(momOneMu.px());
        vecOneMu.SetPy(momOneMu.py());
        vecOneMu.SetPz(momOneMu.pz());
        vecOneMu.SetE(momOneMu.e());
        if (vecOneMu.Pt() > 0) {
          m_histSvc->BookFillHist("PtOneMuInSigJets", 100, 0, 100,
                                  vecOneMu.Pt() * 0.001, m_weight);
          m_histSvc->BookFillHist("EtaOneMuInSigJets", 100, -5, 5,
                                  vecOneMu.Eta(), m_weight);
          TLorentzVector vecJet = jet->p4();
          m_histSvc->BookFillHist("DrOneMuInSigJets", 100, 0, 1,
                                  vecOneMu.DeltaR(vecJet), m_weight);
        }
      }
    }
  }

  return EL::StatusCode::SUCCESS;
}  // fill_jetHistos


EL::StatusCode AnalysisReader_VHQQ::fill_jetHistos (std::vector<const xAOD::Jet*> jets, std::vector<const xAOD::Jet*> signalJets, std::vector<const xAOD::Jet*> forwardJets) {

  m_histSvc->BookFillHist("NJets", 11, -0.5, 10.5, jets.size(), m_weight);
  m_histSvc->BookFillHist("NbJets", 11, -0.5, 10.5, signalJets.size(), m_weight);

  if (!m_doReduceFillHistos) {
    m_histSvc->BookFillHist("NFwdJets", 11, -0.5, 10.5, forwardJets.size(), m_weight);
    for (const xAOD::Jet *jet : jets) {
      m_histSvc->BookFillHist("PtJets", 100,  0, 100, jet->pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("EtaJets", 100, -5,   5, jet->eta(),    m_weight);
    }

    for (const xAOD::Jet *jet : forwardJets) {
      m_histSvc->BookFillHist("PtJets", 100,  0, 100, jet->pt() / 1e3, m_weight);
      m_histSvc->BookFillHist("EtaJets", 100, -5,   5, jet->eta(),    m_weight);
    }
  }

  return EL::StatusCode::SUCCESS;
} // fill_jetHistos



bool AnalysisReader_VHQQ::isBlindedRegion(const unsigned long int eventFlag,
                                          bool isMerged) {
  return (!m_isMC                                        // only data
          && (m_histNameSvc->get_description() == "SR")  // only SR
          && ((m_histNameSvc->get_nTag() == 2)           // 2 tag
              || ((m_histNameSvc->get_nTag() == 1) &&
                  isMerged))  // also 1 tag in merged regime
          && ((passSpecificCuts(eventFlag, {DiLeptonCuts::mHCorrResolved}) &&
               !isMerged) ||
              (passSpecificCuts(eventFlag, {DiLeptonCuts::mHCorrMerged}) &&
               isMerged))  // mBB window
  );

}  // isBlindedRegion

double AnalysisReader_VHQQ::getSMSigVPTSlice() {
  bool isSMSig = (
      // qqZllH125
      m_mcChannel == 341102 ||
      // ggZllH125
      m_mcChannel == 341094 || m_mcChannel == 341095 || m_mcChannel == 341096 ||
      // qqZvvH125
      m_mcChannel == 341101 ||
      // ggZvvH125
      m_mcChannel == 341097 || m_mcChannel == 341098 || m_mcChannel == 341099 ||
      // qqWlvH125
      m_mcChannel == 341100);
  double truthVpT = -1;
  if (!isSMSig) return truthVpT;

  for (const xAOD::TruthParticle *part : *m_truthParts) {
    if (part->status() == 62) {
      if (fabs(part->pdgId()) == 23 || fabs(part->pdgId()) == 24)  // Z or W
        truthVpT =
            TMath::Sqrt(part->px() * part->px() + part->py() * part->py());
    }
  }
  return truthVpT;
}

// for 0lep boosted analysis
void AnalysisReader_VHQQ::rescale_fatjet(double mBB, TLorentzVector &fatJet) {
  double mbb_weight = 125.0 / (mBB * 1e-3);
  fatJet *= mbb_weight;
}
EL::StatusCode AnalysisReader_VHQQ::histFinalize() {
  EL_CHECK("finalize()", AnalysisReader::histFinalize());

  Info("histFinalize()", "About to write histograms.");
  m_histFastSvc.Write(wk());
  Info("histFinalize()", "Done writing histograms.");

  return EL::StatusCode::SUCCESS;
}

// Setup the CorrsAndSysts object m_corrsAndSyst for the current event
EL::StatusCode AnalysisReader_VHQQ::SetupCorrsAndSyst(std::string currentVar,
                                                      bool isCR = false) {
  // Determine from m_histNameSvc->getFullHistName() th sample {ttbar, V+b,...}
  // and number of tags {0tag, 1tag,...} std::string EventClass =
  // m_histNameSvc->getFullHistName("mVH"); //Leave entry blank as we only want
  // sample & tag categorisation
  std::string EventClass = m_histNameSvc->get_sample();
  CAS::SysVar VarUp = CAS::Up;
  CAS::SysBin Bin = CAS::Any;

  if (m_histNameSvc->get_sample() == "ttbar" ||
      m_histNameSvc->get_sample().find("stop") != std::string::npos) {
    m_evtType = m_corrsAndSysts->GetEventType(m_histNameSvc->get_sample());
    // Allocate a enum as to whether the event is a CR or SR.
    // Tehnically this is only needed for the 2-lepton channel under the new
    // HVT/AZH PRSR analysis systematics
    if (isCR) {
      // New PRSR systematics sets enum to TopCR if 2-lepton channel event is a
      // e-mu event. Old HVT moriond 2016 nalysis will set TopCR if nAddBTags >
      // 0 or e-mu event
      m_detailevtType = m_corrsAndSysts->GetDetailEventType("TopCR");
      m_corrsAndSysts->GetSystFromName((TString)currentVar, m_Systematic, Bin,
                                       VarUp);

    } else if (!isCR) {
      // Otherwise ttbar event is a TopSR. May not be entirely true, but the
      // price you pay for a generalising code across 3 channels. But be assured
      // that the CorrsAndSysts package will assign the correct weight
      m_detailevtType = m_corrsAndSysts->GetDetailEventType("TopSR");
      m_corrsAndSysts->GetSystFromName((TString)currentVar, m_Systematic, Bin,
                                       VarUp);

    } else {
      Info("SetupCorrsAndSysts()",
           "Invalid ttbar CR definition for TTbar and Stop systematic");
      return EL::StatusCode::FAILURE;
    }

  } else if (EventClass == "W" || EventClass == "Z" || EventClass == "Wv22" ||
             EventClass == "Zv22") {
    if (EventClass == "Wv22") EventClass = "W";
    if (EventClass == "Zv22") EventClass = "Z";

    // Set event type, systematic name and void detaileventType.
    // SR vs CR determined by mBB
    if (m_physicsMeta.b1Flav == 5 && m_physicsMeta.b2Flav == 5)
      EventClass += "b";  // bb
    else if ((m_physicsMeta.b1Flav == 5 && m_physicsMeta.b2Flav == 4) ||
             (m_physicsMeta.b1Flav == 4 && m_physicsMeta.b2Flav == 5))
      EventClass += "b";  // bc
    else if (m_physicsMeta.b1Flav == 4 && m_physicsMeta.b2Flav == 4)
      EventClass += "c";  // cc
    else if (m_physicsMeta.b1Flav == 5 || m_physicsMeta.b2Flav == 5)
      EventClass += "b";  // bl
    else if (m_physicsMeta.b1Flav == 4 || m_physicsMeta.b2Flav == 4)
      EventClass += "c";  // cl
    else
      EventClass += "l";  // ll
    if (m_debug) std::cout << " Event Class = " << EventClass << std::endl;
    m_evtType = m_corrsAndSysts->GetEventType(EventClass);
    m_detailevtType = CAS::NODETAILNAME;
    m_corrsAndSysts->GetSystFromName((TString)currentVar, m_Systematic, Bin,
                                     VarUp);

  } else {
    Info("AnalysisReader_VHbb1Lep::SetupCorrsAndSysts()",
         "Invalid sample, no systematics applied.");
    return EL::StatusCode::FAILURE;
  }

  return EL::StatusCode::SUCCESS;

}  // End of SetupCorrsAndSysts()

// mVH is in GeV
void AnalysisReader_VHQQ::fillVjetsSystslep(double Mbb, float mVH) {
  // Scale factor (weight)
  float weight_MG5_She = 0.1;
  if (m_debug)
    std::cout << "<fillVjetsSystslep()>    Adding MODEL___VJets systematics"
              << std::endl;

  // Load the correct systematic
  if (m_analysisType == "0lep") {
    SetupCorrsAndSyst("SysVJets_0lep_shape_mVH");
    weight_MG5_She = m_corrsAndSysts->Get_SystematicWeight(
        m_evtType, mVH, Mbb, 0, 0, 0, 0, 0, 0, 0, 0, m_detailevtType,
        m_Systematic, CAS::Up, CAS::Any);
  } else if (m_analysisType == "1lep") {
    SetupCorrsAndSyst("SysVJets_1lep_shape_mVH");
    weight_MG5_She = m_corrsAndSysts->Get_SystematicWeight(
        m_evtType, mVH, Mbb, 0, 0, 0, 0, 0, 0, 0, 0, m_detailevtType,
        m_Systematic, CAS::Up, CAS::Any);
  } else if (m_analysisType == "2lep") {
    SetupCorrsAndSyst("SysVJets_2lep_shape_mVH");
    weight_MG5_She = m_corrsAndSysts->Get_SystematicWeight(
        m_evtType, mVH, Mbb, 0, 0, 0, 0, 0, 0, 0, 0, m_detailevtType,
        m_Systematic, CAS::Up, CAS::Any);
  }

  // Get_Systematics() return scale+1
  weight_MG5_She = weight_MG5_She - 1;
  // if(m_debug) std::cout << "<fillVjetsSystslep()>    weight_M5G5_She : " <<
  // weight_MG5_She << std::endl;
  float min = 0.1;
  if (weight_MG5_She < min) weight_MG5_She = min;
  m_weightSysts.push_back({"MODEL_Vjets_MadGraph__1up", weight_MG5_She});
}

// mVH in GeV
void AnalysisReader_VHQQ::fillTTbarSystslep(int nTag, bool isCR, float mVH) {
  std::vector<std::string> variations;
  variations.push_back("Pow_aMC");
  variations.push_back("Her_Pyt");
  variations.push_back("rHi_rLo");

  // Determine from m_histNameSvc->getFullHistName() th sample {ttbar, V+b,...}
  // and number of tags {0tag, 1tag,...}
  std::string EventClass = m_histNameSvc->getFullHistName(
      "");  // Leave entry blank as we only want sample & tag categorisation
  std::string channel;
  if (m_analysisType == "0lep") {
    channel = "_0lep";
  } else if (m_analysisType == "1lep") {
    channel = "_1lep";
  } else if (m_analysisType == "2lep") {
    channel = "_2lep";
  }

  float weight = 0;
  // Scale factor (weight)
  // Load the correct systematic
  for (std::string shapeComp : variations) {
    SetupCorrsAndSyst("SysTTbar_shape_mVH_" + shapeComp + channel, isCR);
    // Subract 1 to account for relative ratio
    weight = m_corrsAndSysts->Get_SystematicWeight(
                 m_evtType, mVH, 0, 0, 0, 0, 0, 0, 0, 0, nTag, m_detailevtType,
                 m_Systematic, CAS::Up, CAS::Any) -
             1;
    float min = 0.1;
    // cut rad in half
    //    if ( shapeComp == "rHi_rLo" ) weight = 0.5 * (weight - 1) + 1;
    if (weight < min) weight = min;

    if (shapeComp == "Pow_aMC") {
      m_weightSysts.push_back({"MODEL_TTbar_aMcAtNlo__1up", weight});
    } else if (shapeComp == "Her_Pyt") {
      m_weightSysts.push_back({"MODEL_TTbar_Herwig__1up", weight});
    } else if (shapeComp == "rHi_rLo") {
      m_weightSysts.push_back(
          {"MODEL_TTbar_rad__1up", (1 + ((weight - 1) / (weight + 1)))});
      m_weightSysts.push_back(
          {"MODEL_TTbar_rad__1down", (1 - ((weight - 1) / (weight + 1)))});
    }
  }
}  // End of fillTTbarSystslep

// Fills m_weightSysts vector with a series of modelling systematics variations:
//      Powheg+Herwig++ Vs Powheg+Pythia6
//      aMC@NLO+Herwig++ Vs Powheg+Herwig++
//      Powheg+Pythia6 radHi & radLo
// mVH in GeV
void AnalysisReader_VHQQ::fillTopSystslep_PRSR(std::string Regime, int nAddTag,
                                               float mBB, float mVH,
                                               bool isEMu) {
  bool isTTbar = m_histNameSvc->get_sample() == "ttbar";
  bool isStop = m_histNameSvc->get_sample().find("stop") != std::string::npos;

  // Vector that stores the names of the relevant TTbar variations
  std::vector<std::string> variations;
  variations.push_back("Pow_aMC");
  variations.push_back("Her_Pyt");
  variations.push_back("rHi");
  variations.push_back("rLo");
  if (m_histNameSvc->get_sample().find("stop") != std::string::npos)
    variations.push_back("DS");

  // Determine from m_histNameSvc->getFullHistName() th sample {ttbar, V+b,...}
  // and number of tags {0tag, 1tag,...}
  std::string EventClass = m_histNameSvc->getFullHistName(
      "");  // Leave entry blank as we only want sample & tag categorisation

  std::string channel;
  if (m_analysisType == "0lep") {
    channel = "_0lep";
  } else if (m_analysisType == "1lep") {
    channel = "_1lep";
  } else if (m_analysisType == "2lep") {
    channel = "_2lep";
  }

  // single top systematics only for 1-lepton channel
  if (isStop && m_analysisType != "1lep") return;

  float weight = 0;
  // Scale factor (weight)

  // CAS::SysBin SysFormat = m_doBinnedSyst ? CAS::Bin0 : CAS::Any;
  float binned = m_doBinnedSyst ? 1 : 0;

  // Load the correct systematic
  for (std::string shapeComp : variations) {
    if (isTTbar)
      SetupCorrsAndSyst(
          "SysTTbar_" + Regime + "_shape_mVH_" + shapeComp + channel, isEMu);
    else if (isStop)
      SetupCorrsAndSyst(
          "SysStop_" + Regime + "_shape_mVH_" + shapeComp + channel, isEMu);

    // std::cout << " m_currentVar : " << "SysTTbar" + Regime +
    // "_shape_mVH_"+shapeComp+channel << std::endl; Subract 1 to account for
    // relative ratio
    weight = m_corrsAndSysts->Get_SystematicWeight(
                 m_evtType, mVH, mBB, binned, 0, 0, 0, 0, 0, 0, nAddTag,
                 m_detailevtType, m_Systematic, CAS::Up, CAS::Any) -
             1;
    float min = 0.1;
    // std::cout << " Variation : " << shapeComp << std::endl;
    // std::cout << " Weight : " << weight << std::endl;
    if (weight < min) weight = min;

    if (shapeComp == "Pow_aMC") {
      if (isTTbar)
        m_weightSysts.push_back({"MODEL_TTbar_aMcAtNlo__1up", weight});
      else if (isStop)
        m_weightSysts.push_back({"MODEL_Stop_aMcAtNlo__1up", weight});
    } else if (shapeComp == "Her_Pyt") {
      if (isTTbar)
        m_weightSysts.push_back({"MODEL_TTbar_Herwig__1up", weight});
      else if (isStop)
        m_weightSysts.push_back({"MODEL_Stop_Herwig__1up", weight});
    } else if (shapeComp == "rHi") {
      if (isTTbar)
        m_weightSysts.push_back({"MODEL_TTbar_rad__1up", weight});
      else if (isStop)
        m_weightSysts.push_back({"MODEL_Stop_rad__1up", weight});
    } else if (shapeComp == "rLo") {
      if (isTTbar)
        m_weightSysts.push_back({"MODEL_TTbar_rad__1down", weight});
      else if (isStop)
        m_weightSysts.push_back({"MODEL_Stop_rad__1down", weight});
    } else if (shapeComp == "DS") {
      if (m_histNameSvc->get_sample().find("stop") != std::string::npos)
        m_weightSysts.push_back({"MODEL_Stop_DS__1up", weight});
    }
  }
}  // End of fillTTbarSystslep

// mVH is in GeV
void AnalysisReader_VHQQ::fillVjetsSystslep_PRSR(std::string Regime, double Mbb,
                                                 float mVH, int nAddBTags) {
  // Scale factor (weight)
  float weight_VJets = 0.1;
  if (m_debug)
    std::cout << "<fillVjetsSystslep()>    Adding modelling V+jets systematics"
              << std::endl;

  // Read the event flavour and form the string to assign the correct
  // CAS::SysWxy_1lep_<regime>_shape_mVH enumerator
  if (m_debug) {
    std::cout << "  ~~~~~~~~~~~~~~~  Regime: " << Regime << "  ~~~~~~~~~~~~~~~ "
              << std::endl;
    std::cout << " >> Jet 1 flavour = " << m_physicsMeta.b1Flav << std::endl;
    std::cout << " >> Jet 2 flavour = " << m_physicsMeta.b2Flav << std::endl;
  }
  std::string Flavour = "";
  std::string FlavCategory = "";
  if (m_physicsMeta.b1Flav == 5 && m_physicsMeta.b2Flav == 5) {
    Flavour = "bb";
    FlavCategory = "HF";
  } else if ((m_physicsMeta.b1Flav == 5 && m_physicsMeta.b2Flav == 4) ||
             (m_physicsMeta.b1Flav == 4 && m_physicsMeta.b2Flav == 5)) {
    Flavour = "bc";
    FlavCategory = "HF";
  } else if (m_physicsMeta.b1Flav == 4 && m_physicsMeta.b2Flav == 4) {
    Flavour = "cc";
    FlavCategory = "HF";
  } else if (m_physicsMeta.b1Flav == 5 || m_physicsMeta.b2Flav == 5) {
    Flavour = "bl";
    FlavCategory = "hl";
  } else if (m_physicsMeta.b1Flav == 4 || m_physicsMeta.b2Flav == 4) {
    Flavour = "cl";
    FlavCategory = "hl";
  } else {
    Flavour = "l";
    FlavCategory = "l";
  }
  if (m_debug) std::cout << " Event Flavour = " << Flavour << std::endl;

  // Get Sample prefix:
  std::string SamplePrefix = m_histNameSvc->get_sample();
  // Remove "v22" if present
  if (SamplePrefix.find("v22") != std::string::npos) {
    SamplePrefix.erase(SamplePrefix.find("v22"), 3);
  }

  // Form the systematic string key:
  std::string SysKey = "Sys" + SamplePrefix + Flavour + "_" + m_analysisType +
                       "_" + Regime + "_shape_mVH";
  if (m_debug) std::cout << " >> SysKey : " << SysKey << std::endl;
  if (m_debug) std::cout << " >> mVH : " << mVH << std::endl;
  if (m_debug) std::cout << " >> mBB : " << Mbb << std::endl;

  // CAS::SysBin SysFormat = m_doBinnedSyst ? CAS::Bin0 : CAS::Any;
  float SherpaSyst = m_doSherpaSyst ? 2 : 0;
  float binned = m_doBinnedSyst ? 1 : 0;

  // Load the correct systematic
  SetupCorrsAndSyst(SysKey);

  //########### Two options are available: #######################
  //############ 1) Sherpa-Vs-MG5 #########
  weight_VJets = m_corrsAndSysts->Get_SystematicWeight(
      m_evtType, mVH, Mbb, binned, 0, 0, 0, 0, 0, 0, nAddBTags, m_detailevtType,
      m_Systematic, CAS::Up, CAS::Any, m_debug);

  // Check on the analysisType & event process
  // NOTE:::      1-lepton Channel -->>  If Z-boson process then set the weight
  // to 2. This way the loaded scale factor = 1
  //             2-lepton Channel -->>  If W-boson process then set the weight
  //             to 2. This way the loaded scale factor = 1
  if ((SamplePrefix == "W" && m_analysisType == "2lep") ||
      (SamplePrefix == "Z" && m_analysisType == "1lep")) {
    weight_VJets = 2;
  }

  // Get_Systematics() return scale+1
  weight_VJets = weight_VJets - 1;
  if (m_debug)
    std::cout << "<fillVjetsSystslep_PRSR()>    weight_M5G5_She : "
              << weight_VJets << std::endl;
  float min = 0.1;
  if (weight_VJets < min) weight_VJets = min;
  std::string SystName = "MODEL_V" + FlavCategory + "Jets_MadGraph__1up";
  std::string SystName_ProcSpec =
      "MODEL_" + SamplePrefix + FlavCategory + "Jets_MadGraph__1up";
  m_weightSysts.push_back({SystName, weight_VJets});
  m_weightSysts.push_back({SystName_ProcSpec, weight_VJets});
  //##############################################################

  //############### 2) Sherpa Scale Variations ###################
  if (m_doSherpaSyst) {
    // Renormalisation '__1up'
    weight_VJets = 0.1;
    weight_VJets =
        m_corrsAndSysts->Get_SystematicWeight(
            m_evtType, mVH, Mbb, SherpaSyst, 0, 0, 0, 0, 0, 0, nAddBTags,
            CAS::Renorm, m_Systematic, CAS::Up, CAS::Any, m_debug) -
        1;
    // Check on the analysisType & event process
    // NOTE:::      1-lepton Channel -->>  If Z-boson process then set the
    // weight to 2. This way the loaded scale factor = 1
    //             2-lepton Channel -->>  If W-boson process then set the weight
    //             to 2. This way the loaded scale factor = 1
    if ((SamplePrefix == "W" && m_analysisType == "2lep") ||
        (SamplePrefix == "Z" && m_analysisType == "1lep")) {
      weight_VJets = 1;
    }
    // Check for maximum
    if (weight_VJets < min) weight_VJets = min;  // Compose the histogram name
    SystName = "MODEL_V" + FlavCategory + "Jets_SherpaRenorm__1up";
    SystName_ProcSpec =
        "MODEL_" + SamplePrefix + FlavCategory + "Jets_SherpaRenorm__1up";
    m_weightSysts.push_back({SystName, weight_VJets});
    m_weightSysts.push_back({SystName_ProcSpec, weight_VJets});

    // Renormalisation '__1down'
    weight_VJets = 0.1;
    weight_VJets =
        m_corrsAndSysts->Get_SystematicWeight(
            m_evtType, mVH, Mbb, SherpaSyst, 0, 0, 0, 0, 0, 0, nAddBTags,
            CAS::Renorm, m_Systematic, CAS::Do, CAS::Any, m_debug) -
        1;
    // Check on the analysisType & event process
    if ((SamplePrefix == "W" && m_analysisType == "2lep") ||
        (SamplePrefix == "Z" && m_analysisType == "1lep")) {
      weight_VJets = 1;
    }
    // Check for maximum
    if (weight_VJets < min) weight_VJets = min;
    // Compose the histogram name
    SystName = "MODEL_V" + FlavCategory + "Jets_SherpaRenorm__1down";
    SystName_ProcSpec =
        "MODEL_" + SamplePrefix + FlavCategory + "Jets_SherpaRenorm__1down";
    m_weightSysts.push_back({SystName, weight_VJets});
    m_weightSysts.push_back({SystName_ProcSpec, weight_VJets});

    // Factorisation '__1up'
    weight_VJets = 0.1;
    weight_VJets =
        m_corrsAndSysts->Get_SystematicWeight(
            m_evtType, mVH, Mbb, SherpaSyst, 0, 0, 0, 0, 0, 0, nAddBTags,
            CAS::Fac, m_Systematic, CAS::Up, CAS::Any, m_debug) -
        1;
    // Check on the analysisType & event process
    if ((SamplePrefix == "W" && m_analysisType == "2lep") ||
        (SamplePrefix == "Z" && m_analysisType == "1lep")) {
      weight_VJets = 1;
    }
    // Check for maximum
    if (weight_VJets < min) weight_VJets = min;
    // Compose the histogram name
    SystName = "MODEL_V" + FlavCategory + "Jets_SherpaFac__1up";
    SystName_ProcSpec =
        "MODEL_" + SamplePrefix + FlavCategory + "Jets_SherpaFac__1up";
    m_weightSysts.push_back({SystName, weight_VJets});
    m_weightSysts.push_back({SystName_ProcSpec, weight_VJets});

    // Factorisation '__1down'
    weight_VJets = 0.1;
    weight_VJets =
        m_corrsAndSysts->Get_SystematicWeight(
            m_evtType, mVH, Mbb, SherpaSyst, 0, 0, 0, 0, 0, 0, nAddBTags,
            CAS::Fac, m_Systematic, CAS::Do, CAS::Any, m_debug) -
        1;
    // Check on the analysisType & event process
    if ((SamplePrefix == "W" && m_analysisType == "2lep") ||
        (SamplePrefix == "Z" && m_analysisType == "1lep")) {
      weight_VJets = 1;
    }
    // Check for maximum
    if (weight_VJets < min) weight_VJets = min;
    // Compose the histogram name
    SystName = "MODEL_V" + FlavCategory + "Jets_SherpaFac__1down";
    SystName_ProcSpec =
        "MODEL_" + SamplePrefix + FlavCategory + "Jets_SherpaFac__1down";
    m_weightSysts.push_back({SystName, weight_VJets});
    m_weightSysts.push_back({SystName_ProcSpec, weight_VJets});

    // AlphaPDF '__1up'
    weight_VJets = 0.1;
    weight_VJets =
        m_corrsAndSysts->Get_SystematicWeight(
            m_evtType, mVH, Mbb, SherpaSyst, 0, 0, 0, 0, 0, 0, nAddBTags,
            CAS::AlphaPDF, m_Systematic, CAS::Up, CAS::Any, m_debug) -
        1;
    // Check on the analysisType & event process
    if ((SamplePrefix == "W" && m_analysisType == "2lep") ||
        (SamplePrefix == "Z" && m_analysisType == "1lep")) {
      weight_VJets = 1;
    }
    // Check for maximum
    if (weight_VJets < min) weight_VJets = min;
    // Compose the histogram name
    SystName = "MODEL_V" + FlavCategory + "Jets_SherpaAlphaPDF__1up";
    SystName_ProcSpec =
        "MODEL_" + SamplePrefix + FlavCategory + "Jets_SherpaAlphaPDF__1up";
    m_weightSysts.push_back({SystName, weight_VJets});
    m_weightSysts.push_back({SystName_ProcSpec, weight_VJets});

    // AlphaPDF '__1down'
    weight_VJets = 0.1;
    weight_VJets =
        m_corrsAndSysts->Get_SystematicWeight(
            m_evtType, mVH, Mbb, SherpaSyst, 0, 0, 0, 0, 0, 0, nAddBTags,
            CAS::AlphaPDF, m_Systematic, CAS::Do, CAS::Any, m_debug) -
        1;
    // Check on the analysisType & event process
    if ((SamplePrefix == "W" && m_analysisType == "2lep") ||
        (SamplePrefix == "Z" && m_analysisType == "1lep")) {
      weight_VJets = 1;
    }
    // Check for maximum
    if (weight_VJets < min) weight_VJets = min;
    // Compose the histogram name
    SystName = "MODEL_V" + FlavCategory + "Jets_SherpaAlphaPDF__1down";
    SystName_ProcSpec =
        "MODEL_" + SamplePrefix + FlavCategory + "Jets_SherpaAlphaPDF__1down";
    m_weightSysts.push_back({SystName, weight_VJets});
    m_weightSysts.push_back({SystName_ProcSpec, weight_VJets});
  }
  //##############################################################
}

// Void function that applies Sherpa 2.2.1 scale variations as a weighted
// systematic variation histogram
// flag "m_evtWeightVarMode" indicates different ways of applying the
// variations: m_evtWeightVarMode==-1: nominal only, no weight variations
// m_evtWeightVarMode==0:  only most important variations are computed (see
// CxAODTools/Root/EvtWeightVariations.cxx for details) m_evtWeightVarMode==1:
// full set of variations computed m_evtWeightVarMode==2:  applies
// internal-weight-variations indicated cfg file
EL::StatusCode AnalysisReader_VHQQ::apply_EvtWeights() {
  // Check if mode==-1
  if (m_evtWeightVarMode == -1) return EL::StatusCode::SUCCESS;

  // Check that the current variations isNominal
  if (m_currentVar != "Nominal") return EL::StatusCode::SUCCESS;

  // Skip if not MC
  if (!Props::isMC.get(m_eventInfo)) return EL::StatusCode::SUCCESS;

  if (m_evtWeightVarMode == 2) {
    // Check that the variations stored within csVariations vector, are present
    // within the systematic variations stored within the event weight container
    for (auto &csVar : m_csVariations) {
      // Check that the EvtWeightVariations object contains a relevant
      // systematic
      if (m_EvtWeightVar->hasVariation(csVar, m_mcChannel)) {
        // Extract the alternative weight
        float AltWeight =
            m_EvtWeightVar->getWeightVariation(m_eventInfo, csVar);
        // for ttbar PwPy8 uncertainties from internal weights
        // applies weight protection for variation weights greater than 10 times
        // the nominal weight
        if (m_EvtWeightVar->isPowhegPythia8Ttbar(m_mcChannel)) {
          apply_TTbarPwPy8VariationWeightProtection(
              AltWeight, Props::MCEventWeight.get(m_eventInfo));
        }

        // sherpa negative alternative weight protection:
        // fix the sign of alternative weight same as nominal weight
        if (m_isSherpa &&
            AltWeight * Props::MCEventWeight.get(m_eventInfo) < 0 &&
            csVar.find("PDF") == string::npos) {
          // don't change PDF weight
          Warning("AnalysisReader_VHQQ::apply_EvtWeights()",
                  "Sherpa Internal Weight protection: Different sign of %s "
                  "%f and nominal weight %f, reset the sign",
                  &csVar[0], AltWeight, Props::MCEventWeight.get(m_eventInfo));
          AltWeight = -AltWeight;
        }

        // HistSvc.cxx applies m_weightSystsVar entries as a multiplicative
        // factor Therefore insert ratio of AltWeight/NomWeight (Alternative MC
        // weight/Nominal MC weight)
        m_weightSysts.push_back(
            {csVar, AltWeight / Props::MCEventWeight.get(m_eventInfo)});

      }  // Logic End --->>> Cross-check valid Sherpa scale variation with
         // csVariations vector

    }  // Loop End --->>> csVariations config vector search end
  }    // Logic End:mode==2 end
  //---------------------------------------------------------

  //---------------------------------------------------------
  else if (m_evtWeightVarMode == 0 || m_evtWeightVarMode == 1) {
    // loop over the appropriate maps and apply all the variations
    auto intVars = m_EvtWeightVar->getListOfVariations(m_mcChannel);

    // checking if the variation list is read correctly
    if (intVars.empty() && m_EvtWeightVar->isSherpaV(m_mcChannel)) {
      Error("apply_EvtWeights()",
            "Sample is Sherpa V, but can't find appropriate map");
      return EL::StatusCode::FAILURE;
    }
    if (intVars.empty() && m_EvtWeightVar->isSherpaZmumu221(m_mcChannel)) {
      Error("apply_EvtWeights()",
            "Sample is Sherpa Zmumu, but can't find appropriate map");
      return EL::StatusCode::FAILURE;
    }
    if (intVars.empty() && m_EvtWeightVar->isSignalMiNLO(m_mcChannel)) {
      Error("apply_EvtWeights()",
            "Sample is MiNLO signal, but can't find appropriate map");
      return EL::StatusCode::FAILURE;
    }
    if (intVars.empty() && m_EvtWeightVar->isSignalGG(m_mcChannel)) {
      Error("apply_EvtWeights()",
            "Sample is GG signal, but can't find appropriate map");
      return EL::StatusCode::FAILURE;
    }
    if (intVars.empty() && m_EvtWeightVar->isPowhegPythia8Ttbar(m_mcChannel)) {
      Error("apply_EvtWeights()",
            "Sample is ttbar PowhegPythia8, but can't find appropriate map");
      return EL::StatusCode::FAILURE;
    }
    if (intVars.empty() && m_EvtWeightVar->isPowhegPythia8Stop(m_mcChannel)) {
      Error("apply_EvtWeights()",
            "Sample is single top, but can't find appropriate map");
      return EL::StatusCode::FAILURE;
    }

    for (auto &csVar : intVars) {
      // Extract the alternative weight
      float AltWeight = m_EvtWeightVar->getWeightVariation(m_eventInfo, csVar);
      // for ttbar PwPy8 uncertainties from internal weights
      // applies weight protection for variation weights greater than 10 times
      // the nominal weight
      if (m_EvtWeightVar->isPowhegPythia8Ttbar(m_mcChannel)) {
        apply_TTbarPwPy8VariationWeightProtection(
            AltWeight, Props::MCEventWeight.get(m_eventInfo));
      }

      // sherpa negative alternative weight protection:
      // fix the sign of alternative weight same as nominal weight
      if (m_isSherpa && AltWeight * Props::MCEventWeight.get(m_eventInfo) < 0 &&
          csVar.find("PDF") == string::npos) {
        // don't change PDF weight
        AltWeight = -AltWeight;
        Warning("AnalysisReader_VHQQ::apply_EvtWeights()",
                "Sherpa Internal Weight protection: Different sign of %s %f "
                "and nominal weight %f, reset the sign",
                &csVar[0], AltWeight, Props::MCEventWeight.get(m_eventInfo));
      }

      // HistSvc.cxx applies m_weightSystsVar entries as a multiplicative factor
      // Therefore insert ratio of AltWeight/NomWeight (Alternative MC
      // weight/Nominal MC weight)
      m_weightSysts.push_back(
          {csVar, AltWeight / Props::MCEventWeight.get(m_eventInfo)});
    }  // Loop End --->>> csVariations config vector search end

  }  // Logic End:mode==0||1 end
  //---------------------------------------------------------

  else {
    // No need to apply weight leave member function
    return EL::StatusCode::SUCCESS;
  }  // Logic End --->>> Check for valid Sherpa V+Jet DSID number

  return EL::StatusCode::SUCCESS;
}  // apply_EvtWeights()

EL::StatusCode AnalysisReader_VHQQ::apply_TTbarPwPy8VariationWeightProtection(
    float &variationWeight, float nominalWeight, float relativeThreshold) {
  // for ttbar PwPy8 uncertainties from internal weights
  // applies weight protection for variation weights greater than
  // relativeThreshold times the nominal weight
  if (nominalWeight == 0)
    return EL::StatusCode::FAILURE;
  else {
    if (TMath::Abs(variationWeight / nominalWeight) > relativeThreshold) {
      variationWeight = nominalWeight;
    }
  }
  return EL::StatusCode::SUCCESS;
}  // apply_TTbarPwPy8VariationWeightProtection

// void AnalysisReader_VHQQ::applySignal_EvtWeights() {
//   // Check that the current variations isNominal
//   if (m_currentVar != "Nominal") return;

//   // Skip if not MC
//   if (!Props::isMC.get(m_eventInfo)) return;

//   // Need to know sample DSID
//   int DSID = m_eventInfo->mcChannelNumber();

//   // If DSID matches MiNLO sample then proceed with systematic weight
//   // application
//   if (!m_EvtWeightVar->isSignalMiNLO(DSID)) return;

//   //---------------------------------------------------------
//   // I am not going to hardcode 100 PDF variations in a config file .... once
//   // you call this method ... all the sys will be written
//   auto sigVar = m_EvtWeightVar->getListOfVariations();
//   for (auto &csVar : sigVar) {
//     // Check that the EvtWeightVariations object contains a relevant
//     systematic if (m_EvtWeightVar->hasVariationSignal(csVar)) {
//       // Extract the alternative weight
//       float AltWeight = m_EvtWeightVar->getWeightVariation(m_eventInfo,
//       csVar);

//       // HistSvc.cxx applies m_weightSystsVar entries as a multiplicative
//       factor
//       // Therefore insert ratio of AltWeight/NomWeight (Alternative MC
//       // weight/Nominal MC weight)
//       m_weightSysts.push_back(
//           {csVar + "__1up", AltWeight /
//           Props::MCEventWeight.get(m_eventInfo)});

//     }  // Logic End --->>> Cross-check valid Sherpa scale variation with
//        // csVariations vector

//   }  // Loop End --->>> csVariations config vector search end
//   //---------------------------------------------------------

// }  // applySignal_EvtWeights()

int AnalysisReader_VHQQ::vetoDilepTtbarEvents() {
  // Return 0: don't veto, 1: veto, -1: variable doesn't exist - ERROR
  int veto = 0;

  // Check if flag exists
  if (!Props::codeTTBarDecay.exists(m_eventInfo)) {
    veto = -1;
  }

  // Veto if we have a di-lepton decay
  if (Props::codeTTBarDecay.get(m_eventInfo) >= 4 &&
      Props::codeTTBarDecay.get(m_eventInfo) <= 9) {
    veto = 1;
  }

  return veto;
}  // vetoDilepTtbarEvents

int AnalysisReader_VHQQ::vetoDilepWtEvents() {
  // Return 0: don't veto, 1: veto
  int veto = 0;

  // this is hopefully just a temporary solution until we get the info from the
  // maker as done for ttbar... for some reason it happens that there are more
  // than 2 charged leptons in the truth record deal with it as follows - thanks
  // to Paul T for the instructions
  const xAOD::TruthParticleContainer *truthParts =
      m_truthParticleReader->getObjects("Nominal");
  int nLep = 0;
  int nLep_pos = 0;
  int nLep_neg = 0;

  for (const xAOD::TruthParticle *part : *truthParts) {
    int pdgId = part->pdgId();
    if (pdgId == 11 || pdgId == 13 || pdgId == 15) nLep_pos++;
    if (pdgId == -11 || pdgId == -13 || pdgId == -15) nLep_neg++;
  }

  nLep = (nLep_pos > 0 && nLep_neg > 0)
             ? 2
             : (nLep_pos > 0 || nLep_neg > 0) ? 1 : 0;

  // Skip event if dilep
  if (nLep == 2) veto = 1;

  return veto;
}  // vetoDilepWtEvents

// combMass begin

EL::StatusCode AnalysisReader_VHQQ::combMassTmp1(bool & /*isCombMassSyst*/,
                                                 std::string &varName) {
  if (m_CxAODTag == "CxAODTag28") {
    // combined mass is done in the maker and only JER was decided to
    // reculculate
    if (varName.compare("FATJET_JER__1up") == 0) {
      varName = "Nominal";
    }
  }  // Tag28

  return EL::StatusCode::SUCCESS;
}  // combMassTmp1

EL::StatusCode AnalysisReader_VHQQ::combMassTmp2(bool & /*isCombMassSyst*/) {
  xAOD::TStore *store = xAOD::TActiveStore::store();
  std::string containerName =
      m_fatJetReader->getContainerName() + "___" + m_currentVar + "_SC";
  bool doneSC = false;
  const xAOD::JetContainer *container = nullptr;
  if (store->contains<xAOD::JetContainer>(containerName.c_str())) {
    if (store->retrieve(container, containerName.c_str()).isSuccess()) {
      doneSC = true;
    } else {
      Warning("ObjectReader::getObjects()",
              ("Failed to retrieve container '" + containerName +
               " from active TStore'!")
                  .c_str());
    }
  }
  std::pair<xAOD::JetContainer *, xAOD::ShallowAuxContainer *> fatJets_SC;
  if (doneSC) {
    fatJets_SC.first = (xAOD::JetContainer *)container;
    fatJets_SC.second = (xAOD::ShallowAuxContainer *)container->getConstStore();
  } else {
    fatJets_SC = xAOD::shallowCopyContainer(*m_fatJets);
    if (!store->record(fatJets_SC.first, containerName.c_str()).isSuccess()) {
      Warning("ObjectReader::getObjects()",
              ("Failed to record container '" + containerName +
               " to active TStore'!")
                  .c_str());
    }
    if (!store->record(fatJets_SC.second, (containerName + "Aux.").c_str())
             .isSuccess()) {
      Warning("ObjectReader::getObjects()",
              ("Failed to record aux container '" + containerName +
               " to active TStore'!")
                  .c_str());
    }
  }

  if (m_CxAODTag == "CxAODTag28") {
    // combined mass is done in the maker and only JER was decided to
    // reculculate
    for (unsigned int iJet = 0; iJet < fatJets_SC.first->size(); ++iJet) {
      xAOD::Jet *jetSC = fatJets_SC.first->at(iJet);
      if (m_currentVar.compare("FATJET_JER__1up") == 0) {
        CP_CHECK("combMassTmp2()", applySmearingJER(*jetSC), m_debug);
      }
    }
  }  // Tag28

  m_fatJets = fatJets_SC.first;

  return EL::StatusCode::SUCCESS;
}  // combMassTmp2

CP::CorrectionCode AnalysisReader_VHQQ::applySmearingJER(xAOD::Jet &jet) {
  // seed similar to JERSmearingTool
  long int seed = 1.1e+5 * std::abs(jet.phi());
  double smear = 1;
  if (m_CxAODTag == "CxAODTag26") {
    // smearing by 5%
    smear = getSmearFactor(0.05, seed);
  }
  if (m_CxAODTag == "CxAODTag28") {
    // smearing by 2%
    smear = getSmearFactor(0.02, seed);
  }
  // energy smearing: scale pt only
  xAOD::JetFourMom_t p4 = jet.jetP4();
  jet.setJetP4(xAOD::JetFourMom_t(smear * p4.Pt(), p4.Eta(), p4.Phi(), p4.M()));
  return CP::CorrectionCode::Ok;
}  // applySmearingJER

CP::CorrectionCode AnalysisReader_VHQQ::applySmearingJMR(xAOD::Jet &jet) {
  // seed similar to JERSmearingTool
  long int seed = 1.2e+5 * std::abs(jet.phi());
  // smearing by 10%
  double smear = getSmearFactor(0.10, seed);
  // mass smearing: scale mass only
  xAOD::JetFourMom_t p4 = jet.jetP4();
  jet.setJetP4(xAOD::JetFourMom_t(p4.Pt(), p4.Eta(), p4.Phi(), smear * p4.M()));
  return CP::CorrectionCode::Ok;
}  // applySmearingJMR

double AnalysisReader_VHQQ::getSmearFactor(double sigma, long int seed) {
  // Set the seed
  m_rand.SetSeed(seed);
  // Calculate the smearing factor
  double smear = -1;
  // at least 50% change to get smear > 0 => runtime ok
  while (smear <= 0) {
    smear = m_rand.Gaus(1.0, sigma);
  }
  return smear;
}  // getSmearFactor

// combMass end

// STXS begin
int AnalysisReader_VHQQ ::STXS_GetBinFromEvtInfo() {
  int stage1 = -1;
  if (!m_eventInfo) {
    Error("STXS_GetBinFromEvtInfo", "Did not find event info!");
  }
  if (Props::HTXS_Stage1_Category_pTjet30.exists(m_eventInfo)) {
    stage1 = Props::HTXS_Stage1_Category_pTjet30.get(m_eventInfo);
  }
  return stage1;
}

float AnalysisReader_VHQQ ::STXS_GetPtVFromEvtInfo() {
  float ptv = -1.;
  if (!m_eventInfo) {
    Error("STXS_GetPtVFromEvtInfo", "Did not find event info!");
  }
  if (Props::HTXS_V_pt.exists(m_eventInfo)) {
    ptv = Props::HTXS_V_pt.get(m_eventInfo) *
          1.e6;  // for EPS2017, the unit is MeV, but for ICHEP2018, it is TeV
  } else {
    Error("STXS_GetPtVFromEvtInfo()", "Couldn't find STXS ptv information");
  }
  return ptv;
}

int AnalysisReader_VHQQ ::STXS_GetNJetFromEvtInfo() {
  int njet = -1.;
  if (!m_eventInfo) {
    Error("STXS_GetNJetFromEvtInfo", "Did not find event info!");
  }
  if (Props::HTXS_Njets_pTjet30.exists(m_eventInfo)) {
    njet = Props::HTXS_Njets_pTjet30.get(m_eventInfo);
  } else {
    Error("STXS_GetNJetFromEvtInfo()",
          "Couldn't find STXS Njets_pTjet30 information");
  }
  return njet;
}

EL::StatusCode AnalysisReader_VHQQ ::STXS_ReplaceName() {
  int stage1 = STXS_GetBinFromEvtInfo();

  if (stage1 == -1) return EL::StatusCode::SUCCESS;

  std::string stage1_name = STXS_ParseBin();
  std::replace(stage1_name.begin(), stage1_name.end(), '_',
               'x');  // set the name following Paolo's proposal

  if (m_debug)
    Info("AnalysisReader_VHQQ::STXS_ReplaceName()", "The name now is: %s",
         stage1_name.c_str());

  if (m_debug) {
    std::string sample_name = m_histNameSvc->get_sample();
    Info("AnalysisReader_VHQQ::STXS_ReplaceName()",
         "Name changed from %s -> %s", sample_name.c_str(),
         stage1_name.c_str());
  }

  m_histNameSvc->set_sample(stage1_name);

  return EL::StatusCode::SUCCESS;
}

std::string AnalysisReader_VHQQ ::STXS_ParseBin() {
  int stage1 = STXS_GetBinFromEvtInfo();
  float truthPTV = STXS_GetPtVFromEvtInfo();
  int njet = STXS_GetNJetFromEvtInfo();

  int STXS_Stage = -1;
  m_config->getif<int>("STXS_Stage", STXS_Stage);

  std::string stage1_name = "";
  if (STXS_Stage < 0) {
    Warning("AnalysisReader_VHQQ::STXS_ParseBin()",
            "Unknown STXS_Stage : %d !! Exit!", STXS_Stage);
    exit(1);
  } else if (STXS_Stage < 2) {
    stage1_name =
        STXS_ParseBin(stage1, m_mcChannel, STXS_Stage, truthPTV, njet);
  } else {
    stage1_name =
        STXS_ParseBin_Stage1PP(stage1, m_mcChannel, STXS_Stage, truthPTV, njet);
  }
  return stage1_name;
}

std::string AnalysisReader_VHQQ ::STXS_ParseBin(int stage1, int DSID = 0,
                                                int baseline = -1,
                                                float ptv = -1.0,
                                                int njet = -1) {
  if ((baseline > 0) && ((ptv < 0.) || (njet < 0))) {
    Warning("AnalysisReader_VHQQ::STXS_ParseBin()",
            "Finer stage1 splitting used, but ptv or njet is not correct!");
  }

  TString stage1_name = "";
  // qq -> WH 5/6
  if (stage1 == 300)
    stage1_name = "QQ2HLNU_FWDH";
  else if (stage1 == 301) {
    if (baseline == 0)
      stage1_name = "QQ2HLNU_PTV_0_150";
    else {
      if (ptv < 75.e3)
        stage1_name = "QQ2HLNU_PTV_0_75";
      else
        stage1_name = "QQ2HLNU_PTV_75_150";
    }
  } else if (stage1 == 302)
    stage1_name = "QQ2HLNU_PTV_150_250_0J";
  else if (stage1 == 303)
    stage1_name = "QQ2HLNU_PTV_150_250_GE1J";
  else if (stage1 == 304) {
    if (baseline == 0)
      stage1_name = "QQ2HLNU_PTV_GT250";
    else {
      if (njet == 0)
        stage1_name = "QQ2HLNU_PTV_GT250_0J";
      else
        stage1_name = "QQ2HLNU_PTV_GT250_GE1J";
    }
  }
  // qq -> ZH 5/6
  else if (stage1 == 400)
    stage1_name = "QQ2HLL_FWDH";
  else if (stage1 == 401) {
    if (baseline == 0)
      stage1_name = "QQ2HLL_PTV_0_150";
    else {
      if (ptv < 75.e3)
        stage1_name = "QQ2HLL_PTV_0_75";
      else
        stage1_name = "QQ2HLL_PTV_75_150";
    }
  } else if (stage1 == 402)
    stage1_name = "QQ2HLL_PTV_150_250_0J";
  else if (stage1 == 403)
    stage1_name = "QQ2HLL_PTV_150_250_GE1J";
  else if (stage1 == 404) {
    if (baseline == 0)
      stage1_name = "QQ2HLL_PTV_GT250";
    else {
      if (njet == 0)
        stage1_name = "QQ2HLL_PTV_GT250_0J";
      else
        stage1_name = "QQ2HLL_PTV_GT250_GE1J";
    }
  }
  // gg -> ZH 4/6
  else if (stage1 == 500)
    stage1_name = "GG2HLL_FWDH";
  else if (stage1 == 501) {
    if (baseline == 0)
      stage1_name = "GG2HLL_PTV_0_150";
    else {
      if (ptv < 75.e3)
        stage1_name = "GG2HLL_PTV_0_75";
      else
        stage1_name = "GG2HLL_PTV_75_150";
    }
  } else if (stage1 == 502) {
    if (baseline == 0)
      stage1_name = "GG2HLL_PTV_GT150_0J";
    else {
      if (ptv < 250.e3)
        stage1_name = "GG2HLL_PTV_150_250_0J";
      else
        stage1_name = "GG2HLL_PTV_GT250_0J";
    }
  } else if (stage1 == 503) {
    if (baseline == 0)
      stage1_name = "GG2HLL_PTV_GT150_GE1J";
    else {
      if (ptv < 250.e3)
        stage1_name = "GG2HLL_PTV_150_250_GE1J";
      else
        stage1_name = "GG2HLL_PTV_GT250_GE1J";
    }
  }

  //////////////////////////////
  //  for the following mc,
  //  that are NUNU, we decide
  //  to split them from LL.
  //////////////////////////////
  bool do_split = false;
  if ((DSID == 345056) || (DSID == 345058)) do_split = true;
  if (do_split) {
    stage1_name.ReplaceAll("L", "NU");
    // Info("STXS_ParseBin()", "The name is changed as: %s ",
    // stage1_name.Data());
  }

  return string(stage1_name.Data());
}

std::string AnalysisReader_VHQQ ::STXS_ParseBin_Stage1PP(int stage1,
                                                         int DSID = 0,
                                                         int baseline = -1,
                                                         float ptv = -1.0,
                                                         int njet = -1) {
  if ((baseline > 0) && ((ptv < 0.) || (njet < 0))) {
    Warning("AnalysisReader_VHQQ::STXS_ParseBin_Stage1PP()",
            "Finer stage1 splitting used, but ptv or njet is not correct!");
  }

  TString stage1_name = "";
  // qq -> WH 5/6
  if (stage1 == 300 || stage1 == 301 || stage1 == 302 || stage1 == 303 ||
      stage1 == 304)  // QQ2HLNU
  {
    if (stage1 == 300)
      stage1_name = "QQ2HLNU_FWDH";
    else {
      if (ptv < 75.e3)
        stage1_name = "QQ2HLNU_PTV_0_75";
      else if (ptv < 150.e3)
        stage1_name = "QQ2HLNU_PTV_75_150";
      else if (ptv < 250.e3)
        stage1_name = "QQ2HLNU_PTV_150_250";
      else if (ptv < 400.e3)
        stage1_name = "QQ2HLNU_PTV_250_400";
      else
        stage1_name = "QQ2HLNU_PTV_GT400";

      if (njet == 0)
        stage1_name += "_0J";
      else if (njet == 1)
        stage1_name += "_1J";
      else if (njet >= 2)
        stage1_name += "_GE2J";
    }
  }

  // qq -> ZH 5/6
  else if (stage1 == 400 || stage1 == 401 || stage1 == 402 || stage1 == 403 ||
           stage1 == 404)  // QQ2HLL
  {
    if (stage1 == 400)
      stage1_name = "QQ2HLL_FWDH";
    else {
      if (ptv < 75.e3)
        stage1_name = "QQ2HLL_PTV_0_75";
      else if (ptv < 150.e3)
        stage1_name = "QQ2HLL_PTV_75_150";
      else if (ptv < 250.e3)
        stage1_name = "QQ2HLL_PTV_150_250";
      else if (ptv < 400.e3)
        stage1_name = "QQ2HLL_PTV_250_400";
      else
        stage1_name = "QQ2HLL_PTV_GT400";

      if (njet == 0)
        stage1_name += "_0J";
      else if (njet == 1)
        stage1_name += "_1J";
      else if (njet >= 2)
        stage1_name += "_GE2J";
    }
  }

  // gg -> ZH 4/6
  else if (stage1 == 500 || stage1 == 501 || stage1 == 502 ||
           stage1 == 503)  // GG2HLL
  {
    if (stage1 == 500)
      stage1_name = "GG2HLL_FWDH";
    else {
      if (ptv < 75.e3)
        stage1_name = "GG2HLL_PTV_0_75";
      else if (ptv < 150.e3)
        stage1_name = "GG2HLL_PTV_75_150";
      else if (ptv < 250.e3)
        stage1_name = "GG2HLL_PTV_150_250";
      else if (ptv < 400.e3)
        stage1_name = "GG2HLL_PTV_250_400";
      else
        stage1_name = "GG2HLL_PTV_GT400";

      if (njet == 0)
        stage1_name += "_0J";
      else if (njet == 1)
        stage1_name += "_1J";
      else if (njet >= 2)
        stage1_name += "_GE2J";
    }
  } else {
    // Info("STXS_ParseBin_Stage1PP()", "The stage bin (%i) is unknow, so the
    // name will be set as UNKNOWN", stage1); stage1_name =  "UNKNOWN";
    if ((DSID == 345053) || (DSID == 345054))
      stage1_name = "QQ2HLNU_FWDH";  // "QQ2HLNU_UNKNOWN";
    if ((DSID == 345055) || (DSID == 345056))
      stage1_name = "QQ2HLL_FWDH";  // "QQ2HLL_UNKNOWN";
    if ((DSID == 345057) || (DSID == 345058))
      stage1_name = "GG2HLL_FWDH";  // "GG2HLL_UNKNOWN";
  }

  //////////////////////////////
  //  for the following mc,
  //  that are NUNU, we decide
  //  to split them from LL.
  //////////////////////////////
  bool do_split = false;
  if ((DSID == 345056) || (DSID == 345058)) do_split = true;
  if (do_split) {
    stage1_name.ReplaceAll("L", "NU");
    // Info("STXS_ParseBin_Stage1PP()", "The name is changed as: %s ",
    // stage1_name.Data());
  }

  return string(stage1_name.Data());
}

EL::StatusCode AnalysisReader_VHQQ ::STXS_FillYields(
    HistSvc *histSvc, string sample_name, string stage1_name,
    string category_name, double weight, int baseline, bool isMerged) {
  // stage1 category
  static std::string Stage1Names_Base[24] = {
      // qq -> ZH
      "QQ2HNUNU_FWDH",
      "QQ2HNUNU_PTV_0_150",
      "QQ2HNUNU_PTV_150_250_0J",
      "QQ2HNUNU_PTV_150_250_GE1J",
      "QQ2HNUNU_PTV_GT250",
      //"QQ2HNUNU_UNKNOWN",
      // gg -> ZH
      "GG2HNUNU_FWDH",
      "GG2HNUNU_PTV_0_150",
      "GG2HNUNU_PTV_GT150_0J",
      "GG2HNUNU_PTV_GT150_GE1J",
      //"GG2HNUNU_UNKNOWN",
      // qq -> WH
      "QQ2HLNU_FWDH",
      "QQ2HLNU_PTV_0_150",
      "QQ2HLNU_PTV_150_250_0J",
      "QQ2HLNU_PTV_150_250_GE1J",
      "QQ2HLNU_PTV_GT250",
      //"QQ2HLNU_UNKNOWN",
      // qq -> ZH
      "QQ2HLL_FWDH",
      "QQ2HLL_PTV_0_150",
      "QQ2HLL_PTV_150_250_0J",
      "QQ2HLL_PTV_150_250_GE1J",
      "QQ2HLL_PTV_GT250",
      //"QQ2HLL_UNKNOWN",
      // gg -> ZH
      "GG2HLL_FWDH",
      "GG2HLL_PTV_0_150",
      "GG2HLL_PTV_GT150_0J",
      "GG2HLL_PTV_GT150_GE1J",
      //"GG2HLL_UNKNOWN",
      // unknown
      //"UNKNOWN",
      // total, always filled
      "TOTAL",
  };

  static std::string Stage1Names_Nicest[81] = {
      // qq -> ZH NUNU 16
      "QQ2HNUNU_FWDH",
      "QQ2HNUNU_PTV_0_75_0J",
      "QQ2HNUNU_PTV_0_75_1J",
      "QQ2HNUNU_PTV_0_75_GE2J",
      "QQ2HNUNU_PTV_75_150_0J",
      "QQ2HNUNU_PTV_75_150_1J",
      "QQ2HNUNU_PTV_75_150_GE2J",
      "QQ2HNUNU_PTV_150_250_0J",
      "QQ2HNUNU_PTV_150_250_1J",
      "QQ2HNUNU_PTV_150_250_GE2J",
      "QQ2HNUNU_PTV_250_400_0J",
      "QQ2HNUNU_PTV_250_400_1J",
      "QQ2HNUNU_PTV_250_400_GE2J",
      "QQ2HNUNU_PTV_GT400_0J",
      "QQ2HNUNU_PTV_GT400_1J",
      "QQ2HNUNU_PTV_GT400_GE2J",
      //"QQ2HNUNU_UNKNOWN",
      // gg -> ZH NUNU 16
      "GG2HNUNU_FWDH",
      "GG2HNUNU_PTV_0_75_0J",
      "GG2HNUNU_PTV_0_75_1J",
      "GG2HNUNU_PTV_0_75_GE2J",
      "GG2HNUNU_PTV_75_150_0J",
      "GG2HNUNU_PTV_75_150_1J",
      "GG2HNUNU_PTV_75_150_GE2J",
      "GG2HNUNU_PTV_150_250_0J",
      "GG2HNUNU_PTV_150_250_1J",
      "GG2HNUNU_PTV_150_250_GE2J",
      "GG2HNUNU_PTV_250_400_0J",
      "GG2HNUNU_PTV_250_400_1J",
      "GG2HNUNU_PTV_250_400_GE2J",
      "GG2HNUNU_PTV_GT400_0J",
      "GG2HNUNU_PTV_GT400_1J",
      "GG2HNUNU_PTV_GT400_GE2J",
      //"GG2HNUNU_UNKNOWN",
      // qq -> WH 16
      "QQ2HLNU_FWDH",
      "QQ2HLNU_PTV_0_75_0J",
      "QQ2HLNU_PTV_0_75_1J",
      "QQ2HLNU_PTV_0_75_GE2J",
      "QQ2HLNU_PTV_75_150_0J",
      "QQ2HLNU_PTV_75_150_1J",
      "QQ2HLNU_PTV_75_150_GE2J",
      "QQ2HLNU_PTV_150_250_0J",
      "QQ2HLNU_PTV_150_250_1J",
      "QQ2HLNU_PTV_150_250_GE2J",
      "QQ2HLNU_PTV_250_400_0J",
      "QQ2HLNU_PTV_250_400_1J",
      "QQ2HLNU_PTV_250_400_GE2J",
      "QQ2HLNU_PTV_GT400_0J",
      "QQ2HLNU_PTV_GT400_1J",
      "QQ2HLNU_PTV_GT400_GE2J",
      //"QQ2HLNU_UNKNOWN",
      // qq -> ZH LL 16
      "QQ2HLL_FWDH",
      "QQ2HLL_PTV_0_75_0J",
      "QQ2HLL_PTV_0_75_1J",
      "QQ2HLL_PTV_0_75_GE2J",
      "QQ2HLL_PTV_75_150_0J",
      "QQ2HLL_PTV_75_150_1J",
      "QQ2HLL_PTV_75_150_GE2J",
      "QQ2HLL_PTV_150_250_0J",
      "QQ2HLL_PTV_150_250_1J",
      "QQ2HLL_PTV_150_250_GE2J",
      "QQ2HLL_PTV_250_400_0J",
      "QQ2HLL_PTV_250_400_1J",
      "QQ2HLL_PTV_250_400_GE2J",
      "QQ2HLL_PTV_GT400_0J",
      "QQ2HLL_PTV_GT400_1J",
      "QQ2HLL_PTV_GT400_GE2J",
      //"QQ2HLL_UNKNOWN",
      // gg -> ZH LL 16
      "GG2HLL_FWDH",
      "GG2HLL_PTV_0_75_0J",
      "GG2HLL_PTV_0_75_1J",
      "GG2HLL_PTV_0_75_GE2J",
      "GG2HLL_PTV_75_150_0J",
      "GG2HLL_PTV_75_150_1J",
      "GG2HLL_PTV_75_150_GE2J",
      "GG2HLL_PTV_150_250_0J",
      "GG2HLL_PTV_150_250_1J",
      "GG2HLL_PTV_150_250_GE2J",
      "GG2HLL_PTV_250_400_0J",
      "GG2HLL_PTV_250_400_1J",
      "GG2HLL_PTV_250_400_GE2J",
      "GG2HLL_PTV_GT400_0J",
      "GG2HLL_PTV_GT400_1J",
      "GG2HLL_PTV_GT400_GE2J",
      //"GG2HLL_UNKNOWN",
      // unknown
      //"UNKNOWN",
      // total, always filled
      "TOTAL",
  };

  static std::string Stage1Names_Fine[36] = {
      // qq -> ZH NUNU 7
      "QQ2HNUNU_FWDH",
      "QQ2HNUNU_PTV_0_75",
      "QQ2HNUNU_PTV_75_150",
      "QQ2HNUNU_PTV_150_250_0J",
      "QQ2HNUNU_PTV_150_250_GE1J",
      "QQ2HNUNU_PTV_GT250_0J",
      "QQ2HNUNU_PTV_GT250_GE1J",
      //"QQ2HNUNU_UNKNOWN",
      // gg -> ZH NUNU 7
      "GG2HNUNU_FWDH",
      "GG2HNUNU_PTV_0_75",
      "GG2HNUNU_PTV_75_150",
      "GG2HNUNU_PTV_150_250_0J",
      "GG2HNUNU_PTV_150_250_GE1J",
      "GG2HNUNU_PTV_GT250_0J",
      "GG2HNUNU_PTV_GT250_GE1J",
      //"GG2HNUNU_UNKNOWN",
      // qq -> WH 7
      "QQ2HLNU_FWDH",
      "QQ2HLNU_PTV_0_75",
      "QQ2HLNU_PTV_75_150",
      "QQ2HLNU_PTV_150_250_0J",
      "QQ2HLNU_PTV_150_250_GE1J",
      "QQ2HLNU_PTV_GT250_0J",
      "QQ2HLNU_PTV_GT250_GE1J",
      //"QQ2HLNU_UNKNOWN",
      // qq -> ZH LL 7
      "QQ2HLL_FWDH",
      "QQ2HLL_PTV_0_75",
      "QQ2HLL_PTV_75_150",
      "QQ2HLL_PTV_150_250_0J",
      "QQ2HLL_PTV_150_250_GE1J",
      "QQ2HLL_PTV_GT250_0J",
      "QQ2HLL_PTV_GT250_GE1J",
      //"QQ2HLL_UNKNOWN",
      // gg -> ZH LL 7
      "GG2HLL_FWDH",
      "GG2HLL_PTV_0_75",
      "GG2HLL_PTV_75_150",
      "GG2HLL_PTV_150_250_0J",
      "GG2HLL_PTV_150_250_GE1J",
      "GG2HLL_PTV_GT250_0J",
      "GG2HLL_PTV_GT250_GE1J",
      //"GG2HLL_UNKNOWN",
      // unknown
      //"UNKNOWN",
      // total, always filled
      "TOTAL",
  };

  std::string *RecoCategories;

  static vector<std::string> RecoCategories_Resolved = {
      // 0 lep
      "0lep,2tag2jet,150<PTV,SR",
      "0lep,2tag3jet,150<PTV,SR",
      // 1 lep
      "1lep,2tag2jet,150<PTV,WhfCR",
      "1lep,2tag3jet,150<PTV,WhfCR",
      "1lep,2tag2jet,150<PTV,WhfSR",
      "1lep,2tag3jet,150<PTV,WhfSR",
      // 2 lep
      //"2lep,2tag2jet,75<PTV<150,CR",  "2lep,2tag2jet,150<PTV,CR",
      //"2lep,2tag3pjet,75<PTV<150,CR", "2lep,2tag3pjet,150<PTV,CR",
      "2lep,2tag2jet,75<PTV<150,SR",
      "2lep,2tag2jet,150<PTV,SR",
      "2lep,2tag3pjet,75<PTV<150,SR",
      "2lep,2tag3pjet,150<PTV,SR",
  };

  static vector<std::string> RecoCategories_Merged = {
      // 0 lep
      "0lep,2tag0jet,150<PTV<250,SR",
      "0lep,2tag1pjet,150<PTV<250,SR",
      "0lep,2tag0jet,250<PTV<400,SR",
      "0lep,2tag1pjet,250<PTV<400,SR",
      "0lep,2tag0jet,400<PTV,SR",
      "0lep,2tag1pjet,400<PTV,SR",
      // 1 lep
      "1lep,2tag0jet,150<PTV<250,SR",
      "1lep,2tag1pjet,150<PTV<250,SR",
      "1lep,2tag0jet,250<PTV<400,SR",
      "1lep,2tag1pjet,250<PTV<400,SR",
      "1lep,2tag0jet,400<PTV,SR",
      "1lep,2tag1pjet,400<PTV,SR",
      // 2 lep
      "2lep,2tag0pjet,150<PTV<250,SR",
      "2lep,2tag0pjet,250<PTV<400,SR",
      "2lep,2tag0pjet,400<PTV,SR",
  };

  int nRecoCat = -1;
  if (isMerged) {
    RecoCategories = &RecoCategories_Merged[0];
    nRecoCat = RecoCategories_Merged.size();
  } else {
    RecoCategories = &RecoCategories_Resolved[0];
    nRecoCat = RecoCategories_Resolved.size();
  }

  if (m_debug) Info("STXS_FillYields()", "nRecoCat '%d'.", nRecoCat);

  // the following one need to be update!
  if (baseline == 0) {
    if (m_debug)
      Info("STXS_FillYields()", "length(Stage1Names_Base) '%lu'.",
           length(Stage1Names_Base));
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSBase",
                             length(Stage1Names_Base), Stage1Names_Base,
                             nRecoCat, RecoCategories, stage1_name,
                             category_name, weight);
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSBase",
                             length(Stage1Names_Base), Stage1Names_Base,
                             nRecoCat, RecoCategories, "TOTAL", category_name,
                             weight);
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSBase_NoWeight",
                             length(Stage1Names_Base), Stage1Names_Base,
                             nRecoCat, RecoCategories, stage1_name,
                             category_name, 1.);
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSBase_NoWeight",
                             length(Stage1Names_Base), Stage1Names_Base,
                             nRecoCat, RecoCategories, "TOTAL", category_name,
                             1.);
  } else if (baseline == 1) {
    if (m_debug)
      Info("STXS_FillYields()", "length(Stage1Names_Fine) '%lu'.",
           length(Stage1Names_Fine));
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSFine",
                             length(Stage1Names_Fine), Stage1Names_Fine,
                             nRecoCat, RecoCategories, stage1_name,
                             category_name, weight);
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSFine",
                             length(Stage1Names_Fine), Stage1Names_Fine,
                             nRecoCat, RecoCategories, "TOTAL", category_name,
                             weight);
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSFine_NoWeight",
                             length(Stage1Names_Fine), Stage1Names_Fine,
                             nRecoCat, RecoCategories, stage1_name,
                             category_name, 1.);
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSFine_NoWeight",
                             length(Stage1Names_Fine), Stage1Names_Fine,
                             nRecoCat, RecoCategories, "TOTAL", category_name,
                             1.);
  } else if (baseline == 2) {
    if (m_debug)
      Info("STXS_FillYields()", "length(Stage1Names_Nicest) '%lu'.",
           length(Stage1Names_Nicest));
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSNicest",
                             length(Stage1Names_Nicest), Stage1Names_Nicest,
                             nRecoCat, RecoCategories, stage1_name,
                             category_name, weight);
    histSvc->BookFillCutHist(sample_name + "_Stage1Bin_RecoMapEPSNicest",
                             length(Stage1Names_Nicest), Stage1Names_Nicest,
                             nRecoCat, RecoCategories, "TOTAL", category_name,
                             weight);
    histSvc->BookFillCutHist(
        sample_name + "_Stage1Bin_RecoMapEPSNicest_NoWeight",
        length(Stage1Names_Nicest), Stage1Names_Nicest, nRecoCat,
        RecoCategories, stage1_name, category_name, 1.);
    histSvc->BookFillCutHist(
        sample_name + "_Stage1Bin_RecoMapEPSNicest_NoWeight",
        length(Stage1Names_Nicest), Stage1Names_Nicest, nRecoCat,
        RecoCategories, "TOTAL", category_name, 1.);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ ::STXS_FillYields() {
  string sample_name = m_histNameSvc->get_sample();
  int njet = m_histNameSvc->get_nJet();
  float pTV = m_histNameSvc->get_pTV();
  int stage1 = STXS_GetBinFromEvtInfo();

  if (stage1 == -1)
    return EL::StatusCode::SUCCESS;  // only for the STXS samples
  if ((m_analysisType == "2lep") && (m_histNameSvc->get_description() != "SR"))
    return EL::StatusCode::SUCCESS;  // for 2lepton, we don't fill topemuCR

  int STXS_Stage = -1;
  m_config->getif<int>("STXS_Stage", STXS_Stage);

  std::string stage1_name = STXS_ParseBin();
  string str_njets = std::to_string(njet);
  // else if (m_nJet == -4 ) m_name += "4pjet"; due to the special setting in
  // 2lep channel, -3 = 3p already since ICHEP2018
  if ((m_analysisType == "2lep") &&
      ((njet == 3) || (njet == -4) || (njet == -3)))
    str_njets = "3p";

  std::string category_name = "";
  category_name += m_analysisType + ",";
  // category_name += "2tag"+std::to_string(njet)+"jet,";
  category_name += "2tag" + str_njets + "jet,";

  // decide the PtV info
  if (pTV < 75e3) {
    return EL::StatusCode::SUCCESS;
  } else if (pTV < 150e3) {
    if ((m_analysisType != "2lep"))
      return EL::StatusCode::SUCCESS;
    else
      category_name += "75<PTV<150,";
  } else {
    category_name += "150<PTV,";
  }

  category_name += m_histNameSvc->get_description();

  if (m_debug)
    Info("STXS_FillYields()", "Category name is '%s'.", category_name.c_str());

  STXS_FillYields(m_histSvc, sample_name, stage1_name, category_name, m_weight,
                  STXS_Stage);

  return EL::StatusCode::SUCCESS;
}
// STXS end

// STXS for boosted
EL::StatusCode AnalysisReader_VHQQ ::STXS_FillYields_Boosted() {
  string sample_name = m_histNameSvc->get_sample();
  int njet = m_histNameSvc->get_nJet();
  float pTV = m_histNameSvc->get_pTV();
  int stage1 = STXS_GetBinFromEvtInfo();
  int nAddBjet = m_histNameSvc->get_nBTagTrackJetUnmatched();

  if (stage1 == -1)
    return EL::StatusCode::SUCCESS;  // only for the STXS samples

  // don't look into CRs
  if ((m_analysisType == "2lep") && (m_histNameSvc->get_description() != "SR"))
    return EL::StatusCode::SUCCESS;  // for 2lepton, we don't fill topemuCR
  if (!(m_analysisType == "2lep") && (nAddBjet != 0))
    return EL::StatusCode::SUCCESS;  // don't fill ttbar CR in 0L and 1L

  int STXS_Stage = -1;
  m_config->getif<int>("STXS_Stage", STXS_Stage);

  std::string stage1_name = STXS_ParseBin();
  string str_njets = std::to_string(njet);
  if (njet < 0) {
    str_njets = "0p";
  } else if (njet >= 1) {
    str_njets = "1p";
  }

  std::string category_name = "";
  category_name += m_analysisType + ",";
  category_name += "2tag" + str_njets + "jet,";

  // decide the PtV info
  if (pTV < 150e3) {
    return EL::StatusCode::SUCCESS;
  } else if (pTV > 150e3 && pTV < 250e3) {
    category_name += "150<PTV<250,";
  } else if (pTV > 250e3 && pTV < 400e3) {
    category_name += "250<PTV<400,";
  } else if (pTV > 400e3) {
    category_name += "400<PTV,";
  }

  category_name += m_histNameSvc->get_description();

  if (m_debug)
    Info("STXS_FillYields()", "Category name is '%s'.", category_name.c_str());

  STXS_FillYields(m_histSvc, sample_name, stage1_name, category_name, m_weight,
                  STXS_Stage, true);

  return EL::StatusCode::SUCCESS;
}
// STXS end

/// initialise pointers to zero
EL::StatusCode AnalysisReader_VHQQ::initialiseObjectBranches() {
  // jets
  vIn_jet_pt = 0;
  vIn_jet_eta = 0;
  vIn_jet_phi = 0;
  vIn_jet_e = 0;
  vIn_jet_mv2c10 = 0;
  vIn_jet_mv2c100 = 0;
  vIn_jet_mv2cl100 = 0;
  vIn_jet_DL1_pb = 0;
  vIn_jet_DL1_pc = 0;
  vIn_jet_DL1_pu = 0;
  vIn_jet_jvt = 0;
  vIn_jet_truthflav = 0;

  // electrons
  vIn_el_pt = 0;
  vIn_el_eta = 0;
  vIn_el_phi = 0;
  vIn_el_e = 0;
  vIn_el_charge = 0;

  // muons
  vIn_mu_pt = 0;
  vIn_mu_eta = 0;
  vIn_mu_phi = 0;
  vIn_mu_e = 0;
  vIn_mu_charge = 0;

  // taus
  vIn_tau_pt = 0;
  vIn_tau_eta = 0;
  vIn_tau_phi = 0;
  vIn_tau_e = 0;
  vIn_tau_charge = 0;
  return EL::StatusCode::SUCCESS;
}

/// set pointers to new obejcts
EL::StatusCode AnalysisReader_VHQQ::defineObjectBranches() {
  // jets
  vIn_jet_pt = new std::vector<float>();
  vIn_jet_eta = new std::vector<float>();
  vIn_jet_phi = new std::vector<float>();
  vIn_jet_e = new std::vector<float>();
  vIn_jet_mv2c10 = new std::vector<float>();
  vIn_jet_mv2c100 = new std::vector<float>();
  vIn_jet_mv2cl100 = new std::vector<float>();
  vIn_jet_DL1_pb = new std::vector<float>();
  vIn_jet_DL1_pc = new std::vector<float>();
  vIn_jet_DL1_pu = new std::vector<float>();
  vIn_jet_jvt = new std::vector<float>();
  vIn_jet_truthflav = new std::vector<int>();

  // electrons
  vIn_el_pt = new std::vector<float>();
  vIn_el_eta = new std::vector<float>();
  vIn_el_phi = new std::vector<float>();
  vIn_el_e = new std::vector<float>();
  vIn_el_charge = new std::vector<float>();

  // muons
  vIn_mu_pt = new std::vector<float>();
  vIn_mu_eta = new std::vector<float>();
  vIn_mu_phi = new std::vector<float>();
  vIn_mu_e = new std::vector<float>();
  vIn_mu_charge = new std::vector<float>();

  // taus
  vIn_tau_pt = new std::vector<float>();
  vIn_tau_eta = new std::vector<float>();
  vIn_tau_phi = new std::vector<float>();
  vIn_tau_e = new std::vector<float>();
  vIn_tau_charge = new std::vector<float>();

  return EL::StatusCode::SUCCESS;
}

/// Clear all branches to empty them, set met to dummy values
EL::StatusCode AnalysisReader_VHQQ::resetObjectBranches() {
  // jets
  vIn_jet_pt->clear();
  vIn_jet_eta->clear();
  vIn_jet_phi->clear();
  vIn_jet_e->clear();
  vIn_jet_mv2c10->clear();
  vIn_jet_mv2c100->clear();
  vIn_jet_mv2cl100->clear();
  vIn_jet_DL1_pb->clear();
  vIn_jet_DL1_pc->clear();
  vIn_jet_DL1_pu->clear();
  vIn_jet_jvt->clear();
  vIn_jet_truthflav->clear();

  // electrons
  vIn_el_pt->clear();
  vIn_el_eta->clear();
  vIn_el_phi->clear();
  vIn_el_e->clear();
  vIn_el_charge->clear();

  // muons
  vIn_mu_pt->clear();
  vIn_mu_eta->clear();
  vIn_mu_phi->clear();
  vIn_mu_e->clear();
  vIn_mu_charge->clear();

  // taus
  vIn_tau_pt->clear();
  vIn_tau_eta->clear();
  vIn_tau_phi->clear();
  vIn_tau_e->clear();
  vIn_tau_charge->clear();

  // met
  m_met_met = -99;
  m_met_phi = -99;

  return EL::StatusCode::SUCCESS;
}

/// Delete branches and set pointers to zero
EL::StatusCode AnalysisReader_VHQQ::finaliseObjectBranches() {
  // jets
  delete vIn_jet_pt;
  vIn_jet_pt = 0;
  delete vIn_jet_eta;
  vIn_jet_eta = 0;
  delete vIn_jet_phi;
  vIn_jet_phi = 0;
  delete vIn_jet_e;
  vIn_jet_e = 0;
  delete vIn_jet_mv2c10;
  vIn_jet_mv2c10 = 0;
  delete vIn_jet_mv2c100;
  vIn_jet_mv2c100 = 0;
  delete vIn_jet_mv2cl100;
  vIn_jet_mv2cl100 = 0;
  delete vIn_jet_DL1_pb;
  vIn_jet_DL1_pb = 0;
  delete vIn_jet_DL1_pc;
  vIn_jet_DL1_pc = 0;
  delete vIn_jet_DL1_pu;
  vIn_jet_DL1_pu = 0;
  delete vIn_jet_jvt;
  vIn_jet_jvt = 0;
  delete vIn_jet_truthflav;
  vIn_jet_truthflav = 0;

  // electrons
  delete vIn_el_pt;
  vIn_el_pt = 0;
  delete vIn_el_eta;
  vIn_el_eta = 0;
  delete vIn_el_phi;
  vIn_el_phi = 0;
  delete vIn_el_e;
  vIn_el_e = 0;
  delete vIn_el_charge;
  vIn_el_charge = 0;

  // muons
  delete vIn_mu_pt;
  vIn_mu_pt = 0;
  delete vIn_mu_eta;
  vIn_mu_eta = 0;
  delete vIn_mu_phi;
  vIn_mu_phi = 0;
  delete vIn_mu_e;
  vIn_mu_e = 0;
  delete vIn_mu_charge;
  vIn_mu_charge = 0;

  // taus
  delete vIn_tau_pt;
  vIn_tau_pt = 0;
  delete vIn_tau_eta;
  vIn_tau_eta = 0;
  delete vIn_tau_phi;
  vIn_tau_phi = 0;
  delete vIn_tau_e;
  vIn_tau_e = 0;
  delete vIn_tau_charge;
  vIn_tau_charge = 0;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ::fillObjectBranches(
    const std::vector<const xAOD::Jet *> &jets,
    // const std::vector<const xAOD::Electron*> &electrons,
    // const std::vector<const xAOD::Muon*> &muons,
    // const std::vector<const xAOD::TauJet*> &taus,
    // const xAOD::JetContainer *jets,
    const xAOD::ElectronContainer *electrons, const xAOD::MuonContainer *muons,
    const xAOD::TauJetContainer *taus, const xAOD::MissingET *met) {
  /// Make sure all vectors are reset
  EL_CHECK("AnalysisReader_VHQQ::fillObjectBranches()", resetObjectBranches());

  /// Fill all information into the vectors for saving
  for (const auto *const jet : jets) {
    vIn_jet_pt->push_back(jet->pt());
    vIn_jet_eta->push_back(jet->eta());
    vIn_jet_phi->push_back(jet->phi());
    vIn_jet_e->push_back(jet->e());
    vIn_jet_mv2c10->push_back(jet->auxdata<float>("MV2c10"));
    // vIn_jet_mv2c100->push_back(jet->auxdata<float>("MV2c100"));
    // vIn_jet_mv2cl100->push_back(jet->auxdata<float>("MV2cl100"));
    vIn_jet_DL1_pb->push_back(jet->auxdata<double>("DL1_pb"));
    vIn_jet_DL1_pc->push_back(jet->auxdata<double>("DL1_pc"));
    vIn_jet_DL1_pu->push_back(jet->auxdata<double>("DL1_pu"));
    // vIn_jet_jvt->push_back(jet->auxdata<float>("Jvt"));
    if (m_isMC) {
      vIn_jet_truthflav->push_back(Props::HadronConeExclTruthLabelID.get(jet));
    }
  }

  for (const auto *const el : *electrons) {
    vIn_el_pt->push_back(el->pt());
    vIn_el_eta->push_back(el->eta());
    vIn_el_phi->push_back(el->phi());
    vIn_el_e->push_back(el->e());
    vIn_el_charge->push_back(el->charge());
  }

  for (const auto *const mu : *muons) {
    vIn_mu_pt->push_back(mu->pt());
    vIn_mu_eta->push_back(mu->eta());
    vIn_mu_phi->push_back(mu->phi());
    vIn_mu_e->push_back(mu->e());
    vIn_mu_charge->push_back(mu->charge());
  }

  for (const auto *const tau : *taus) {
    vIn_tau_pt->push_back(tau->pt());
    vIn_tau_eta->push_back(tau->eta());
    vIn_tau_phi->push_back(tau->phi());
    vIn_tau_e->push_back(tau->e());
    vIn_tau_charge->push_back(tau->charge());
  }

  m_met_met = met->met();
  m_met_phi = met->phi();

  /// Add branches to the easytree, done on the fly
  m_etree->SetBranchAndValue<std::vector<float>>("jet_pt", *vIn_jet_pt,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_eta", *vIn_jet_eta,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_phi", *vIn_jet_phi,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_e", *vIn_jet_e,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_mv2c10", *vIn_jet_mv2c10,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>(
      "jet_mv2c100", *vIn_jet_mv2c100, std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>(
      "jet_mv2cl100", *vIn_jet_mv2cl100, std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_DL1_pb", *vIn_jet_DL1_pb,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_DL1_pc", *vIn_jet_DL1_pc,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_DL1_pu", *vIn_jet_DL1_pu,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("jet_jvt", *vIn_jet_jvt,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<int>>(
      "jet_truthflav", *vIn_jet_truthflav, std::vector<int>());

  // electrons
  m_etree->SetBranchAndValue<std::vector<float>>("el_pt", *vIn_el_pt,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("el_eta", *vIn_el_eta,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("el_phi", *vIn_el_phi,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("el_e", *vIn_el_e,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("el_charge", *vIn_el_charge,
                                                 std::vector<float>());

  // muons
  m_etree->SetBranchAndValue<std::vector<float>>("mu_pt", *vIn_mu_pt,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("mu_eta", *vIn_mu_eta,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("mu_phi", *vIn_mu_phi,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("mu_e", *vIn_mu_e,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("mu_charge", *vIn_mu_charge,
                                                 std::vector<float>());

  // taus
  m_etree->SetBranchAndValue<std::vector<float>>("tau_pt", *vIn_tau_pt,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("tau_eta", *vIn_tau_eta,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("tau_phi", *vIn_tau_phi,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("tau_e", *vIn_tau_e,
                                                 std::vector<float>());
  m_etree->SetBranchAndValue<std::vector<float>>("tau_charge", *vIn_tau_charge,
                                                 std::vector<float>());

  // met
  m_etree->SetBranchAndValue<float>("met_met", m_met_met, -99);
  m_etree->SetBranchAndValue<float>("met_phi", m_met_phi, -99);

  return EL::StatusCode::SUCCESS;
}

float AnalysisReader_VHQQ::getCTagSF(int flav, int syst) {
  if (syst == 0) return 1;

  if (flav == 4) {
    if (syst == -1)
      return 0.7;
    else if (syst == 1)
      return 1.3;
    else
      return 1;
  } else if (flav == 0) {
    if (syst == -2)
      return 0.7;
    else if (syst == 2)
      return 1.3;
    else
      return 1;
  } else if (flav == 5) {
    if (syst == -3)
      return 0.9;
    else if (syst == 3)
      return 1.1;
    else
      return 1;
  } else
    return 1;
}

float AnalysisReader_VHQQ::getCTagSFWeight(const xAOD::Jet *jet, int syst) {
  int isTagged = BTagProps::isTagged.get(jet);
  float eff = BTagProps::eff.get(jet);
  int flav = Props::HadronConeExclTruthLabelID.get(jet);

  float SF = getCTagSF(flav, syst);
  if (isTagged)
    return eff * SF;
  else
    return 1 - eff * SF;
}

EL::StatusCode AnalysisReader_VHQQ::emulateCTagSystematics(
    std::vector<const xAOD::Jet *> jetsForBTagging) {
  float nomWeight = 1;
  float weight = 1;

  // nominal
  for (auto jet : jetsForBTagging) {
    nomWeight *= getCTagSFWeight(jet, 0);
  }

  // loop over systematics
  for (auto syst : m_cTagSystVec) {
    for (auto jet : jetsForBTagging) {
      weight *= getCTagSFWeight(jet, syst.second);
    }
    m_weightSysts.push_back({syst.first, (float)(weight / nomWeight)});
  }

  return EL::StatusCode::SUCCESS;
}

std::tuple<TLorentzVector, TLorentzVector> AnalysisReader_VHQQ::calculateFSR(
    TLorentzVector B1, TLorentzVector B2, TLorentzVector J3,
    double ptv_for_FSR) {
  double a, b, c;
  if (m_analysisType == "2lep") {
    // 2-Lep
    a = 0.8722;
    b = 3.4351;
    c = 0.0077;
  } else {
    // 0-/1-Lep
    a = 0;
    b = 1.95;
    c = 0.0013;
  }
  bool FSRCut = B1.DeltaR(J3) + B2.DeltaR(J3) < a + b * exp(-c * ptv_for_FSR);

  if (FSRCut) {
    m_hasFSR = 1;
    if (B1.DeltaR(J3) < B2.DeltaR(J3)) {
      // Correct the leading bjet
      B1.SetPtEtaPhiM((B1 + J3).Pt(), (B1 + J3).Eta(), (B1 + J3).Phi(),
                      (B1 + J3).M());
    } else {
      // Correct the subleading bjet
      B2.SetPtEtaPhiM((B2 + J3).Pt(), (B2 + J3).Eta(), (B2 + J3).Phi(),
                      (B2 + J3).M());
    }
  }

  return std::make_tuple(B1, B2);
}

void AnalysisReader_VHQQ::migrateCategoryFSR(
    std::vector<const xAOD::Jet *> &signalJets,
    std::vector<const xAOD::Jet *> &forwardJets) {
  if (m_physicsMeta.nForwardJet == 1) {
    forwardJets.erase(forwardJets.begin());
  } else {
    signalJets.erase(signalJets.begin() + 2);
  }
  setevent_nJets(signalJets, forwardJets);  // reset the number of jets
}

EL::StatusCode AnalysisReader_VHQQ::compute_TT_min_deltaR_scale_factor(
    const string &filename, const int &flav_j1, const int &flav_j2,
    const float &min_dR, float &scale_factor) {
  // build the string to search 0 = light-jets, 4 = c-jets, 5 = b-jets

  static std::map<int, std::array<double, 5>> reweighting_params;
  static bool initialized = false;

  if (!initialized) {
    std::string path_fit = getenv("WorkDir_DIR");
    ifstream myfile(path_fit + "/data/CxAODReader_VHbb/" + filename);
    if (!myfile.is_open()) return EL::StatusCode::FAILURE;

    std::string line, data, label;
    std::array<double, 5> params;
    while (std::getline(myfile, line)) {
      std::stringstream linestream(line);
      std::getline(linestream, data, ',');
      label = data;
      // std::cout << label[0] << "   ---- " << label[1] << std::endl;
      int i = 0;
      while (std::getline(linestream, data, ',')) {
        params[i] = std::stod(data);
        ++i;
      }
      reweighting_params[std::stoi(label)] = params;
    }
    myfile.close();
    initialized = true;
  }

  // compute scale_factor
  float scale_factor_tmp(1);
  if (flav_j1 != 15 && flav_j2 != 15) {
    std::array<double, 5> p = reweighting_params.at(10 * flav_j1 + flav_j2);
    scale_factor_tmp = p[0] * min_dR * min_dR * min_dR * min_dR +
                       p[1] * min_dR * min_dR * min_dR +
                       p[2] * min_dR * min_dR + p[3] * min_dR + p[4];
  }

  // update scale_factor if correction not 0
  if (scale_factor_tmp > 0) {
    scale_factor = scale_factor_tmp;
  } else {
    scale_factor = 1;
  }

  return EL::StatusCode::SUCCESS;

}  // compute_TT_min_deltaR_scale_factor

EL::StatusCode AnalysisReader_VHQQ::compute_TT_min_deltaR_correction(
    std::vector<const xAOD::Jet *> selectedJets,
    std::map<std::string, std::map<int, float>> &btag_TT_weights,
    const bool &do_btag_Syst) {
  // apply dR correction on every sample excepted ttbar and stop
  if (m_histNameSvc->get_sample() == "ttbar" ||
      m_histNameSvc->get_sample().find("stop") != std::string::npos) {
    btag_TT_weights = m_bTagTool->compute_TruthTag_EventWeight_VHcc(
        selectedJets, do_btag_Syst);
  } else {
    // calculate minimum delta R between tagged jet and closest jet + their
    // flavours
    float min_dR_jet1(999);
    int flav_1_j1 = Props::HadronConeExclTruthLabelID.get(selectedJets.at(0));
    int flav_2_j1(-1);
    TLorentzVector vec_jet1 = selectedJets.at(0)->p4();

    for (unsigned int jet_i = 1; jet_i < selectedJets.size(); jet_i++) {
      TLorentzVector vec_jeti = selectedJets.at(jet_i)->p4();
      float dR = vec_jet1.DeltaR(vec_jeti);
      if (min_dR_jet1 > dR) {
        min_dR_jet1 = dR;
        flav_2_j1 =
            Props::HadronConeExclTruthLabelID.get(selectedJets.at(jet_i));
      }
    }

    float min_dR_jet2(999);
    int flav_1_j2 = Props::HadronConeExclTruthLabelID.get(selectedJets.at(1));
    int flav_2_j2(-1);
    TLorentzVector vec_jet2 = selectedJets.at(1)->p4();
    for (unsigned int jet_i = 0; jet_i < selectedJets.size(); jet_i++) {
      if (jet_i != 1) {
        TLorentzVector vec_jeti = selectedJets.at(jet_i)->p4();
        float dR = vec_jet2.DeltaR(vec_jeti);
        if (min_dR_jet2 > dR) {
          min_dR_jet2 = dR;
          flav_2_j2 =
              Props::HadronConeExclTruthLabelID.get(selectedJets.at(jet_i));
        }
      }
    }

    // compute correction factors to be applied on efficiencies
    std::string filename = "Zjets_fitparameters.txt";
    float c_jet1(1);
    float c_jet2(1);

    EL_CHECK("AnalysisReader_VHQQ::compute_TT_min_deltaR_correction",
             compute_TT_min_deltaR_scale_factor(filename, flav_1_j1, flav_2_j1,
                                                min_dR_jet1, c_jet1));

    EL_CHECK("AnalysisReader_VHQQ::compute_TT_min_deltaR_correction",

             compute_TT_min_deltaR_scale_factor(filename, flav_1_j2, flav_2_j2,
                                                min_dR_jet2, c_jet2));

    btag_TT_weights = m_bTagTool->compute_TruthTag_EventWeight_VHcc(
        selectedJets, do_btag_Syst, c_jet1, c_jet2);
  }

  return EL::StatusCode::SUCCESS;
}
