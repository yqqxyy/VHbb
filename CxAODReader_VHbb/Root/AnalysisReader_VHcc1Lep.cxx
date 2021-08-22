#include <cfloat>
#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb1lep.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

#include <CxAODReader_VHbb/AnalysisReader_VHcc1Lep.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHcc1Lep::AnalysisReader_VHcc1Lep()
    : AnalysisReader_VHQQ1Lep(),
      m_ApplyFatJetMuOR(false),
      m_BDTSyst_debug(false),
      m_GenerateSTXS(false),
      m_applyPUWeight(true),
      m_doBJetEnergyCorr(true),
      m_doBlindingData(true),
      m_doBlindingMC(false),
      m_doInputPlots(true),
      m_doIsoInv(false),
      m_doMJEtree(false),
      m_doMbbRejHist(false),
      m_doMbbRescaling(true),
      m_doMergeCR(false),
      m_doRemoveDiLepOverlap(false),
      m_doSplitLepCharge(false),
      m_doSplitLepFlavour(false),
      m_doSplitWhfCR(false),
      m_doTLVPlots(false),
      m_mVHvsPtvCut(false),
      m_nominalOnly(false),
      m_printEventWeightsForCutFlow(false),
      m_reduceHistograms(false),
      m_useTTBarPTWFiltered1LepSamples(false),
      m_writeEasyTree(false),
      m_writeMVATree(false),
      m_writeObjectsInEasyTree(false),
      m_dataOneOverSF(1.0),
      m_mbbMergedHighEdge(145e3),
      m_mbbMergedLowEdge(75e3),
      m_mbbMergedLowerBound(50e3),
      m_mbbMergedUpperBound(200e3),
      m_mbbResolvedHighEdge(140e3),
      m_mbbResolvedLowEdge(110e3),
      m_mbbRestrictHighEdge(200e3),
      m_mbbRestrictLowEdge(50e3),
      m_minElMET(30e3),
      m_minMuMET(0e3),
      m_BDTSystVariations() {}

AnalysisReader_VHcc1Lep::~AnalysisReader_VHcc1Lep() {}

