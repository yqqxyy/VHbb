#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ2Lep.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"

#include "CxAODTools_VHbb/TriggerTool_VHbb2lep.h"
#include "CxAODTools_VHbb/VHbb2lepEvtSelection.h"

#include "KinematicFit/KinematicFit.h"

#include <TMVA/Reader.h>
#include "TXMLEngine.h"

#include <iomanip>

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHQQ2Lep::AnalysisReader_VHQQ2Lep()
    : AnalysisReader_VHQQ(), m_doMergeModel(false) {}

AnalysisReader_VHQQ2Lep::~AnalysisReader_VHQQ2Lep() {}

EL::StatusCode AnalysisReader_VHQQ2Lep::initializeSelection() {
  EL_CHECK("AnalysisReader_VHQQ2Lep::initializeSelection ()",
           AnalysisReader_VHQQ::initializeSelection());

  Info("AnalysisReader_VHQQ2Lep::initializeSelection()",
       "Initializing 2 lepton selection");
  m_eventSelection = new VHbb2lepEvtSelection(m_config);
  m_fillFunction = std::bind(&AnalysisReader_VHQQ2Lep::run_2Lep_analysis, this);

  m_config->getif<bool>("doMergeModel", m_doMergeModel);
  m_config->getif<bool>("doRemoveDilepOverlap", m_doRemoveDilepOverlap);

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

EL::StatusCode AnalysisReader_VHQQ2Lep::initializeTools() {
  EL_CHECK("AnalysisReader_VHQQ2Lep::initializeTools()",
           AnalysisReader_VHQQ::initializeTools());

  m_triggerTool = new TriggerTool_VHbb2lep(*m_config);
  EL_CHECK("AnalysisReader_VHQQ2Lep::initializeTools()",
           m_triggerTool->initialize());

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
  }

  m_mvaVHbbFullRun2oldDefault = new MVAApplication_TMVA(
      m_analysisType, MVAType::ichepStyleIncl1LepMedium, "mvaOldDefault");
  m_mvaVHbbFullRun2oldDefault->Initialise(
      "$WorkDir_DIR/data/CxAODReader_VHbb/BDT_fullTruthTag_polVars_ICHEPstyle/",
      m_tree);
  m_mvaVHbbApps["mvaOldDefault"] = m_mvaVHbbFullRun2oldDefault;

  m_mvaVHbbFullRun2oldDefaultVZ =
      new MVAApplication_TMVA(m_analysisType, MVAType::ichepStyleIncl1LepMedium,
                              "mvadibosonOldDefault");
  m_mvaVHbbFullRun2oldDefaultVZ->Initialise(
      "$WorkDir_DIR/data/CxAODReader_VHbb/BDT_fullTruthTag_ICHEPstyle_diboson/"
      "2L/",
      m_tree);
  m_mvaVHbbApps["mvadibosonOldDefault"] = m_mvaVHbbFullRun2oldDefaultVZ;

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHQQ2Lep::setLeptonVariables(Lepton &l1, Lepton &l2,
                                                 const xAOD::Electron *el1,
                                                 const xAOD::Electron *el2,
                                                 const xAOD::Muon *mu1,
                                                 const xAOD::Muon *mu2) {
  if (mu1 && mu2) {
    l1.vec = mu1->p4();
    l2.vec = mu2->p4();
    l1.vec_resc = mu1->p4();
    l2.vec_resc = mu2->p4();
    // rescale only muons
    rescale_leptons((l1.vec + l2.vec).M(), l1.vec_resc, l2.vec_resc);
    l1.charge = Props::charge.get(mu1);
    l2.charge = Props::charge.get(mu2);
    l1.flav = lepFlav::mu;
    l2.flav = lepFlav::mu;
    // l1.isZHTight = ( Props::muonQuality.get(mu1) >=
    // xAOD::Muon::Quality::Tight); l2.isZHTight = ( Props::muonQuality.get(mu2)
    // >= xAOD::Muon::Quality::Tight);
    l1.resolution = Props::resolution.get(mu1);
    l2.resolution = Props::resolution.get(mu2);
  } else if (el1 && el2) {
    l1.vec = el1->p4();
    l2.vec = el2->p4();
    l1.vec_resc = el1->p4();
    l2.vec_resc = el2->p4();
    l1.charge = Props::charge.get(el1);
    l2.charge = Props::charge.get(el2);
    l1.flav = lepFlav::el;
    l2.flav = lepFlav::el;
    l1.isZHTight = Props::isMediumLH.get(el1);
    l2.isZHTight = Props::isMediumLH.get(el2);
    l1.resolution = Props::resolution.get(el1);
    l2.resolution = Props::resolution.get(el2);
  } else {
    if (mu1->p4().Pt() > el1->p4().Pt()) {
      l1.vec = mu1->p4();
      l2.vec = el1->p4();
      l1.vec_resc = mu1->p4();
      l2.vec_resc = el1->p4();
      l1.charge = Props::charge.get(mu1);
      l2.charge = Props::charge.get(el1);
      l1.flav = lepFlav::mu;
      l2.flav = lepFlav::el;
      // l1.isZHTight = ( Props::muonQuality.get(mu1) >=
      // xAOD::Muon::Quality::Tight);
      l2.isZHTight = Props::isMediumLH.get(el1);
      l1.resolution = Props::resolution.get(mu1);
      l2.resolution = Props::resolution.get(el1);
    } else {
      l1.vec = el1->p4();
      l2.vec = mu1->p4();
      l1.vec_resc = el1->p4();
      l2.vec_resc = mu1->p4();
      l1.charge = Props::charge.get(el1);
      l2.charge = Props::charge.get(mu1);
      l1.flav = lepFlav::el;
      l2.flav = lepFlav::mu;
      l1.isZHTight = Props::isMediumLH.get(el1);
      // l2.isZHTight = ( Props::muonQuality.get(mu1) >=
      // xAOD::Muon::Quality::Tight);
      l1.resolution = Props::resolution.get(el1);
      l2.resolution = Props::resolution.get(mu1);
    }
  }

  l1.vec_kf = l1.vec;
  l2.vec_kf = l2.vec;

}  // setLeptonVariables

bool AnalysisReader_VHQQ2Lep::pass2LepTrigger(double &triggerSF_nominal,
                                              ResultVHbb2lep selectionResult) {
  const xAOD::Electron *el1 = selectionResult.el1;
  const xAOD::Electron *el2 = selectionResult.el2;
  const xAOD::Muon *mu1 = selectionResult.mu1;
  const xAOD::Muon *mu2 = selectionResult.mu2;
  const xAOD::MissingET *met = selectionResult.met;

  bool isMu = (mu1 && mu2);
  bool isE = (el1 && el2);

  CP::SystematicSet sysSet;
  EL_CHECK("pass2LepTrigger", m_triggerTool->applySystematicVariation(sysSet));
  m_triggerTool->setEventInfo(m_eventInfo, m_randomRunNumber);
  m_triggerTool->setElectrons({el1, el2});
  m_triggerTool->setMuons({mu1, mu2});
  m_triggerTool->setMET(met);

  bool triggerDec;
  if (m_doMETTriggerin2L || m_doMETMuonTrigger) {
    triggerDec = ((TriggerTool_VHbb2lep *)m_triggerTool)
                     ->getDecisionAndSFwithMET(triggerSF_nominal);
    // Electron Channel: Single Lepton Trigger
    // Muon Channel: MET Trigger or MET + Muon Trigger
  } else {
    triggerDec = m_triggerTool->getDecisionAndScaleFactor(triggerSF_nominal);
    // Electron Channel: Single Lepton Trigger
    // Muon Channel: Single Lepton Trigger
  }

  if (!triggerDec) return false;

  // handle systematics
  if (m_isMC && (m_currentVar == "Nominal")) {
    for (size_t i = 0; i < m_triggerSystList.size(); i++) {
      // not computing useless systematics
      if (isE && (m_triggerSystList.at(i).find("MUON_EFF_Trig") !=
                  std::string::npos)) {
        m_weightSysts.push_back({m_triggerSystList.at(i), 1.0});
        continue;
      }

      if (isMu &&
          (m_triggerSystList.at(i).find("EL_EFF_Trig") != std::string::npos)) {
        m_weightSysts.push_back({m_triggerSystList.at(i), 1.0});
        continue;
      }

      // get decision + weight
      double triggerSF = 1.;

      CP::SystematicSet sysSet(m_triggerSystList.at(i));
      EL_CHECK("pass2LepTrigger",
               m_triggerTool->applySystematicVariation(sysSet));
      if (m_doMETTriggerin2L || m_doMETMuonTrigger) {
        ((TriggerTool_VHbb2lep *)m_triggerTool)
            ->getDecisionAndSFwithMET(triggerSF);
      } else {
        m_triggerTool->getDecisionAndScaleFactor(triggerSF);
      }

      if (triggerSF_nominal > 0)
        m_weightSysts.push_back(
            {m_triggerSystList.at(i), (float)(triggerSF / triggerSF_nominal)});
      else
        Error("run_2Lep_analysis()",
              "Nominal trigger SF=0!, The systematics will not be generated.");
    }
  }

  return true;
}  // pass2LepTrigger

void AnalysisReader_VHQQ2Lep::checkMllCut(const TLorentzVector &ZVec,
                                          const TLorentzVector &VHVecResolved,
                                          const TLorentzVector &VHVecMerged,
                                          bool &passMllResolved,
                                          bool &passMllMerged) {
  float lowMllcut_resolved =
      lowMllCut(VHVecResolved.M());  // returns cut value in GeV
  float upMllcut_resolved = upMllCut(VHVecResolved.M());
  float lowMllcut_merged = lowMllCut(VHVecMerged.M());
  float upMllcut_merged = upMllCut(VHVecMerged.M());

  // Sherpa samples have at truth level lower mLL cut applied mLL > 40 GeV
  //-> restrict cut
  if (lowMllcut_resolved < 40.) lowMllcut_resolved = 40.;
  if (lowMllcut_merged < 40.) lowMllcut_merged = 40.;

  // resolved
  if (ZVec.M() / 1000. > lowMllcut_resolved &&
      ZVec.M() / 1000. < upMllcut_resolved)
    passMllResolved = true;
  else
    passMllResolved = false;

  // merged - mVH-dependent cuts
  if (ZVec.M() / 1000. > lowMllcut_merged && ZVec.M() / 1000. < upMllcut_merged)
    passMllMerged = true;
  else
    passMllMerged = false;

}  // checkMllCut

float AnalysisReader_VHQQ2Lep::lowMllCut(const float mVH) {
  return -0.03 * mVH / 1000. + 87;

}  // lowMllCut

float AnalysisReader_VHQQ2Lep::upMllCut(const float mVH) {
  return 0.013 * mVH / 1000. + 97;

}  // upMllCut

float AnalysisReader_VHQQ2Lep::METHTcut(const float mVH) {
  return 0.008 * mVH / 1000. + 1.15;

}  // METHTcut

void AnalysisReader_VHQQ2Lep::checkMbbWindow(
    const TLorentzVector &HVec_resolved, const TLorentzVector &HVec_merged,
    bool &passMHResolved, bool &passMHMerged, bool &inMHSideBandResolved,
    bool &inMHSideBandMerged) {
  // SR cuts
  float lowCutTight = 100e3;
  float lowCutLoose = 75e3;
  float upCut = 145e3;
  float upBound = 200e3;
  float lowBound = 50e3;
  // schematic:  |--outer--|--low--|SR|--high--|--outer--|

  // set everything to false in the beginning - that way we only need to set to
  // true the cuts that pass
  passMHResolved = false;
  passMHMerged = false;
  inMHSideBandResolved = false;
  inMHSideBandMerged = false;

  //---------
  // resolved

  if ((HVec_resolved.M() >= lowCutTight) && (HVec_resolved.M() < upCut))
    passMHResolved = true;
  else {
    if (HVec_resolved.M() >= lowBound && HVec_resolved.M() < lowCutTight) {
      inMHSideBandResolved = true;
      m_physicsMeta.mbbSideBandResolved =
          PhysicsMetadata::MbbSideBandResolved::Low;
    } else if (HVec_resolved.M() >= upCut && HVec_resolved.M() < upBound) {
      inMHSideBandResolved = true;
      m_physicsMeta.mbbSideBandResolved =
          PhysicsMetadata::MbbSideBandResolved::High;
    } else {
      m_physicsMeta.mbbSideBandResolved =
          PhysicsMetadata::MbbSideBandResolved::Outer;
    }
  }

  //---------
  // merged
  if ((HVec_merged.M() > lowCutLoose) && (HVec_merged.M() < upCut))
    passMHMerged = true;
  else {
    if (HVec_merged.M() >= lowBound && HVec_merged.M() < lowCutLoose) {
      inMHSideBandMerged = true;
      m_physicsMeta.mbbSideBandMerged = PhysicsMetadata::MbbSideBandMerged::Low;
    } else if (HVec_merged.M() >= upCut && HVec_merged.M() < upBound) {
      inMHSideBandMerged = true;
      m_physicsMeta.mbbSideBandMerged =
          PhysicsMetadata::MbbSideBandMerged::High;
    } else {
      m_physicsMeta.mbbSideBandMerged =
          PhysicsMetadata::MbbSideBandMerged::Outer;
    }
  }

}  // checkMbbWindow

void AnalysisReader_VHQQ2Lep::checkPTVCut(const TLorentzVector &ZVec,
                                          const TLorentzVector &VHVec_resolved,
                                          const TLorentzVector &VHVec_merged,
                                          bool &passPtvResolved,
                                          bool &passPtvMerged) {
  float pTVcut_resolved =
      pTVCut(VHVec_resolved.M());  // returns cut value in GeV
  float pTVcut_merged = pTVCut(VHVec_merged.M());

  // resolved
  if (ZVec.Pt() / 1000. > pTVcut_resolved)
    passPtvResolved = true;
  else
    passPtvResolved = false;

  // merged - mVH-dependent cuts
  if (ZVec.Pt() / 1000. > pTVcut_merged)
    passPtvMerged = true;
  else
    passPtvMerged = false;

}  // checkPTVCut

float AnalysisReader_VHQQ2Lep::pTVCut(const float mVH) {
  if (mVH / 1000. < 320)
    return 0.;
  else
    return 20 + 9 * sqrt(mVH / 1000. - 320);
}

void AnalysisReader_VHQQ2Lep::setTwoLeptonFlavour(const Lepton &l1,
                                                  const Lepton &l2) {
  if (l1.flav == l2.flav) {
    if (l1.flav == lepFlav::mu)
      m_physicsMeta.flavor = PhysicsMetadata::Flavor::MuMu;
    else
      m_physicsMeta.flavor = PhysicsMetadata::Flavor::ElEl;
  } else {
    if (l1.flav == lepFlav::mu)
      m_physicsMeta.flavor = PhysicsMetadata::Flavor::MuEl;
    else
      m_physicsMeta.flavor = PhysicsMetadata::Flavor::ElMu;
  }

}  // setTwoLeptonFlavour

void AnalysisReader_VHQQ2Lep::selectRegime(
    unsigned long int eventFlag, double ZpTV,
    std::vector<unsigned long int> cuts_SR_resolved,
    std::vector<unsigned long int> cuts_SR_merged,
    std::vector<unsigned long int> cuts_mBBcr_resolved,
    std::vector<unsigned long int> cuts_mBBcr_merged,
    std::vector<unsigned long int> cuts_top_resolved,
    std::vector<unsigned long int> cuts_top_merged) {
  bool has2pSigJets =
      passSpecificCuts(eventFlag, {DiLeptonCuts::AtLeast2SigJets});
  bool has1pFatJets = passSpecificCuts(eventFlag, {DiLeptonCuts::AtLeast1FJ});

  // --- Cuts for loose SR definition, only used for resolved+merged combination
  // studies --- //
  std::vector<unsigned long int> cuts_SR_resolved_loose = {
      DiLeptonCuts::AtLeast2SigJets, DiLeptonCuts::mHCorrResolved};
  std::vector<unsigned long int> cuts_SR_merged_loose = {
      DiLeptonCuts::mHCorrMerged, DiLeptonCuts::AtLeast1FJ,
      DiLeptonCuts::LeadFJ2TJ};

  if (m_analysisStrategy == "Resolved") {
    if (has2pSigJets) m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "Merged") {
    if (has1pFatJets) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "RecyclePtV") {
    // Give priority to merged above 500 GeV
    if (ZpTV / 1.e3 > 500.) {
      if (has1pFatJets)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      else if (has2pSigJets)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    } else {
      if (has2pSigJets)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      else if (has1pFatJets)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
  } else if (m_analysisStrategy == "SimpleMerge500") {
    // Only merged above 500 GeV
    if (ZpTV / 1.e3 >= 500.) {
      if (has1pFatJets) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    } else {
      if (has2pSigJets)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
  } else if (m_analysisStrategy == "PriorityResolved") {
    if (has2pSigJets)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (has1pFatJets)
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
    else if ((passSpecificCuts(eventFlag, cuts_mBBcr_resolved) ||
              passSpecificCuts(eventFlag, cuts_top_resolved))) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
    // 4. check if it goes into any of the merged regions
    else if ((passSpecificCuts(eventFlag, cuts_mBBcr_merged) ||
              passSpecificCuts(eventFlag, cuts_top_merged))) {
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
    else if ((passSpecificCuts(eventFlag, cuts_mBBcr_merged) ||
              passSpecificCuts(eventFlag, cuts_top_merged))) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
    // 4. check if it goes into any of the resolved regions
    else if ((passSpecificCuts(eventFlag, cuts_mBBcr_resolved) ||
              passSpecificCuts(eventFlag, cuts_top_resolved))) {
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
  } else if (m_analysisStrategy == "RecyclePtVSR") {
    if (ZpTV / 1.e3 <= 500.) {
      // 1. check if event can go into resolved SR
      if (passSpecificCuts(eventFlag, cuts_SR_resolved)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 2. check if event can go into merged SR
      else if (passSpecificCuts(eventFlag, cuts_SR_merged)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 3. check if event has >= 2 small jets
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_resolved) ||
                passSpecificCuts(eventFlag, cuts_top_resolved))) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 4. check if event has >= 1 fat-jet
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_merged) ||
                passSpecificCuts(eventFlag, cuts_top_merged))) {
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
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_merged) ||
                passSpecificCuts(eventFlag, cuts_top_merged))) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 4. check if event has >= 2 small jets
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_resolved) ||
                passSpecificCuts(eventFlag, cuts_top_resolved))) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
    }
  } else if (m_analysisStrategy == "RecyclePtVSRloose") {
    if (ZpTV / 1.e3 <= 500.) {
      // 1. check if event can go into resolved SR
      if (passSpecificCuts(eventFlag, cuts_SR_resolved_loose)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 2. check if event can go into merged SR
      else if (passSpecificCuts(eventFlag, cuts_SR_merged_loose)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 3. check if event has >= 2 small jets
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_resolved) ||
                passSpecificCuts(eventFlag, cuts_top_resolved))) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 4. check if event has >= 1 fat-jet
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_merged) ||
                passSpecificCuts(eventFlag, cuts_top_merged))) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
    } else {
      // 1. check if event can go into merged SR
      if (passSpecificCuts(eventFlag, cuts_SR_merged_loose)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 2. check if event can go into resolved SR
      else if (passSpecificCuts(eventFlag, cuts_SR_resolved_loose)) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
      // 3. check if event has >= 1 fat-jet
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_merged) ||
                passSpecificCuts(eventFlag, cuts_top_merged))) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      }
      // 4. check if event has >= 2 small jets
      else if ((passSpecificCuts(eventFlag, cuts_mBBcr_resolved) ||
                passSpecificCuts(eventFlag, cuts_top_resolved))) {
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      }
    }
  }
}  // selectRegime

