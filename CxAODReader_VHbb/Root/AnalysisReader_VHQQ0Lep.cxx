#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader/OSTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb0lep.h"
#include "CxAODTools_VHbb/VHbb0lepEvtSelection.h"

#include <CxAODReader_VHbb/AnalysisReader_VHQQ0Lep.h>
#include <TMVA/Reader.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHQQ0Lep::AnalysisReader_VHQQ0Lep()
    : AnalysisReader_VHQQ(), m_doMergeModel(false) {}

AnalysisReader_VHQQ0Lep::~AnalysisReader_VHQQ0Lep() {}

EL::StatusCode AnalysisReader_VHQQ0Lep::initializeTools() {
  EL_CHECK("initializeTools()", AnalysisReader_VHQQ::initializeTools());
  // BDT Reader tool
  // ----------------
  m_mvaVHbbFullRun2oldDefault = new MVAApplication_TMVA(
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
      "0L/",
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
  }

  m_triggerTool = new TriggerTool_VHbb0lep(*m_config);
  EL_CHECK("AnalysisReader_VHQQ0Lep::initializeTools()",
           m_triggerTool->initialize());
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ0Lep::initializeSelection() {
  EL_CHECK("AnalysisReader_VHQQ0Lep::initializeSelection ()",
           AnalysisReader_VHQQ::initializeSelection());

  Info("AnalysisReader_VHQQ0Lep::initializeSelection()",
       "Initializing 0 lepton selection");
  m_eventSelection = new VHbb0lepEvtSelection(m_config);
  m_fillFunction = std::bind(&AnalysisReader_VHQQ0Lep::run_0Lep_analysis, this);

  m_config->getif<bool>("mbbwindow", m_mbbwindow);
  m_config->getif<bool>("met_JETcorrection", m_met_JETcorrection);
  m_config->getif<bool>("met_MZcorrection", m_met_MZcorrection);
  m_config->getif<bool>("doCutflow", m_doCutflow);
  m_config->getif<bool>("doMergeModel", m_doMergeModel);

  std::string modelType = "";
  m_config->getif<std::string>("modelType", modelType);
  if (modelType == "AZh")
    m_model = Model::AZh;
  else if (modelType == "HVT")
    m_model = Model::HVT;
  else if (modelType == "CUT")
    m_model = Model::CUT;
  else if (modelType == "MVA")
    m_model = Model::MVA;
  else
    m_model = Model::undefined;

  return EL::StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////
///                *** MVA ***                         ///
//////////////////////////////////////////////////////////
void AnalysisReader_VHQQ0Lep::setVHCandidate(VHCand& resolvedVH,
                                             VHCand& mergedVH,
                                             TLorentzVector& metVec, Jet& j1,
                                             Jet& j2, Jet& fj1) {
  // get rescaled jets
  rescale_jets((j1.vec_corr + j2.vec_corr).M(), j1.vec_resc, j2.vec_resc);
  rescale_fatjet(fj1.vec_corr.M(), fj1.vec_resc);

  // build dijets removing longitudinal component and mass
  TLorentzVector dijet_nlc, dijet_nlc_corr, dijet_nlc_resc;
  dijet_nlc.SetPxPyPzE(j1.vec.Px() + j2.vec.Px(), j1.vec.Py() + j2.vec.Py(), 0,
                       sqrt(pow(j1.vec.Px() + j2.vec.Px(), 2) +
                            pow(j1.vec.Py() + j2.vec.Py(), 2)));
  dijet_nlc_corr.SetPxPyPzE(j1.vec_corr.Px() + j2.vec_corr.Px(),
                            j1.vec_corr.Py() + j2.vec_corr.Py(), 0,
                            sqrt(pow(j1.vec_corr.Px() + j2.vec_corr.Px(), 2) +
                                 pow(j1.vec_corr.Py() + j2.vec_corr.Py(), 2)));
  dijet_nlc_resc.SetPxPyPzE(j1.vec_resc.Px() + j2.vec_resc.Px(),
                            j1.vec_resc.Py() + j2.vec_resc.Py(), 0,
                            sqrt(pow(j1.vec_resc.Px() + j2.vec_resc.Px(), 2) +
                                 pow(j1.vec_resc.Py() + j2.vec_resc.Py(), 2)));

  Jet fj1_nlc;
  fj1_nlc.vec.SetPxPyPzE(
      fj1.vec.Px(), fj1.vec.Py(), 0,
      sqrt(fj1.vec.Px() * fj1.vec.Px() + fj1.vec.Py() * fj1.vec.Py() +
           fj1.vec.M() * fj1.vec.M()));
  fj1_nlc.vec_corr.SetPxPyPzE(fj1.vec_corr.Px(), fj1.vec_corr.Py(), 0,
                              sqrt(fj1.vec_corr.Px() * fj1.vec_corr.Px() +
                                   fj1.vec_corr.Py() * fj1.vec_corr.Py() +
                                   fj1.vec_corr.M() * fj1.vec_corr.M()));
  fj1_nlc.vec_resc.SetPxPyPzE(
      fj1.vec_resc.Px(), fj1.vec_resc.Py(), 0,
      sqrt(fj1.vec_resc.Px() * fj1.vec_resc.Px() +
           fj1.vec_resc.Py() *
               fj1.vec_resc.Py()));  // + fj1.vec_resc.M() * fj1.vec_resc.M() ??

  // apply corrections to met
  double mz = 91187.6;
  double met_x = metVec.Px();
  double met_y = metVec.Py();
  double met_e = metVec.E();
  TLorentzVector metVecCorr;
  if (m_met_JETcorrection == true) {
    met_x =
        met_x + j1.vec.Px() + j2.vec.Px() - j1.vec_corr.Px() - j2.vec_corr.Px();
    met_y =
        met_y + j1.vec.Py() + j2.vec.Py() - j1.vec_corr.Py() - j2.vec_corr.Py();
    met_e = sqrt(met_x * met_x + met_y * met_y);
  }
  if (m_met_MZcorrection == true) {
    met_e = sqrt(met_x * met_x + met_y * met_y + mz * mz);
  }
  metVecCorr.SetPxPyPzE(met_x, met_y, 0, met_e);

  resolvedVH.vec = metVec + dijet_nlc;
  resolvedVH.vec_corr = metVec + dijet_nlc_corr;
  resolvedVH.vec_resc = metVecCorr + dijet_nlc_resc;

  // define VH vandidate for merged
  mergedVH.vec = metVec + fj1_nlc.vec;
  mergedVH.vec_corr = metVec + fj1_nlc.vec_corr;
  mergedVH.vec_resc =
      metVecCorr +
      fj1_nlc.vec_resc;  // rescaled jets and corrected met are used only in SR

}  // setVHCandidate

////////////////////////////////////////////////////////////
///         ** Check if pass 0Lep trigger **             ///
////////////////////////////////////////////////////////////
bool AnalysisReader_VHQQ0Lep::pass0LepTrigger(double& triggerSF_nominal,
                                              ResultVHbb0lep selectionResult) {
  const xAOD::MissingET* met = selectionResult.met;

  CP::SystematicSet sysSet;
  EL_CHECK("pass0LepTrigger", m_triggerTool->applySystematicVariation(sysSet));
  m_triggerTool->setEventInfo(m_eventInfo, m_randomRunNumber);
  m_triggerTool->setMET(met);
  bool triggerDec = m_triggerTool->getDecisionAndScaleFactor(triggerSF_nominal);
  if (!triggerDec) return false;

  // handle systematics
  if (m_isMC && (m_currentVar == "Nominal")) {
    for (size_t i = 0; i < m_triggerSystList.size(); i++) {
      // get decision + weight
      double triggerSF = 1.;

      // syst of SF(sumpt) only for 3 jet case
      if (m_triggerSystList.at(i).find("METTrigSumpt") != std::string::npos &&
          !((m_physicsMeta.nSigJet >= 3) ||
            (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1)))
        continue;

      CP::SystematicSet sysSet(m_triggerSystList.at(i));
      m_triggerTool->applySystematicVariation(sysSet);
      triggerDec = m_triggerTool->getDecisionAndScaleFactor(triggerSF);

      if (triggerSF_nominal > 0)
        m_weightSysts.push_back(
            {m_triggerSystList.at(i), (float)(triggerSF / triggerSF_nominal)});
      else
        Error("run_0Lep_analysis()",
              "Nominal trigger SF=0!, The systematics will not be generated.");
    }
  }

  return true;
}  // pass0LepTrigger

////////////////////////////////////////////////////////////
///                   ** Check Mbb window **             ///
////////////////////////////////////////////////////////////
void AnalysisReader_VHQQ0Lep::checkMbbWindow(const TLorentzVector& HVecResolved,
                                             const TLorentzVector& HVecMerged,
                                             bool& passMHResolved,
                                             bool& passMHMerged,
                                             bool& passMHResolvedSideBand,
                                             bool& passMHMergedSideBand) {
  passMHResolved = false;
  passMHMerged = false;
  passMHResolvedSideBand = false;
  passMHMergedSideBand = false;

  if ((HVecResolved.M() > 110e3) && (HVecResolved.M() < 140e3))
    passMHResolved = true;

  if ((HVecMerged.M() > 75e3) && (HVecMerged.M() < 145e3)) passMHMerged = true;

  if ((HVecResolved.M() <= 110e3) && (HVecResolved.M() >= 50e3)) {
    m_physicsMeta.mbbSideBandResolved =
        PhysicsMetadata::MbbSideBandResolved::Low;
    passMHResolvedSideBand = true;
  } else if ((HVecResolved.M() >= 140e3) && (HVecResolved.M() <= 200e3)) {
    m_physicsMeta.mbbSideBandResolved =
        PhysicsMetadata::MbbSideBandResolved::High;
    passMHResolvedSideBand = true;
  }
  if ((HVecMerged.M() <= 75e3) && (HVecMerged.M() >= 50e3)) {
    m_physicsMeta.mbbSideBandMerged = PhysicsMetadata::MbbSideBandMerged::Low;
    passMHMergedSideBand = true;
  } else if ((HVecMerged.M() >= 145e3) && (HVecMerged.M() <= 200e3)) {
    m_physicsMeta.mbbSideBandMerged = PhysicsMetadata::MbbSideBandMerged::High;
    passMHMergedSideBand = true;
  }

}  // checkMbbWindow

////////////////////////////////////////////////////////////
///             ** Select analysis regime **             ///
////////////////////////////////////////////////////////////
void AnalysisReader_VHQQ0Lep::selectRegime(
    unsigned long int eventFlag,
    std::vector<unsigned long int> cuts_SR_resolved,
    std::vector<unsigned long int> cuts_SR_merged,
    std::vector<unsigned long int> cuts_common_resolved,
    std::vector<unsigned long int> cuts_common_merged) {
  bool has2pSigJets =
      passSpecificCuts(eventFlag, {ZeroLeptonCuts::AtLeast2SigJets});
  bool has1pFatJets =
      passSpecificCuts(eventFlag, {ZeroLeptonCuts::AtLeast1FatJet});

  bool isMETGT500 = passSpecificCuts(eventFlag, {ZeroLeptonCuts::METGT500});

  bool inMassWindowResolved =
      passSpecificCuts(eventFlag, {ZeroLeptonCuts::MHCorrResolvedSideBand}) ||
      passSpecificCuts(eventFlag, {ZeroLeptonCuts::MHCorrResolved});
  bool inMassWindowMerged =
      passSpecificCuts(eventFlag, {ZeroLeptonCuts::MHCorrMergedSideBand}) ||
      passSpecificCuts(eventFlag, {ZeroLeptonCuts::MHCorrMerged});

  if (m_analysisStrategy == "Resolved") {
    if (has2pSigJets) m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "Merged") {
    if (has1pFatJets) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "RecyclePtV") {
    // Give priority to merged above 500 GeV
    if (isMETGT500) {
      if (passSpecificCuts(eventFlag, cuts_common_merged) && inMassWindowMerged)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      else if (passSpecificCuts(eventFlag, cuts_common_resolved) &&
               inMassWindowResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    } else {
      if (passSpecificCuts(eventFlag, cuts_common_resolved) &&
          inMassWindowResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      else if (passSpecificCuts(eventFlag, cuts_common_merged) &&
               inMassWindowMerged)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
  } else if (m_analysisStrategy == "SimpleMerge500") {
    // Only merged above 500 GeV
    if (isMETGT500) {
      if (has1pFatJets) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    } else {
      if (has2pSigJets)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
  } else if (m_analysisStrategy == "PriorityResolved") {
    if (passSpecificCuts(eventFlag, cuts_common_resolved) &&
        inMassWindowResolved)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passSpecificCuts(eventFlag, cuts_common_merged) &&
             inMassWindowMerged)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "PriorityResolvedSR") {
    // 1. check if event can go into resolved SR
    if (passSpecificCuts(eventFlag, cuts_SR_resolved)) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
    // 2. check if event can go into merged SR
    else if (passSpecificCuts(eventFlag, cuts_SR_merged)) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
    // 3. check if it goes into any of the resolved regions
    else if (passSpecificCuts(eventFlag, cuts_common_resolved) &&
             passSpecificCuts(eventFlag,
                              {ZeroLeptonCuts::MHCorrResolvedSideBand})) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
    // 4. check if it goes into any of the merged regions
    else if (passSpecificCuts(eventFlag, cuts_common_merged) &&
             passSpecificCuts(eventFlag,
                              {ZeroLeptonCuts::MHCorrMergedSideBand})) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
  } else if (m_analysisStrategy == "PriorityMergedSR") {
    // 1. check if event can go into merged SR
    if (passSpecificCuts(eventFlag, cuts_SR_merged)) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
    // 2. check if event can go into resolved SR
    else if (passSpecificCuts(eventFlag, cuts_SR_resolved)) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
    // 3. check if it goes into any of the merged regions
    else if (passSpecificCuts(eventFlag, cuts_common_merged) &&
             passSpecificCuts(eventFlag,
                              {ZeroLeptonCuts::MHCorrMergedSideBand})) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
    // 4. check if it goes into any of the resolved regions
    else if (passSpecificCuts(eventFlag, cuts_common_resolved) &&
             passSpecificCuts(eventFlag,
                              {ZeroLeptonCuts::MHCorrResolvedSideBand})) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
  } else if (m_analysisStrategy == "RecyclePtVSR") {
    if (!isMETGT500) {
      // 1. check if event can go into resolved SR
      if (passSpecificCuts(eventFlag, cuts_SR_resolved)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 2. check if event can go into merged SR
      else if (passSpecificCuts(eventFlag, cuts_SR_merged)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 3. check if event has >= 2 small jets
      else if (passSpecificCuts(eventFlag, cuts_common_resolved) &&
               passSpecificCuts(eventFlag,
                                {ZeroLeptonCuts::MHCorrResolvedSideBand})) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 4. check if event has >= 1 fat-jet
      else if (passSpecificCuts(eventFlag, cuts_common_merged) &&
               passSpecificCuts(eventFlag,
                                {ZeroLeptonCuts::MHCorrMergedSideBand})) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
    } else {
      // 1. check if event can go into merged SR
      if (passSpecificCuts(eventFlag, cuts_SR_merged)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 2. check if event can go into resolved SR
      else if (passSpecificCuts(eventFlag, cuts_SR_resolved)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 3. check if event has >= 1 fat-jet
      else if (passSpecificCuts(eventFlag, cuts_common_merged) &&
               passSpecificCuts(eventFlag,
                                {ZeroLeptonCuts::MHCorrMergedSideBand})) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 4. check if event has >= 2 small jets
      else if (passSpecificCuts(eventFlag, cuts_common_resolved) &&
               passSpecificCuts(eventFlag,
                                {ZeroLeptonCuts::MHCorrResolvedSideBand})) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
    }
  }
}  // selectRegime

std::vector<const xAOD::Jet*> AnalysisReader_VHQQ0Lep::set_JetsForAntiQCD(
    std::vector<const xAOD::Jet*> fatJets,
    std::vector<const xAOD::Jet*> signalJets,
    std::vector<const xAOD::Jet*> forwardJets, unsigned int pTthreshold) {
  std::vector<const xAOD::Jet*> JetsForAntiQCD;
  const xAOD::Jet* fatjet;
  if (fatJets.size() >= 1) {
    fatjet = fatJets.at(0);
    for (auto signaljet : signalJets) {
      if (signaljet->p4().DeltaR(fatjet->p4()) > 1.0) {
        if (signaljet->p4().Pt() / 1.e3 >= pTthreshold) {
          JetsForAntiQCD.push_back(signaljet);
        }
      }
    }
    for (auto forwardjet : forwardJets) {
      if (forwardjet->p4().DeltaR(fatjet->p4()) > 1.0) {
        if (forwardjet->p4().Pt() / 1.e3 >= pTthreshold) {
          JetsForAntiQCD.push_back(forwardjet);
        }
      }
    }
  }
  return JetsForAntiQCD;
}  // set_JetsForAntiQCD

////////////////////////////////////////////////////////////
///        ** Fill cut flow for 0Lep selection **        ///
////////////////////////////////////////////////////////////
EL::StatusCode AnalysisReader_VHQQ0Lep::fill0LepCutFlow(
    unsigned long int eventFlag) {
  std::string dir = "CutFlow/Nominal/";
  bool isVHres =
      (m_model == Model::AZh || m_model == Model::HVT) ? true : false;
  std::vector<unsigned long int> incrementCuts;

  // VH resonance resolved cuts
  //-------------------------------------------------------------------------
  static std::string cutsVHresResolvedNames[28] = {
      "All",        "Trigger",     "Nlepton",
      "Tauveto",    "MET",         "dphiMETMPT",
      "Njets",      "Nsignaljets", "mindphiMETjets",
      "Sumpt",      "MPT",         "pTB1",
      "dphiB1B2",   "dphiMETB1B2", "mbbwindow_corr",
      "MET500",     "0tag_23jet",  "0tag_4jet",
      "0tag_5pjet", "1tag_23jet",  "1tag_4jet",
      "1tag_5pjet", "2tag_23jet",  "2tag_4jet",
      "2tag_5pjet", "3ptag_23jet", "3ptag_4jet",
      "3ptag_5pjet"};

  std::vector<unsigned long int> cutsVHresResolved = {
      ZeroLeptonCuts::AllCxAOD,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::TauVeto,
      ZeroLeptonCuts::METResolved,
      ZeroLeptonCuts::DPhiMETMPT,
      ZeroLeptonCuts::AtLeast2Jets,
      ZeroLeptonCuts::AtLeast2SigJets,
      ZeroLeptonCuts::MinDPhiMETJetsResolved,
      ZeroLeptonCuts::SumJetPt,
      ZeroLeptonCuts::MinMPTBJet0or1Resolved,
      ZeroLeptonCuts::LeadJetPt,
      ZeroLeptonCuts::DPhiBB,
      ZeroLeptonCuts::DPhiMETDijetResolved,
      ZeroLeptonCuts::MHCorrResolved,
      ZeroLeptonCuts::METLT500};

  // VH resonance merged cuts
  //-------------------------------------------------------------------------
  static std::string cutsVHresMergedNames[17] = {"All",
                                                 "Trigger",
                                                 "Nlepton",
                                                 "Tauveto",
                                                 "MET",
                                                 "MPT",
                                                 "1goodfatjet",
                                                 "mindphiMETjets",
                                                 "dphiMETFJ",
                                                 "dphiMETMPT",
                                                 "mfjwindow_corr",
                                                 "MET500",
                                                 "2match_tag",
                                                 "0tag_1pfat0pjet",
                                                 "1tag_1pfat0pjet",
                                                 "2tag_1pfat0pjet",
                                                 "3ptag_1pfat0pjet"};

  std::vector<unsigned long int> cutsVHresMerged = {
      ZeroLeptonCuts::AllCxAOD,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::TauVeto,
      ZeroLeptonCuts::METMerged,
      ZeroLeptonCuts::MinMPTMerged,
      ZeroLeptonCuts::AtLeast1FatJet,
      ZeroLeptonCuts::MinDPhiMETJetsMerged,
      ZeroLeptonCuts::DPhiMETDijetMerged,
      ZeroLeptonCuts::DPhiMETMPT,
      ZeroLeptonCuts::MHCorrMerged,
      ZeroLeptonCuts::METGT500};

  // SM resolved cuts
  //-------------------------------------------------------------------------
  static std::string cutsSMResolvedNames[28] = {
      "All",         "Trigger",    "Nlepton",        "MET",
      "dphiMETMPT",  "Njets",      "Nsignaljets",    "mindphiMETjets",
      "Sumpt",       "MPT",        "pTB1",           "dphiB1B2",
      "dphiMETB1B2", "dRB1B2",     "mbbwindow_corr", "MET500",
      "0tag_2jet",   "0tag_3jet",  "0tag_4pjet",     "1tag_2jet",
      "1tag_3jet",   "1tag_4pjet", "2tag_2jet",      "2tag_3jet",
      "2tag_4pjet",  "3ptag_2jet", "3ptag_3jet",     "3ptag_4pjet"};

  std::vector<unsigned long int> cutsSMResolved = {
      ZeroLeptonCuts::AllCxAOD,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::METResolved,
      ZeroLeptonCuts::DPhiMETMPT,
      ZeroLeptonCuts::AtLeast2Jets,
      ZeroLeptonCuts::AtLeast2SigJets,
      ZeroLeptonCuts::MinDPhiMETJetsResolved,
      ZeroLeptonCuts::SumJetPt,
      ZeroLeptonCuts::MinMPTBJet0or1Resolved,
      ZeroLeptonCuts::LeadJetPt,
      ZeroLeptonCuts::DPhiBB,
      ZeroLeptonCuts::DPhiMETDijetResolved};
  std::vector<unsigned long int> cuts_SM = {ZeroLeptonCuts::DRB1B2};
  if (m_model == Model::CUT) {
    cutsSMResolved.insert(cutsSMResolved.end(), cuts_SM.begin(), cuts_SM.end());
  }

  // SM merged cuts
  //-------------------------------------------------------------------------
  static std::string cutsSMMergedNames[15] = {"All",
                                              "Trigger",
                                              "Nlepton",
                                              "MET",
                                              "1goodfatjet",
                                              "passVRJetOR",
                                              "mindphiMETjets",
                                              "dphiMETFJ",
                                              "dphiMETMPT",
                                              "min2MatchedTJ",
                                              "0tag_1pfat0pjet",
                                              "1tag_1pfat0pjet",
                                              "2tag_1pfat0pjet",
                                              "3ptag_1pfat0pjet"};

  std::vector<unsigned long int> cutsSMMerged = {
      ZeroLeptonCuts::AllCxAOD,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::METMerged,
      ZeroLeptonCuts::AtLeast1FatJet,
      ZeroLeptonCuts::passVRJetOR,
      ZeroLeptonCuts::MinDPhiMETJetsMerged,
      ZeroLeptonCuts::DPhiMETDijetMerged,
      ZeroLeptonCuts::DPhiMETMPT};

  //-------------------------------------------------------------------------

  //** VH resonance cut flow **, both AZh and HVT
  //--------------------------------------------------------
  if (isVHres && (passSpecificCuts(eventFlag, {ZeroLeptonCuts::METLT500}))) {
    // Resolved
    for (unsigned long int i = 0; i < cutsVHresResolved.size(); ++i) {
      incrementCuts.push_back(cutsVHresResolved.at(i));

      if (!passSpecificCuts(eventFlag, incrementCuts)) break;
      std::string label = cutsVHresResolvedNames[i];

      m_histSvc->BookFillCutHist(dir + "CutsVHresResolved",
                                 length(cutsVHresResolvedNames),
                                 cutsVHresResolvedNames, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsVHresResolvedNoWeight",
                                 length(cutsVHresResolvedNames),
                                 cutsVHresResolvedNames, label, 1.);
    }

    incrementCuts.clear();

    if (passSpecificCuts(eventFlag, cutsVHresResolved)) {
      // Decide in which bin the event has to go based on ntags, njets
      int bin = 16;

      if (m_physicsMeta.nTags == 0)
        bin += 0;
      else if (m_physicsMeta.nTags == 1)
        bin += 3;
      else if (m_physicsMeta.nTags == 2)
        bin += 6;
      else if (m_physicsMeta.nTags > 2)
        bin += 9;

      if (m_physicsMeta.nJets <= 3) bin += 0;
      if (m_physicsMeta.nJets == 4) bin += 1;
      if (m_physicsMeta.nJets >= 5) bin += 2;

      std::string label = cutsVHresResolvedNames[bin];
      m_histSvc->BookFillCutHist(dir + "CutsVHresResolved",
                                 length(cutsVHresResolvedNames),
                                 cutsVHresResolvedNames, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsVHresResolvedNoWeight",
                                 length(cutsVHresResolvedNames),
                                 cutsVHresResolvedNames, label, 1.);
    }

  }  // isVHres
  if (passSpecificCuts(eventFlag, {ZeroLeptonCuts::METGT500}) &&
      isVHres) {  // for cutflow comparison with the old code, to be removed?

    //** VH resonance cut flow **, both AZh and HVT
    //--------------------------------------------------------

    // Merged
    for (unsigned long int i = 0; i < cutsVHresMerged.size(); ++i) {
      incrementCuts.push_back(cutsVHresMerged.at(i));

      if (!passSpecificCuts(eventFlag, incrementCuts)) break;

      std::string label = cutsVHresMergedNames[i];
      m_histSvc->BookFillCutHist(dir + "CutsVHresMerged",
                                 length(cutsVHresMergedNames),
                                 cutsVHresMergedNames, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsVHresMergedNoWeight",
                                 length(cutsVHresMergedNames),
                                 cutsVHresMergedNames, label, 1.);
    }

    incrementCuts.push_back(ZeroLeptonCuts::AtLeast1MatchedTrackJet);

    if (passSpecificCuts(eventFlag, incrementCuts)) {
      std::string label = cutsVHresMergedNames[12];
      m_histSvc->BookFillCutHist(dir + "CutsVHresMerged",
                                 length(cutsVHresMergedNames),
                                 cutsVHresMergedNames, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsVHresMergedNoWeight",
                                 length(cutsVHresMergedNames),
                                 cutsVHresMergedNames, label, 1.);
    }
    if (passSpecificCuts(eventFlag, incrementCuts)) {
      // Decide in which bin the event has to go based on ntags, njets
      int bin = 13;

      if (m_physicsMeta.nTagsInFJ == 0)
        bin += 0;
      else if (m_physicsMeta.nTagsInFJ == 1)
        bin += 1;
      else if (m_physicsMeta.nTagsInFJ == 2)
        bin += 2;
      else if (m_physicsMeta.nTagsInFJ >= 3)
        bin += 3;

      std::string label = cutsVHresMergedNames[bin];
      m_histSvc->BookFillCutHist(dir + "CutsVHresMerged",
                                 length(cutsVHresMergedNames),
                                 cutsVHresMergedNames, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsVHresMergedNoWeight",
                                 length(cutsVHresMergedNames),
                                 cutsVHresMergedNames, label, 1.);
    }
    incrementCuts.clear();
  }  // METGT500 && VHres

  // ** SM cut flow ** (resolved)
  //--------------------------------------------------------
  if (!isVHres && m_analysisStrategy == "Resolved") {
    for (unsigned long int i = 0; i < cutsSMResolved.size(); ++i) {
      incrementCuts.push_back(cutsSMResolved.at(i));

      if (!passSpecificCuts(eventFlag, incrementCuts)) break;
      std::string label = cutsSMResolvedNames[i];

      m_histSvc->BookFillCutHist(dir + "CutsSM", length(cutsSMResolvedNames),
                                 cutsSMResolvedNames, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsSMNoWeight",
                                 length(cutsSMResolvedNames),
                                 cutsSMResolvedNames, label, 1.);
    }

    incrementCuts.clear();

    if (passSpecificCuts(eventFlag, cutsSMResolved)) {
      // Decide in which bin the event has to go based on ntags, njets
      int bin = 17;

      if (m_physicsMeta.nTags == 0)
        bin += 0;
      else if (m_physicsMeta.nTags == 1)
        bin += 3;
      else if (m_physicsMeta.nTags == 2)
        bin += 6;
      else if (m_physicsMeta.nTags > 2)
        bin += 9;

      if (m_physicsMeta.nJets == 2) bin += 0;
      if (m_physicsMeta.nJets == 3) bin += 1;
      if (m_physicsMeta.nJets >= 4) bin += 2;

      std::string label = cutsSMResolvedNames[bin];
      m_histSvc->BookFillCutHist(dir + "CutsSM", length(cutsSMResolvedNames),
                                 cutsSMResolvedNames, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsSMNoWeight",
                                 length(cutsSMResolvedNames),
                                 cutsSMResolvedNames, label, 1.);
    }

  }  // ** SM cut flow ** (merged) ------------------------------
  else if (!isVHres && m_analysisStrategy == "Merged") {
    for (unsigned long int i = 0; i < cutsSMMerged.size(); ++i) {
      incrementCuts.push_back(cutsSMMerged.at(i));

      if (!passSpecificCuts(eventFlag, incrementCuts)) break;

      std::string label = cutsSMMergedNames[i];
      m_histSvc->BookFillCutHist(dir + "CutsSMMerged",
                                 length(cutsSMMergedNames), cutsSMMergedNames,
                                 label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsSMMergedNoWeight",
                                 length(cutsSMMergedNames), cutsSMMergedNames,
                                 label, 1.);
    }

    incrementCuts.push_back(ZeroLeptonCuts::AtLeast2MatchedTrackJets);

    if (passSpecificCuts(eventFlag, incrementCuts)) {
      std::string label = cutsVHresMergedNames[10];
      m_histSvc->BookFillCutHist(dir + "CutsSMMerged",
                                 length(cutsSMMergedNames), cutsSMMergedNames,
                                 label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsSMMergedNoWeight",
                                 length(cutsSMMergedNames), cutsSMMergedNames,
                                 label, 1.);
    }
    if (passSpecificCuts(eventFlag, incrementCuts)) {
      // Decide in which bin the event has to go based on ntags, njets
      int bin = 11;

      if (m_physicsMeta.nTagsInFJ == 0)
        bin += 0;
      else if (m_physicsMeta.nTagsInFJ == 1)
        bin += 1;
      else if (m_physicsMeta.nTagsInFJ == 2)
        bin += 2;
      else if (m_physicsMeta.nTagsInFJ >= 3)
        bin += 3;

      std::string label = cutsVHresMergedNames[bin];
      m_histSvc->BookFillCutHist(dir + "CutsSMMerged",
                                 length(cutsSMMergedNames), cutsSMMergedNames,
                                 label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsSMMergedNoWeight",
                                 length(cutsSMMergedNames), cutsSMMergedNames,
                                 label, 1.);
    }
    incrementCuts.clear();
  }

  return EL::StatusCode::SUCCESS;
}  // fill0LepCutFlow

////////////////////////////////////////////////////////////
///         ** Fill MVA tree (resolved)**                ///
////////////////////////////////////////////////////////////
EL::StatusCode AnalysisReader_VHQQ0Lep::fillMVATreeVHbbResolved0Lep(
    TLorentzVector& b1, TLorentzVector& b2, TLorentzVector& j3,
    const std::vector<const xAOD::Jet*>& selectedJets,
    const std::vector<const xAOD::Jet*>& signalJets,
    const std::vector<const xAOD::Jet*>& forwardJets, TLorentzVector& HVec,
    TLorentzVector& metVec, const xAOD::MissingET*& met, int nTaus,
    float bTagWeight) {
  // DO NOT ENTER NEW VARIABLES HERE IF NOT ABSOLUTELY NECESSARY FOR MVA
  // please use the EasyTree functionality for studies

  // m_tree->SetVariation(m_currentVar);

  // first fill the common variables (metVec = VVec)
  fillMVATreeVHbbResolved(b1, b2, j3, selectedJets, signalJets, forwardJets,
                          HVec, metVec, metVec, nTaus);

  // only take the first three jets into account
  if (m_tree->nJ >= 3)
    m_tree->MEff = (b1.Pt() + b2.Pt() + j3.Pt() + metVec.Pt()) / 1e3;
  else
    m_tree->MEff = (b1.Pt() + b2.Pt() + metVec.Pt()) / 1e3;

  m_tree->METSig = Props::metSig.get(met);

  m_tree->bTagWeight = bTagWeight;

  return EL::StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////
///          ** Fill MVA tree (boosted)**                ///
////////////////////////////////////////////////////////////
EL::StatusCode AnalysisReader_VHQQ0Lep::fillMVATreeBoosted_C2(
    const std::vector<const xAOD::Jet*>& fatJets) {
  if (m_boostedtree->nFatJets > 0) {
    m_boostedtree->C2LeadFatJet = Props::C2.get(fatJets.at(0));
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ0Lep::fillMVATreeBoosted(
    const std::vector<const xAOD::Jet*>& signalJets,
    const std::vector<const xAOD::Jet*>& forwardJets, TLorentzVector& mptVec,
    TLorentzVector& metVec, const int nFatJet, const int nMatchedTrackJet,
    // const int nBTagMatchedTrackJet, const int nUnmatchedTrackJet
    TLorentzVector& HVec, double dPhiMETMPT, double mindPhiMerg,
    double mindPhiMerg_3LeadJets, VHCand mergedVH, TLorentzVector& VHVec) {
  m_boostedtree->Reset();
  // EVENT INFO //
  m_boostedtree->EventWeight = m_weight;
  m_boostedtree->EventNumber = m_eventInfo->eventNumber();
  // SMALL JETS //
  m_boostedtree->nSigJets = m_physicsMeta.nSigJet;
  if (m_physicsMeta.nSigJet > 0) {
    m_boostedtree->pTSigJet1 = (signalJets.at(0))->p4().Pt();
    m_boostedtree->etaSigJet1 = (signalJets.at(0))->p4().Eta();
    if (m_physicsMeta.nSigJet > 1) {
      m_boostedtree->pTSigJet2 = (signalJets.at(1))->p4().Pt();
      m_boostedtree->etaSigJet2 = (signalJets.at(1))->p4().Eta();
      if (m_physicsMeta.nSigJet > 2) {
        m_boostedtree->pTSigJet3 = (signalJets.at(2))->p4().Pt();
        m_boostedtree->etaSigJet3 = (signalJets.at(2))->p4().Eta();
      }
    }
  }
  m_boostedtree->nFwdJets = m_physicsMeta.nForwardJet;
  if (m_physicsMeta.nForwardJet > 0) {
    m_boostedtree->pTFwdJet1 = (forwardJets.at(0))->p4().Pt();
    m_boostedtree->etaFwdJet1 = (forwardJets.at(0))->p4().Eta();
    if (m_physicsMeta.nForwardJet > 1) {
      m_boostedtree->pTFwdJet2 = (forwardJets.at(1))->p4().Pt();
      m_boostedtree->etaFwdJet2 = (forwardJets.at(1))->p4().Eta();
      if (m_physicsMeta.nForwardJet > 2) {
        m_boostedtree->pTFwdJet3 = (forwardJets.at(2))->p4().Pt();
        m_boostedtree->etaFwdJet3 = (forwardJets.at(2))->p4().Eta();
      }
    }
  }

  m_boostedtree->MPT = mptVec.Pt();
  m_boostedtree->MET = metVec.Pt();

  m_boostedtree->nFatJets = nFatJet;
  m_boostedtree->nMatchTrkJetLeadFatJet = nMatchedTrackJet;
  m_boostedtree->nBTagMatchTrkJetLeadFatJet = m_physicsMeta.nTagsInFJ;
  m_boostedtree->nBTagUnmatchTrkJetLeadFatJet = m_physicsMeta.nAddBTrkJets;

  if (nFatJet > 0) {
    TLorentzVector LeadFatJet = HVec;
    m_boostedtree->pTLeadFatJet = LeadFatJet.Pt();
    m_boostedtree->etaLeadFatJet = LeadFatJet.Eta();
    m_boostedtree->phiLeadFatJet = LeadFatJet.Phi();
    m_boostedtree->mJ = LeadFatJet.M();

    m_boostedtree->dPhiMETLeadFatJet = fabs(metVec.DeltaPhi(HVec));
    m_boostedtree->MEff = HVec.Pt() + metVec.Pt();
  }

  m_boostedtree->dPhiMETMPT = dPhiMETMPT;
  m_boostedtree->mindPhiMETJets = mindPhiMerg;
  m_boostedtree->mindPhiMETJets_3leadjets = mindPhiMerg_3LeadJets;  //!
  m_boostedtree->mVH =
      mergedVH.vec_corr.M();                // not rescaled, muon-in-jet applied
  m_boostedtree->mVH_rescaled = VHVec.M();  // rescaled
  m_boostedtree->pTVH = VHVec.Pt();         // rescaled

  m_boostedtree->sample = m_histNameSvc->getFullSample();
  m_boostedtree->SetVariation(m_currentVar);

  // fill MVA tree for boosted regime
  // if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged &&
  // m_histNameSvc -> get_description() == "SR" &&
  // m_histNameSvc->get_isNominal())
  //  m_boostedtree->Fill();

  return EL::StatusCode::SUCCESS;
}

// Elisabeth: comment out histogram filling from
// variables that are not longer in MVATree
// -> fill from EasyTree
////////////////////////////////////////////////////////////
///      ** Fill histograms for 0Lep selection **        ///
////////////////////////////////////////////////////////////
EL::StatusCode AnalysisReader_VHQQ0Lep::fill0LepHistResolved() {
  if (m_model == Model::AZh || m_model == Model::HVT) {
    // m_histSvc->BookFillHist("mVH", 600, 0, 6000, m_tree->mVH / 1e3,
    // m_weight);
    if (m_doOnlyInputs && !m_doReduceFillHistos) {
      m_histSvc->BookFillHist("mBB", 100, 0, 500, m_tree->mBB, m_weight);
    }

  } else {  // not AZh or HVT, but MVA or CUT or others
    if (m_doBootstrap) {
      m_histSvc->GenerateBootstrapWeights(m_weight, m_eventInfo->runNumber(),
                                          m_eventInfo->eventNumber(),
                                          m_eventInfo->mcChannelNumber());
      std::string mBBName = "mBB";
      if (m_model == Model::CUT) mBBName = "mBBCutBased";
      m_histSvc->BookFillBootstrapHist(
          mBBName, 100, 0, 500, m_tree->mBB,
          m_weight);  // for post-fit plots mBB (w/ mu-in-jet and ptreco corr)
      // BDT
      if (m_model == Model::MVA) {
        for (std::map<std::string, MVAApplication_TMVA*>::iterator mva_iter =
                 m_mvaVHbbApps.begin();
             mva_iter != m_mvaVHbbApps.end(); mva_iter++) {
          std::string mvaName = mva_iter->first;
          float mvaScore = mva_iter->second->Evaluate(
              m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
          m_histSvc->BookFillHist(mvaName, 500, -1, 1, mvaScore,
                                  m_weight);  // new mva discriminant
        }
      }
    } else {  // these are done for all settings, so what you want only for
              // doOnlyFit put here

      /*m_OStree->Reset();
      fillOSTree();
      m_OStree->Fill();*/

      std::string mBBName = "mBB";
      if (m_model == Model::CUT) mBBName = "mBBCutBased";
      m_histSvc->BookFillHist(
          mBBName, 100, 0, 500, m_tree->mBB,
          m_weight);  // for post-fit plots mBB (w/ mu-in-jet and ptreco corr)
      // BDT
      if (m_model == Model::MVA) {
        for (std::map<std::string, MVAApplication_TMVA*>::iterator mva_iter =
                 m_mvaVHbbApps.begin();
             mva_iter != m_mvaVHbbApps.end(); mva_iter++) {
          std::string mvaName = mva_iter->first;
          float mvaScore = mva_iter->second->Evaluate(
              m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
          m_histSvc->BookFillHist(mvaName, 500, -1, 1, mvaScore,
                                  m_weight);  // new mva discriminant
        }
      }
      if (m_doOnlyInputs) {
        m_histSvc->BookFillHist("MET", 200, 0, 2000, m_tree->MET, m_weight);
      }
    }
  }

  if (m_doReduceFillHistos) {
    if (m_model == Model::AZh || m_model == Model::HVT) {
      m_histSvc->BookFillHist("mBB", 100, 0, 500, m_tree->mBB, m_weight);
      // to check a percent of events where we use forward jets:
      if (m_tree->nSigJet == 2 && m_tree->nForwardJet >= 1) {
        m_histSvc->BookFillHist("mBB_fwdJets", 100, 0, 500, m_tree->mBB,
                                m_weight);
      }

      m_histSvc->BookFillHist("MET", 200, 0, 2000, m_tree->MET, m_weight);
      m_histSvc->BookFillHist("METSig", 300, 0., 30., m_tree->METSig, m_weight);
      m_histSvc->BookFillHist("METOverSqrtHT", 300, 0., 30.,
                              m_tree->MET / TMath::Sqrt(m_tree->MEff),
                              m_weight);
      m_histSvc->BookFillHist("pTB1", 200, 0, 2000, m_tree->pTB1, m_weight);
      m_histSvc->BookFillHist("pTB2", 200, 0, 2000, m_tree->pTB2, m_weight);
      m_histSvc->BookFillHist("EtaB1", 100, -5, 5, m_tree->etaB1, m_weight);
      m_histSvc->BookFillHist("EtaB2", 100, -5, 5, m_tree->etaB2, m_weight);

      m_histSvc->BookFillHist("Njets", 25, 0, 25, m_tree->nJ, m_weight);
      m_histSvc->BookFillHist("SumPtJets", 600, 0, 3000, m_tree->sumPtJets,
                              m_weight);

    }  // done if AZh or HVT
  }    // done if reduce set of Histos

  if (!m_doOnlyInputs) {
    // MVA inputs (except mBB, already stored along with mva) for post-fit plots
    m_histSvc->BookFillHist("softMET", 200, 0, 1000, m_tree->softMET, m_weight);
    m_histSvc->BookFillHist("MET", 400, 0, 2000, m_tree->MET, m_weight);
    m_histSvc->BookFillHist("pTB1", 200, 0, 2000, m_tree->pTB1, m_weight);
    m_histSvc->BookFillHist("pTB2", 200, 0, 2000, m_tree->pTB2, m_weight);
    m_histSvc->BookFillHist("dPhiBB", 100, 0, 3.2, m_tree->dPhiBB, m_weight);
    m_histSvc->BookFillHist("dEtaBB", 100, 0, 5, m_tree->dEtaBB, m_weight);
    m_histSvc->BookFillHist("dRBB", 100, 0, 5, m_tree->dRBB, m_weight);
    m_histSvc->BookFillHist("dPhiVBB", 100, 0, 3.2, m_tree->dPhiVBB, m_weight);
    m_histSvc->BookFillHist("HT", 400, 0, 2000, m_tree->MEff, m_weight);
    if (m_tree->nJ >= 3) {
      m_histSvc->BookFillHist("pTJ3", 100, 0, 500, m_tree->pTJ3, m_weight);
      m_histSvc->BookFillHist("mBBJ", 200, 0, 1000, m_tree->mBBJ, m_weight);
    }  // done if more than three jets
  }    // done if !m_doOnlyInputs

  if (!m_doOnlyInputs && !m_doReduceFillHistos) {
    if (m_model == Model::AZh || m_model == Model::HVT) {
      m_histSvc->BookFillHist(
          "mBB", 100, 0, 500, m_tree->mBB,
          m_weight);  // for post-fit plots mBB (w/ mu-in-jet and ptreco corr)
      m_histSvc->BookFillHist("MET", 200, 0, 2000, m_tree->MET,
                              m_weight);  // for post-fit plots MET
    }

    m_histSvc->BookFillHist("METSig", 300, 0., 30., m_tree->METSig,
                            m_weight);  // for post-fit plots METSig
    m_histSvc->BookFillHist("METOverSqrtHT", 300, 0., 30.,
                            m_tree->MET / TMath::Sqrt(m_tree->MEff),
                            m_weight);  // for post-fit plots METOverSqrtHT
    m_histSvc->BookFillHist("nTaus", 5, 0, 5, m_tree->nTaus, m_weight);
    m_histSvc->BookFillHist("SumPtJets", 600, 0, 3000, m_tree->sumPtJets,
                            m_weight);
    m_histSvc->BookFillHist("Njets", 25, 0, 25, m_tree->nJ,
                            m_weight);  // for post-fit plots central+forward
    m_histSvc->BookFillHist("NTags", 5, 0, 5, m_tree->nTags, m_weight);
    m_histSvc->BookFillHist(
        "EtaB1", 100, -5, 5, m_tree->etaB1,
        m_weight);  // for post-fit plots (w/o mu-in-jet and ptreco corr)
    m_histSvc->BookFillHist(
        "EtaB2", 100, -5, 5, m_tree->etaB2,
        m_weight);  // for post-fit plots (w/o mu-in-jet and ptreco corr)
    m_histSvc->BookFillHist("RandomRunNumber", 100, 2.5e5, 3.5e5,
                            m_randomRunNumber, m_weight);
    m_histSvc->BookFillHist("PileupReweight", 100, 0, 2, m_pileupReweight,
                            m_weight);
    m_histSvc->BookFillHist("AverageMu", 100, 0, 100, m_averageMu, m_weight);
    m_histSvc->BookFillHist("ActualMu", 100, 0, 100, m_actualMu, m_weight);
    m_histSvc->BookFillHist("AverageMuScaled", 100, 0, 100, m_averageMuScaled,
                            m_weight);
    m_histSvc->BookFillHist("ActualMuScaled", 100, 0, 100, m_actualMuScaled,
                            m_weight);

    // J3 - fill for easy book keeping of plots, but filled with the default
    // (dummy) values
    m_histSvc->BookFillHist("EtaJ3", 100, -5, 5, m_tree->etaJ3, m_weight);
    m_histSvc->BookFillHist("PhiJ3", 200, -3.15, 3.15, m_tree->phiJ3, m_weight);

  }  // done if !m_doOnlyInputs && !m_doReduceFillHistos

  return EL::StatusCode::SUCCESS;
}  // fill0LepHistResolved

EL::StatusCode AnalysisReader_VHQQ0Lep::fill0LepHistMerged_C2() {
  double M = m_boostedtree->mJ / 1e3;
  float C2 = m_boostedtree->C2LeadFatJet;

  if (!m_doOnlyInputs && !m_doReduceFillHistos) {
    // C2 distribution
    m_histSvc->BookFillHist("C2", 200, 0., 1., C2, m_weight);

    // C2 distribution in Higgs Mass window
  /*  if (m_boostedtree->mJ / 1e3 > 100. && m_boostedtree->mJ / 1e3 < 140.) {
      m_histSvc->BookFillHist("C2mJ100M140", 200, 0., 1., C2, m_weight);
    }*/ //by wym
  }

  //wym : do only C2
  return EL::StatusCode::SUCCESS;
  //by wym

  // mJ with C2 cuts applied
  for (auto cut_info : m_c2Cuts) {
    if (C2 < cut_info.second)
      m_histSvc->BookFillHist(("mJ" + cut_info.first).c_str(), 100, 0, 500, M,
                              m_weight);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ0Lep::fill0LepHistMerged() {

  //wym : do only C2
  return EL::StatusCode::SUCCESS;
  //by wym

  if (m_doOnlyInputs && !m_doReduceFillHistos) {
    m_histSvc->BookFillHist("MET", 200, 0, 2000, m_boostedtree->MET / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("mJ", 100, 0, 500, m_boostedtree->mJ / 1e3,
                            m_weight);
  }
  if (m_doReduceFillHistos) {
    m_histSvc->BookFillHist("MET", 200, 0, 2000, m_boostedtree->MET / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("mJ", 100, 0, 500, m_boostedtree->mJ / 1e3,
                            m_weight);
    // to check a percent of events where we use forward jets:
    if (m_boostedtree->nSigJets == 2 && m_boostedtree->nFwdJets >= 1)
      m_histSvc->BookFillHist("mJ_fwdJets", 100, 0, 500,
                              m_boostedtree->mJ / 1e3, m_weight);

    m_histSvc->BookFillHist("pTLeadFatJet", 200, 0, 2000,
                            m_boostedtree->pTLeadFatJet / 1e3, m_weight);
    m_histSvc->BookFillHist("etaLeadFatJet", 100, -3, 3,
                            m_boostedtree->etaLeadFatJet, m_weight);
    m_histSvc->BookFillHist("NBTagMatchedTrackJetLeadFatJet", 8, 0, 8,
                            m_boostedtree->nBTagMatchTrkJetLeadFatJet,
                            m_weight);
    m_histSvc->BookFillHist("NMatchedTrackJetLeadFatJet", 8, 0, 8,
                            m_boostedtree->nMatchTrkJetLeadFatJet, m_weight);
    m_histSvc->BookFillHist("NBTagUnmatchedTrackJetLeadFatJet", 8, 0, 8,
                            m_boostedtree->nBTagUnmatchTrkJetLeadFatJet,
                            m_weight);

    m_histSvc->BookFillHist("NFatJets", 8, 0, 8, m_boostedtree->nFatJets,
                            m_weight);

    m_histSvc->BookFillHist("dPhiMETMPT", 18, 0, 180,
                            m_boostedtree->dPhiMETMPT * 180 / TMath::Pi(),
                            m_weight);
    m_histSvc->BookFillHist(
        "dPhiMETLeadFatJet", 18, 0, 180,
        m_boostedtree->dPhiMETLeadFatJet * 180 / TMath::Pi(), m_weight);
    m_histSvc->BookFillHist("MindPhiMETJet", 18, 0, 180,
                            m_boostedtree->mindPhiMETJets * 180 / TMath::Pi(),
                            m_weight);
  }

  if (!m_doOnlyInputs && !m_doReduceFillHistos) {
    m_histSvc->BookFillHist("mVH", 600, 0, 6000, m_boostedtree->mVH / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("MET", 200, 0, 2000, m_boostedtree->MET / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("mJ", 100, 0, 500, m_boostedtree->mJ / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("pTLeadFatJet", 200, 0, 2000,
                            m_boostedtree->pTLeadFatJet / 1e3, m_weight);
    m_histSvc->BookFillHist("NBTagMatchedTrackJetLeadFatJet", 8, 0, 8,
                            m_boostedtree->nBTagMatchTrkJetLeadFatJet,
                            m_weight);

    m_histSvc->BookFillHist("NFatJets", 8, 0, 8, m_boostedtree->nFatJets,
                            m_weight);
    m_histSvc->BookFillHist("etaLeadFatJet", 100, -3, 3,
                            m_boostedtree->etaLeadFatJet, m_weight);
    m_histSvc->BookFillHist("NSignalJets", 15, 0, 15, m_boostedtree->nSigJets,
                            m_weight);
    m_histSvc->BookFillHist("PtSigJet1", 100, 0, 2500,
                            m_boostedtree->pTSigJet1 / 1e3, m_weight);
    m_histSvc->BookFillHist("PtSigJet2", 100, 0, 2500,
                            m_boostedtree->pTSigJet2 / 1e3, m_weight);
    m_histSvc->BookFillHist("PtSigJet3", 100, 0, 2500,
                            m_boostedtree->pTSigJet3 / 1e3, m_weight);
    m_histSvc->BookFillHist("EtaSigJet1", 100, -3, 3, m_boostedtree->etaSigJet1,
                            m_weight);
    m_histSvc->BookFillHist("EtaSigJet2", 100, -3, 3, m_boostedtree->etaSigJet2,
                            m_weight);
    m_histSvc->BookFillHist("EtaSigJet3", 100, -3, 3, m_boostedtree->etaSigJet3,
                            m_weight);

    m_histSvc->BookFillHist("NForwardJets", 15, 0, 15, m_boostedtree->nFwdJets,
                            m_weight);
    m_histSvc->BookFillHist("PtFwdJet1", 100, 0, 2500,
                            m_boostedtree->pTFwdJet1 / 1e3, m_weight);
    m_histSvc->BookFillHist("PtFwdJet2", 100, 0, 2500,
                            m_boostedtree->pTFwdJet2 / 1e3, m_weight);
    m_histSvc->BookFillHist("PtFwdJet3", 100, 0, 2500,
                            m_boostedtree->pTFwdJet3 / 1e3, m_weight);
    m_histSvc->BookFillHist("EtaFwdJet1", 100, -3, 3, m_boostedtree->etaFwdJet1,
                            m_weight);
    m_histSvc->BookFillHist("EtaFwdJet2", 100, -3, 3, m_boostedtree->etaFwdJet2,
                            m_weight);
    m_histSvc->BookFillHist("EtaFwdJet3", 100, -3, 3, m_boostedtree->etaFwdJet3,
                            m_weight);

    m_histSvc->BookFillHist("NMatchedTrackJetLeadFatJet", 8, 0, 8,
                            m_boostedtree->nMatchTrkJetLeadFatJet, m_weight);
    m_histSvc->BookFillHist("NBTagUnmatchedTrackJetLeadFatJet", 8, 0, 8,
                            m_boostedtree->nBTagUnmatchTrkJetLeadFatJet,
                            m_weight);
    m_histSvc->BookFillHist("dPhiMETMPT", 18, 0, 180,
                            m_boostedtree->dPhiMETMPT * 180 / TMath::Pi(),
                            m_weight);
    m_histSvc->BookFillHist(
        "dPhiMETLeadFatJet", 18, 0, 180,
        m_boostedtree->dPhiMETLeadFatJet * 180 / TMath::Pi(), m_weight);
    m_histSvc->BookFillHist("MindPhiMETJet", 18, 0, 180,
                            m_boostedtree->mindPhiMETJets * 180 / TMath::Pi(),
                            m_weight);
    // m_histSvc -> BookFillHist("MindPhiMETJet_3leadjets",          18, 0, 180,
    // m_boostedtree -> mindPhiMETJets_3leadjets,     m_weight);
    m_histSvc->BookFillHist("phiLeadFatJet", 36, -180, 180,
                            m_boostedtree->phiLeadFatJet * 180 / TMath::Pi(),
                            m_weight);
    m_histSvc->BookFillHist("AverageMu", 100, 0, 100, m_averageMu, m_weight);
    m_histSvc->BookFillHist("ActualMu", 100, 0, 100, m_actualMu, m_weight);
    m_histSvc->BookFillHist("AverageMuScaled", 100, 0, 100, m_averageMuScaled,
                            m_weight);
    m_histSvc->BookFillHist("ActualMuScaled", 100, 0, 100, m_actualMuScaled,
                            m_weight);

    m_histSvc->BookFillHist("MET_Track", 100, 0, 1000, m_boostedtree->MPT / 1e3,
                            m_weight);
  }

  return EL::StatusCode::SUCCESS;
}  // fill0LepHistMerged

// Elisabeth: comment out EasyTree filling from
// variables that are not longer in MVATree
////////////////////////////////////////////////////////////
///               ** Fill easy tree **                   ///
////////////////////////////////////////////////////////////

// variables specific to resolved VHbb 0L analysis
EL::StatusCode AnalysisReader_VHQQ0Lep::fillETreeResolved(
    TLorentzVector& /*HVec*/, TLorentzVector& /*bbjVec*/,
    TLorentzVector& /*metVec*/,
    const std::vector<const xAOD::Jet*>& /*selectedJets*/, Jet& /*j1_sel*/,
    Jet& /*j2_sel*/, const std::vector<const xAOD::Jet*>& fatJets,
    const std::vector<const xAOD::Jet*>& trackJets) {
  // EasyTree reset already done in fillETreeCommon

  m_etree->SetBranchAndValue<double>("nJ", m_tree->nJ, -99.);
  m_etree->SetBranchAndValue<double>("mBB", m_tree->mBB, -99.);
  m_etree->SetBranchAndValue<double>("dRBB", m_tree->dRBB, -99.);
  m_etree->SetBranchAndValue<double>("dEtaBB", m_tree->dEtaBB, -99.);
  m_etree->SetBranchAndValue<double>("dPhiBB", m_tree->dPhiBB, -99.);
  // m_etree->SetBranchAndValue<double>("pTBB", m_tree->pTBB / 1.e3, -99.);
  // m_etree->SetBranchAndValue<double>("yBB", m_tree->yBB, -99.);
  m_etree->SetBranchAndValue<double>("MET", m_tree->MET, -99.);
  m_etree->SetBranchAndValue<double>("HT", m_tree->MEff, -99.);
  // m_etree->SetBranchAndValue<double>("MPT", m_tree->MPT / 1.e3, -99.);
  // m_etree->SetBranchAndValue<double>("dPhiMETdijet", m_tree->dPhiMETdijet,
  //                                    -99.);
  // m_etree->SetBranchAndValue<double>("MindPhiMETJet", m_tree->mindPhi, -99.);
  // m_etree->SetBranchAndValue<double>("dPhiMETMPT", m_tree->dPhiMETMPT, -99.);

  m_etree->SetBranchAndValue<double>("pTB1", m_tree->pTB1, -99.);
  m_etree->SetBranchAndValue<double>("etaB1", m_tree->etaB1, -99.);
  m_etree->SetBranchAndValue<double>("phiB1", m_tree->phiB1, -99.);
  m_etree->SetBranchAndValue<double>("mB1", m_tree->mB1, -99.);

  m_etree->SetBranchAndValue<double>("pTB2", m_tree->pTB2, -99.);
  m_etree->SetBranchAndValue<double>("etaB2", m_tree->etaB2, -99.);
  m_etree->SetBranchAndValue<double>("phiB2", m_tree->phiB2, -99.);
  m_etree->SetBranchAndValue<double>("mB2", m_tree->mB2, -99.);

  m_etree->SetBranchAndValue<double>("pTJ3", m_tree->pTJ3, -99.);
  m_etree->SetBranchAndValue<double>("etaJ3", m_tree->etaJ3, -99.);
  m_etree->SetBranchAndValue<double>("phiJ3", m_tree->phiJ3, -99.);
  m_etree->SetBranchAndValue<double>("mJ3", m_tree->mJ3, -99.);

  m_etree->SetBranchAndValue<double>("mBBJ", m_tree->mBBJ, -99.);
  m_etree->SetBranchAndValue<double>("sumPtJets", m_tree->sumPtJets, -99.);

  m_etree->SetBranchAndValue<double>("mva", m_tree->BDT, -99.);
  m_etree->SetBranchAndValue<double>("mvadiboson", m_tree->BDT_VZ, -99.);

  m_etree->SetBranchAndValue<int>("nTags", m_tree->nTags, -99);
  m_etree->SetBranchAndValue<int>("b1Flav", m_physicsMeta.b1Flav, -99);
  m_etree->SetBranchAndValue<int>("b2Flav", m_physicsMeta.b2Flav, -99);
  m_etree->SetBranchAndValue<int>("j3Flav", m_physicsMeta.j3Flav, -99);

  m_etree->SetBranchAndValue<double>("bin_MV2c10B1", m_tree->bin_MV2c10B1,
                                     -99.);
  m_etree->SetBranchAndValue<double>("bin_MV2c10B2", m_tree->bin_MV2c10B2,
                                     -99.);
  m_etree->SetBranchAndValue<double>("bin_MV2c10J3", m_tree->bin_MV2c10J3,
                                     -99.);

  m_etree->SetBranchAndValue<int>("nTaus", m_tree->nTaus, -99);

  // EL_CHECK("AnalysisReader_VHQQ0Lep::fillETree",
  // compute_jetSelectedQuantities(selectedJets));
  m_etree->SetBranchAndValue<int>("nFatJets", fatJets.size(), -99);
  if (fatJets.size() >= 1) {
    m_etree->SetBranchAndValue<int>("nbTagsInFJ", m_physicsMeta.nTagsInFJ, -99);
    m_etree->SetBranchAndValue<int>("nTrkjetsInFJ",
                                    Props::nTrackJets.get(fatJets.at(0)), -99);
  }
  m_etree->SetBranchAndValue<int>("nbTagsOutsideFJ", m_physicsMeta.nAddBTrkJets,
                                  -99);
  m_etree->SetBranchAndValue<int>("nTrkJets", trackJets.size(), -99);

  return EL::StatusCode::SUCCESS;
}  // fillETreeResolved

// variables common to resolved and merged VHbb 0L analyses
EL::StatusCode AnalysisReader_VHQQ0Lep::fillETreeCommon() {
  // EasyTree reset
  m_etree->Reset();
  m_etree->SetVariation(m_currentVar);

  // keep type double for BDT's Spectator variables

  string Regime = "none";
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    Regime = "merged";
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved)
    Regime = "resolved";

  m_etree->SetBranchAndValue<std::string>("Description",
                                          m_histNameSvc->get_description(), "");
  m_etree->SetBranchAndValue<std::string>("Regime", Regime, "none");
  m_etree->SetBranchAndValue<std::string>("Sample",
                                          m_histNameSvc->getFullSample(), "");
  m_etree->SetBranchAndValue<int>("ChannelNumber", m_mcChannel, -99);
  m_etree->SetBranchAndValue<int>("RunNumber", m_eventInfo->runNumber(), -99);
  m_etree->SetBranchAndValue<double>("EventNumber", m_eventInfo->eventNumber(),
                                     -99.);
  m_etree->SetBranchAndValue<double>("EventNumberMod2",
                                     (m_eventInfo->eventNumber()) % 2, -99.);
  m_etree->SetBranchAndValue<double>("EventWeight", m_weight, -99.);
  m_etree->SetBranchAndValue<double>("AverageMu", m_averageMu, -99);
  m_etree->SetBranchAndValue<double>("ActualMu", m_actualMu, -99);
  m_etree->SetBranchAndValue<double>("AverageMuScaled", m_averageMuScaled, -99);
  m_etree->SetBranchAndValue<double>("ActualMuScaled", m_actualMuScaled, -99);

  return EL::StatusCode::SUCCESS;
}  // fillETreeCommon

// fill variables specific to merged analysis
EL::StatusCode AnalysisReader_VHQQ0Lep::fillETreeMerged(
    TLorentzVector& metVec, TLorentzVector& mptVec,
    const std::vector<const xAOD::Jet*>& trackJetsInLeadFJ) {
  // EasyTree reset already done in fillETreeCommon
  m_etree->SetBranchAndValue<double>(
      "dPhiMETMPT", m_boostedtree->dPhiMETMPT * 180 / TMath::Pi(), -99.);
  m_etree->SetBranchAndValue<double>(
      "dPhiMETLeadFatJet", m_boostedtree->dPhiMETLeadFatJet * 180 / TMath::Pi(),
      -99.);
  m_etree->SetBranchAndValue<double>("EtaFwdJet1", m_boostedtree->etaFwdJet1,
                                     -99.);
  m_etree->SetBranchAndValue<double>("EtaFwdJet2", m_boostedtree->etaFwdJet2,
                                     -99.);
  m_etree->SetBranchAndValue<double>("EtaFwdJet3", m_boostedtree->etaFwdJet3,
                                     -99.);
  m_etree->SetBranchAndValue<double>("etaLeadFatJet",
                                     m_boostedtree->etaLeadFatJet, -99.);
  m_etree->SetBranchAndValue<double>("EtaSigJet1", m_boostedtree->etaSigJet1,
                                     -99.);
  m_etree->SetBranchAndValue<double>("EtaSigJet2", m_boostedtree->etaSigJet2,
                                     -99.);
  m_etree->SetBranchAndValue<double>("EtaSigJet3", m_boostedtree->etaSigJet3,
                                     -99.);
  m_etree->SetBranchAndValue<double>("MET", m_boostedtree->MET / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("MET_Track", m_boostedtree->MPT / 1e3,
                                     -99.);
  m_etree->SetBranchAndValue<double>(
      "MindPhiMETJet", m_boostedtree->mindPhiMETJets * 180 / TMath::Pi(), -99.);
  m_etree->SetBranchAndValue<double>("mJ", m_boostedtree->mJ / 1e3, -99.);
  //  m_etree->SetBranchAndValue<double>("MLeadFatJet_fwdJets",
  //                                     m_boostedtree->MLeadFatJet / 1e3,
  //                                     -99.);
  m_etree->SetBranchAndValue<double>("mVH", m_boostedtree->mVH / 1e3, -99.);
  m_etree->SetBranchAndValue<int>("NBTagMatchedTrackJetLeadFatJet",
                                  m_boostedtree->nBTagMatchTrkJetLeadFatJet,
                                  -99.);
  m_etree->SetBranchAndValue<int>("NBTagUnmatchedTrackJetLeadFatJet",
                                  m_boostedtree->nBTagUnmatchTrkJetLeadFatJet,
                                  -99.);
  m_etree->SetBranchAndValue<int>("NFatJets", m_boostedtree->nFatJets, -99.);
  m_etree->SetBranchAndValue<int>("NMatchedTrackJetLeadFatJet",
                                  m_boostedtree->nMatchTrkJetLeadFatJet, -99.);
  m_etree->SetBranchAndValue<int>("NSignalJets", m_boostedtree->nSigJets, -99.);
  m_etree->SetBranchAndValue<int>("NForwardJets", m_boostedtree->nFwdJets,
                                  -99.);
  m_etree->SetBranchAndValue<double>(
      "phiLeadFatJet", m_boostedtree->phiLeadFatJet * 180 / TMath::Pi(), -99.);
  m_etree->SetBranchAndValue<double>("phiMET", metVec.Phi() * 180 / TMath::Pi(),
                                     -99.);
  m_etree->SetBranchAndValue<double>("phiMPT", mptVec.Phi() * 180 / TMath::Pi(),
                                     -99.);

  m_etree->SetBranchAndValue<double>("PtFwdJet1",
                                     m_boostedtree->pTFwdJet1 / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("PtFwdJet2",
                                     m_boostedtree->pTFwdJet2 / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("PtFwdJet3",
                                     m_boostedtree->pTFwdJet3 / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("pTLeadFatJet",
                                     m_boostedtree->pTLeadFatJet / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("PtSigJet1",
                                     m_boostedtree->pTSigJet1 / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("PtSigJet2",
                                     m_boostedtree->pTSigJet2 / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("PtSigJet3",
                                     m_boostedtree->pTSigJet3 / 1e3, -99.);
  double SUSYMET = -99000.;
  if (Props::SUSYMET.exists(m_eventInfo)) {
    SUSYMET = Props::SUSYMET.get(m_eventInfo);
  }
  m_etree->SetBranchAndValue<double>("SUSYMET", SUSYMET / 1e3, -99.);

  float PtTrkJet1InLeadFJ = -99.;
  float EtaTrkJet1InLeadFJ = -99.;
  float PhiTrkJet1InLeadFJ = -99.;
  int IsTaggedTrkJet1InLeadFJ = -99;

  float PtTrkJet2InLeadFJ = -99.;
  float EtaTrkJet2InLeadFJ = -99.;
  float PhiTrkJet2InLeadFJ = -99.;
  int IsTaggedTrkJet2InLeadFJ = -99;

  float PtTrkJet3InLeadFJ = -99.;
  float EtaTrkJet3InLeadFJ = -99.;
  float PhiTrkJet3InLeadFJ = -99.;
  int IsTaggedTrkJet3InLeadFJ = -99;

  if (trackJetsInLeadFJ.size() >= 1) {
    PtTrkJet1InLeadFJ = trackJetsInLeadFJ.at(0)->p4().Pt();
    EtaTrkJet1InLeadFJ = trackJetsInLeadFJ.at(0)->p4().Eta();
    PhiTrkJet1InLeadFJ = trackJetsInLeadFJ.at(0)->p4().Phi();
    IsTaggedTrkJet1InLeadFJ = BTagProps::isTagged.get(trackJetsInLeadFJ.at(0));
  }
  if (trackJetsInLeadFJ.size() >= 2) {
    PtTrkJet2InLeadFJ = trackJetsInLeadFJ.at(1)->p4().Pt();
    EtaTrkJet2InLeadFJ = trackJetsInLeadFJ.at(1)->p4().Eta();
    PhiTrkJet2InLeadFJ = trackJetsInLeadFJ.at(1)->p4().Phi();
    IsTaggedTrkJet2InLeadFJ = BTagProps::isTagged.get(trackJetsInLeadFJ.at(1));
  }
  if (trackJetsInLeadFJ.size() >= 3) {
    PtTrkJet3InLeadFJ = trackJetsInLeadFJ.at(2)->p4().Pt();
    EtaTrkJet3InLeadFJ = trackJetsInLeadFJ.at(2)->p4().Eta();
    PhiTrkJet3InLeadFJ = trackJetsInLeadFJ.at(2)->p4().Phi();
    IsTaggedTrkJet3InLeadFJ = BTagProps::isTagged.get(trackJetsInLeadFJ.at(2));
  }

  m_etree->SetBranchAndValue<double>("PtTrkJet1InLeadFJ",
                                     PtTrkJet1InLeadFJ / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("EtaTrkJet1InLeadFJ", EtaTrkJet1InLeadFJ,
                                     -99.);
  m_etree->SetBranchAndValue<double>(
      "PhiTrkJet1InLeadFJ", PhiTrkJet1InLeadFJ * 180 / TMath::Pi(), -99.);
  m_etree->SetBranchAndValue<int>("IsTaggedTrkJet1InLeadFJ",
                                  IsTaggedTrkJet1InLeadFJ, -99);
  m_etree->SetBranchAndValue<double>("PtTrkJet2InLeadFJ",
                                     PtTrkJet2InLeadFJ / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("EtaTrkJet2InLeadFJ", EtaTrkJet2InLeadFJ,
                                     -99.);
  m_etree->SetBranchAndValue<double>(
      "PhiTrkJet2InLeadFJ", PhiTrkJet2InLeadFJ * 180 / TMath::Pi(), -99.);
  m_etree->SetBranchAndValue<int>("IsTaggedTrkJet2InLeadFJ",
                                  IsTaggedTrkJet2InLeadFJ, -99);
  m_etree->SetBranchAndValue<double>("PtTrkJet3InLeadFJ",
                                     PtTrkJet3InLeadFJ / 1e3, -99.);
  m_etree->SetBranchAndValue<double>("EtaTrkJet3InLeadFJ", EtaTrkJet3InLeadFJ,
                                     -99.);
  m_etree->SetBranchAndValue<double>(
      "PhiTrkJet3InLeadFJ", PhiTrkJet3InLeadFJ * 180 / TMath::Pi(), -99.);
  m_etree->SetBranchAndValue<int>("IsTaggedTrkJet3InLeadFJ",
                                  IsTaggedTrkJet3InLeadFJ, -99);

  // add additional calo jet info to ntuples, for definition of HP and LP SR
  if (m_SplitBoostedInputsAddJets) {
    m_etree->SetBranchAndValue<int>("NAdditionalCaloJets",
                                    m_histNameSvc->get_nJet(), -1);
  }

  if (m_isMC) {
    if (m_evtWeightVarMode == 0) {
      // add internal weights to easy tree only for essentials to avoid massive
      // trees
      std::unordered_set<std::string> intVars =
          m_EvtWeightVar->getListOfVariations(m_mcChannel);
      for (auto& csVar : intVars) {
        // Extract the alternative weight
        float AltWeight =
            m_EvtWeightVar->getWeightVariation(m_eventInfo, csVar);
        m_etree->SetBranchAndValue<float>("Sys__" + csVar, AltWeight, -222.0);
      }
    }
    // add flavour composition for track jets in fat jet to easytree
    m_etree->SetBranchAndValue<int>("flavB1", m_physicsMeta.b1Flav, -1);
    m_etree->SetBranchAndValue<int>("flavB2", m_physicsMeta.b2Flav, -1);
    m_etree->SetBranchAndValue<int>("flavJ3", m_physicsMeta.j3Flav, -1);
    if (m_EvtWeightVar->isPowhegPythia8Ttbar(m_mcChannel)) {
      if (Props::codeTTBarDecay.exists(m_eventInfo)) {
        m_etree->SetBranchAndValue<int>(
            "TTBarDecay", Props::codeTTBarDecay.get(m_eventInfo), -1);
      }
    }
  }

  // variable commented out in histogram filling: "MindPhiMETJet_3leadjets"
  // m_boostedtree -> mindPhiMETJets_3leadjets

  return EL::StatusCode::SUCCESS;
}  // fillETreeMerged

// variables common to resolved and merged VHbb 0L analyses
EL::StatusCode AnalysisReader_VHQQ0Lep::fillETreeWeights(float triggerSF,
                                                         float JVTweight,
                                                         float btagWeight) {
  if (m_isMC)
    m_etree->SetBranchAndValue<float>(
        "MCEventWeight", Props::MCEventWeight.get(m_eventInfo), -99999999);
  m_etree->SetBranchAndValue<float>(
      "LumiWeight", Props::LumiWeight.get(m_eventInfo), -99999999);
  m_etree->SetBranchAndValue<float>("PUWeight", m_pileupReweight, -99999999);
  m_etree->SetBranchAndValue<float>("BTagSF", btagWeight, -99999999);
  m_etree->SetBranchAndValue<float>("TriggerSF", triggerSF, -99999999);
  m_etree->SetBranchAndValue<float>("JVTWeight", JVTweight, -999999);

  return EL::StatusCode::SUCCESS;
}  // fillETreeWeights

EL::StatusCode AnalysisReader_VHQQ0Lep::getTruthInfo_0Lep(
    int nTag, std::vector<const xAOD::Jet*> jets) {
  enum decaytype {
    undefined = 0,
    el,
    mu,
    tau,
    elel,
    elmu,
    mumu,
    eltau,
    mutau,
    tautau
  };
  enum jettype { other = 0, b, c, l, taujet, eljet, mujet };
  TLorentzVector jet1, jet2;
  if (jets.size() > 0) jet1 = jets.at(0)->p4();
  if (jets.size() > 1) jet2 = jets.at(1)->p4();
  // look for ttbar background
  int index = -1;
  int nEl = 0;
  int nMu = 0;
  int nTau = 0;
  std::vector<int> pdgIdPart;
  pdgIdPart.clear();
  double deltaR1_min = 999.;
  int type1 = 0;
  double deltaR2_min = 999.;
  int type2 = 0;
  int jet0flav = -1;
  int jet1flav = -1;
  // ORDERING IN CxAOD
  // ttbar evt --> 0: top, 1: anti-top, 2: W+, 3: b, 4: W-, 5: anti-b, 6-9:
  // decay products of the Ws
  for (auto* part : *m_truthParts) {
    index++;
    // std::cout << " index = " << index << " pdg = " << part->pdgId() <<
    // std::endl;
    TLorentzVector truthp;
    truthp.SetPtEtaPhiM(part->pt(), part->eta(), part->phi(), part->m());
    // SANITY CHECKS FOR TTBAR EVENT //
    if (m_tree->sample.find("ttbar") != std::string::npos) {
      if (((index == 0 || index == 1) && fabs(part->pdgId()) != 6) ||
          ((index == 2 || index == 4) &&
           (fabs(part->pdgId()) != 24 && fabs(part->pdgId()) != 5)) ||
          ((index == 3 || index == 5) &&
           (fabs(part->pdgId()) != 5 && fabs(part->pdgId()) != 24))) {
        std::cout << "Found a weird truth event, particle with index " << index
                  << " has pdgId = " << part->pdgId() << std::endl;
      }
    }
    // MATCH JETS TO TRUE OBJECTS
    if (m_tree->sample.find("ttbar") != std::string::npos) {
      double deltaR1 = jet1.DeltaR(truthp);
      bool match1 = false;
      double deltaR2 = jet2.DeltaR(truthp);
      bool match2 = false;
      if (fabs(part->pdgId()) != 6 && fabs(part->pdgId()) != 24 &&
          fabs(part->pdgId()) != 12 && fabs(part->pdgId()) != 14 &&
          fabs(part->pdgId()) != 16) {
        if (deltaR1 < 0.4 && deltaR1 < deltaR1_min) match1 = true;
        if (deltaR2 < 0.4 && deltaR2 < deltaR2_min) match2 = true;
        if (match1 && match2) {
          if (deltaR1 < deltaR2)
            match2 = false;
          else
            match1 = false;
        }
        if (match1) {
          deltaR1_min = deltaR1;
          type1 = part->pdgId();
        }
        if (match2) {
          deltaR2_min = deltaR2;
          type2 = part->pdgId();
        }
      }  // exclude W, top, neutrinos from match
    }    // check it's ttbar sample
    // NOW LOOKING AT W DECAY
    if (m_tree->sample.find("ttbar") != std::string::npos && index < 6)
      continue;
    pdgIdPart.push_back(part->pdgId());
    // GET FINAL STATE OBJECTS //
    // CHECK IF LEPTONIC DECAY
    if (fabs(part->pdgId()) == 11) {
      nEl++;
    } else if (fabs(part->pdgId()) == 13) {
      nMu++;
    } else if (fabs(part->pdgId()) == 15) {
      nTau++;
    }
  }  // loop on truth particles
  // std::cout << "match1: pdg id = " << type1 << " deltaR = " << deltaR1_min <<
  // std::endl; std::cout << "match2: pdg id = " << type2 << " deltaR = " <<
  // deltaR2_min << std::endl;
  // SET JET MATCHES
  int jettype1 = jettype::other;
  if (fabs(type1) == 5)
    jettype1 = jettype::b;
  else if (fabs(type1) == 4)
    jettype1 = jettype::c;
  else if (fabs(type1) == 15)
    jettype1 = jettype::taujet;
  else if (fabs(type1) == 11)
    jettype1 = jettype::eljet;
  else if (fabs(type1) == 13)
    jettype1 = jettype::mujet;
  else {
    jettype1 = jettype::l;
  }
  //
  int jettype2 = jettype::other;
  if (fabs(type2) == 5)
    jettype2 = jettype::b;
  else if (fabs(type2) == 4)
    jettype2 = jettype::c;
  else if (fabs(type2) == 15)
    jettype2 = jettype::taujet;
  else if (fabs(type2) == 11)
    jettype2 = jettype::eljet;
  else if (fabs(type2) == 13)
    jettype2 = jettype::mujet;
  else {
    jettype2 = jettype::l;
  }
  // SET TRUE JET FLAVOUR (MATCH DONE WHEN PRODUCING CxAODs
  if (jets.size() > 0) {
    int tmp = Props::HadronConeExclTruthLabelID.get(jets.at(0));
    if (tmp == 5)
      jet0flav = jettype::b;
    else if (tmp == 4)
      jet0flav = jettype::c;
    else
      jet0flav = jettype::l;
  }
  if (jets.size() > 1) {
    int tmp = Props::HadronConeExclTruthLabelID.get(jets.at(1));
    if (tmp == 5)
      jet1flav = jettype::b;
    else if (tmp == 4)
      jet1flav = jettype::c;
    else
      jet1flav = jettype::l;
  }
  // W DECAYS
  int mydecay = decaytype::undefined;
  if (nEl == 2) {
    mydecay = decaytype::elel;
  } else if (nMu == 2) {
    mydecay = decaytype::mumu;
  } else if (nTau == 2) {
    mydecay = decaytype::tautau;
  } else if (nEl == 1) {
    if (nTau == 1)
      mydecay = decaytype::eltau;
    else if (nMu == 1)
      mydecay = decaytype::elmu;
    else
      mydecay = decaytype::el;
  } else if (nMu == 1) {
    if (nTau == 1)
      mydecay = decaytype::mutau;
    else
      mydecay = decaytype::mu;
  } else if (nTau == 1) {
    mydecay = decaytype::tau;
  }
  // m_histNameSvc->set_description("mBBincl");
  // FULLY INCLUSIVE
  m_histNameSvc->set_nTag(-1);  // b-tag
  m_histNameSvc->set_nJet(-1);
  m_histSvc->BookFillHist("trueDecayType", 11, 0, 11, mydecay, m_weight);
  m_histSvc->BookFillHist("j1vsj2match", 10, 0, 10, 10, 0, 10, jettype1,
                          jettype2, m_weight);
  m_histSvc->BookFillHist("j1vsj2flav", 10, 0, 10, 10, 0, 10, jet0flav,
                          jet1flav, m_weight);
  // PER-JET REGIONS
  m_histNameSvc->set_nTag(-1);  // b-tag
  m_histNameSvc->set_nJet(m_physicsMeta.nJets);
  m_histSvc->BookFillHist("trueDecayType", 11, 0, 11, mydecay, m_weight);
  m_histSvc->BookFillHist("j1vsj2match", 10, 0, 10, 10, 0, 10, jettype1,
                          jettype2, m_weight);
  m_histSvc->BookFillHist("j1vsj2flav", 10, 0, 10, 10, 0, 10, jet0flav,
                          jet1flav, m_weight);
  // PER-JET REGIONS AND PER-TAG
  m_histNameSvc->set_nTag(nTag);  // b-tag
  m_histNameSvc->set_nJet(m_physicsMeta.nJets);
  m_histSvc->BookFillHist("trueDecayType", 11, 0, 11, mydecay, m_weight);
  m_histSvc->BookFillHist("j1vsj2match", 10, 0, 10, 10, 0, 10, jettype1,
                          jettype2, m_weight);
  m_histSvc->BookFillHist("j1vsj2flav", 10, 0, 10, 10, 0, 10, jet0flav,
                          jet1flav, m_weight);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ0Lep::finalizeTools() {
  for (auto reader_it : m_mvaVHbbApps) delete reader_it.second;

  EL_CHECK("AnalysisReader_VHQQ::finalizeTools()",
           AnalysisReader_VHQQ::finalizeTools());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ0Lep::fill0LepHistWithModulableCuts(
    const unsigned long int eventFlag) {
  // this function allows drawing histograms with different cuts than standard
  // ones you typically can skip cuts by commenting them, set additional flags,
  // ... default is to have the SM resolved cut based analysis
  std::string defaultDescription = m_histNameSvc->get_description();
  HistNameSvc::AnalysisType defaultAnalysisType =
      m_histNameSvc->get_analysisType();
  if (defaultAnalysisType == HistNameSvc::AnalysisType::CUT)
    return (EL::StatusCode::SUCCESS);  // default selection here is also CUT

  std::vector<unsigned long int> cuts_SRcuts = {
      ZeroLeptonCuts::AllCxAOD,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::METResolved,
      ZeroLeptonCuts::DPhiMETMPT,
      ZeroLeptonCuts::AtLeast2Jets,
      ZeroLeptonCuts::AtLeast2SigJets,
      ZeroLeptonCuts::MinDPhiMETJetsResolved,
      ZeroLeptonCuts::SumJetPt,
      ZeroLeptonCuts::MinMPTBJet0or1Resolved,
      ZeroLeptonCuts::LeadJetPt,
      ZeroLeptonCuts::DPhiBB,
      ZeroLeptonCuts::DPhiMETDijetResolved,
      ZeroLeptonCuts::DRB1B2};

  if (passSpecificCuts(eventFlag, cuts_SRcuts)) {
    m_histNameSvc->set_description("SRcuts");
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
    m_histNameSvc->set_nTag(m_physicsMeta.nTags);  // b-tag
    fill0LepHistResolved();
  }

  m_histNameSvc->set_description(defaultDescription);
  m_histNameSvc->set_analysisType(defaultAnalysisType);

  return (EL::StatusCode::SUCCESS);
}  // AnalysisReader_VHQQ0Lep::fill0LepHistWithModulableCuts
