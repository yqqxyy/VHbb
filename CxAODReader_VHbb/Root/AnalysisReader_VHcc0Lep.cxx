#include <algorithm>
#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader/OSTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb0lep.h"
#include "CxAODTools_VHbb/VHbb0lepEvtSelection.h"

#include <CxAODReader_VHbb/AnalysisReader_VHcc0Lep.h>
#include <TMVA/Reader.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

EL::StatusCode AnalysisReader_VHcc0Lep::run_0Lep_analysis() {
  // only Resolved MVA/CUT analysis supported for VHcc
  //-----------------------------
  if (m_analysisStrategy != "Resolved") {
    Error("AnalysisReader_VHcc0Lep::run_0Lep_analysis",
          "VHcc0Lep only support Resolved CUT/MVA searchd");
    return EL::StatusCode::FAILURE;
  }

  if (m_model == Model::MVA) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::MVA);
  } else if (m_model == Model::CUT) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
  } else {
    Error("AnalysisReader_VHcc0Lep::run_0Lep_analysis",
          "VHcc0Lep only support Resolved CUT/MVA searchd");
    return EL::StatusCode::FAILURE;
  }

  // Reset physics metadata
  m_physicsMeta = PhysicsMetadata();

  if (m_isMC) m_bTagTool->setMCIndex_VHcc(m_mcChannel, m_period);

  ResultVHbb0lep selectionResult =
      ((VHbb0lepEvtSelection *)m_eventSelection)->result();

  m_physicsMeta.channel = PhysicsMetadata::Channel::ZeroLep;

  const xAOD::MissingET *met = selectionResult.met;
  std::vector<const xAOD::Jet *> signalJets = selectionResult.signalJets;
  std::vector<const xAOD::Jet *> forwardJets = selectionResult.forwardJets;
  std::vector<const xAOD::TauJet *> taus = selectionResult.taus;

  // set jet number: m_physicsMeta.nJets, m_physicsMeta.nSigJet,
  // m_physicsMeta.nForwardJet
  // nJets = nSigJet + nForwardJet
  setevent_nJets(signalJets, forwardJets);

  // skip events with 4+ jets
  if (m_doOnlyInputs && m_physicsMeta.nJets > 4) {
    return EL::StatusCode::SUCCESS;
  }

  ////////////////////////////////////////////////////////////
  //         OBJECT DEFINITIONS                             //
  ////////////////////////////////////////////////////////////

  // **Definition** : c/b-tagging for small-R jets
  // compute_btagging();
  //---------------------------------------------------------

  std::vector<const xAOD::Jet *> selectedJets;
  int tagcatExcl = -1;
  int sub_leading_bjets = 0;

  m_bTagTool->tagjet_selection_vhcc(signalJets, forwardJets, selectedJets,
                                    tagcatExcl, sub_leading_bjets);
  for (const auto &jet : selectedJets) {
    BTagProps::Quantile.set(jet, -1);
  }

  m_physicsMeta.nTags = tagcatExcl;

  if (tagcatExcl == 0 && signalJets.size() == 0) selectedJets.clear();

  // **Definition** : jets
  //---------------------------------------------------------
  // small-R jets for mBB/mCC calculation, taken from selectedJets
  Jet j1_sel, j2_sel, j3_sel;
  EL_CHECK("AnalysisReader_VHcc0Lep::run_0Lep_analysis()",
           setJetVariables(j1_sel, j2_sel, j3_sel, selectedJets));

  // MET vector
  //---------------------------------------------------------
  TLorentzVector metVec;
  metVec.SetPxPyPzE(met->mpx(), met->mpy(), 0, met->met());

  // MPT vector
  //---------------------------------------------------------
  TLorentzVector mptVec;
  mptVec.SetPxPyPzE(m_mpt->mpx(), m_mpt->mpy(), 0, m_mpt->met());

  // MET truth
  // if(m_truth){ truthMET=tr->met()/1000.; }
  // getTruthMET();

  //  **Definition** : Higgs vector & VH system
  Higgs resolvedH, mergedH;
  VHCand resolvedVH, mergedVH;
  Jet fj1;  // This is just a dummy jet for set(Higgs|VH)Candidate
  setHiggsCandidate(resolvedH, mergedH, j1_sel, j2_sel, fj1);
  setVHCandidate(resolvedVH, mergedVH, metVec, j1_sel, j2_sel, fj1);
  ////////////////////////////////////////////////////////////
  /////////// CHECK SELECTION - STORE INFORMATION   //////////
  ////////////////////////////////////////////////////////////

  // Initialize eventFlag - all cuts are initialized to false by default
  unsigned long int eventFlag = 0;

  // All events (CxAOD)
  //------------------------
  updateFlag(eventFlag, ZeroLeptonCuts::AllCxAOD);

  // **Selection & SF**: trigger
  //----------------------------
  double triggerSF = 1.;
  // C1: trigger
  if (pass0LepTrigger(triggerSF, selectionResult)) {
    updateFlag(eventFlag, ZeroLeptonCuts::Trigger);
  }
  // How many SF includes in the m_weight?
  /* std::cout << "*****" << std::endl;
  std::cout << "First m_weight = " << m_weight << std::endl; */
  if (m_isMC) {
    m_weight *= triggerSF;
  }

  //// **Selection** : Tau veto  (TauVeto is disabled. In order to
  /// back-compatible with CxAOD Framework, this part of codes can not be
  /// removed)
  ////----------------------------
  // updateFlag(eventFlag, ZeroLeptonCuts::TauVeto);

  // **Selection** : MET cut
  //-----------------------------------------
  if (metVec.Pt() > 150e3) {
    updateFlag(eventFlag, ZeroLeptonCuts::METResolved);
  }

  // **Selection** : dPhi(MET,MPT) cut
  //-----------------------------------------
  double dPhiMETMPT = 10000;
  if (!TMath::IsNaN(mptVec.Phi())) {
    // temporary fix, need to understand why mpt vec =
    // nan for some systematics variations
    dPhiMETMPT = fabs(metVec.DeltaPhi(mptVec));
    if (dPhiMETMPT < TMath::Pi() / 2.) {
      updateFlag(eventFlag, ZeroLeptonCuts::DPhiMETMPT);
    }
  }

  // **Selection** : number of jets
  //----------------------------------------
  if (m_physicsMeta.nJets >= 2) {
    updateFlag(eventFlag, ZeroLeptonCuts::AtLeast2Jets);
  }

  // **Selection** : cut on nSignalJet
  //-----------------------------------------
  if (m_physicsMeta.nSigJet >= 2) {
    updateFlag(eventFlag, ZeroLeptonCuts::AtLeast2SigJets);
  }
  if (m_isMC) {
    m_weight *= compute_JVTSF(signalJets);
  }

  // **Selection** : min[dPhi(MET,jets)] cut
  //-----------------------------------------
  // Resolved: check first three leading jets
  unsigned int n_seljets = selectedJets.size();
  double min_dphi = 1000;
  double sumpt = 0;
  // Get min dphi(met, jet) and sum pt in one pass
  for (const auto &jet : selectedJets) {
    sumpt += jet->pt();
    min_dphi = std::min(min_dphi, std::abs(jet->p4().DeltaPhi(metVec)));
  }

  double threshold_mindPhi = (n_seljets >= 3 ? 30 : 20) * (TMath::Pi() / 180);
  if ((n_seljets > 1) && (min_dphi > threshold_mindPhi)) {
    updateFlag(eventFlag, ZeroLeptonCuts::MinDPhiMETJetsResolved);
  }

  // **Selection** : sum[jet_pt] cut
  //-----------------------------------------
  double threshold_sumpt = n_seljets >= 3 ? 150e3 : 120e3;
  if ((n_seljets > 1) && (sumpt > threshold_sumpt)) {
    updateFlag(eventFlag, ZeroLeptonCuts::SumJetPt);
  }

  // **Selection** : MPT cut, applied only for 0 and 1 tag, NOT for 2 tag
  //----------------------
  // tmp disable the MPT cut for VHcc
  // updateFlag(eventFlag, ZeroLeptonCuts::MinMPTBJet0or1Resolved);
  // Changed to 30 GeV based on studies here:
  // https://indico.cern.ch/event/839425/
  if (m_physicsMeta.nTags <= 2 && mptVec.Pt() >= 30e3) {
    updateFlag(eventFlag, ZeroLeptonCuts::MinMPTBJet0or1Resolved);
  }

  // **Selection** : Third b-jet veto : for the moment commented out
  //----------------------
  if (m_doTruthTagging || (sub_leading_bjets == 0))
    updateFlag(eventFlag, ZeroLeptonCuts::ThirdBJetVeto);
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
  if (selectedJets.size() >= 2) {
    dPhiBB = fabs(j1_sel.vec.DeltaPhi(j2_sel.vec));
  }
  if (dPhiBB <= 140 * TMath::Pi() / 180) {
    updateFlag(eventFlag, ZeroLeptonCuts::DPhiBB);
  }

  // **Selection** : dPhi(MET,dijet) cut
  //----------------------
  // double dPhiMETdijetResolved = fabs(metVec.DeltaPhi(resolvedH.vec));
  double dPhiMETdijetResolved = fabs(metVec.DeltaPhi(resolvedH.vec_corr));
  if (dPhiMETdijetResolved >= 120 * TMath::Pi() / 180) {
    updateFlag(eventFlag, ZeroLeptonCuts::DPhiMETDijetResolved);
  }

  // **Selection** : dR(b1,b2) cut (2015-10-18: added this cut)
  // commented for input production
  //----------------------
  double dRb1b2 = 1000;
  if (selectedJets.size() >= 2) {
    // applied in SM measurement, cut based approach only
    dRb1b2 = fabs(j1_sel.vec.DeltaR(j2_sel.vec));
    double threshold_dRb1b2 = 1.6;
    if (metVec.Pt() > 250e3) {
      threshold_dRb1b2 = 1.2;
    }
    if (dRb1b2 < threshold_dRb1b2) {
      updateFlag(eventFlag, ZeroLeptonCuts::DRB1B2);
    }
  }

  // not used in this analysis, tmp disable this cut
  //// **Selection** : mbb invariant mass window + mbb-sidebands
  ////----------------------

  // bool passMHCorrResolved(false), passMHCorrMerged(false),
  // passMHResolvedSideBand(false), passMHMergedSideBand(false);

  // checkMbbWindow(resolvedH.vec_corr, TLorentzVector(), passMHCorrResolved,
  // passMHCorrMerged, passMHResolvedSideBand, passMHMergedSideBand);

  // if ( passMHCorrResolved ){     updateFlag(eventFlag,
  // ZeroLeptonCuts::MHCorrResolved);} if ( passMHResolvedSideBand ){
  // updateFlag(eventFlag, ZeroLeptonCuts::MHCorrResolvedSideBand);}

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
      ZeroLeptonCuts::ThirdBJetVeto,
      ZeroLeptonCuts::LeadJetPt,
      ZeroLeptonCuts::DPhiBB,
      ZeroLeptonCuts::DPhiMETDijetResolved};

  // tmp disable this cut
  // CUT analysis has DRB1B2 cut
  if (m_model == Model::CUT) {
    std::vector<unsigned long int> cuts_SM = {ZeroLeptonCuts::DRB1B2};
    cuts_common_resolved.insert(cuts_common_resolved.end(), cuts_SM.begin(),
                                cuts_SM.end());
  }

  ////cut for VHcc signal region
  // std::vector<unsigned long int> cuts_SR_resolved  =
  // {ZeroLeptonCuts::MHCorrResolved};
  // cuts_SR_resolved.insert(cuts_SR_resolved.end(),
  // cuts_common_resolved.begin(), cuts_common_resolved.end());
  //
  //// --- Cuts for merged analysis --- // useless
  // std::vector<unsigned long int> cuts_common_merged ,cuts_SR_merged;
  ////select merged or resolved regime
  ////--------------------------------------------------------
  // selectRegime(eventFlag, cuts_SR_resolved, cuts_SR_merged,
  // cuts_common_resolved, cuts_common_merged);
  if (passSpecificCuts(eventFlag, {ZeroLeptonCuts::AtLeast2SigJets})) {
    m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  }

  /*
  //--------------------------------------------------------
  // jet histograms before set region
  //--------------------------------------------------------
  // could check jet distribution before other cuts. but has huge amount of QCD
  contaminant. Not very interesting now

  // --- Cuts for jet histograms --- //
  std::vector<unsigned long int> cuts_jet_hists_resolved =
  {ZeroLeptonCuts::AllCxAOD, ZeroLeptonCuts::Trigger,
  ZeroLeptonCuts::METResolved, ZeroLeptonCuts::DPhiMETMPT,
  ZeroLeptonCuts::AtLeast2Jets,  ZeroLeptonCuts::AtLeast2SigJets};

  if ( m_physicsMeta.regime == PhysicsMetadata::Regime::resolved  &&
  passSpecificCuts(eventFlag, cuts_jet_hists_resolved)){ if( !m_doOnlyInputs &&
  !m_doReduceFillHistos )
  {
  if (!m_doMergePtVBins){ m_histNameSvc -> set_pTV(metVec.Pt());}
  EL_CHECK("AnalysisReader_VHcc0Lep::fill_0Lep", fill_nJetHistos(signalJets,
  "Sig")); EL_CHECK("AnalysisReader_VHcc0Lep::fill_0Lep",
  fill_nJetHistos(forwardJets, "Fwd")); m_histSvc->BookFillHist("njets", 25, 0,
  25, m_physicsMeta.nJets,     m_weight);
  }
  }*/

  // Set event flavour, histogram naming
  //------------------------------
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    setevent_flavour(selectedJets);
    // m_histNameSvc->set_nTag(m_physicsMeta.nTags, tagType);
    m_histNameSvc->set_nTag(m_physicsMeta.nTags);
    m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
    if (!m_doMergePtVBins) {
      m_histNameSvc->set_pTV(metVec.Pt());
    }

    if (m_doMergeJetBins && m_physicsMeta.nJets >= 2) {
      m_histNameSvc->set_nJet(-2);
    } else {
      m_histNameSvc->set_nJet(m_physicsMeta.nJets);
    }
  }

  // Select region for histo filling
  //----------------------------
  // **RE-Definition** : Higgs, ZH - according to regime for hist filling
  TLorentzVector HVec, bbjVec;
  TLorentzVector VHVec;

  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    HVec = resolvedH.vec_corr;
    // if (m_physicsMeta.nJets > 2) bbjVec =  resolvedH.vec_corr +
    // j3_sel.vec_corr;
    if (m_physicsMeta.nSigJet >= 3 ||
        (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1))
      bbjVec = resolvedH.vec_corr + j3_sel.vec_corr;
    VHVec = resolvedVH.vec_corr;
  }

  //------------------------------------------------
  // store w/ / w/o PU weight as systematic variation
  // depends on what setting was chosen in config
  //------------------------------------------------

  if (!m_config->get<bool>("applyPUWeight"))
    m_weightSysts.push_back({"_withPU", (float)(m_pileupReweight)});

  /////////////////////////////////////////////////////////////
  // CorrsAndSyst, for resolved regime
  /////////////////////////////////////////////////////////////

  if (m_isMC &&
      ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    float cs_njet = (m_physicsMeta.nSigJet <= 3 ? m_physicsMeta.nSigJet : 3);
    // cs_njet = m_physicsMeta.nSigJet < 2 ? 2 : cs_njet;
    applyCS(metVec.Pt(), metVec.Pt(), cs_njet, m_physicsMeta.nTags, j1_sel.vec,
            j2_sel.vec);
  }

  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);

  /*bool isNominal = false;
  if (m_currentVar == "Nominal") isNominal = true;*/
  //-----------------------------
  // VJets Systematic Histograms
  //-----------------------------
  // NOTE :: mBB is in MeV because CorrsAndSysts uses MeV
  /*if ( !nominalOnly && (m_histNameSvc -> get_sample() == "W" || m_histNameSvc
    -> get_sample() == "Wv22"
    || m_histNameSvc -> get_sample() == "Z" || m_histNameSvc -> get_sample() ==
    "Zv22")
    && isNominal
    && ( m_model == Model::HVT || m_model == Model::AZh )
    && ( m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ||
    m_physicsMeta.regime == PhysicsMetadata::Regime::merged)) { double mBB =
    HVec.M(); float  mVH = m_physicsMeta.regime ==
    PhysicsMetadata::Regime::resolved ? VHVec.M() : -1; std::string Regime =
    m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ? "Resolved" :
    "Boosted";

    fillVjetsSystslep_PRSR( Regime, mBB, mVH/1e3); //mVH= GeV, mBB = MeV
    }*/
  // -----------------------------------------------
  // Internal weight variations Histograms
  // -----------------------------------------------
  if (m_evtWeightVarMode != -1 && m_currentVar == "Nominal" && m_isMC) {
    EL_CHECK("applyEvtWeights()", apply_EvtWeights());
  }

  //-----------------------------
  // TTbar Systematic Histograms
  //-----------------------------
  /*if ( m_histNameSvc -> get_sample() == "ttbar"
    && ( m_model == Model::HVT || m_model == Model::AZh )
    && ( m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ||
  m_physicsMeta.regime == PhysicsMetadata::Regime::merged) ) {

  if (!nominalOnly && isNominal){
  double mBB = HVec.M();
  float  mVH = m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ?
  VHVec.M() : -1; std::string Regime = m_physicsMeta.regime ==
  PhysicsMetadata::Regime::resolved ? "Resolved" : "Boosted"; float nAddBTags =
  m_physicsMeta.regime == PhysicsMetadata::Regime::resolved ? 0 :
  m_physicsMeta.nAddBTrkJets;

  fillTopSystslep_PRSR( Regime, nAddBTags, mBB, mVH/1e3);

  }
  }*/

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
  if (m_doCutflow) {
    static std::string cuts_common_resolved_Names[26] = {
        "AllCxAOD",       "Trigger",    "MET",
        "dphiMETMPT",     "Njets",      "Nsignaljets",
        "mindphiMETjets", "Sumpt",      "MPT",
        "ThirdBJetVeto",  "pTB1",       "dphiB1B2",
        "dphiMETB1B2",    "dRBB",       "0tag_2jet",
        "0tag_3jet",      "0tag_4pjet", "1tag_2jet",
        "1tag_3jet",      "1tag_4pjet", "2tag_2jet",
        "2tag_3jet",      "2tag_4pjet", "3ptag_2jet",
        "3ptag_3jet",     "3ptag_4pjet"};
    fill0LepCutFlow(eventFlag, cuts_common_resolved, cuts_common_resolved_Names,
                    26, 14);
  }

  // VHcc0Lep Resolved
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved &&
      (passSpecificCuts(eventFlag, cuts_common_resolved))) {
    // Fill MVA tree for the resolved analysis regime
    //-----------------------------------------------
    // fillMVATreeResolved( VHVec, HVec, metVec, mptVec, selectedJets, j1_sel,
    // j2_sel, j3_sel, sumpt, dRb1b2, dPhiMETMPT, dPhiMETdijetResolved, mindPhi,
    // metSig, metSig_PU, metSig_soft, metSig_hard, metOverSqrtSumET,
    // metOverSqrtHT, 0
    // );//the last para=0 means tau_size=0
    //    fillMVATreeResolved(
    //        VHVec, HVec, metVec, mptVec, selectedJets, j1_sel, j2_sel, j3_sel,
    //        sumpt, dRb1b2, dPhiMETMPT, dPhiMETdijetResolved, mindPhi, 0, 0, 0,
    //        0, 0, 0, 0);  // the last paras=0 means they are not used in this
    //        analysis

    m_tree->Reset();
    m_tree->SetVariation(m_currentVar);

    // **Calculate** : c-tagging SF
    std::map<std::string, std::map<int, float>> btag_TT_weights;
    bool do_btag_Syst =
        (!m_config->get<bool>("nominalOnly") && m_currentVar == "Nominal");

    if (m_isMC && selectedJets.size() > 1) {
      // Calculate direct tagging and truth tagging coefficients, re-weight them
      // if needed based on minimum dR corrections
      EL_CHECK("AnalysisReader_VHQQ::compute_TT_min_deltaR_correction",
               compute_TT_min_deltaR_correction(selectedJets, btag_TT_weights,
                                                do_btag_Syst));
      // This is the direct tagging weight
      m_weight *= btag_TT_weights["Nominal"][-1];
    }

    double direct_weight = btag_TT_weights["Nominal"][-1];
    fillMVATreeVHbbResolved0Lep(
        j1_sel.vec_corr, j2_sel.vec_corr, j3_sel.vec_corr, selectedJets,
        signalJets, forwardJets, HVec, metVec, met, taus.size(), direct_weight);

    // exclusive ntags
    // m_histNameSvc->set_nTag(m_physicsMeta.nTags, tagType);  // b-tag
    m_histNameSvc->set_nTag(m_physicsMeta.nTags);  // b-tag
    // if (m_physicsMeta.nTags == 2 && m_physicsMeta.nJets < 4) {

    if (m_isMC && m_histNameSvc->get_description() != "") {
      bool do_btag_Syst =
          (!m_config->get<bool>("nominalOnly") && m_currentVar == "Nominal");
      for (int n_truthtags = 0; n_truthtags < 3; n_truthtags++) {
        m_histNameSvc->set_nTag(n_truthtags);

        float weight = m_weight / btag_TT_weights["Nominal"][-1] *
                       btag_TT_weights["Nominal"][n_truthtags];

        int n_systs(0);

        if (do_btag_Syst) {
          // before filling the histogram, we need to place the
          // right variation value in m_weightSysts,
          // for every iteration, we need to replace the old values (for 0tag
          // for example) with the new values (for 1tag)
          for (auto effSF : btag_TT_weights) {
            std::string varName = effSF.first;
            if (varName == "Nominal") continue;
            m_weightSysts.push_back(
                {varName, btag_TT_weights[varName][n_truthtags] /
                              btag_TT_weights["Nominal"][n_truthtags]});
            n_systs++;  // count the systs so we can get rid of them after
                        // filling the histogram
          }
        }

        m_histSvc->BookFillHist("mBBTT", 100, 0, 500, m_tree->mBB, weight);
        //        m_histSvc->BookFillHist("EtaB1TT", 60, -3, 3,
        //                        m_tree->etaB1, weight);
        // m_histSvc->BookFillHist("EtaB2TT", 60, -3, 3,
        //                        m_tree->etaB2, weight);
        // m_histSvc->BookFillHist("pTVTT", 100, 0, 1000,
        //                        m_tree->pTV, weight);
        // m_histSvc->BookFillHist("pTB1TT", 200, 0, 2000,
        //                        m_tree->pTB1, weight);
        // m_histSvc->BookFillHist("pTB2TT", 150, 0, 1500,
        //                        m_tree->pTB2, weight);
        //        m_histSvc->BookFillHist("dRBBTT", 100, 0, 5,
        //                        m_tree->dRBB, weight);

        if (do_btag_Syst) {
          m_weightSysts.resize(m_weightSysts.size() - n_systs);
        }
      }

      m_histNameSvc->set_nTag(tagcatExcl);

      if (do_btag_Syst) {
        float dt_weight = btag_TT_weights["Nominal"][-1];
        for (auto effSF : btag_TT_weights) {
          std::string varName = effSF.first;
          if (varName == "Nominal") continue;
          m_weightSysts.push_back(
              {varName, btag_TT_weights[varName][-1] / dt_weight});
        }
      }
    } else if (m_isMC == false && m_histNameSvc->get_description() != "") {
      m_histSvc->BookFillHist("mBBTT", 100, 0, 500, m_tree->mBB, m_weight);
      /*
      m_histSvc->BookFillHist("EtaB1TT", 60, -3, 3,
                              m_tree->etaB1, m_weight);
      m_histSvc->BookFillHist("EtaB2TT", 60, -3, 3,
                              m_tree->etaB2, m_weight);
      m_histSvc->BookFillHist("pTVTT", 100, 0, 1000, m_tree->pTV,
                              m_weight);
      m_histSvc->BookFillHist("pTB1TT", 200, 0, 2000,
                              m_tree->pTB1, m_weight);
      m_histSvc->BookFillHist("pTB2TT", 150, 0, 1500,
                              m_tree->pTB2, m_weight);
      m_histSvc->BookFillHist("dRBBTT", 100, 0, 5, m_tree->dRBB,
                              m_weight);
      */
    }

    if (m_physicsMeta.nTags <= 2 && m_physicsMeta.nJets < 4) {
      if (m_doTruthTagging) {
        double total_weight = m_weight;
        for (int n_truthtags = 0; n_truthtags <= 2; ++n_truthtags) {
          m_histNameSvc->set_nTag(n_truthtags);
          double truth_tag_weight = btag_TT_weights["Nominal"][n_truthtags];
          m_weight = total_weight * truth_tag_weight / direct_weight;
          fill0LepHistResolved();
          // reset m_weight just in case
          m_weight = total_weight;
        }
      } else {
        fill0LepHistResolved();
      }
    } else if (!m_doOnlyInputs) {
      fill0LepHistResolved();
    }
    // inclusive ntags
    if (!m_doOnlyInputs) {
      m_histNameSvc->set_nTag(-1);  // b/c-tag
      fill0LepHistResolved();
    }

    // Fill Mbb inclusive plots for SM signal slice
    float SMSigVPTSlice = getSMSigVPTSlice();
    string Slice;
    if (SMSigVPTSlice > 0. && !m_doOnlyInputs) {
      if (SMSigVPTSlice < 150000.)
        Slice = "_0_150truthptv";
      else if (SMSigVPTSlice < 250000.)
        Slice = "_150_250truthptv";
      else
        Slice = "_250truthptv";
      m_histNameSvc->set_description(string("SR" + Slice));
      // exclusive ntags
      // m_histNameSvc->set_nTag(m_physicsMeta.nTags, tagType);  // b-tag
      m_histNameSvc->set_nTag(m_physicsMeta.nTags);  // b-tag
      fill0LepHistResolved();
      // inclusive ntags
      m_histNameSvc->set_nTag(-1);  // b-tag
      fill0LepHistResolved();
    }  // SMSigVPTSlice > 0. && !m_doOnlyInputs

    // Fill trees
    //-----------
    if (m_histNameSvc->get_isNominal()) {
      m_tree->Fill();

      bool writeEasyTree = false;
      m_config->getif<bool>("writeEasyTree", writeEasyTree);
      bool writeObjectsInEasyTree = false;
      m_config->getif<bool>("writeObjectsInEasyTree", writeObjectsInEasyTree);

      // if(writeEasyTree and (m_physicsMeta.nJets < 4) )
      if (writeObjectsInEasyTree || writeEasyTree) {
        fillETree();
        if (writeObjectsInEasyTree) {
          EL_CHECK("AnalysisReader_VHcc2Lep::run_0Lep_analysis()",
                   AnalysisReader_VHQQ::fillObjectBranches(
                       signalJets, m_electrons, m_muons, m_taus, m_met));
        }
        m_etree->Fill();
      }
    }
  }  // VHcc0Lep Resolved

  return EL::StatusCode::SUCCESS;
}  // run_0Lep_analysis