EL::StatusCode AnalysisReader_VHQQ2Lep::getKFResolved(const int nSelJets,
                                                      Jet &j1, Jet &j2, Jet &j3,
                                                      Lepton &l1, Lepton &l2) {
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl) {
    m_KF->AddElectron("ELE1", l1.vec, l1.resolution);
    m_KF->AddElectron("ELE2", l2.vec, l2.resolution);
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu) {
    m_KF->AddMuon("MUO1", l1.vec, l1.resolution);
    m_KF->AddMuon("MUO2", l2.vec, l2.resolution);
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu) {
    m_KF->AddElectron("ELE1", l1.vec, l1.resolution);
    m_KF->AddMuon("MUO1", l2.vec, l2.resolution);
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) {
    m_KF->AddMuon("MUO1", l1.vec, l1.resolution);
    m_KF->AddElectron("ELE1", l2.vec, l2.resolution);
  }
  if (m_doICHEP) {
    if (j1.isTagged) {
      m_KF->AddJet("JET1", j1.vec_onemu, j1.resolution, "B", j1.isMu04);
    } else {
      m_KF->AddJet("JET1", j1.vec_onemu, j1.resolution, "A", j1.isMu04);
    }
    if (j2.isTagged) {
      m_KF->AddJet("JET2", j2.vec_onemu, j2.resolution, "B", j2.isMu04);
    } else {
      m_KF->AddJet("JET2", j2.vec_onemu, j2.resolution, "A", j2.isMu04);
    }
    if (nSelJets == 3) {
      if (j3.isTagged) {
        m_KF->AddJet("JET3", j3.vec_onemu, j3.resolution, "B", j3.isMu04);
      } else {
        m_KF->AddJet("JET3", j3.vec_onemu, j3.resolution, "A", j3.isMu04);
      }
    }
  } else {
    // VHbb run: apply transfer function only on tagged jets
    // VHcc run: apply transfer functions on both tagged and not tagged jets
    //       -> Shown to perform slightly better on VHcc
    // Implementation:
    //    - Steer configuration (VHbb or VHcc)
    //    - Define tagged options (B) and not tagged option (A) for VHbb
    //    - Define tagged and not tagged options (B) for VHcc
    string tagged_option = "B";      // default for VHbb
    string not_tagged_option = "A";  // default for VHbb

    if (m_kfConfig == "2019c") {
      tagged_option = "B";
      not_tagged_option = "B";
    }

    if (j1.isTagged) {
      m_KF->AddJet("JET1", j1.vec_onemu, j1.resolution, tagged_option, j1.isMu);
    } else {
      m_KF->AddJet("JET1", j1.vec_onemu, j1.resolution, not_tagged_option,
                   j1.isMu);
    }
    if (j2.isTagged) {
      m_KF->AddJet("JET2", j2.vec_onemu, j2.resolution, tagged_option, j2.isMu);
    } else {
      m_KF->AddJet("JET2", j2.vec_onemu, j2.resolution, not_tagged_option,
                   j2.isMu);
    }
    if (nSelJets == 3) {
      if (j3.isTagged) {
        m_KF->AddJet("JET3", j3.vec_onemu, j3.resolution, tagged_option,
                     j3.isMu);
      } else {
        m_KF->AddJet("JET3", j3.vec_onemu, j3.resolution, not_tagged_option,
                     j3.isMu);
      }
    }
    TLorentzVector softTermVec;
    softTermVec.SetPxPyPzE(-1.0 * Props::soft_mpx.get(m_met),
                           -1.0 * Props::soft_mpy.get(m_met), 0, 0);
    m_KF->AddSoftTerm("SoftTerm", softTermVec, 10.0);
  }

  if (!m_KF->CheckParticlesOk()) {
    m_KF->EventReset();
    return EL::StatusCode::SUCCESS;
  }
  if (!m_KF->RunFit()) {
    Error("AnalysisReader_VHQQ2Lep::getKFResolved",
          "Failed to run KF! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if (!m_KF->IsFitConverged()) {
    // Info("VHQQ2Lep::getKFResolved()", "!m_KF->IsFitConverged() -> skip");
    return EL::StatusCode::SUCCESS;
  }

  j1.vec_kf = m_KF->GetFitTLVector("JET1");
  j2.vec_kf = m_KF->GetFitTLVector("JET2");
  if (nSelJets == 3) {
    j3.vec_kf = m_KF->GetFitTLVector("JET3");
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl) {
    l1.vec_kf = m_KF->GetFitTLVector("ELE1");
    l2.vec_kf = m_KF->GetFitTLVector("ELE2");
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu) {
    l1.vec_kf = m_KF->GetFitTLVector("MUO1");
    l2.vec_kf = m_KF->GetFitTLVector("MUO2");
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu) {
    l1.vec_kf = m_KF->GetFitTLVector("ELE1");
    l2.vec_kf = m_KF->GetFitTLVector("MUO1");
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) {
    l1.vec_kf = m_KF->GetFitTLVector("MUO1");
    l2.vec_kf = m_KF->GetFitTLVector("ELE1");
  }

  m_KFPrefitLHD = m_KF->GetPrefitParameter(KF::Idx::TotLHD);
  m_KFPostfitLHD = m_KF->GetPostfitParameter(KF::Idx::TotLHD);

  return EL::StatusCode::SUCCESS;

}  // getKFResolved

