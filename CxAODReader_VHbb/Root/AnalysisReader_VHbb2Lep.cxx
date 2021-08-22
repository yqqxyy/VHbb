#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader/OSTree.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ2Lep.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb2lep.h"
#include "CxAODTools_VHbb/VHbb2lepEvtSelection.h"
#include "KinematicFit/KinematicFit.h"

#include <CxAODReader_VHbb/AnalysisReader_VHbb2Lep.h>
#include <TMVA/Reader.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
#include "TXMLEngine.h"

#include <iomanip>

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHbb2Lep::AnalysisReader_VHbb2Lep() : AnalysisReader_VHQQ2Lep() {
  m_doMergeModel = false;
  m_isSLEvent = false;
}

AnalysisReader_VHbb2Lep::~AnalysisReader_VHbb2Lep() {}

EL::StatusCode AnalysisReader_VHbb2Lep::run_2Lep_analysis() {
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
    // 410646(7): PP8 inclusive single (anti-)top Wt
    if (m_mcChannel == 410646 || m_mcChannel == 410647) {
      int veto = false;
      if (m_CxAODTag == "CxAODTag31") {
        veto = vetoDilepWtEvents();
      } else if (m_CxAODTag >= "CxAODTag32") {
        // to use the TTBarDecay from CxAOD, so it is the same function as for
        // the TTbar
        veto = vetoDilepTtbarEvents();
      }
      if (veto == -1) {
        if (m_CxAODTag == "CxAODTag31") {
          Error("run_2Lep_analysis()",
                "CxAODTag31, something went wrong evaluating if STopWt is "
                "dilep or not, but not using Props::codeTTBarDecay!");
        } else if (m_CxAODTag >= "CxAODTag32") {
          Error("run_2Lep_analysis()",
                "CxAODTag32, something went wrong evaluating if STopWt is "
                "dilep, as Props::codeTTBarDecay doesn't exist!");
        }
        return EL::StatusCode::FAILURE;
      } else if (veto == 1) {
        return EL::StatusCode::SUCCESS;
      }
    }
  }
  //----------------------------

  // reset physics metadata
  m_physicsMeta = PhysicsMetadata();

  // Initialize eventFlag - all cuts are initialized to false by default
  unsigned long int eventFlag = 0;

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
  std::vector<const xAOD::Jet *> subJets = selectionResult.subJets;
  std::vector<const xAOD::TauJet *> taus = selectionResult.taus;
  setevent_nJets(signalJets, forwardJets, fatJets);

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
  // These vectors contain respectively all the track jets matched to the
  // leading, sub-leading, sub-sub-leading fat jets (independent of whether or
  // not any of these track jets are b-tagged)
  std::vector<const xAOD::Jet *> trackJetsInFatJet1, trackJetsInFatJet2,
      trackJetsInFatJet3;
  //
  // These vectors containing respectively all the track jets outside the
  // leading, sub-leading, sub-sub-leading fat jets (independent of whether or
  // not any of these track jets are b-tagged)
  std::vector<const xAOD::Jet *> trackJetsNotInFatJet1, trackJetsNotInFatJet2,
      trackJetsNotInFatJet3;
  // Additional vectors used to compute the b-tagging weights
  std::vector<const xAOD::Jet *> trackJetsForBTagSF, bTaggedUnmatchedTrackJets;

  // TODO: Harmonize large-R jet / track-jet association with resonant analysis
  if (m_doTruthTagging) {
    if (!(m_analysisStrategy == "Merged"))  //  For merged computed later
      compute_TRF_tagging(signalJets);
  } else {
    compute_btagging();
  }
  //
  if (fatJets.size() == 0) {  //  TT for boosted: necessary to run b-tagging in
                              //  TRF mode on all jets
    for (auto trackJet : trackJets) trackJetsNotInFatJet1.push_back(trackJet);
  }

  if (fatJets.size() >= 1)
    matchTrackJetstoFatJets(trackJets, fatJets.at(0), &trackJetsInFatJet1,
                            &trackJetsNotInFatJet1);
  if (fatJets.size() >= 2)
    matchTrackJetstoFatJets(trackJets, fatJets.at(1), &trackJetsInFatJet2,
                            &trackJetsNotInFatJet2);
  if (fatJets.size() >= 3)
    matchTrackJetstoFatJets(trackJets, fatJets.at(2), &trackJetsInFatJet3,
                            &trackJetsNotInFatJet3);

  if (m_doTruthTagging) {
    if (m_analysisStrategy != "Resolved")
      compute_TRF_tagging(trackJetsInFatJet1, trackJetsNotInFatJet1, true);
    // TT on trackJetsInFatJet1,
    // DT on trackJetsNotInFatJet1,
    // true = these are trackjets
  }

  int boostedtagcatExcl = -1;
  int nAddBTags = -1;
  // subset of the track jets matched to the LFJ, it depends on the tag strategy
  std::vector<const xAOD::Jet *> selectedtrackJetsInFatJet;
  selectedtrackJetsInFatJet.clear();
  if (fatJets.size() >= 1) {
    tagTrackjet_selection(fatJets.at(0), trackJetsInFatJet1,
                          trackJetsNotInFatJet1, selectedtrackJetsInFatJet,
                          bTaggedUnmatchedTrackJets, trackJetsForBTagSF,
                          boostedtagcatExcl, nAddBTags);
  }
  if (fatJets.size() >= 2)
    set_BtagProperties_fatJets(fatJets.at(1), trackJetsInFatJet2,
                               trackJetsNotInFatJet2);
  if (fatJets.size() >= 3)
    set_BtagProperties_fatJets(fatJets.at(2), trackJetsInFatJet3,
                               trackJetsNotInFatJet3);

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
            "DR matching is not available for CoM. \n Select"
            "Select forceTrackJetFRmatching as false to allow Ghost "
            "Association \n"
            "Exiting!");
      return EL::StatusCode::FAILURE;
    } else {
      CoMtagging_LeadFJ(subJets, fatJets.at(0), selectedCoMsubjets,
                        boostedtagcatExcl);
    }
  }

  /////////////////////////////////////////////////////////////
  // **Definition** : selected jets
  int tagcatExcl = -1;
  std::vector<const xAOD::Jet *> selectedJets;
  selectedJets.clear();

  if (m_analysisStrategy == "Resolved") {
    // removing the possibility of tagging R04 for merged
    tagjet_selection(signalJets, forwardJets, selectedJets, tagcatExcl);
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
  if ((m_physicsMeta.nFatJet) > 0) {
    // number of lead / sublead b-tagged track jets associated to leading fat
    // jet
    m_physicsMeta.nTagsInFJ = Props::nBTags.get(fatJets.at(0));
    // number of b-tagged track jets not associated to leading fat jet
    m_physicsMeta.nAddBTrkJets = Props::nAddBTags.get(fatJets.at(0));
  }
  // number of b-tagged track jets
  int nTaggedTrkJets = 0;
  for (auto jet : trackJets) {
    if (BTagProps::isTagged.get(jet)) nTaggedTrkJets++;
  }
  //
  // define jets for b-tagging weight computation and event labelling
  //------------------------------------------------------------------
  if (m_physicsMeta.nFatJet > 0) {
    m_physicsMeta.nTagsInFJ = boostedtagcatExcl;
    m_physicsMeta.nAddBTrkJets = nAddBTags;
  }

  // Set pTV for FSR
  double ptv_for_FSR = ZVec.Pt() / 1e3;

  //
  // define jets to be used for mBB + possible additional jet
  //---------------------------------------------------------
  Jet j1, j2, j3;     // small-R jets
  Jet fj1, fj2, fj3;  // fat jets
  EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis()",
           setJetVariables(j1, j2, j3, selectedJets, ptv_for_FSR));
  EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis()",
           setFatJetVariables(fj1, fj2, fj3, fatJets));

  m_isSLEvent = false;
  if (j1.isMu || j2.isMu) {
    m_isSLEvent = true;
  }

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

  TLorentzVector metVec;
  metVec.SetPtEtaPhiM(met->met(), 0, met->phi(), 0);
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
  if (m_doKF) {
    if (m_analysisStrategy == "Resolved") {
      int numberOfJets = selectedJets.size();
      if (m_doFSRrecovery && m_hasFSR > 0.5) numberOfJets--;
      if ((m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl ||
           m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu ||
           m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu ||
           m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) &&
          ((m_physicsMeta.nJets == 2) || (m_physicsMeta.nJets == 3)) &&
          m_physicsMeta.nTags == 2) {
        EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis()",
                 getKFResolved(numberOfJets, j1, j2, j3, l1, l2));
        if (m_model == Model::CUT || m_model == Model::MVA) {
          j1.vec_corr = j1.vec_kf;
          j2.vec_corr = j2.vec_kf;
        }
      }
      resolvedH.vec_kf = j1.vec_kf + j2.vec_kf;
    } else if (m_analysisStrategy == "Merged") {
      if ((m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElEl ||
           m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuMu ||
           m_physicsMeta.flavor == PhysicsMetadata::Flavor::ElMu ||
           m_physicsMeta.flavor == PhysicsMetadata::Flavor::MuEl) &&
          (m_physicsMeta.nFatJet > 0))
        EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis()",
                 getKFMerged(m_physicsMeta.nJets, fj1, l1, l2, signalJets,
                             forwardJets));
      fj1.vec_corr = fj1.vec_kf;
    } else {
      Warning("AnalysisReader_VHbb2Lep::run_2Lep_analysis()",
              "Currently KF is only used in Resolved or Merged strategy.");
    }
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

  // **Selection & SF** : trigger
  //----------------------------
  double triggerSF = 1.;
  if (pass2LepTrigger(triggerSF, selectionResult))
    updateFlag(eventFlag, DiLeptonCuts::Trigger);
  if (m_isMC) m_weight *= triggerSF;

  // **Selection** : same flavor
  //----------------------------
  if (isMu || isE)
    updateFlag(eventFlag, DiLeptonCuts::SFLeptons);
  else
    updateFlag(eventFlag, DiLeptonCuts::DFLeptons);

  if (m_isMC) m_weight *= Props::leptonSF.get(m_eventInfo);

  // **Selection** : cut on lepton pT
  //------------------------------------
  if (l1.vec.Pt() / 1000. >= 27.) {
    updateFlag(eventFlag, DiLeptonCuts::LeadLepPt27);
    if (l2.vec.Pt() / 1000. >= 25.)
      updateFlag(eventFlag, DiLeptonCuts::pTBothLeptons25);
  }

  // **Selection** : cut on lepton charge and muon eta
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

  // **Selection** : Mll cut
  //-----------------------------
  if ((ZVec.M() / 1000. >= 81.) && (ZVec.M() / 1000. < 101.))
    updateFlag(eventFlag, DiLeptonCuts::MllZwindowResolved);
  if ((ZVec.M() / 1000. >= 66.) && (ZVec.M() / 1000. < 116.))
    updateFlag(eventFlag, DiLeptonCuts::MllZwindowMerged);

  if (ZVec.M() / 1.e3 > 40.) updateFlag(eventFlag, DiLeptonCuts::MllAbove40);

  // **Selection** : Delta lepton pt cut
  //-------------------------------------
  if ((l1.vec.Pt() - l2.vec.Pt()) / ZVec.Pt() < 0.8)
    updateFlag(eventFlag, DiLeptonCuts::DLepPt);

  // **Selection** : METHT cut
  //----------------------------------
  if (m_model == Model::CUT || m_model == Model::MVA) {
    if (METHTsmallR / sqrt(1000.) < 3.5) {
      updateFlag(eventFlag, DiLeptonCuts::METHTResolved);
    }  // METHT cut is applied only in resolved analysis
    updateFlag(eventFlag, DiLeptonCuts::METHTMerged);
  }

  // **Selection** : cut on nJet
  //-----------------------------------
  if (m_physicsMeta.nJets >= 2)
    updateFlag(eventFlag, DiLeptonCuts::AtLeast2Jets);

  // **Selection** : cut on nSignalJet
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

  // **Selection** : pt(leading jet) >= 45 GeV
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
  if ((m_model == Model::CUT || m_model == Model::MVA) &&
      selectedJets.size() > 1 && j1.vec_onemu.DeltaR(j2.vec_onemu) < dRCut) {
    updateFlag(eventFlag, DiLeptonCuts::dRCut);
  }

  //  **Selection** : mbb w/o mu-in-jet (+ pT reco) corr
  //------------------------------------------
  bool passMHResolved(false), passMHMerged(false), inMHSideBandResolved(false),
      inMHSideBandMerged(false);
  checkMbbWindow(resolvedH.vec, mergedH.vec, passMHResolved, passMHMerged,
                 inMHSideBandResolved, inMHSideBandMerged);

  if (passMHResolved) updateFlag(eventFlag, DiLeptonCuts::mHNoCorrResolved);
  if (passMHMerged) updateFlag(eventFlag, DiLeptonCuts::mHNoCorrMerged);

  // mbb w/ mu-in-jet (+ pT reco) corr
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

  //  **Selection** : jet multiplicity cuts
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
  // Selection for Boosted Analysis
  if (m_physicsMeta.nFatJet > 0) {
    // **Selection** : at least 1 fat jet
    //-----------------------------------------
    updateFlag(eventFlag, DiLeptonCuts::AtLeast1FJ);

    // **Selection** : checking overlap of VR track jets
    //-----------------------------------------
    if (!m_doVRJetOR) {
      updateFlag(eventFlag, DiLeptonCuts::passVRJetOR);
    } else if (passVRJetOR(trackJetsForBTagSF)) {
      updateFlag(eventFlag, DiLeptonCuts::passVRJetOR);
    }

    // **Selection** : at least 1 track jet in Fat jet
    //-----------------------------------------
    if (trackJetsInFatJet1.size() >= 1) {
      updateFlag(eventFlag, DiLeptonCuts::LeadFJ1pTJ);
    }

    // **Selection** : at least 2 track jet in Fat jet
    // CoM always has 2 subjets
    //-----------------------------------------
    if (!doCoMtagging) {
      if (trackJetsInFatJet1.size() >= 2)
        updateFlag(eventFlag, DiLeptonCuts::LeadFJ2TJ);
    } else {
      updateFlag(eventFlag, DiLeptonCuts::LeadFJ2TJ);
    }

    // **Selection** : 2 btag track jets
    //-----------------------------------------
    nFatJetTags = Props::nBTags.get(fatJets.at(0));
    if (nFatJetTags == 2) updateFlag(eventFlag, DiLeptonCuts::LeadFJ2b);

    // **Selection** : fat jet mass (before correction) > 50GeV
    //----------------------------------------------------------
    if (fj1.vec.M() / 1e3 > 50) updateFlag(eventFlag, DiLeptonCuts::MLeadFJ50);

    // **Selection** : DY(V,H)
    //-------------------------------------
    if (fabs(ZVec.Rapidity() - fj1.vec.Rapidity()) < 1.4)
      updateFlag(eventFlag, DiLeptonCuts::DYVH);
  }

  // FSR: after the selection, migrate FSR events
  if (m_doFSRrecovery && m_hasFSR > 0.5)
    migrateCategoryFSR(signalJets, forwardJets);

  /////////////////////////////////////////////////////////////
  //  **Fill cutflow** :
  EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
           fill_2lepCutFlow(eventFlag, m_physicsMeta.nJets, tagcatExcl,
                            nFatJetTags, isMu, isE));

  ///////////////////////////////////////////////////
  // HISTO FILLING before categorisations        ///
  ///////////////////////////////////////////////////
  if (!m_doOnlyInputs) {
    if (m_analysisStrategy != "Merged")
      EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
               fill_nJetHistos(signalJets, "Sig"));
    if (!m_doReduceFillHistos)
      EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
               fill_nJetHistos(forwardJets, "Fwd"));
    if (m_analysisStrategy == "Merged")
      EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
               fill_nJetHistos(fatJets, "Fat"));
    if (!m_doReduceFillHistos)
      EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
               fill_nJetHistos(trackJets, "Trk"));
  }

  ///////////////////////////////////////////////////
  // DEFINITION OF CUTS FOR DIFFERENT REGIONS     ///
  ///////////////////////////////////////////////////

  // --- Cuts for resolved analysis --- //
  std::vector<unsigned long int> cuts_common_resolved = {};
  std::vector<unsigned long int> cuts_SR_resolved = {};
  std::vector<unsigned long int> cuts_top_resolved = {};

  cuts_common_resolved = {
      DiLeptonCuts::LeadLepPt27,       DiLeptonCuts::Trigger,
      DiLeptonCuts::OSLeptons,         DiLeptonCuts::AtLeast2Jets,
      DiLeptonCuts::AtLeast2SigJets,   DiLeptonCuts::PtB145,
      DiLeptonCuts::MllZwindowResolved};
  cuts_SR_resolved = {DiLeptonCuts::SFLeptons};

  // addition cuts for Cut based analysis
  std::vector<unsigned long int> cuts_common_resolved_CUT = {
      DiLeptonCuts::dRCut};
  std::vector<unsigned long int> cuts_SR_resolved_CUT = {
      DiLeptonCuts::METHTResolved};
  if (m_model == Model::CUT) {
    cuts_common_resolved.insert(cuts_common_resolved.end(),
                                cuts_common_resolved_CUT.begin(),
                                cuts_common_resolved_CUT.end());
    cuts_SR_resolved.insert(cuts_SR_resolved.end(),
                            cuts_SR_resolved_CUT.begin(),
                            cuts_SR_resolved_CUT.end());
  }

  // SR = SR-specific + common
  cuts_SR_resolved.insert(cuts_SR_resolved.end(), cuts_common_resolved.begin(),
                          cuts_common_resolved.end());
  // topCR = top-specific + common
  cuts_top_resolved.insert(cuts_top_resolved.end(),
                           cuts_common_resolved.begin(),
                           cuts_common_resolved.end());
  // mBBcr: remnant from resonant VHbb, not used in SM VHbb analysis.
  // Keep the mBBcr variable here for Priority Regime
  std::vector<unsigned long int> cuts_mBBcr_resolved = {};
  cuts_mBBcr_resolved.insert(cuts_mBBcr_resolved.end(),
                             cuts_SR_resolved.begin(), cuts_SR_resolved.end());

  // --- Cuts for merged analysis --- //
  std::vector<unsigned long int> cuts_common_merged = {};
  std::vector<unsigned long int> cuts_SR_merged = {};
  std::vector<unsigned long int> cuts_top_merged = {};

  cuts_common_merged = {DiLeptonCuts::LeadLepPt27,
                        DiLeptonCuts::Trigger,
                        DiLeptonCuts::OSLeptons,
                        DiLeptonCuts::AtLeast1FJ,
                        DiLeptonCuts::passVRJetOR,
                        DiLeptonCuts::LeadFJ2TJ,
                        DiLeptonCuts::MllZwindowMerged,
                        DiLeptonCuts::DLepPt,
                        DiLeptonCuts::DYVH};
  cuts_SR_merged = {DiLeptonCuts::SFLeptons, DiLeptonCuts::MLeadFJ50};
  cuts_top_merged = {DiLeptonCuts::MLeadFJ50};

  // SR = SR-specific + common
  cuts_SR_merged.insert(cuts_SR_merged.end(), cuts_common_merged.begin(),
                        cuts_common_merged.end());
  // topCR = top-specific + common
  cuts_top_merged.insert(cuts_top_merged.end(), cuts_common_merged.begin(),
                         cuts_common_merged.end());
  // mBBcr is not used in SM VHbb analysis, only used for Priority Regime
  std::vector<unsigned long int> cuts_mBBcr_merged = {};
  cuts_mBBcr_merged.insert(cuts_mBBcr_merged.end(), cuts_SR_merged.begin(),
                           cuts_SR_merged.end());

  /////////////////////////////////////////////////////////////
  //      DECISION TO USE MERGED OR RESOLVED ANALYSIS        //
  /////////////////////////////////////////////////////////////
  selectRegime(eventFlag, ZVec.Pt(), cuts_SR_resolved, cuts_SR_merged,
               cuts_mBBcr_resolved, cuts_mBBcr_merged, cuts_top_resolved,
               cuts_top_merged);
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::undefined) {
    // Warning("AnalysisReader_VHbb2Lep::run_2Lep_analysis()",
    //"Skipping undefined regime with eventFlag %lu", eventFlag);
    return EL::StatusCode::SUCCESS;
  }

  /////////////////////////////////////////////////////////////
  //         EVENT CATEGORIZATION BASED ON BIT FLAG          //
  /////////////////////////////////////////////////////////////

  // choose right analysis model
  //---------------------------
  if (m_model == Model::MVA) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::MVA);
  } else if (m_model == Model::CUT) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
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
    if (!doCoMtagging)
      setevent_flavour(selectedtrackJetsInFatJet);
    else
      setevent_flavour(selectedCoMsubjets);

    if (m_doMergeModel)
      m_histNameSvc->set_nTrackJetInFatJet(trackJetsInFatJet1.size());
    m_histNameSvc->set_nTag(m_physicsMeta.nTagsInFJ);
    m_histNameSvc->set_nFatJet(m_physicsMeta.nFatJet);
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

      if (m_doNewRegions) {
        if (m_physicsMeta.nJets == 2) {
          float dRBB_min = m_fit_func_2j10->Eval(ZVec.Pt() * 1e-03);
          float dRBB_max = m_fit_func_2j95->Eval(ZVec.Pt() * 1e-03);

          if (j1.vec.DeltaR(j2.vec) > dRBB_max)
            m_histNameSvc->set_description("CRHigh");
          else if (j1.vec.DeltaR(j2.vec) < dRBB_min)
            m_histNameSvc->set_description("CRLow");
          else
            m_histNameSvc->set_description("SR");

        }

        else if (m_physicsMeta.nJets >= 3) {
          float dRBB_min = m_fit_func_3j10->Eval(ZVec.Pt() * 1e-03);
          float dRBB_max = m_fit_func_3j85->Eval(ZVec.Pt() * 1e-03);

          if (j1.vec.DeltaR(j2.vec) > dRBB_max)
            m_histNameSvc->set_description("CRHigh");
          else if (j1.vec.DeltaR(j2.vec) < dRBB_min)
            m_histNameSvc->set_description("CRLow");
          else
            m_histNameSvc->set_description("SR");
        }
      }
    } else if (passSpecificCuts(eventFlag, cuts_top_resolved) &&
               !passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons})) {
      m_physicsMeta.region = PhysicsMetadata::Region::topCR;
      m_histNameSvc->set_description("topemucr");

      if (m_doNewRegions) {
        if (m_physicsMeta.nJets == 2) {
          float dRBB_min = m_fit_func_2j10->Eval(ZVec.Pt() * 1e-03);
          float dRBB_max = m_fit_func_2j95->Eval(ZVec.Pt() * 1e-03);

          if (j1.vec.DeltaR(j2.vec) > dRBB_max)
            m_histNameSvc->set_description("topemucrHigh");
          else if (j1.vec.DeltaR(j2.vec) < dRBB_min)
            m_histNameSvc->set_description("topemucrLow");
          else
            m_histNameSvc->set_description("topemucr");

        }

        else if (m_physicsMeta.nJets >= 3) {
          float dRBB_min = m_fit_func_3j10->Eval(ZVec.Pt() * 1e-03);
          float dRBB_max = m_fit_func_3j85->Eval(ZVec.Pt() * 1e-03);

          if (j1.vec.DeltaR(j2.vec) > dRBB_max)
            m_histNameSvc->set_description("topemucrHigh");
          else if (j1.vec.DeltaR(j2.vec) < dRBB_min)
            m_histNameSvc->set_description("topemucrLow");
          else
            m_histNameSvc->set_description("topemucr");
        }
      }
    }
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    if (passSpecificCuts(eventFlag, cuts_SR_merged)) {
      m_histNameSvc->set_description("SR");
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
    VHVec = resolvedVH.vec_corr;
  } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    HVec = mergedH.vec_corr;
    if (m_physicsMeta.nFatJet > 1) bbjVec = mergedH.vec_corr + j2.vec_corr;
    bool rescaleFatJetMass = false;
    m_config->getif<bool>("rescaleFatJetMass", rescaleFatJetMass);
    if (rescaleFatJetMass && m_histNameSvc->get_description() == "SR") {
      VHVec = mergedVH.vec_resc;
    } else
      VHVec = mergedVH.vec_corr;
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
      btagWeight = computeBTagSFWeight(trackJetsForBTagSF,
                                       m_trackJetReader->getContainerName());
      // To be added once CoM is fully calibrated
      // if (doCoMtagging)
      // btagWeight *= computeBTagSFWeight(selectedCoMsubjets,
      //                                m_subJetReader->getContainerName());
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
  //    if ( m_physicsMeta.nFatJet > 0 )
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

  /////////////////////////////////////////////////////////////
  // **CorrsAndSyst**:
  // resolved
  if (m_isMC && (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) &&
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
    EL_CHECK("applyCS",
             applyCS(cs_vpt, cs_mbb, cs_truthPt, cs_dphi, cs_dr, cs_ptb1,
                     cs_ptb2, cs_met, cs_avgTopPt, cs_njet, cs_ntag));
  } else if (m_isMC &&
             (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) &&
             (selectedtrackJetsInFatJet.size() > 1) &&
             passSpecificCuts(eventFlag, cuts_common_merged) &&
             passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons}) &&
             ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    EL_CHECK("applyCS()", applyCS(ZVec.Pt(), HVec.M(), -1));
  }

  // -----------------------------------------------------------------
  // DEPRECATED
  // Sherpa weight variations -> input for modelling syst
  // will only be run if appropriate syst are added to csVariations
  //(MUR0.5_MUF1_PDF261000 MUR2_MUF1_PDF261000 MUR1_MUF0.5_PDF261000
  // MUR1_MUF2_PDF261000 MUR0.5_MUF0.5_PDF261000 MUR2_MUF2_PDF261000
  // MUR1_MUF1_PDF25300 MUR1_MUF1_PDF13000)
  // bool nominalOnly = false;
  // m_config->getif<bool>("nominalOnly", nominalOnly);
  // if (!nominalOnly && m_currentVar == "Nominal" && m_isMC)
  //   applySherpaVJet_EvtWeights();
  // -----------------------------------------------------------------

  //-----------------------------------------------
  // Internal weight variations Histograms
  //-----------------------------------------------
  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);
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

  // Elisabeth: determine if an event belongs to the signal region / the control
  // regions
  // -> only fill MVATree for the events in the signal region
  // -> also need BDT scores in the top control region
  bool isSRevent = false;
  bool isTopCRevent = false;
  bool passCUTselection = false;

  // if (passSpecificCuts(eventFlag, cuts_SR_resolved)) isSRevent = true;
  // if (passSpecificCuts(eventFlag, cuts_top_resolved) &&
  //    !passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons}))
  //  isTopCRevent = true;
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
    if (passSpecificCuts(eventFlag, cuts_SR_resolved)) {
      isSRevent = true;
      if (passSpecificCuts(eventFlag, cuts_common_resolved_CUT) &&
          passSpecificCuts(eventFlag, cuts_SR_resolved_CUT))
        passCUTselection = true;
    } else if (passSpecificCuts(eventFlag, cuts_top_resolved) &&
               !passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons})) {
      isTopCRevent = true;
      if (passSpecificCuts(eventFlag, cuts_common_resolved_CUT))
        passCUTselection = true;
    }
  }  // resolved
  if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
    if (passSpecificCuts(eventFlag, cuts_SR_merged))
      isSRevent = true;
    else if (passSpecificCuts(eventFlag, cuts_top_merged) &&
             !passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons}))
      isTopCRevent = true;
  }

  bool writeMVATree = false;
  bool writeEasyTree = false;
  m_config->getif<bool>("writeMVATree", writeMVATree);
  m_config->getif<bool>("writeEasyTree", writeEasyTree);
  if (m_physicsMeta.nSigJet >= 2 || m_physicsMeta.nFatJet >= 1) {
    // MVA tree
    m_tree->Reset();
    m_tree->SetVariation(m_currentVar);
    TLorentzVector b1, b2;
    if (j1.isTagged && !j2.isTagged) {  // 1 b-tagged => bjet is a first jet.
      b1 = j1.vec_corr;
      b2 = j2.vec_corr;
    } else if (!j1.isTagged && j2.isTagged) {
      b1 = j2.vec_corr;
      b2 = j1.vec_corr;
    } else {  // 0 or 2 b-tagged => order by their pT.
      if (j1.vec.Pt() > j2.vec.Pt() || !j2.isTagged) {
        b1 = j1.vec_corr;
        b2 = j2.vec_corr;
      } else {
        b1 = j2.vec_corr;
        b2 = j1.vec_corr;
      }
    }

    if (isSRevent || isTopCRevent || writeEasyTree) {
      EL_CHECK("fillMVATreeVHbbResolved2Lep",
               fillMVATreeVHbbResolved2Lep(
                   b1, b2, j3.vec, selectedJets, signalJets, forwardJets, HVec,
                   ZVec, metVec, met, l1, l2, taus.size(), btagWeight));

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
    }

    // ETree reset
    m_etree->Reset();
    m_etree->SetVariation(m_currentVar);

    // fill ETree
    if (writeEasyTree || !m_doOnlyInputs) {
      // skip easy tree filling if we only want inputs plots
      (*m_etree)["L1Vec"] = l1.vec;
      (*m_etree)["L2Vec"] = l2.vec;
      (*m_etree)["ZVec"] = ZVec;
      (*m_etree)["HVec"] = HVec;
      (*m_etree)["VHVec"] = VHVec;
      (*m_etree)["HVecPtReco"] = HVecPtReco;
      (*m_etree)["resolvedVH"] = resolvedVH.vec_resc;
      (*m_etree)["resolvedH"] = resolvedH.vec_corr;
      (*m_etree)["lbVec"] = lbVec;
      (*m_etree)["bbjVec"] = bbjVec;

      EL_CHECK("fillETreeCommon",
               fillETreeCommon(triggerSF, JVTweight, btagWeight));
      EL_CHECK("fillETreeSelectedJets", fillETreeSelectedJets(selectedJets));
      EL_CHECK("fillETreeLepton", fillETreeLepton(l1, l2));
      EL_CHECK("fillETreeResolved",
               fillETreeResolved(j1, j2, HTsmallR, METHTsmallR));
      if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged ||
          writeEasyTree) {
        if (m_physicsMeta.nFatJet >= 1)
          (*m_etree)["fatj1Vec"] = fatJets.at(0)->p4();
        if (m_physicsMeta.nFatJet >= 2)
          (*m_etree)["fatj2Vec"] = fatJets.at(1)->p4();
        if (m_physicsMeta.nFatJet >= 3)
          (*m_etree)["fatj3Vec"] = fatJets.at(2)->p4();
        if (trackJets.size() >= 1)
          (*m_etree)["trkj1Vec"] = trackJets.at(0)->p4();
        if (trackJets.size() >= 2)
          (*m_etree)["trkj2Vec"] = trackJets.at(1)->p4();
        if (trackJets.size() >= 3)
          (*m_etree)["trkj3Vec"] = trackJets.at(2)->p4();
        if (trackJetsInFatJet1.size() >= 1) {
          (*m_etree)["trkj1LeadFJVec"] = trackJetsInFatJet1.at(0)->p4();
          if (trackJetsInFatJet1.size() >= 2) {
            (*m_etree)["trkj1LeadFJVec"] = trackJetsInFatJet1.at(1)->p4();
          }
        }
        int ntrackjets = 0;
        if (fatJets.size() > 0)
          ntrackjets = Props::nTrackJets.get(fatJets.at(0));
        EL_CHECK(
            "fillETreeMerged",
            fillETreeMerged(mergedVH, mergedH, trackJets.size(), nTaggedTrkJets,
                            ntrackjets, HTlargeR, METHTlargeR));
      }
      // internal weight
      if (m_evtWeightVarMode == 0 && m_isMC) {
        // add internal weights to easy tree only for essentials to avoid
        // massive trees
        std::unordered_set<std::string> intVars =
            m_EvtWeightVar->getListOfVariations(m_mcChannel);
        for (auto &csVar : intVars) {
          // Extract the alternative weight
          float AltWeight = 1.0;
          if ((m_tree->sample == "Zbb" || m_tree->sample == "Zbc" ||
               m_tree->sample == "Zbl" || m_tree->sample == "Zcc" ||
               m_tree->sample == "Zcl" || m_tree->sample == "Zl"))
            AltWeight = m_EvtWeightVar->getWeightVariation(m_eventInfo, csVar);
          m_etree->SetBranchAndValue<float>("Sys__" + csVar, AltWeight, -999.0);
        }
      }

      // Mbb monitor
      if (m_doMbbMonitor) {
        Higgs thisHiggs;
        if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
          thisHiggs = resolvedH;
        } else if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged) {
          thisHiggs = mergedH;
        }
        EL_CHECK("fillETreeMbbMonitor",
                 fillETreeMbbMonitor(thisHiggs, l1, l2, j1, j2, j3));
      }

      // other local/temporary variables for easy tree
      int isMediumL1 = 1;
      int isMediumL2 = 1;
      if (isE) {
        isMediumL1 = Props::isMediumLH.get(el1);
        isMediumL2 = Props::isMediumLH.get(el2);
      }
      m_etree->SetBranchAndValue<int>("isMediumL1", isMediumL1, -99);
      m_etree->SetBranchAndValue<int>("isMediumL2", isMediumL2, -99);
      m_etree->SetBranchAndValue<int>(
          "passedTrigger", passSpecificCuts(eventFlag, {DiLeptonCuts::Trigger}),
          -1);
      m_etree->SetBranchAndValue<int>(
          "passedTauVeto", passSpecificCuts(eventFlag, {DiLeptonCuts::tauVeto}),
          -1);
      m_etree->SetBranchAndValue<int>("passVRJetOR",
                                      passVRJetOR(trackJetsForBTagSF), -99);
    }  // easy tree filling

    // fill trees
    if (writeMVATree && m_histNameSvc->get_isNominal() &&
        isSRevent == true) {  // only fill for SR events
      m_tree->Fill();
    }
  }

  //*********************************************************//
  //                       EASY TREE                          //
  //*********************************************************//
  if (m_physicsMeta.nSigJet >= 2 || m_physicsMeta.nFatJet >= 1) {
    if (writeEasyTree && m_histNameSvc->get_isNominal()) {
      m_etree->Fill();
    }
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

  /////////////////////////////////////////////////////////////
  // **Histos**: only fill histos for defined regions
  bool GenerateSTXS = false;
  m_config->getif<bool>("GenerateSTXS", GenerateSTXS);
  if (m_histNameSvc->get_description() != "") {
    EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
             fill_MVAVariablesHistos2Lep(isMu, isE));
    EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
             fill_EasyTreeHistos2Lep(isMu, isE));
    // if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved)
    EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
             fill_jetHistos2Lep(signalJets, forwardJets));
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::merged)
      EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
               fill_fatJetHistos(fatJets));
    if (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved) {
      for (std::map<std::string, MVAApplication_TMVA *>::iterator mva_iter =
               m_mvaVHbbApps.begin();
           mva_iter != m_mvaVHbbApps.end(); mva_iter++) {
        std::string mvaName = mva_iter->first;
        float mvaScore = mva_iter->second->Evaluate(
            m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
        m_histSvc->BookFillHist(mvaName, 500, -1, 1, mvaScore,
                                m_weight);  // new mva discriminant
      }
      double mbb = m_tree->mBB;
      if ((m_model == Model::CUT || passCUTselection) && !m_doNewRegions) {
        m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
        m_histSvc->BookFillHist("mBBCutBased", 100, 0., 500., mbb, m_weight);
        if (m_model == Model::MVA)
          m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::MVA);
      }
    }
  }
  // Fill C2 info
  if (m_isStoreMJwC2 && (m_physicsMeta.nFatJet > 0) &&
      (m_physicsMeta.regime == PhysicsMetadata::Regime::merged)) {
    float C2 = Props::C2.get(fatJets.at(0));
    double mbb = m_tree->mBB;
    if (!m_doOnlyInputs && !m_doReduceFillHistos) {
      // C2 distribution
      m_histSvc->BookFillHist("C2", 200, 0., 1., C2, m_weight);

      // C2 distribution in Higgs Mass window
      if (mbb > 100. && mbb < 140.) {
        m_histSvc->BookFillHist("C2mJ100M140", 200, 0., 1., C2, m_weight);
      }
    }

    // mJ with C2 cuts applied
    for (auto cut_info : m_c2Cuts) {
      if (C2 < cut_info.second)
        m_histSvc->BookFillHist(("mJ" + cut_info.first).c_str(), 100, 0, 500,
                                mbb, m_weight);
    }
  }
  if (!m_doOnlyInputs && passSpecificCuts(eventFlag, cuts_common_merged) &&
      passSpecificCuts(eventFlag, {DiLeptonCuts::SFLeptons}) &&
      fj1.vec.M() / 1e3 > 30) {
    // mJ > 30 GeV plots
    m_histNameSvc->set_description("SR");
    m_histSvc->BookFillHist("mJIncl", 100, 0, 500, HVec.M() / 1e3, m_weight);
  }
  if (GenerateSTXS && m_physicsMeta.nTags < 3) {
    if (STXS_GetBinFromEvtInfo() != -1) {  // check if it is the STXS signals
      STXS_FillYields();                   // a 2-D histogram filled
      std::string temp_name = m_histNameSvc->get_sample();
      STXS_ReplaceName();  // replace the signal name with the STXS bin
      if (m_histNameSvc->get_description() != "") {
        EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
                 fill_MVAVariablesHistos2Lep(isMu, isE));
        EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
                 fill_EasyTreeHistos2Lep(isMu, isE));
        EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
                 fill_jetHistos2Lep(signalJets, forwardJets));
        EL_CHECK("AnalysisReader_VHbb2Lep::run_2Lep_analysis",
                 fill_fatJetHistos(fatJets));
        if (!m_doOnlyInputs && !m_doReduceFillHistos)
          m_histSvc->BookFillHist("BDT", 100, 0, 1, m_tree->BDT, m_weight);
        if (!m_doOnlyInputs || m_physicsMeta.nTags == 2) {
          for (std::map<std::string, MVAApplication_TMVA *>::iterator mva_iter =
                   m_mvaVHbbApps.begin();
               mva_iter != m_mvaVHbbApps.end(); mva_iter++) {
            std::string mvaName = mva_iter->first;
            float mvaScore = mva_iter->second->Evaluate(
                m_tree->EventNumber, m_tree->nTags, m_tree->nJ, m_tree->pTV);
            m_histSvc->BookFillHist(mvaName, 500, -1, 1, mvaScore,
                                    m_weight);  // new mva discriminant

            double mbb = m_tree->mBB;
            if ((m_model == Model::CUT || passCUTselection) &&
                !m_doNewRegions) {
              m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
              m_histSvc->BookFillHist("mBBCutBased", 100, 0., 500., mbb,
                                      m_weight);
              if (m_model == Model::MVA)
                m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::MVA);
            }
          }
          m_histNameSvc->set_sample(
              temp_name);  // set the histogram name back from STXS addition
        }
      }
    }  // is STXS signal
  }    // do STXS signals
  // Info("AnalysisReader_VHbb2Lep::run_2Lep_analysis()", "Region: %s, and
  // nTags: %d, njet: %d ", m_histNameSvc->get_description().c_str(),
  // m_physicsMeta.nTags, m_histNameSvc->get_nJet());

  return EL::StatusCode::SUCCESS;
}  // run_2Lep_analysis
