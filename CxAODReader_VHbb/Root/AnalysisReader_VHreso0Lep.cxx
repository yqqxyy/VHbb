#include "CxAODReader_VHbb/AnalysisReader_VHreso0Lep.h"

#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb0lep.h"
#include "CxAODTools_VHbb/VHbb0lepEvtSelection.h"

#include "TLorentzVector.h"

// default constructor for all the event variables
// allows to easily reset all values at the beginning of every run_0Lep_analysis
// call
AnalysisReader_VHreso0Lep::EventVariables::EventVariables()
    : met(nullptr),
      signalJets(),
      forwardJets(),
      fatJets(),
      trackJets(),
      selectedJets(),
      taus(),
      matchedLeadTrackJets(),
      unmatchedTrackJets(),
      trackJetsForLabelling(),
      trackJetsForBTagging(),
      selectedEventJets(),
      selectedEventFatJets(),
      metSig(-1),
      metSig_PU(-1),
      metSig_soft(-1),
      metSig_hard(-1),
      metOverSqrtSumET(-1),
      metOverSqrtHT(-1),
      metVec(),
      mptVec(),
      resolvedH(),
      mergedH(),
      resolvedVH(),
      mergedVH(),
      HVec(),
      VHVec(),
      bbjVec(),
      sumpt(-1),
      dPhiMETMPT(1e6),
      dPhiMETdijetResolved(1e6),
      dPhiMETdijetMerged(1e6),
      dRb1b2(1e6),
      mindPhi(1e6),
      mindPhiMerged(1e6),
      mindPhiMerged_3LeadJets(1e6),
      eventFlag(0),
      cuts_common_resolved(),
      cuts_SR_resolved(),
      cuts_jet_hists_resolved(),
      cuts_common_merged(),
      cuts_SR_merged() {}

// constructor
AnalysisReader_VHreso0Lep::AnalysisReader_VHreso0Lep()
    : AnalysisReader_VHQQ0Lep(),
      // config values are not part of the struct EventVariables because they
      // are set in initializeEvent
      m_nominalOnly(false),
      m_alwaysFillEasyTree(false),
      m_applyTauVeto(false),
      m_doTrackJetOverlapVeto(false),
      m_doMETSignificanceCut(false),
      m_generateSTXSsignals(false),
      m_writeEasyTree(false),
      m_vars() {}

AnalysisReader_VHreso0Lep::~AnalysisReader_VHreso0Lep() {}