EL::StatusCode AnalysisReader_VHQQ2Lep::getKFMerged(
    const int njets, Jet &fj1, Lepton &l1, Lepton &l2,
    std::vector<const xAOD::Jet *> &signalJets,
    std::vector<const xAOD::Jet *> &forwardJets) {
  if ((unsigned)njets != signalJets.size() + forwardJets.size()) {
    Info("VHQQ2Lep::getKFMerged()",
         "njets!=signalJets.size()+forwardJets.size()");
    return EL::StatusCode::SUCCESS;
  }

  int nAddJet = 0;
  for (const xAOD::Jet *jet : signalJets) {
    TLorentzVector tlv = jet->p4();
    if (fj1.vec_onemu.DeltaR(tlv) > 1.0) {
      nAddJet += 1;
      m_KF->AddJet("JET" + std::to_string(nAddJet), tlv,
                   Props::resolution.get(jet), "A",
                   Props::nrMuonInJet.get(jet) > 0);
    }
  }
  for (const xAOD::Jet *jet : forwardJets) {
    TLorentzVector tlv = jet->p4();
    if (fj1.vec_onemu.DeltaR(tlv) > 1.0) {
      nAddJet += 1;
      m_KF->AddJet("JET" + std::to_string(nAddJet), tlv,
                   Props::resolution.get(jet), "A",
                   Props::nrMuonInJet.get(jet) > 0);
    }
  }

  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl) {
    m_KF->AddElectron("ELE1", l1.vec, l1.resolution);
    m_KF->AddElectron("ELE2", l2.vec, l2.resolution);
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu) {
    m_KF->AddMuon("MUO1", l1.vec, l1.resolution);
    m_KF->AddMuon("MUO2", l2.vec, l2.resolution);
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu) {
    m_KF->AddElectron("ELE1", l1.vec, l1.resolution);
    m_KF->AddMuon("MUO1", l2.vec, l2.resolution);
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) {
    m_KF->AddMuon("MUO1", l1.vec, l1.resolution);
    m_KF->AddElectron("ELE1", l2.vec, l2.resolution);
  }
  m_KF->AddJet("FAT1", fj1.vec_onemu, fj1.resolution, "A", fj1.isSL);
  TLorentzVector softTermVec;
  softTermVec.SetPxPyPzE(-1.0 * Props::soft_mpx.get(m_met),
                         -1.0 * Props::soft_mpy.get(m_met), 0, 0);
  m_KF->AddSoftTerm("SoftTerm", softTermVec, 10.0);

  if (!m_KF->CheckParticlesOk()) {
    m_KF->EventReset();
    return EL::StatusCode::SUCCESS;
  }

  if (!m_KF->RunFit(20.0)) {
    Error("VHQQ2Lep::getKFMerged", "Failed to run KF! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  if (!m_KF->IsFitConverged()) {
    // Info("VHQQ2Lep::getKFMerged()", "!m_KF->IsFitConverged() -> skip");
    return EL::StatusCode::SUCCESS;
  }

  fj1.vec_kf = m_KF->GetFitTLVector("FAT1");
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl) {
    l1.vec_kf = m_KF->GetFitTLVector("ELE1");
    l2.vec_kf = m_KF->GetFitTLVector("ELE2");
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu) {
    l1.vec_kf = m_KF->GetFitTLVector("MUO1");
    l2.vec_kf = m_KF->GetFitTLVector("MUO2");
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu) {
    l1.vec_kf = m_KF->GetFitTLVector("ELE1");
    l2.vec_kf = m_KF->GetFitTLVector("MUO1");
  }
  if (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) {
    l1.vec_kf = m_KF->GetFitTLVector("MUO1");
    l2.vec_kf = m_KF->GetFitTLVector("ELE1");
  }

  m_KFPrefitLHD = m_KF->GetPrefitParameter(KF::Idx::TotLHD);
  m_KFPostfitLHD = m_KF->GetPostfitParameter(KF::Idx::TotLHD);

  return EL::StatusCode::SUCCESS;

}  // getKFMerged

void AnalysisReader_VHQQ2Lep::setVHCandidate(VHCand &resolvedVH,
                                             VHCand &mergedVH, Lepton l1,
                                             Lepton l2, Jet j1, Jet j2,
                                             Jet fj1) {
  // get rescaled jets and leptons
  rescale_jets((j1.vec_corr + j2.vec_corr).M(), j1.vec_resc, j2.vec_resc);
  rescale_fatjet(fj1.vec_corr.M(), fj1.vec_resc);

  rescale_jets((j1.vec_gsc + j2.vec_gsc).M(), j1.vec_gsc, j2.vec_gsc);
  rescale_jets((j1.vec_onemu + j2.vec_onemu).M(), j1.vec_onemu, j2.vec_onemu);
  rescale_jets((j1.vec_ptreco + j2.vec_ptreco).M(), j1.vec_ptreco,
               j2.vec_ptreco);
  rescale_jets((j1.vec_kf + j2.vec_kf).M(), j1.vec_kf, j2.vec_kf);

  rescale_fatjet(fj1.vec_gsc.M(), fj1.vec_gsc);
  rescale_fatjet(fj1.vec_onemu.M(), fj1.vec_onemu);
  rescale_fatjet(fj1.vec_ptreco.M(), fj1.vec_ptreco);
  rescale_fatjet(fj1.vec_kf.M(), fj1.vec_kf);

  // built VH candidate
  TLorentzVector Zresc = l1.vec_resc + l2.vec_resc;  // rescale mumu for all
  // resolved
  resolvedVH.vec = Zresc + j1.vec + j2.vec;
  resolvedVH.vec_corr = Zresc + j1.vec_corr + j2.vec_corr;
  resolvedVH.vec_resc = Zresc + j1.vec_resc + j2.vec_resc;

  resolvedVH.vec_gsc = Zresc + j1.vec_gsc + j2.vec_gsc;
  resolvedVH.vec_onemu = Zresc + j1.vec_onemu + j2.vec_onemu;
  resolvedVH.vec_ptreco = Zresc + j1.vec_ptreco + j2.vec_ptreco;
  resolvedVH.vec_kf = Zresc + j1.vec_kf + j2.vec_kf;

  // merged
  mergedVH.vec = Zresc + fj1.vec;
  mergedVH.vec_corr = Zresc + fj1.vec_corr;
  mergedVH.vec_resc = Zresc + fj1.vec_resc;

  mergedVH.vec_gsc = Zresc + fj1.vec_gsc;
  mergedVH.vec_onemu = Zresc + fj1.vec_onemu;
  mergedVH.vec_ptreco = Zresc + fj1.vec_ptreco;
  mergedVH.vec_kf = Zresc + fj1.vec_kf;

}  // setVHCandidate

