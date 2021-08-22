#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader/OSTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb0lep.h"
#include "CxAODTools_VHbb/VHbb0lepEvtSelection.h"

#include <CxAODReader_VHbb/AnalysisReader_VHbb0Lep.h>
#include <TMVA/Reader.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHbb0Lep::AnalysisReader_VHbb0Lep()
    : AnalysisReader_VHQQ0Lep() {}

AnalysisReader_VHbb0Lep::~AnalysisReader_VHbb0Lep() {}
EL::StatusCode AnalysisReader_VHbb0Lep::run_0Lep_analysis() {
  if (m_debug) {
    Info("AnalysisReader_VHbb0Lep::run_0Lep_analysis()",
         "Region: %s, and nTags: %d, njet: %d ",
         m_histNameSvc->get_description().c_str(), m_physicsMeta.nTags,
         m_histNameSvc->get_nJet());
  }

  // Choose right analysis model
  //-----------------------------
  if (m_model == Model::MVA) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::MVA);
  } else if (m_model == Model::CUT) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
  }

  // Reset physics metadata
  m_physicsMeta = PhysicsMetadata();

  // Initialize eventFlag - all cuts are initialized to false by default
  unsigned long int eventFlag = 0;

  ResultVHbb0lep selectionResult =
      ((VHbb0lepEvtSelection *)m_eventSelection)->result();

  m_physicsMeta.channel = PhysicsMetadata::Channel::ZeroLep;

  const xAOD::MissingET *met = selectionResult.met;
  std::vector<const xAOD::Jet *> signalJets = selectionResult.signalJets;
  std::vector<const xAOD::Jet *> forwardJets = selectionResult.forwardJets;
  std::vector<const xAOD::Jet *> fatJets = selectionResult.fatJets;
  std::vector<const xAOD::Jet *> trackJets = selectionResult.trackJets;
  std::vector<const xAOD::Jet *> subJets = selectionResult.subJets;
  std::vector<const xAOD::TauJet *> taus = selectionResult.taus;

  unsigned int pTthreshold = 70;
  std::vector<const xAOD::Jet *> JetsForAntiQCD = set_JetsForAntiQCD(
      fatJets, signalJets, forwardJets,
      pTthreshold);  // vector with all central +  fwd jets not matched to the
                     // fat jet with a pT > 70 GeV.

  setevent_nJets(signalJets, forwardJets, fatJets);

  // Preselection:
  // Resolved: If doOnlyInputs or ReduceFillHistos one does not need to process
  // n_jet != {2,3} events Merged: If doOnlyInputs or ReduceFillHistos one does
  // not need to process n_fatjet != 1 (for the moment)

  // Resolved:
  if ((m_analysisStrategy == "Resolved") &&
      (m_model == Model::MVA || m_model == Model::CUT) &&
      (m_doOnlyInputs || m_doReduceFillHistos)) {
    if (!(m_physicsMeta.nJets <= 3)) {
      // store only 2jet and 3jet that we really use in fit, so return now
      return EL::StatusCode::SUCCESS;
    }
    // Merged:
  } else if ((m_analysisStrategy == "Merged") && (m_model == Model::CUT) &&
             (m_doOnlyInputs || m_doReduceFillHistos)) {
    if (!(m_physicsMeta.nFatJet >= 1)) {
      return EL::StatusCode::SUCCESS;
    }
  }

  // In both resolved and boosted analysis, if doOnlyInputs =true we only save
  // 2tag histo that are used in the fit. We apply this request in the following
  // lines.

  ////////////////////////////////////////////////////////////
  //         OBJECT DEFINITIONS                             //
  ////////////////////////////////////////////////////////////

  // **Definition** : b-tagging for small-R jets
  //---------------------------------------------------------
  // TODO: Harmonize large-R jet / track-jet association with resonant analysis
  if (m_doTruthTagging) {
    if (!(m_analysisStrategy == "Merged"))  //  For merged computed late
      compute_TRF_tagging(signalJets);
  } else {
    compute_btagging();
  }

  int tagcatExcl = -1;
  std::vector<const xAOD::Jet *> selectedJets;
  selectedJets.clear();

  if (m_analysisStrategy == "Resolved") {  // removing the possibility
                                           // of tagging R04 for merged
    tagjet_selection(signalJets, forwardJets, selectedJets, tagcatExcl);
    m_physicsMeta.nTags = tagcatExcl;
  }

  // Tag-preselection for resolved in reduced mode
  if ((m_analysisStrategy == "Resolved") &&
      (m_model == Model::MVA || m_model == Model::CUT) &&
      (m_doOnlyInputs || m_doReduceFillHistos)) {
    if (!(m_physicsMeta.nTags == 2)) {
      // store only 2tag that we really use in fit, so return now
      return EL::StatusCode::SUCCESS;
    }
  }

  // **Definition** : jets
  //---------------------------------------------------------
  Jet j1, j2, j3;  // small-R jets, taken from signal/fwd jets
  Jet j1_sel, j2_sel,
      j3_sel;  // small-R jets for mBB calculation, taken from selectedJets
  Jet fj1, fj2, fj3;  // fat jets

  if (m_physicsMeta.nSigJet >= 1) j1.vec = signalJets.at(0)->p4();
  if (m_physicsMeta.nSigJet >= 2) j2.vec = signalJets.at(1)->p4();
  if (m_physicsMeta.nSigJet >= 3)
    j3.vec = signalJets.at(2)->p4();
  else if (m_physicsMeta.nForwardJet >= 1)
    j3.vec = forwardJets.at(0)->p4();

  // Set pTV for FSR
  double ptv_for_FSR = met->met() / 1e3;

  EL_CHECK("AnalysisReader_VHbb0Lep::run_0Lep_analysis()",
           setJetVariables(j1_sel, j2_sel, j3_sel, selectedJets, ptv_for_FSR));

  // Vectors to hold the trackjets in the leading fat-jet, outside of the
  // leading fat jet and the ones used to compute the b-tagging weight
  //...for event labelling used in the merged analysis
  std::vector<const xAOD::Jet *> trackJetsInFatJet1, trackJetsInFatJet2,
      trackJetsInFatJet3;  // trackJetsInFatJet1, trackJetsInFatJet2,
                           // trackJetsInFatJet3 contain respectively all the
                           // track jets matched to the leading, sub-leading,
                           // sub-sub-leading fat jets (independent of whether
                           // or not any of these track jets are b-tagged)
  std::vector<const xAOD::Jet *> trackJetsNotInFatJet1, trackJetsNotInFatJet2,
      trackJetsNotInFatJet3;  // trackJetsNotInFatJet1, trackJetsNotInFatJet2,
                              // trackJetsNotInFatJet3 contain respectively all
                              // the track jets outside the leading,
                              // sub-leading, sub-sub-leading fat jets
                              // (independent of whether or not any of these
                              // track jets are b-tagged)
  std::vector<const xAOD::Jet *> trackJetsForBTagSF;
  std::vector<const xAOD::Jet *> bTaggedUnmatchedTrackJets;

  if (fatJets.size() == 0) {  //  TT for boosted: necessary to run b-tagging in
                              //  TRF mode on all jets
    for (auto trackJet : trackJets) trackJetsNotInFatJet1.push_back(trackJet);
  }

  if (fatJets.size() >= 1) {
    matchTrackJetstoFatJets(trackJets, fatJets.at(0), &trackJetsInFatJet1,
                            &trackJetsNotInFatJet1);
  }
  if (fatJets.size() >= 2) {
    matchTrackJetstoFatJets(trackJets, fatJets.at(1), &trackJetsInFatJet2,
                            &trackJetsNotInFatJet2);
  }
  if (fatJets.size() >= 3) {
    matchTrackJetstoFatJets(trackJets, fatJets.at(2), &trackJetsInFatJet3,
                            &trackJetsNotInFatJet3);
  }

  //  compute TT for boosted
  if (m_doTruthTagging) {
    if (m_analysisStrategy != "Resolved") {
      compute_TRF_tagging(trackJetsInFatJet1, trackJetsNotInFatJet1,
                          true);  // TT on trackJetsInFatJet1,
                                  // DT on trackJetsNotInFatJet1,
                                  // true = these are trackjets
    }
  }

  int boostedtagcatExcl = -1;
  int nAddBTags = -1;
  std::vector<const xAOD::Jet *>
      selectedtrackJetsInFatJet;  // subset of the track jets matched to the
                                  // LFJ, it depends on the tag strategy.
                                  // The leading two jets define the #b-tags

  selectedtrackJetsInFatJet.clear();
  if (fatJets.size() >= 1) {
    tagTrackjet_selection(fatJets.at(0), trackJetsInFatJet1,
                          trackJetsNotInFatJet1, selectedtrackJetsInFatJet,
                          bTaggedUnmatchedTrackJets, trackJetsForBTagSF,
                          boostedtagcatExcl, nAddBTags);
  }
  if (fatJets.size() >= 2) {
    set_BtagProperties_fatJets(fatJets.at(1), trackJetsInFatJet2,
                               trackJetsNotInFatJet2);
  }
  if (fatJets.size() >= 3) {
    set_BtagProperties_fatJets(fatJets.at(2), trackJetsInFatJet3,
                               trackJetsNotInFatJet3);
  }

  bool doCoMtagging = false;
  m_config->getif<bool>("doCoMtagging", doCoMtagging);
  std::vector<const xAOD::Jet *>
      selectedCoMsubjets;  // CoM subjets (2) associated with LFJ

  if (doCoMtagging && fatJets.size() >= 1) {
    boostedtagcatExcl =
        -1;  // Re-calculating b-tags in LFJ but now with CoM method

    // CoM does not support DR matching. Exit if selected
    bool forceTrackJetDRmatching = false;
    m_config->getif<bool>("forceTrackJetDRmatching", forceTrackJetDRmatching);
    if (forceTrackJetDRmatching) {
      Error("CoMtagging_LeadFJ()",
            "DR matching is not available for CoM. \n"
            "Select forceTrackJetFRmatching as false to allow Ghost "
            "Association \n"
            "Exiting!");
      return EL::StatusCode::FAILURE;
    } else {
      CoMtagging_LeadFJ(subJets, fatJets.at(0), selectedCoMsubjets,
                        boostedtagcatExcl);
    }
  }

  EL_CHECK("AnalysisReader_VHbb0Lep::run_0Lep_analysis()",
           setFatJetVariables(fj1, fj2, fj3, fatJets));

  // count # calo jets not matched to the leading fat jet
  unsigned int nAdditionalCaloJets =
      countAdditionalCaloJets(signalJets, forwardJets, fj1);

  // get number of b-tagged track jets outside of the leading fat jet
  //# b-tagged track jets
  int nBTagTrackJet = 0;
  for (auto jet : trackJets) {
    if (BTagProps::isTagged.get(jet)) nBTagTrackJet++;
  }

  //# b-tagged track jets matched to leading fat jet
  if (m_physicsMeta.nFatJet > 0) {
    m_physicsMeta.nTagsInFJ = boostedtagcatExcl;
    m_physicsMeta.nAddBTrkJets = nAddBTags;
  }

  // do not save histos for 0 tag and 1 tag categories for the fit inputs
  if ((m_analysisStrategy == "Merged") && m_doOnlyInputs &&
      (m_physicsMeta.nTagsInFJ < 2))
    return EL::StatusCode::SUCCESS;

  // **Definition** : HVec
  //---------------------------------------------------------
  Higgs resolvedH, mergedH;
  setHiggsCandidate(resolvedH, mergedH, j1_sel, j2_sel, fj1);

  // MET vector
  //---------------------------------------------------------
  TLorentzVector metVec;
  metVec.SetPxPyPzE(met->mpx(), met->mpy(), 0, met->met());

  // MET Significance
  //---------------------------------------------------------

  // Elisabeth: not used. comment out to remove compilation warnings
  // if still want to fill EasyTree/histograms with this info
  // need to remove these comments

  // double metSig = -1.;
  // double metSig_PU = -1.;
  // double metSig_hard = -1.;
  // double metSig_soft = -1.;
  // double metOverSqrtSumET = -1.;
  // double metOverSqrtHT = -1.;

  // metSig = Props::metSig.get(met);
  // if (Props::metSig_PU.exists(met)) {
  //   metSig_PU = Props::metSig_PU.get(met);
  // } else {
  //   metSig_hard = Props::metSig_hard.get(met);
  //   metSig_soft = Props::metSig_soft.get(met);
  // }
  // metOverSqrtSumET = Props::metOverSqrtSumET.get(met);
  // metOverSqrtHT = Props::metOverSqrtHT.get(met);

  // MPT vector
  //---------------------------------------------------------
  TLorentzVector mptVec;
  mptVec.SetPxPyPzE(m_mpt->mpx(), m_mpt->mpy(), 0, m_mpt->met());

  //  **Definition** : ZH system
  //---------------------------------------------------------
  VHCand resolvedVH, mergedVH;
  setVHCandidate(resolvedVH, mergedVH, metVec, j1_sel, j2_sel, fj1);

  // sumpt
  // -------------------------------------------------------
  double sumpt = -1;
  if ((m_physicsMeta.nSigJet >= 3) ||
      (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1)) {
    sumpt = j1.vec.Pt() + j2.vec.Pt() + j3.vec.Pt();
  } else if (m_physicsMeta.nSigJet == 2) {
    sumpt = j1.vec.Pt() + j2.vec.Pt();
  }
  m_triggerTool->setSumpt(sumpt);

  // Add mbb cutbased plot for SM MVA analysis to save running full analysis
  // again
  bool passCutBased = false;

  ////////////////////////////////////////////////////////////
  /////////// CHECK SELECTION - STORE INFORMATION   //////////
  ////////////////////////////////////////////////////////////

  // All events (CxAOD)
  //------------------------
  updateFlag(eventFlag, ZeroLeptonCuts::AllCxAOD);

  // **Selection & SF**: trigger
  //----------------------------
  double triggerSF = 1.;
  // C1: trigger
  if (pass0LepTrigger(triggerSF, selectionResult))
    updateFlag(eventFlag, ZeroLeptonCuts::Trigger);
  if (m_isMC) m_weight *= triggerSF;

  // **Selection** : MET cut
  //-----------------------------------------
  if (metVec.Pt() > 150e3) updateFlag(eventFlag, ZeroLeptonCuts::METResolved);
  if (metVec.Pt() > 200e3) updateFlag(eventFlag, ZeroLeptonCuts::METMerged);

  // **Selection** : dPhi(MET,MPT) cut
  //-----------------------------------------
  double dPhiMETMPT = 10000;
  if (!TMath::IsNaN(mptVec.Phi()))
    dPhiMETMPT = fabs(
        metVec.DeltaPhi(mptVec));  // temporary fix, need to understand why mpt
                                   // vec = nan for some systematics variations
  if (dPhiMETMPT < TMath::Pi() / 2.)
    updateFlag(eventFlag, ZeroLeptonCuts::DPhiMETMPT);

  // **Selection** : number of jets
  //----------------------------------------
  if (m_physicsMeta.nJets >= 2)
    updateFlag(eventFlag, ZeroLeptonCuts::AtLeast2Jets);

  // **Selection** : cut on nSignalJet
  //-----------------------------------------
  if (m_physicsMeta.nSigJet >= 2)
    updateFlag(eventFlag, ZeroLeptonCuts::AtLeast2SigJets);
  float JVTweight = 1;
  if (m_isMC) {
    JVTweight = compute_JVTSF(signalJets);
    m_weight *= JVTweight;
  }
  // **Selection** : at least 1 fat jet
  //-----------------------------------------
  if (m_physicsMeta.nFatJet >= 1)
    updateFlag(eventFlag, ZeroLeptonCuts::AtLeast1FatJet);

  // **Selection** : at least 1 fat jet with pT > 250 GeV (pT > 200 GeV & |eta|
  // < 2 cuts applied on preselection level), matched track jet multiplicity,
  // checking overlap of VR track jets
  //-----------------------------------------
  if (m_physicsMeta.nFatJet >= 1) {
    updateFlag(eventFlag, ZeroLeptonCuts::AtLeast1FatJet);

    // Checking for at least 1 (2) track jets matched to the leading fat jet
    // By definition, CoM always have two subjets. Flag is passed by default.

    if (!doCoMtagging) {
      if (trackJetsInFatJet1.size() >= 1)
        updateFlag(eventFlag, ZeroLeptonCuts::AtLeast1MatchedTrackJet);
      if (trackJetsInFatJet1.size() >= 2)
        updateFlag(eventFlag, ZeroLeptonCuts::AtLeast2MatchedTrackJets);
    } else {
      updateFlag(eventFlag, ZeroLeptonCuts::AtLeast2MatchedTrackJets);
    }
    if (!m_doVRJetOR) {
      updateFlag(eventFlag, ZeroLeptonCuts::passVRJetOR);
    } else if (passVRJetOR(trackJetsForBTagSF)) {
      updateFlag(eventFlag, ZeroLeptonCuts::passVRJetOR);
    }
  }
  if (fj1.vec.M() / 1e3 > 50.0) {
    updateFlag(eventFlag, ZeroLeptonCuts::mJ);
  }

  // **Selection** : min[dPhi(MET,jets)] cut
  //-----------------------------------------
  // Resolved: check first three leading jets
  double mindPhi1 = 1000, mindPhi2 = 1000, mindPhi3 = 1000;
  if (m_physicsMeta.nSigJet >= 1) mindPhi1 = fabs(j1.vec.DeltaPhi(metVec));
  if (m_physicsMeta.nSigJet >= 2) mindPhi2 = fabs(j2.vec.DeltaPhi(metVec));
  if ((m_physicsMeta.nSigJet >= 3) ||
      (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1))
    mindPhi3 = fabs(j3.vec.DeltaPhi(metVec));
  double mindPhi = mindPhi1;
  if (mindPhi2 < mindPhi) mindPhi = mindPhi2;
  if (mindPhi3 < mindPhi) mindPhi = mindPhi3;

  if (m_model == Model::CUT || m_model == Model::MVA) {
    if ((m_physicsMeta.nJets < 3) and (mindPhi > 20 * TMath::Pi() / 180))
      updateFlag(eventFlag, ZeroLeptonCuts::MinDPhiMETJetsResolved);
    else if ((m_physicsMeta.nJets >= 3) and (mindPhi > 30 * TMath::Pi() / 180))
      updateFlag(eventFlag, ZeroLeptonCuts::MinDPhiMETJetsResolved);
  }

  // Merged: check all jets
  double mindPhiMerg = TMath::Pi();
  double mindPhiMerg_3LeadJets = 1000;  // TODO
  for (auto jetsforantiQCD : JetsForAntiQCD) {
    double dPhi = fabs(jetsforantiQCD->p4().DeltaPhi(metVec));
    if (dPhi < mindPhiMerg) mindPhiMerg = dPhi;
  }
  if (mindPhiMerg > 30 * TMath::Pi() / 180)
    updateFlag(eventFlag, ZeroLeptonCuts::MinDPhiMETJetsMerged);

  // **Selection** : sum[jet_pt] cut
  //-----------------------------------------
  if ((m_physicsMeta.nSigJet >= 3) ||
      (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1)) {
    if (sumpt > 150e3) updateFlag(eventFlag, ZeroLeptonCuts::SumJetPt);
  } else if (m_physicsMeta.nSigJet == 2) {
    if (sumpt > 120e3) updateFlag(eventFlag, ZeroLeptonCuts::SumJetPt);
  }

  // **Selection** : MPT cut, applied only for 0 and 1 tag, NOT for 2 tag
  //----------------------
  if ((m_physicsMeta.nTags < 2 && mptVec.Pt() >= 30e3) ||
      m_physicsMeta.nTags >= 2)
    updateFlag(eventFlag, ZeroLeptonCuts::MinMPTBJet0or1Resolved);

  // **Selection** : Third b-jet veto : for the moment commented out
  //----------------------
  // if (m_physicsMeta.nTags > 2) updateFlag(eventFlag,
  // ZeroLeptonCuts::ThirdBJetVeto); // 3rd b-veto not applied [histo stored
  // separately for 2tag and 3ptag]

  // **Selection** : pT cut on leading jet
  //----------------------
  if (selectedJets.size() > 0 && selectedJets.at(0)->pt() >= 45e3)
    updateFlag(eventFlag, ZeroLeptonCuts::LeadJetPt);

  // **Selection** : dPhi(b1,b2) cut
  //----------------------
  double dPhiBB = 1000;
  if (selectedJets.size() >= 2) dPhiBB = fabs(j1_sel.vec.DeltaPhi(j2_sel.vec));
  if (dPhiBB <= 140 * TMath::Pi() / 180)
    updateFlag(eventFlag, ZeroLeptonCuts::DPhiBB);

  // **Selection** : dPhi(MET,dijet) cut
  //----------------------
  double dPhiMETdijetResolved = fabs(metVec.DeltaPhi(resolvedH.vec));
  if (dPhiMETdijetResolved >= 120 * TMath::Pi() / 180)
    updateFlag(eventFlag, ZeroLeptonCuts::DPhiMETDijetResolved);
  double dPhiMETdijetMerged = fabs(metVec.DeltaPhi(mergedH.vec));
  if (dPhiMETdijetMerged >= 120 * TMath::Pi() / 180)
    updateFlag(eventFlag, ZeroLeptonCuts::DPhiMETDijetMerged);

  // **Selection** : dR(b1,b2) cut (2015-10-18: added this cut)
  // commented for input production
  //----------------------
  double dRb1b2 = 1000;
  if (selectedJets.size() >= 2) {
    dRb1b2 = fabs(j1_sel.vec.DeltaR(
        j2_sel.vec));  // applied in SM measurement, cut based approach only
    double dRb1b2LowMET_SMCut = 1.8;
    double dRb1b2HighMET_SMCut = 1.2;
    if (metVec.Pt() > 200e3 && dRb1b2 <= dRb1b2HighMET_SMCut)
      updateFlag(eventFlag, ZeroLeptonCuts::DRB1B2);
    else if (metVec.Pt() <= 200e3 && dRb1b2 <= dRb1b2LowMET_SMCut)
      updateFlag(eventFlag, ZeroLeptonCuts::DRB1B2);
  }

  // **Selection** :  MET 500
  //------------------------
  if (met->met() / 1.e3 < 500.) updateFlag(eventFlag, ZeroLeptonCuts::METLT500);
  if (met->met() / 1.e3 >= 500.)
    updateFlag(eventFlag, ZeroLeptonCuts::METGT500);

  // FSR: after the selection, migrate FSR events
  if (m_doFSRrecovery && m_hasFSR > 0.5)
    migrateCategoryFSR(signalJets, forwardJets);

  // **Selection** : mbb invariant mass window + mbb-sidebands
  //----------------------
  bool passMHCorrResolved(false), passMHCorrMerged(false),
      passMHResolvedSideBand(false), passMHMergedSideBand(false);
  checkMbbWindow(resolvedH.vec_corr, mergedH.vec_corr, passMHCorrResolved,
                 passMHCorrMerged, passMHResolvedSideBand,
                 passMHMergedSideBand);

  if (passMHCorrResolved) updateFlag(eventFlag, ZeroLeptonCuts::MHCorrResolved);
  if (passMHCorrMerged) updateFlag(eventFlag, ZeroLeptonCuts::MHCorrMerged);
  if (passMHResolvedSideBand)
    updateFlag(eventFlag, ZeroLeptonCuts::MHCorrResolvedSideBand);
  if (passMHMergedSideBand)
    updateFlag(eventFlag, ZeroLeptonCuts::MHCorrMergedSideBand);

  ///////////////////////////////////////////////////
  // DEFINITION OF CUTS FOR DIFFERENT REGIONS     ///
  ///////////////////////////////////////////////////

  // --- Cuts for resolved analysis --- //
  std::vector<unsigned long int> cuts_common_resolved = {
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
      ZeroLeptonCuts::DPhiMETDijetResolved};

  std::vector<unsigned long int> cuts_SM = {ZeroLeptonCuts::DRB1B2};
  if (m_model == Model::CUT) {
    cuts_common_resolved.insert(cuts_common_resolved.end(), cuts_SM.begin(),
                                cuts_SM.end());
  }
  std::vector<unsigned long int> cuts_SR_resolved = {
      ZeroLeptonCuts::MHCorrResolved};
  cuts_SR_resolved.insert(cuts_SR_resolved.end(), cuts_common_resolved.begin(),
                          cuts_common_resolved.end());
  std::vector<unsigned long int> cuts_SRcut_resolved = {ZeroLeptonCuts::DRB1B2};
  cuts_SRcut_resolved.insert(cuts_SRcut_resolved.end(),
                             cuts_common_resolved.begin(),
                             cuts_common_resolved.end());

  if (m_model == Model::MVA)
    passCutBased = passSpecificCuts(eventFlag, cuts_SRcut_resolved);

  // --- Cuts for jet histograms --- //
  std::vector<unsigned long int> cuts_jet_hists_resolved = {
      ZeroLeptonCuts::AllCxAOD,     ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::METResolved,  ZeroLeptonCuts::DPhiMETMPT,
      ZeroLeptonCuts::AtLeast2Jets, ZeroLeptonCuts::AtLeast2SigJets};

  // --- Cuts for merged analysis --- //
  std::vector<unsigned long int> cuts_common_merged = {
      ZeroLeptonCuts::AllCxAOD,
      ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::METMerged,
      ZeroLeptonCuts::AtLeast1FatJet,
      ZeroLeptonCuts::passVRJetOR,
      ZeroLeptonCuts::MinDPhiMETJetsMerged,
      ZeroLeptonCuts::DPhiMETDijetMerged,
      ZeroLeptonCuts::DPhiMETMPT,
      ZeroLeptonCuts::AtLeast2MatchedTrackJets};

  std::vector<unsigned long int> cuts_SR_merged = {ZeroLeptonCuts::mJ};
  // opportunity to add additional cuts for a SR
  cuts_SR_merged.insert(cuts_SR_merged.end(), cuts_common_merged.begin(),
                        cuts_common_merged.end());

  // looser selections for merged easy tree
  std::vector<unsigned long int> cuts_merged_loose = {
      ZeroLeptonCuts::AllCxAOD,    ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::METMerged,   ZeroLeptonCuts::AtLeast1FatJet,
      ZeroLeptonCuts::passVRJetOR, ZeroLeptonCuts::AtLeast2MatchedTrackJets};

  // select merged or resolved regime
  //--------------------------------------------------------
  selectRegime(eventFlag, cuts_SR_resolved, cuts_SR_merged,
               cuts_common_resolved, cuts_SR_merged);

  //--------------------------------------------------------
  // jet histograms before set region
  //--------------------------------------------------------

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved &&
      passSpecificCuts(eventFlag, cuts_jet_hists_resolved)) {
    if (!m_doOnlyInputs && !m_doReduceFillHistos) {
      if (!m_doMergePtVBins) m_histNameSvc->set_pTV(metVec.Pt());
      EL_CHECK("AnalysisReader_VHbb0Lep::fill_0Lep",
               fill_nJetHistos(signalJets, "Sig"));
      EL_CHECK("AnalysisReader_VHbb0Lep::fill_0Lep",
               fill_nJetHistos(forwardJets, "Fwd"));
      m_histSvc->BookFillHist("njets", 25, 0, 25, m_physicsMeta.nJets,
                              m_weight);
    }
  }

  // Set event flavour, histogram naming
  //------------------------------
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    setevent_flavour(selectedJets);
    m_histNameSvc->set_nTag(m_physicsMeta.nTags);
    m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
    if (!m_doMergePtVBins) m_histNameSvc->set_pTV(metVec.Pt());

    if (m_doMergeJetBins && m_physicsMeta.nJets >= 2)
      m_histNameSvc->set_nJet(-2);
    else
      m_histNameSvc->set_nJet(m_physicsMeta.nJets);
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    // Flavor labeling option: TrackJetCone
    if (!doCoMtagging)
      setevent_flavour(selectedtrackJetsInFatJet);
    else
      setevent_flavour(selectedCoMsubjets);

    if (m_doMergeModel)
      m_histNameSvc->set_nTrackJetInFatJet(trackJetsInFatJet1.size());
    m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);
    m_histNameSvc->set_nFatJet(m_physicsMeta.nFatJet);
    if (!m_doMergePtVBins) m_histNameSvc->set_pTV(metVec.Pt());
    m_histNameSvc->set_nBTagTrackJetUnmatched(m_physicsMeta.nAddBTrkJets);
    m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
    if (m_SplitBoostedInputsAddJets)
      m_histNameSvc->set_nJet(nAdditionalCaloJets);
  }

  // **RE-Definition** : Higgs, ZH - according to regime for hist filling
  TLorentzVector HVec, bbjVec;
  TLorentzVector VHVec;

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    HVec = resolvedH.vec_corr;
    if (m_physicsMeta.nJets > 2) bbjVec = resolvedH.vec_corr + j3_sel.vec_corr;
    VHVec = resolvedVH.vec_corr;
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    HVec = mergedH.vec_corr;
    VHVec = mergedVH.vec_corr;
  }

  // **Calculate** : b-tagging SF
  float btagWeight = 1.;
  if (m_isMC) {
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
      btagWeight =
          computeBTagSFWeight(signalJets, m_jetReader->getContainerName());
    } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
      //      if (m_doTruthTagging) //
      //	btagWeight = computeBTagSFWeight(
      //					 trackJetsInFatJet1,
      // m_trackJetReader->getContainerName(),
      // trackJetsNotInFatJet1);
      //      else
      btagWeight = computeBTagSFWeight(
          trackJetsForBTagSF,
          m_trackJetReader->getContainerName());  //  note: this should
                                                  // now work also for TT
      // To be added once CoM is fully calibrated
      // if (doCoMtagging)
      // btagWeight *= computeBTagSFWeight(selectedCoMsubjets,
      //                                m_subJetReader->getContainerName());
    }
    m_weight *= btagWeight;
    if (m_doTruthTagging)
      BTagProps::truthTagEventWeight.set(m_eventInfo, btagWeight);
  }

  //------------------------------------------------
  // store w/ / w/o PU weight as systematic variation
  // depends on what setting was chosen in config
  //------------------------------------------------

  if (!m_config->get<bool>("applyPUWeight"))
    m_weightSysts.push_back({"_withPU", (float)(m_pileupReweight)});

  /////////////////////////////////////////////////////////////
  // CorrsAndSyst

  if ((m_analysisStrategy == "Resolved") && m_isMC &&
      (m_model == Model::CUT || m_model == Model::MVA) &&
      ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    float cs_njet = (m_physicsMeta.nSigJet <= 3 ? m_physicsMeta.nSigJet : 3);
    EL::StatusCode statusApplyCS =
        applyCS(metVec.Pt(), metVec.Pt(), cs_njet, m_physicsMeta.nTags,
                j1_sel.vec, j2_sel.vec);
    if (statusApplyCS == EL::StatusCode::FAILURE)
      return EL::StatusCode::SUCCESS;
  } else if (m_isMC &&
             (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) &&
             (selectedtrackJetsInFatJet.size() > 1) &&
             passSpecificCuts(eventFlag, cuts_common_merged) &&
             ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    EL_CHECK("applyCS()", applyCS(metVec.Pt(), HVec.M(), nAdditionalCaloJets));
  }

  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);

  bool isNominal = false;
  if (m_currentVar == "Nominal") isNominal = true;
  //-----------------------------
  // VJets Systematic Histograms
  //-----------------------------
  // NOTE :: mBB is in MeV because CorrsAndSysts uses MeV
  if (!nominalOnly &&
      (m_histNameSvc->get_sample() == "W" ||
       m_histNameSvc->get_sample() == "Wv22" ||
       m_histNameSvc->get_sample() == "Z" ||
       m_histNameSvc->get_sample() == "Zv22") &&
      isNominal && (m_model == Model::HVT || m_model == Model::AZh) &&
      (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ||
       m_physicsMeta.regime == PhysicsMetadata::Regime::merged)) {
    double mBB = HVec.M();
    float mVH = m_physicsMeta.regime == PhysicsMetadata::Regime::resolved
                    ? VHVec.M()
                    : mergedVH.vec_corr.M();
    std::string Regime =
        m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ? "Resolved"
                                                                  : "Boosted";

    fillVjetsSystslep_PRSR(Regime, mBB, mVH / 1e3);  // mVH= GeV, mBB = MeV
  }
  //-----------------------------------------------
  // Internal weight variations Histograms
  //-----------------------------------------------
  if (m_evtWeightVarMode != -1 && m_currentVar == "Nominal" && m_isMC) {
    EL_CHECK("applyEvtWeights()", apply_EvtWeights());
  }

  //-----------------------------
  // TTbar Systematic Histograms
  //-----------------------------
  if (m_histNameSvc->get_sample() == "ttbar" &&
      (m_model == Model::HVT || m_model == Model::AZh) &&
      (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ||
       m_physicsMeta.regime == PhysicsMetadata::Regime::merged)) {
    if (!nominalOnly && isNominal) {
      double mBB = HVec.M();
      float mVH = m_physicsMeta.regime == PhysicsMetadata::Regime::resolved
                      ? VHVec.M()
                      : mergedVH.vec_corr.M();
      std::string Regime =
          m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ? "Resolved"
                                                                    : "Boosted";
      float nAddBTags =
          m_physicsMeta.regime == PhysicsMetadata::Regime::resolved
              ? 0
              : m_physicsMeta.nAddBTrkJets;

      fillTopSystslep_PRSR(Regime, nAddBTags, mBB, mVH / 1e3);
    }
  }

  //*********************************************************//
  //              ttbar  BDT Based Systematics               //
  //                                                         //
  //*********************************************************//
  if ((m_histNameSvc->get_sample() == "ttbar" ||
       m_histNameSvc->get_sample() == "W") &&
      m_currentVar == "Nominal" && !nominalOnly) {
    m_BDTSyst->ResetInputs();

    DefineVarBDTReweight_0lep(j1_sel.vec_corr, j2_sel.vec_corr, j3_sel.vec_corr,
                              selectedJets, signalJets, forwardJets, metVec,
                              metVec, HVec);

    int channel = 0;
    if (m_analysisType == "1lep") channel = 1;
    if (m_analysisType == "2lep") channel = 2;

    m_BDTSyst_scores = AnalysisReader_VHQQ::ComputeBDTSystematics(
        channel, btagWeight, m_physicsMeta.nJets, tagcatExcl);
  }

  ////////////////////////////////////////////////////////////
  ///        ** Check for sample extensions **             ///
  ////////////////////////////////////////////////////////////

  // reweight for extension 0Lep TTbar
  EL_CHECK("AnalysisReader::applyExtension0LepTTbarWeight()",
           applyExtension0LepTTbarWeight());

  // reweight for extension 0Lep Znunu
  EL_CHECK("AnalysisReader::applyExtension0LepZnunuWeight()",
           applyExtension0LepZnunuWeight());

  ////////////////////////////////////////////////////////////
  ///           ** Tree ang histogram filling **           ///
  ////////////////////////////////////////////////////////////

  // fill cut flow
  if (m_doCutflow) fill0LepCutFlow(eventFlag);

  ////////////////////////////////
  //      ** Resolved **        //
  ////////////////////////////////

  bool writeEasyTree = false;
  m_config->getif<bool>("writeEasyTree", writeEasyTree);
  bool writeMVATree = false;
  m_config->getif<bool>("writeMVATree", writeMVATree);

  bool callFillEasyTree =
      false;  // set to true if the selectinos to write the easytree for either
              // the resolved or the merged analysis are satisfied AND
              // writeEasyTree is set to true in the config file
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved &&
      (passSpecificCuts(eventFlag, cuts_common_resolved))) {
    // fill MVA tree
    m_tree->Reset();
    m_tree->SetVariation(m_currentVar);
    fillMVATreeVHbbResolved0Lep(
        j1_sel.vec_corr, j2_sel.vec_corr, j3_sel.vec_corr, selectedJets,
        signalJets, forwardJets, HVec, metVec, met, taus.size(), btagWeight);

    // New MVA application - evaluate and store MVA scores
    for (std::map<std::string, MVAApplication_TMVA *>::iterator mva_iter =
             m_mvaVHbbApps.begin();
         mva_iter != m_mvaVHbbApps.end(); mva_iter++) {
      if (mva_iter->first == "mva") {
        m_tree->BDT = mva_iter->second->Evaluate(
            m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
      }
      if (mva_iter->first == "mvadiboson") {
        m_tree->BDT_VZ = mva_iter->second->Evaluate(
            m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
      }
    }

    // New SR and CR for Resolved
    if (m_doNewRegions) {
      if (m_physicsMeta.nJets == 2) {
        float dRBB_min = m_fit_func_2j10->Eval(metVec.Pt() * 1e-03);
        float dRBB_max = m_fit_func_2j95->Eval(metVec.Pt() * 1e-03);

        if (j1_sel.vec.DeltaR(j2_sel.vec) > dRBB_max)
          m_histNameSvc->set_description("CRHigh");
        else if (j1_sel.vec.DeltaR(j2_sel.vec) < dRBB_min)
          m_histNameSvc->set_description("CRLow");
        else
          m_histNameSvc->set_description("SR");
      }

      else if (m_physicsMeta.nJets >= 3) {
        float dRBB_min = m_fit_func_3j10->Eval(metVec.Pt() * 1e-03);
        float dRBB_max = m_fit_func_3j85->Eval(metVec.Pt() * 1e-03);

        if (j1_sel.vec.DeltaR(j2_sel.vec) > dRBB_max)
          m_histNameSvc->set_description("CRHigh");
        else if (j1_sel.vec.DeltaR(j2_sel.vec) < dRBB_min)
          m_histNameSvc->set_description("CRLow");
        else
          m_histNameSvc->set_description("SR");
      }
    }

    //--------------------------------------------------------
    // Fill histograms for the resolved analysis regime
    //--------------------------------------------------------

    // exclusive ntags
    m_histNameSvc->set_nTag(m_physicsMeta.nTags);  // b-tag
    if (m_physicsMeta.nTags == 2 && m_physicsMeta.nJets < 4) {
      bool GenerateSTXS = false;
      m_config->getif<bool>("GenerateSTXS", GenerateSTXS);
      fill0LepHistResolved();
      if (GenerateSTXS) {
        if (STXS_GetBinFromEvtInfo() !=
            -1) {             // check if it is the STXS signals
          STXS_FillYields();  // a 2-D histogram filled
          std::string temp_name = m_histNameSvc->get_sample();
          STXS_ReplaceName();  // replace the signal name with the STXS bin
          fill0LepHistResolved();
          m_histNameSvc->set_sample(temp_name);  // set the histogram name back
        }
      }
      // fill0LepHistWithModulableCuts(eventFlag);
    } else {  // other than 2tag2jet and 2tag3jet
      if (!(m_doOnlyInputs || m_doReduceFillHistos)) {
        fill0LepHistResolved();
      }
    }
    // inclusive ntags
    if (!(m_doOnlyInputs || m_doReduceFillHistos)) {
      m_histNameSvc->set_nTag(-1);  // b-tag
      fill0LepHistResolved();
    }
    if (passCutBased && (m_doOnlyInputs || m_doReduceFillHistos) &&
        !m_doNewRegions) {
      m_histNameSvc->set_analysisType(
          HistNameSvc::AnalysisType::CUT);  // Have to switch to cut based if
                                            // MVA run analysis to get right
                                            // pTV splitting passCutBased only
                                            // if Model::MVA
      m_histSvc->BookFillHist(
          "mBBCutBased", 100, 0, 500, m_tree->mBB,
          m_weight);  // for post-fit plots mBB (w/ mu-in-jet and ptreco corr)
      m_histNameSvc->set_analysisType(
          HistNameSvc::AnalysisType::MVA);  // And now switch back to MVA
    }

    // Fill Mbb inclusive plots for SM signal slice
    /*
    float SMSigVPTSlice = getSMSigVPTSlice();
    string Slice;
    if ( SMSigVPTSlice > 0. && !(m_doOnlyInputs || m_doReduceFillHistos)){
      if ( SMSigVPTSlice < 150000. ) Slice = "_0_150truthptv";
      else if ( SMSigVPTSlice < 250000. ) Slice = "_150_250truthptv";
      else  Slice = "_250truthptv";
      m_histNameSvc -> set_description(string("SR" + Slice));
      // exclusive ntags
      m_histNameSvc -> set_nTag(m_physicsMeta.nTags); // b-tag
      fill0LepHistResolved();
      // inclusive ntags
      m_histNameSvc -> set_nTag(-1); // b-tag
      fill0LepHistResolved();
    } // SMSigVPTSlice > 0. && !m_doOnlyInputs
    */

    //--------------------------------------------------------
    // Fill trees
    //--------------------------------------------------------
    if (m_histNameSvc->get_isNominal() && writeMVATree) {
      m_tree->Fill();
    }
    if (writeEasyTree and (m_physicsMeta.nJets < 4)) {
      callFillEasyTree = true;
    }
  }  // Resolved

  ////////////////////////////////
  //      ** SM Merged **       //
  ////////////////////////////////

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged &&
      (passSpecificCuts(eventFlag, cuts_common_merged) ||
       (m_histNameSvc->get_isNominal() &&
        passSpecificCuts(eventFlag, cuts_merged_loose)))) {
    // fill MVA tree for the boosted analysis regime
    //-----------------------------------------------
    if (passSpecificCuts(eventFlag, cuts_SR_merged))
      m_histNameSvc->set_description("SR");
    else
      m_histNameSvc->set_description(
          "L");  // name L (Loose) the region when SR criteria not satisfied

    fillMVATreeBoosted(signalJets, forwardJets, mptVec, metVec,
                       m_physicsMeta.nFatJet, trackJetsInFatJet1.size(), HVec,
                       dPhiMETMPT, mindPhiMerg, mindPhiMerg_3LeadJets, mergedVH,
                       VHVec);
    fillMVATreeBoosted_C2(fatJets);

    if (passSpecificCuts(
            eventFlag,
            cuts_SR_merged)) {  // the if selection for merged is looser now so
                                // this is needed to fill the histograms only
                                // for cuts_SR_merged

      // STXS?
      //-----------------------------------------------
      if (m_GenerateSTXS) {
        // start inputs filling for each STXS signal truth bin
        if (STXS_GetBinFromEvtInfo() != -1) {
          STXS_FillYields_Boosted();
          std::string temp_name = m_histNameSvc->get_sample();
          STXS_ReplaceName();  // replace the signal name with the STXS

          m_histSvc->BookFillHist("mJ", 100, 0, 500, m_boostedtree->mJ / 1e3,
                                  m_weight);
          m_histNameSvc->set_sample(temp_name);  // set the histogram name back
        }
      } else {
        // fill histograms for the boosted analysis regime
        //-----------------------------------------------
        fill0LepHistMerged();
        if (m_isStoreMJwC2) fill0LepHistMerged_C2();

        // fill additional sets of histograms for crosschecks
        //-----------------------------------------------
        if (!m_doOnlyInputs || m_doReduceFillHistos) {
          // btag incl / addbtag incl
          m_histNameSvc->set_nTag(-1);  // b-tag
          m_histNameSvc->set_nBTagTrackJetUnmatched(
              -1);  // external track jet b-tag
          fill0LepHistMerged();

          // btag incl / addbtag excl
          m_histNameSvc->set_nTag(-1);  // b-tag
          m_histNameSvc->set_nBTagTrackJetUnmatched(
              m_physicsMeta.nAddBTrkJets);  // external track jet b-tag
          fill0LepHistMerged();

          // btag excl / addbtag incl
          m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);  // b-tag
          m_histNameSvc->set_nBTagTrackJetUnmatched(
              -1);  // external track jet b-tag
          fill0LepHistMerged();
        }
      }  // !doOnlyInputs
    }
    m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);
    m_histNameSvc->set_nBTagTrackJetUnmatched(m_physicsMeta.nAddBTrkJets);
    if (passSpecificCuts(eventFlag, cuts_common_merged) &&
        ((fj1.vec.M() / 1e3) >= 30) && !m_doOnlyInputs) {
      m_histNameSvc->set_description("SR");
      m_histSvc->BookFillHist("mJIncl", 100, 0, 500, HVec.M() / 1e3, m_weight);
    }

    // Fill tree
    if (writeEasyTree) {
      callFillEasyTree = true;
    }
  }  // Merged

  if (callFillEasyTree) {
    fillETreeCommon();  // IMPORTANT: this needs to be called to have
                        // the easy tree reset
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved)
      fillETreeResolved(HVec, bbjVec, metVec, selectedJets, j1_sel, j2_sel,
                        fatJets, trackJets);
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
      fillETreeWeights(
          triggerSF, JVTweight,
          btagWeight);  // this could be called also for resolved if the
                        // individual weight branches are needed
      fillETreeMerged(mptVec, metVec, selectedtrackJetsInFatJet);
    }
    m_etree->Fill();
  }

  //*********************************************************//
  //                       OS TREE                          //
  //*********************************************************//
  bool writeOSTree = false;
  m_config->getif<bool>("writeOSTree", writeOSTree);
  if (writeOSTree) {
    if (passSpecificCuts(eventFlag, cuts_common_resolved)) {
      if (m_physicsMeta.nJets == 2 || m_physicsMeta.nJets == 3) {
        m_OStree->Reset();
        fillOSTree();
        m_OStree->Fill();
      }
    }
  }

  return EL::StatusCode::SUCCESS;
}  // run_0Lep_analysis

