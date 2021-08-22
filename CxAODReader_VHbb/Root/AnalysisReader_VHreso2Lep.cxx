#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ2Lep.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb2lep.h"
#include "CxAODTools_VHbb/VHbb2lepEvtSelection.h"
#include "KinematicFit/KinematicFit.h"

#include <CxAODReader_VHbb/AnalysisReader_VHreso2Lep.h>
#include <TMVA/Reader.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
#include "TXMLEngine.h"

#include <iomanip>

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHreso2Lep::AnalysisReader_VHreso2Lep()
    : AnalysisReader_VHQQ2Lep() {
  m_doMergeModel = false;
}

AnalysisReader_VHreso2Lep::~AnalysisReader_VHreso2Lep() {}

EL::StatusCode AnalysisReader_VHreso2Lep::run_2Lep_analysis() {
  // remove overlap between dilep and inclusive / non-all had ttbar and single
  // top Wt samples
  //----------------------------------------------------------------------------
  // requires to include both in the sample list!
  // apply to which samples? just to the nominal PP8 ones?
  // todo: move vetoDilepTtbarEvents implementation to VHQQ so that it can also
  // be used by VHcc? - wait until harmonised treatment available for ttbar and
  // single top?
  if (m_doRemoveDilepOverlap) {
    // ttbar
    if (m_mcChannel == 410470) {  // 410470: PP8 non-all-had ttbar
      int veto = vetoDilepTtbarEvents();
      if (veto == -1) {
        Error("run_2Lep_analysis()", "Props::codeTTBarDecay doesn't exist!");
        return EL::StatusCode::FAILURE;
      } else if (veto == 1) {
        // Event has to be vetoed
        return EL::StatusCode::SUCCESS;
      }
    }

    // single top Wt
    if (m_mcChannel == 410646 ||
        m_mcChannel ==
            410647) {  // 410646(7): PP8 inclusive single (anti-)top Wt
      int veto = vetoDilepWtEvents();
      // Event has to be vetoed
      if (veto == 1) return EL::StatusCode::SUCCESS;
    }
  }
  //----------------------------

  // reset physics metadata
  m_physicsMeta = PhysicsMetadata();

  // turn truth tagging off for merged analysis for the moment
  if (m_analysisStrategy != "Resolved") m_doTruthTagging = false;

  // Initialize eventFlag - all cuts are initialized to false by default
  unsigned long int eventFlag = 0;

  // EasyTree reset
  m_etree->Reset();
  m_etree->SetVariation(m_currentVar);

  // reset MVA tree variables
  m_tree->Reset();
  m_tree->SetVariation(m_currentVar);
  ResultVHbb2lep selectionResult =
      ((VHbb2lepEvtSelection *)m_eventSelection)->result();

  m_physicsMeta.channel = PhysicsMetadata::Channel::TwoLep;

  const xAOD::Electron *el1 = selectionResult.el1;
  const xAOD::Electron *el2 = selectionResult.el2;
  const xAOD::Muon *mu1 = selectionResult.mu1;
  const xAOD::Muon *mu2 = selectionResult.mu2;
  const xAOD::MissingET *met = selectionResult.met;
  std::vector<const xAOD::Jet *> signalJets = selectionResult.signalJets;
  std::vector<const xAOD::Jet *> forwardJets = selectionResult.forwardJets;
  std::vector<const xAOD::Jet *> fatJets = selectionResult.fatJets;
  std::vector<const xAOD::Jet *> trackJets = selectionResult.trackJets;
  std::vector<const xAOD::TauJet *> taus = selectionResult.taus;
  setevent_nJets(signalJets, forwardJets);

  ////////////////////////////////////////////////////////////
  //         OBJECT DEFINITIONS                             //
  ////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////
  // **Leptons Definition** :
  Lepton l1;
  Lepton l2;
  setLeptonVariables(l1, l2, el1, el2, mu1, mu2);
  setTwoLeptonFlavour(l1, l2);

  bool isMu = (m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu);
  bool isE = (m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl);

  /////////////////////////////////////////////////////////////
  // **Definition** : Z
  TLorentzVector ZVec = l1.vec + l2.vec;

  /////////////////////////////////////////////////////////////
  // **Definition** : b-tagging

  // for tagging in merged regime: vectors to hold the trackjets in the leading
  // fat-jet and not associated to the leading fat jet
  std::vector<const xAOD::Jet *> trackJetsInLeadFJ, trackJetsNotInLeadFJ;

  // TODO: Harmonize large-R jet / track-jet association with resonant analysis
  if (m_doTruthTagging) {
    compute_TRF_tagging(signalJets);
  } else {
    compute_btagging();
  }
  compute_fatjetTags(trackJets, fatJets, &trackJetsInLeadFJ,
                     &trackJetsNotInLeadFJ);

  /////////////////////////////////////////////////////////////
  // **Definition** : selected jets
  int tagcatExcl = -1;
  std::vector<const xAOD::Jet *> selectedJets;
  selectedJets.clear();

  tagjet_selection(signalJets, forwardJets, selectedJets, tagcatExcl);
  if (m_model == Model::CUT || m_model == Model::MVA) {
    // if Forward jet has higher pT assign it as a third jet.
    if (selectedJets.size() == 3) {
      for (unsigned int ifjet = 0; ifjet < forwardJets.size(); ++ifjet) {
        if (selectedJets.at(2)->pt() < forwardJets.at(ifjet)->pt())
          selectedJets.at(2) = forwardJets.at(ifjet);
      }
    }
  }
  m_physicsMeta.nTags = tagcatExcl;
  // btags inside/outside fat jet
  if (fatJets.size() > 0) {
    // number of lead / sublead b-tagged track jets associated to leading fat
    // jet
    m_physicsMeta.nTagsInFJ = Props::nBTags.get(fatJets.at(0));
    ;
    // number of b-tagged track jets not associated to leading fat jet
    m_physicsMeta.nAddBTrkJets = Props::nAddBTags.get(fatJets.at(0));
  }
  // number of b-tagged track jets
  int nTaggedTrkJets = 0;
  for (auto jet : trackJets) {
    if (BTagProps::isTagged.get(jet)) nTaggedTrkJets++;
  }

  // define jets for b-tagging weight computation and event labelling
  //------------------------------------------------------------------
  std::vector<const xAOD::Jet *> trackJetsForBTagging,
      bTaggedTrackJetsNotInLeadFJ, trackjetsForLabelling;
  // consider two leading track jets matched to the lead. fat jet
  // and all track jets not matched to fat jet in b-tagging event weight
  // computation
  trackJetsForBTagging = trackJetsInLeadFJ;
  trackJetsForBTagging.insert(trackJetsForBTagging.end(),
                              trackJetsNotInLeadFJ.begin(),
                              trackJetsNotInLeadFJ.end());
  // find track jets used for flavour labelling the event: one (two) leading
  // ones inside the fat jet, leading (b-tagged) one outside of the fat jet
  // first: find b-tagged unmatched track jets
  for (auto jet : trackJetsNotInLeadFJ) {
    if (BTagProps::isTagged.get(jet))
      bTaggedTrackJetsNotInLeadFJ.push_back(jet);
  }
  // second: check consistency with what was derived before
  if ((unsigned int)m_physicsMeta.nAddBTrkJets !=
      bTaggedTrackJetsNotInLeadFJ.size()) {
    Error("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
          "m_physicsMeta.nAddBTrkJets != bTaggedTrackJetsNotInLeadFJ.size()");
    return EL::StatusCode::FAILURE;
  }
  // third: select jets for event flavour labelling
  trackjetsForLabelling = trackJetsInLeadFJ;
  if (m_physicsMeta.nAddBTrkJets)
    trackjetsForLabelling.insert(trackjetsForLabelling.end(),
                                 bTaggedTrackJetsNotInLeadFJ.begin(),
                                 bTaggedTrackJetsNotInLeadFJ.end());
  else
    trackjetsForLabelling.insert(trackjetsForLabelling.end(),
                                 trackJetsNotInLeadFJ.begin(),
                                 trackJetsNotInLeadFJ.end());

  // define jets to be used for mBB + possible additional jet
  //---------------------------------------------------------
  Jet j1, j2, j3;     // small-R jets
  Jet fj1, fj2, fj3;  // fat jets
  EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis()",
           setJetVariables(j1, j2, j3, selectedJets));
  EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis()",
           setFatJetVariables(fj1, fj2, fj3, fatJets));

  /////////////////////////////////////////////////////////////
  // **Definition**: bl system
  TLorentzVector lbVec = defineMinDRLepBSyst(l1, l2, j1, j2);

  /////////////////////////////////////////////////////////////
  // **Definition**: MET

  // Elisabeth: not used. comment out to remove compilation warnings
  // if still want to fill EasyTree/histograms with this info
  // need to remove these comments

  // double metSig = -1.;
  // double metSig_PU = -1.;
  // double metSig_soft = -1.;
  // double metSig_hard = -1.;

  // metSig = Props::metSig.get(met);
  // if (Props::metSig_PU.exists(met)) {
  //   metSig_PU = Props::metSig_PU.get(met);
  // } else {
  //   metSig_hard = Props::metSig_hard.get(met);
  //   metSig_soft = Props::metSig_soft.get(met);
  // }
  /////////////////////////////////////////////////////////////
  // **Definition** : HT and METHT
  double HTsmallR(0.), HTlargeR(0.);
  computeHT(HTsmallR, HTlargeR, l1, l2, signalJets, forwardJets, fatJets);

  double METHTsmallR = met->met() / sqrt(HTsmallR);
  double METHTlargeR = met->met() / sqrt(HTlargeR);

  /////////////////////////////////////////////////////////////
  // **Definition** : HVec
  Higgs resolvedH, mergedH;
  setHiggsCandidate(resolvedH, mergedH, j1, j2, fj1);
  if ((m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl ||
       m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu ||
       m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu ||
       m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) &&
      ((m_physicsMeta.nJets == 2) || (m_physicsMeta.nJets == 3)) &&
      m_physicsMeta.nTags == 2) {
    EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis()",
             getKFResolved(selectedJets.size(), j1, j2, j3, l1, l2));
    if (m_model == Model::CUT || m_model == Model::MVA) {
      j1.vec_corr = j1.vec_kf;
      j2.vec_corr = j2.vec_kf;
    }
  }
  resolvedH.vec_kf = j1.vec_kf + j2.vec_kf;
  if ((m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl ||
       m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu ||
       m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu ||
       m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) &&
      (fatJets.size() == 1 &&
       ((m_physicsMeta.nJets == 0 && selectedJets.size() == 0) ||
        (m_physicsMeta.nJets == 1 && selectedJets.size() == 1) ||
        (m_physicsMeta.nJets == 2 && selectedJets.size() == 2) ||
        (m_physicsMeta.nJets == 3 && selectedJets.size() == 3)))) {
    EL_CHECK(
        "AnalysisReader_VHreso2Lep::run_2Lep_analysis()",
        getKFMerged(m_physicsMeta.nJets, fj1, l1, l2, signalJets, forwardJets));
  }
  mergedH.vec_kf = fj1.vec_kf;

  if (m_model == Model::CUT || m_model == Model::MVA) {
    setHiggsCandidate(resolvedH, mergedH, j1, j2, fj1);
  }

  /////////////////////////////////////////////////////////////
  //  **Definition** : ZH system
  VHCand resolvedVH, mergedVH;
  setVHCandidate(resolvedVH, mergedVH, l1, l2, j1, j2, fj1);

  ////////////////////////////////////////////////////////////
  //         CHECK SELECTION - STORE INFORMATION            //
  ////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  // C0: All events (CxAOD)
  //------------------------
  updateFlag(eventFlag, DiLeptonCuts::AllCxAOD);

  // **Selection & SF**: trigger
  //----------------------------
  double triggerSF = 1.;
  // C1: trigger
  if (pass2LepTrigger(triggerSF, selectionResult))
    updateFlag(eventFlag, DiLeptonCuts::Trigger);
  if (m_isMC) m_weight *= triggerSF;

  // **Selection** : C2: same flavor
  //----------------------------
  if (isMu || isE)
    updateFlag(eventFlag, DiLeptonCuts::SFLeptons);
  else
    updateFlag(eventFlag, DiLeptonCuts::DFLeptons);

  if (m_isMC) m_weight *= Props::leptonSF.get(m_eventInfo);

  // **CR Selection** : cut on lepton pT
  //------------------------------------
  if (l1.vec.Pt() / 1000. >= 27.) {
    updateFlag(eventFlag, DiLeptonCuts::LeadLepPt27);
    if (l2.vec.Pt() / 1000. >= 25.)
      updateFlag(eventFlag, DiLeptonCuts::pTBothLeptons25);
  }

  // **CR Selection** : cut on lepton charge and muon eta (for merged analysis)
  //-----------------------------------------------------
  if (isE) {
    updateFlag(eventFlag, DiLeptonCuts::OSLeptons);
    updateFlag(eventFlag, DiLeptonCuts::MuonEtaLT2p5);
  } else if (isMu) {
    if (l1.charge != l2.charge) updateFlag(eventFlag, DiLeptonCuts::OSLeptons);
    if (fabs(l1.vec.Eta()) < 2.5 && fabs(l2.vec.Eta()) < 2.5)
      updateFlag(eventFlag, DiLeptonCuts::MuonEtaLT2p5);
  } else {  // emu channel
    if (l1.charge != l2.charge) updateFlag(eventFlag, DiLeptonCuts::OSLeptons);
    if (((l1.flav == lepFlav::mu) && (fabs(l1.vec.Eta()) < 2.5)) ||
        ((l2.flav == lepFlav::mu) && (fabs(l2.vec.Eta()) < 2.5)))
      updateFlag(eventFlag, DiLeptonCuts::MuonEtaLT2p5);
  }

  // **Selection** : tau veto
  //----------------------------------------
  if (!taus.size()) updateFlag(eventFlag, DiLeptonCuts::tauVeto);

  // **Selection** : C3: Mll cut
  //-----------------------------
  bool passMllResolved(false), passMllMerged(false);
  if (m_model == Model::AZh || m_model == Model::HVT)
    checkMllCut(ZVec, resolvedVH.vec_corr, mergedVH.vec_corr, passMllResolved,
                passMllMerged);
  else if (m_model == Model::CUT || m_model == Model::MVA) {
    if ((ZVec.M() / 1000. >= 81.) && (ZVec.M() / 1000. < 101.)) {
      passMllResolved = true;
      passMllMerged = true;
    }
  }

  if (passMllResolved) updateFlag(eventFlag, DiLeptonCuts::MllZwindowResolved);
  if (passMllMerged) updateFlag(eventFlag, DiLeptonCuts::MllZwindowMerged);
  if (ZVec.M() / 1.e3 > 40.) updateFlag(eventFlag, DiLeptonCuts::MllAbove40);
  // if ((ZVec.M() / 1000. >= 71.) && (ZVec.M() / 1000. < 121.))
  // updateFlag(eventFlag, DiLeptonCuts::LooseMll);

  //   **Selection** : C4: METHT cut
  //----------------------------------
  if (m_model == Model::AZh || m_model == Model::HVT) {
    if (METHTsmallR / sqrt(1000.) < METHTcut(resolvedVH.vec_corr.M()))
      updateFlag(eventFlag, DiLeptonCuts::METHTResolved);
    if (METHTsmallR / sqrt(1000.) < METHTcut(mergedVH.vec_corr.M()))
      updateFlag(eventFlag, DiLeptonCuts::METHTMerged);
  } else if (m_model == Model::CUT || m_model == Model::MVA) {
    if (METHTsmallR / sqrt(1000.) < 3.5)
      updateFlag(eventFlag, DiLeptonCuts::METHTResolved);
    updateFlag(eventFlag, DiLeptonCuts::METHTMerged);
  }

  // **Selection** : C5: cut on nJet
  //-----------------------------------
  if (m_physicsMeta.nJets >= 2)
    updateFlag(eventFlag, DiLeptonCuts::AtLeast2Jets);

  // **Selection** : C6: cut on nSignalJet
  //-----------------------------------------
  if (m_physicsMeta.nSigJet >= 2)
    updateFlag(eventFlag, DiLeptonCuts::AtLeast2SigJets);
  float JVTweight = 1;
  if (m_isMC) {
    JVTweight = compute_JVTSF(signalJets);
    m_weight *= JVTweight;
  }

  // **Selection** : B-tagging/Jet selection
  //-----------------------------------------
  // C7: at least 1 b
  if (tagcatExcl >= 1) updateFlag(eventFlag, DiLeptonCuts::AtLeast1B);
  // C8: exactly 2 b
  if (tagcatExcl == 2) updateFlag(eventFlag, DiLeptonCuts::Exactly2B);
  // if (tagcatExcl < 3) updateFlag(eventFlag, DiLeptonCuts::Veto3B);

  // **Selection** :  C9: pt(leading jet) >= 45 GeV
  //-----------------------------------------
  if (selectedJets.size() > 0 && j1.vec.Pt() / 1.e3 > 45.)
    updateFlag(eventFlag, DiLeptonCuts::PtB145);

  // **Selection** : dRCut for SM CBA
  //-----------------------------------------
  float dRCut = 3.0;
  if (ZVec.Pt() * 0.001 > 200) {
    dRCut = 1.2;
  } else if (ZVec.Pt() * 0.001 > 150) {
    dRCut = 1.8;
  }
  if (m_model == Model::CUT && selectedJets.size() > 1 &&
      j1.vec_onemu.DeltaR(j2.vec_onemu) < dRCut) {
    updateFlag(eventFlag, DiLeptonCuts::dRCut);
  }

  //  **Selection** : C10': mbb w/o mu-in-jet (+ pT reco) corr
  //------------------------------------------
  bool passMHResolved(false), passMHMerged(false), inMHSideBandResolved(false),
      inMHSideBandMerged(false);
  checkMbbWindow(resolvedH.vec, mergedH.vec, passMHResolved, passMHMerged,
                 inMHSideBandResolved, inMHSideBandMerged);

  if (passMHResolved) updateFlag(eventFlag, DiLeptonCuts::mHNoCorrResolved);
  if (passMHMerged) updateFlag(eventFlag, DiLeptonCuts::mHNoCorrMerged);

  // C10: mbb w/ mu-in-jet (+ pT reco) corr
  //----------------------------------------
  checkMbbWindow(resolvedH.vec_corr, mergedH.vec_corr, passMHResolved,
                 passMHMerged, inMHSideBandResolved, inMHSideBandMerged);

  if (passMHResolved) updateFlag(eventFlag, DiLeptonCuts::mHCorrResolved);
  if (passMHMerged) updateFlag(eventFlag, DiLeptonCuts::mHCorrMerged);
  if (inMHSideBandResolved)
    updateFlag(eventFlag, DiLeptonCuts::mHCorrInsideSideBandResolved);
  if (inMHSideBandMerged)
    updateFlag(eventFlag, DiLeptonCuts::mHCorrInsideSideBandMerged);

  // **Selection** :  PtZ
  //------------------------
  if (ZVec.Pt() / 1.e3 < 500.)
    updateFlag(eventFlag, DiLeptonCuts::PtZLT500);
  else
    updateFlag(eventFlag, DiLeptonCuts::PtZGT500);

  bool passPtvResolved(false), passPtvMerged(false);
  if (m_model == Model::AZh || m_model == Model::HVT) {
    checkPTVCut(ZVec, resolvedVH.vec_corr, mergedVH.vec_corr, passPtvResolved,
                passPtvMerged);
    if (passPtvResolved) updateFlag(eventFlag, DiLeptonCuts::PtZResolved);
    if (passPtvMerged) updateFlag(eventFlag, DiLeptonCuts::PtZMerged);
  }

  //  **Selection** : C11-C14: jet multiplicity cuts
  //--------------------------------------------------
  if (m_physicsMeta.nJets == 2) {
    updateFlag(eventFlag, DiLeptonCuts::Exactly2Jets);
  } else if (m_physicsMeta.nJets == 3) {
    updateFlag(eventFlag, DiLeptonCuts::Exactly3Jets);
  } else if (m_physicsMeta.nJets == 4) {
    updateFlag(eventFlag, DiLeptonCuts::Exactly4Jets);
  } else if (m_physicsMeta.nJets >= 5) {
    updateFlag(eventFlag, DiLeptonCuts::AtLeast5Jets);
  }

  int nFatJetTags = 0;
  // Merged selection
  if (fatJets.size() > 0) {
    nFatJetTags = Props::nBTags.get(fatJets.at(0));
    // at least one track-jet in lead FJ
    updateFlag(eventFlag, DiLeptonCuts::AtLeast1FJ);
    // count number of b-tagged leading track jets
    if (trackJetsInLeadFJ.size() >= 1) {
      updateFlag(eventFlag, DiLeptonCuts::LeadFJ1pTJ);
      if (nFatJetTags == 2) updateFlag(eventFlag, DiLeptonCuts::LeadFJ2b);
    }
    if (trackJetsInLeadFJ.size() > 1) {
      updateFlag(eventFlag, DiLeptonCuts::LeadFJ2TJ);
    }
  }
  /////////////////////////////////////////////////////////////
  //  **Fill cutflow** :
  fill_2lepCutFlow(eventFlag, m_physicsMeta.nJets, tagcatExcl, nFatJetTags,
                   isMu, isE);

  ///////////////////////////////////////////////////
  // HISTO FILLING before categorisations        ///
  ///////////////////////////////////////////////////
  if (!m_doOnlyInputs) {
    EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
             fill_nJetHistos(signalJets, "Sig"));
    if (!m_doReduceFillHistos)
      EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
               fill_nJetHistos(forwardJets, "Fwd"));
    EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
             fill_nJetHistos(fatJets, "Fat"));
    if (!m_doReduceFillHistos)
      EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
               fill_nJetHistos(trackJets, "Trk"));
  }

  ///////////////////////////////////////////////////
  // DEFINITION OF CUTS FOR DIFFERENT REGIONS     ///
  ///////////////////////////////////////////////////

  // --- Cuts for resolved analysis --- //
  std::vector<unsigned long int> cuts_common_resolved = {
      DiLeptonCuts::Trigger,         DiLeptonCuts::LeadLepPt27,
      DiLeptonCuts::OSLeptons,       DiLeptonCuts::AtLeast2Jets,
      DiLeptonCuts::AtLeast2SigJets, DiLeptonCuts::PtB145};
  std::vector<unsigned long int> cuts_mBBcr_resolved_baseline = {
      DiLeptonCuts::SFLeptons, DiLeptonCuts::MllZwindowResolved,
      DiLeptonCuts::METHTResolved, DiLeptonCuts::PtZResolved};
  std::vector<unsigned long int> cuts_restrict_mBBcr_resolved = {
      DiLeptonCuts::mHCorrInsideSideBandResolved};
  std::vector<unsigned long int> cuts_SR_resolved = {
      DiLeptonCuts::mHCorrResolved};
  std::vector<unsigned long int> cuts_top_resolved;
  if (m_model == Model::AZh || m_model == Model::HVT)
    cuts_top_resolved = {DiLeptonCuts::MllZwindowResolved,
                         DiLeptonCuts::mHCorrResolved,
                         DiLeptonCuts::PtZResolved};
  else if (m_model == Model::CUT || m_model == Model::MVA)
    cuts_top_resolved = {DiLeptonCuts::MllAbove40};

  if (m_model == Model::CUT) {
    cuts_common_resolved = {DiLeptonCuts::LeadLepPt27,
                            DiLeptonCuts::Trigger,
                            DiLeptonCuts::OSLeptons,
                            DiLeptonCuts::AtLeast2Jets,
                            DiLeptonCuts::AtLeast2SigJets,
                            DiLeptonCuts::PtB145,
                            DiLeptonCuts::dRCut};
    cuts_mBBcr_resolved_baseline = {DiLeptonCuts::SFLeptons,
                                    DiLeptonCuts::MllZwindowResolved,
                                    DiLeptonCuts::METHTResolved};
    cuts_SR_resolved = {};
    cuts_top_resolved = {DiLeptonCuts::MllZwindowResolved};
  }

  if (m_model == Model::MVA) {
    cuts_common_resolved = {
        DiLeptonCuts::LeadLepPt27,     DiLeptonCuts::Trigger,
        DiLeptonCuts::OSLeptons,       DiLeptonCuts::AtLeast2Jets,
        DiLeptonCuts::AtLeast2SigJets, DiLeptonCuts::PtB145};
    cuts_mBBcr_resolved_baseline = {DiLeptonCuts::SFLeptons,
                                    DiLeptonCuts::MllZwindowResolved};
    cuts_SR_resolved = {};
    cuts_top_resolved = {DiLeptonCuts::MllZwindowResolved};
  }

  // mBBcr = common + mBBcr-baseline + restriction to 50-200 GeV
  std::vector<unsigned long int> cuts_mBBcr_resolved;
  cuts_mBBcr_resolved.insert(cuts_mBBcr_resolved.end(),
                             cuts_common_resolved.begin(),
                             cuts_common_resolved.end());
  cuts_mBBcr_resolved.insert(cuts_mBBcr_resolved.end(),
                             cuts_mBBcr_resolved_baseline.begin(),
                             cuts_mBBcr_resolved_baseline.end());
  cuts_mBBcr_resolved.insert(cuts_mBBcr_resolved.end(),
                             cuts_restrict_mBBcr_resolved.begin(),
                             cuts_restrict_mBBcr_resolved.end());
  // SR = (SR-specific) + common + mBBcr-baseline
  cuts_SR_resolved.insert(cuts_SR_resolved.end(), cuts_common_resolved.begin(),
                          cuts_common_resolved.end());
  cuts_SR_resolved.insert(cuts_SR_resolved.end(),
                          cuts_mBBcr_resolved_baseline.begin(),
                          cuts_mBBcr_resolved_baseline.end());
  // topCR = top-specific + common
  cuts_top_resolved.insert(cuts_top_resolved.end(),
                           cuts_common_resolved.begin(),
                           cuts_common_resolved.end());

  // --- Cuts for merged analysis --- //
  std::vector<unsigned long int> cuts_common_merged = {
      DiLeptonCuts::Trigger,         DiLeptonCuts::LeadLepPt27,
      DiLeptonCuts::AtLeast1FJ,      DiLeptonCuts::LeadFJ1pTJ,
      DiLeptonCuts::pTBothLeptons25, DiLeptonCuts::MuonEtaLT2p5};
  std::vector<unsigned long int> cuts_mBBcr_merged_baseline = {
      DiLeptonCuts::SFLeptons, DiLeptonCuts::MllZwindowMerged,
      DiLeptonCuts::METHTMerged, DiLeptonCuts::PtZMerged};
  std::vector<unsigned long int> cuts_restrict_mBBcr_merged = {
      DiLeptonCuts::mHCorrInsideSideBandMerged};
  std::vector<unsigned long int> cuts_SR_merged = {DiLeptonCuts::mHCorrMerged};
  std::vector<unsigned long int> cuts_top_merged;
  if (m_model == Model::AZh || m_model == Model::HVT)
    cuts_top_merged = {DiLeptonCuts::MllZwindowMerged,
                       DiLeptonCuts::mHCorrMerged, DiLeptonCuts::PtZMerged};
  else if (m_model == Model::CUT || m_model == Model::MVA)
    cuts_top_merged = {DiLeptonCuts::MllAbove40};

  if (m_model == Model::CUT) {
    cuts_common_merged = {DiLeptonCuts::LeadLepPt27, DiLeptonCuts::Trigger,
                          DiLeptonCuts::OSLeptons, DiLeptonCuts::AtLeast1FJ,
                          DiLeptonCuts::LeadFJ2TJ};
    cuts_mBBcr_merged_baseline = {DiLeptonCuts::SFLeptons,
                                  DiLeptonCuts::MllZwindowMerged,
                                  DiLeptonCuts::METHTMerged};
    cuts_SR_merged = {};
    cuts_top_merged = {DiLeptonCuts::MllZwindowMerged};
  }

  // mBBcr = common + mBBcr-baseline + restriction to 50-200 GeV
  std::vector<unsigned long int> cuts_mBBcr_merged;
  cuts_mBBcr_merged.insert(cuts_mBBcr_merged.end(), cuts_common_merged.begin(),
                           cuts_common_merged.end());
  cuts_mBBcr_merged.insert(cuts_mBBcr_merged.end(),
                           cuts_mBBcr_merged_baseline.begin(),
                           cuts_mBBcr_merged_baseline.end());
  cuts_mBBcr_merged.insert(cuts_mBBcr_merged.end(),
                           cuts_restrict_mBBcr_merged.begin(),
                           cuts_restrict_mBBcr_merged.end());
  // SR = (SR-specific) + common + mBBcr-baseline
  cuts_SR_merged.insert(cuts_SR_merged.end(), cuts_common_merged.begin(),
                        cuts_common_merged.end());
  cuts_SR_merged.insert(cuts_SR_merged.end(),
                        cuts_mBBcr_merged_baseline.begin(),
                        cuts_mBBcr_merged_baseline.end());
  // topCR = top-specific + common
  cuts_top_merged.insert(cuts_top_merged.end(), cuts_common_merged.begin(),
                         cuts_common_merged.end());

  /////////////////////////////////////////////////////////////
  //      DECISION TO USE MERGED OR RESOLVED ANALYSIS        //
  /////////////////////////////////////////////////////////////
  selectRegime(eventFlag, ZVec.Pt(), cuts_SR_resolved, cuts_SR_merged,
               cuts_mBBcr_resolved, cuts_mBBcr_merged, cuts_top_resolved,
               cuts_top_merged);

  /////////////////////////////////////////////////////////////
  //         EVENT CATEGORIZATION BASED ON BIT FLAG          //
  /////////////////////////////////////////////////////////////

  // choose right analysis model
  //---------------------------
  if (m_model == Model::MVA) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::MVA);
  } else if (m_model == Model::CUT) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
  } else if (m_model == Model::AZh || m_model == Model::HVT) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::VHres);
  }

  // Event flavor, nTag, nJet, pTV
  //------------------------------
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    setevent_flavour(selectedJets);
    m_histNameSvc->set_nTag(tagcatExcl);
    m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
    if (!m_doMergePtVBins) m_histNameSvc->set_pTV(ZVec.Pt());
    if (m_doMergeJetBins && m_physicsMeta.nJets >= 2)
      m_histNameSvc->set_nJet(-2);
    else
      m_histNameSvc->set_nJet(m_physicsMeta.nJets);

    if ((m_model == Model::CUT || m_model == Model::MVA)) {
      if (m_doMergeJetBins && m_physicsMeta.nJets >= 3) {
        m_histNameSvc->set_nJet(-3);
      } else if (m_physicsMeta.nJets >= 4) {
        m_histNameSvc->set_nJet(-4);
      } else {
        m_histNameSvc->set_nJet(m_physicsMeta.nJets);
      }
    }
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    // Flavor labeling option: TrackJetCone
    setevent_flavour(trackjetsForLabelling);

    if (m_doMergeModel)
      m_histNameSvc->set_nTrackJetInFatJet(trackJetsInLeadFJ.size());
    m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);
    m_histNameSvc->set_nFatJet(fatJets.size());
    if (!m_doMergePtVBins) m_histNameSvc->set_pTV(ZVec.Pt());
    m_histNameSvc->set_nBTagTrackJetUnmatched(m_physicsMeta.nAddBTrkJets);
    m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
  }

  // Region
  //--------
  m_histNameSvc->set_description("");
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    if (passSpecificCuts(eventFlag, cuts_SR_resolved)) {
      m_histNameSvc->set_description("SR");
    } else if (passSpecificCuts(eventFlag, cuts_mBBcr_resolved) &&
               !passSpecificCuts(eventFlag, {DiLeptonCuts::mHCorrResolved})) {
      if (!m_doMergeCR) {
        if (m_physicsMeta.mbbSideBandResolved ==
            PhysicsMetadata::MbbSideBandResolved::Low)
          m_histNameSvc->set_description("lowmBBcr");
        else if (m_physicsMeta.mbbSideBandResolved ==
                 PhysicsMetadata::MbbSideBandResolved::High)
          m_histNameSvc->set_description("highmBBcr");
      } else if (m_physicsMeta.mbbSideBandResolved ==
                     PhysicsMetadata::MbbSideBandResolved::Low ||
                 m_physicsMeta.mbbSideBandResolved ==
                     PhysicsMetadata::MbbSideBandResolved::High) {
        m_histNameSvc->set_description("mBBcr");
      }
    } else if (passSpecificCuts(eventFlag, cuts_top_resolved) &&
               !passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons})) {
      m_histNameSvc->set_description("topemucr");
    }
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    if (passSpecificCuts(eventFlag, cuts_SR_merged)) {
      m_histNameSvc->set_description("SR");
    } else if (passSpecificCuts(eventFlag, cuts_mBBcr_merged) &&
               !passSpecificCuts(eventFlag, {DiLeptonCuts::mHCorrMerged})) {
      if (!m_doMergeCR) {
        if (m_physicsMeta.mbbSideBandMerged ==
            PhysicsMetadata::MbbSideBandMerged::Low)
          m_histNameSvc->set_description("lowmBBcr");
        else if (m_physicsMeta.mbbSideBandMerged ==
                 PhysicsMetadata::MbbSideBandMerged::High)
          m_histNameSvc->set_description("highmBBcr");
      } else if (m_physicsMeta.mbbSideBandMerged ==
                     PhysicsMetadata::MbbSideBandMerged::Low ||
                 m_physicsMeta.mbbSideBandMerged ==
                     PhysicsMetadata::MbbSideBandMerged::High) {
        m_histNameSvc->set_description("mBBcr");
      }
    } else if (passSpecificCuts(eventFlag, cuts_top_merged) &&
               !passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons})) {
      m_histNameSvc->set_description("topemucr");
    }
  }

  /////////////////////////////////////////////////////////////
  // **RE-Definition** : Higgs, ZH - according to regime for HIST FILLING
  TLorentzVector HVec, bbjVec, HVecPtReco;
  TLorentzVector VHVec;

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    HVec = resolvedH.vec_corr;
    HVecPtReco = resolvedH.vec_ptreco;
    if (m_physicsMeta.nJets > 2) bbjVec = resolvedH.vec_ptreco + j3.vec;
    // if(m_histNameSvc->get_description() == "SR") VHVec = resolvedVH.vec_resc;
    // else
    VHVec = resolvedVH.vec_corr;
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    HVec = mergedH.vec_corr;
    if (fatJets.size() > 1) bbjVec = mergedH.vec_corr + j2.vec_corr;
    bool rescaleFatJetMass = false;
    m_config->getif<bool>("rescaleFatJetMass", rescaleFatJetMass);
    if (rescaleFatJetMass && m_histNameSvc->get_description() == "SR") {
      VHVec = mergedVH.vec_resc;
    } else
      VHVec = mergedVH.vec_corr;
  }

  TLorentzVector HVec_gsc, HVec_onemu, HVec_ptreco, HVec_kf;
  TLorentzVector VHVec_gsc, VHVec_onemu, VHVec_ptreco, VHVec_kf;

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    HVec_gsc = resolvedH.vec_gsc;
    HVec_onemu = resolvedH.vec_onemu;
    HVec_ptreco = resolvedH.vec_ptreco;
    HVec_kf = resolvedH.vec_kf;
    VHVec_gsc = resolvedVH.vec_gsc;
    VHVec_onemu = resolvedVH.vec_onemu;
    VHVec_ptreco = resolvedVH.vec_ptreco;
    VHVec_kf = resolvedVH.vec_kf;
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    HVec_gsc = mergedH.vec_gsc;
    HVec_onemu = mergedH.vec_onemu;
    HVec_ptreco = mergedH.vec_ptreco;
    HVec_kf = mergedH.vec_kf;
    VHVec_gsc = mergedVH.vec_gsc;
    VHVec_onemu = mergedVH.vec_onemu;
    VHVec_ptreco = mergedVH.vec_ptreco;
    VHVec_kf = mergedVH.vec_kf;
  }

  /////////////////////////////////////////////////////////////
  // **Selection** : check if in blinded region
  // m_doBlinding = isBlindedRegion(eventFlag, m_physicsMeta.regime ==
  // PhysicsMetadata::Regime::merged);
  m_doBlinding = false;

  /////////////////////////////////////////////////////////////
  // **Calculate** : b-tagging SF
  float btagWeight = 1.;  // for later use
  if (m_isMC) {
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
      btagWeight =
          computeBTagSFWeight(signalJets, m_jetReader->getContainerName());
    } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
      btagWeight = computeBTagSFWeight(trackJetsForBTagging,
                                       m_trackJetReader->getContainerName());
    }
    m_weight *= btagWeight;
    if (m_doTruthTagging)
      BTagProps::truthTagEventWeight.set(m_eventInfo, btagWeight);
  }
  /////////////////////////////////////////////////////////////
  // store w/ / w/o PU weight as systematic variation
  // depends on what setting was chosen in config
  if (!m_config->get<bool>("applyPUWeight"))
    m_weightSysts.push_back({"_withPU", (float)(m_pileupReweight)});

  /////////////////////////////////////////////////////////////
  // Print event weights
  bool printEventWeightsForCutFlow = false;
  m_config->getif<bool>("printEventWeightsForCutFlow",
                        printEventWeightsForCutFlow);
  if (m_isMC && printEventWeightsForCutFlow &&
      passSpecificCuts(
          eventFlag,
          {DiLeptonCuts::Trigger, DiLeptonCuts::SFLeptons,
           DiLeptonCuts::MllZwindowResolved, DiLeptonCuts::METHTResolved,
           DiLeptonCuts::AtLeast2Jets, DiLeptonCuts::AtLeast2SigJets,
           DiLeptonCuts::AtLeast1B, DiLeptonCuts::Exactly2B})) {
    printf(
        "Run: %7u  ,  Event: %8llu  -  PU = %6.3f  ,  Lepton = %5.3f  ,  "
        "Trigger = %5.3f , BTag = %5.3f\n",
        m_eventInfo->runNumber(), m_eventInfo->eventNumber(), m_pileupReweight,
        Props::leptonSF.get(m_eventInfo), triggerSF, btagWeight);
  }
  //  if ( passSpecificCuts(eventFlag, cuts_SR_merged) ) {
  //    printf("-> Event = %8llu  %6.3f   %8.7f   %6.3f   %6.1f   %1d   %1d
  //    %6.4f   %6.4f   %8.7f   %1d\n",
  //           m_eventInfo->eventNumber(),
  //           fatJets.at(0)->p4().Pt()/1.e3,
  //           fatJets.at(0)->p4().Eta(),
  //           fatJets.at(0)->p4().M()/1.e3,
  //           ZVec.M(),
  //           Props::nBTags.get(fatJets.at(0)),
  //           trackJetsInLeadFJ.size(),
  //           l1.vec.Pt(),
  //           l2.vec.Pt(),
  //           l1.vec.Eta(),
  //           l2.vec.Eta(),
  //           m_physicsMeta.nAddBTrkJets);
  //  }
  //  else {
  //    if ( fatJets.size() > 0 )
  //    printf("-> Event = %8llu  %6.3f   %8.7f   %6.3f   %6.1f   %1d   %1d
  //    %6.4f   %6.4f   %8.7f   %1d   FAIL\n",
  //           m_eventInfo->eventNumber(),
  //           fatJets.at(0)->p4().Pt()/1.e3,
  //           fatJets.at(0)->p4().Eta(),
  //           fatJets.at(0)->p4().M()/1.e3,
  //           ZVec.M(),
  //           Props::nBTags.get(fatJets.at(0)),
  //           trackJetsInLeadFJ.size(),
  //           l1.vec.Pt(),
  //           l2.vec.Pt(),
  //           l1.vec.Eta(),
  //           l2.vec.Eta(),
  //           m_physicsMeta.nAddBTrkJets);
  //    else printf("-> Event = %8llu  0 fat-jets\n");
  //  }

  bool isNominal = false;
  if (m_currentVar == "Nominal") isNominal = true;
  /////////////////////////////////////////////////////////////
  // **CorrsAndSyst**:
  // resolved
  if (m_isMC && (m_model != Model::AZh && m_model != Model::HVT) &&
      (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) &&
      ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    // Check quantities (truth, reco, missing, ...) --> not well defined yet

    TLorentzVector j1Temp = signalJets.at(0)->p4();
    TLorentzVector j2Temp = signalJets.at(1)->p4();

    float cs_dr = fabs(j1Temp.DeltaR(j2Temp));
    float cs_dphi = fabs(j1Temp.DeltaPhi(j2Temp));
    float cs_vpt = ZVec.Pt();
    float cs_mbb = (j1.vec + j2.vec).M();  // need to use uncorrected/scaled
                                           // jets
    float cs_ptb1 = j1.vec.Pt();
    float cs_ptb2 = j2.vec.Pt();
    float cs_met = met->met();
    float cs_njet = (m_physicsMeta.nSigJet <= 3 ? m_physicsMeta.nSigJet : 3);
    float cs_ntag = tagcatExcl;
    float cs_truthPt = -1;
    float cs_avgTopPt = -1;

    // truth-level variables --> please check the definition if you want to
    // apply the systematic variations from CorrsAndSysts to samples different
    // from the nominal choices PowhegPythia6 for ttbar Sherpa for W+jets,
    // Z+jets, diboson
    truthVariablesCS(cs_truthPt, cs_avgTopPt);
    applyCS(cs_vpt, cs_mbb, cs_truthPt, cs_dphi, cs_dr, cs_ptb1, cs_ptb2,
            cs_met, cs_avgTopPt, cs_njet, cs_ntag);
  }

  // new Systematics for Vh resonance PRSR analysis
  bool nominalOnly = false;
  bool doModelSyst = true;
  m_config->getif<bool>("nominalOnly", nominalOnly);
  m_config->getif<bool>("doModelSyst", doModelSyst);

  if (m_isMC && (m_model == Model::AZh || m_model == Model::HVT)) {
    std::string Regime =
        (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) ? "Resolved"
                                                                    : "Boosted";
    bool isEMu = (passSpecificCuts(eventFlag, cuts_top_resolved) &&
                  !passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons}));

    if (m_histNameSvc->get_sample() == "ttbar") {
      if (!nominalOnly && isNominal && doModelSyst)
        fillTopSystslep_PRSR(Regime, m_physicsMeta.nAddBTrkJets, HVec.M(),
                             VHVec.M() / 1e3, isEMu);  // mVH= GeV, mBB = MeV
    } else if (m_histNameSvc->get_sample() == "W" ||
               m_histNameSvc->get_sample() == "Wv22" ||
               m_histNameSvc->get_sample() == "Z" ||
               m_histNameSvc->get_sample() == "Zv22") {
      if (!nominalOnly && isNominal && doModelSyst)
        fillVjetsSystslep_PRSR(Regime, HVec.M(),
                               VHVec.M() / 1e3);  // mVH= GeV, mBB = MeV
    }
  }

  // -----------------------------------------------------------------
  // DEPRECATED
  // Sherpa weight variations -> input for modelling syst
  // will only be run if appropriate syst are added to csVariations
  //(MUR0.5_MUF1_PDF261000 MUR2_MUF1_PDF261000 MUR1_MUF0.5_PDF261000
  // MUR1_MUF2_PDF261000 MUR0.5_MUF0.5_PDF261000 MUR2_MUF2_PDF261000
  // MUR1_MUF1_PDF25300 MUR1_MUF1_PDF13000)
  // if (!nominalOnly && m_currentVar == "Nominal" && m_isMC)
  //   applySherpaVJet_EvtWeights();
  // -----------------------------------------------------------------

  //-----------------------------------------------
  // Internal weight variations Histograms
  //-----------------------------------------------
  if (m_evtWeightVarMode != -1 && m_currentVar == "Nominal" && m_isMC) {
    EL_CHECK("applyEvtWeights()", apply_EvtWeights());
  }

  /////////////////////////////////////////////////////////////
  //          TREE and HISTO filling                         //
  /////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////
  // **Trees**: only fill trees if there are more than 2 signal jets or more
  // than 1 fat-jet
  // easy tree
  if (m_physicsMeta.nSigJet >= 2 || fatJets.size() >= 1) {
    // just used for shuffeling
    (*m_etree)["L1Vec"] = l1.vec;
    (*m_etree)["L2Vec"] = l2.vec;
    (*m_etree)["ZVec"] = ZVec;
    (*m_etree)["HVec"] = HVec;
    (*m_etree)["VHVec"] = VHVec;
    if (m_physicsMeta.nJets > 2) (*m_etree)["bbjVec"] = bbjVec;

    // written to output
    m_etree->SetBranchAndValue<std::string>(
        "Description", m_histNameSvc->get_description(), "");
    m_etree->SetBranchAndValue<std::string>("Sample",
                                            m_histNameSvc->getFullSample(), "");
    m_etree->SetBranchAndValue<std::string>(
        "EventFlavor", m_histNameSvc->getEventFlavour(), "");
    if (m_isMC)
      m_etree->SetBranchAndValue<float>("MCChannelNumber", m_mcChannel, -99999);
    m_etree->SetBranchAndValue<float>("EventWeight", m_weight, -99999999);
    if (m_isMC)
      m_etree->SetBranchAndValue<float>(
          "MCEventWeight", Props::MCEventWeight.get(m_eventInfo), -99999999);
    m_etree->SetBranchAndValue<float>(
        "LumiWeight", Props::LumiWeight.get(m_eventInfo), -99999999);
    float NTruthJetWeight = Props::NTruthWZJets20.exists(m_eventInfo)
                                ? Props::NTruthWZJets20.get(m_eventInfo)
                                : -99;
    m_etree->SetBranchAndValue<float>("NTruthJetWeight", NTruthJetWeight,
                                      -99999999);
    m_etree->SetBranchAndValue<float>("PUWeight", m_pileupReweight, -99999999);
    m_etree->SetBranchAndValue<float>("BTagSF", btagWeight, -99999999);
    m_etree->SetBranchAndValue<float>("TriggerSF", triggerSF, -99999999);
    m_etree->SetBranchAndValue<float>("LeptonSF",
                                      Props::leptonSF.get(m_eventInfo), -99999);
    m_etree->SetBranchAndValue<float>("JVTWeight", JVTweight, -999999);
    m_etree->SetBranchAndValue<int>("EventNumber", m_eventInfo->eventNumber(),
                                    -1);
    m_etree->SetBranchAndValue<int>(
        "passedTrigger", passSpecificCuts(eventFlag, {DiLeptonCuts::Trigger}),
        -1);
    m_etree->SetBranchAndValue<int>(
        "passedTauVeto", passSpecificCuts(eventFlag, {DiLeptonCuts::tauVeto}),
        -1);
    m_etree->SetBranchAndValue<float>("AverageMu", m_averageMu, -99);
    m_etree->SetBranchAndValue<float>("ActualMu", m_actualMu, -99);
    m_etree->SetBranchAndValue<float>("AverageMuScaled", m_averageMuScaled,
                                      -99);
    m_etree->SetBranchAndValue<float>("ActualMuScaled", m_actualMuScaled, -99);
    int Nvtx = 0;
    if (Props::NVtx2Trks.exists(m_eventInfo))
      Nvtx = Props::NVtx2Trks.get(m_eventInfo);
    else if (Props::NVtx3Trks.exists(m_eventInfo))
      Nvtx = Props::NVtx3Trks.get(m_eventInfo);
    m_etree->SetBranchAndValue<int>("Nvtx", Nvtx, -99);
    m_etree->SetBranchAndValue<float>("ZPV", Props::ZPV.get(m_eventInfo), -99);
    m_etree->SetBranchAndValue<float>("ptL1", l1.vec.Pt() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("ptL2", l2.vec.Pt() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("etaL1", l1.vec.Eta(), -99);
    m_etree->SetBranchAndValue<float>("etaL2", l2.vec.Eta(), -99);
    m_etree->SetBranchAndValue<float>("flavL1", l1.flav, -99);
    m_etree->SetBranchAndValue<float>("flavL2", l2.flav, -99);
    int isMediumL1 = 1;
    int isMediumL2 = 1;
    if (isE) {
      isMediumL1 = Props::isMediumLH.get(el1);
      isMediumL2 = Props::isMediumLH.get(el2);
    }
    m_etree->SetBranchAndValue<int>("isMediumL1", isMediumL1, -99);
    m_etree->SetBranchAndValue<int>("isMediumL2", isMediumL2, -99);
    m_etree->SetBranchAndValue<float>("chargeL1", l1.charge, -99);
    m_etree->SetBranchAndValue<float>("chargeL2", l2.charge, -99);
    m_etree->SetBranchAndValue<float>("dEtaLL",
                                      fabs(l1.vec.Eta() - l2.vec.Eta()), -99);
    m_etree->SetBranchAndValue<float>("dPhiLL", fabs(l1.vec.DeltaPhi(l2.vec)),
                                      -99);
    m_etree->SetBranchAndValue<float>("mLL", ZVec.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("pTV", ZVec.Pt() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("MET", met->met() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("HT", HTsmallR / 1e3, -99);
    m_etree->SetBranchAndValue<float>("METHT", METHTsmallR / sqrt(1000.), -99);
    m_etree->SetBranchAndValue<float>(
        "dEtaBBcor", fabs(j1.vec_corr.Eta() - j2.vec_corr.Eta()), -99);
    m_etree->SetBranchAndValue<float>(
        "dPhiBBcor", fabs(j1.vec_corr.DeltaPhi(j2.vec_corr)), -99);
    m_etree->SetBranchAndValue<float>(
        "dRBBcor", fabs(j1.vec_corr.DeltaR(j2.vec_corr)), -99);
    m_etree->SetBranchAndValue<float>("dPhiVBB", fabs(ZVec.DeltaPhi(HVec)),
                                      -99);
    m_etree->SetBranchAndValue<float>("dEtaVBB", fabs(ZVec.Eta() - HVec.Eta()),
                                      -99);
    m_etree->SetBranchAndValue<float>("dPhiVBBPtReco",
                                      fabs(ZVec.DeltaPhi(HVecPtReco)), -99);
    m_etree->SetBranchAndValue<float>("dEtaVBBPtReco",
                                      fabs(ZVec.Eta() - HVecPtReco.Eta()), -99);
    m_etree->SetBranchAndValue<float>("dRVBB", fabs(ZVec.DeltaR(HVec)), -99);
    m_etree->SetBranchAndValue<float>("pTVH", VHVec.Pt() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("mVH", VHVec.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("mVHres", resolvedVH.vec_resc.M() / 1e3,
                                      -99);
    m_etree->SetBranchAndValue<float>("mVHmerg", mergedVH.vec_corr.M() / 1e3,
                                      -99);
    m_etree->SetBranchAndValue<float>("mBBres", resolvedH.vec_corr.M() / 1e3,
                                      -99);
    m_etree->SetBranchAndValue<float>("mBBmerg", mergedH.vec_corr.M() / 1e3,
                                      -99);
    m_etree->SetBranchAndValue<float>("mLB", lbVec.M() / 1e3, -99);
    m_etree->SetBranchAndValue<int>("nTaus", taus.size(), -99);
    // m_etree->SetBranchAndValue("blind", (), false);
    EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
             compute_jetSelectedQuantities(selectedJets));
    // Adding here the fat-jet and track-jet related quantities in order to
    // avoid changing compute_jetSelectedQuantities which is used by the other
    // analyses
    m_etree->SetBranchAndValue<int>("nFatJets", fatJets.size(), -99);
    if (fatJets.size() >= 1) {
      m_etree->SetBranchAndValue<int>("nbTagsInFJ", m_physicsMeta.nTagsInFJ,
                                      -99);
      m_etree->SetBranchAndValue<int>(
          "nTrkjetsInFJ", Props::nTrackJets.get(fatJets.at(0)), -99);
    }
    m_etree->SetBranchAndValue<int>("nbTagsOutsideFJ",
                                    m_physicsMeta.nAddBTrkJets, -99);
    m_etree->SetBranchAndValue<int>("nTrkJets", trackJets.size(), -99);
    m_etree->SetBranchAndValue<int>("nBTrkJets", nTaggedTrkJets, -99);
    m_etree->SetBranchAndValue<float>("HTfat", HTlargeR, -99);
    m_etree->SetBranchAndValue<float>("METHTfat", METHTlargeR, -99);
    if (fatJets.size() >= 1) (*m_etree)["fatj1Vec"] = fatJets.at(0)->p4();
    if (fatJets.size() >= 2) (*m_etree)["fatj2Vec"] = fatJets.at(1)->p4();
    if (fatJets.size() >= 3) (*m_etree)["fatj3Vec"] = fatJets.at(2)->p4();
    if (trackJets.size() >= 1) (*m_etree)["trkj1Vec"] = trackJets.at(0)->p4();
    if (trackJets.size() >= 2) (*m_etree)["trkj2Vec"] = trackJets.at(1)->p4();
    if (trackJets.size() >= 3) (*m_etree)["trkj3Vec"] = trackJets.at(2)->p4();
    if (trackJetsInLeadFJ.size() >= 1) {
      (*m_etree)["trkj1LeadFJVec"] = trackJetsInLeadFJ.at(0)->p4();
      // m_etree->SetBranchAndValue<float>("trkj1LeadFJ_MV2c20",
      // Props::MV2c20.get(trackJetsInLeadFJ.at(0)), -99);
      if (trackJetsInLeadFJ.size() >= 2) {
        (*m_etree)["trkj2LeadFJVec"] = trackJetsInLeadFJ.at(1)->p4();
        // m_etree->SetBranchAndValue<float>("trkj2LeadFJ_MV2c20",
        // Props::MV2c20.get(trackJetsInLeadFJ.at(1)), -99);
      }
    }

    m_etree->SetBranchAndValue<float>("GSCMbb", HVec_gsc.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("OneMuMbb", HVec_onemu.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("PtRecoMbb", HVec_ptreco.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("KFMbb", HVec_kf.M() / 1e3, -99);

    m_etree->SetBranchAndValue<float>("GSCMvh", VHVec_gsc.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("OneMuMvh", VHVec_onemu.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("PtRecoMvh", VHVec_ptreco.M() / 1e3, -99);
    m_etree->SetBranchAndValue<float>("KFMvh", VHVec_kf.M() / 1e3, -99);

    // Regime
    string Regime = "none";
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
      Regime = "merged";
    } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved)
      Regime = "resolved";

    m_etree->SetBranchAndValue<std::string>("Regime", Regime, "none");

    // MVA tree
    // Elisabeth: comment out
    // -> you could use AnalysisReader_VHQQ2Lep::fillMVATreeVHbbResolved2Lep
    // -> if additional variables needed:
    //        -> use EasyTree or
    //        -> create MVATree_VHreso

    // m_tree->EventWeight = m_weight;
    // m_tree->sample = m_histNameSvc->getFullSample();
    // m_tree->EventNumber = m_eventInfo->eventNumber();
    // m_tree->nJ = m_physicsMeta.nJets;
    // m_tree->dPhiVBB = fabs(ZVec.DeltaPhi(HVecPtReco));
    // m_tree->dEtaVBB = fabs(ZVec.Eta() - HVecPtReco.Eta());
    // m_tree->pTV = ZVec.Pt();
    // m_tree->mLL = ZVec.M();
    // m_tree->MET = met->met();
    // m_tree->HT = HTsmallR;
    // m_tree->METHT = METHTsmallR;
    // m_tree->METSig = metSig;
    // if (metSig_PU != -1) {
    //   m_tree->METSig_PU = metSig_PU;
    // } else {
    //   m_tree->METSig_soft = metSig_soft;
    //   m_tree->METSig_hard = metSig_hard;
    // }

    bool writeMVATree = false;
    bool writeEasyTree = false;
    m_config->getif<bool>("writeMVATree", writeMVATree);
    m_config->getif<bool>("writeEasyTree", writeEasyTree);
    // fill trees
    if (writeMVATree && m_histNameSvc->get_isNominal()) {
      m_tree->Fill();
    }
    if (writeEasyTree && m_histNameSvc->get_isNominal()) {
      m_etree->Fill();
    }
  }

  // VHbb
  if (m_physicsMeta.nJets == 2) {
    /////////////////////////////////////////////////////////////
    // **Histos**: only fill histos for defined regions
    bool GenerateSTXS = false;
    m_config->getif<bool>("GenerateSTXS", GenerateSTXS);
    if (!GenerateSTXS || m_physicsMeta.nTags > 2) {
      if (m_histNameSvc->get_description() != "") {
        if (!m_doOnlyInputs)
          EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                   fill_jetHistos(signalJets, forwardJets));
        EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                 fill_jetSelectedHistos());
        if (!m_doOnlyInputs)
          EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                   fill_fatJetHistos(fatJets));
        EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                 fill_2LepHistos(isMu, isE));
        if (!m_doOnlyInputs && !m_doReduceFillHistos)
          m_histSvc->BookFillHist("BDT", 100, 0, 1, m_tree->BDT, m_weight);
      }
    }
  } else {
    if (STXS_GetBinFromEvtInfo() != -1 &&
        m_physicsMeta.nTags < 3) {  // check if it is the STXS signals
      STXS_FillYields();            // a 2-D histogram filled
      std::string temp_name = m_histNameSvc->get_sample();
      STXS_ReplaceName();  // replace the signal name with the STXS bin
      if (m_histNameSvc->get_description() != "") {
        if (!m_doOnlyInputs)
          EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                   fill_jetHistos(signalJets, forwardJets));
        EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                 fill_jetSelectedHistos());
        if (!m_doOnlyInputs)
          EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                   fill_fatJetHistos(fatJets));
        EL_CHECK("AnalysisReader_VHreso2Lep::run_2Lep_analysis",
                 fill_2LepHistos(isMu, isE));
        if (!m_doOnlyInputs && !m_doReduceFillHistos)
          m_histSvc->BookFillHist("BDT", 100, 0, 1, m_tree->BDT, m_weight);
      }
      m_histNameSvc->set_sample(
          temp_name);  // set the histogram name back from STXS addition
    }
  }

  // Info("AnalysisReader_VHreso2Lep::run_2Lep_analysis()", "Region: %s, and
  // nTags: %d, njet: %d ", m_histNameSvc->get_description().c_str(),
  // m_physicsMeta.nTags, m_histNameSvc->get_nJet());

  return EL::StatusCode::SUCCESS;
}  // run_2Lep_analysis