EL::StatusCode AnalysisReader_VHQQ2Lep::fill_2lepCutFlow(
    unsigned long int eventFlag, const int nJets, const int nTags,
    const int nFatJetTags, bool isMu, bool isE) {
  // systematics variations are skipped via histNameSvc
  std::string dir = "CutFlow/Nominal/";

  // Resolved cutlfow
  static std::string cuts[23] = {
      "All",          "Trigger",     "2LepSF",       "OSleptons",
      "mll",          "METHT",       "AtLeast2Jets", "AtLeast2SigJets",
      "pTB145",       "mbbCorr",     "PtZLT500",     "0tag_23jets",
      "0tag_4jets",   "0tag_5pjets", "1tag_23jets",  "1tag_4jets",
      "1tag_5pjets",  "2tag_23jets", "2tag_4jets",   "2tag_5pjets",
      "3ptag_23jets", "3ptag_4jets", "3ptag_5pjets"};

  std::vector<unsigned long int> cutsResolved = {
      DiLeptonCuts::AllCxAOD,
      DiLeptonCuts::Trigger,
      DiLeptonCuts::SFLeptons,
      DiLeptonCuts::OSLeptons,
      DiLeptonCuts::MllZwindowResolved,
      DiLeptonCuts::METHTResolved,
      DiLeptonCuts::AtLeast2Jets,
      DiLeptonCuts::AtLeast2SigJets,
      DiLeptonCuts::PtB145,
      DiLeptonCuts::mHCorrResolved,
      DiLeptonCuts::PtZLT500};
  std::vector<unsigned long int> incrementCuts;

  // All cuts up to PtZLT500 are appplied sequentially - if one
  for (unsigned long int i = 0; i < cutsResolved.size(); ++i) {
    incrementCuts.push_back(cutsResolved.at(i));
    if (!passSpecificCuts(eventFlag, incrementCuts)) break;

    std::string label = cuts[i];

    m_histSvc->BookFillCutHist(dir + "Cuts", length(cuts), cuts, label,
                               m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsNoWeight", length(cuts), cuts, label,
                               1.);

    if (isE) {
      m_histSvc->BookFillCutHist(dir + "CutsElec", length(cuts), cuts, label,
                                 m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsElecNoWeight", length(cuts), cuts,
                                 label, 1.);
    } else if (isMu) {
      m_histSvc->BookFillCutHist(dir + "CutsMuon", length(cuts), cuts, label,
                                 m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsMuonNoWeight", length(cuts), cuts,
                                 label, 1.);
    }
  }

  // Fill rest of bins based on ntags, njets, only for events that pass all
  // cuts in cutsResolved
  if (passSpecificCuts(eventFlag, cutsResolved)) {
    // Decide in which bin the event has to go based on ntags, njets
    int bin = 10;
    if (nTags == 1)
      bin += 3;
    else if (nTags == 2)
      bin += 6;
    else if (nTags >= 3)
      bin += 9;
    if (nJets == 2 || nJets == 3)
      bin += 1;
    else if (nJets == 4)
      bin += 2;
    else if (nJets >= 5)
      bin += 3;

    std::string label = cuts[bin];

    m_histSvc->BookFillCutHist(dir + "Cuts", length(cuts), cuts, label,
                               m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsNoWeight", length(cuts), cuts, label,
                               1.);

    if (isE) {
      m_histSvc->BookFillCutHist(dir + "CutsElec", length(cuts), cuts, label,
                                 m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsElecNoWeight", length(cuts), cuts,
                                 label, 1.);
    } else if (isMu) {
      m_histSvc->BookFillCutHist(dir + "CutsMuon", length(cuts), cuts, label,
                                 m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsMuonNoWeight", length(cuts), cuts,
                                 label, 1.);
    }
  }

  // Merged cutflow
  std::vector<std::string> cutsMergedNames = {};
  std::vector<unsigned long int> cutsMerged = {};

  // Weitao: change the array to vector, flexible, but push_back can be slow
  cutsMergedNames.push_back("All");
  cutsMerged.push_back(DiLeptonCuts::AllCxAOD);
  cutsMergedNames.push_back("Trigger");
  cutsMerged.push_back(DiLeptonCuts::Trigger);
  cutsMergedNames.push_back("LeadLepPt27");
  cutsMerged.push_back(DiLeptonCuts::LeadLepPt27);
  cutsMergedNames.push_back("SFLeptons");
  cutsMerged.push_back(DiLeptonCuts::SFLeptons);
  cutsMergedNames.push_back("OSLeptons");
  cutsMerged.push_back(DiLeptonCuts::OSLeptons);
  cutsMergedNames.push_back("MllZwindow");
  cutsMerged.push_back(DiLeptonCuts::MllZwindowMerged);
  cutsMergedNames.push_back("AtLeast1FJ");
  cutsMerged.push_back(DiLeptonCuts::AtLeast1FJ);
  cutsMergedNames.push_back("passVRJetOR");
  cutsMerged.push_back(DiLeptonCuts::passVRJetOR);
  cutsMergedNames.push_back("AtLeast2TJinFJ");
  cutsMerged.push_back(DiLeptonCuts::LeadFJ2TJ);
  if (m_analysis == "VHbb") {
    cutsMergedNames.push_back("MLeadFJ50");
    cutsMerged.push_back(DiLeptonCuts::MLeadFJ50);
    cutsMergedNames.push_back("DLepPt");
    cutsMerged.push_back(DiLeptonCuts::DLepPt);
    cutsMergedNames.push_back("DYVH");
    cutsMerged.push_back(DiLeptonCuts::DYVH);
  } else {
    cutsMergedNames.push_back("PtZGT500");
    cutsMerged.push_back(DiLeptonCuts::PtZGT500);
    cutsMergedNames.push_back("mbbCorr");
    cutsMerged.push_back(DiLeptonCuts::mHCorrMerged);
    cutsMergedNames.push_back("PassEtaCuts");
    cutsMerged.push_back(DiLeptonCuts::MuonEtaLT2p5);
  }

  // for different ntag categories
  cutsMergedNames.push_back("0tag_1pfat");
  cutsMergedNames.push_back("1tag_1pfat");
  cutsMergedNames.push_back("2tag_1pfat");
  cutsMergedNames.push_back("3ptag_1pfat");

  incrementCuts.clear();

  for (unsigned long int i = 0; i < cutsMerged.size(); ++i) {
    incrementCuts.push_back(cutsMerged.at(i));
    if (!passSpecificCuts(eventFlag, incrementCuts)) break;

    std::string label = cutsMergedNames[i];

    m_histSvc->BookFillCutHist(dir + "CutsMerged", cutsMergedNames.size(),
                               &cutsMergedNames[0], label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsMergedNoWeight",
                               cutsMergedNames.size(), &cutsMergedNames[0],
                               label, 1.);
  }

  // Fill rest of bins based on ntags, njets, only for events that pass all
  // cuts in cutsMerged
  if (passSpecificCuts(eventFlag, cutsMerged)) {
    // Decide in which bin the event has to go based on ntags, njets
    int bin = cutsMerged.size();
    if (nFatJetTags == 0)
      bin += 0;
    else if (nFatJetTags == 1)
      bin += 1;
    else if (nFatJetTags == 2)
      bin += 2;
    else if (nFatJetTags >= 3)
      bin += 3;

    std::string label = cutsMergedNames[bin];

    m_histSvc->BookFillCutHist(dir + "CutsMerged", cutsMergedNames.size(),
                               &cutsMergedNames[0], label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsMergedNoWeight",
                               cutsMergedNames.size(), &cutsMergedNames[0],
                               label, 1.);
  }

  // MVA cutflow

  static std::string cutsMVA_str[21] = {
      "All",         "LeadLepPt27", "Trigger",      "2LepSF",
      "OSleptons",   "mll",         "AtLeast2Jets", "AtLeast2SigJets",
      "pTB145",      "0tag_2jets",  "0tag_3jets",   "0tag_4pjets",
      "1tag_2jets",  "1tag_3jets",  "1tag_4pjets",  "2tag_2jets",
      "2tag_3jets",  "2tag_4pjets", "3ptag_2jets",  "3ptag_3jets",
      "3ptag_4pjets"};
  std::vector<unsigned long int> cutsMVA_int = {
      DiLeptonCuts::AllCxAOD,     DiLeptonCuts::LeadLepPt27,
      DiLeptonCuts::Trigger,      DiLeptonCuts::SFLeptons,
      DiLeptonCuts::OSLeptons,    DiLeptonCuts::MllZwindowResolved,
      DiLeptonCuts::AtLeast2Jets, DiLeptonCuts::AtLeast2SigJets,
      DiLeptonCuts::PtB145};
  incrementCuts.clear();

  for (unsigned long int i = 0; i < cutsMVA_int.size(); ++i) {
    incrementCuts.push_back(cutsMVA_int.at(i));
    if (!passSpecificCuts(eventFlag, incrementCuts)) break;
    std::string label = cutsMVA_str[i];
    m_histSvc->BookFillCutHist(dir + "CutsMVA", length(cutsMVA_str),
                               cutsMVA_str, label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsMVANoWeight", length(cutsMVA_str),
                               cutsMVA_str, label, 1.);
    if (isE) {
      m_histSvc->BookFillCutHist(dir + "CutsMVAElec", length(cutsMVA_str),
                                 cutsMVA_str, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsMVAElecNoWeight",
                                 length(cutsMVA_str), cutsMVA_str, label, 1.);
    } else if (isMu) {
      m_histSvc->BookFillCutHist(dir + "CutsMVAMuon", length(cutsMVA_str),
                                 cutsMVA_str, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsMVAMuonNoWeight",
                                 length(cutsMVA_str), cutsMVA_str, label, 1.);
    }
  }

  if (passSpecificCuts(eventFlag, cutsMVA_int)) {
    int bin = 8;
    if (nTags == 1)
      bin += 3;
    else if (nTags == 2)
      bin += 6;
    else if (nTags >= 3)
      bin += 9;
    if (nJets == 2)
      bin += 1;
    else if (nJets == 3)
      bin += 2;
    else if (nJets >= 4)
      bin += 3;
    std::string label = cutsMVA_str[bin];
    m_histSvc->BookFillCutHist(dir + "CutsMVA", length(cutsMVA_str),
                               cutsMVA_str, label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsMVANoWeight", length(cutsMVA_str),
                               cutsMVA_str, label, 1.);
    if (isE) {
      m_histSvc->BookFillCutHist(dir + "CutsMVAElec", length(cutsMVA_str),
                                 cutsMVA_str, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsMVAElecNoWeight",
                                 length(cutsMVA_str), cutsMVA_str, label, 1.);
    } else if (isMu) {
      m_histSvc->BookFillCutHist(dir + "CutsMVAMuon", length(cutsMVA_str),
                                 cutsMVA_str, label, m_weight);
      m_histSvc->BookFillCutHist(dir + "CutsMVAMuonNoWeight",
                                 length(cutsMVA_str), cutsMVA_str, label, 1.);
    }
  }

  // MVA cutflow for emu-CR

  static std::string cutsMVA_emuCR_str[21] = {
      "All",         "LeadLepPt27", "Trigger",      "2LepDF",
      "OSleptons",   "mll",         "AtLeast2Jets", "AtLeast2SigJets",
      "pTB145",      "0tag_2jets",  "0tag_3jets",   "0tag_4pjets",
      "1tag_2jets",  "1tag_3jets",  "1tag_4pjets",  "2tag_2jets",
      "2tag_3jets",  "2tag_4pjets", "3ptag_2jets",  "3ptag_3jets",
      "3ptag_4pjets"};
  std::vector<unsigned long int> cutsMVA_emuCR_int = {
      DiLeptonCuts::AllCxAOD,     DiLeptonCuts::LeadLepPt27,
      DiLeptonCuts::Trigger,      DiLeptonCuts::DFLeptons,
      DiLeptonCuts::OSLeptons,    DiLeptonCuts::MllZwindowResolved,
      DiLeptonCuts::AtLeast2Jets, DiLeptonCuts::AtLeast2SigJets,
      DiLeptonCuts::PtB145};
  incrementCuts.clear();

  for (unsigned long int i = 0; i < cutsMVA_emuCR_int.size(); ++i) {
    incrementCuts.push_back(cutsMVA_emuCR_int.at(i));
    if (!passSpecificCuts(eventFlag, incrementCuts)) break;
    std::string label = cutsMVA_emuCR_str[i];
    m_histSvc->BookFillCutHist(dir + "CutsMVAElMu", length(cutsMVA_emuCR_str),
                               cutsMVA_emuCR_str, label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsMVAElMuNoWeight",
                               length(cutsMVA_emuCR_str), cutsMVA_emuCR_str,
                               label, 1.);
  }

  if (passSpecificCuts(eventFlag, cutsMVA_emuCR_int)) {
    int bin = 8;
    if (nTags == 1)
      bin += 3;
    else if (nTags == 2)
      bin += 6;
    else if (nTags >= 3)
      bin += 9;
    if (nJets == 2)
      bin += 1;
    else if (nJets == 3)
      bin += 2;
    else if (nJets >= 4)
      bin += 3;
    std::string label = cutsMVA_emuCR_str[bin];
    m_histSvc->BookFillCutHist(dir + "CutsMVAElMu", length(cutsMVA_emuCR_str),
                               cutsMVA_emuCR_str, label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsMVAElMuNoWeight",
                               length(cutsMVA_emuCR_str), cutsMVA_emuCR_str,
                               label, 1.);

  }  // emuCR

  return EL::StatusCode::SUCCESS;
}  // fill_2lepCutFlow