////////////////////////////////////////////////////////////
///        ** Fill cut flow for 0Lep selection **        ///
////////////////////////////////////////////////////////////
EL::StatusCode AnalysisReader_VHcc0Lep::fill0LepCutFlow(
    unsigned long int eventFlag, std::vector<unsigned long int> cuts,
    std::string cutsNames[], int length_cutsNames, int bin_ntag_njet) {
  // std::cout<<"eventFlag="<<eventFlag<<std::endl;
  std::string dir = "CutFlow/Nominal/";
  std::vector<unsigned long int> incrementCuts;

  for (unsigned long int i = 0; i < cuts.size(); ++i) {
    incrementCuts.push_back(cuts.at(i));

    if (!passSpecificCuts(eventFlag, incrementCuts)) break;
    std::string label = cutsNames[i];

    m_histSvc->BookFillCutHist(dir + "CutsSM", length_cutsNames, cutsNames,
                               label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsSMNoWeight", length_cutsNames,
                               cutsNames, label, 1.);
  }
  incrementCuts.clear();

  if (passSpecificCuts(eventFlag, cuts)) {
    if (m_physicsMeta.nTags == 0)
      bin_ntag_njet += 0;
    else if (m_physicsMeta.nTags == 1)
      bin_ntag_njet += 3;
    else if (m_physicsMeta.nTags == 2)
      bin_ntag_njet += 6;
    else if (m_physicsMeta.nTags > 2)
      bin_ntag_njet += 9;

    if (m_physicsMeta.nJets == 2) bin_ntag_njet += 0;
    if (m_physicsMeta.nJets == 3) bin_ntag_njet += 1;
    if (m_physicsMeta.nJets >= 4) bin_ntag_njet += 2;

    std::string label = cutsNames[bin_ntag_njet];
    m_histSvc->BookFillCutHist(dir + "CutsSM", length_cutsNames, cutsNames,
                               label, m_weight);
    m_histSvc->BookFillCutHist(dir + "CutsSMNoWeight", length_cutsNames,
                               cutsNames, label, 1.);
  }

  return EL::StatusCode::SUCCESS;
}  // fill0LepCutFlow