void AnalysisReader_VHbb0Lep::DefineVarBDTReweight_0lep(
    TLorentzVector &j1, TLorentzVector &j2, TLorentzVector &j3,
    std::vector<const xAOD::Jet *> &selectedJets,
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets, TLorentzVector &metVec,
    TLorentzVector &VVec, TLorentzVector &HVec) {
  // MET
  m_BDTSyst->m_Variables["MET"] = metVec.Pt() * 1e-03;
  // dEtaBB
  m_BDTSyst->m_Variables["dEtaBB"] = fabs(VVec.Eta() - HVec.Eta());

  // Ht
  // Attention: this is before b-jet corrections
  float sumpt = 0;
  for (unsigned int s_i = 0; s_i < signalJets.size(); s_i++) {
    sumpt += signalJets.at(s_i)->pt();
  }
  for (unsigned int f_i = 0; f_i < forwardJets.size(); f_i++) {
    sumpt += forwardJets.at(f_i)->pt();
  }

  Float_t HT = sumpt + metVec.Pt();
  m_BDTSyst->m_Variables["HT"] = HT * 1e-03;

  m_BDTSyst->m_Variables["mBB"] = HVec.M() * 1e-03;

  m_BDTSyst->m_Variables["dRBB"] = j1.DeltaR(j2);

  m_BDTSyst->m_Variables["dPhiVBB"] = fabs(VVec.DeltaPhi(HVec));

  m_BDTSyst->m_Variables["pTB1"] = j1.Pt() * 1e-03;
  m_BDTSyst->m_Variables["pTB2"] = j2.Pt() * 1e-03;
  if (selectedJets.size() >= 3) {
    m_BDTSyst->m_Variables["mBBJ"] = (HVec + j3).M() * 1e-03;
    m_BDTSyst->m_Variables["pTJ3"] = j3.Pt() * 1e-03;
  }

}  // DefineVarBDTReweight_0lep