EL::StatusCode AnalysisReader_VHQQ2Lep::fill_2LepHistos(bool isMu, bool isE) {
  if (!m_doBlinding && (m_model == Model::AZh || m_model == Model::HVT))
    m_histSvc->BookFillHist("mVH", 600, 0, 6000, m_etree->get<float>("mVH"),
                            m_weight);
  // if(!m_doBlinding) m_histSvc->BookFillHist("mVH_noW", 400, 0,  4000,
  // m_etree->get<float>("mVH") / 1000., 1.);

  if (!m_doOnlyInputs && !m_doReduceFillHistos) {
    m_histSvc->BookFillHist("AverageMu", 360, 0, 90,
                            m_etree->get<float>("AverageMu"), m_weight);
    m_histSvc->BookFillHist("ActualMu", 360, 0, 90,
                            m_etree->get<float>("ActualMu"), m_weight);
    m_histSvc->BookFillHist("AverageMuScaled", 360, 0, 90,
                            m_etree->get<float>("AverageMuScaled"), m_weight);
    m_histSvc->BookFillHist("ActualMuScaled", 360, 0, 90,
                            m_etree->get<float>("ActualMuScaled"), m_weight);
    if (isE)
      m_histSvc->BookFillHist("mVH_ee", 600, 0, 6000,
                              m_etree->get<float>("mVH"), m_weight);
    if (isMu)
      m_histSvc->BookFillHist("mVH_mm", 600, 0, 6000,
                              m_etree->get<float>("mVH"), m_weight);
    m_histSvc->BookFillHist("METHT", 30, 0, 15, m_etree->get<float>("METHT"),
                            m_weight);
    m_histSvc->BookFillHist("MET", 40, 0, 200, m_etree->get<float>("MET"),
                            m_weight);
    m_histSvc->BookFillHist("NVtx", 60, 0, 60, m_etree->get<int>("Nvtx"),
                            m_weight);
    m_histSvc->BookFillHist("ZPV", 400, -200, 200, m_etree->get<float>("ZPV"),
                            m_weight);
    m_histSvc->BookFillHist("pTL1", 200, 0, 1000, m_etree->get<float>("ptL1"),
                            m_weight);
    m_histSvc->BookFillHist("pTL2", 200, 0, 500, m_etree->get<float>("ptL2"),
                            m_weight);
    m_histSvc->BookFillHist("EtaL1", 100, -5, 5, m_etree->get<float>("etaL1"),
                            m_weight);
    m_histSvc->BookFillHist("EtaL2", 100, -5, 5, m_etree->get<float>("etaL2"),
                            m_weight);
    m_histSvc->BookFillHist("dEtaLL", 100, 0, 5, m_etree->get<float>("dEtaLL"),
                            m_weight);
    m_histSvc->BookFillHist("dPhiLL", 100, 0, 3.15,
                            m_etree->get<float>("dPhiLL"), m_weight);
    m_histSvc->BookFillHist("mLL", 120, 30, 150, m_etree->get<float>("mLL"),
                            m_weight);
    m_histSvc->BookFillHist("HT", 100, 0, 2000, m_etree->get<float>("HT"),
                            m_weight);

    m_histSvc->BookFillHist("pTVH", 200, 0, 1000, m_etree->get<float>("pTVH"),
                            m_weight);
    m_histSvc->BookFillHist("dPhiVH", 64, -3.2, 3.2,
                            m_etree->get<float>("dPhiVBB"), m_weight);
    m_histSvc->BookFillHist("dEtaVH", 50, 0, 5, m_etree->get<float>("dEtaVBB"),
                            m_weight);
    m_histSvc->BookFillHist("dRVH", 50, 0, 5, m_etree->get<float>("dRVBB"),
                            m_weight);
    m_histSvc->BookFillHist("mLB", 500, 0, 500, m_etree->get<float>("mLB"),
                            m_weight);

    if (isMu) {
      m_histSvc->BookFillHist("Mmumu", 60, 60, 120, m_etree->get<float>("mLL"),
                              m_weight);
      m_histSvc->BookFillHist("pTZmumu", 100, 0, 500,
                              m_etree->get<float>("pTV"), m_weight);
      m_histSvc->BookFillHist("EtaM1", 150, -3, 3, m_etree->get<float>("etaL1"),
                              m_weight);
      m_histSvc->BookFillHist("EtaM2", 150, -3, 3, m_etree->get<float>("etaL2"),
                              m_weight);
    } else if (isE) {
      m_histSvc->BookFillHist("Mee", 60, 60, 120, m_etree->get<float>("mLL"),
                              m_weight);
      m_histSvc->BookFillHist("pTZee", 100, 0, 500, m_etree->get<float>("pTV"),
                              m_weight);
      m_histSvc->BookFillHist("EtaE1", 150, -3, 3, m_etree->get<float>("etaL1"),
                              m_weight);
      m_histSvc->BookFillHist("EtaE2", 150, -3, 3, m_etree->get<float>("etaL2"),
                              m_weight);
    }
  }

  return EL::StatusCode::SUCCESS;
}  // fill_2LepHistos

// For the analysis that use MVA Tree, filling all histogram from MVA tree if
// the variables are inclueded in MVA tree
EL::StatusCode AnalysisReader_VHQQ2Lep::fill_MVAVariablesHistos2Lep(bool isMu,
                                                                    bool isE) {
  EL_CHECK("AnalysisReader_VHQQ2Lep::fill_MVAVariablesHistos2Lep()",
           AnalysisReader_VHQQ::fill_MVAVariablesHistos());
  if (m_doOnlyInputs) return EL::StatusCode::SUCCESS;
  if (m_doReduceFillHistos) return EL::StatusCode::SUCCESS;
  // "not important" plots, for debugging or other studies
  m_histSvc->BookFillHist("mLL", 120, 30, 150, m_tree->mLL, m_weight);
  m_histSvc->BookFillHist("pTL1", 200, 0, 1000, m_tree->pTL1, m_weight);
  m_histSvc->BookFillHist("pTL2", 200, 0, 500, m_tree->pTL2, m_weight);
  m_histSvc->BookFillHist("EtaL1", 100, -5, 5, m_tree->etaL1, m_weight);
  m_histSvc->BookFillHist("EtaL2", 100, -5, 5, m_tree->etaL2, m_weight);
  m_histSvc->BookFillHist("PhiL1", 100, -TMath::Pi(), TMath::Pi(),
                          m_tree->phiL1, m_weight);
  m_histSvc->BookFillHist("PhiL2", 100, -TMath::Pi(), TMath::Pi(),
                          m_tree->phiL2, m_weight);
  m_histSvc->BookFillHist("METSig", 30, 0, 15, m_tree->METSig, m_weight);
  m_histSvc->BookFillHist("cosThetaLep", 50, -1, 1, m_tree->cosThetaLep,
                          m_weight);
  // e mu separately
  if (isMu) {
    m_histSvc->BookFillHist("Mmumu", 60, 60, 120, m_tree->mLL, m_weight);
    m_histSvc->BookFillHist("pTZmumu", 100, 0, 500, m_tree->pTV, m_weight);
    m_histSvc->BookFillHist("EtaM1", 150, -3, 3, m_tree->etaL1, m_weight);
    m_histSvc->BookFillHist("EtaM2", 150, -3, 3, m_tree->etaL2, m_weight);
  } else if (isE) {
    m_histSvc->BookFillHist("Mee", 60, 60, 120, m_tree->mLL, m_weight);
    m_histSvc->BookFillHist("pTZee", 100, 0, 500, m_tree->pTV, m_weight);
    m_histSvc->BookFillHist("EtaE1", 150, -3, 3, m_tree->etaL1, m_weight);
    m_histSvc->BookFillHist("EtaE2", 150, -3, 3, m_tree->etaL2, m_weight);
  }
  return EL::StatusCode::SUCCESS;
}  // fill_MVAVariablesHistos2Lep

// fill the variables saved in easy tree but not in MVA tree
EL::StatusCode AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep(bool isMu,
                                                                bool isE) {
  // using in VHbb analysis, instead of fill_2LepHistos()
  if (m_doOnlyInputs) return EL::StatusCode::SUCCESS;
  if (m_doReduceFillHistos) return EL::StatusCode::SUCCESS;

  // from ETreeCommon
  m_histSvc->BookFillHist("AverageMu", 360, 0, 90,
                          m_etree->get<float>("AverageMu"), m_weight);
  m_histSvc->BookFillHist("ActualMu", 360, 0, 90,
                          m_etree->get<float>("ActualMu"), m_weight);
  m_histSvc->BookFillHist("AverageMuScaled", 360, 0, 90,
                          m_etree->get<float>("AverageMuScaled"), m_weight);
  m_histSvc->BookFillHist("ActualMuScaled", 360, 0, 90,
                          m_etree->get<float>("ActualMuScaled"), m_weight);
  m_histSvc->BookFillHist("NVtx", 60, 0, 60, m_etree->get<int>("Nvtx"),
                          m_weight);
  m_histSvc->BookFillHist("ZPV", 400, -200, 200, m_etree->get<float>("ZPV"),
                          m_weight);

  // from ETreeResolved
  m_histSvc->BookFillHist("mVH", 600, 0, 6000, m_etree->get<float>("mVH"),
                          m_weight);
  m_histSvc->BookFillHist("HT", 100, 0, 2000, m_etree->get<float>("HT"),
                          m_weight);
  m_histSvc->BookFillHist("METHT", 30, 0, 15, m_etree->get<float>("METHT"),
                          m_weight);
  m_histSvc->BookFillHist("pTVH", 200, 0, 1000, m_etree->get<float>("pTVH"),
                          m_weight);
  m_histSvc->BookFillHist("dRVH", 50, 0, 5, m_etree->get<float>("dRVBB"),
                          m_weight);
  m_histSvc->BookFillHist("mLB", 500, 0, 500, m_etree->get<float>("mLB"),
                          m_weight);

  // from ETreeSelectedJets
  int nJets = m_tree->nJ;
  m_histSvc->BookFillHist("pTBB", 400, 0, 2000, m_etree->get<float>("pTBB"),
                          m_weight);
  if (nJets >= 1)
    m_histSvc->BookFillHist("MV2c10B1", 100, -1, 1,
                            m_etree->get<float>("MV2c10B1"), m_weight);
  if (nJets >= 2)
    m_histSvc->BookFillHist("MV2c10B2", 100, -1, 1,
                            m_etree->get<float>("MV2c10B2"), m_weight);
  if (nJets >= 3)
    m_histSvc->BookFillHist("pTBBJ", 600, 0, 3000, m_etree->get<float>("pTBBJ"),
                            m_weight);

  // only for 2L
  m_histSvc->BookFillHist("dEtaLL", 100, 0, 5, m_etree->get<float>("dEtaLL"),
                          m_weight);
  m_histSvc->BookFillHist("dPhiLL", 100, 0, TMath::Pi(),
                          m_etree->get<float>("dPhiLL"), m_weight);

  // mBB Monitor
  if (m_doMbbMonitor) {
    EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
             fill_EasyTreeHistos2LepMbbMonitor(""));
    if (m_isSLEvent) {
      EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
               fill_EasyTreeHistos2LepMbbMonitor("Sem"));
    } else {
      EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
               fill_EasyTreeHistos2LepMbbMonitor("Had"));
    }
    if (isMu) {
      EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
               fill_EasyTreeHistos2LepMbbMonitor("Zuu"));
      if (m_isSLEvent) {
        EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
                 fill_EasyTreeHistos2LepMbbMonitor("ZuuSem"));
      } else {
        EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
                 fill_EasyTreeHistos2LepMbbMonitor("ZuuHad"));
      }
    }
    if (isE) {
      EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
               fill_EasyTreeHistos2LepMbbMonitor("Zee"));
      if (m_isSLEvent) {
        EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
                 fill_EasyTreeHistos2LepMbbMonitor("ZeeSem"));
      } else {
        EL_CHECK("AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2Lep",
                 fill_EasyTreeHistos2LepMbbMonitor("ZeeHad"));
      }
    }

  }  // m_doMbbMonitor

  return EL::StatusCode::SUCCESS;
}  // fill_EasyTreeHistos2Lep