EL::StatusCode AnalysisReader_VHcc0Lep::fillETree() {
  // EasyTree reset
  m_etree->Reset();
  m_etree->SetVariation(m_currentVar);

  // keep type double for BDT's Spectator variables

  string Regime = "none";
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    Regime = "resolved";
  }

  m_etree->SetBranchAndValue<std::string>("Description",
                                          m_histNameSvc->get_description(), "");
  m_etree->SetBranchAndValue<std::string>("Regime", Regime, "none");
  m_etree->SetBranchAndValue<std::string>("Sample",
                                          m_histNameSvc->getFullSample(), "");

  m_etree->SetBranchAndValue<int>("ChannelNumber", m_tree->ChannelNumber, -99);
  m_etree->SetBranchAndValue<int>("RunNumber", m_eventInfo->runNumber(), -99);
  m_etree->SetBranchAndValue<double>("EventNumber", m_tree->EventNumber, -99.);
  m_etree->SetBranchAndValue<double>("EventNumberMod2",
                                     (m_tree->EventNumber) % 2, -99.);
  m_etree->SetBranchAndValue<double>("EventWeight", m_weight, -99.);

  m_etree->SetBranchAndValue<double>("AverageMu", m_averageMu, -99);
  m_etree->SetBranchAndValue<double>("ActualMu", m_actualMu, -99);
  m_etree->SetBranchAndValue<double>("AverageMuScaled", m_averageMuScaled, -99);
  m_etree->SetBranchAndValue<double>("ActualMuScaled", m_actualMuScaled, -99);

  m_etree->SetBranchAndValue<double>("EtaB1", m_tree->etaB1, -99.);
  m_etree->SetBranchAndValue<double>("EtaB2", m_tree->etaB2, -99.);
  m_etree->SetBranchAndValue<double>("EtaJ3", m_tree->etaJ3, -99.);
  m_etree->SetBranchAndValue<double>("PhiB1", m_tree->phiB1, -99.);
  m_etree->SetBranchAndValue<double>("PhiB2", m_tree->phiB2, -99.);
  m_etree->SetBranchAndValue<double>("PhiJ3", m_tree->phiJ3, -99.);
  m_etree->SetBranchAndValue<double>("pTB1", m_tree->pTB1, -99.);
  m_etree->SetBranchAndValue<double>("pTB2", m_tree->pTB2, -99.);
  m_etree->SetBranchAndValue<double>("pTJ3", m_tree->pTJ3, -99.);
  m_etree->SetBranchAndValue<double>("mB1", m_tree->mB1, -99.);
  m_etree->SetBranchAndValue<double>("mB2", m_tree->mB2, -99.);
  m_etree->SetBranchAndValue<double>("mJ3", m_tree->mJ3, -99.);

  m_etree->SetBranchAndValue<double>("Njets", m_tree->nJ, -99.);
  m_etree->SetBranchAndValue<int>("NTags", m_physicsMeta.nTags, -99);
  m_etree->SetBranchAndValue<int>("nTaus", m_tree->nTaus, -99);

  m_etree->SetBranchAndValue<double>("MET", m_tree->MET, -99.);
  //  m_etree->SetBranchAndValue<double>("MET_Track", m_tree->MPT / 1.e3, -99.);
  //  m_etree->SetBranchAndValue<double>("MET_Truth", truthMET, -99.);
  m_etree->SetBranchAndValue<double>("MEff", m_tree->MEff, -99.);

  //  m_etree->SetBranchAndValue<double>("SumPtJet", m_tree->sumPt / 1.e3,
  //  -99.);
  m_etree->SetBranchAndValue<double>("mBB", m_tree->mBB, -99.);
  m_etree->SetBranchAndValue<double>("mBBJ", m_tree->mBBJ, -99.);

  //  m_etree->SetBranchAndValue<double>("pTBB", m_tree->pTBB / 1.e3, -99.);
  m_etree->SetBranchAndValue<double>("dRBB", m_tree->dRBB, -99.);
  //  m_etree->SetBranchAndValue<double>("yBB", m_tree->yBB, -99.);
  m_etree->SetBranchAndValue<double>("dPhiBB", m_tree->dPhiBB, -99.);
  m_etree->SetBranchAndValue<double>("dEtaBB", m_tree->dEtaBB, -99.);

  //  m_etree->SetBranchAndValue<double>("dRB1J3", m_tree->dRB1J3, -99.);
  //  m_etree->SetBranchAndValue<double>("dRB2J3", m_tree->dRB2J3, -99.);
  //  m_etree->SetBranchAndValue<double>("mindRBJ3", m_tree->mindRBJ3, -99.);
  //  m_etree->SetBranchAndValue<double>("maxdRBJ3", m_tree->maxdRBJ3, -99.);

  //  m_etree->SetBranchAndValue<double>("MindPhiMETJet", m_tree->mindPhi,
  //  -99.);

  m_etree->SetBranchAndValue<double>("dPhiVBB", m_tree->dPhiVBB,
                                     -99.);  // mva input
  //  m_etree->SetBranchAndValue<double>("pTBBoverMET", m_tree->pTBBoverMET,
  //  -99.); m_etree->SetBranchAndValue<double>("pTBBMETAsym",
  //  m_tree->pTBBMETAsym, -99.);
  //  m_etree->SetBranchAndValue<double>("dPhiMETMPT", m_tree->dPhiMETMPT,
  //  -99.);

  m_etree->SetBranchAndValue<int>("flavB1", m_physicsMeta.b1Flav, -99);
  m_etree->SetBranchAndValue<int>("flavB2", m_physicsMeta.b2Flav, -99);
  m_etree->SetBranchAndValue<int>("flavJ3", m_physicsMeta.j3Flav, -99);
  // m_etree->SetBranchAndValue<double>("MV2c10B1", m_tree -> MV1cB1,  -99.);
  // m_etree->SetBranchAndValue<double>("MV2c10B2", m_tree -> MV1cB2,  -99.);
  // m_etree->SetBranchAndValue<double>("MV2c10J3", m_tree -> MV1cJ3,  -99.);
  // for (auto bdt:m_tree->BDTs) { m_etree->SetBranchAndValue<double>
  // (bdt.first, bdt.second, -99); }

  return EL::StatusCode::SUCCESS;
}