EL::StatusCode AnalysisReader_VHreso0Lep::run_0Lep_analysis() {
  // Run the zero lepton VH reso analysis. It was attempted to keep the code
  // modular for better readabilty. This method hence calls many subroutines.

  if (m_debug) {
    Info("run_0Lep_analysis()", "Region: %s, and nTags: %d, njet: %d ",
         m_histNameSvc->get_description().c_str(), m_physicsMeta.nTags,
         m_histNameSvc->get_nJet());
  }

  m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::VHres);

  // Reset all event variables
  m_vars = {};

  // Update member variables according to the current state
  EL_CHECK("run_0Lep_analysis()", UpdateSettings());

  // Select the objects
  EL_CHECK("run_0Lep_analysis()", SelectObjects());

  // Reset physics metadata
  m_physicsMeta = PhysicsMetadata();
  m_physicsMeta.channel = PhysicsMetadata::Channel::ZeroLep;
  setevent_nJets(m_vars.signalJets, m_vars.forwardJets);

  // Get selected jets and calualate number of tags
  EL_CHECK("run_0Lep_analysis()", DoJetsAndTagging());

  // Store additional jet information in special Jet struct
  EL_CHECK("run_0Lep_analysis()", setJetVariables());

  // Store additional fat jet information in special Jet struct
  EL_CHECK("run_0Lep_analysis()", setFatJetVariables());

  // Set the resolved and merged higgs candidates
  EL_CHECK("run_0Lep_analysis()", setHiggsCandidate());

  // Get all MET significance values
  EL_CHECK("run_0Lep_analysis()", SetMetAndMetSignificance());

  // Set MPT
  EL_CHECK("run_0Lep_analysis()", SetMpt());

  // Set resolved and merged VH candidates
  EL_CHECK("run_0Lep_analysis()", setVHCandidate());

  // Set sumpt
  EL_CHECK("run_0Lep_analysis()", SetSumpt());

  // Determine event flag (do the cut flow)
  EL_CHECK("AnalsysiReader_VHreso0Lep::run_0Lep_analysis()", DoEventFlag());

  // Set the cuts that we want to use
  EL_CHECK("run_0Lep_analysis()", SetCuts());

  // Select the regime based on cuts
  selectRegime(m_vars.eventFlag, m_vars.cuts_SR_resolved, m_vars.cuts_SR_merged,
               m_vars.cuts_common_resolved, m_vars.cuts_common_merged);

  // Histograms for number jets before the region is set
  EL_CHECK("run_0Lep_analysis()", DoJetHistos());

  // Set event flavour based on selection
  EL_CHECK("run_0Lep_analysis()", SetEventFlavour());

  // Set the description (SR, mBBcr, etc.) in the histNameSvc
  EL_CHECK("run_0Lep_analysis()", SetHistNameSvcDescription());

  // Select the Lorentz vectors with which we calculate the kinematic variables
  EL_CHECK("run_0Lep_analysis()", SelectCandidateVectors());

  // Calculate and apply the b tagging weight from the correct jets
  EL_CHECK("run_0Lep_analysis()", ApplyBTaggingWeight());

  // If pileup weights are not applied immediately we automatically store then
  // as a weight systematic
  if (!m_applyPUWeight) {
    m_weightSysts.push_back({"_withPU", (float)(m_pileupReweight)});
  }

  // Do the necessary variations for V+jets
  EL_CHECK("run_0Lep_analysis()", DoVJetsVariations());

  // Do the necessary variations for ttbar
  EL_CHECK("run_0Lep_analysis()", DoTTbarVariations());

  // Fill Cut flow if requested
  EL_CHECK("run_0Lep_analysis()", fill0LepCutFlow());

  // We have to fill the MVA trees, because the histograms are filled with their
  // information

  // Elisabeth: comment out
  // -> you could use AnalysisReader_VHQQ0Lep::fillMVATreeVHbbResolved0Lep
  // -> if additional variables needed:
  //        -> use EasyTree or
  //        -> create MVATree_VHreso

  // EL_CHECK("run_0Lep_analysis()",fillMVATreeResolved());

  EL_CHECK("run_0Lep_analysis()", fillMVATreeBoosted());

  // Take care of resolved histograms
  EL_CHECK("run_0Lep_analysis()", DoResolvedOutput());

  // Take care of resolved histograms
  EL_CHECK("run_0Lep_analysis()", DoMergedOutput());

  // Take care of event tree(s)
  EL_CHECK("run_0Lep_analysis()", FillEasyTree());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::UpdateSettings() noexcept {
  // Update all member variables that depend on the current state of the class

  if (m_debug) {
    Info("UpdateSettings()", "Beginning of function UpdateSettings()");
  }

  if (m_analysisStrategy != "Resolved") {
    m_doTruthTagging = false;
  }

  if (m_debug) {
    Info("UpdateSettings()", "End of function UpdateSettings()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SelectObjects() {
  // Get all objects from the VHbb0lepEvtSelection object and store objects in
  // member variables

  if (m_debug) {
    Info("SelectObjects()", "Beginning of function SelectObjects()");
  }

  ResultVHbb0lep selectionResult =
      ((VHbb0lepEvtSelection *)m_eventSelection)->result();

  m_vars.met = selectionResult.met;
  m_vars.signalJets = selectionResult.signalJets;
  m_vars.forwardJets = selectionResult.forwardJets;
  m_vars.fatJets = selectionResult.fatJets;
  m_vars.trackJets = selectionResult.trackJets;
  m_vars.taus = selectionResult.taus;

  if (m_debug) {
    Info("SelectObjects()", "End of function SelectObjects()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::DoJetsAndTagging() {
  // Evaluate the selected jets and based on them do b tagging

  if (m_debug) {
    Info("DoJetsAndTagging()", "Beginning of function DoJetsAndTagging()");
  }

  // b-tagging for small-R jets
  if (m_doTruthTagging) {
    compute_TRF_tagging(m_vars.signalJets);
  } else {
    compute_btagging();
  }

  // number of tags in selected jets
  int tagcatExcl = -1;
  tagjet_selection(m_vars.signalJets, m_vars.forwardJets, m_vars.selectedJets,
                   tagcatExcl);
  m_physicsMeta.nTags = tagcatExcl;

  // Vectors to hold the trackjets in the leading fat-jet, outside of the
  // leading fat jet and the ones used to compute the b-tagging weight
  //..for event labelling used in HVT/AZh analysis
  compute_fatjetTags(m_vars.trackJets, m_vars.fatJets,
                     &m_vars.matchedLeadTrackJets, &m_vars.unmatchedTrackJets);

  // get number of b-tagged track jets outside of the leading fat jet
  //# b-tagged track jets
  int nBTagTrackJet = 0;

  for (auto jet : m_vars.trackJets) {
    if (BTagProps::isTagged.get(jet)) nBTagTrackJet++;
  }
  //# b-tagged track jets matched to leading fat jet
  if (m_vars.fatJets.size() > 0) {
    m_physicsMeta.nTagsInFJ = Props::nBTags.get(m_vars.fatJets[0]);
    m_physicsMeta.nAddBTrkJets = Props::nAddBTags.get(m_vars.fatJets[0]);
  }
  // used to compute the b-tag weight
  m_vars.trackJetsForBTagging = m_vars.matchedLeadTrackJets;
  m_vars.trackJetsForBTagging.insert(m_vars.trackJetsForBTagging.end(),
                                     m_vars.unmatchedTrackJets.begin(),
                                     m_vars.unmatchedTrackJets.end());

  std::vector<const xAOD::Jet *> bTaggedUnmatchedTrackJets;
  // find track jets used for flavour labelling the event: one (two) leading
  // ones inside the fat jet, leading (b-tagged) one outside of the fat jet
  // first: find b-tagged unmatched track jets
  for (auto jet : m_vars.unmatchedTrackJets) {
    if (BTagProps::isTagged.get(jet)) bTaggedUnmatchedTrackJets.push_back(jet);
  }
  // second: check consistency with what was derived before
  if ((unsigned int)m_physicsMeta.nAddBTrkJets !=
      bTaggedUnmatchedTrackJets.size()) {
    Error("AnalysisReader_VHreso0Lep::DoJetsAndTaggin()",
          "m_physicsMeta.nAddBTrkJets != bTaggedUnmatchedTrackJets.size()");
    return EL::StatusCode::FAILURE;
  }
  // third: select jets for event flavour labelling
  m_vars.trackJetsForLabelling = m_vars.matchedLeadTrackJets;
  if (m_physicsMeta.nAddBTrkJets) {
    m_vars.trackJetsForLabelling.insert(m_vars.trackJetsForLabelling.end(),
                                        bTaggedUnmatchedTrackJets.begin(),
                                        bTaggedUnmatchedTrackJets.end());
  } else {
    m_vars.trackJetsForLabelling.insert(m_vars.trackJetsForLabelling.end(),
                                        m_vars.unmatchedTrackJets.begin(),
                                        m_vars.unmatchedTrackJets.end());
  }

  if (m_debug) {
    Info("DoJetsAndTagging()", "End of function DoJetsAndTagging()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::setJetVariables() {
  // Overloads the AnalysisReader_VHQQ implementation that takes additional jet
  // objects as argument
  // Here, we retrieve information from / save information to member variables
  // TODO: Unitilialized information? Isn't this a problem?

  if (m_debug) {
    Info("setJetVariables()", "Beginning of function setJetVariables()");
  }

  Jet j1, j2, j3;

  EL_CHECK(
      "AnalysisReader_VHbb0Lep::setJetVariables()",
      AnalysisReader_VHQQ::setJetVariables(j1, j2, j3, m_vars.selectedJets));

  if (m_vars.selectedJets.size() >= 1) {
    m_vars.selectedEventJets.push_back(j1);
  }
  if (m_vars.selectedJets.size() >= 2) {
    m_vars.selectedEventJets.push_back(j2);
  }
  if (m_vars.selectedJets.size() >= 3) {
    m_vars.selectedEventJets.push_back(j3);
  }

  if (m_debug) {
    Info("setJetVariables()", "End of function setJetVariables()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::setFatJetVariables() {
  // Overloads the AnalysisReader_VHQQ implementation that takes additional jet
  // objects as argument
  // Here, we retrieve information from / save information to member variables
  // TODO: Unitilialized information? Isn't this a problem?

  if (m_debug) {
    Info("setFatJetVariables()", "Beginning of function setFatJetVariables()");
  }

  Jet j1, j2, j3;

  EL_CHECK("AnalysisReader_VHbb0Lep::setFatJetVariables()",
           AnalysisReader_VHQQ::setFatJetVariables(j1, j2, j3, m_vars.fatJets));

  if (m_vars.fatJets.size() >= 1) {
    m_vars.selectedEventFatJets.push_back(j1);
  }
  if (m_vars.fatJets.size() >= 2) {
    m_vars.selectedEventFatJets.push_back(j2);
  }
  if (m_vars.fatJets.size() >= 3) {
    m_vars.selectedEventFatJets.push_back(j3);
  }

  if (m_debug) {
    Info("setFatJetVariables()", "End of function setFatJetVariables()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::setHiggsCandidate() {
  // Overloads the AnalysisReaer_VHQQ implementation that does not store
  // information in member variables
  // Use event jet information where available and unitialized information else
  // to calculate the Higgs candidates from the small jets and the leading large
  // jet
  // TODO: Unitilialized information? Isn't this a problem?

  if (m_debug) {
    Info("setHiggsCandidate()", "Beginning of function setHiggsCandidate()");
  }

  Jet j1;
  Jet j2;
  Jet fj;

  if (m_vars.selectedEventJets.size() >= 1) {
    j1 = m_vars.selectedEventJets[0];
  }
  if (m_vars.selectedEventJets.size() >= 2) {
    j2 = m_vars.selectedEventJets[1];
  }

  if (m_vars.selectedEventFatJets.size() >= 1) {
    fj = m_vars.selectedEventFatJets[0];
  }

  AnalysisReader_VHQQ::setHiggsCandidate(m_vars.resolvedH, m_vars.mergedH, j1,
                                         j2, fj);

  if (m_debug) {
    Info("setHiggsCandidate()", "End of function setHiggsCandidate()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SetMetAndMetSignificance() {
  // Get MET vector from MET object. Then get signifiances from
  // met container if specified in config
  // TODO: Can't we simply use the xAOD::MissingET *met that is specified in
  //       AnalysisReader?

  if (m_debug) {
    Info("SetMetAndMetSignificance()",
         "Beginning of function SetMetAndMetSignificance()");
  }

  m_vars.metVec.SetPxPyPzE(m_vars.met->mpx(), m_vars.met->mpy(), 0,
                           m_vars.met->met());

  if (!m_vars.met) {
    Error("AnalysisReader_VHreso0Lep::SetMetAndMetSignificance()",
          "No MET container set.");
    return EL::StatusCode::FAILURE;
  }

  m_vars.metSig = Props::metSig.get(m_vars.met);
  m_vars.metOverSqrtSumET = Props::metOverSqrtSumET.get(m_vars.met);
  m_vars.metOverSqrtHT = Props::metOverSqrtHT.get(m_vars.met);
  if (!Props::metSig_PU.exists(m_vars.met)) {
    if (m_debug) {
      Info("SetMetAndMetSignificance()", "Props::metSig_PU doesn't exist");
    }
    m_vars.metSig_soft = Props::metSig_soft.get(m_vars.met);
    m_vars.metSig_hard = Props::metSig_hard.get(m_vars.met);
  } else {
    m_vars.metSig_PU = Props::metSig_PU.get(m_vars.met);
  }

  if (m_debug) {
    Info("SetMetAndMetSignificance()",
         "End of function SetMetAndMetSignificance()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SetMpt() {
  // Uses the xAOD::MissingET *m_mpt that is defined in AnalysisReader

  if (m_debug) {
    Info("SetMpt()", "Beginning of function SetMpt()");
  }

  m_vars.mptVec.SetPxPyPzE(m_mpt->mpx(), m_mpt->mpy(), 0, m_mpt->met());

  if (m_debug) {
    Info("SetMpt()", "End of function SetMpt()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::setVHCandidate() {
  // Overloads the AnalysisReaer_VHQQ implementation that does not store
  // information in member variables
  // Use event jet information where available and unitialized information else
  // to calculate the Higgs candidates from the small jets and the leading large
  // jet
  // TODO: Unitilialized information? Isn't this a problem?

  if (m_debug) {
    Info("setVHCandidate()", "Beginning of function setVHCandidate()");
  }

  Jet j1;
  Jet j2;
  Jet fj;

  if (m_vars.selectedEventJets.size() >= 2) {
    j1 = m_vars.selectedEventJets[0];
    j2 = m_vars.selectedEventJets[1];
  } else if (m_vars.selectedEventJets.size() == 1) {
    j1 = m_vars.selectedEventJets[0];
  }

  if (m_vars.selectedEventFatJets.size() >= 1) {
    fj = m_vars.selectedEventFatJets[0];
  }

  AnalysisReader_VHQQ0Lep::setVHCandidate(m_vars.resolvedVH, m_vars.mergedVH,
                                          m_vars.metVec, j1, j2, fj);

  if (m_debug) {
    Info("setVHCandidate()", "End of function setVHCandidate()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SetSumpt() {
  // Calculate jet pt sum
  // Use either 3 signal jets or 2 signal jets and 1 forward jet

  if (m_debug) {
    Info("SetSumpt()", "Beginning of function SetSumpt()");
  }

  if (m_physicsMeta.nSigJet >= 3) {
    m_vars.sumpt = m_vars.signalJets[0]->pt() + m_vars.signalJets[1]->pt() +
                   m_vars.signalJets[2]->pt();
  } else if ((m_physicsMeta.nSigJet == 2) && (m_physicsMeta.nForwardJet >= 1)) {
    m_vars.sumpt = m_vars.signalJets[0]->pt() + m_vars.signalJets[1]->pt() +
                   m_vars.forwardJets[0]->pt();
  } else if (m_physicsMeta.nSigJet == 2) {
    m_vars.sumpt = m_vars.signalJets[0]->pt() + m_vars.signalJets[1]->pt();
  } else {
    if (m_debug) {
      Info("SetSumpt()", "Don't have enough jets to calculate sumpt");
      Info("SetSumpt()",
           "m_vars.signalJets.size() = %lu, m_vars.forwardJets.size() = %lu, "
           "m_physicsMeta.nSigJet = %d, m_physicsMeta.nForwardJet = %d",
           m_vars.signalJets.size(), m_vars.forwardJets.size(),
           m_physicsMeta.nSigJet, m_physicsMeta.nForwardJet);
    }
    // Make sure that nothing went wrong with the physicsMeta
    if ((int(m_vars.signalJets.size()) != m_physicsMeta.nSigJet) ||
        (int(m_vars.forwardJets.size()) != m_physicsMeta.nForwardJet)) {
      Error("SetSumpt()",
            "The jet vectors and the jet information in the metadata are "
            "mismatched.");
      Error("SetSumppt()",
            "m_vars.signalJets.size() = %lu, m_vars.forwardJets.size() = %lu, "
            "m_physicsMeta.nSigJet = %d, m_physicsMeta.nForwardJet = %d",
            m_vars.signalJets.size(), m_vars.forwardJets.size(),
            m_physicsMeta.nSigJet, m_physicsMeta.nForwardJet);
      return EL::StatusCode::FAILURE;
    }
  }

  m_triggerTool->setSumpt(m_vars.sumpt);

  if (m_debug) {
    Info("SetSumpt()", "End of function SetSumpt()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::DoEventFlag() {
  // Determine which cuts the event passes
  // Differences between different analyses and resolved / merged are taken into
  // account
  // The used updateFlag method should be called only once per cut, since by
  // using a logical OR once a bit is set to 1 it can not be reset to 0
  //
  // The CutFlow consists of
  //   1. All CxAOD
  //   2. Trigger (-> trigger scalefactor)
  //   3. Tau Veto (optional, set on/off in config)
  //   4. MET
  //   5. dPhi(MET, MPT)
  //   6. number of jets
  //   7. number of signal jets
  //   8. number of fat jets (at least 1)
  //   9. number of track jets
  //  10. track jet overlap removal
  //  11. min[dPhi(MET, jet)]
  //  12. sum jet pT
  //  13. MPT
  //  14. Third b jet veto  -- TODO: This cut is not yet implemented
  //  15. leading small R jet pT
  //  16. dPhi(b1, b2)
  //  17. dPhi(MET, dijet)
  //  18. dR(b1, b2)
  //  19. MET greater / smaller than 500 GeV
  //  20. MET significance
  //  21. mBB signal window / side bands

  if (m_debug) {
    Info("DoEventFlag()", "Beginning of function DoEventFlag()");
  }

  // 1. All events (CxAOD)
  //------------------------
  // automatically passed by all events
  updateFlag(m_vars.eventFlag, ZeroLeptonCuts::AllCxAOD, true);

  // 2. trigger
  //----------------------------
  double triggerSF = 1.;
  if (pass0LepTrigger(triggerSF,
                      ((VHbb0lepEvtSelection *)m_eventSelection)->result())) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::Trigger, true);
  }
  if (m_isMC) m_weight *= triggerSF;

  // 3. Tau veto
  //----------------------------
  if (!m_applyTauVeto || m_model == Model::AZh) {
    // do not apply the cut -> always pass it
    // (cut never applied in AZh analysis)
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::TauVeto, true);
  } else if (m_applyTauVeto && m_vars.taus.size() == 0) {
    // we do the cut but there are no taus -> pass it
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::TauVeto, true);
  }

  // 4. MET cut
  //-----------------------------------------
  if (m_vars.metVec.Pt() > 150e3) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::METResolved);
  }
  if (m_vars.metVec.Pt() > 200e3) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::METMerged);
  }

  // 5. dPhi(MET,MPT) cut
  //-----------------------------------------
  if (!TMath::IsNaN(m_vars.mptVec.Phi())) {
    // TODO: temporary fix, need to understand why mpt vec = nan for some
    //       systematics variations
    m_vars.dPhiMETMPT = fabs(m_vars.metVec.DeltaPhi(m_vars.mptVec));
  }
  if (m_vars.dPhiMETMPT < TMath::Pi() / 2.) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::DPhiMETMPT);
  }

  // 6. number of jets
  //----------------------------------------
  if (m_physicsMeta.nJets >= 2) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::AtLeast2Jets);
  }

  // 7. cut on nSignalJet
  //-----------------------------------------
  if (m_physicsMeta.nSigJet >= 2) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::AtLeast2SigJets);
  }
  if (m_isMC) {
    m_weight *= compute_JVTSF(m_vars.signalJets);
  }

  // 8. at least 1 fat jet
  //-----------------------------------------
  // TODO: put number of fat jets into m_physicsMeta
  if (m_vars.fatJets.size() >= 1) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::AtLeast1FatJet);
  }

  // 9. at least 1 fat jet with pT > 250 GeV
  //    (pT > 200 GeV & |eta| < 2 cuts applied on preselection level)
  //    matched track jet multiplicity
  //-----------------------------------------
  if (m_vars.fatJets.size() >= 1) {
    // TODO: Add the cut on pT  > 250
    if (m_vars.matchedLeadTrackJets.size() >= 1)
      updateFlag(m_vars.eventFlag, ZeroLeptonCuts::AtLeast1MatchedTrackJet);
    if (m_vars.matchedLeadTrackJets.size() >= 2)
      updateFlag(m_vars.eventFlag, ZeroLeptonCuts::AtLeast2MatchedTrackJets);
  }

  // 10. Track Jet Overlap Removal
  //-----------------------------------------
  // always pass if we don't care about the cut
  if (!m_doTrackJetOverlapVeto) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::NonOverlappingMatchedTrackJets,
               true);
  } else if (m_allLeadFJMatchedTrackJets.size() == 1) {
    // if there is only one track jet an overlap is impossible -> pass
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::NonOverlappingMatchedTrackJets,
               true);
  } else if (m_trackJetReader->getContainerName() !=
             "AntiKtVR30Rmax4Rmin02TrackJets") {
    // only done for the specific VRs: AntiKtVR30Rmax4Rmin02TrackJets
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::NonOverlappingMatchedTrackJets,
               true);
  } else if (m_allLeadFJMatchedTrackJets.size() > 1) {
    bool pass = true;
    for (auto tj1 : m_allLeadFJMatchedTrackJets) {
      // tj1 shall run over all track jets with pT > 10 GeV and nTrk > 1
      if (tj1->pt() < 10e3) {
        continue;
      }
      if (Props::TrkJetnumConstituents.get(tj1) <= 1) {
        continue;
      }
      for (auto tj2 : m_allLeadFJMatchedTrackJets) {
        if (tj1 == tj2) {
          continue;
        }
        // tj2 only considered if it has pT > 5 GeV and nTrk > 1
        if (tj2->pt() < 5e3) {
          continue;
        }
        if (Props::TrkJetnumConstituents.get(tj2) <= 1) {
          continue;
        }
        double jetR1 = max(0.02, min(0.4, 30. / (tj1->pt() / 1e3)));
        double jetR2 = max(0.02, min(0.4, 30. / (tj2->pt() / 1e3)));
        double deltaR = tj1->p4().DeltaR(tj2->p4());
        if (deltaR < min(jetR1, jetR2)) {
          pass = false;
          break;
        }
        /* Condition: DeltaR(jet_i,jet_j) < min(R_jet_i, R_jet_j where i runs on
        all the jets inspected by b-tagging and j runs on all the VR track-jets
        with pT>5 GeV and nTrk>1 with the obvious caveat to not consider the
        i==j case. To be noted that b-tagging on VR track-jets is recommended
        for VR track-jets with pT>10 GeV and nTrk>1 therefore the jets spanned
        by the index i is a subset of the jets spanned by the index j.

        Action if the condition is satisfied: Remove the event.

        This recommendation implies some care when slimming at the derivation
        level. The radius of each VR track jet can be calculated as
        max(0.02,min(0.4,30./(jet pT [GeV]))).
        */
      }
      if (!pass) {
        break;
      }
    }
    if (pass) {
      updateFlag(m_vars.eventFlag,
                 ZeroLeptonCuts::NonOverlappingMatchedTrackJets, true);
    }
  }

  // 11. min[dPhi(MET,jets)] cut
  //-----------------------------------------
  // Resolved: check first three leading jets
  float dPhi1 = 1000;
  float dPhi2 = 1000;
  float dPhi3 = 1000;

  if (m_physicsMeta.nSigJet >= 1) {
    dPhi1 = fabs(m_vars.signalJets[0]->p4().DeltaPhi(m_vars.metVec));
  }
  if (m_physicsMeta.nSigJet >= 2) {
    dPhi2 = fabs(m_vars.signalJets[1]->p4().DeltaPhi(m_vars.metVec));
  }
  if (m_physicsMeta.nSigJet >= 3) {
    dPhi3 = fabs(m_vars.signalJets[2]->p4().DeltaPhi(m_vars.metVec));
  } else if (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1) {
    dPhi3 = fabs(m_vars.forwardJets[0]->p4().DeltaPhi(m_vars.metVec));
  }
  m_vars.mindPhi = dPhi1;
  if (dPhi2 < m_vars.mindPhi) m_vars.mindPhi = dPhi2;
  if (dPhi3 < m_vars.mindPhi) m_vars.mindPhi = dPhi3;

  if (((m_physicsMeta.nSigJet + m_physicsMeta.nForwardJet) <= 3) &&
      (m_vars.mindPhi > 20 * TMath::Pi() / 180)) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MinDPhiMETJetsResolved, true);
  } else if (((m_physicsMeta.nSigJet + m_physicsMeta.nForwardJet) > 3) &&
             (m_vars.mindPhi > 30 * TMath::Pi() / 180)) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MinDPhiMETJetsResolved, true);
  }

  // Merged: check all jets
  for (auto signaljet : m_vars.signalJets) {
    float dPhi = fabs(signaljet->p4().DeltaPhi(m_vars.metVec));
    if (dPhi < m_vars.mindPhiMerged) {
      m_vars.mindPhiMerged = dPhi;
    }
  }
  for (auto forwardjet : m_vars.forwardJets) {
    float dPhi = fabs(forwardjet->p4().DeltaPhi(m_vars.metVec));
    if (dPhi < m_vars.mindPhiMerged) {
      m_vars.mindPhiMerged = dPhi;
    }
  }
  if (m_vars.mindPhiMerged > 20 * TMath::Pi() / 180) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MinDPhiMETJetsMerged, true);
  }

  // 12. sum[jet_pt] cut
  //-----------------------------------------
  if ((m_physicsMeta.nSigJet >= 3) ||
      (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1)) {
    if (m_vars.sumpt > 150e3)
      updateFlag(m_vars.eventFlag, ZeroLeptonCuts::SumJetPt);
  } else if (m_physicsMeta.nSigJet == 2) {
    if (m_vars.sumpt > 120e3)
      updateFlag(m_vars.eventFlag, ZeroLeptonCuts::SumJetPt);
  }

  // 13. MPT cut
  //----------------------
  if (m_physicsMeta.nTags < 2 && m_vars.mptVec.Pt() >= 30e3) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MinMPTBJet0or1Resolved, true);
  } else if (m_physicsMeta.nTags >= 2) {
    // 2tag always passes it because cut is only applied in 0 and 1tag
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MinMPTBJet0or1Resolved, true);
  }
  if (m_vars.mptVec.Pt() >= 30e3) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MinMPTMerged);
  }

  // 14. Third b-jet veto
  //----------------------
  // TODO: ZeroLeptonCuts::ThirdBJetVeto is not yet implemented
  if (m_physicsMeta.nTags < 3) {
    // less than 3 tags -> pass veto
    // updateFlag(m_eventFlag, ZeroLeptonCuts::ThirdBJetVeto, true);
  }

  // 15. pT cut on leading jet
  //----------------------
  if (m_vars.selectedJets.size() > 0 && m_vars.selectedJets[0]->pt() >= 45e3) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::LeadJetPt, true);
  }

  // 16. dPhi(b1,b2) cut
  //----------------------
  if (m_vars.selectedJets.size() >= 2) {
    float dPhiBB = 1000;
    dPhiBB = fabs(m_vars.selectedEventJets[0].vec.DeltaPhi(
        m_vars.selectedEventJets[1].vec));
    if (dPhiBB <= 140 * TMath::Pi() / 180)
      updateFlag(m_vars.eventFlag, ZeroLeptonCuts::DPhiBB, true);
  }

  // 17. dPhi(MET,dijet) cut
  //----------------------
  m_vars.dPhiMETdijetResolved =
      fabs(m_vars.metVec.DeltaPhi(m_vars.resolvedH.vec));
  if (m_vars.dPhiMETdijetResolved >= 120 * TMath::Pi() / 180) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::DPhiMETDijetResolved, true);
  }
  float dPhiMETdijetMerged = fabs(m_vars.metVec.DeltaPhi(m_vars.mergedH.vec));
  if (dPhiMETdijetMerged >= 120 * TMath::Pi() / 180) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::DPhiMETDijetMerged, true);
  }

  // 18. dR(b1,b2) cut (2015-10-18: added this cut)
  // commented for input production
  //----------------------
  // applied in SM measurement, cut based approach only
  if (m_vars.selectedJets.size() >= 2) {
    m_vars.dRb1b2 = fabs(m_vars.selectedEventJets[0].vec.DeltaR(
        m_vars.selectedEventJets[1].vec));
    float dRb1b2LowMET_SMCut = 1.8;
    float dRb1b2HighMET_SMCut = 1.2;
    if (m_vars.metVec.Pt() > 200e3 && m_vars.dRb1b2 <= dRb1b2HighMET_SMCut) {
      updateFlag(m_vars.eventFlag, ZeroLeptonCuts::DRB1B2, true);
    } else if (m_vars.metVec.Pt() <= 200e3 &&
               m_vars.dRb1b2 <= dRb1b2LowMET_SMCut) {
      updateFlag(m_vars.eventFlag, ZeroLeptonCuts::DRB1B2, true);
    }
  }

  // 19.  MET 500
  //------------------------
  if (m_vars.met->met() / 1.e3 < 500.) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::METLT500, true);
  } else {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::METGT500, true);
  }

  // 20. MET significance
  {
    // At this point we do not know m_physicsMeta.regime so we will have to
    // calculate the cut using both the resolved and merged VH candidates
    auto metsig_cut = [](float mvh) {
      // this function is the result of a pol2 fit evaluated in this study:
      // https://indico.cern.ch/event/770226/contributions/3218415/attachments/1753984/2843007/2018-11-15_metsig.pdf
      // other presentation on the METsignificance with first results:
      // https://indico.cern.ch/event/770225/contributions/3207914/attachments/1749738/2834515/2018-11-08_andi_metsig.pdf
      return max<float>(12.0, 10.1402 + 0.00428571e-3 * min<float>(mvh, 900e3) +
                                  6.94444e-12 * min<float>(mvh, 900e3) *
                                      min<float>(mvh, 900e3));
    };
    // resolved
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::METSignificanceResolved,
               m_vars.metSig > metsig_cut(m_vars.resolvedVH.vec_corr.M()));
    // merged
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::METSignificanceMerged,
               m_vars.metSig > metsig_cut(m_vars.mergedVH.vec_corr.M()));
    if (m_debug) {
      Info("DoEventFlag()", "metSig = %.2f", m_vars.metSig);
      Info("DoEventFlag()",
           "resolved: mvh = %.2f, cut_val = %.2f, passSpecificCut = %d",
           m_vars.resolvedVH.vec_corr.M(),
           metsig_cut(m_vars.mergedVH.vec_corr.M()),
           passSpecificCuts(m_vars.eventFlag,
                            {ZeroLeptonCuts::METSignificanceResolved}));
      Info("DoEventFlag()",
           "merged: mvh = %.2f, cut_val = %.2f, passSpecificCut = %d",
           m_vars.mergedVH.vec_corr.M(),
           metsig_cut(m_vars.mergedVH.vec_corr.M()),
           passSpecificCuts(m_vars.eventFlag,
                            {ZeroLeptonCuts::METSignificanceMerged}));
    }
  }

  // 21. mbb invariant mass window + mbb-sidebands
  //----------------------
  bool passMHCorrResolved = false;
  bool passMHCorrMerged = false;
  bool passMHResolvedSideBand = false;
  bool passMHMergedSideBand = false;

  checkMbbWindow(m_vars.resolvedH.vec_corr, m_vars.mergedH.vec_corr,
                 passMHCorrResolved, passMHCorrMerged, passMHResolvedSideBand,
                 passMHMergedSideBand);

  if (passMHCorrResolved) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MHCorrResolved, true);
  }
  if (passMHCorrMerged) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MHCorrMerged, true);
  }
  if (passMHResolvedSideBand) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MHCorrResolvedSideBand, true);
  }
  if (passMHMergedSideBand) {
    updateFlag(m_vars.eventFlag, ZeroLeptonCuts::MHCorrMergedSideBand, true);
  }

  if (m_debug) {
    Info("DoEventFlag()", "End of function DoEventFlag()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SetCuts() {
  // A seprate method to initialize the cuts for the different classes of events
  // One could think about defining these vectors as static or global constants
  // but the compiler probably takes care of that anyway and like this it is at
  // least tidy

  if (m_debug) {
    Info("SetCuts()", "Beginning of function SetCuts()");
  }

  m_vars.cuts_common_resolved = {ZeroLeptonCuts::AllCxAOD,
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
                                 ZeroLeptonCuts::DPhiMETDijetResolved};
  if (m_doMETSignificanceCut) {
    m_vars.cuts_common_resolved.insert(m_vars.cuts_common_resolved.end(),
                                       ZeroLeptonCuts::METSignificanceResolved);
  }

  // resolved signal region - start from common cuts
  m_vars.cuts_SR_resolved = m_vars.cuts_common_resolved;
  // add additional cuts
  std::vector<unsigned long int> additional_SR_resolved_cuts = {
      ZeroLeptonCuts::MHCorrResolved};
  m_vars.cuts_SR_resolved.insert(m_vars.cuts_SR_resolved.end(),
                                 additional_SR_resolved_cuts.begin(),
                                 additional_SR_resolved_cuts.end());

  // cuts for jet histograms
  m_vars.cuts_jet_hists_resolved = {
      ZeroLeptonCuts::AllCxAOD,       ZeroLeptonCuts::Trigger,
      ZeroLeptonCuts::TauVeto,        ZeroLeptonCuts::METResolved,
      ZeroLeptonCuts::DPhiMETMPT,     ZeroLeptonCuts::AtLeast2Jets,
      ZeroLeptonCuts::AtLeast2SigJets};

  m_vars.cuts_common_merged = {ZeroLeptonCuts::AllCxAOD,
                               ZeroLeptonCuts::Trigger,
                               ZeroLeptonCuts::TauVeto,
                               ZeroLeptonCuts::METMerged,
                               ZeroLeptonCuts::MinMPTMerged,
                               ZeroLeptonCuts::AtLeast1FatJet,
                               ZeroLeptonCuts::NonOverlappingMatchedTrackJets,
                               ZeroLeptonCuts::MinDPhiMETJetsMerged,
                               ZeroLeptonCuts::DPhiMETDijetMerged,
                               ZeroLeptonCuts::DPhiMETMPT};
  if (m_doMETSignificanceCut) {
    m_vars.cuts_common_merged.insert(m_vars.cuts_common_merged.end(),
                                     ZeroLeptonCuts::METSignificanceMerged);
  }

  // merged signal region - start from common cuts
  m_vars.cuts_SR_merged = m_vars.cuts_common_merged;
  // add additional cuts
  std::vector<unsigned long int> additional_SR_merged_cuts = {
      ZeroLeptonCuts::MHCorrMerged};
  m_vars.cuts_SR_merged.insert(m_vars.cuts_SR_merged.end(),
                               additional_SR_merged_cuts.begin(),
                               additional_SR_merged_cuts.end());

  if (m_debug) {
    Info("SetCuts()", "End of function SetCuts()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::DoJetHistos() {
  // Write histograms with number of signal and forward jets
  // This should be performed after the regime was determined, but before the
  // region is set

  if (m_debug) {
    Info("DoJetHistos()", "Beginning of function DoJetHistos()");
  }

  if (m_doOnlyInputs || m_doReduceFillHistos) {
    // requested that less histograms are written
    if (m_debug) {
      Info("DoJetHistos()", "m_doOnlyInputs || m_doReduceFillHistos -> return");
    }
    return EL::StatusCode::SUCCESS;
  }

  if (m_physicsMeta.regime != PhysicsMetadata::Regime::resolved) {
    // histograms are only written for resolved
    if (m_debug) {
      Info("DoJetHistos()", "not in resolved -> return");
    }
    return EL::StatusCode::SUCCESS;
  }

  if (!passSpecificCuts(m_vars.eventFlag, m_vars.cuts_jet_hists_resolved)) {
    // does not pass the required cuts for jet histograms
    if (m_debug) {
      Info("DoJetHistos()", "does not pass resolved cuts -> return");
    }
    return EL::StatusCode::SUCCESS;
  }

  if (!m_doMergePtVBins) {
    // histNameSvc needs information about MET to bin it
    m_histNameSvc->set_pTV(m_vars.metVec.Pt());
  }

  EL_CHECK("AnalysisReader_VHbb0Lep::fill_0Lep",
           fill_nJetHistos(m_vars.signalJets, "Sig"));
  EL_CHECK("AnalysisReader_VHbb0Lep::fill_0Lep",
           fill_nJetHistos(m_vars.forwardJets, "Fwd"));
  m_histSvc->BookFillHist("njets", 25, 0, 25, m_physicsMeta.nJets, m_weight);

  if (m_debug) {
    Info("DoJetHistos()", "End of function DoJetHistos()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SetEventFlavour() {
  // Determine the event flavour and configure the histogram naming service
  // accordingly
  // In resolved we consider the number of tags, jets, and MET
  // and this results in names like
  //   * 2tag2pjets_150ptv
  //   * 1tag3jet_0_150ptv
  //   * etc.
  // In merged we have more options to determine the event flavour but the
  // result is ultimately the same. Here we have regions like
  //   * 1tag1pfat_150ptv
  //   * 2tag1pfat_0_150ptv
  //   * etc.

  if (m_debug) {
    Info("SetEventFlavour()", "Beginning of function SetEventFlavour()");
  }

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    setevent_flavour(m_vars.selectedJets);
    m_histNameSvc->set_nTag(m_physicsMeta.nTags);
    m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
    if (!m_doMergePtVBins) {
      m_histNameSvc->set_pTV(m_vars.metVec.Pt());
    }

    if (m_doMergeJetBins && m_physicsMeta.nJets >= 2) {
      // -2 results in "2p" label
      m_histNameSvc->set_nJet(-2);
    } else {
      m_histNameSvc->set_nJet(m_physicsMeta.nJets);
    }
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    // Flavor labeling option: TrackJetCone
    setevent_flavour(m_vars.trackJetsForLabelling);

    if (m_doMergeModel) {
      m_histNameSvc->set_nTrackJetInFatJet(m_vars.matchedLeadTrackJets.size());
    }
    m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);
    m_histNameSvc->set_nFatJet(m_vars.fatJets.size());
    if (!m_doMergePtVBins) {
      m_histNameSvc->set_pTV(m_vars.metVec.Pt());
    }
    m_histNameSvc->set_nBTagTrackJetUnmatched(m_physicsMeta.nAddBTrkJets);
    m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
  }

  if (m_debug) {
    Info("SetEventFlavour()", "End of function SetEventFlavour()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SelectCandidateVectors() {
  // The vectors that we want to use depend on the regime and the model
  // Here we select the correct object from our Higgs and VH candidates

  if (m_debug) {
    Info("SelectCandidateVectors()",
         "Beginning of function SelectCandidateVectors()");
  }

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    // use the resolved higgs candidate
    m_vars.HVec = m_vars.resolvedH.vec_corr;

    // calculate bbj if a third jet is available
    if (m_physicsMeta.nJets > 2) {
      m_vars.bbjVec =
          m_vars.resolvedH.vec_corr + m_vars.selectedEventJets[2].vec_corr;
    }

    // use rescaled mass in SR and corrected mass in mBB sidewindow
    if (m_histNameSvc->get_description() == "SR") {
      m_vars.VHVec = m_vars.resolvedVH.vec_resc;
    } else {
      m_vars.VHVec = m_vars.resolvedVH.vec_corr;
    }
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    // use the merged higgs candidate
    m_vars.HVec = m_vars.mergedH.vec_corr;

    // use rescaled mass in SR and corrected mass in mBB sidewindow
    if (m_histNameSvc->get_description() == "SR") {
      m_vars.VHVec = m_vars.mergedVH.vec_resc;
    } else {
      m_vars.VHVec = m_vars.mergedVH.vec_corr;
    }
  }

  if (m_debug) {
    Info("SelectCandidateVectors()",
         "End of function SelectCandidateVectors()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SetHistNameSvcDescription() {
  // Configure the histNameSvc with the region/description

  if (m_debug) {
    Info("SetHistNameSvcDescription()",
         "Beginning of function SetHistNameSvcDescription()");
  }

  m_histNameSvc->set_description("");

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    if (passSpecificCuts(m_vars.eventFlag, m_vars.cuts_SR_resolved)) {
      m_histNameSvc->set_description("SR");
    } else if (passSpecificCuts(m_vars.eventFlag,
                                m_vars.cuts_common_resolved) &&
               passSpecificCuts(m_vars.eventFlag,
                                {ZeroLeptonCuts::MHCorrResolvedSideBand})) {
      // resolved mBB sideband
      if (m_physicsMeta.mbbSideBandResolved ==
          PhysicsMetadata::MbbSideBandResolved::Low) {
        // low mBB
        if (m_doMergeCR) {
          m_histNameSvc->set_description("mBBcr");
        } else {
          m_histNameSvc->set_description("lowmBBcr");
        }
      } else if (m_physicsMeta.mbbSideBandResolved ==
                 PhysicsMetadata::MbbSideBandResolved::High) {
        // high mBB
        if (m_doMergeCR) {
          m_histNameSvc->set_description("mBBcr");
        } else {
          m_histNameSvc->set_description("highmBBcr");
        }
      }
    }
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    if (passSpecificCuts(m_vars.eventFlag, m_vars.cuts_SR_merged)) {
      // merged signal region
      m_histNameSvc->set_description("SR");
    } else if (passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged) &&
               passSpecificCuts(m_vars.eventFlag,
                                {ZeroLeptonCuts::MHCorrMergedSideBand})) {
      // merged mBB sideband
      if (m_physicsMeta.mbbSideBandMerged ==
          PhysicsMetadata::MbbSideBandMerged::Low) {
        // low mBB
        if (m_doMergeCR) {
          m_histNameSvc->set_description("mBBcr");
        } else {
          m_histNameSvc->set_description("lowmBBcr");
        }
      } else if (m_physicsMeta.mbbSideBandMerged ==
                 PhysicsMetadata::MbbSideBandMerged::High) {
        // high mBB
        if (m_doMergeCR) {
          m_histNameSvc->set_description("mBBcr");
        } else {
          m_histNameSvc->set_description("highmBBcr");
        }
      }
    }
  }

  if (m_debug) {
    Info("SetHistNameSvcDescription()",
         "End of function SetHistNameSvcDescription()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::ApplyBTaggingWeight() {
  // Calcualtes the b tagging weight from the correct set of jets and applies
  // it to m_weight. If m_doTruthTagging is true, it also stores the weight in
  // the event info

  if (m_debug) {
    Info("ApplyBTaggingWeight()",
         "Beginning of function ApplyBTaggingWeight()");
  }

  // only for MC
  if (!m_isMC) {
    return EL::StatusCode::SUCCESS;
  }

  float btagWeight = 1.;
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    btagWeight =
        computeBTagSFWeight(m_vars.signalJets, m_jetReader->getContainerName());
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    btagWeight = computeBTagSFWeight(m_vars.trackJetsForBTagging,
                                     m_trackJetReader->getContainerName());
  }
  m_weight *= btagWeight;
  if (m_doTruthTagging) {
    BTagProps::truthTagEventWeight.set(m_eventInfo, btagWeight);
  }

  if (m_debug) {
    Info("ApplyBTaggingWeight()", "End of function ApplyBTaggingWeight()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::DoVJetsVariations() {
  // V+jet variations
  // NOTE :: mBB is in MeV because CorrsAndSysts uses MeV

  if (m_debug) {
    Info("DoVJetsVariations()", "Beginning of function DoVJetsVariations()");
  }

  if (m_debug) {
    Info("DoVJetsVariations()", "Before calling apply_EvtWeights()");
  }
  // Internal weight variations Histograms
  if (m_evtWeightVarMode != -1 && m_currentVar == "Nominal" && m_isMC) {
    EL_CHECK("applyEvtWeights()", apply_EvtWeights());
  }

  // V+Jet Systematic Histograms
  if (m_currentVar != "Nominal") {
    if (m_debug) {
      Info("DoVJetsVariations()", "Not Nominal -> return");
    }
    return EL::StatusCode::SUCCESS;
  }
  if (!(m_histNameSvc->get_sample() == "W" ||
        m_histNameSvc->get_sample() == "Wv22" ||
        m_histNameSvc->get_sample() == "Z" ||
        m_histNameSvc->get_sample() == "Zv22")) {
    if (m_debug) {
      Info("DoVJetsVariations()", "Not the right sample (%s) -> return",
           m_histNameSvc->get_sample().c_str());
    }
    return EL::StatusCode::SUCCESS;
  }
  float mVH;
  float mBB = m_vars.HVec.M();
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    mVH = m_vars.VHVec.M();
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    mVH = m_vars.mergedVH.vec_corr.M();
  } else {
    if (m_debug) {
      Info("DoVJetsVariations()", "Neither resolved nor merged -> return");
    }
    return EL::StatusCode::SUCCESS;
  }
  std::string Regime = m_physicsMeta.regime == PhysicsMetadata::Regime::resolved
                           ? "Resolved"
                           : "Boosted";

  if (m_debug) {
    Info("DoVJetsVariations()", "Before calling fillVjetsSystslep_PRSR");
  }
  fillVjetsSystslep_PRSR(Regime, mBB, mVH / 1e3);  // mVH = GeV, mBB = MeV

  if (m_debug) {
    Info("DoVJetsVariations()", "End of function DoVJetsVariations()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::DoTTbarVariations() {
  // Some systematics for ttbar

  if (m_debug) {
    Info("DoTTbarVariations()", "Beginning of function DoTTbarVariations()");
  }

  if (m_histNameSvc->get_sample() != "ttbar") {
    if (m_debug) {
      Info("DoTTbarVariations()", "Not ttbar -> return");
    }
    return EL::StatusCode::SUCCESS;
  }
  if (m_physicsMeta.regime != PhysicsMetadata::Regime::resolved &&
      m_physicsMeta.regime != PhysicsMetadata::Regime::merged) {
    if (m_debug) {
      Info("DoTTbarVariations()", "Neither resolved nor merged -> return");
    }
    return EL::StatusCode::SUCCESS;
  }

  bool isNominal = m_currentVar == "Nominal";

  if (m_nominalOnly || !isNominal) {
    if (m_debug) {
      Info("DoTTbarVariations()", "m_nominalOnly || !isNominal -> return");
    }
    return EL::StatusCode::SUCCESS;
  }

  float mBB = m_vars.HVec.M();
  float mVH = m_physicsMeta.regime == PhysicsMetadata::Regime::resolved
                  ? m_vars.VHVec.M()
                  : m_vars.mergedVH.vec_corr.M();
  std::string Regime = m_physicsMeta.regime == PhysicsMetadata::Regime::resolved
                           ? "Resolved"
                           : "Boosted";
  float nAddBTags = m_physicsMeta.regime == PhysicsMetadata::Regime::resolved
                        ? 0
                        : m_physicsMeta.nAddBTrkJets;

  if (m_debug) {
    Info("DoTTbarVariations()", "Before calling fillTopSystslep_PRSR");
  }
  fillTopSystslep_PRSR(Regime, nAddBTags, mBB,
                       mVH / 1e3);  // mVH = GeV, mBB = MeV

  if (m_debug) {
    Info("DoTTbarVariations()", "End of function DoTTbarVariations()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::fill0LepCutFlow() {
  // Overloads the method from AnalysisReader_VHQQ0Lep to use member variables

  if (m_debug) {
    Info("fill0LepCutflow()", "Beginning of function fill0LepCutflow()");
  }

  if (!m_doCutflow) {
    if (m_debug) {
      Info("fill0LepCutflow()", "!m_doCutflow ->return");
    }
    return EL::StatusCode::SUCCESS;
  }

  AnalysisReader_VHQQ0Lep::fill0LepCutFlow(m_vars.eventFlag);

  if (m_debug) {
    Info("fill0LepCutflow()", "End of function fill0LepCutflow()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::DoResolvedOutput() {
  // Writes all histograms and event trees in resolved
  // The trees might get filled event if we are not in the resolved regime
  // (depends on settings in the config)
  // The histograms, however, are only filled if we are in resolved and pass
  // the necessary cuts

  if (m_debug) {
    Info("DoResolvedOutput()", "Beginning of function DoResolvedOutput()");
  }

  if (!m_alwaysFillEasyTree &&
      m_physicsMeta.regime != PhysicsMetadata::Regime::resolved) {
    if (m_debug) {
      Info("DoResolvedOutput()",
           "!m_alwaysFillEasyTree && m_physicsMeta.regime != "
           "PhysicsMetadata::Regime::resolved -> return");
    }
    return EL::StatusCode::SUCCESS;
  }
  if (!m_alwaysFillEasyTree &&
      !passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_resolved)) {
    if (m_debug) {
      Info("DoResolvedOutput()",
           "!m_alwaysFillEasyTree && !passSpecificCuts(m_eventFlag, "
           "m_cuts_common_resolved) -> return");
    }
    return EL::StatusCode::SUCCESS;
  }

  // when filling the histograms we make sure that we are in the right regime
  // and pass the right cuts
  if (m_physicsMeta.regime != PhysicsMetadata::Regime::resolved) {
    return EL::StatusCode::SUCCESS;
  }
  if (!passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_resolved)) {
    return EL::StatusCode::SUCCESS;
  }
  if (m_debug) {
    Info("DoResolvedOutput()",
         "Before calling applyExtension0LepTTbarWeight()");
  }
  // reweight for extension 0Lep TTbar
  EL_CHECK("AnalysisReader_VHreso0Lep::DoResolvedOutput()",
           applyExtension0LepTTbarWeight());

  // reweight for extension 0Lep Znunu
  if (m_debug) {
    Info("DoResolvedOutput()",
         "Before calling applyExtension0LepZnunuWeight()");
  }
  EL_CHECK("AnalysisReader_VHreso0Lep::DoResolvedOutput()",
           applyExtension0LepZnunuWeight());

  if (m_histNameSvc->get_description() == "") {
    if (m_debug) {
      Info("DoResolvedOutput()", "Empty histNameSvc description -> return");
      return EL::StatusCode::SUCCESS;
    }
  }

  // Fill histograms
  if (!m_doOnlyInputs) {
    // inclusive ntags
    m_histNameSvc->set_nTag(-1);  // b-tag
    fill0LepHistResolved();
  }
  // exclusive ntags - write always
  m_histNameSvc->set_nTag(m_physicsMeta.nTags);  // b-tag
  if (m_debug) {
    Info("DoResolvedOutput()", "Before calling fill0LepHistResolved()");
  }
  fill0LepHistResolved();

  if (m_debug) {
    Info("DoResolvedOutput()", "End of function DoResolvedOutput()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::DoMergedOutput() {
  // Writes all histograms and event trees in merged

  if (m_debug) {
    Info("DoMergedOutput()", "Beginning of function DoMergedOutput()");
  }

  if (!m_alwaysFillEasyTree &&
      m_physicsMeta.regime != PhysicsMetadata::Regime::merged) {
    if (m_debug) {
      Info("DoMergedOutput()",
           "!m_alwaysFillEasyTree && m_physicsMeta.regime != "
           "PhysicsMetadata::Regime::merged -> return");
    }
    return EL::StatusCode::SUCCESS;
  }
  if (!m_alwaysFillEasyTree &&
      !passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged)) {
    if (m_debug) {
      Info("DoMergedOutput()",
           "!m_alwaysFillEasyTree && !passSpecificCuts(m_eventFlag, "
           "m_cuts_common_merged) -> return");
    }
    return EL::StatusCode::SUCCESS;
  }

  if (m_physicsMeta.regime != PhysicsMetadata::Regime::merged ||
      !passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged)) {
    if (m_debug) {
      Info("DoMergedOutput()",
           "m_physicsMeta.regime != PhysicsMetadata::Regime::merged || "
           "!passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged) -> "
           "return");
    }
    return EL::StatusCode::SUCCESS;
  }

  // fill histograms (for limits) for the boosted analysis regime
  if (m_histNameSvc->get_description() != "") {
    fill0LepHistMerged();
  }

  if (m_doOnlyInputs) {
    if (m_debug) {
      Info("DoMergedOutput()", "m_doOnlyInputs -> return");
    }
    return EL::StatusCode::SUCCESS;
  }
  // fill additional sets of histograms for the boosted analysis regime

  // **Set 1**: exclusive mass regions
  //--------------------------------
  // mass excl / btag incl / addbtag incl
  m_histNameSvc->set_nTag(-1);                    // b-tag
  m_histNameSvc->set_nBTagTrackJetUnmatched(-1);  // external track jet b-tag
  if (m_debug) {
    Info("DoMergedOutput()",
         "Before fill0LepHistMerged (nTag -1; nBTagTrackJetUnmatched -1");
  }
  fill0LepHistMerged();

  // mass excl / btag incl / addbtag excl
  m_histNameSvc->set_nTag(-1);  // b-tag
  m_histNameSvc->set_nBTagTrackJetUnmatched(
      m_physicsMeta.nAddBTrkJets);  // external track jet b-tag
  if (m_debug) {
    Info("DoMergedOutput()",
         "Before fill0LepHistMerged (nTag -1; nbTagTrackJetUnmachted is "
         "nAddBtrkJets");
  }
  fill0LepHistMerged();

  // mass excl / btag excl / addbtag incl
  m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);  // b-tag
  m_histNameSvc->set_nBTagTrackJetUnmatched(-1);     // external track jet b-tag
  if (m_debug) {
    Info("DoMergedOutput()",
         "Before fill0LepHistMerged (nTag nTagsInFJ; nbTagTrackJetUnmachted "
         "-1");
  }
  fill0LepHistMerged();

  // **Set 2**: inclusive mass region
  //--------------------------------
  m_histNameSvc->set_description("mBBincl");
  //--------------------------------

  // mass incl / btag incl / addbtag incl
  m_histNameSvc->set_nTag(-1);                    // b-tag
  m_histNameSvc->set_nBTagTrackJetUnmatched(-1);  // external track jet b-tag
  if (m_debug) {
    Info("DoMergedOutput()",
         "Before fill0LepHistMerged (mBBincl; nTag -1; nbTagTrackJetUnmachted "
         "-1");
  }
  fill0LepHistMerged();

  // mass incl / btag incl / addbtag excl
  m_histNameSvc->set_nTag(-1);  // b-tag
  m_histNameSvc->set_nBTagTrackJetUnmatched(
      m_physicsMeta.nAddBTrkJets);  // external track jet b-tag
  if (m_debug) {
    Info("DoMergedOutput()",
         "Before fill0LepHistMerged (mBBincl; nTag -1; nbTagTrackJetUnmachted "
         "nAddBtrkJets");
  }
  fill0LepHistMerged();

  // mass incl / btag excl / addbtag incl
  m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);  // b-tag
  m_histNameSvc->set_nBTagTrackJetUnmatched(-1);     // external track jet b-tag
  if (m_debug) {
    Info("DoMergedOutput()",
         "Before fill0LepHistMerged (mBBincl; nTag nTagsInFJ; "
         "nbTagTrackJetUnmachted -1");
  }
  fill0LepHistMerged();

  // mass incl / btag incl / addbtag incl
  m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);  // b-tag
  m_histNameSvc->set_nBTagTrackJetUnmatched(
      m_physicsMeta.nAddBTrkJets);  // external track jet b-tag
  if (m_debug) {
    Info("DoMergedOutput()",
         "Before fill0LepHistMerged (mBBincl; nTag nTagsInFJ; "
         "nbTagTrackJetUnmachted nAddBTrkJets");
  }
  fill0LepHistMerged();

  if (m_debug) {
    Info("DoMergedOutput()", "End of function DoMergedOutput()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::FillEasyTree() {
  if (m_writeEasyTree && m_histNameSvc->get_isNominal()) {
    m_etree->Reset();
    m_etree->SetVariation(m_currentVar);

    EL_CHECK("AnalysisReader_VHreso0Lep::FillEasyTree()",
             SetCommonBranchesAndValues());

    m_etree->Fill();
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::SetCommonBranchesAndValues() {
  // branches in the easy tree that are filled for both the resolved and the
  // merged selection

  if (m_debug) {
    Info("SetCommonBranchesAndValues()",
         "Beginning of function SetCommonBranchesAndValues()");

    Info("SetCommonBranchesAndValues()",
         "m_vars.selectedEventJets.size()    = %ld",
         m_vars.selectedEventJets.size());
    Info("SetCommonBranchesAndValues()", "m_vars.selectedJets.size() = %ld",
         m_vars.selectedJets.size());
    Info("SetCommonBranchesAndValues()", "m_vars.fatJets.size()      = %ld",
         m_vars.fatJets.size());
  }

  m_etree->SetBranchAndValue<std::string>("Description",
                                          m_histNameSvc->get_description(), "");
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    m_etree->SetBranchAndValue<std::string>("Regime", "resolved", "none");
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    m_etree->SetBranchAndValue<std::string>("Regime", "merged", "none");
  }

  m_etree->SetBranchAndValue<std::string>("Sample",
                                          m_histNameSvc->getFullSample(), "");

  m_etree->SetBranchAndValue<int>(
      "passesResolvedSR",
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_SR_resolved), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMergedSR",
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_SR_merged), -1);
  m_etree->SetBranchAndValue<int>(
      "passesResolved",
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_resolved), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMerged",
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged), -1);

  if (m_isMC) {
    m_etree->SetBranchAndValue<int>("ChannelNumber",
                                    m_eventInfo->mcChannelNumber(), -99);
  }
  m_etree->SetBranchAndValue<int>("RunNumber", m_eventInfo->runNumber(), -99);
  m_etree->SetBranchAndValue<int>("EventNumber", m_eventInfo->eventNumber(),
                                  -99);
  m_etree->SetBranchAndValue<float>("EventWeight", m_weight, -99.);
  m_etree->SetBranchAndValue<float>("AverageMu", m_averageMu, -99);
  m_etree->SetBranchAndValue<float>("ActualMu", m_actualMu, -99);
  m_etree->SetBranchAndValue<float>("AverageMuScaled", m_averageMuScaled, -99);
  m_etree->SetBranchAndValue<float>("ActualMuScaled", m_actualMuScaled, -99);

  m_etree->SetBranchAndValue<int>("nJ", m_physicsMeta.nJets, -99);
  m_etree->SetBranchAndValue<int>("nSigJet", m_physicsMeta.nSigJet, -99);
  m_etree->SetBranchAndValue<int>("nForwardJet", m_physicsMeta.nForwardJet,
                                  -99);
  m_etree->SetBranchAndValue<int>("nFatJets", m_vars.fatJets.size(), -99);
  if (m_vars.fatJets.size() >= 1) {
    m_etree->SetBranchAndValue<int>("nbTagsInFJ", m_physicsMeta.nTagsInFJ, -99);
    m_etree->SetBranchAndValue<int>(
        "nTrackJetsInFJ", Props::nTrackJets.get(m_vars.fatJets[0]), -99);
  }
  m_etree->SetBranchAndValue<int>("nbTagsOutsideFJ", m_physicsMeta.nAddBTrkJets,
                                  -99);
  m_etree->SetBranchAndValue<int>("nTrackJets", m_vars.trackJets.size(), -99);
  m_etree->SetBranchAndValue<int>("nTags", m_physicsMeta.nTags, -99);
  m_etree->SetBranchAndValue<int>("nTaus", m_vars.taus.size(), -99);

  m_etree->SetBranchAndValue<float>("mVH", m_vars.VHVec.M(), -99.);
  m_etree->SetBranchAndValue<float>("mBB", m_vars.HVec.M(), -99.);
  m_etree->SetBranchAndValue<float>("MET", m_vars.metVec.Pt(), -99.);
  m_etree->SetBranchAndValue<float>("METSig", m_vars.metSig, -99.);
  if (m_vars.metSig_PU != -1) {
    m_etree->SetBranchAndValue<float>("METSig_PU", m_vars.metSig_PU, -99.);
  } else {
    m_etree->SetBranchAndValue<float>("METSig_soft", m_vars.metSig_soft, -99.);
    m_etree->SetBranchAndValue<float>("METSig_hard", m_vars.metSig_hard, -99.);
  }
  m_etree->SetBranchAndValue<float>("METOverSqrtSumET", m_vars.metOverSqrtSumET,
                                    -99.);
  m_etree->SetBranchAndValue<float>("METOverSqrtHT", m_vars.metOverSqrtHT,
                                    -99.);
  m_etree->SetBranchAndValue<float>("sumpt", m_vars.sumpt, -99.);
  m_etree->SetBranchAndValue<float>("dPhiMETMPT", m_vars.dPhiMETMPT, -99.);
  m_etree->SetBranchAndValue<float>("dPhiMETdijet", m_vars.dPhiMETdijetResolved,
                                    -99.);
  m_etree->SetBranchAndValue<float>("mindPhi", m_vars.mindPhi, -99.);
  m_etree->SetBranchAndValue<float>("MPT", m_vars.mptVec.M(), -99.);
  if (m_vars.selectedJets.size() >= 1 && m_vars.selectedEventJets.size() >= 1) {
    m_etree->SetBranchAndValue<float>(
        "pTB1", m_vars.selectedEventJets[0].vec_corr.Pt(), -99.);
    m_etree->SetBranchAndValue<float>(
        "etaB1", m_vars.selectedEventJets[0].vec_corr.Eta(), -99.);
    m_etree->SetBranchAndValue<float>(
        "phiB1", m_vars.selectedEventJets[0].vec_corr.Phi(), -99.);
    m_etree->SetBranchAndValue<float>(
        "mB1", m_vars.selectedEventJets[0].vec_corr.M(), -99.);
    m_etree->SetBranchAndValue<float>(
        "MV2c10B1", Props::MV2c10.get(m_vars.selectedJets[0]), -99.);
    m_etree->SetBranchAndValue<float>(
        "bin_MV2c10B1", BTagProps::Quantile.get(m_vars.selectedJets[0]), -99.);
  }
  if (m_vars.selectedJets.size() >= 2 && m_vars.selectedEventJets.size() >= 2) {
    m_etree->SetBranchAndValue<float>(
        "pTB2", m_vars.selectedEventJets[1].vec_corr.Pt(), -99.);
    m_etree->SetBranchAndValue<float>(
        "etaB2", m_vars.selectedEventJets[1].vec_corr.Eta(), -99.);
    m_etree->SetBranchAndValue<float>(
        "phiB2", m_vars.selectedEventJets[1].vec_corr.Phi(), -99.);
    m_etree->SetBranchAndValue<float>(
        "mB2", m_vars.selectedEventJets[1].vec_corr.M(), -99.);
    m_etree->SetBranchAndValue<float>(
        "MV2c10B2", Props::MV2c10.get(m_vars.selectedJets[1]), -99.);
    m_etree->SetBranchAndValue<float>(
        "bin_MV2c10B2", BTagProps::Quantile.get(m_vars.selectedJets[1]), -99.);

    m_etree->SetBranchAndValue<float>(
        "dRBB",
        m_vars.selectedEventJets[0].vec_corr.DeltaR(
            m_vars.selectedEventJets[1].vec_corr),
        -99.);
    m_etree->SetBranchAndValue<float>(
        "dEtaBB",
        fabs(m_vars.selectedEventJets[0].vec_corr.Eta() -
             m_vars.selectedEventJets[1].vec_corr.Eta()),
        -99.);
    m_etree->SetBranchAndValue<float>(
        "dPhiBB",
        m_vars.selectedEventJets[0].vec_corr.DeltaPhi(
            m_vars.selectedEventJets[1].vec_corr),
        -99.);
    float pTBB = (m_vars.selectedEventJets[0].vec_corr +
                  m_vars.selectedEventJets[1].vec_corr)
                     .Pt();
    m_etree->SetBranchAndValue<float>("pTBB", pTBB, -99);
    m_etree->SetBranchAndValue<float>(
        "yBB",
        fabs((m_vars.selectedEventJets[0].vec_corr +
              m_vars.selectedEventJets[1].vec_corr)
                 .Rapidity()),
        -99);
    m_etree->SetBranchAndValue<float>("pTBBoverMET", pTBB / m_vars.metVec.Pt(),
                                      -99);
    m_etree->SetBranchAndValue<float>(
        "pTBBMETAsym", (pTBB - m_vars.metVec.M()) / (pTBB + m_vars.metVec.M()),
        -99);
    m_etree->SetBranchAndValue<float>(
        "MEff",
        m_vars.selectedEventJets[0].vec_corr.Pt() +
            m_vars.selectedEventJets[1].vec_corr.Pt() + m_vars.metVec.Pt(),
        -99.);
    TLorentzVector bb = m_vars.selectedEventJets[0].vec_corr +
                        m_vars.selectedEventJets[1].vec_corr;
    TVector3 bh = bb.BoostVector();
    TLorentzVector b1s = TLorentzVector(m_vars.selectedEventJets[0].vec_corr);
    b1s.Boost(-bh);
    double cBA = b1s.Vect().Unit().Dot(bh.Unit());
    m_etree->SetBranchAndValue<float>("costheta", fabs(cBA), -99);
  }
  if (m_vars.selectedJets.size() >= 3 && m_vars.selectedEventJets.size() >= 3) {
    m_etree->SetBranchAndValue<float>(
        "pTJ3", m_vars.selectedEventJets[2].vec_corr.Pt(), -99.);
    m_etree->SetBranchAndValue<float>(
        "etaJ3", m_vars.selectedEventJets[2].vec_corr.Eta(), -99.);
    m_etree->SetBranchAndValue<float>(
        "phiJ3", m_vars.selectedEventJets[2].vec_corr.Phi(), -99.);
    m_etree->SetBranchAndValue<float>(
        "mJ3", m_vars.selectedEventJets[2].vec_corr.M(), -99.);
    m_etree->SetBranchAndValue<float>(
        "MV2c10J3", Props::MV2c10.get(m_vars.selectedJets[2]), -99.);
    m_etree->SetBranchAndValue<float>(
        "bin_MV2c10J3", BTagProps::Quantile.get(m_vars.selectedJets[2]), -99.);

    float dRB1J3 = m_vars.selectedEventJets[2].vec_corr.DeltaR(
        m_vars.selectedEventJets[0].vec_corr);
    float dRB2J3 = m_vars.selectedEventJets[2].vec_corr.DeltaR(
        m_vars.selectedEventJets[1].vec_corr);
    m_etree->SetBranchAndValue<float>("dRB1J3", dRB1J3, -99.);
    m_etree->SetBranchAndValue<float>("dRB2J3", dRB2J3, -99.);
    m_etree->SetBranchAndValue<float>("mindRBJ3",
                                      dRB1J3 < dRB2J3 ? dRB1J3 : dRB2J3, -99.);
    m_etree->SetBranchAndValue<float>("maxdRBJ3",
                                      dRB1J3 > dRB2J3 ? dRB1J3 : dRB2J3, -99.);
    m_etree->SetBranchAndValue<float>(
        "mBBJ", (m_vars.HVec + m_vars.selectedEventJets[2].vec_corr).M(), -99.);
    m_etree->SetBranchAndValue<float>(
        "MEff3",
        m_vars.selectedEventJets[0].vec_corr.Pt() +
            m_vars.selectedEventJets[1].vec_corr.Pt() +
            m_vars.selectedEventJets[2].vec_corr.Pt() + m_vars.metVec.Pt(),
        -99.);
  }

  if (m_physicsMeta.nJets == 2) {
    // nJets is set in AnalysisReader_VHQQ::setevent_nJets to signalJets.size()
    // + forwardJets.size(). This means that it could be that the number of
    // signalJets is 0 or 1 and the forwardJets make up for the difference. In
    // our current CutFlow these events are cut away. So we don't bother
    // calculating "HT" from forward jets and set it to -99, i.e. we don't set
    // it at all
    if (m_vars.selectedEventJets.size() >= 2) {
      m_etree->SetBranchAndValue("HT",
                                 m_vars.selectedEventJets[0].vec_corr.Pt() +
                                     m_vars.selectedEventJets[1].vec_corr.Pt() +
                                     m_vars.metVec.Pt(),
                                 -99.);
    }
  } else if (m_physicsMeta.nJets > 2) {
    // In this case we consider the third selected jet. Apart from that, the
    // same logic as above applies.
    if (m_vars.selectedEventJets.size() >= 3) {
      m_etree->SetBranchAndValue("HT",
                                 m_vars.selectedEventJets[0].vec_corr.Pt() +
                                     m_vars.selectedEventJets[1].vec_corr.Pt() +
                                     m_vars.selectedEventJets[2].vec_corr.Pt() +
                                     m_vars.metVec.Pt(),
                                 -99.);
    }
  }

  // full CutFlow
  m_etree->SetBranchAndValue<int>(
      "passesAllCxAOD",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::AllCxAOD}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesTrigger",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::Trigger}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesTauVeto",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::TauVeto}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesAtLeast2Jets",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::AtLeast2Jets}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesAtLeast2SigJets",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::AtLeast2SigJets}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesAtLeast1FatJet",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::AtLeast1FatJet}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesAtLeast1MatchedTrackJet",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::AtLeast1MatchedTrackJet}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesAtLeast2MatchedTrackJets",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::AtLeast2MatchedTrackJets}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesExactly2FatJetBTags",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::Exactly2FatJetBTags}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesMETResolved",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::METResolved}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMETMerged",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::METMerged}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesDPhiMETMPT",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::DPhiMETMPT}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMinDPhiMETJetsResolved",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::MinDPhiMETJetsResolved}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesMinDPhiMETJetsMerged",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::MinDPhiMETJetsMerged}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesSumJetPt",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::SumJetPt}), -1);
  m_etree->SetBranchAndValue<int>(
      "MinMPTBJet0or1Resolved",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::MinMPTBJet0or1Resolved}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesMinMPTMerged",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::MinMPTMerged}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesLeadJetPt",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::LeadJetPt}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesDPhiBB",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::DPhiBB}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesDPhiMETDijetResolved",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::DPhiMETDijetResolved}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesDPhiMETDijetMerged",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::DPhiMETDijetMerged}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesDRB1B2",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::DRB1B2}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMETLT500",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::METLT500}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMETGT500",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::METGT500}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMHCorrResolved",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::MHCorrResolved}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMHCorrMerged",
      passSpecificCuts(m_vars.eventFlag, {ZeroLeptonCuts::MHCorrMerged}), -1);
  m_etree->SetBranchAndValue<int>(
      "passesMHCorrResolvedSideBand",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::MHCorrResolvedSideBand}),
      -1);
  m_etree->SetBranchAndValue<int>(
      "passesMHCorrMergedSideBand",
      passSpecificCuts(m_vars.eventFlag,
                       {ZeroLeptonCuts::MHCorrMergedSideBand}),
      -1);

  // some variables for Run-2 paper studies
  float truthHiggsPt = -1;
  float truthHiggsEta = -1;
  float truthHiggsPhi = -1;
  float truthHiggsM = -1;
  float leadingFatJetHiggsDR = -1;
  float subleadingFatJetHiggsDR = -1;
  // Find Truth Higgs
  if (m_isMC) {
    for (auto *part : *m_truthParts) {
      if (!part) {
        continue;
      }
      if (part->pdgId() != 25) {
        continue;
      }
      truthHiggsPt = part->pt();
      truthHiggsEta = part->eta();
      truthHiggsPhi = part->phi();
      truthHiggsM = part->m();
      TLorentzVector truthHiggsVec = part->p4();
      if (m_vars.fatJets.size() > 0) {
        leadingFatJetHiggsDR = truthHiggsVec.DeltaR(m_vars.fatJets[0]->p4());
      }
      if (m_vars.fatJets.size() > 1) {
        subleadingFatJetHiggsDR = truthHiggsVec.DeltaR(m_vars.fatJets[1]->p4());
      }
      break;
    }
  }
  if (m_vars.fatJets.size() > 0) {
    m_etree->SetBranchAndValue<float>("leadingFatJetPt",
                                      m_vars.fatJets[0]->pt(), -1);
    m_etree->SetBranchAndValue<float>("leadingFatJetM", m_vars.fatJets[0]->m(),
                                      -1);
    m_etree->SetBranchAndValue<float>("leadingFatJetEta",
                                      m_vars.fatJets[0]->eta(), -1);
    m_etree->SetBranchAndValue<float>("leadingFatJetPhi",
                                      m_vars.fatJets[0]->phi(), -1);
    m_etree->SetBranchAndValue<float>("leadingFatJetHiggsDR",
                                      leadingFatJetHiggsDR, -3);
  }
  if (m_vars.fatJets.size() > 1) {
    m_etree->SetBranchAndValue<float>("subleadingFatJetPt",
                                      m_vars.fatJets[1]->pt(), -1);
    m_etree->SetBranchAndValue<float>("subleadingFatJetM",
                                      m_vars.fatJets[1]->m(), -1);
    m_etree->SetBranchAndValue<float>("subleadingFatJetEta",
                                      m_vars.fatJets[1]->eta(), -1);
    m_etree->SetBranchAndValue<float>("subleadingFatJetPhi",
                                      m_vars.fatJets[1]->phi(), -1);
    m_etree->SetBranchAndValue<float>("subleadingFatJetHiggsDR",
                                      subleadingFatJetHiggsDR, -3);
  }

  m_etree->SetBranchAndValue<float>("truthHiggsPt", truthHiggsPt, -3);
  m_etree->SetBranchAndValue<float>("truthHiggsEta", truthHiggsEta, -3);
  m_etree->SetBranchAndValue<float>("truthHiggsPhi", truthHiggsPhi, -3);
  m_etree->SetBranchAndValue<float>("truthHiggsM", truthHiggsM, -3);
  m_etree->SetBranchAndValue<float>("higgsCandidatePt", m_vars.HVec.Pt(), -1);

  if (m_allLeadFJMatchedTrackJets.size() > 0) {
    m_etree->SetBranchAndValue<float>("TrackJet1Pt",
                                      m_allLeadFJMatchedTrackJets[0]->pt(), -1);
  }
  if (m_allLeadFJMatchedTrackJets.size() > 1) {
    m_etree->SetBranchAndValue<float>("TrackJet2Pt",
                                      m_allLeadFJMatchedTrackJets[1]->pt(), -1);
    m_etree->SetBranchAndValue<float>(
        "dRTrackJet12",
        m_allLeadFJMatchedTrackJets[0]->p4().DeltaR(
            m_allLeadFJMatchedTrackJets[1]->p4()),
        -1);
    float pt1 = m_allLeadFJMatchedTrackJets[0]->pt();
    float pt2 = m_allLeadFJMatchedTrackJets[1]->pt();
    m_etree->SetBranchAndValue<float>("pTTrackJet12Asymmetry",
                                      (pt1 - pt2) / (pt1 + pt2), -1);
  }
  if (m_allLeadFJMatchedTrackJets.size() > 2) {
    m_etree->SetBranchAndValue<float>("TrackJet2Pt",
                                      m_allLeadFJMatchedTrackJets[2]->pt(), -1);
  }

  if (m_debug) {
    Info("SetCommonBranchesAndValues()",
         "End of function SetCommonBranchesAndValues()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso0Lep::initializeEvent() {
  if (m_debug) {
    Info("initializeEvent", "This is initializeEvent from the VHreso reader.");
  }

  if (m_debug) {
    Info("initalizeEvent()", "Calling the base class initializeEvent now");
  }
  EL_CHECK("initialize()", AnalysisReader::initializeEvent());

  // Read all variables that are relevant for the VHreso analysis from the
  // config and store the information in appropriate member variables.
  // The method consists of two parts. First, all default values are
  // initialised and then the information is read from the config.

  // default values
  m_nominalOnly = false;
  m_alwaysFillEasyTree = false;
  m_applyPUWeight = false;
  m_applyTauVeto = false;
  m_doTrackJetOverlapVeto = false;
  m_doMETSignificanceCut = false;
  m_generateSTXSsignals = false;
  m_writeEasyTree = false;
  m_alwaysFillEasyTree = false;

  // values from config
  m_config->getif<bool>("nominalOnly", m_nominalOnly);
  m_config->getif<bool>("applyPUWeight", m_applyPUWeight);
  m_config->getif<bool>("applyTauVetoHVT", m_applyTauVeto);
  m_config->getif<bool>("alwaysFillEasyTree", m_alwaysFillEasyTree);
  m_config->getif<bool>("doTrackJetOverlapVeto", m_doTrackJetOverlapVeto);
  m_config->getif<bool>("doMETSignificanceCut", m_doMETSignificanceCut);
  m_config->getif<bool>("generateSTXSsignals", m_generateSTXSsignals);
  m_config->getif<bool>("writeEasyTree", m_writeEasyTree);
  m_config->getif<bool>("alwaysFillEasyTree", m_alwaysFillEasyTree);

  if (m_debug) {
    Info("initalizeEvent()", "End of function initializeEvent()");
  }

  return EL::StatusCode::SUCCESS;
}

// Elisabeth: comment out
// -> you could use AnalysisReader_VHQQ0Lep::fillMVATreeVHbbResolved0Lep
// -> if additional variables needed:
//        -> use EasyTree or
//        -> create MVATree_VHreso

// EL::StatusCode AnalysisReader_VHreso0Lep::fillMVATreeResolved() {
// Overloads the AnalysisReader_VHQQ0Lep::fillMVATreeResolved
// The best option would be to fill the MVA tree with information from the
// EasyTree. That way, we only have to take care of what we put into the
// EasyTree. But currently the EasyTree can be turned off in the config and it
// is also never filled for systematics. That's why we will the MVAtree here
// again with independent variables. In the worst case a variable like "MET"
// can have different values in the two trees. So make sure that you pay
// attention when changing entries!

// Jet empty;

// AnalysisReader_VHQQ0Lep::fillMVATreeResolved(
//     m_vars.VHVec, m_vars.HVec, m_vars.metVec, m_vars.mptVec,
//     m_vars.selectedJets,
//     m_vars.selectedEventJets.size() > 0 ? m_vars.selectedEventJets[0] :
//     empty, m_vars.selectedEventJets.size() > 1 ?
//     m_vars.selectedEventJets[1] : empty, m_vars.selectedEventJets.size() >
//     2 ? m_vars.selectedEventJets[2] : empty, m_vars.sumpt, m_vars.dRb1b2,
//     m_vars.dPhiMETMPT, m_vars.dPhiMETdijetResolved, m_vars.mindPhi,
//     m_vars.metSig, m_vars.metSig_PU, m_vars.metSig_soft,
//     m_vars.metSig_hard, m_vars.metOverSqrtSumET, m_vars.metOverSqrtHT,
//     m_vars.taus.size());

// return EL::StatusCode::SUCCESS;
// }

EL::StatusCode AnalysisReader_VHreso0Lep::fillMVATreeBoosted() {
  // Overloads the AnalysisReader_VHQQ0Lep::fillMVATreeBoosted
  // The best option would be to fill the MVA tree with information from the
  // EasyTree. That way, we only have to take care of what we put into the
  // EasyTree. But currently the EasyTree can be turned off in the config and it
  // is also never filled for systematics. That's why we will the MVAtree here
  // again with independent variables. In the worst case a variable like "MET"
  // can have different values in the two trees. So make sure that you pay
  // attention when changing entries!

  AnalysisReader_VHQQ0Lep::fillMVATreeBoosted(
      m_vars.signalJets, m_vars.forwardJets, m_vars.mptVec, m_vars.metVec,
      m_vars.fatJets.size(), m_vars.matchedLeadTrackJets.size(), m_vars.HVec,
      m_vars.dPhiMETMPT, m_vars.mindPhiMerged, m_vars.mindPhiMerged_3LeadJets,
      m_vars.mergedVH, m_vars.VHVec);

  return EL::StatusCode::SUCCESS;
}