EL::StatusCode AnalysisReader_VHcc1Lep::fill_VBF(){
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHcc1Lep::run_1Lep_analysis() {
  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;
  if (m_debug) std::cout << " >>>>> Starting run_1Lep_analysis " << std::endl;
  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;

  //*********************************************************//
  //                     READ CONFIGURATION                  //
  //*********************************************************//

  if (m_isMC) m_bTagTool->setMCIndex_VHcc(m_mcChannel, m_period);
  //*********************************************************//
  //                     EVENT INITIALISATION                //
  //*********************************************************//
  ResultVHbb1lep selectionResult =
      ((VHbb1lepEvtSelection *)m_eventSelection)->result();
  const xAOD::Electron *el = selectionResult.el;
  const xAOD::Muon *mu = selectionResult.mu;
  TLorentzVector lepVecM;
  if (el)
    lepVecM = el->p4();
  else if (mu)
    lepVecM = mu->p4();

  // remove overlap between dilep and inclusive
  // for nonallhad ttbar and single-top Wt samples
  //----------------------------------------------------------------------------------------
  if (m_doRemoveDilepOverlap) {
    // ttbar 410472 dilep ttbar will replace the dilep from these below:
    // 410470: nonallhad ttbar default
    // 345951: nonallhad PTW slice 100-200
    // 346031: nonallhad PTW slice 200-inf
    // though note that originally (ICHEP 2018) only 410470 was in the if
    // but it seems to me (Adrian) more correct with all three
    // though I compared the two cases and I see absolutely no change

    if (m_mcChannel == 410470 || m_mcChannel == 345951 ||
        m_mcChannel == 346031) {
      int veto = vetoDilepTtbarEvents();
      if (veto == -1) {
        Error("run_1Lep_analysis()",
              "Something went wrong evaluating if ttbar is dilep, as "
              "Props::codeTTBarDecay doesn't exist!");
        return EL::StatusCode::FAILURE;
      } else if (veto == 1) {
        // std::cout << "find a dilepton ttbar veto it"
        //          << "\n";
        // Event has to be vetoed
        return EL::StatusCode::SUCCESS;
      }
    }

    // single top Wt apply to ttbar for the moment in 1L
    // 410648 and 410649 stopWt_dilep_PwPy8 will replace these below:
    // 410646 and 410647 stopWt_PwPy8
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
          Error("run_1Lep_analysis()",
                "CxAODTag31, something went wrong evaluating if STopWt is "
                "dilep or not, but not using Props::codeTTBarDecay!");
        } else if (m_CxAODTag >= "CxAODTag32") {
          Error("run_1Lep_analysis()",
                "CxAODTag32, something went wrong evaluating if STopWt is "
                "dilep, as Props::codeTTBarDecay doesn't exist!");
        }
        return EL::StatusCode::FAILURE;
      } else if (veto == 1) {
        // std::cout << "find a dilepton stopWt veto it"
        //          << "\n";
        // Event has to be vetoed
        return EL::StatusCode::SUCCESS;
      }
    }

  }  // end if m_doRemoveDilepOverlap

  // Merge pTW filter ttbar samples with the nominal one
  //---------------------------------------------------

  if (m_useTTBarPTWFiltered1LepSamples) {
    float pTWFilteringWeight = pTWFiltering(lepVecM);
    m_weight *= pTWFilteringWeight;
  }

  // Initialise lepton flavour category
  m_leptonFlavour = lepFlav::Combined;

  // initialize eventFlag - all cuts are initialized to false by default
  unsigned long eventFlagResolved = 0;
  unsigned long eventFlagMerged = 0;

  // reset physics metadata
  m_physicsMeta = PhysicsMetadata();
  m_physicsMeta.channel = PhysicsMetadata::Channel::OneLep;
  // Set the Analysis type for histograms
  if (m_model == Model::HVT) {
    if (m_debug)
      std::cout << " >>>>> Setting HistNameSvc::AnalysisType::VHres"
                << std::endl;
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::VHres);
  } else if (m_model == Model::CUT) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::CUT);
  } else if (m_model == Model::MVA) {
    m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::MVA);
  }
  if (m_debug)
    std::cout << " >>>>> Event Initialisation complete " << std::endl;

  // if(m_debug) std::cout << " >>>>> HistNameSvc m_variation : " <<
  // m_histNameSvc->get_variation() << std::endl;

  //*********************************************************//
  //      INIT ALL VARIABLES FROM THE START                  //
  //*********************************************************//

  if (m_doSplitLepFlavour) {
    m_leptonFlavour = (el ? lepFlav::El : lepFlav::Mu);
  }
  const xAOD::MissingET *missingET = selectionResult.met;
  std::vector<const xAOD::Jet *> signalJets = selectionResult.signalJets;
  std::vector<const xAOD::Jet *> forwardJets = selectionResult.forwardJets;
  std::vector<const xAOD::Jet *> fatJets = selectionResult.fatJets;
  std::vector<const xAOD::Jet *> trackJets = selectionResult.trackJets;
  std::vector<const xAOD::TauJet *> taus = selectionResult.taus;
  std::vector<const xAOD::Jet *> selectedJets;

  int nSignalJet = signalJets.size();
  int nForwardJet = forwardJets.size();
  int nJet = nSignalJet + nForwardJet;

  setevent_nJets(signalJets, forwardJets, fatJets);

  // **Definition** :  forward jets

  Jet fwdj1, fwdj2, fwdj3;
  if (nForwardJet >= 1) fwdj1.vec = forwardJets.at(0)->p4();
  if (nForwardJet >= 2) fwdj2.vec = forwardJets.at(1)->p4();
  if (nForwardJet >= 3) fwdj3.vec = forwardJets.at(2)->p4();

  // **Definition** :  signal jets

  Jet cenj1, cenj2, cenj3;
  if (nSignalJet >= 1) cenj1.vec = signalJets.at(0)->p4();
  if (nSignalJet >= 2) cenj2.vec = signalJets.at(1)->p4();
  if (nSignalJet >= 3) cenj3.vec = signalJets.at(2)->p4();

  // **Definition** : define jet pair/triplet for anti-QCD cut

  Jet preselj1, preselj2, preselj3;
  if (nSignalJet >= 1) preselj1.vec = signalJets.at(0)->p4();
  if (nSignalJet >= 2) preselj2.vec = signalJets.at(1)->p4();
  if (nSignalJet >= 3)
    preselj3.vec = signalJets.at(2)->p4();
  else if (nForwardJet >= 1)
    preselj3.vec = forwardJets.at(0)->p4();

  if (m_debug) std::cout << " >>>>> Formed Jet 4-Vectors " << std::endl;

  int tagcatExcl = -1;
  int nSubleadingBtags = 0;

  // compute_btagging();

  m_bTagTool->tagjet_selection_vhcc(signalJets, forwardJets, selectedJets,
                                    tagcatExcl, nSubleadingBtags);

  m_physicsMeta.nTags = tagcatExcl;

  setevent_flavour(
      selectedJets);  // default from resolved analysis; is overwritten later
  int nSelectedJet = selectedJets.size();

  // clear

  // **Definition**: define b-jet system

  Higgs H;
  bbjSystem bbj;
  Jet selj1, selj2, selj3;

  Jet j1, j2, j3;
  setJetVariables(j1, j2, j3, selectedJets);

  if (nSelectedJet >= 1) selj1.vec = selectedJets.at(0)->p4();
  if (nSelectedJet >= 2) selj2.vec = selectedJets.at(1)->p4();
  if (nSelectedJet >= 3) selj3.vec = selectedJets.at(2)->p4();
  if (nSelectedJet >= 2) H.vec = selj1.vec + selj2.vec;
  if (nSelectedJet >= 3) bbj.vec = H.vec + selj3.vec;

  // **Definition**: b-jet corrected vectors

  j1.vec_corr = selj1.vec;  // fall back
  j2.vec_corr = selj2.vec;
  j3.vec_corr = selj3.vec;

  // don't perform VHbb corrections in VHcc for now
  // if ((nSelectedJet >= 1) && BTagProps::isTagged.get(selectedJets.at(0))) {
  //   EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
  //            getBJetEnergyCorrTLV(selectedJets.at(0), j1.vec_corr, false,
  //                                 m_jetCorrType));
  // }
  // if ((nSelectedJet >= 2) && BTagProps::isTagged.get(selectedJets.at(1))) {
  //   EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
  //            getBJetEnergyCorrTLV(selectedJets.at(1), j2.vec_corr, false,
  //                                 m_jetCorrType));
  // }
  // if ((nSelectedJet >= 3) && BTagProps::isTagged.get(selectedJets.at(2))) {
  //   EL_CHECK("AnalysisReader_VHQQ::setJetVariables() ",
  //            getBJetEnergyCorrTLV(selectedJets.at(2), j3.vec_corr, false,
  //                                 m_jetCorrType));
  // }

  if (nSelectedJet >= 2) H.vec_corr = j1.vec_corr + j2.vec_corr;
  if (nSelectedJet >= 3)
    bbj.vec_corr = H.vec_corr + j3.vec_corr;  // should use corrected 3rd jet?

  if (m_debug)
    std::cout << " >>>>> BJetEnergyCorr FatJet (muon-in-jet) complete"
              << std::endl;

  // **Definition**: mbb rescaling

  j1.vec_resc = j1.vec_corr;  // baseline
  j2.vec_resc = j2.vec_corr;
  H.vec_resc = H.vec_corr;

  if (m_doMbbRescaling && (H.vec_corr.M() > m_mbbResolvedLowEdge) &&
      (H.vec_corr.M() < m_mbbResolvedHighEdge)) {
    rescale_jets(H.vec_corr.M(), j1.vec_resc, j2.vec_resc);
  }
  if (nSelectedJet >= 2) H.vec_resc = j1.vec_resc + j2.vec_resc;

  // **Definition** : met
  MET met, met_merged;

  Double_t metSig = -1.;
  Double_t metSig_PU = -1.;
  Double_t metSig_soft = -1.;
  Double_t metSig_hard = -1.;
  Double_t metOverSqrtSumET = -1.;
  Double_t metOverSqrtHT = -1.;

  if (missingET) {
    met.vec.SetPtEtaPhiM(missingET->met(), 0, missingET->phi(), 0);
    if (nSelectedJet >= 2)

      met.vec_corr = getMETCorrTLV(missingET, {&selj1.vec, &selj2.vec},
                                   {&j1.vec_corr, &j2.vec_corr});
    if (nSelectedJet >= 2)
      met.vec_resc = getMETCorrTLV(missingET, {&selj1.vec, &selj2.vec},
                                   {&j1.vec_resc, &j2.vec_resc});

    // MET Significance
    //---------------------------------------------------------
    metSig = Props::metSig.get(missingET);
    if (Props::metSig_PU.exists(missingET)) {
      metSig_PU = Props::metSig_PU.get(missingET);
    } else {
      metSig_hard = Props::metSig_hard.get(missingET);
      metSig_soft = Props::metSig_soft.get(missingET);
    }
    metOverSqrtSumET = Props::metOverSqrtSumET.get(missingET);
    metOverSqrtHT = Props::metOverSqrtHT.get(missingET);
  }

  // **Definition** : lepton
  Lepton lep, nu, nu_merged;
  int nLooseEl = 0;
  int nSigEl = 0;
  int nLooseMu = 0;
  int nSigMu = 0;
  if (el) {
    lep.vec = el->p4();
    if (Props::isVHLooseElectron.get(el) == 1) nLooseEl = 1;
    if (Props::isWHSignalElectron.get(el) == 1) nSigEl = 1;
  } else if (mu) {
    lep.vec = mu->p4();
    if (Props::isVHLooseMuon.get(mu) == 1) nLooseMu = 1;
    if (Props::isWHSignalMuon.get(mu) == 1) nSigMu = 1;
  } else {
    Error("run_1Lep_analysis()", "Missing lepton!");
    return EL::StatusCode::FAILURE;
  }

  // **Definition** : neutrino using W mass constraint
  nu.vec = getNeutrinoTLV(met.vec, lep.vec, true);
  nu.vec_corr = getNeutrinoTLV(met.vec_corr, lep.vec, true);
  nu.vec_resc = getNeutrinoTLV(met.vec_resc, lep.vec, true);
  nu_merged.vec_corr = getNeutrinoTLV(met_merged.vec_corr, lep.vec, true);
  nu_merged.vec_resc = getNeutrinoTLV(met_merged.vec_resc, lep.vec, true);

  // **Definition** : W
  VCand W, W_merged;
  lep.vecT.SetPtEtaPhiM(lep.vec.Pt(), 0, lep.vec.Phi(), 0);
  W.vecT = lep.vecT + met.vec;
  W.vec = lep.vec + nu.vec;
  W.vec_corr = lep.vec + nu.vec_corr;
  W.vec_resc = lep.vec + nu.vec_resc;
  W_merged.vec_corr = lep.vec + nu_merged.vec_corr;
  W_merged.vec_resc = lep.vec + nu_merged.vec_resc;

  // **Definition** : VH resonance
  VHCand VH, VH_merged;

  if (nSelectedJet >= 2) VH.vec = W.vec + H.vec;
  if (nSelectedJet >= 2) VH.vec_corr = W.vec_corr + H.vec_corr;
  if (nSelectedJet >= 2) VH.vec_resc = W.vec_resc + H.vec_resc;

  if (m_debug) std::cout << " >>>>> Formulated VH system 4-Vector" << std::endl;

  // sumpt
  // -------------------------------------------------------
  double sumpt = -1;
  if ((m_physicsMeta.nSigJet >= 3) ||
      (m_physicsMeta.nSigJet == 2 && m_physicsMeta.nForwardJet >= 1)) {
    sumpt = preselj1.vec.Pt() + preselj2.vec.Pt() + preselj3.vec.Pt();
  } else if (m_physicsMeta.nSigJet == 2) {
    sumpt = preselj1.vec.Pt() + preselj2.vec.Pt();
  }
  m_triggerTool->setSumpt(sumpt);

  //*********************************************************//
  //                  RESOLVED BITFLAG / CUTFLOW             //
  //*********************************************************//

  if (m_debug) std::cout << " >>>>> Resolved BitFlag" << std::endl;

  // C0: All events (CxAOD)
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::AllCxAOD);

  if (m_debug) {
    Info(
        "AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
        ">>>>>>>>>>>>>>>>> AllCxAOD : %d",
        passSpecificCuts(eventFlagResolved, {OneLeptonResolvedCuts::AllCxAOD}));
  }

  // C1: trigger
  double triggerSF = 1.;
  bool isTriggered = pass1LepTrigger(triggerSF, selectionResult);

  if (m_debug)

    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> isTriggered : %d", isTriggered);

  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::Trigger, isTriggered);

  // C2: 1 loose lepton
  if (m_debug)
    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> nLooseEl + nLooseMu : %d", nLooseEl + nLooseMu);

  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::LooseLeptons,
             (nLooseEl + nLooseMu == 1));

  // C3: 1 signal lepton  (HVT Lepton defintion raised to 27GeV du to new 2016
  // trigger menu) (SM VH(bb) Electron Channel and medium pTV region Muon
  // channel raised to 27GeV, High pTV region Muon Channel stay with 25GeV)
  // retrieve quantities on isolation and lepton quality to restore cuts
  // on reader level that were opened for the MJ CxAODs

  bool passPtLep27 = passLeptonPt27(selectionResult, lep.vec, W.vecT);
  bool passIsoInv = passIsoInverted(selectionResult, lep.vec);
  bool passNom = passNomIsolation(selectionResult, lep.vec);
  bool passLepQuality = passLeptonQuality(selectionResult);

  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SignalLeptons,
             passPtLep27 and passLepQuality and (nSigEl + nSigMu == 1) and
                 ((!m_doIsoInv and passNom) or
                  ((m_doIsoInv or m_doQCD) and passIsoInv)));

  // C4: njets
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::AtLeast2Jets, nJet >= 2);
  if (m_debug)

    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> AtLeast2Jets : %d",
         passSpecificCuts(eventFlagResolved,
                          {OneLeptonResolvedCuts::AtLeast2Jets}));

  // C4.5
  // hcohen @ 28.10.19
  // no need for 4p jet events
  if (nJet >= 4) {
    return EL::StatusCode::SUCCESS;
  }

  // C5: nSignalJets
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::AtLeast2SigJets,
             (m_model == Model::HVT) ? (nSignalJet >= 2 && nSignalJet <= 3)
                                     : nSignalJet >= 2);
  if (m_debug)
    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> AtLeast2SigJets : %d",
         passSpecificCuts(eventFlagResolved,
                          {OneLeptonResolvedCuts::AtLeast2SigJets}));

  // C6: pt > 45
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::Pt45,
             selj1.vec.Pt() > 45e3);
  if (m_debug)
    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> Pt45 : %d",
         passSpecificCuts(eventFlagResolved, {OneLeptonResolvedCuts::Pt45}));

  // C7, C8 MET mTW cut

  bool passMETcut =
      passMET(W.vecT, met.vec, nSigEl, nSigMu, m_minElMET, m_minMuMET);
  bool passmTWcut = passmTW(W.vecT);

  if ((nSigEl == 1 && nSigMu == 0) || (nSigEl == 0 && nSigMu == 1)) {
    updateFlag(eventFlagResolved, OneLeptonResolvedCuts::MET, passMETcut);
    updateFlag(eventFlagResolved, OneLeptonResolvedCuts::mTW, passmTWcut);
  } else {
    Error("run_1lep()", "Invalid lepton type");
    return EL::StatusCode::FAILURE;
  }

  if (m_debug)
    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> mTW : %d",
         passSpecificCuts(eventFlagResolved, {OneLeptonResolvedCuts::mTW}));
  if (m_debug)
    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> MET : %d",
         passSpecificCuts(eventFlagResolved, {OneLeptonResolvedCuts::MET}));

  // C9: 3rd b-jet veto in VHcc we don't use this cut
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::Veto3bjet,
             true);  // tagcatExcl < 3

  if (m_debug)
    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> Veto3bjet : %d",
         passSpecificCuts(eventFlagResolved,
                          {OneLeptonResolvedCuts::Veto3bjet}));

  // mbb rejection
  bool is_not_in_mbbRestrict = true;
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::mbbRestrict,
             is_not_in_mbbRestrict);

  if (m_debug)
    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> mBBRestrict : %d",
         passSpecificCuts(eventFlagResolved,
                          {OneLeptonResolvedCuts::mbbRestrict}));

  // mbb corrected
  bool is_inside_mbbCorr = true;
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::mbbCorr,
             is_inside_mbbCorr);
  if (m_debug)

    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> mBBCorr : %d",
         passSpecificCuts(eventFlagResolved, {OneLeptonResolvedCuts::mbbCorr}));

  // pTV

  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::pTV,
             W.vecT.Pt() / 1.e3 > 75);

  if (m_debug)

    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> pTV : %d",
         passSpecificCuts(eventFlagResolved, {OneLeptonResolvedCuts::pTV}));

  // // dRBB cut in Cut-based
  // H.C -  Harmonized with 2L
  if (W.vecT.Pt() <= 150e3)
    return EL::StatusCode::SUCCESS;  // no need for lower pTV region in 1L
  // updateFlag(eventFlagResolved, OneLeptonResolvedCuts::dRBB,
  //           selj1.vec.DeltaR(selj2.vec) < 3.0);
  else if (W.vecT.Pt() > 150e3 && W.vecT.Pt() <= 250e3)
    updateFlag(eventFlagResolved, OneLeptonResolvedCuts::dRBB,
               selj1.vec.DeltaR(selj2.vec) < 1.6);
  else if (W.vecT.Pt() > 250e3)
    updateFlag(eventFlagResolved, OneLeptonResolvedCuts::dRBB,
               selj1.vec.DeltaR(selj2.vec) < 1.2);

  // additional mTW cut in Cut-based
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::mTW_add,
             W.vecT.M() < 120e3);

  // dphi
  double mindPhi1 = fabs(preselj1.vec.DeltaPhi(met.vec));
  double mindPhi2 = fabs(preselj2.vec.DeltaPhi(met.vec));
  double mindPhi3 = 1000;
  if (nJet >= 3) mindPhi3 = fabs(preselj3.vec.DeltaPhi(met.vec));
  double mindPhi = std::min(mindPhi1, std::min(mindPhi2, mindPhi3));

  // for SR flags, require all cuts up to C10
  bool passResolvedSR = passSpecificCuts(eventFlagResolved, cuts_SR_resolved);
  bool passResolvedCR =
      !passResolvedSR && passSpecificCuts(eventFlagResolved, cuts_CR_resolved);

  if (m_debug)

    Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
         ">>>>>>>>>>>>>>>>> passResolvedSR : %d", passResolvedSR);

  // May need to switch to passSPecificCuts() version instead of passAllCutsUpTo
  // passSpecificCuts(eventFlagResolved, {OneLeptonResolvedCuts::AllCxAOD,
  // OneLeptonResolvedCuts::Trigger, OneLeptonResolvedCuts::LooseLeptons,
  //                                     OneLeptonResolvedCuts::SignalLeptons,
  //                                     OneLeptonResolvedCuts::AtLeast2Jets,
  //                                     OneLeptonResolvedCuts::AtLeast2SigJets,
  //                                     OneLeptonResolvedCuts::Pt45,
  //                                     OneLeptonResolvedCuts::MET,
  //                                     OneLeptonResolvedCuts::mTW,
  //                                     OneLeptonResolvedCuts::Veto3bjet,
  //                                     OneLeptonResolvedCuts::mbbRestrict,
  //                                     OneLeptonResolvedCuts::mbbCorr,
  //                                     OneLeptonResolvedCuts::pTV});
  if (m_model == Model::CUT)
    passResolvedSR = passSpecificCuts(eventFlagResolved, cuts_model_resolved);

  // C10: SR_0tag_2jet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_0tag_2jet,
             passResolvedSR && (tagcatExcl == 0) && (nJet == 2));
  // C11: SR_0tag_3jet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_0tag_3jet,
             passResolvedSR && (tagcatExcl == 0) && (nJet == 3));
  // C12: SR_0tag_4pjet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_0tag_4pjet,
             passResolvedSR && (tagcatExcl == 0) && (nJet >= 4));
  // C13: SR_1tag_2jet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_1tag_2jet,
             passResolvedSR && (tagcatExcl == 1) && (nSubleadingBtags == 0) &&
                 (nJet == 2));
  // C14: SR_1tag_3jet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_1tag_3jet,
             passResolvedSR && (tagcatExcl == 1) && (nSubleadingBtags == 0) &&
                 (nJet == 3));
  // C15: SR_1tag_4pjet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_1tag_4pjet,
             passResolvedSR && (tagcatExcl == 1) && (nSubleadingBtags == 0) &&
                 (nJet >= 4));
  // C16: SR_2tag_2jet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_2tag_2jet,
             passResolvedSR && (tagcatExcl == 2) && (nSubleadingBtags == 0) &&
                 (nJet == 2));
  // C17: SR_2tag_3jet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_2tag_3jet,
             passResolvedSR && (tagcatExcl == 2) && (nSubleadingBtags == 0) &&
                 (nJet == 3));
  // C18: SR_2tag_4pjet
  updateFlag(eventFlagResolved, OneLeptonResolvedCuts::SR_2tag_4pjet,
             passResolvedSR && (tagcatExcl == 2) && (nSubleadingBtags == 0) &&
                 (nJet >= 4));

  //*********************************************************//
  //      DECISION TO USE MERGED OR RESOLVED ANALYSIS        //
  //*********************************************************//

  bool passMerged = (m_physicsMeta.nFatJet >= 1);
  bool passResolved = (m_physicsMeta.nSigJet >= 2);

  if (m_debug)
    std::cout << " >>>>> Resolved/Merged Analysis Decision " << std::endl;
  if (m_debug) std::cout << "       passMerged : " << passMerged << std::endl;
  if (m_debug)
    std::cout << "       passResolved : " << passResolved << std::endl;
  if (m_debug)
    std::cout << "       m_analysisStrategy : " << m_analysisStrategy
              << std::endl;

  // the function selectRegime chooses between
  // m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  // m_physicsMeta.regime = PhysicsMetadata::Regime::resolved
  bool passMergedCR = false;
  bool passMergedSR = false;

  selectRegime(W.vecT.Pt(), tagcatExcl, fatJets, passResolved, passMerged,
               passResolvedSR, passMergedSR, passResolvedCR, passMergedCR);

  bool isResolved = (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved);
  bool isMerged = (m_physicsMeta.regime == PhysicsMetadata::Regime::merged);

  if (m_debug)
    std::cout << " >>>>> Resolved/Merged Analysis Decision Complete"
              << std::endl;
  if (m_debug) std::cout << "       isMerged : " << isMerged << std::endl;
  if (m_debug) std::cout << "       isResolved : " << isResolved << std::endl;

  //*********************************************************//
  //                    CALCULATE WEIGHTS                    //
  //*********************************************************//

  // **Weight**: Lepton
  double leptonSF = 1.;
  if (m_isMC) {
    leptonSF = Props::leptonSF.get(m_eventInfo);
  }

  m_weight *= leptonSF;

  // **Weight** : Trigger
  // double triggerSF = 1.;
  // bool isTriggered =  pass1LepTrigger(triggerSF,selectionResult);
  // if (m_isMC) m_weight *= triggerSF; <- is done already inside
  // pass1LepTrigger

  // **Weight** : JVT
  if (m_isMC) m_weight *= compute_JVTSF(signalJets);

  // **Weight** : b-tagging
  float btagWeight = 1.;

  bool do_btag_Syst = (!m_nominalOnly && m_currentVar == "Nominal");

  std::map<std::string, std::map<int, float>> btag_TT_weights;

  // compute_TruthTag_EventWeight_VHcc also computes the direct tag btag weight
  if (m_isMC && nSelectedJet > 1) {
    EL_CHECK("AnalysisReader_VHQQ::compute_TT_min_deltaR_correction",
             compute_TT_min_deltaR_correction(selectedJets, btag_TT_weights,
                                              do_btag_Syst));
  }

  float eventweight_before_btagging_weight = m_weight;

  // extract the direct tag weight
  btagWeight = btag_TT_weights["Nominal"][-1];

  if (m_isMC) {
    m_weight *= btagWeight;
  }

  // **Weight** : Pileup
  if (m_isMC) {
    if (!m_applyPUWeight) {
      m_weightSysts.push_back({"_withPU", (float)(m_pileupReweight)});
    }
  }

  // **Weight** : FakeFactor method for QCD
  if (m_doQCD) {
    m_histNameSvc->set_sample("multijet");
    if (el && nSigEl == 1) {
      EL_CHECK("run_1Lep_analysis()", applyFFweight(W.vecT, met.vec, el));
    }
    if (mu && nLooseMu == 1) {
      EL_CHECK("run_1Lep_analysis()", applyFFweight(W.vecT, met.vec, mu));
    }
    // applyFFweight might set event weight to zero in some cases,
    // like e.g. bad track isolation for electrons
    // (DBL_DIG: number of double digits, defined in <cfloat>)
    if (abs(m_weight) < (1. / (DBL_DIG - 1))) {
      if (m_debug) {
        Info("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
             "abs(m_weight) < 1e-%d after applyFFweight. Reject event.",
             (DBL_DIG - 1));
      }
      return EL::StatusCode::SUCCESS;
    }
  }
  // **Weight** : Template Method for SM VH(bb) multijet estimation.
  std::string samplename = m_histNameSvc->get_sample();
  if (m_doIsoInv && (m_model == Model::MVA || m_model == Model::CUT)) {
    if (el) m_histNameSvc->set_sample("multijetEl");
    if (mu) m_histNameSvc->set_sample("multijetMu");
    if (m_isMC) m_weight *= -1;
  }

  if (m_debug) std::cout << " >>>>> Applied Weights" << std::endl;

  //*********************************************************//
  //         EVENT CATEGORIZATION BASED ON BIT FLAG          //
  //*********************************************************//

  if (m_debug) std::cout << " >>>>> Event Categoristion Start" << std::endl;

  m_histNameSvc->set_description("");

  // leptons
  if (!m_doMergePtVBins) m_histNameSvc->set_pTV(W.vecT.Pt());

  // here the difference is made between merged and resolved in histnames
  if (isResolved) {
    setevent_flavour(selectedJets);
    m_histNameSvc->set_nTag(tagcatExcl);
    m_histNameSvc->set_nJet(m_physicsMeta.nJets);
    // m_histNameSvc->set_nJet(m_physicsMeta.nJets);
    if (m_doMergeJetBins)
      m_histNameSvc->set_nJet(m_physicsMeta.nJets >= 2 ? -2
                                                       : m_physicsMeta.nJets);
    else
      m_histNameSvc->set_nJet(m_physicsMeta.nJets);
    // m_histNameSvc->set_Resolved(isResolved);
  }

  m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);

  // Region
  if (isResolved) {
    if (passSpecificCuts(eventFlagResolved, cuts_resolved)) {
      if (m_model == Model::MVA) {
        m_histNameSvc->set_description("SR");
        m_physicsMeta.region = PhysicsMetadata::Region::SR;
      } else if (m_model == Model::CUT) {
        if (passSpecificCuts(eventFlagResolved, cuts_resolved_CUT)) {
          m_histNameSvc->set_description("SR");
          m_physicsMeta.region = PhysicsMetadata::Region::SR;
        }
      }

      if (m_debug)
        std::cout << " >>>>> EventCategory : "
                  << m_histNameSvc->getFullHistName("") << std::endl;

    } else {
      // Skip the event
      if (!m_doCutflow) return EL::StatusCode::SUCCESS;
    }
  }

  float Mtop = calculateMtop(
      lep.vec, met.vec, j1.vec_corr,
      j2.vec_corr);  // Moved these here to be used for W+hf CR definition
  float dYWH = calculatedYWH(lep.vec, met.vec, j1.vec_corr, j2.vec_corr);

  if (isResolved) {
    if (m_histNameSvc->get_description() != "") {
      std::string descr = m_histNameSvc->get_description();

      if (m_doSplitLepCharge) {
        int charge = 0;
        if (el)
          charge = el->charge();
        else
          charge = mu->charge();
        if (charge < 0)
          descr += "_minus";
        else
          descr += "_plus";
      }
      m_histNameSvc->set_description(descr);
    }
  }

  // (m_debug) std::cout << " >>>>> EventCategory : " <<
  // m_histNameSvc->getFullHistName("mVH") << std::endl;

  // Check if event in blinded region
  /*  bool isBlindingRegion =
      (isResolved &&
       passSpecificCuts(eventFlagResolved, OneLeptonResolvedCuts::AllCxAOD,
  OneLeptonResolvedCuts::Trigger, OneLeptonResolvedCuts::LooseLeptons,
  OneLeptonResolvedCuts::SignalLeptons, OneLeptonResolvedCuts::AtLeast2Jets,
  OneLeptonResolvedCuts::AtLeast2SigJets, OneLeptonResolvedCuts::Pt45,
  OneLeptonResolvedCuts::MET, OneLeptonResolvedCuts::mTW,
  OneLeptonResolvedCuts::Veto3bjet, OneLeptonResolvedCuts::mbbRestrict,
  OneLeptonResolvedCuts::mbbCorr, OneLeptonResolvedCuts::pTV) &&
       (m_histNameSvc->get_nTag() == 1)) ||
      (isResolved &&
       passAllCutsUpTo(eventFlagResolved, OneLeptonResolvedCuts::pTV, {}) &&
       (m_histNameSvc->get_nTag() == 2)) ||
      (isMerged &&
       passAllCutsUpTo(eventFlagMerged, OneLeptonMergedCuts::pTV, {}) &&
       (m_histNameSvc->get_nTag() == 1)) ||
      (isMerged &&
       passAllCutsUpTo(eventFlagMerged, OneLeptonMergedCuts::pTV, {}) &&
       (m_histNameSvc->get_nTag() == 2));

  m_doBlinding = isBlindingRegion &&
                 ((m_isMC && m_doBlindingMC) || (!m_isMC && m_doBlindingData));
  */

  //*********************************************************//
  //                  CORRS AND SYS                          //
  // This part needs some props from histnamesvc             //
  // -> don't call earlier                                   //
  // This part changes weights                               //
  // -> don't fill histograms before                         //
  //*********************************************************//

  std::vector<std::pair<TString, Float_t>> BDTSyst_scores;

  if (m_isMC &&
      ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    // apply CorrsAndSysts to m_weight and m_weightSysts (relies on
    // m_histNameSvc): added the MJ_TrigEff systematic from the template method
    // on 23-Jan-2017 this should be moved to the TriggerTool_VHbb eventually
    if (isResolved) applyCorrections();
    if (isResolved)
      applySystematics(W.vecT.Pt(), met.vec.Pt(), nJet, tagcatExcl, j1.vec_corr,
                       j2.vec_corr, m_model == Model::CUT, Mtop);
  }

  if (m_doIsoInv &&
      (m_csCorrections.size() != 0 || m_csVariations.size() != 0)) {
    applyElMuCS(samplename, nJet, selectionResult, lep.vec);
  }

  //-------------------------
  // TTbar & Stop systematics histos
  //--------------------------
  if ((samplename == "ttbar" || samplename.find("stop") != std::string::npos) &&
      m_currentVar == "Nominal" && m_model == Model::HVT &&
      (isResolved || isMerged) && !m_nominalOnly) {
    double mBB = H.vec_corr.M();
    float mVH = isResolved
                    ? (m_doMbbRescaling ? VH.vec_resc.M() : VH.vec_corr.M())
                    : VH_merged.vec_corr.M();
    std::string Regime = isResolved ? "Resolved" : "Boosted";
    float nAddBTags = 0;

    // Fills m_weightSysts vector with a series of modelling systematics
    // variations:
    //      Powheg+Herwig++ Vs Powheg+Pythia6
    //      aMC@NLO+Herwig++ Vs Powheg+Herwig++
    //      Powheg+Pythia6 radHi & radLo
    // NOTE: mVH in GeV --> mVH/1e3 is used
    fillTopSystslep_PRSR(Regime, nAddBTags, mBB,
                         mVH / 1e3);  // mVH= GeV, mBB = MeV
  }

  //*********************************************************//
  //              ttbar  BDT Based Systematics               //
  //                                                         //
  //*********************************************************//

  if (m_histNameSvc->get_sample() == "ttbar" && m_currentVar == "Nominal") {
    int channel = 0;
    if (m_analysisType == "1lep") channel = 1;
    if (m_analysisType == "2lep") channel = 2;

    BDTSyst_scores =
        ApplyBDTSyst(channel, selectedJets, W.vecT, lep.vec, Mtop, bbj.vec_corr,
                     j3.vec_corr, dYWH, btagWeight, nJet);
  }

  //-----------------------------
  // VJets Systematic Histograms
  //-----------------------------
  // NOTE :: mBB is in MeV because CorrsAndSysts uses MeV !!!!!!!!!!!!!!!!!!!
  if ((m_histNameSvc->get_sample() == "W" ||
       m_histNameSvc->get_sample() == "Wv22") &&
      m_currentVar == "Nominal" && m_model == Model::HVT &&
      (isResolved || isMerged) && !m_nominalOnly) {
    double mBB = H.vec_corr.M();
    float mVH = isResolved
                    ? (m_doMbbRescaling ? VH.vec_resc.M() : VH.vec_corr.M())
                    : VH_merged.vec_corr.M();
    // fillVjetsSystslep( mBB, mVH/1e3);
    std::string Regime = isResolved ? "Resolved" : "Boosted";
    float nAddBTags = 0;
    fillVjetsSystslep_PRSR(Regime, mBB, mVH / 1e3,
                           nAddBTags);  // mVH= GeV, mBB = MeV
  }

  //-----------------------------------------------
  // Internal weight variations Histograms
  //-----------------------------------------------
  if (m_evtWeightVarMode != -1 && m_currentVar == "Nominal" && m_isMC) {
    EL_CHECK("applyEvtWeights()", apply_EvtWeights());
  }

  if (m_debug) std::cout << " >>>>> CorrsAndSysts Applied" << std::endl;

  //*********************************************************//
  //                       MVA TREE                          //
  //*********************************************************//

  if (m_debug) std::cout << " >>>>> MVA Tree" << std::endl;

  if (m_model == Model::MVA ||
      m_writeMVATree) {  // Only need the MVA tree for SM VH MVA analysis.  User
                         // can still generate tree when setting bool
                         // writeMVATree = true in config file
    m_tree->Reset();
    m_tree->SetVariation(m_currentVar);
    fillMVATreeVHbbResolved1Lep(j1.vec_corr, j2.vec_corr, j3.vec_corr,
                                selectedJets, signalJets, forwardJets,
                                H.vec_corr, W.vecT, met.vec, missingET, lep.vec,
                                taus.size(), btagWeight);

    // fill MVA tree; makes sure nothing is overwritten later
    if (m_histNameSvc->get_isNominal() &&
        (m_histNameSvc->get_description() != "")) {
      m_tree->Fill();
    }
  }

  //*********************************************************//
  //                       EASY TREE                         //
  //*********************************************************//

  if (m_debug) std::cout << " >>>>> EASY TREE" << std::endl;

  if (m_writeObjectsInEasyTree && !m_writeEasyTree)
    m_writeEasyTree = true;  // What way of configuration is this???

  // EasyTree reset
  m_etree->Reset();
  m_etree->SetVariation(m_currentVar);

  if ((nSelectedJet >= 2 || m_physicsMeta.nFatJet >= 1)) {
    // event info
    m_etree->SetBranchAndValue<std::string>(
        "Description", m_histNameSvc->get_description(), "");
    m_etree->SetBranchAndValue<std::string>("Sample",
                                            m_histNameSvc->getFullSample(), "");
    m_etree->SetBranchAndValue<std::string>(
        "EventFlavor", m_histNameSvc->getEventFlavour(), "");
    m_etree->SetBranchAndValue<int>("EventNumber", m_eventInfo->eventNumber(),
                                    -1);
    m_etree->SetBranchAndValue<float>("AverageMu", m_averageMu, -99);
    m_etree->SetBranchAndValue<float>("ActualMu", m_actualMu, -99);
    m_etree->SetBranchAndValue<float>("AverageMuScaled", m_averageMuScaled,
                                      -99);
    m_etree->SetBranchAndValue<int>("isTriggered", isTriggered, -1);
    m_etree->SetBranchAndValue<float>("ActualMuScaled", m_actualMuScaled, -99);
    // weights
    m_etree->SetBranchAndValue<float>("EventWeight", m_weight, -1.);
    m_etree->SetBranchAndValue<float>("PUWeight", m_pileupReweight, -1.);
    m_etree->SetBranchAndValue<float>("BTagSF", btagWeight, -1.);
    m_etree->SetBranchAndValue<float>("TriggerSF", triggerSF, -1.);
    m_etree->SetBranchAndValue<float>("LeptonSF", leptonSF, -1.);

    // objects counts
    m_etree->SetBranchAndValue<int>("nJets", nJet, -1);
    m_etree->SetBranchAndValue<int>("nSigJets", nSignalJet, -1);
    m_etree->SetBranchAndValue<int>("nFwdJets", nForwardJet, -1);
    m_etree->SetBranchAndValue<int>("nFats", m_physicsMeta.nFatJet, -1);
    m_etree->SetBranchAndValue<int>("nTaus", taus.size(), -1);
    m_etree->SetBranchAndValue<int>("nTags", m_physicsMeta.nTags, -1);
    m_etree->SetBranchAndValue<int>("nbJets", m_physicsMeta.nTags, -1);
    m_etree->SetBranchAndValue<int>("nElectrons", nSigEl, -1);
    m_etree->SetBranchAndValue<int>("nMuons", nSigMu, -1);
    m_etree->SetBranchAndValue<float>("Mtop", Mtop / 1e3, -1.);
    // event categorisation
    //     m_etree->SetBranchAndValue<int>("isMerged",      isMerged, -1);
    //     m_etree->SetBranchAndValue<int>("isResolved",    isResolved, -1);
    m_etree->SetBranchAndValue<int>("EventRegime",
                                    static_cast<int>(m_physicsMeta.regime), -1);
    m_etree->SetBranchAndValue<unsigned long>(
        "eventFlagResolved/l", eventFlagResolved,
        0);  // "/l" in the end to determine type
    m_etree->SetBranchAndValue<unsigned long>("eventFlagMerged/l",
                                              eventFlagMerged, 0);

    // general 1-lepton quantities
    m_etree->SetBranchAndValue<float>("ptL1", lep.vec.Pt() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("etaL1", lep.vec.Eta(), -10.);
    m_etree->SetBranchAndValue<float>("phiL1", lep.vec.Phi(), -10.);
    m_etree->SetBranchAndValue<float>("mL1", lep.vec.M() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("mTW", W.vecT.M() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("pTW", W.vecT.Pt() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("phiW", W.vecT.Phi(), -10);
    //    m_etree->SetBranchAndValue<float>("mW", W.vecT.M() / 1e3, -1); //this
    //    is also the transverse W mass?
    m_etree->SetBranchAndValue<float>("met", met.vec.Pt() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("metSig", metSig, -1.);
    if (metSig_PU != -1) {
      m_etree->SetBranchAndValue<float>("metSig_PU", metSig_PU, -1.);
    } else {
      m_etree->SetBranchAndValue<float>("metSig_soft", metSig_soft, -1.);
      m_etree->SetBranchAndValue<float>("metSig_hard", metSig_hard, -1.);
    }
    m_etree->SetBranchAndValue<float>("metOverSqrtSumET", metOverSqrtSumET,
                                      -1.);
    m_etree->SetBranchAndValue<float>("metOverSqrtHT", metOverSqrtHT, -1.);

    float j1MV2c10 = -1.;
    if (nSelectedJet > 0) j1MV2c10 = Props::MV2c10.get(selectedJets.at(0));
    m_etree->SetBranchAndValue<float>("pTB1", j1.vec_corr.Pt() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("etaB1", j1.vec_corr.Eta(), -10.);
    m_etree->SetBranchAndValue<float>("phiB1", j1.vec_corr.Phi(), -10.);
    m_etree->SetBranchAndValue<float>("mB1", j1.vec_corr.M() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("MV2c10B1", j1MV2c10, -1.);
    m_etree->SetBranchAndValue<int>("flavB1", m_physicsMeta.b1Flav, -1);
    float j2MV2c10 = -1.;
    if (nSelectedJet > 1) j2MV2c10 = Props::MV2c10.get(selectedJets.at(1));
    m_etree->SetBranchAndValue<float>("pTB2", j2.vec_corr.Pt() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("etaB2", j2.vec_corr.Eta(), -10.);
    m_etree->SetBranchAndValue<float>("phiB2", j2.vec_corr.Phi(), -10.);
    m_etree->SetBranchAndValue<float>("mB2", j2.vec_corr.M() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("MV2c10B2", j2MV2c10, -1.);
    m_etree->SetBranchAndValue<int>("flavB2", m_physicsMeta.b2Flav, -1);
    m_etree->SetBranchAndValue<float>("pTBB", H.vec_corr.Pt() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("etaBB", H.vec_corr.Eta(), -10.);
    m_etree->SetBranchAndValue<float>("phiBB", H.vec_corr.Phi(), -10.);
    m_etree->SetBranchAndValue<float>("mBB", H.vec_corr.M() / 1e3, -1.);
    m_etree->SetBranchAndValue<float>("dRBB", j1.vec_corr.DeltaR(j2.vec_corr),
                                      -1.);
    m_etree->SetBranchAndValue<float>("dPhiBB",
                                      j1.vec_corr.DeltaPhi(j2.vec_corr), -10.);
    m_etree->SetBranchAndValue<float>(
        "dEtaBB", fabs(j1.vec_corr.Eta() - j2.vec_corr.Eta()), -1.);
    m_etree->SetBranchAndValue<float>("dPhiVBB",
                                      fabs(W.vecT.DeltaPhi(H.vec_corr)), -1.);
    m_etree->SetBranchAndValue<float>(
        "mindPhilepB",
        std::min(fabs(lep.vec.DeltaPhi(j1.vec_corr)),
                 fabs(lep.vec.DeltaPhi(j2.vec_corr))),
        -1.);
    m_etree->SetBranchAndValue<float>("mVH", VH.vec_corr.M() / 1e3, -1.);

    // 3-jet events
    if (nSelectedJet > 2) {
      m_etree->SetBranchAndValue<float>("pTJ3", j3.vec_corr.Pt() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("etaJ3", j3.vec_corr.Eta(), -10.);
      m_etree->SetBranchAndValue<float>("phiJ3", j3.vec_corr.Phi(), -10.);
      m_etree->SetBranchAndValue<float>("mJ3", j3.vec_corr.M() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("mBBJ", bbj.vec_corr.M() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("pTBBJ", bbj.vec_corr.Pt() / 1e3, -1.);
    }

    if (m_doMJEtree) {  // MJ study variables - save if told to in config file
      float ptCone20 = -5;
      float topoCone20 = -5;
      if (nSigMu == 1) {
        ptCone20 = Props::ptcone20.get(mu);
        topoCone20 = Props::topoetcone20.get(mu);
      } else if (nSigEl == 1) {
        ptCone20 = Props::ptvarcone20.get(el);
        topoCone20 = Props::topoetcone20.get(el);
      }

      m_etree->SetBranchAndValue<float>("topoetcone20", topoCone20, -5);
      m_etree->SetBranchAndValue<float>("ptvarcone20", ptCone20, -5);
      m_etree->SetBranchAndValue<float>("lmetdPhi",
                                        fabs(lep.vec.DeltaPhi(met.vec)), -99);
      m_etree->SetBranchAndValue<double>("jjmindPhi", mindPhi, -99);
      m_etree->SetBranchAndValue<float>(
          "jmetdPhi", fabs(j1.vec_corr.DeltaPhi(met.vec)), -99);
      m_etree->SetBranchAndValue<float>(
          "jldPhi", fabs(j1.vec_corr.DeltaPhi(lep.vec)), -99);
    }

    if (m_writeObjectsInEasyTree) {
      EL_CHECK("AnalysisReader_VHcc1Lep::run_1Lep_analysis()",
               AnalysisReader_VHQQ::fillObjectBranches(signalJets, m_electrons,
                                                       m_muons, m_taus, m_met));
    }
    if (m_writeEasyTree) {
      if (m_histNameSvc->get_isNominal() &&
          (passSpecificCuts(eventFlagMerged, cuts_easytree_merged) ||
           passSpecificCuts(eventFlagResolved, cuts_easytree_resolved) ||
           m_writeObjectsInEasyTree)
          //         && passSpecificCuts(eventFlagMerged, {
          //         OneLeptonMergedCuts::AtLeast1FatJet,
          //         OneLeptonMergedCuts::AtLeast2TrackJets } )
          //         && (Props::nBTags.get(fatJets.at(0)) >= 1)
      ) {
        m_etree->Fill();
      }
    }
  }

  //*********************************************************//
  //                    HISTOGRAMS                           //
  //*********************************************************//

  if (m_debug) std::cout << " >>>>> Histogram Formation" << std::endl;

  // fill cutflow histogram
  if (m_doCutflow) {
    fill_1lepResolvedCutFlow(eventFlagResolved);
  }

  SystLevel ApplySysts = SystLevel::All;

  // uncomment to fill ftag efficiency histograms
  // if (m_isMC && m_histNameSvc->get_description() != "") {
  //   for (unsigned int jet_i = 0; jet_i < signalJets.size(); jet_i++) {
  //     const xAOD::Jet *jet = signalJets.at(jet_i);

  //     if (jet_i < 2) {
  //       AnalysisReader_VHQQ::fill_FtagEff_Hists(
  //           jet, eventweight_before_btagging_weight, "ctag");
  //     } else {
  //       AnalysisReader_VHQQ::fill_FtagEff_Hists(
  //           jet, eventweight_before_btagging_weight, "btag");
  //     }
  //   }
  // }
  // fill truth tagging histograms
  if (m_isMC && m_histNameSvc->get_description() != "" &&
      nSubleadingBtags == 0) {
    for (int n_truthtags = 1; n_truthtags < 3; n_truthtags++) {
      m_histNameSvc->set_nTag(n_truthtags);

      float weight = eventweight_before_btagging_weight *
                     btag_TT_weights["Nominal"][n_truthtags];

      int n_systs(0);

      if (do_btag_Syst) {  // before filling the histogram, we need to place the
                           // right variation value in m_weightSysts,
        // for every iteration, we need to replace the old values (for 0tag for
        // example) with the new values (for 1tag)
        for (auto effSF : btag_TT_weights) {
          std::string varName = effSF.first;
          if (varName == "Nominal") continue;

          m_weightSysts.push_back(
              {varName, btag_TT_weights[varName][n_truthtags] /
                            btag_TT_weights["Nominal"][n_truthtags]});
          n_systs++;  // count the systs so we can get rid of them after filling
                      // the histogram
        }
      }
      // Fill only 2 & 3 jets events
      BookFillHist_VHQQ1Lep("mBBTT", 100, 0, 500, H.vec_corr.M() / 1e3, weight,
                            ApplySysts);
      /*
      BookFillHist_VHQQ1Lep("pTVTT", 100, 0, 1000, W.vecT.Pt() / 1e3,
                            weight,
                            ApplySysts);
      BookFillHist_VHQQ1Lep("dRBBTT", 100, 0, 5,
      j1.vec_corr.DeltaR(j2.vec_corr),weight, ApplySysts);

      BookFillHist_VHQQ1Lep("etaB1TT", 60, -3, 3,
      j1.vec_corr.Eta(), weight,
                            ApplySysts);
      BookFillHist_VHQQ1Lep("etaB2TT", 60, -3, 3, j2.vec_corr.Eta(),
                            weight,
                            ApplySysts);
      BookFillHist_VHQQ1Lep("pTB1TT", 200, 0, 2000, j1.vec_corr.Pt() / 1e3,
                            weight,
                            ApplySysts);
      BookFillHist_VHQQ1Lep("pTB2TT", 150, 0, 1500, j2.vec_corr.Pt() / 1e3,
                            weight,
                            ApplySysts);
      */

      if (do_btag_Syst) {
        // remove the variations
        m_weightSysts.resize(m_weightSysts.size() - n_systs);
      }
    }

    // go back to direct tagging
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
  } else if (m_isMC == false && m_histNameSvc->get_description() != "" &&
             nSubleadingBtags == 0) {
    BookFillHist_VHQQ1Lep("mBBTT", 100, 0, 500, H.vec_corr.M() / 1e3, m_weight,
                          ApplySysts);
  }

  if ((m_histNameSvc->get_description() != "") && nSubleadingBtags == 0 &&
      tagcatExcl > 0) {
    // BookFillHist_VHQQ1Lep("pTV", 200, 0, 2000, W.vecT.Pt() / 1e3, m_weight,
    //                       SystLevel::None);
    BookFillHist_VHQQ1Lep("mBB", 100, 0, 500, H.vec_corr.M() / 1e3, m_weight,
                          ApplySysts);
    // BookFillHist_VHQQ1Lep("dRBB", 100, 0., 6.,
    // j1.vec_corr.DeltaR(j2.vec_corr), m_weight,
    //                       ApplySysts);
  }

  if (m_debug) std::cout << " >>>>> Formed Histogram Inputs" << std::endl;

  return EL::StatusCode::SUCCESS;
}  // run_1Lep_analysis

EL::StatusCode AnalysisReader_VHcc1Lep::initializeSelection() {
  // Read all variables that are relevant for the VHreso analysis from the
  // config and store the information in appropriate member variables.
  // The method consists of two parts. First, all default values are
  // initialised and then the information is read from the config.

  if (m_debug) {
    Info("initalizeSelection()", "Beginning of function initializeSelection()");
  }

  EL_CHECK("initializeSelection()",
           AnalysisReader_VHQQ1Lep::initializeSelection());

  m_config->getif<bool>("BDTSyst_debug", m_BDTSyst_debug);
  m_config->getif<double>("minElMET", m_minElMET);
  m_config->getif<double>("minMuMET", m_minMuMET);

  // Run the reader in inverted isolation mode?
  // (for multijet estimate)
  m_config->getif<bool>("doIsoInv", m_doIsoInv);
  // read in parameters from config and overwrite, if exist
  m_config->getif<bool>("doBJetEnergyCorr", m_doBJetEnergyCorr);
  m_config->getif<bool>("doMbbRescaling", m_doMbbRescaling);
  m_config->getif<bool>("doBlindingData", m_doBlindingData);
  m_config->getif<bool>("doBlindingMC", m_doBlindingMC);
  m_config->getif<bool>("doInputPlots", m_doInputPlots);
  m_config->getif<bool>("doTLVPlots", m_doTLVPlots);
  m_config->getif<bool>("doSplitLepFlavour", m_doSplitLepFlavour);
  m_config->getif<bool>("doSplitLepCharge", m_doSplitLepCharge);
  m_config->getif<bool>("doSplitWhfCR", m_doSplitWhfCR);
  m_config->getif<bool>("reduceHistograms", m_reduceHistograms);
  m_config->getif<bool>("doMJEtree", m_doMJEtree);
  m_config->getif<bool>("printEventWeightsForCutFlow",
                        m_printEventWeightsForCutFlow);
  m_config->getif<double>("mbbResolvedLowEdge", m_mbbResolvedLowEdge);
  m_config->getif<double>("mbbResolvedHighEdge", m_mbbResolvedHighEdge);
  m_config->getif<double>("mbbMergedLowEdge", m_mbbMergedLowEdge);
  m_config->getif<double>("mbbMergedHighEdge", m_mbbMergedHighEdge);
  m_config->getif<double>("mbbRestrictLowEdge", m_mbbRestrictLowEdge);
  m_config->getif<double>("mbbRestrictHighEdge", m_mbbRestrictHighEdge);
  m_config->getif<double>("mbbMergedLowerBound", m_mbbMergedLowerBound);
  m_config->getif<double>("mbbMergedUpperBound", m_mbbMergedUpperBound);
  m_config->getif<bool>("doMbbRejHist", m_doMbbRejHist);
  m_config->getif<double>("dataOneOverSF", m_dataOneOverSF);
  m_config->getif<bool>("ApplyFatJetMuOR", m_ApplyFatJetMuOR);
  m_config->getif<bool>("mVHvsPtvCut", m_mVHvsPtvCut);
  m_config->getif<bool>("writeMVATree", m_writeMVATree);
  m_config->getif<bool>("writeEasyTree", m_writeEasyTree);
  m_config->getif<bool>("doMergeCR", m_doMergeCR);
  m_config->getif<bool>("doCutflow", m_doCutflow);
  m_config->getif<bool>("nominalOnly", m_nominalOnly);
  m_config->getif<bool>("doRemoveDilepOverlap", m_doRemoveDilepOverlap);
  m_config->getif<bool>("useTTBarPTWFiltered1LepSamples",
                        m_useTTBarPTWFiltered1LepSamples);
  m_config->getif<bool>("applyPUWeight", m_applyPUWeight);
  m_config->getif<bool>("writeObjectsInEasyTree", m_writeObjectsInEasyTree);
  m_config->getif<bool>("GenerateSTXS", m_GenerateSTXS);

  m_config->getif<std::vector<std::string>>("BDTSystVariations",
                                            m_BDTSystVariations);

  if (m_debug) {
    Info("initalizeSelection()", "End of function initializeSelection()");
  }

  return EL::StatusCode::SUCCESS;
}

std::vector<std::pair<TString, Float_t>> AnalysisReader_VHcc1Lep::ApplyBDTSyst(
    int channel, std::vector<const xAOD::Jet *> selectedJets,
    TLorentzVector WVecT, TLorentzVector lepVec, float Mtop,
    TLorentzVector bbjVecCorr, TLorentzVector j3VecCorr, float dYWH,
    float btagWeight, int nJet) {
  TLorentzVector j1_truth, j2_truth, truth_metVec;

  std::vector<std::pair<TString, Float_t>> BDT_scores;

  if (m_truthMET && selectedJets.size() > 1) {
    j1_truth.SetPtEtaPhiM(selectedJets.at(0)->jetP4("TruthWZ").Pt(),
                          selectedJets.at(0)->jetP4("TruthWZ").Eta(),
                          selectedJets.at(0)->jetP4("TruthWZ").Phi(),
                          selectedJets.at(0)->jetP4("TruthWZ").M());
    j2_truth.SetPtEtaPhiM(selectedJets.at(1)->jetP4("TruthWZ").Pt(),
                          selectedJets.at(1)->jetP4("TruthWZ").Eta(),
                          selectedJets.at(1)->jetP4("TruthWZ").Phi(),
                          selectedJets.at(1)->jetP4("TruthWZ").M());

    m_BDTSyst->m_Variables["EventMod2"] = m_eventInfo->eventNumber() % 2;
    m_BDTSyst->m_Variables["EventNumber"] = m_eventInfo->eventNumber();
    m_BDTSyst->m_Variables["nJ"] = nJet;
    m_BDTSyst->m_Variables["EventWeight"] = m_weight;
    m_BDTSyst->m_Variables["btagWeight"] = btagWeight;
    m_BDTSyst->m_Variables["nTag"] = 2;
    m_BDTSyst->m_Variables["FlavorLabel"] = m_histNameSvc->getFlavorLabel();
    m_BDTSyst->m_Variables["mBB"] = (j1_truth + j2_truth).M() * 1e-03;
    m_BDTSyst->m_Variables["dRBB"] = (j1_truth).DeltaR(j2_truth);
    m_BDTSyst->m_Variables["dPhiVBB"] =
        fabs(WVecT.DeltaPhi(j1_truth + j2_truth));
    m_BDTSyst->m_Variables["dPhiLBmin"] = std::min(
        fabs(lepVec.DeltaPhi(j1_truth)), fabs(lepVec.DeltaPhi(j2_truth)));
    m_BDTSyst->m_Variables["pTV"] = WVecT.Pt() * 1e-03;
    m_BDTSyst->m_Variables["pTL"] = lepVec.Pt() * 1e-03;
    m_BDTSyst->m_Variables["pTB1"] = j1_truth.Pt() * 1e-03;
    m_BDTSyst->m_Variables["pTB2"] = j2_truth.Pt() * 1e-03;
    m_BDTSyst->m_Variables["mTW"] = WVecT.M() * 1e-03;
    m_BDTSyst->m_Variables["mTop"] = Mtop * 1e-03;
    m_BDTSyst->m_Variables["MET"] = m_truthMET->met() * 1e-03;  // Truth MET

    m_BDTSyst->m_Variables["dYWH"] = dYWH;
    m_BDTSyst->m_Variables["mBBJ"] = bbjVecCorr.M() * 1e-03;
    m_BDTSyst->m_Variables["pTJ3"] = j3VecCorr.Pt() * 1e-03;

    if (m_BDTSyst->m_Variables["nJ"] > 1 && m_BDTSyst->m_Variables["nJ"] < 4 &&
        m_BDTSyst->m_Variables["pTV"] > 150) {
      for (auto m_BDTSystVariation : m_BDTSystVariations) {
        if (m_BDTSyst->m_Variables["FlavorLabel"] < 2) {
          m_weightSysts.push_back(
              {"_" + m_BDTSystVariation + "__1up",
               m_BDTSyst->DetermineWeight(
                   m_BDTSystVariation, m_histNameSvc->get_sample(), channel,
                   m_BDTSyst->m_Variables["BDT_" + m_BDTSystVariation])});

          BDT_scores.push_back(std::make_pair(
              "BDT" + m_BDTSystVariation,
              m_BDTSyst->m_Variables["BDT_" + m_BDTSystVariation]));
        }

        else
          m_weightSysts.push_back({"_" + m_BDTSystVariation + "__1up", 1.});
      }
    }
  }

  return BDT_scores;
}