EL::StatusCode AnalysisReader_VHQQ2Lep::fill_EasyTreeHistos2LepMbbMonitor(
    const string &eventType) {
  if (m_doFSRrecovery && m_hasFSR > 0.5) {
    m_histNameSvc->set_nJet(3);
  }
  m_histSvc->BookFillHist(eventType + "GSCMbb", 100, 0, 500,
                          m_etree->get<float>("GSCMbb"), m_weight);
  m_histSvc->BookFillHist(eventType + "OneMuMbb", 100, 0, 500,
                          m_etree->get<float>("OneMuMbb"), m_weight);
  m_histSvc->BookFillHist(eventType + "PtRecoMbb", 100, 0, 500,
                          m_etree->get<float>("PtRecoMbb"), m_weight);
  if (m_doFSRrecovery && m_hasFSR > 0.5) {
    m_histNameSvc->set_nJet(2);
  }
  m_histSvc->BookFillHist(eventType + "FSRMbb", 100, 0, 500,
                          m_etree->get<float>("FSRMbb"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFMbb", 100, 0, 500,
                          m_etree->get<float>("KFMbb"), m_weight);

  m_histSvc->BookFillHist(eventType + "GSCPtH", 100, 0, 1000,
                          m_etree->get<float>("GSCPtH"), m_weight);
  m_histSvc->BookFillHist(eventType + "OneMuPtH", 100, 0, 1000,
                          m_etree->get<float>("OneMuPtH"), m_weight);
  m_histSvc->BookFillHist(eventType + "PtRecoPtH", 100, 0, 1000,
                          m_etree->get<float>("PtRecoPtH"), m_weight);
  m_histSvc->BookFillHist(eventType + "FSRPtH", 100, 0, 1000,
                          m_etree->get<float>("FSRPtH"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFPtH", 100, 0, 1000,
                          m_etree->get<float>("KFPtH"), m_weight);

  // response
  m_histSvc->BookFillHist(eventType + "GSCMbbResp", 100, 0, 2,
                          m_etree->get<float>("GSCMbbResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "OneMuMbbResp", 100, 0, 2,
                          m_etree->get<float>("OneMuMbbResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "PtRecoMbbResp", 100, 0, 2,
                          m_etree->get<float>("PtRecoMbbResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "FSRMbbResp", 100, 0, 2,
                          m_etree->get<float>("FSRMbbResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFMbbResp", 100, 0, 2,
                          m_etree->get<float>("KFMbbResp"), m_weight);

  m_histSvc->BookFillHist(eventType + "GSCPtHResp", 100, 0, 2,
                          m_etree->get<float>("GSCPtHResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "OneMuPtHResp", 100, 0, 2,
                          m_etree->get<float>("OneMuPtHResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "PtRecoPtHResp", 100, 0, 2,
                          m_etree->get<float>("PtRecoPtHResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "FSRPtHResp", 100, 0, 2,
                          m_etree->get<float>("FSRPtHResp"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFPtHResp", 100, 0, 2,
                          m_etree->get<float>("KFPtHResp"), m_weight);

  // balance
  m_histSvc->BookFillHist(eventType + "GSCPxVH", 100, -100, 100,
                          m_etree->get<float>("GSCPxVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "OneMuPxVH", 100, -100, 100,
                          m_etree->get<float>("OneMuPxVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "PtRecoPxVH", 100, -100, 100,
                          m_etree->get<float>("PtRecoPxVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "FSRPxVH", 100, -100, 100,
                          m_etree->get<float>("FSRPxVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFPxVH", 100, -100, 100,
                          m_etree->get<float>("KFPxVH"), m_weight);

  m_histSvc->BookFillHist(eventType + "GSCPyVH", 100, -100, 100,
                          m_etree->get<float>("GSCPyVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "OneMuPyVH", 100, -100, 100,
                          m_etree->get<float>("OneMuPyVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "PtRecoPyVH", 100, -100, 100,
                          m_etree->get<float>("PtRecoPyVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "FSRPyVH", 100, -100, 100,
                          m_etree->get<float>("FSRPyVH"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFPyVH", 100, -100, 100,
                          m_etree->get<float>("KFPyVH"), m_weight);

  // object
  m_histSvc->BookFillHist(eventType + "KFpTL1", 100, 0, 1000,
                          m_etree->get<float>("KFptL1"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFpTL2", 100, 0, 500,
                          m_etree->get<float>("KFptL2"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFmLL", 120, 30, 150,
                          m_etree->get<float>("KFmLL"), m_weight);

  m_histSvc->BookFillHist(eventType + "KFpTJ1", 100, 0, 1000,
                          m_etree->get<float>("KFptJ1"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFpTJ2", 100, 0, 500,
                          m_etree->get<float>("KFptJ2"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFpTJ3", 100, 0, 500,
                          m_etree->get<float>("KFptJ3"), m_weight);

  m_histSvc->BookFillHist(eventType + "SoftMetPx", 100, -100, 100,
                          m_etree->get<float>("SoftMetPx"), m_weight);
  m_histSvc->BookFillHist(eventType + "SoftMetPy", 100, -100, 100,
                          m_etree->get<float>("SoftMetPy"), m_weight);

  m_histSvc->BookFillHist(eventType + "ResL1", 100, 0, 1,
                          m_etree->get<float>("ResL1"), m_weight);
  m_histSvc->BookFillHist(eventType + "ResL2", 100, 0, 1,
                          m_etree->get<float>("ResL2"), m_weight);
  m_histSvc->BookFillHist(eventType + "ResJ1", 100, 0, 1,
                          m_etree->get<float>("ResJ1"), m_weight);
  m_histSvc->BookFillHist(eventType + "ResJ2", 100, 0, 1,
                          m_etree->get<float>("ResJ2"), m_weight);
  m_histSvc->BookFillHist(eventType + "ResJ3", 100, 0, 1,
                          m_etree->get<float>("ResJ3"), m_weight);

  // others
  m_histSvc->BookFillHist(eventType + "KFPrefitLHD", 100, 0, 200,
                          m_etree->get<float>("KFPrefitLHD"), m_weight);
  m_histSvc->BookFillHist(eventType + "KFPostfitLHD", 100, 0, 200,
                          m_etree->get<float>("KFPostfitLHD"), m_weight);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHQQ2Lep::fill_jetHistos2Lep(
    std::vector<const xAOD::Jet *> signalJets,
    std::vector<const xAOD::Jet *> forwardJets) {
  // used in VHbb analysis, instead of fill_jetHistos to avoid double filling
  // with fill_MVAVariablesHistos(). Need to harmonize among VHcc and VHres
  // Reader.
  if (m_doOnlyInputs) return EL::StatusCode::SUCCESS;
  if (m_doReduceFillHistos) return EL::StatusCode::SUCCESS;
  for (const xAOD::Jet *jet : signalJets) {
    m_histSvc->BookFillHist("PtJets", 100, 0, 100, jet->pt() / 1e3, m_weight);
    m_histSvc->BookFillHist("EtaJets", 100, -5, 5, jet->eta(), m_weight);
  }

  for (const xAOD::Jet *jet : forwardJets) {
    m_histSvc->BookFillHist("PtJets", 100, 0, 100, jet->pt() / 1e3, m_weight);
    m_histSvc->BookFillHist("EtaJets", 100, -5, 5, jet->eta(), m_weight);
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
}  // fill_jetHistos2Lep

//********** Fill Easy Tree **********//
EL::StatusCode AnalysisReader_VHQQ2Lep::fillETreeCommon(float triggerSF,
                                                        float JVTweight,
                                                        float btagWeight) {
  // inherit from MVA tree, same order as fillMVATreeVHbbResolved()
  m_etree->SetBranchAndValue<std::string>("Sample", m_tree->sample, "");
  m_etree->SetBranchAndValue<int>("EventNumber", m_tree->EventNumber, -99);
  m_etree->SetBranchAndValue<float>("EventWeight", m_tree->EventWeight, -99);
  m_etree->SetBranchAndValue<int>("MCChannelNumber", m_tree->ChannelNumber,
                                  -99);

  m_etree->SetBranchAndValue<int>("nSigJets", m_tree->nSigJet, -1);
  m_etree->SetBranchAndValue<int>("nFwdJets", m_tree->nForwardJet, -1);
  m_etree->SetBranchAndValue<int>("nJets", m_tree->nJ, -1);
  m_etree->SetBranchAndValue<int>("nbJets", m_tree->nTags, -1);
  m_etree->SetBranchAndValue<int>("nTaus", m_tree->nTaus, -99);
  m_etree->SetBranchAndValue<float>("MET", m_tree->MET, -99);
  m_etree->SetBranchAndValue<float>("dPhiVBB", m_tree->dPhiVBB, -99);
  m_etree->SetBranchAndValue<float>("dEtaVBB", m_tree->dEtaVBB, -99);

  m_etree->SetBranchAndValue<float>("sumPtJets", m_tree->sumPtJets, -99);
  m_etree->SetBranchAndValue<float>("pTV", m_tree->pTV, -99);

  m_etree->SetBranchAndValue<float>("mBB", m_tree->mBB, -99);
  m_etree->SetBranchAndValue<float>("dRBB", m_tree->dRBB, -99);
  m_etree->SetBranchAndValue<float>("dEtaBB", m_tree->dEtaBB, -99);
  m_etree->SetBranchAndValue<float>("dPhiBB", m_tree->dPhiBB, -99);
  m_etree->SetBranchAndValue<float>("mBBJ", m_tree->mBBJ, -99);

  m_etree->SetBranchAndValue<float>("pTB1", m_tree->pTB1, -99);
  m_etree->SetBranchAndValue<float>("pTB2", m_tree->pTB2, -99);
  m_etree->SetBranchAndValue<float>("pTJ3", m_tree->pTJ3, -99);
  m_etree->SetBranchAndValue<float>("etaB1", m_tree->etaB1, -99);
  m_etree->SetBranchAndValue<float>("etaB2", m_tree->etaB2, -99);
  m_etree->SetBranchAndValue<float>("etaJ3", m_tree->etaJ3, -99);
  m_etree->SetBranchAndValue<float>("phiB1", m_tree->phiB1, -99);
  m_etree->SetBranchAndValue<float>("phiB2", m_tree->phiB2, -99);
  m_etree->SetBranchAndValue<float>("phiJ3", m_tree->phiJ3, -99);
  m_etree->SetBranchAndValue<float>("mB1", m_tree->mB1, -99);
  m_etree->SetBranchAndValue<float>("mB2", m_tree->mB2, -99);
  m_etree->SetBranchAndValue<float>("mJ3", m_tree->mJ3, -99);
  m_etree->SetBranchAndValue<float>("mva", m_tree->BDT, -99);
  m_etree->SetBranchAndValue<float>("mvadiboson", m_tree->BDT_VZ, -99);

  // Variables not included in MVA tree:
  string Regime = "none";
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    Regime = "merged";
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved)
    Regime = "resolved";

  m_etree->SetBranchAndValue<std::string>("Description",
                                          m_histNameSvc->get_description(), "");
  m_etree->SetBranchAndValue<std::string>("Regime", Regime, "none");
  m_etree->SetBranchAndValue<std::string>("EventFlavor",
                                          m_histNameSvc->getEventFlavour(), "");
  m_etree->SetBranchAndValue<float>("AverageMu", m_averageMu, -99);
  m_etree->SetBranchAndValue<float>("ActualMu", m_actualMu, -99);
  m_etree->SetBranchAndValue<float>("AverageMuScaled", m_averageMuScaled, -99);
  m_etree->SetBranchAndValue<float>("ActualMuScaled", m_actualMuScaled, -99);
  m_etree->SetBranchAndValue<int>("RunNumber", m_eventInfo->runNumber(), -99);

  float MCeventweight = 1;
  if (m_isMC) MCeventweight = Props::MCEventWeight.get(m_eventInfo);
  m_etree->SetBranchAndValue<float>("MCEventWeight", MCeventweight, -99999);
  m_etree->SetBranchAndValue<float>("LumiWeight",
                                    Props::LumiWeight.get(m_eventInfo), -99999);
  m_etree->SetBranchAndValue<float>("PUWeight", m_pileupReweight, -99999);
  m_etree->SetBranchAndValue<float>("LeptonSF",
                                    Props::leptonSF.get(m_eventInfo), -99999);

  float NTruthJetWeight = Props::NTruthWZJets20.exists(m_eventInfo)
                              ? Props::NTruthWZJets20.get(m_eventInfo)
                              : -99999;
  m_etree->SetBranchAndValue<float>("NTruthJetWeight", NTruthJetWeight, -99999);
  m_etree->SetBranchAndValue<float>("BTagSF", btagWeight, -99999);
  m_etree->SetBranchAndValue<float>("TriggerSF", triggerSF, -99999);
  m_etree->SetBranchAndValue<float>("LeptonSF",
                                    Props::leptonSF.get(m_eventInfo), -99999);
  m_etree->SetBranchAndValue<float>("JVTWeight", JVTweight, -99999);

  int Nvtx = 0;
  if (Props::NVtx2Trks.exists(m_eventInfo))
    Nvtx = Props::NVtx2Trks.get(m_eventInfo);
  else if (Props::NVtx3Trks.exists(m_eventInfo))
    Nvtx = Props::NVtx3Trks.get(m_eventInfo);
  m_etree->SetBranchAndValue<int>("Nvtx", Nvtx, -99);
  m_etree->SetBranchAndValue<float>("ZPV", Props::ZPV.get(m_eventInfo), -99);

  return EL::StatusCode::SUCCESS;
}  // fillETreeCommon

EL::StatusCode AnalysisReader_VHQQ2Lep::fillETreeSelectedJets(
    const std::vector<const xAOD::Jet *> &selectedJets) {
  if (selectedJets.size() >= 1) (*m_etree)["j1Vec"] = selectedJets.at(0)->p4();
  if (selectedJets.size() >= 2) (*m_etree)["j2Vec"] = selectedJets.at(1)->p4();
  if (selectedJets.size() >= 3) (*m_etree)["j3Vec"] = selectedJets.at(2)->p4();

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

  m_etree->SetBranchAndValue<float>("pTBB", HVec.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("pTBBJ", bbj.Pt() / 1e3, -99);

  if (m_isMC) {  // those values get overwritten if event ends up in merged
                 // regime...
    m_etree->SetBranchAndValue<int>("flavB1", m_physicsMeta.b1Flav, -99);
    m_etree->SetBranchAndValue<int>("flavB2", m_physicsMeta.b2Flav, -99);
    m_etree->SetBranchAndValue<int>("flavJ3", m_physicsMeta.j3Flav, -99);
  }
  float mv2c10_b1 = -99;
  float mv2c10_b2 = -99;
  if (selectedJets.size() >= 1)
    mv2c10_b1 = Props::MV2c10.get(selectedJets.at(0));
  if (selectedJets.size() >= 2)
    mv2c10_b2 = Props::MV2c10.get(selectedJets.at(1));
  m_etree->SetBranchAndValue<float>("MV2c10B1", mv2c10_b1, -99);
  m_etree->SetBranchAndValue<float>("MV2c10B2", mv2c10_b2, -99);
  return EL::StatusCode::SUCCESS;
}  // fillETreeSelectedJets

EL::StatusCode AnalysisReader_VHQQ2Lep::fillETreeLepton(Lepton l1, Lepton l2) {
  m_etree->SetBranchAndValue<float>("ptL1", m_tree->pTL1, -99);
  m_etree->SetBranchAndValue<float>("ptL2", m_tree->pTL2, -99);
  m_etree->SetBranchAndValue<float>("etaL1", m_tree->etaL1, -99);
  m_etree->SetBranchAndValue<float>("etaL2", m_tree->etaL2, -99);
  m_etree->SetBranchAndValue<float>("phiL1", m_tree->phiL1, -99);
  m_etree->SetBranchAndValue<float>("phiL2", m_tree->phiL2, -99);
  m_etree->SetBranchAndValue<float>("mLL", m_tree->mLL, -99);
  m_etree->SetBranchAndValue<float>("cosThetaLep", m_tree->cosThetaLep, -99);
  m_etree->SetBranchAndValue<float>("flavL1", l1.flav, -99);
  m_etree->SetBranchAndValue<float>("flavL2", l2.flav, -99);
  m_etree->SetBranchAndValue<float>("chargeL1", l1.charge, -99);
  m_etree->SetBranchAndValue<float>("chargeL2", l2.charge, -99);
  m_etree->SetBranchAndValue<float>("mL1", l1.vec.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mL2", l2.vec.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("dEtaLL", fabs(l1.vec.Eta() - l2.vec.Eta()),
                                    -99);
  m_etree->SetBranchAndValue<float>("dPhiLL", fabs(l1.vec.DeltaPhi(l2.vec)),
                                    -99);

  return EL::StatusCode::SUCCESS;
}  // fillEtreeLepton

EL::StatusCode AnalysisReader_VHQQ2Lep::fillETreeResolved(Jet j1, Jet j2,
                                                          double HTsmallR,
                                                          double METHTsmallR) {
  TLorentzVector ZVec = m_etree->exists<TLorentzVector>("ZVec")
                            ? (*m_etree)["ZVec"]
                            : TLorentzVector();
  TLorentzVector HVec = m_etree->exists<TLorentzVector>("HVec")
                            ? (*m_etree)["HVec"]
                            : TLorentzVector();
  TLorentzVector lbVec = m_etree->exists<TLorentzVector>("lbVec")
                             ? (*m_etree)["lbVec"]
                             : TLorentzVector();
  TLorentzVector VHVec = m_etree->exists<TLorentzVector>("VHVec")
                             ? (*m_etree)["VHVec"]
                             : TLorentzVector();
  TLorentzVector resolvedH = m_etree->exists<TLorentzVector>("resolvedH")
                                 ? (*m_etree)["resolvedH"]
                                 : TLorentzVector();
  TLorentzVector resolvedVH = m_etree->exists<TLorentzVector>("resolvedVH")
                                  ? (*m_etree)["resolvedVH"]
                                  : TLorentzVector();
  TLorentzVector HVecPtReco = m_etree->exists<TLorentzVector>("HVecPtReco")
                                  ? (*m_etree)["HVecPtReco"]
                                  : TLorentzVector();

  m_etree->SetBranchAndValue<float>("HT", HTsmallR / sqrt(1e3), -99);
  m_etree->SetBranchAndValue<float>("METHT", METHTsmallR / sqrt(1e3), -99);
  m_etree->SetBranchAndValue<float>(
      "dEtaBBcor", fabs(j1.vec_corr.Eta() - j2.vec_corr.Eta()), -99);
  m_etree->SetBranchAndValue<float>(
      "dPhiBBcor", fabs(j1.vec_corr.DeltaPhi(j2.vec_corr)), -99);
  m_etree->SetBranchAndValue<float>("dRBBcor",
                                    fabs(j1.vec_corr.DeltaR(j2.vec_corr)), -99);
  m_etree->SetBranchAndValue<float>("dPhiVBBPtReco",
                                    fabs(ZVec.DeltaPhi(HVecPtReco)), -99);
  m_etree->SetBranchAndValue<float>("dEtaVBBPtReco",
                                    fabs(ZVec.Eta() - HVecPtReco.Eta()), -99);
  m_etree->SetBranchAndValue<float>("dRVBB", fabs(ZVec.DeltaR(HVec)), -99);
  m_etree->SetBranchAndValue<float>("pTVH", VHVec.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mVH", VHVec.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mVHres", resolvedVH.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mBBres", resolvedH.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mLB", lbVec.M() / 1e3, -99);

  return EL::StatusCode::SUCCESS;
}  // fillEtreeResolved

EL::StatusCode AnalysisReader_VHQQ2Lep::fillETreeMerged(
    VHCand &mergedVH, Higgs &mergedH, int nTrkJets, int nTaggedTrkJets,
    int nTrkjetsInFJ, double HTlargeR, double METHTlargeR) {
  TLorentzVector Fjet1 = m_etree->exists<TLorentzVector>("fatj1Vec")
                             ? (*m_etree)["fatj1Vec"]
                             : TLorentzVector();
  TLorentzVector Fjet2 = m_etree->exists<TLorentzVector>("fatj2Vec")
                             ? (*m_etree)["fatj2Vec"]
                             : TLorentzVector();
  TLorentzVector HVec = m_etree->exists<TLorentzVector>("HVec")
                            ? (*m_etree)["HVec"]
                            : TLorentzVector();

  m_etree->SetBranchAndValue<float>("etaFjet1", Fjet1.Eta(), -99);
  m_etree->SetBranchAndValue<float>("phiFjet1", Fjet1.Phi(), -99);
  m_etree->SetBranchAndValue<float>("pTFjet1", Fjet1.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mFjet1", Fjet1.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("etaFjet2", Fjet2.Eta(), -99);
  m_etree->SetBranchAndValue<float>("phiFjet2", Fjet2.Phi(), -99);
  m_etree->SetBranchAndValue<float>("pTFjet2", Fjet2.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mFjet2", Fjet2.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("etaH", HVec.Eta(), -99);
  m_etree->SetBranchAndValue<float>("phiH", HVec.Phi(), -99);
  m_etree->SetBranchAndValue<float>("pTH", HVec.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("mH", HVec.M() / 1e3, -99);

  m_etree->SetBranchAndValue<float>("mVHmerg", mergedVH.vec_corr.M() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("mBBmerg", mergedH.vec_corr.M() / 1e3, -99);
  m_etree->SetBranchAndValue<int>("nFatJets", m_physicsMeta.nFatJet, -99);
  m_etree->SetBranchAndValue<int>("nbTagsInFJ", m_physicsMeta.nTagsInFJ, -99);
  m_etree->SetBranchAndValue<int>("nTrkjetsInFJ", nTrkjetsInFJ, -99);
  m_etree->SetBranchAndValue<int>("nbTagsOutsideFJ", m_physicsMeta.nAddBTrkJets,
                                  -99);
  m_etree->SetBranchAndValue<int>("nTrkJets", nTrkJets, -99);
  m_etree->SetBranchAndValue<int>("nBTrkJets", nTaggedTrkJets, -99);
  m_etree->SetBranchAndValue<float>("HTfat", HTlargeR / sqrt(1e3), -99);
  m_etree->SetBranchAndValue<float>("METHTfat", METHTlargeR / sqrt(1e3), -99);

  return EL::StatusCode::SUCCESS;
}  // fillETreeMerged

EL::StatusCode AnalysisReader_VHQQ2Lep::fillETreeMbbMonitor(
    const Higgs &h1, const Lepton &l1, const Lepton &l2, const Jet &j1,
    const Jet &j2, const Jet &j3) {
  m_etree->SetBranchAndValue<float>("GSCMbb", h1.vec_gsc.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("OneMuMbb", h1.vec_onemu.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("PtRecoMbb", h1.vec_ptreco.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("FSRMbb", h1.vec_fsr.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("KFMbb", h1.vec_kf.M() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("TruthWZMbb", h1.vec_truthwz.M() / 1e3,
                                    -99);

  m_etree->SetBranchAndValue<float>("GSCPtH", h1.vec_gsc.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("OneMuPtH", h1.vec_onemu.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("PtRecoPtH", h1.vec_ptreco.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("FSRPtH", h1.vec_fsr.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("KFPtH", h1.vec_kf.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("TruthWZPtH", h1.vec_truthwz.Pt() / 1e3,
                                    -99);

  // response
  m_etree->SetBranchAndValue<float>("GSCMbbResp",
                                    h1.vec_gsc.M() / h1.vec_truthwz.M(), -99);
  m_etree->SetBranchAndValue<float>("OneMuMbbResp",
                                    h1.vec_onemu.M() / h1.vec_truthwz.M(), -99);
  m_etree->SetBranchAndValue<float>(
      "PtRecoMbbResp", h1.vec_ptreco.M() / h1.vec_truthwz.M(), -99);
  m_etree->SetBranchAndValue<float>("FSRMbbResp",
                                    h1.vec_fsr.M() / h1.vec_truthwz.M(), -99);
  m_etree->SetBranchAndValue<float>("KFMbbResp",
                                    h1.vec_kf.M() / h1.vec_truthwz.M(), -99);

  m_etree->SetBranchAndValue<float>("GSCPtHResp",
                                    h1.vec_gsc.Pt() / h1.vec_truthwz.Pt(), -99);
  m_etree->SetBranchAndValue<float>(
      "OneMuPtHResp", h1.vec_onemu.Pt() / h1.vec_truthwz.Pt(), -99);
  m_etree->SetBranchAndValue<float>(
      "PtRecoPtHResp", h1.vec_ptreco.Pt() / h1.vec_truthwz.Pt(), -99);
  m_etree->SetBranchAndValue<float>("FSRPtHResp",
                                    h1.vec_fsr.Pt() / h1.vec_truthwz.Pt(), -99);
  m_etree->SetBranchAndValue<float>("KFPtHResp",
                                    h1.vec_kf.Pt() / h1.vec_truthwz.Pt(), -99);

  // balance
  TLorentzVector z_vec = l1.vec + l2.vec;

  m_etree->SetBranchAndValue<float>("GSCPxVH", (h1.vec_gsc + z_vec).Px() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("OneMuPxVH",
                                    (h1.vec_onemu + z_vec).Px() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("PtRecoPxVH",
                                    (h1.vec_ptreco + z_vec).Px() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("FSRPxVH", (h1.vec_fsr + z_vec).Px() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("KFPxVH", (h1.vec_kf + z_vec).Px() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("TruthWZPxVH",
                                    (h1.vec_truthwz + z_vec).Px() / 1e3, -99);

  m_etree->SetBranchAndValue<float>("GSCPyVH", (h1.vec_gsc + z_vec).Py() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("OneMuPyVH",
                                    (h1.vec_onemu + z_vec).Py() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("PtRecoPyVH",
                                    (h1.vec_ptreco + z_vec).Py() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("FSRPyVH", (h1.vec_fsr + z_vec).Py() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("KFPyVH", (h1.vec_kf + z_vec).Py() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("TruthWZPyVH",
                                    (h1.vec_truthwz + z_vec).Py() / 1e3, -99);

  // object
  m_etree->SetBranchAndValue<float>("KFptL1", l1.vec_kf.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("KFptL2", l2.vec_kf.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("KFmLL", (l1.vec_kf + l2.vec_kf).M() / 1e3,
                                    -99);
  m_etree->SetBranchAndValue<float>("KFptJ1", j1.vec_kf.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("KFptJ2", j2.vec_kf.Pt() / 1e3, -99);
  m_etree->SetBranchAndValue<float>("KFptJ3", j3.vec_kf.Pt() / 1e3, -99);

  m_etree->SetBranchAndValue<float>("SoftMetPx",
                                    Props::soft_mpx.get(m_met) / 1e3, -99);
  m_etree->SetBranchAndValue<float>("SoftMetPy",
                                    Props::soft_mpy.get(m_met) / 1e3, -99);

  m_etree->SetBranchAndValue<float>("ResL1", l1.resolution, -99);
  m_etree->SetBranchAndValue<float>("ResL2", l2.resolution, -99);
  m_etree->SetBranchAndValue<float>("ResJ1", j1.resolution, -99);
  m_etree->SetBranchAndValue<float>("ResJ2", j2.resolution, -99);
  m_etree->SetBranchAndValue<float>("ResJ3", j3.resolution, -99);

  // others
  m_etree->SetBranchAndValue<float>("KFPrefitLHD", m_KFPrefitLHD, -99);
  m_etree->SetBranchAndValue<float>("KFPostfitLHD", m_KFPostfitLHD, -99);
  m_KFPrefitLHD = 0;
  m_KFPostfitLHD = 0;

  return EL::StatusCode::SUCCESS;
}  // fillETreeMbbMonitor

EL::StatusCode AnalysisReader_VHQQ2Lep::fillMVATreeVHbbResolved2Lep(
    TLorentzVector &b1, TLorentzVector &b2, TLorentzVector &j3,
    const std::vector<const xAOD::Jet *> &selectedJets,
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &HVec,
    TLorentzVector &ZVec, TLorentzVector &metVec, const xAOD::MissingET *&met,
    const Lepton &lep1, const Lepton &lep2, int nTaus, float bTagWeight) {
  // m_tree->SetVariation(m_currentVar);

  // DO NOT ENTER NEW VARIABLES HERE IF NOT ABSOLUTELY NECESSARY FOR MVA
  // please use the EasyTree functionality for studies

  EL_CHECK("fillMVATreeVHbbResolved",
           fillMVATreeVHbbResolved(b1, b2, j3, selectedJets, signalJets,
                                   forwardJets, HVec, ZVec, metVec, nTaus));

  m_tree->MEff = (b1.Pt() + b2.Pt() + j3.Pt() + lep1.vec.Pt() + lep2.vec.Pt() +
                  metVec.Pt()) /
                 1e3;

  m_tree->bTagWeight = bTagWeight;
  m_tree->etaV = ZVec.Eta();
  m_tree->mLL = ZVec.M() / 1e3;
  m_tree->pTL1 = lep1.vec.Pt() / 1e3;
  m_tree->pTL2 = lep2.vec.Pt() / 1e3;
  m_tree->etaL1 = lep1.vec.Eta();
  m_tree->etaL2 = lep2.vec.Eta();
  m_tree->phiL1 = lep1.vec.Phi();
  m_tree->phiL2 = lep2.vec.Phi();

  // ATTENTION: In 2 lepton we currently don't use object based METSig
  // we calculate quasi-METSig by hand
  // m_tree->METSig = Props::metSig.get(met);
  bool useObjMETSig = false;
  m_tree->METSig =
      (useObjMETSig)
          ? Props::metSig.get(met)
          : m_tree->MET /
                (sqrt(m_tree->sumPtJets + m_tree->pTL1 + m_tree->pTL2));

  // Z-polarizartion
  float cosThetaLep = -99;
  computePolarizationVariable(lep1, lep2, &cosThetaLep);
  m_tree->cosThetaLep = cosThetaLep;

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHQQ2Lep::printMVA() {
  if (m_etree->get<int>("nbJets") != 2) return;
  if (m_etree->get<std::string>("Description") != "SR") return;
  if (m_etree->get<float>("pTV") < 75) return;

  printf(
      "XXX EventNumber %d EventWeight %6f NJet %d NsigJet %d NfwdJet %d NTag "
      "%d BDT %6.3f\n",
      m_etree->get<int>("EventNumber"), m_etree->get<float>("EventWeight"),
      m_etree->get<int>("nJets"), m_etree->get<int>("nSigJets"),
      m_etree->get<int>("nFwdJets"), m_etree->get<int>("nbJets"), m_tree->BDT);
  printf(
      "XXX EventNumber %llu Input mBB %6.3f dRBB %6.3f dEtaVBB %6.3f dPhiVBB "
      "%6.3f pTV %6.3f pTB1 %6.3f pTB2 %6.3f METSig %6.3f mLL %6.3f mBBJ "
      "%6.3f "
      "pTJ3 %6.3f cosThetaLep %6.3f",
      m_tree->EventNumber, m_tree->mBB, m_tree->dRBB, m_tree->dEtaVBB,
      m_tree->dPhiVBB, m_tree->pTV, m_tree->pTB1, m_tree->pTB2, m_tree->METSig,
      m_tree->mLL, m_tree->mBBJ, m_tree->pTJ3, m_tree->cosThetaLep);
  if (m_doMbbMonitor) {
    printf(" mBB(KF) %6.3f mBB(PtReco) %6.3f mBB(Uncorr) %6.3f",
           m_etree->get<float>("KFMbb"), m_etree->get<float>("PtRecoMbb"),
           m_etree->get<float>("GSCMbb"));
  }
  printf("\n");

  return;
}

EL::StatusCode AnalysisReader_VHQQ2Lep::finalizeTools() {
  EL_CHECK("AnalysisReader_VHQQ2lep::finalizeTools()",
           AnalysisReader_VHQQ::finalizeTools());

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHQQ2Lep ::computePolarizationVariable(const Lepton &lep1,
                                                           const Lepton &lep2,
                                                           float *cosThetaLep) {
  TLorentzVector neglep, poslep;
  if (lep1.charge < 0) {
    neglep = lep1.vec;
    poslep = lep2.vec;
  } else {
    neglep = lep2.vec;
    poslep = lep1.vec;
  }
  TLorentzRotation lr = getTrafoToHelicityFrame(neglep, poslep);
  neglep.Transform(lr);
  poslep.Transform(lr);
  // std::cout << "(*)L(-) vector: " << components(neglep) << std::endl;
  // std::cout << "(*)L(+) vector: " << components(poslep) << std::endl;

  *cosThetaLep = TMath::Cos(neglep.Theta());
  return;
}

TLorentzRotation AnalysisReader_VHQQ2Lep ::getTrafoToHelicityFrame(
    const TLorentzVector &neglep, const TLorentzVector &poslep) {
  bool debug = false;
  // spacial rotation
  TLorentzVector l1 = neglep, l2 = poslep;
  TLorentzVector z = l1 + l2;
  TLorentzVector beamaxis(0.0, 0.0, 1.0, 1.0);

  if (debug) std::cout << "----" << std::endl;
  if (debug)
    std::cout << "(0)Z vector: " << components(z) << " mass:" << z.M()
              << std::endl;
  if (debug) std::cout << "(0)L(-) vector: " << components(l1) << std::endl;
  if (debug) std::cout << "(0)L(+) vector: " << components(l2) << std::endl;
  if (debug) std::cout << "(0)Beam axis: " << components(beamaxis) << std::endl;

  //(1) Rotaion so that Z bosons are in the x-z plane.
  TRotation r1;
  r1.RotateZ(-z.Phi());
  TLorentzRotation lr1 = TLorentzRotation(r1);
  z = z.Transform(lr1);
  l1 = l1.Transform(lr1);
  l2 = l2.Transform(lr1);
  beamaxis = beamaxis.Transform(lr1);

  if (debug)
    std::cout << "(1)Z vector: " << components(z) << " mass:" << z.M()
              << std::endl;
  if (debug) std::cout << "(1)L(-) vector: " << components(l1) << std::endl;
  if (debug) std::cout << "(1)L(+) vector: " << components(l2) << std::endl;
  if (debug) std::cout << "(1)Beam axis: " << components(beamaxis) << std::endl;

  //(2) Boost along the z axis so that Z bosons move in the x-axis direction
  TLorentzRotation lr2;
  lr2.Boost(0.0, 0.0, -z.Pz() / z.E());
  z = z.Transform(lr2);
  l1 = l1.Transform(lr2);
  l2 = l2.Transform(lr2);
  beamaxis = beamaxis.Transform(lr2);

  if (debug)
    std::cout << "(2)Z vector: " << components(z) << " mass:" << z.M()
              << std::endl;
  if (debug) std::cout << "(2)L(-) vector: " << components(l1) << std::endl;
  if (debug) std::cout << "(2)L(+) vector: " << components(l2) << std::endl;
  if (debug) std::cout << "(2)Beam axis: " << components(beamaxis) << std::endl;

  //(3) Rotation so that Z bosons move in the z direction. The collision axes
  // are in the x-z plane there.
  TRotation r3;
  r3.RotateY(-z.Theta());
  TLorentzRotation lr3 = TLorentzRotation(r3);
  z = z.Transform(lr3);
  l1 = l1.Transform(lr3);
  l2 = l2.Transform(lr3);
  beamaxis = beamaxis.Transform(lr3);

  if (debug)
    std::cout << "(3)Z vector: " << components(z) << " " << z.M() << std::endl;
  if (debug) std::cout << "(3)L(-) vector: " << components(l1) << std::endl;
  if (debug) std::cout << "(3)L(+) vector: " << components(l2) << std::endl;
  if (debug) std::cout << "(3)Beam axis: " << components(beamaxis) << std::endl;

  //(4) Boost along the z axis to rest frames of Z bosons
  TLorentzRotation lr4;
  lr4.Boost(0.0, 0.0, -z.Pz() / z.E());
  z = z.Transform(lr4);
  l1 = l1.Transform(lr4);
  l2 = l2.Transform(lr4);
  beamaxis = beamaxis.Transform(lr4);

  if (debug)
    std::cout << "(4)Z vector: " << components(z) << " mass:" << z.M()
              << std::endl;
  if (debug) std::cout << "(4)L(-) vector: " << components(l1) << std::endl;
  if (debug) std::cout << "(4)L(+) vector: " << components(l2) << std::endl;
  if (debug) std::cout << "(4)Beam axis: " << components(beamaxis) << std::endl;
  if (debug) std::cout << "(4)ThetaLep: " << l1.Theta() << std::endl;

  TLorentzRotation lr = lr4 * lr3 * lr2 * lr1;

  return lr;
}

std::string AnalysisReader_VHQQ2Lep ::components(TLorentzVector v) {
  std::ostringstream oss;
  oss << "(px,py,pz,p0) = (" << v.Px() << ", " << v.Py() << ", " << v.Pz()
      << ", " << v.E() << ")";
  return oss.str();
}
