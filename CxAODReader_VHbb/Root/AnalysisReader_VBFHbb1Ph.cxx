#include <EventLoop/IWorker.h>
#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/MVATree_VBFHbb.h"
#include "CxAODTools_VHbb/VBFHbb1phEvtSelection.h"
#include "TGraphAsymmErrors.h"
#include "TH1F.h"
#include "TH3D.h"

#include <CxAODReader_VHbb/AnalysisReader_VBFHbb1Ph.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

AnalysisReader_VBFHbb1Ph::AnalysisReader_VBFHbb1Ph() : AnalysisReader_VHQQ() {}

AnalysisReader_VBFHbb1Ph::~AnalysisReader_VBFHbb1Ph() {}

EL::StatusCode AnalysisReader_VBFHbb1Ph::initializeSelection() {
  AnalysisReader_VHQQ::initializeSelection();

  m_eventSelection = new VBFHbb1phEvtSelection();
  m_fillFunction = std::bind(&AnalysisReader_VBFHbb1Ph::fill_vbf1ph, this);

  //*********************************************************//
  //                     READ CONFIGURATION                  //
  //*********************************************************//
  m_bTagCut = 100;
  m_doCutBased = false;
  m_doMVAPreSel = false;
  m_doTrigStudy = false;
  m_doBlinding = false;
  m_checkOrthogonality = false;
  std::vector<std::string> bTagToolConfigs;

  m_config->getif<bool>("doCutBased", m_doCutBased);
  m_config->getif<bool>("doMVAPreSel", m_doMVAPreSel);
  m_config->getif<bool>("doTrigStudy", m_doTrigStudy);
  m_config->getif<float>("bTagCut", m_bTagCut);
  m_config->getif<bool>("doBlinding", m_doBlinding);
  m_config->getif<bool>("checkOrthogonality", m_checkOrthogonality);

  m_config->getif<std::vector<std::string> >(
      "bTagToolConfigs", bTagToolConfigs);  // get offline btag wp to choose the
                                            // correct bjet trigger SF

  m_config->getif<std::string>("CxAODTag",
                               m_CxAODTag);  // for backward compatibility

  //*********************************************************//
  //          files for bjet trigger efficiency              //
  //*********************************************************//

  TString name_BJetTrig_EFF_Jet = "g_Eff_offJets77_match_hlt_match_hlt77_jetPt";
  TString name_BJetTrig_SF_Jet = "g_SF_offJets77_match_hlt_match_hlt77_jetPt";
  TString name_BJetTrig_EFF_Event = "g_Eff_Event_leadingJet_jetEta";
  if (bTagToolConfigs.size() >= 4) {
    TString offline_btag_wp = bTagToolConfigs[1];
    Info("AnalysisReader_VBFHbb1Ph::initializeSelection()",
         "Found offline btagging working point: %s%%", offline_btag_wp.Data());
    name_BJetTrig_EFF_Jet =
        "g_Eff_offJets" + offline_btag_wp + "_match_hlt_match_hlt77_jetPt";
    name_BJetTrig_SF_Jet =
        "g_SF_offJets" + offline_btag_wp + "_match_hlt_match_hlt77_jetPt";
  } else {
    Error("AnalysisReader_VBFHbb1Ph::initializeSelection()",
          "Can not determine the offline btagging working point. Use 77%% for "
          "bjet trigger SF.");
  }

  // from
  // /afs/cern.ch/user/l/lmcclymo/public/bJetTriggers/2016_Effs/060317_effs/BJetTriggerEfficiencies-00-02-01.root
  // (Mar. 9, 2017)
  m_BJetTrigEff_file = new TFile(
      "$WorkDir_DIR/data/CxAODReader_VHbb/BJetTriggerEfficiencies.root");
  m_BJetTrig_EFF_Jet =
      (TGraphAsymmErrors*)m_BJetTrigEff_file->Get(name_BJetTrig_EFF_Jet);
  Info(
      "AnalysisReader_VBFHbb1Ph::initializeSelection()",
      "Loaded " + name_BJetTrig_EFF_Jet + " for bjet trigger data efficiency.");
  m_BJetTrig_SF_Jet =
      (TGraphAsymmErrors*)m_BJetTrigEff_file->Get(name_BJetTrig_SF_Jet);
  Info("AnalysisReader_VBFHbb1Ph::initializeSelection()",
       "Loaded " + name_BJetTrig_SF_Jet + " for bjet trigger scale factor.");
  m_BJetTrig_EFF_Event =
      (TGraphAsymmErrors*)m_BJetTrigEff_file->Get(name_BJetTrig_EFF_Event);
  Info("AnalysisReader_VBFHbb1Ph::initializeSelection()",
       "Loaded " + name_BJetTrig_EFF_Event +
           " for bjet trigger primary finding inefficiency/scale factor.");
  // bin structure of per jet efficiency
  float bin_pt[12] = {35.0,  50.0,  70.0,  90.0,  120.0, 150.0,
                      180.0, 240.0, 300.0, 400.0, 500.0, 700.0};
  hist_BJetTrig_EFF_Jet =
      new TH1F("BJetTrigEFF_Jet", "BJetTrigEFF_Jet", 11, bin_pt);
  // bin structure of per event efficiency
  float bin_eta[9] = {-2.5, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.5};
  hist_BJetTrig_EFF_Event =
      new TH1F("BJetTrigEFF_Event", "BJetTrigEFF_Event", 8, bin_eta);

  //*********************************************************//
  //                functions for systematics                //
  //*********************************************************//
  // apply TST uncertainty on HT_soft
  // taken from
  // http://atlas.web.cern.ch/Atlas/GROUPS/DATABASE/GroupData/METUtilities/data16_13TeV/rec_July16v1/SoftTermsSyst_TrackSoftTerm_FullSim-AFII_Comb20152016_v1.root
  m_SoftTermSyst_file = new TFile(
      "$WorkDir_DIR/data/CxAODReader_VHbb/"
      "vbfgamma_HTsoftSyst_SoftTermsSyst_TrackSoftTerms_NCB_v1.root");
  m_shiftpara_pthard_njet_mu = (TH3D*)m_SoftTermSyst_file->Get("shiftpara_TST");
  m_resopara_pthard_njet_mu = (TH3D*)m_SoftTermSyst_file->Get("resopara_TST");
  m_resoperp_pthard_njet_mu = (TH3D*)m_SoftTermSyst_file->Get("resoperp_TST");

  // function for L1EM trigger systematic
  f_TrigL1EMSF = new TF1("f_TrigL1EMSF",
                         "0.5*(TMath::Erf((x-[0])/(sqrt(2)*[1]))+1)", 25, 100);

  //*********************************************************//
  //         functions for kinematic reweighting             //
  //*********************************************************//
  // function for dEtaJJ reweighting
  // f_dEtaJJSF = new TF1("f_dEtaJJSF","0.7936+10.88*(exp(-0.9323*x))",1,9); //
  // full 2015+2016, 0+1tag, NewPU NJge4 (05feb17)
  f_dEtaJJSF = new TF1("f_dEtaJJSF", "0.8173+22.78*(exp(-1.155*x))", 0,
                       9);  // full 2015+2016, 0+1tag trigger, 77wp NJge4, ext
                            // Non-Res MC (08mar17)

  // function for mindRBPh reweighting
  // f_mindRBPhSF = new TF1("f_mindRBPhSF","0.1439 + (1.23*x) + ((-0.4683)*x*x)
  // + (0.05164*x*x*x)",0,9); // full 2015+2016, 0+1tag, NewPU NJge4 (05feb17)
  f_mindRBPhSF = new TF1(
      "f_mindRBPhSF", "0.1334 + (1.224*x) + ((-0.4524)*x*x) + (0.04774*x*x*x)",
      0, 9);  // full 2015+2016, 0+1tag trigger, 77wp NJge4, ext Non-Res MC
              // (08mar17)

  // function for pTJJ reweighting
  // f_pTJJSF = new TF1("f_pTJJSF","0.9298+0.4423*(exp(-0.01643*x))",1,9); //
  // full 2015+2016, 0+1tag, NewPU NJge4 (05feb17)
  f_pTJJSF = new TF1("f_pTJJSF", "0.9401+0.4294*(exp(-0.01745*x))", 20,
                     9999);  // full 2015+2016, 0+1tag trigger, 77wp NJge4, ext
                             // Non-Res MC (08mar17)

  // function for pTBal reweighting
  // f_pTBalSF = new TF1("f_pTBalSF","1.237 + ((-3.486)*x) + ((11.44)*x*x) +
  // ((-10.72)*x*x*x)",0.0,1.0); // full 2015+2016, 0+1tag, NewPU NJge4
  // (05feb17)
  f_pTBalSF = new TF1("f_pTBalSF",
                      "1.176 + ((-2.75)*x) + ((9.438)*x*x) + ((-8.894)*x*x*x)",
                      0.0, 1.0);  // full 2015+2016, 0+1tag trigger, 77wp NJge4,
                                  // ext Non-Res MC (08mar17)

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VBFHbb1Ph::fill_vbf1ph() {
  struct ptsort {
    bool operator()(TLorentzVector jet1, TLorentzVector jet2) {
      return (jet1.Pt() > jet2.Pt());
    }
  } m_ptsort;  // ptsort

  // m_debug = true;

  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;
  if (m_debug) std::cout << " >>>>>>>>>>> Starting fill_vbf1ph " << std::endl;
  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;

  //*********************************************************//
  //                     initialize cutflow                  //
  //*********************************************************//
  const int ncuts = 9;
  static std::string cuts[ncuts] = {"Pre",     "JetSel",   "SFs",
                                    "Trigger", "mJJ",      "pTBB",
                                    "SR2tag",  "Incl4cen", "Incl2cen"};

  //*********************************************************//
  //                     EVENT INITIALISATION                //
  //*********************************************************//

  float GeV = 1000.;
  float invGeV = 0.001;
  float pi = TMath::Pi();
  ResultVBFHbb1ph selectionResult =
      ((VBFHbb1phEvtSelection*)m_eventSelection)->result();
  m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "Pre", m_weight);
  m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts, "Pre", 1.);

  //*********************************************************//
  //      INIT ALL VARIABLES FROM THE START                  //
  //*********************************************************//

  const xAOD::Photon* photon = selectionResult.ph;
  std::vector<const xAOD::Jet*> jets = selectionResult.jets;
  std::vector<const xAOD::Jet*> trackJets = selectionResult.trackJets;
  // std::vector<const xAOD::Electron*> electrons = selectionResult.electrons;
  // std::vector<const xAOD::Muons*> muons        = selectionResults.muons;

  for (uint i = 0; i < jets.size(); ++i) {
    BTagProps::isTagged.set(jets.at(i), 0);
  }

  int tagcatExcl(-1), tagcatIncl(-1);
  std::vector<const xAOD::Jet*> signalJets, vbfJets, vbfJets2;

  //  if(! jet_selection_vbf(jets, signalJets, vbfJets, tagcatExcl,
  //  tagcatIncl)){
  if (!jet_selection_vbf4(jets, signalJets, vbfJets, vbfJets2, tagcatExcl,
                          tagcatIncl)) {
    // std::printf("failed jet selection vbf\n");
    return EL::StatusCode::SUCCESS;
  }
  m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "JetSel", m_weight);
  m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts, "JetSel", 1.);

  int tagCatExclDirect = tagcatExcl;

  TLorentzVector j1Vec, j2Vec, b1Vec, b2Vec, phVec;
  j1Vec = vbfJets.at(0)->p4();
  j2Vec = vbfJets.at(1)->p4();
  b1Vec = signalJets.at(0)->p4();
  b2Vec = signalJets.at(1)->p4();
  phVec = photon->p4();

  TLorentzVector b1VecCorr, b2VecCorr;
  b1VecCorr = b1Vec;
  b2VecCorr = b2Vec;

  getBJetEnergyCorrTLV(signalJets.at(0), b1VecCorr, false, m_jetCorrType);
  getBJetEnergyCorrTLV(signalJets.at(1), b2VecCorr, false, m_jetCorrType);

  TLorentzVector HVec, HVecCorr;
  HVec = b1Vec + b2Vec;
  HVecCorr = b1VecCorr + b2VecCorr;

  //*********************************************************//
  //                    CALCULATE BLINDING                   //
  //*********************************************************//
  /*
  float m_mBB = HVecCorr.M()*0.001;
  if(!m_isMC && m_doBlinding && m_mBB>80. && m_mBB<140.)
    {
      return EL::StatusCode::SUCCESS;
    }
  */
  //*********************************************************//
  //                    CALCULATE BTAGGING                   //
  //*********************************************************//

  if (m_doTruthTagging) {
    m_bTagTool->truth_tag_jets(m_eventInfo->eventNumber(), signalJets,
                               m_config);
    tagcatExcl = 2;

    //    // calculated for 77% WP
    //    if(m_mcChannel == 343392){
    //      //std::printf("applying TRF SF to NonRes\n");
    //      m_weight *= 1.11;
    //    }
  }

  //*********************************************************//
  //                    CALCULATE WEIGHTS                    //
  //*********************************************************//

  double photonSF = 1.;
  float jvtSF = 1.;
  float btagSF = 1.;
  float QGTaggerSFJ1 = 1.;
  float QGTaggerSFJ2 = 1.;
  float bkgSF = 1.;

  if (m_isMC) {
    // **Weight**: Photon
    photonSF = Props::tightEffSF.get(photon);
    m_weight *= photonSF;

    // **Weight** : jvt efficiency
    if (m_CxAODTag == "CxAODTag28") jvtSF = Props::JvtSF.get(m_eventInfo);
    m_weight *= jvtSF;

    // **Weight** : b-tagging
    btagSF = computeBTagSFWeight(signalJets, m_jetReader->getContainerName());
    m_weight *= btagSF;

    // **Weight** : nTrk weight systematic
    if (m_CxAODTag == "CxAODTag28") {
      QGTaggerSFJ1 = Props::QGTaggerWeight.get(vbfJets.at(0));
      QGTaggerSFJ2 = Props::QGTaggerWeight.get(vbfJets.at(1));
    }
    m_weight *= (QGTaggerSFJ1 * QGTaggerSFJ2);

    // **Weight** : pileup // handled by PU tool
    // float puWeight = 1.;
    // puWeight = m_pileupReweight;

    if (m_mcChannel == 344180) {  // reweight NonResbbjja to match data
      m_config->getif<float>("bkgSF", bkgSF);
      m_weight *= bkgSF;
    }
  }

  // SFs for kinematic variable reweighting are moved downwards after the
  // calculation of kinamatic variables, before filling MVATree

  if (m_debug) {
    std::cout << " >>>>> Applied Weights" << std::endl;
    std::printf("photonSF: %f\n", photonSF);
    std::printf("jvtSF: %f\n", jvtSF);
    std::printf("btagSF: %f\n", btagSF);
    std::printf("QGTaggerSFJ1: %f\n", QGTaggerSFJ1);
    std::printf("QGTaggerSFJ2: %f\n", QGTaggerSFJ2);
    std::printf("NonRes bkgSF: %f\n", bkgSF);
  }

  m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "SFs", m_weight);
  m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts, "SFs", 1.);

  //*********************************************************//
  //               CALCULATE EVENT VARIABLES                 //
  //*********************************************************//

  int Nvtx = 0;
  if (Props::NVtx2Trks.exists(m_eventInfo))
    Nvtx = Props::NVtx2Trks.get(m_eventInfo);
  else if (Props::NVtx3Trks.exists(m_eventInfo))
    Nvtx = Props::NVtx3Trks.get(m_eventInfo);

  // basic
  double m_mJJ = (j1Vec + j2Vec).M();
  double m_dEtaJJ = fabs(j1Vec.Eta() - j2Vec.Eta());
  double m_pTJJ = (j1Vec + j2Vec).Pt();

  float m_dRBB = b1Vec.DeltaR(b2Vec);
  float minEta = j1Vec.Eta() < j2Vec.Eta() ? j1Vec.Eta() : j2Vec.Eta();
  float maxEta = j1Vec.Eta() > j2Vec.Eta() ? j1Vec.Eta() : j2Vec.Eta();

  // Loop over trackJets: compute HT_soft, NtrkJet
  float m_HT_soft = 0;
  float nTrkJet2 = 0;
  float nTrkJet5 = 0;
  float nTrkJet10 = 0;
  std::pair<std::pair<float, float>, float> El(
      getCenterSlopeOfEllipse(signalJets.at(0), signalJets.at(1)));
  for (const auto* const jetPtr : trackJets) {
    if (trackJetIsInEllipse(El, jetPtr, m_dRBB, 0.4)) continue;
    if (jetPtr->p4().DeltaR(j1Vec) < 0.4) continue;
    if (jetPtr->p4().DeltaR(j2Vec) < 0.4) continue;
    if (jetPtr->p4().DeltaR(phVec) < 0.4) continue;
    nTrkJet2++;
    if (jetPtr->pt() > 5000.) nTrkJet5++;
    if (jetPtr->pt() > 7000.) m_HT_soft += jetPtr->pt();
    if (jetPtr->pt() > 10000.) nTrkJet10++;
  }

  // HT_soft systematic
  float m_HT_soft_SCALEUP = 0;
  float m_HT_soft_SCALEDOWN = 0;
  float m_HT_soft_RESOPARA = 0;
  float m_HT_soft_RESOPERP = 0;
  float ptHard = (j1Vec + j2Vec + b1Vec + b2Vec + phVec).Pt();
  int phbin = m_shiftpara_pthard_njet_mu->GetXaxis()->FindBin(ptHard);
  if (phbin > m_shiftpara_pthard_njet_mu->GetNbinsX())
    phbin = m_shiftpara_pthard_njet_mu->GetNbinsX();
  int const jetbin = 1;  // no dependence on jet container size
  int const mubin = 1;   // no dependence on mu
  double const ptHardShift =
      m_shiftpara_pthard_njet_mu->GetBinContent(phbin, jetbin, mubin);
  TRandom m_rand;
  double const randGaus = m_rand.Gaus(0., 1.);
  double const smearpara =
      m_resopara_pthard_njet_mu->GetBinContent(phbin, jetbin, mubin) * randGaus;
  double const smearperp =
      m_resoperp_pthard_njet_mu->GetBinContent(phbin, jetbin, mubin) * randGaus;
  m_HT_soft_SCALEUP = m_HT_soft + ptHardShift;
  m_HT_soft_SCALEDOWN = m_HT_soft - ptHardShift;
  if (m_HT_soft_SCALEDOWN < 0)
    m_HT_soft_SCALEDOWN = 0;  // avoid negative HT_soft
  m_HT_soft_RESOPARA = m_HT_soft + ptHardShift + smearpara;
  m_HT_soft_RESOPERP = m_HT_soft + ptHardShift + smearperp;

  // Loop over all calo jets: compute nJets, HT, nJetGap, HT_gap, pTJ5, mindR
  // between jets
  int nJets = 0;
  int nJets40 = 0;
  float HT = 0;
  int m_nJetGap = 0;
  float m_HT_gap = 0;
  float pTJ5 = -1;
  float m_SumPtExtraJets = 0;
  float m_mindRJ1J = 9999;  // mindR for all calo jets
  float m_mindRJ2J = 9999;
  float m_mindRB1J = 9999;
  float m_mindRB2J = 9999;
  float m_mindRJ1Ji =
      9999;  // mindR for vbf jets w.r.t extra calo jets (vbf incl definition)
  float m_mindRJ2Ji = 9999;
  float etaJ1cen =
      9999;  // leading central jet for applying event level bjet trigger SF
  float pTJ1cen = 9999;

  for (const auto* const jetPtr : jets) {
    // === all calo jets ===
    if (!Props::isVetoJet.get(jetPtr)) continue;
    float eta = jetPtr->p4().Eta();
    bool isInGap = (eta < maxEta) && (eta > minEta);
    bool isExtra = jetPtr != signalJets.at(0) && jetPtr != signalJets.at(1) &&
                   jetPtr != vbfJets.at(0) && jetPtr != vbfJets.at(1);
    nJets++;
    HT += jetPtr->pt();
    float dRJ1J = j1Vec.DeltaR(jetPtr->p4());
    float dRJ2J = j2Vec.DeltaR(jetPtr->p4());
    float dRB1J = b1Vec.DeltaR(jetPtr->p4());
    float dRB2J = b2Vec.DeltaR(jetPtr->p4());
    if ((jetPtr != vbfJets.at(0)) && dRJ1J < m_mindRJ1J) m_mindRJ1J = dRJ1J;
    if ((jetPtr != vbfJets.at(1)) && dRJ2J < m_mindRJ2J) m_mindRJ2J = dRJ2J;
    if ((jetPtr != signalJets.at(0)) && dRB1J < m_mindRB1J) m_mindRB1J = dRB1J;
    if ((jetPtr != signalJets.at(1)) && dRB2J < m_mindRB2J) m_mindRB2J = dRB2J;
    if (jetPtr != signalJets.at(0) && jetPtr != signalJets.at(1) &&
        jetPtr != vbfJets.at(0) && jetPtr != vbfJets.at(1)) {
      if (dRJ1J < m_mindRJ1Ji) m_mindRJ1Ji = dRJ1J;
      if (dRJ2J < m_mindRJ2Ji) m_mindRJ2Ji = dRJ2J;
    }

    // === jets pT>40 GeV ===
    if (jetPtr->pt() > 40 * GeV) {
      nJets40++;
    }

    // === leading central jet ===
    if (fabs(eta) < 2.5 && etaJ1cen > 9990) {
      etaJ1cen = eta;
      pTJ1cen = jetPtr->pt();
    }

    // === calo jets in VBF jet gap ===
    if (isInGap) {
      m_nJetGap++;
    }

    // === calo jets that are not signal/vbf jets ===
    if (isExtra) {
      if (pTJ5 < 0) pTJ5 = jetPtr->pt();
    }

    // === extra calo jets in VBF jet gap ===
    if (isInGap && isExtra) {
      m_HT_gap += jetPtr->pt();
      if (fabs(eta) < 2.5) m_SumPtExtraJets += jetPtr->pt();
    }
  }
  HT += phVec.Pt();
  // if no extra jets
  if (m_mindRJ1Ji > 9000) m_mindRJ1Ji = -1;
  if (m_mindRJ2Ji > 9000) m_mindRJ2Ji = -1;

  // pTBal
  float m_pTBal = (b1Vec + b2Vec + j1Vec + j2Vec + phVec).Pt();
  m_pTBal /= (b1Vec.Pt() + b2Vec.Pt() + j1Vec.Pt() + j2Vec.Pt() + phVec.Pt());

  // centrality
  float m_cenHJJ =
      fabs(HVec.Rapidity() - 0.5 * (j1Vec.Rapidity() + j2Vec.Rapidity()));
  m_cenHJJ /= fabs(j1Vec.Rapidity() - j2Vec.Rapidity());

  float m_cenHgJJ = fabs((HVec + phVec).Rapidity() -
                         0.5 * (j1Vec.Rapidity() + j2Vec.Rapidity()));
  m_cenHgJJ /= fabs(j1Vec.Rapidity() - j2Vec.Rapidity());

  float m_cenPhJJ =
      fabs(phVec.Rapidity() - 0.5 * (j1Vec.Rapidity() + j2Vec.Rapidity()));
  m_cenPhJJ /= fabs(j1Vec.Rapidity() - j2Vec.Rapidity());

  float m_dPhiPhBmin =
      (fabs(b1Vec.DeltaPhi(phVec)) < fabs(b2Vec.DeltaPhi(phVec))
           ? fabs(b1Vec.DeltaPhi(phVec))
           : fabs(b2Vec.DeltaPhi(phVec)));

  // dEtaJJBB
  float etaJJ = (j1Vec.Eta() * j1Vec.Pt() + j2Vec.Eta() * j2Vec.Pt()) /
                (j1Vec.Pt() + j2Vec.Pt());
  float etaBB = (b1Vec.Eta() * b1Vec.Pt() + b2Vec.Eta() * b2Vec.Pt()) /
                (b1Vec.Pt() + b2Vec.Pt());
  float dEtaJJBB = fabs(etaJJ - etaBB);

  float dPhiBBJJ = fabs(HVec.DeltaPhi(j1Vec + j2Vec));

  // pTTot
  float pTTot = (j1Vec + j2Vec + b1Vec + b2Vec + phVec).Pt();
  float pZTot = fabs((j1Vec + j2Vec + b1Vec + b2Vec + phVec).Pz());

  // pTJ4 for trigger systematic
  float pTJ4 = (jets.at(3))->pt();

  // etaJStar
  float etaJStar = 0.5 * (fabs(j1Vec.Eta()) + fabs(j2Vec.Eta())) -
                   0.5 * (fabs(b1Vec.Eta()) + fabs(b2Vec.Eta()));

  // cosTheta CMS def
  TLorentzVector rest4j = b1Vec + b2Vec + j1Vec + j2Vec;
  TVector3 boostVec4j = -(rest4j.BoostVector());
  TLorentzVector b1Vec4j = b1Vec;
  TLorentzVector b2Vec4j = b2Vec;
  TLorentzVector j1Vec4j = j1Vec;
  TLorentzVector j2Vec4j = j2Vec;
  b1Vec4j.Boost(boostVec4j);
  b2Vec4j.Boost(boostVec4j);
  j1Vec4j.Boost(boostVec4j);
  j2Vec4j.Boost(boostVec4j);
  TVector3 b1Vec3 = b1Vec4j.Vect();
  TVector3 b2Vec3 = b2Vec4j.Vect();
  TVector3 j1Vec3 = j1Vec4j.Vect();
  TVector3 j2Vec3 = j2Vec4j.Vect();
  TVector3 b1xb2 = b1Vec3.Cross(b2Vec3);
  TVector3 j1xj2 = j1Vec3.Cross(j2Vec3);
  float cosTheta_CMS = b1xb2 * j1xj2 / (b1xb2.Mag() * j1xj2.Mag());

  // cosTheta ATLAS def
  TLorentzVector j1VecH = j1Vec;
  TLorentzVector j2VecH = j2Vec;
  TVector3 boostVecH = -(HVec.BoostVector());
  j1VecH.Boost(boostVecH);
  j2VecH.Boost(boostVecH);
  TVector3 j1VecH3 = j1VecH.Vect();
  TVector3 j2VecH3 = j2VecH.Vect();
  TVector3 j1xj2p = j1VecH3.Cross(j2VecH3);
  TVector3 HVec3 = HVec.Vect();
  float cosTheta_ATLAS = j1xj2p.Dot(HVec3) / (j1xj2p.Mag() * HVec3.Mag());

  // new variables for MVA

  // nTrk in jet
  int m_nTrkPt5J1 = (j1Vec.Pt() > 50. * GeV && fabs(j1Vec.Eta()) < 2.1)
                        ? Props::NumTrkPt500PV.get(vbfJets.at(0))
                        : -1;
  int m_nTrkPt5J2 = (j2Vec.Pt() > 50. * GeV && fabs(j2Vec.Eta()) < 2.1)
                        ? Props::NumTrkPt500PV.get(vbfJets.at(1))
                        : -1;
  int m_nTrkPt10J1 = (j1Vec.Pt() > 50. * GeV && fabs(j1Vec.Eta()) < 2.1)
                         ? Props::NumTrkPt1000PV.get(vbfJets.at(0))
                         : -1;
  int m_nTrkPt10J2 = (j2Vec.Pt() > 50. * GeV && fabs(j2Vec.Eta()) < 2.1)
                         ? Props::NumTrkPt1000PV.get(vbfJets.at(1))
                         : -1;
  if (m_CxAODTag == "CxAODTag28") {
    m_nTrkPt5J1 = Props::QGTagger.get(vbfJets.at(0));
    m_nTrkPt5J2 = Props::QGTagger.get(vbfJets.at(1));
  }

  // mindR jet photon
  float m_mindRBPh = (b1Vec.DeltaR(phVec) < b2Vec.DeltaR(phVec))
                         ? b1Vec.DeltaR(phVec)
                         : b2Vec.DeltaR(phVec);
  float m_mindRJPh = (j1Vec.DeltaR(phVec) < j2Vec.DeltaR(phVec))
                         ? j1Vec.DeltaR(phVec)
                         : j2Vec.DeltaR(phVec);
  float m_dR_BPhovJPh = (m_mindRJPh > 0) ? m_mindRBPh / m_mindRJPh : 0;

  // systematic weights
  float m_forwardJetScale_up(1), m_forwardJetScale_down(1), m_pTBalScale_up(1),
      m_pTBalScale_down(1), m_pTBalPS_up(1), m_pTBalPS_down(1);
  if (m_mcChannel == 344177) {  // HbbjjaSM125
    m_forwardJetScale_up = 1.012 * (1.17132 - 1.15424e-07 * m_mJJ);
    m_forwardJetScale_down = 1.005 * (0.926557 + 4.28189e-08 * m_mJJ);
    m_pTBalScale_up =
        0.990 *
        (m_pTBal < 0.30
             ? (0.799064 - 0.510465 * m_pTBal + 14.9522 * m_pTBal * m_pTBal)
             : (0.799064 - 0.510465 * 0.3 + 14.9522 * 0.3 * 0.3));
    m_pTBalScale_down = 1.063 * (m_pTBal < 0.18 ? (1.53911 - 6.12413 * m_pTBal)
                                                : (1.53911 - 6.12413 * 0.18));
    m_pTBalPS_up =
        0.988 *
        (m_pTBal < 0.28
             ? (0.957859 - 0.444295 * m_pTBal + 5.94812 * m_pTBal * m_pTBal)
             : (0.957859 - 0.444295 * 0.28 + 5.94812 * 0.28 * 0.28));
    m_pTBalPS_down =
        1.009 *
        (m_pTBal < 0.28
             ? (1.0699 - 0.703138 * m_pTBal - 0.131708 * m_pTBal * m_pTBal)
             : (1.0699 - 0.703138 * 0.28 - 0.131708 * 0.28 * 0.28));
  }
  if (m_mcChannel == 344178) {  // ZbbjjaEWK
    m_forwardJetScale_up = 1.016 * (1.12535 - 8.88659e-08 * m_mJJ);
    m_forwardJetScale_down = 1.021 * (0.93262 + 2.9634e-08 * m_mJJ);
    m_pTBalScale_up =
        1.037 *
        (m_pTBal < 0.30
             ? (0.819056 + 0.0426767 * m_pTBal + 8.6885 * m_pTBal * m_pTBal)
             : (0.819056 + 0.0426767 * 0.3 + 8.6885 * 0.3 * 0.3));
    m_pTBalScale_down = 1.003 * (m_pTBal < 0.18 ? (1.44614 - 5.02236 * m_pTBal)
                                                : (1.44614 - 5.02236 * 0.18));
    m_pTBalPS_up =
        0.994 *
        (m_pTBal < 0.28
             ? (0.914215 + 0.755625 * m_pTBal + 1.11801 * m_pTBal * m_pTBal)
             : (0.914215 + 0.755625 * 0.28 + 1.11801 * 0.28 * 0.28));
    m_pTBalPS_down =
        0.993 *
        (m_pTBal < 0.28
             ? (1.0259 + 0.256341 * m_pTBal - 2.75529 * m_pTBal * m_pTBal)
             : (1.0259 + 0.256341 * 0.28 - 2.75529 * 0.28 * 0.28));
  }
  if (m_mcChannel == 344179) {  // ZbbjjaQCD
    m_pTBalScale_up = 0.971 * (m_pTBal < 0.18 ? (0.702844 + 2.52501 * m_pTBal)
                                              : (0.702844 + 2.52501 * 0.18));
    m_pTBalScale_down =
        1.012 *
        (m_pTBal < 0.30
             ? (1.22333 - 0.255027 * m_pTBal - 5.08028 * m_pTBal * m_pTBal)
             : (1.22333 - 0.255027 * 0.3 - 5.08028 * 0.3 * 0.3));
  }

  // jet trigger efficiency systematic
  float m_TrigJetEff_up(1), m_TrigJetEff_down(1);
  if (pTJ4 < 50 * GeV) {
    m_TrigJetEff_up = 1.02;
    m_TrigJetEff_down = 0.98;
  }

  // L1_EM22VHI trigger efficiency systematic (DOWN only)
  f_TrigL1EMSF->SetParameters(23.165, 4.935);
  float m_TrigL1EMEffon(1);
  if (phVec.Pt() < 40 * GeV) {
    m_TrigL1EMEffon = f_TrigL1EMSF->Eval(phVec.Pt() * invGeV);
  }

  //*********************************************************//
  //               CALCULATE TRIGGER VARIABLES               //
  //*********************************************************//

  // analysis trigger ICHEP 2016
  //  int passTrigger =
  //  Props::passHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700.get(m_eventInfo);
  //  int matchTrigger =
  //  Props::matchHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700.get(photon);
  int passTrigger = 0;
  int matchTrigger = 0;

  bool match_ph =
      Props::
          matchPhHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700
              .get(photon);
  bool match_b1 =
      Props::
          matchBJetHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700
              .get(signalJets.at(0));
  bool match_b2 =
      Props::
          matchBJetHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700
              .get(signalJets.at(1));
  double bjetTrigPt_SF = 1.;
  double weight_bjetTrigPt_SFup = 1.;
  double weight_bjetTrigPt_SFdown = 1.;
  double bjetTrigEta_SF = 1.;
  double weight_bjetTrigEta_SFup = 1.;
  double weight_bjetTrigEta_SFdown = 1.;
  if (m_isMC &&
      m_doTrigStudy) {  // for trigger study, store the nominal SFs for all
                        // events, but do not apply in EventWeight.
    bjetTrigPt_SF = GetBjetTriggerWeightPt(
        (b1Vec.Pt() * 0.001), (b2Vec.Pt() * 0.001), match_b1, match_b2);
    bjetTrigEta_SF = GetBjetTriggerWeightEta(etaJ1cen);
  }

  int trigRunNumber = m_randomRunNumber;
  if (!m_isMC) trigRunNumber = m_eventInfo->runNumber();
  if (trigRunNumber < 0) {
    std::cout << "WARNING: Failed to retrieve a valid RunNumber for trigger. "
                 "Force to use 0btag trigger. Please check!!!"
              << std::endl;
  }

  // 0+1-tag trigger scheme Moriond 2017
  if (trigRunNumber <= 304494) {  // 0-tag for data15, data16 Peroid ABCDEF
    passTrigger = Props::passHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700.get(
        m_eventInfo);
    matchTrigger =
        Props::matchHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700.get(photon);
  } else {  // 1-tag for Perhid GIKL
    // passTrigger =
    // Props::
    // passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700
    // .get(m_eventInfo);

    passTrigger =
        Props::
            passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700
                .get(m_eventInfo) ||
        Props::
            passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700
                .get(m_eventInfo) ||
        Props::
            passHLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500
                .get(m_eventInfo);
    matchTrigger = match_ph && (match_b1 || match_b2);
    if (m_isMC &&
        !m_doTrigStudy) {  // for physics analysis, apply the nominal SF to
                           // EventWeight, and store the variation wrt nominal
                           // in separate variables
      // event level SF from per-jet SF(pT)
      bjetTrigPt_SF = GetBjetTriggerWeightPt(
          (b1Vec.Pt() * 0.001), (b2Vec.Pt() * 0.001), match_b1, match_b2);
      weight_bjetTrigPt_SFup =
          GetBjetTriggerWeightPt((b1Vec.Pt() * 0.001), (b2Vec.Pt() * 0.001),
                                 match_b1, match_b2, 1) /
          bjetTrigPt_SF;
      weight_bjetTrigPt_SFdown =
          GetBjetTriggerWeightPt((b1Vec.Pt() * 0.001), (b2Vec.Pt() * 0.001),
                                 match_b1, match_b2, -1) /
          bjetTrigPt_SF;
      // event level SF from per-event SF(eta)
      bjetTrigEta_SF = GetBjetTriggerWeightEta(etaJ1cen);
      weight_bjetTrigEta_SFup = GetBjetTriggerWeightEta(etaJ1cen, 1) /
                                bjetTrigEta_SF;  // store variation wrt nominal
      weight_bjetTrigEta_SFdown =
          GetBjetTriggerWeightEta(etaJ1cen, -1) / bjetTrigEta_SF;
      // nominal event weight
      m_weight *= (bjetTrigPt_SF * bjetTrigEta_SF);
    }
  }

  // extra information for trigger studies
  // for trigger emulation
  float m_l1_emet = -99;
  float m_hlt_phpt = -99;
  float m_hlt_j1pt = -99;
  float m_hlt_j2pt = -99;
  float m_hlt_j3pt = -99;
  float m_hlt_j4pt = -99;
  float m_hlt_j5pt = -99;
  float m_hlt_mjj = -99;
  // for trigger efficiency
  float m_hlt_phpt_matched = -99;
  float m_hlt_j4pt_phor = -99;
  float m_hlt_mjj_phor = -99;
  TLorentzVector m_trigPh_matched;
  std::vector<TLorentzVector> m_trigJet_phor;
  m_trigJet_phor.clear();
  std::vector<int> m_trigJetIdx_failOR;
  m_trigJetIdx_failOR.clear();

  if (m_doTrigStudy) {
    // L1 EM
    //	 std::vector<float> l1_emet = Props::L1_EMEt.get(m_eventInfo);
    //	 std::vector<float> l1_emeta = Props::L1_EMEta.get(m_eventInfo);
    //	 std::vector<float> l1_emphi = Props::L1_EMPhi.get(m_eventInfo);
    //	 if(l1_emet.size()){
    //		std::sort(l1_emet.begin(), l1_emet.end());
    //		std::reverse(l1_emet.begin(), l1_emet.end());
    //		m_l1_emet = l1_emet.at(0);
    //	 }

    // HLT photon
    std::vector<float> hlt_phpt = Props::HLT_PhotonPt.get(m_eventInfo);
    std::vector<float> hlt_pheta = Props::HLT_PhotonEta.get(m_eventInfo);
    std::vector<float> hlt_phphi = Props::HLT_PhotonPhi.get(m_eventInfo);
    if (hlt_phpt.size()) {
      // for emulation
      std::sort(hlt_phpt.begin(), hlt_phpt.end());
      std::reverse(hlt_phpt.begin(), hlt_phpt.end());
      m_hlt_phpt = hlt_phpt.at(0);
      // for efficiency study: select hlt photon matched to offline selected
      // photon.
      for (unsigned int iph = 0; iph < hlt_phpt.size(); ++iph) {
        TLorentzVector trigphoton;
        trigphoton.SetPtEtaPhiM(hlt_phpt.at(iph), hlt_pheta.at(iph),
                                hlt_phphi.at(iph), 0);
        if (trigphoton.DeltaR(phVec) > 0.07) continue;
        if (trigphoton.Pt() > m_trigPh_matched.Pt()) {
          m_trigPh_matched = trigphoton;
        }
      }
      m_hlt_phpt_matched = m_trigPh_matched.Pt();
    }

    // HLT jets
    std::vector<float> hlt_jetpt = Props::HLT_JetPt.get(m_eventInfo);
    std::vector<float> hlt_jeteta = Props::HLT_JetEta.get(m_eventInfo);
    std::vector<float> hlt_jetphi = Props::HLT_JetPhi.get(m_eventInfo);
    std::vector<float> hlt_jetpt_sorted = hlt_jetpt;
    std::sort(hlt_jetpt_sorted.begin(), hlt_jetpt_sorted.end());
    std::reverse(hlt_jetpt_sorted.begin(), hlt_jetpt_sorted.end());
    if (hlt_jetpt.size() > 0) {
      // for emulation
      m_hlt_j1pt = hlt_jetpt_sorted.at(0);
      if (hlt_jetpt_sorted.size() > 1) {
        m_hlt_j2pt = hlt_jetpt_sorted.at(1);
        if (hlt_jetpt_sorted.size() > 2) {
          m_hlt_j3pt = hlt_jetpt_sorted.at(2);
          if (hlt_jetpt_sorted.size() > 3) {
            m_hlt_j4pt = hlt_jetpt_sorted.at(3);
            if (hlt_jetpt_sorted.size() > 4) {
              m_hlt_j5pt = hlt_jetpt_sorted.at(4);
            }
          }
        }
      }
      // for efficiency study: select hlt jets not overlap with the selected hlt
      // photon
      for (unsigned int ijet = 0; ijet < hlt_jetpt.size(); ++ijet) {
        TLorentzVector trigjet;
        trigjet.SetPtEtaPhiM(hlt_jetpt.at(ijet), hlt_jeteta.at(ijet),
                             hlt_jetphi.at(ijet), 0);
        if (trigjet.DeltaR(m_trigPh_matched) < 0.4) {
          m_trigJetIdx_failOR.push_back(ijet);
          continue;
        }
        m_trigJet_phor.push_back(trigjet);
      }
      if (m_trigJet_phor.size() >= 4) {
        std::sort(m_trigJet_phor.begin(), m_trigJet_phor.end(), m_ptsort);
        m_hlt_j4pt_phor = (m_trigJet_phor.at(3)).Pt();
      }
    }

    // HLT_mJJ
    for (unsigned int i = 0; i < hlt_jetpt.size(); ++i) {
      TLorentzVector j1, j2;
      j1.SetPtEtaPhiM(hlt_jetpt.at(i), hlt_jeteta.at(i), hlt_jetphi.at(i), 0);
      for (unsigned int j = i + 1; j < hlt_jetpt.size(); ++j) {
        j2.SetPtEtaPhiM(hlt_jetpt.at(j), hlt_jeteta.at(j), hlt_jetphi.at(j), 0);
        float tmp_mjj = (j1 + j2).M();
        if (tmp_mjj > m_hlt_mjj) m_hlt_mjj = tmp_mjj;  // for emulation
        if (tmp_mjj > m_hlt_mjj_phor) {                // for efficiency study
          bool j1phor =
              std::find(m_trigJetIdx_failOR.begin(), m_trigJetIdx_failOR.end(),
                        i) != m_trigJetIdx_failOR.end();
          bool j2phor =
              std::find(m_trigJetIdx_failOR.begin(), m_trigJetIdx_failOR.end(),
                        j) != m_trigJetIdx_failOR.end();
          if (!j1phor && !j2phor) m_hlt_mjj_phor = tmp_mjj;
        }
      }
    }
  }

  // Calculate MVA Pre selction
  int m_passMVAPreSel = 1;
  if (!passTrigger) m_passMVAPreSel = 0;

  // Currently, trigger match seems not working for 2017 trigger, to be
  // investigated, currently comment out if (!matchTrigger) m_passMVAPreSel = 0;
  if (m_passMVAPreSel) {
    m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "Trigger",
                               m_weight);
    m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts, "Trigger",
                               1.);
  }

  // Remove 30 GeV photon pt Cut
  // if (phVec.Pt() < 30. * GeV) m_passMVAPreSel = 0;
  if ((j1Vec + j2Vec).M() < 800. * GeV) m_passMVAPreSel = 0;
  if (m_passMVAPreSel) {
    m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "mJJ", m_weight);
    m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts, "mJJ", 1.);
  }
  if (HVec.Pt() < 80. * GeV) m_passMVAPreSel = 0;
  if (m_passMVAPreSel) {
    m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "pTBB", m_weight);
    m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts, "pTBB", 1.);
  }
  if (m_passMVAPreSel && tagcatExcl >= 2) {
    m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "SR2tag", m_weight);
    m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts, "SR2tag",
                               1.);
  }

  int m_passIncl4cenSel = 0;
  int m_passIncl2cenSel = 0;
  if (m_checkOrthogonality) {
    m_passIncl4cenSel = jet_selection_vbfincl4cen(jets);
    m_passIncl2cenSel = jet_selection_vbfincl2cen(jets);
    if (m_passMVAPreSel && tagcatExcl >= 2) {  // pass VBF+photon selection
      if (m_passIncl4cenSel) {
        m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "Incl4cen",
                                   m_weight);
        m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts,
                                   "Incl4cen", 1.);
      }
      if (m_passIncl2cenSel) {
        m_histSvc->BookFillCutHist("CutFlow/Cuts", ncuts, cuts, "Incl2cen",
                                   m_weight);
        m_histSvc->BookFillCutHist("CutFlow/CutsNoWeight", ncuts, cuts,
                                   "Incl2cen", 1.);
      }
    }  // end of pass VBF+photon selection
  }

  //*********************************************************//
  //            Kinematic variables reweighting              //
  //*********************************************************//

  bool doKinematicRW = false;
  m_config->getif<bool>("doKinematicRW", doKinematicRW);

  float dEtaJJSF = 1.;
  float mindRBPhSF = 1.;
  float pTJJSF = 1.;
  float pTBalSF = 1.;

  if (doKinematicRW && m_mcChannel == 344180) {
    //	 // **reweight dEtaJJ **
    //	 if (m_dEtaJJ < 1.3) { dEtaJJSF = f_dEtaJJSF->Eval(1.3); }
    //	 else { dEtaJJSF = f_dEtaJJSF->Eval(m_dEtaJJ); }
    //
    //	 // **reweight mindRBPh **
    //	 mindRBPhSF = f_mindRBPhSF->Eval(m_mindRBPh);
    //
    //	 // **reweight pTJJ **
    //	 pTJJSF = f_pTJJSF->Eval(m_pTJJ*0.001);
    //
    //	 // **reweight pTBal **
    //	 if ((m_pTBal > 0.0) && (m_pTBal < 1.0)) { pTBalSF =
    // f_pTBalSF->Eval(m_pTBal); } 	 else { pTBalSF = 1; }

    // **reweight dEtaJJ **
    if ((m_dEtaJJ > 0.0) && (m_dEtaJJ < 9.0)) {
      if (m_dEtaJJ < 1.9) {
        dEtaJJSF = f_dEtaJJSF->Eval(1.9);
      }  // full 2015+2016, 0+1tag trigger, NJge4, ext Non-Res MC (08mar17)
      else {
        dEtaJJSF = f_dEtaJJSF->Eval(m_dEtaJJ);
      }
    }

    // **reweight mindRBPh **
    if ((m_mindRBPh > 0.4) && (m_mindRBPh < 6.0)) {
      mindRBPhSF = f_mindRBPhSF->Eval(m_mindRBPh);
    }

    // **reweight pTJJ **
    if ((m_pTJJ * 0.001) > 20.0) {
      pTJJSF = f_pTJJSF->Eval(m_pTJJ * 0.001);
    }

    // **reweight pTBal **
    if ((m_pTBal > 0.0) && (m_pTBal < 1.0)) {
      pTBalSF = f_pTBalSF->Eval(m_pTBal);
    }
    if (pTBalSF < 0.1) {
      pTBalSF = 0.1;
    }

    m_weight *= dEtaJJSF;
    m_weight *= mindRBPhSF;
    m_weight *= pTJJSF;
    m_weight *= pTBalSF;
  }

  //*********************************************************//
  //                       MVA TREE                          //
  //*********************************************************//
/*
  if (m_debug) std::cout << " >>>>> MVA Tree" << std::endl;

  m_tree_vbf->Reset();
  m_tree_vbf->SetVariation(m_currentVar);

  m_tree_vbf->passTrig = passTrigger;
  m_tree_vbf->matchTrig = matchTrigger;
  m_tree_vbf->passMVAPreSel = m_passMVAPreSel;

  m_tree_vbf->sample = m_histNameSvc->getFullSample();
  m_tree_vbf->MCChannelNumber = m_mcChannel;
  m_tree_vbf->randomRunNumber = m_randomRunNumber;
  m_tree_vbf->RunNumber = m_eventInfo->runNumber();
  m_tree_vbf->EventNumber = m_eventInfo->eventNumber();
  m_tree_vbf->EventWeight = m_weight;
  m_tree_vbf->dEtaJJSF = dEtaJJSF;
  m_tree_vbf->mindRBPhSF = mindRBPhSF;
  m_tree_vbf->pTJJSF = pTJJSF;
  m_tree_vbf->pTBalSF = pTBalSF;
  m_tree_vbf->bjetTrigPt_SF = bjetTrigPt_SF;
  m_tree_vbf->bjetTrigEta_SF = bjetTrigEta_SF;

  m_tree_vbf->AverageMu = m_averageMu;
  m_tree_vbf->ActualMu = m_actualMu;
  m_tree_vbf->AverageMuScaled = m_averageMuScaled;
  m_tree_vbf->ActualMuScaled = m_actualMuScaled;
  m_tree_vbf->Nvtx = Nvtx;
  m_tree_vbf->ZPV = Props::ZPV.get(m_eventInfo);

  m_tree_vbf->tagCatExcl = tagcatExcl;
  m_tree_vbf->tagCatExclDirect = tagCatExclDirect;

  //  m_tree_vbf->nJ              = nJets;
  m_tree_vbf->nJets = nJets40;
  m_tree_vbf->mJJ = m_mJJ;  //(j1Vec + j2Vec).M();
  m_tree_vbf->pTJJ = m_pTJJ;
  m_tree_vbf->dEtaJJ = m_dEtaJJ;  // fabs(j1Vec.Eta() - j2Vec.Eta());
  m_tree_vbf->dRJJ = j1Vec.DeltaR(j2Vec);
  m_tree_vbf->dPhiJJ = fabs(j1Vec.DeltaPhi(j2Vec));
  m_tree_vbf->maxEta = TMath::Max(fabs(j1Vec.Eta()), fabs(j2Vec.Eta()));

  m_tree_vbf->mBB = HVecCorr.M();
  m_tree_vbf->mBBNoCorr = (b1Vec + b2Vec).M();
  m_tree_vbf->dRBB = m_dRBB;
  m_tree_vbf->dPhiBB = fabs(b1Vec.DeltaPhi(b2Vec));
  m_tree_vbf->dEtaBB = fabs(b1Vec.Eta() - b2Vec.Eta());
  m_tree_vbf->pTBB = HVec.Pt();
  m_tree_vbf->dPhiPhBmin = m_dPhiPhBmin;
  m_tree_vbf->mBBPh = (HVecCorr + phVec).M();

  //  m_tree_vbf->nJetGap     = m_nJetGap;
  //  m_tree_vbf->HT_gap      = m_HT_gap;
  m_tree_vbf->HT_soft = m_HT_soft;
  //  m_tree_vbf->HT_soft_SCALEUP     = m_HT_soft_SCALEUP;
  //  m_tree_vbf->HT_soft_SCALEDOWN     = m_HT_soft_SCALEDOWN;
  //  m_tree_vbf->HT_soft_RESOPARA     = m_HT_soft_RESOPARA;
  //  m_tree_vbf->HT_soft_RESOPERP     = m_HT_soft_RESOPERP;
  m_tree_vbf->pTBal = m_pTBal;

  //  m_tree_vbf->nTrkJet2     = nTrkJet2;
  //  m_tree_vbf->nTrkJet5     = nTrkJet5;
  //  m_tree_vbf->nTrkJet10     = nTrkJet10;
  m_tree_vbf->dEtaJJBB = dEtaJJBB;
  m_tree_vbf->dPhiBBJJ = dPhiBBJJ;
  m_tree_vbf->pTTot = pTTot;
  m_tree_vbf->pZTot = pZTot;
  m_tree_vbf->HT = HT;
  m_tree_vbf->pTJ5 = pTJ5;
  //  m_tree_vbf->pTJ4        = pTJ4;
  m_tree_vbf->etaJStar = etaJStar;
  m_tree_vbf->cosThetaC = fabs(cosTheta_CMS);
  //  m_tree_vbf->cosThetaA   = fabs(cosTheta_ATLAS);
  //
  //  m_tree_vbf->cenHJJ      = m_cenHJJ;
  //  m_tree_vbf->cenHgJJ     = m_cenHgJJ;
  m_tree_vbf->cenPhJJ = m_cenPhJJ;

  m_tree_vbf->pTJ1 = j1Vec.Pt();
  m_tree_vbf->pTJ2 = j2Vec.Pt();
  m_tree_vbf->pTB1 = b1Vec.Pt();
  m_tree_vbf->pTB2 = b2Vec.Pt();
  m_tree_vbf->pTPh = phVec.Pt();

  m_tree_vbf->etaJ1 = j1Vec.Eta();
  m_tree_vbf->etaJ2 = j2Vec.Eta();
  m_tree_vbf->etaB1 = b1Vec.Eta();
  m_tree_vbf->etaB2 = b2Vec.Eta();
  m_tree_vbf->etaPh = phVec.Eta();

  m_tree_vbf->MV2c10B1 = Props::MV2c10.get(signalJets.at(0));
  m_tree_vbf->MV2c10B2 = Props::MV2c10.get(signalJets.at(1));
  m_tree_vbf->WidthJ1 = Props::Width.get(vbfJets.at(0));
  m_tree_vbf->WidthJ2 = Props::Width.get(vbfJets.at(1));

  m_tree_vbf->dRB1Ph = b1Vec.DeltaR(phVec);
  m_tree_vbf->dRB2Ph = b2Vec.DeltaR(phVec);
  m_tree_vbf->dRJ1Ph = j1Vec.DeltaR(phVec);
  m_tree_vbf->dRJ2Ph = j2Vec.DeltaR(phVec);

  m_tree_vbf->dRB1J1 = b1Vec.DeltaR(j1Vec);
  m_tree_vbf->dRB1J2 = b1Vec.DeltaR(j2Vec);
  m_tree_vbf->dRB2J1 = b2Vec.DeltaR(j1Vec);
  m_tree_vbf->dRB2J2 = b2Vec.DeltaR(j2Vec);

  m_tree_vbf->nTrkPt5J1 = (float)m_nTrkPt5J1;
  m_tree_vbf->nTrkPt5J2 = (float)m_nTrkPt5J2;
  //  m_tree_vbf->nTrkPt10J1        = (float)m_nTrkPt10J1;
  //  m_tree_vbf->nTrkPt10J2        = (float)m_nTrkPt10J2;
  m_tree_vbf->SumPtExtraJets = m_SumPtExtraJets;
  m_tree_vbf->mindRBPh = m_mindRBPh;
  m_tree_vbf->mindRJPh = m_mindRJPh;
  //  m_tree_vbf->dR_BPhovJPh         = m_dR_BPhovJPh;
  //  m_tree_vbf->mindRJ1J         = m_mindRJ1J;
  //  m_tree_vbf->mindRJ2J         = m_mindRJ2J;
  //  m_tree_vbf->mindRB1J         = m_mindRB1J;
  //  m_tree_vbf->mindRB2J         = m_mindRB2J;
  m_tree_vbf->mindRJ1Ji = m_mindRJ1Ji;
  m_tree_vbf->mindRJ2Ji = m_mindRJ2Ji;*/

  //  if(m_doTrigStudy){
  //	 // triggers decision
  //	 m_tree_vbf->passVBF18 =
  // Props::passHLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700.get(m_eventInfo);
  //	 m_tree_vbf->passVBF20 =
  // Props::passHLT_g25_loose_L1EM20VH_2j40_0eta490_3j25_0eta490_invm700.get(m_eventInfo);
  //	 m_tree_vbf->passVBF22 =
  // Props::passHLT_g25_loose_2j40_0eta490_3j25_0eta490_invm700.get(m_eventInfo);
  //	 m_tree_vbf->passVBF0b =
  // Props::passHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700.get(m_eventInfo);
  //	 m_tree_vbf->passVBF1b =
  // Props::passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700.get(m_eventInfo);
  //	 m_tree_vbf->passVBF1bs =
  // Props::passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700.get(m_eventInfo);
  //	 m_tree_vbf->passVBF2b =
  // Props::passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490.get(m_eventInfo);
  //	 m_tree_vbf->passVBF2bs =
  // Props::passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490.get(m_eventInfo);
  //	 m_tree_vbf->passVBF25g4j =
  // Props::passHLT_g25_loose_L1EM20VH_4j35_0eta490.get(m_eventInfo);
  //	 m_tree_vbf->passEM13VH = Props::passL1_EM13VH.get(m_eventInfo);
  //	 m_tree_vbf->passEM15VH = Props::passL1_EM15VH.get(m_eventInfo);
  //	 m_tree_vbf->passEM18VH = Props::passL1_EM18VH.get(m_eventInfo);
  //	 m_tree_vbf->passEM20VH = Props::passL1_EM20VH.get(m_eventInfo);
  //	 m_tree_vbf->passEM22VHI = Props::passL1_EM22VHI.get(m_eventInfo);
  //	 m_tree_vbf->passg10 = Props::passHLT_g10_loose.get(m_eventInfo);
  //	 m_tree_vbf->passg15 = Props::passHLT_g15_loose_L1EM7.get(m_eventInfo);
  //	 m_tree_vbf->passg20 = Props::passHLT_g20_loose_L1EM12.get(m_eventInfo);
  ////	 m_tree_vbf->passg20m = Props::passHLT_g20_medium.get(m_eventInfo);
  //	 m_tree_vbf->passg25 = Props::passHLT_g25_loose_L1EM15.get(m_eventInfo);
  //	 m_tree_vbf->passg35 = Props::passHLT_g35_loose_L1EM15.get(m_eventInfo);
  //	 m_tree_vbf->passg40 = Props::passHLT_g40_loose_L1EM15.get(m_eventInfo);
  //	 m_tree_vbf->passg45 = Props::passHLT_g45_loose_L1EM15.get(m_eventInfo);
  //	 m_tree_vbf->passg50 = Props::passHLT_g50_loose_L1EM15.get(m_eventInfo);
  //	 m_tree_vbf->passg60 = Props::passHLT_g60_loose.get(m_eventInfo);
  //	 m_tree_vbf->passg70 = Props::passHLT_g70_loose.get(m_eventInfo);
  //	 m_tree_vbf->passg80 = Props::passHLT_g80_loose.get(m_eventInfo);
  //	 m_tree_vbf->passg100 = Props::passHLT_g100_loose.get(m_eventInfo);
  //	 m_tree_vbf->passg120 = Props::passHLT_g120_loose.get(m_eventInfo);
  //  }
  //  // trigger object variables
  //  // for emulation
  //  m_tree_vbf->l1_emet = m_l1_emet;
  //  m_tree_vbf->hlt_phpt = m_hlt_phpt;
  //  m_tree_vbf->hlt_j1pt = m_hlt_j1pt;
  //  m_tree_vbf->hlt_j2pt = m_hlt_j2pt;
  //  m_tree_vbf->hlt_j3pt = m_hlt_j3pt;
  //  m_tree_vbf->hlt_j4pt = m_hlt_j4pt;
  //  m_tree_vbf->hlt_j5pt = m_hlt_j5pt;
  //  m_tree_vbf->hlt_mjj = m_hlt_mjj;
  //  // for efficiency study
  //  m_tree_vbf->hlt_phpt_matched = m_hlt_phpt_matched;
  //  m_tree_vbf->hlt_j4pt_phor = m_hlt_j4pt_phor;
  //  m_tree_vbf->hlt_mjj_phor = m_hlt_mjj_phor;
  //
  //  m_tree_vbf->weight_forwardJetScale_up = m_forwardJetScale_up;
  //  m_tree_vbf->weight_forwardJetScale_down = m_forwardJetScale_down;
  //  m_tree_vbf->weight_pTBalScale_up = m_pTBalScale_up;
  //  m_tree_vbf->weight_pTBalScale_down = m_pTBalScale_down;
  //  m_tree_vbf->weight_pTBalPS_up = m_pTBalPS_up;
  //  m_tree_vbf->weight_pTBalPS_down = m_pTBalPS_down;
  //
  //
  //  m_tree_vbf->weight_TrigJetEff_up = m_TrigJetEff_up;
  //  m_tree_vbf->weight_TrigJetEff_down = m_TrigJetEff_down;
  //  m_tree_vbf->weight_TrigL1EMEffon = m_TrigL1EMEffon;

  // after filling the MVA tree, start to evaluate MVA
  m_tree_vbf->ReadMVA();

  m_tree_vbf->Fill();

  // if (m_isMC){
  // m_tree_vbf->MCWeight    = Props::MCEventWeight.get(m_eventInfo);
  //}
  // m_tree_vbf->passTrig = m_triggerTool->getTriggerDecision(m_eventInfo,
  // triggerSF, 0, 0, 0, 0, 0, photon, m_eventInfo->get_RandomRunNumber(), "");
  // m_tree_vbf->mBB_Regression =
  // (signalJets.at(0)->jetP4("Regression")+signalJets.at(1)->jetP4("Regression")).M();
  // m_tree_vbf->mBB_OneMu =
  // (signalJets.at(0)->jetP4("OneMu")+signalJets.at(1)->jetP4("OneMu")).M();
  // m_tree_vbf->mBB_PtRecollbbOneMuPartonBukinNew =
  // (signalJets.at(0)->jetP4("PtRecollbbOneMuPartonBukinNew")+signalJets.at(1)->jetP4("PtRecollbbOneMuPartonBukinNew")).M();
  // m_tree_vbf->MV2c20B1 = Props::MV2c20.get(signalJets.at(0));
  // m_tree_vbf->MV2c20B2 = Props::MV2c20.get(signalJets.at(1));

  //*********************************************************//
  //                       EASY TREE                         //
  //*********************************************************//

  if (m_debug) std::cout << " >>>>> EASY TREE" << std::endl;

  m_etree->Reset();
  m_etree->SetVariation(m_currentVar);

  // event info
  // m_etree->SetBranchAndValue("sample", m_histNameSvc->getFullSample(), "");
  m_etree->SetBranchAndValue("RunNumber", m_eventInfo->runNumber(),
                             (unsigned int)(-1));
  m_etree->SetBranchAndValue("EventNumber", m_eventInfo->eventNumber(),
                             (long long unsigned int)(-1));
  m_etree->SetBranchAndValue("MCChannelNumber", m_mcChannel, -1);

  // for fit study
  m_etree->SetBranchAndValue("tagCatExcl", tagcatExcl, -1);
  //  m_etree->SetBranchAndValue("tagCatExclDirect", tagCatExclDirect, -1);
  m_etree->SetBranchAndValue("mBB", HVecCorr.M(), double(-1));
  //  m_etree->SetBranchAndValue("mBBNoCorr", (b1Vec + b2Vec).M(), double(-1));
  //m_etree->SetBranchAndValue("BDT", m_tree_vbf->BDT, float(-10.));
  m_etree->SetBranchAndValue("nJets", nJets40, -1);

  // MVA preselection
  m_etree->SetBranchAndValue("passMVAPreSel", m_passMVAPreSel, 0);
  //  m_etree->SetBranchAndValue("passTrig", passTrigger, 0);
  //  m_etree->SetBranchAndValue("matchTrig", matchTrigger, 0);
  //  m_etree->SetBranchAndValue("mJJ", m_mJJ, double(-1)); //(j1Vec +
  //  j2Vec).M(); m_etree->SetBranchAndValue("pTBB", HVec.Pt(), double(-1));

  // event weight
  m_etree->SetBranchAndValue("EventWeight", m_weight, double(-1));
  //  m_etree->SetBranchAndValue("dEtaJJSF", dEtaJJSF, float(1));
  //  m_etree->SetBranchAndValue("mindRBPhSF", mindRBPhSF, float(1));
  //  m_etree->SetBranchAndValue<float>("BTagSF",      btagWeight,  -1.);
  //  m_etree->SetBranchAndValue<float>("PhotonSF",    photonSF,    -1.);
  //  m_etree->SetBranchAndValue<float>("jvtSF",    jvtSF,    -1.);
  //  m_etree->SetBranchAndValue<float>("QGTaggerSFJ1",    QGTaggerSFJ1, -1.);
  //  m_etree->SetBranchAndValue<float>("QGTaggerSFJ2",    QGTaggerSFJ2, -1.);

  if (m_checkOrthogonality) {
    m_etree->SetBranchAndValue("passIncl4cenSel", m_passIncl4cenSel, 0);
    m_etree->SetBranchAndValue("passIncl2cenSel", m_passIncl2cenSel, 0);
  }

  if (m_doTrigStudy) {
    // general offline vars
    m_etree->SetBranchAndValue("randomRunNumber", m_randomRunNumber, -1);
    m_etree->SetBranchAndValue<float>("mJJ", m_mJJ,
                                      double(-1));  //(j1Vec + j2Vec).M();
    m_etree->SetBranchAndValue<float>("pTBB", HVec.Pt(), float(-1));
    m_etree->SetBranchAndValue<float>("pTJ1", j1Vec.Pt(), float(-1));
    m_etree->SetBranchAndValue<float>("pTJ2", j2Vec.Pt(), float(-1));
    m_etree->SetBranchAndValue<float>("pTPh", phVec.Pt(), float(-1));
    m_etree->SetBranchAndValue<float>("pTB1", b1Vec.Pt(), float(-1));
    m_etree->SetBranchAndValue<float>("pTB2", b2Vec.Pt(), float(-1));
    m_etree->SetBranchAndValue("MV2c10B1", Props::MV2c10.get(signalJets.at(0)),
                               float(-10));
    m_etree->SetBranchAndValue("MV2c10B2", Props::MV2c10.get(signalJets.at(1)),
                               float(-10));
    // trigger specific offline vars
    m_etree->SetBranchAndValue("bjetTrigPt_SF", bjetTrigPt_SF, double(-1));
    m_etree->SetBranchAndValue("bjetTrigEta_SF", bjetTrigEta_SF, double(-1));
    m_etree->SetBranchAndValue<int>("matchB1", match_b1, -1);
    m_etree->SetBranchAndValue<int>("matchB2", match_b2, -1);
    m_etree->SetBranchAndValue("pTJ4", pTJ4, float(-1));
    m_etree->SetBranchAndValue("pTJ1cen", pTJ1cen, float(-1));
    m_etree->SetBranchAndValue("etaJ1cen", etaJ1cen, float(-1));
    // triggers decision
    m_etree->SetBranchAndValue(
        "passVBF18",
        Props::passHLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700.get(
            m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF20",
        Props::passHLT_g25_loose_L1EM20VH_2j40_0eta490_3j25_0eta490_invm700.get(
            m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF22",
        Props::passHLT_g25_loose_2j40_0eta490_3j25_0eta490_invm700.get(
            m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF0b",
        Props::passHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700.get(
            m_eventInfo),
        -1);
    m_etree->SetBranchAndValue<int>(
        "matchVBF0b",
        Props::matchHLT_g25_medium_L1EM22VHI_4j35_0eta490_invm700.get(photon),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF1b",
        Props::
            passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_3j35_0eta490_invm700
                .get(m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF1bs",
        Props::
            passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c2077_split_3j35_0eta490_invm700
                .get(m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF2b",
        Props::passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_2j35_0eta490
            .get(m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF2bs",
        Props::
            passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c2077_split_2j35_0eta490
                .get(m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF1bmv2c10s",
        Props::
            passHLT_g25_medium_L1EM22VHI_j35_0eta490_bmv2c1077_split_3j35_0eta490_invm700
                .get(m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBF2bmv2c10s",
        Props::
            passHLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490
                .get(m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBFbmv2c10s500",
        Props::
            passHLT_g20_tight_icaloloose_j35_bmv2c1077_split_3j35_0eta490_invm500
                .get(m_eventInfo),
        -1);
    m_etree->SetBranchAndValue(
        "passVBFbmv2c10s500gsc",
        Props::
            passHLT_g20_tight_icaloloose_j15_gsc35_bmv2c1077_split_3j35_0eta490_invm500
                .get(m_eventInfo),
        -1);

    m_etree->SetBranchAndValue(
        "passVBF25g4j",
        Props::passHLT_g25_loose_L1EM20VH_4j35_0eta490.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passEM13VH",
                               Props::passL1_EM13VH.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passEM15VH",
                               Props::passL1_EM15VH.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passEM18VH",
                               Props::passL1_EM18VH.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passEM20VH",
                               Props::passL1_EM20VH.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passEM22VHI",
                               Props::passL1_EM22VHI.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passg10",
                               Props::passHLT_g10_loose.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue(
        "passg15", Props::passHLT_g15_loose_L1EM7.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue(
        "passg20", Props::passHLT_g20_loose_L1EM12.get(m_eventInfo), -1);
    //	 m_etree->SetBranchAndValue("passg20m",
    // Props::passHLT_g20_medium.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue(
        "passg25", Props::passHLT_g25_loose_L1EM15.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue(
        "passg35", Props::passHLT_g35_loose_L1EM15.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue(
        "passg40", Props::passHLT_g40_loose_L1EM15.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue(
        "passg45", Props::passHLT_g45_loose_L1EM15.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue(
        "passg50", Props::passHLT_g50_loose_L1EM15.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passg60",
                               Props::passHLT_g60_loose.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passg70",
                               Props::passHLT_g70_loose.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passg80",
                               Props::passHLT_g80_loose.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passg100",
                               Props::passHLT_g100_loose.get(m_eventInfo), -1);
    m_etree->SetBranchAndValue("passg120",
                               Props::passHLT_g120_loose.get(m_eventInfo), -1);
    // trigger object variables
    // for emulation
    m_etree->SetBranchAndValue("l1_emet", m_l1_emet, float(-1));
    m_etree->SetBranchAndValue("hlt_phpt", m_hlt_phpt, float(-1));
    m_etree->SetBranchAndValue("hlt_j1pt", m_hlt_j1pt, float(-1));
    m_etree->SetBranchAndValue("hlt_j2pt", m_hlt_j2pt, float(-1));
    m_etree->SetBranchAndValue("hlt_j3pt", m_hlt_j3pt, float(-1));
    m_etree->SetBranchAndValue("hlt_j4pt", m_hlt_j4pt, float(-1));
    m_etree->SetBranchAndValue("hlt_j5pt", m_hlt_j5pt, float(-1));
    m_etree->SetBranchAndValue("hlt_mjj", m_hlt_mjj, float(-1));
    // for efficiency study
    m_etree->SetBranchAndValue("hlt_phpt_matched", m_hlt_phpt_matched,
                               float(-1));
    m_etree->SetBranchAndValue("hlt_j4pt_phor", m_hlt_j4pt_phor, float(-1));
    m_etree->SetBranchAndValue("hlt_mjj_phor", m_hlt_mjj_phor, float(-1));
  }

  // weight systematics
  for (unsigned int i = 0; i < m_weightSysts.size(); i++) {
    m_etree->SetBranchAndValue(m_weightSysts.at(i).name,
                               m_weightSysts.at(i).factor, float(1));
  }
  if (m_weightSysts.size()) {
    // bjet trigger systematics
    m_etree->SetBranchAndValue("weight_bjetTrigPt_SFup", weight_bjetTrigPt_SFup,
                               double(-1));
    m_etree->SetBranchAndValue("weight_bjetTrigPt_SFdown",
                               weight_bjetTrigPt_SFdown, double(-1));
    m_etree->SetBranchAndValue("weight_bjetTrigEta_SFup",
                               weight_bjetTrigEta_SFup, double(-1));
    m_etree->SetBranchAndValue("weight_bjetTrigEta_SFdown",
                               weight_bjetTrigEta_SFdown, double(-1));
    m_etree->SetBranchAndValue<float>("pTB1", b1Vec.Pt(), float(-1));
    m_etree->SetBranchAndValue<float>("pTB2", b2Vec.Pt(), float(-1));
    m_etree->SetBranchAndValue("etaJ1cen", etaJ1cen, float(-1));

    // L1 EM syst
    m_etree->SetBranchAndValue("weight_TrigL1EMEffon", m_TrigL1EMEffon,
                               float(1));
    // m_etree->SetBranchAndValue<float>("pTPh", phVec.Pt(), float(-1));

    // jet trigger syst
    m_etree->SetBranchAndValue("weight_TrigJetEff_up", m_TrigJetEff_up,
                               float(1));
    m_etree->SetBranchAndValue("weight_TrigJetEff_down", m_TrigJetEff_down,
                               float(1));
    // m_etree->SetBranchAndValue("pTJ4", pTJ4, float(-1));

    // theory syst
    m_etree->SetBranchAndValue("weight_forwardJetScale_up",
                               m_forwardJetScale_up, float(1));
    m_etree->SetBranchAndValue("weight_forwardJetScale_down",
                               m_forwardJetScale_down, float(1));
    m_etree->SetBranchAndValue("weight_pTBalScale_up", m_pTBalScale_up,
                               float(1));
    m_etree->SetBranchAndValue("weight_pTBalScale_down", m_pTBalScale_down,
                               float(1));
    m_etree->SetBranchAndValue("weight_pTBalPS_up", m_pTBalPS_up, float(1));
    m_etree->SetBranchAndValue("weight_pTBalPS_down", m_pTBalPS_down, float(1));
    // m_etree->SetBranchAndValue("pTBal", m_pTBal, float(-1));
    // m_etree->SetBranchAndValue<float>("mJJ", m_mJJ, double(-1));
  }

  m_etree->Fill();

  //*********************************************************//
  //                  CUT BASED / MVA SELECTION              //
  //*********************************************************//

  bool passCuts = true;
  // cut based analysis -- Based on Song-Mings study
  if (m_doCutBased) {
    if (!passTrigger) passCuts = false;
    if ((j1Vec + j2Vec).M() < 800. * GeV) passCuts = false;
    if (HVec.Pt() < 100. * GeV) passCuts = false;
    if (m_HT_soft > 50. * GeV) passCuts = false;
    if (m_cenPhJJ > 0.6) passCuts = false;
    if (m_pTBal > 0.2) passCuts = false;
    if (b1Vec.DeltaR(phVec) < 1.0) passCuts = false;
    if (b2Vec.DeltaR(phVec) < 1.0) passCuts = false;
  }

  // MVA pre-selection -- only apply to histograms, not to MVATree
  if (m_doMVAPreSel) {
    // if (!passTrigger)                           passCuts = false;
    // if (!matchTrigger)                          passCuts = false;
    // if (phVec.Pt() < 30. * GeV)                 passCuts = false;
    // if ((j1Vec+j2Vec).M() < 800. * GeV)         passCuts = false;
    // if (HVec.Pt() < 80. * GeV)                  passCuts = false;
    passCuts = bool(m_passMVAPreSel);
  }

  // if (nJets > 4) passCuts = false;

  if (!passCuts) {
    // std::printf("Failed kinematic cuts! Skipping this event...\n");
    return EL::StatusCode::SUCCESS;
  }
  // std::printf("passed cuts!\n");

  //*********************************************************//
  //                    HISTOGRAMS                           //
  //*********************************************************//

  // set histogram naming conventions
  // -------------------------------
  m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::VBF);
  m_histNameSvc->set_nTag(tagcatExcl);  // set histogram name
  m_histNameSvc->set_nJet(nJets40);     // more histogram naming
  // if(HVecCorr.M() < 110e3 || HVecCorr.M() > 140e3)
  // m_histNameSvc->set_description("mBBcr");

  // std::cout << "Current variation: " << m_currentVar << std::endl;
  int nIter = 1;
  float m_sysFact = 1;
  if ((m_currentVar == "Nominal") and m_isMC) nIter = 16;

  for (int i = 0; i < nIter; ++i) {
    if (i == 0) {
      m_sysFact = 1;
      m_histNameSvc->set_variation(m_currentVar);
      m_tree_vbf->WidthJ1 = Props::Width.get(vbfJets.at(0));
      m_tree_vbf->WidthJ2 = Props::Width.get(vbfJets.at(1));
      m_tree_vbf->ReadMVA();
      // std::printf("Filling nominal hists with weight %f and BDT score %f\n",
      // m_weight, m_tree_vbf->BDT);
    }
    if (i == 1) {
      m_sysFact = 1;
      m_histNameSvc->set_variation("JetWidth__1up");
      m_tree_vbf->WidthJ1 = Props::Width.get(vbfJets.at(0)) * 1.1;
      m_tree_vbf->WidthJ2 = Props::Width.get(vbfJets.at(1)) * 1.1;
      m_tree_vbf->ReadMVA();
      // std::printf("Filling jet width up hists with weight %f and BDT score
      // %f\n", m_weight, m_tree_vbf->BDT);
    }
    if (i == 2) {
      m_sysFact = 1;
      m_histNameSvc->set_variation("JetWidth__1down");
      m_tree_vbf->WidthJ1 = Props::Width.get(vbfJets.at(0)) * 0.9;
      m_tree_vbf->WidthJ2 = Props::Width.get(vbfJets.at(1)) * 0.9;
      m_tree_vbf->ReadMVA();
      // std::printf("Filling jet width down hists with weight %f and BDT score
      // %f\n\n", m_weight, m_tree_vbf->BDT);
    }

    // need to reset these for the other variations
    if (i > 2) {
      m_tree_vbf->WidthJ1 = Props::Width.get(vbfJets.at(0));
      m_tree_vbf->WidthJ2 = Props::Width.get(vbfJets.at(1));
    }

    if (i == 3) {
      m_histNameSvc->set_variation("ForwardJetScale__1up");
      m_sysFact = m_forwardJetScale_up;
      // None for ZgammaQCD
      m_tree_vbf->ReadMVA();
    }
    if (i == 4) {
      m_histNameSvc->set_variation("ForwardJetScale__1down");
      m_sysFact = m_forwardJetScale_down;
      // None for ZgammaQCD
      m_tree_vbf->ReadMVA();
    }
    if (i == 5) {
      m_histNameSvc->set_variation("PtBalScale__1up");
      m_sysFact = m_pTBalScale_up;
      m_tree_vbf->ReadMVA();
    }
    if (i == 6) {
      m_histNameSvc->set_variation("PtBalScale__1down");
      m_sysFact = m_pTBalScale_down;
      m_tree_vbf->ReadMVA();
    }
    if (i == 7) {
      m_sysFact = 1;
      m_histNameSvc->set_variation("HTSoftTSTScale__1up");
      m_tree_vbf->HT_soft = m_HT_soft_SCALEUP;
      m_tree_vbf->ReadMVA();
    }
    if (i == 8) {
      m_sysFact = 1;
      m_histNameSvc->set_variation("HTSoftTSTScale__1down");
      m_tree_vbf->HT_soft = m_HT_soft_SCALEDOWN;
      m_tree_vbf->ReadMVA();
    }
    if (i == 9) {
      m_sysFact = 1;
      m_histNameSvc->set_variation("HTSoftTSTResoPara__1up");
      m_tree_vbf->HT_soft = m_HT_soft_RESOPARA;
      m_tree_vbf->ReadMVA();
    }
    if (i == 10) {
      m_sysFact = 1;
      m_histNameSvc->set_variation("HTSoftTSTResoPerp__1up");
      m_tree_vbf->HT_soft = m_HT_soft_RESOPERP;
      m_tree_vbf->ReadMVA();
    }
    if (i > 10) {  // reset HT_soft to nominal
      m_tree_vbf->HT_soft = m_HT_soft;
      m_tree_vbf->ReadMVA();
    }
    if (i == 11) {
      m_histNameSvc->set_variation("Trig_Jet_EFF__1up");
      m_sysFact = m_TrigJetEff_up;
    }
    if (i == 12) {
      m_histNameSvc->set_variation("Trig_Jet_EFF__1down");
      m_sysFact = m_TrigJetEff_down;
    }
    if (i == 13) {
      m_histNameSvc->set_variation("Trig_L1EM_EFF");
      m_sysFact = m_TrigL1EMEffon;
    }
    if (i == 14) {
      m_histNameSvc->set_variation("PtBalPS__1up");
      m_sysFact = m_pTBalPS_up;
      m_tree_vbf->ReadMVA();
    }
    if (i == 15) {
      m_histNameSvc->set_variation("PtBalPS__1down");
      m_sysFact = m_pTBalPS_down;
      m_tree_vbf->ReadMVA();
    }

    // if(m_tree_vbf->BDT<0.1) m_histNameSvc->set_pTV(0); // BDT binning
    // else m_histNameSvc->set_pTV(1);
    /*if (m_tree_vbf->BDT < -0.1)
      m_histNameSvc->set_pTV(0);  // BDT binning
    else if (m_tree_vbf->BDT >= -0.1 && m_tree_vbf->BDT < 0.1)
      m_histNameSvc->set_pTV(1);  //  method follows VH convention :(
    else
      m_histNameSvc->set_pTV(2);

    // after passCuts start filling histograms
    m_histSvc->BookFillHist("BDT", 100, -1, 1, m_tree_vbf->BDT,
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("AverageMu", 360, 0.0, 90.0, m_averageMu,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("ActualMu", 360, 0.0, 90.0, m_actualMu,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("AverageMuScaled", 360, 0.0, 90.0,
                            m_averageMuScaled, m_weight * m_sysFact);
    m_histSvc->BookFillHist("ActualMuScaled", 360, 0.0, 90.0, m_actualMuScaled,
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("NVtx", 90, 0, 90, Nvtx, m_weight * m_sysFact);
    m_histSvc->BookFillHist("ZPV", 400, -200, 200, Props::ZPV.get(m_eventInfo),
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("nJets", 6, 4, 10, nJets, m_weight * m_sysFact);
    m_histSvc->BookFillHist("mJJ", 50, 0., 5000., m_tree_vbf->mJJ * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTJJ", 50, 0., 800., (j1Vec + j2Vec).Pt() * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dEtaJJ", 50, 0., 9., m_tree_vbf->dEtaJJ,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRJJ", 50, 0., 10., j1Vec.DeltaR(j2Vec),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dPhiJJ", 50, 0., pi, fabs(j1Vec.DeltaPhi(j2Vec)),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("maxEta", 50, 0., 4.5,
                            TMath::Max(fabs(j1Vec.Eta()), fabs(j2Vec.Eta())),
                            m_weight * m_sysFact);

    // m_histSvc -> BookFillHist("mBB"       ,  35, 50.,  400.,
    // HVecCorr.M()*invGeV,                        m_weight * m_sysFact);
    // if(m_tree_vbf->BDT<0.1)
    //  m_histSvc -> BookFillHist("mBB"       ,  40, 50.,  450.,
    //  HVecCorr.M()*invGeV,                        m_weight * m_sysFact);
    // else
    //  m_histSvc -> BookFillHist("mBB"       ,  20, 50.,  250.,
    //  HVecCorr.M()*invGeV,                        m_weight * m_sysFact);

    if (m_tree_vbf->BDT < -0.1)
      m_histSvc->BookFillHist("mBB", 40, 50., 450., HVecCorr.M() * invGeV,
                              m_weight * m_sysFact);
    else if (m_tree_vbf->BDT >= -0.1 && m_tree_vbf->BDT < 0.1)
      m_histSvc->BookFillHist("mBB", 30, 50., 350., HVecCorr.M() * invGeV,
                              m_weight * m_sysFact);
    else
      m_histSvc->BookFillHist("mBB", 20, 50., 250., HVecCorr.M() * invGeV,
                              m_weight * m_sysFact);

    m_histSvc->BookFillHist("mBBPh", 50, 0., 1500.,
                            (HVecCorr + phVec).M() * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRBB", 50, 0., 6., m_dRBB, m_weight * m_sysFact);
    m_histSvc->BookFillHist("dPhiBB", 50, 0., pi, fabs(b1Vec.DeltaPhi(b2Vec)),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dEtaBB", 50, 0., 5.,
                            fabs(b1Vec.Eta() - b2Vec.Eta()),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTBB", 50, 0., 600., HVec.Pt() * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dPhiPhBmin", 50, 0., pi, m_dPhiPhBmin,
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("nJetGap", 8, 0, 8, m_nJetGap,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("HT_gap", 50, 0., 600., m_HT_gap * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("HT_soft", 50, 0., 400.,
                            m_tree_vbf->HT_soft * invGeV, m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTBal", 50, 0., 1.0, m_tree_vbf->pTBal,
                            m_weight * m_sysFact);*/

    m_histSvc->BookFillHist("nTrkJet2", 35, 0, 35, nTrkJet2,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("nTrkJet5", 14, 0, 14, nTrkJet5,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("nTrkJet10", 9, 0, 9, nTrkJet10,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dEtaJJBB", 50, 0., 6., dEtaJJBB,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dPhiBBJJ", 50, 0., 6., dPhiBBJJ,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTTot", 50, 0., 400., pTTot * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pZTot", 50, 0., 5000., pZTot * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("HT", 50, 180., 2000., HT * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTJ5", 50, 20., 300., pTJ5 * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("etaJStar", 50, -2.5, 4.5, etaJStar,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("cosThetaC", 50, -1., 1., fabs(cosTheta_CMS),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("cosThetaA", 50, -1., 1., fabs(cosTheta_ATLAS),
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("cenHJJ", 50, 0., 10., m_cenHJJ,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("cenHgJJ", 50, 0., 10., m_cenHgJJ,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("cenPhJJ", 50, 0., 10., m_cenPhJJ,
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("pTJ1", 50, 40., 700., j1Vec.Pt() * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTJ2", 50, 40., 300., j2Vec.Pt() * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTB1", 50, 40., 600., b1Vec.Pt() * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTB2", 50, 40., 300., b2Vec.Pt() * invGeV,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("pTPH", 50, 30., 400., phVec.Pt() * invGeV,
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("etaJ1", 50, -4.7, 4.7, j1Vec.Eta(),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("etaJ2", 50, -4.7, 4.7, j2Vec.Eta(),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("etaB1", 50, -2.7, 2.7, b1Vec.Eta(),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("etaB2", 50, -2.7, 2.7, b2Vec.Eta(),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("etaPH", 50, -2.7, 2.7, phVec.Eta(),
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("MV2C10B1", 50, -1., 1.,
                            Props::MV2c10.get(signalJets.at(0)),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("MV2C10B2", 50, -1., 1.,
                            Props::MV2c10.get(signalJets.at(1)),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("WidthJ1", 50, 0., 0.3, m_tree_vbf->WidthJ1,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("WidthJ2", 50, 0., 0.3, m_tree_vbf->WidthJ2,
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("dRB1Ph", 50, 0., 6., b1Vec.DeltaR(phVec),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRB2Ph", 50, 0., 6., b2Vec.DeltaR(phVec),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRJ1Ph", 50, 0., 8., j1Vec.DeltaR(phVec),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRJ2Ph", 50, 0., 8., j2Vec.DeltaR(phVec),
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("dRB1J1", 50, 0., 8., b1Vec.DeltaR(j1Vec),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRB1J2", 50, 0., 8., b1Vec.DeltaR(j2Vec),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRB2J1", 50, 0., 8., b2Vec.DeltaR(j1Vec),
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dRB2J2", 50, 0., 8., b2Vec.DeltaR(j2Vec),
                            m_weight * m_sysFact);

    m_histSvc->BookFillHist("nTrkPt5J1", 60, -10., 50.0, m_nTrkPt5J1,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("nTrkPt5J2", 60, -10., 50.0, m_nTrkPt5J1,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("nTrkPt10J1", 60, -10., 50.0, m_nTrkPt10J1,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("nTrkPt10J2", 60, -10., 50.0, m_nTrkPt10J2,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("SumPtExtraJets", 50, 0., 600.,
                            m_SumPtExtraJets * invGeV, m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRBPh", 90, 0., 9.0, m_mindRBPh,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRBPh", 90, 0., 9.0, m_mindRBPh,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("dR_BPhovJPh", 100, 0., 10.0, m_dR_BPhovJPh,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRJ1J", 90, 0., 9.0, m_mindRJ1J,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRJ2J", 90, 0., 9.0, m_mindRJ2J,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRB1J", 90, 0., 9.0, m_mindRB1J,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRB2J", 90, 0., 9.0, m_mindRB2J,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRJ1Ji", 90, 0., 9.0, m_mindRJ1Ji,
                            m_weight * m_sysFact);
    m_histSvc->BookFillHist("mindRJ2Ji", 90, 0., 9.0, m_mindRJ2Ji,
                            m_weight * m_sysFact);
  }
  return EL::StatusCode::SUCCESS;
}

bool AnalysisReader_VBFHbb1Ph::jet_selection_vbf(
    std::vector<const xAOD::Jet*> inputJets,
    std::vector<const xAOD::Jet*>& signalJets,
    std::vector<const xAOD::Jet*>& vbfJets, int& tagcatExcl, int& tagcatIncl) {
  string tagStrategy, tagAlgorithm;
  m_config->getif<std::string>("tagStrategy",
                               tagStrategy);  // SignalJets, AllJets
  m_config->getif<std::string>("tagAlgorithm",
                               tagAlgorithm);  // FlavLabel,FlavTag

  signalJets.clear();
  vbfJets.clear();
  tagcatExcl = 0;
  tagcatIncl = -1;

  // first sort jets by b-tagging probability
  struct bsort {
    bool operator()(const xAOD::Jet* jet1, const xAOD::Jet* jet2) {
      return (Props::MV2c10.get(jet1) > Props::MV2c10.get(jet2));
    }
  } m_bsort;  // bsort

  std::sort(inputJets.begin(), inputJets.end(), m_bsort);

  // for(int i = 0; i<inputJets.size(); ++i)
  //  std::printf("jetMV2c10: %f\n", Props::MV2c10.get(inputJets.at(i)));

  // pick the 2 most likely b-tagged jets as signal jets.
  // all others as vbf jets
  for (const auto* const jetPtr : inputJets) {
    // if(! Props::isVBFJet.get(jetPtr)) continue;
    if (!Props::isVetoJet.get(jetPtr)) continue;
    if (jetPtr->pt() < 40000) continue;
    if (signalJets.size() < 2 && fabs(jetPtr->eta()) < 2.5)
      signalJets.push_back(jetPtr);
    else
      vbfJets.push_back(jetPtr);
  }

  // check that we have enough of each type of jet
  if (signalJets.size() < 2 || vbfJets.size() < 2) return false;

  // find the pair of jets that yields the highest mjj
  const xAOD::Jet* vbfjet1(nullptr);
  const xAOD::Jet* vbfjet2(nullptr);
  double maxMjj(-1.);
  for (const auto* const jetPtr1 : vbfJets) {
    for (const auto* const jetPtr2 : vbfJets) {
      if (jetPtr1 == jetPtr2) continue;  // skip if they are the same jet
      // if(! (jetPtr1->eta()*jetPtr2->eta() < 0 )) continue; // require jets to
      // be in opposite hemispheres
      TLorentzVector jet1(jetPtr1->p4()), jet2(jetPtr2->p4());
      if ((jet1 + jet2).M() > maxMjj) {
        maxMjj = (jet1 + jet2).M();
        vbfjet1 = jetPtr1;
        vbfjet2 = jetPtr2;
      }
    }
  }

  // if we didn't find any vbf jets
  if (maxMjj < 0) return false;

  m_bTagTool->setJetAuthor(m_jetReader->getContainerName());
  if (m_bTagTool->isTagged(*(signalJets.at(1)))) {
    tagcatExcl++;
    BTagProps::isTagged.set(signalJets.at(1), 1);
  }
  if (m_bTagTool->isTagged(*(signalJets.at(0)))) {
    tagcatExcl++;
    BTagProps::isTagged.set(signalJets.at(0), 1);
  }

  // tagcatIncl is not used for now
  // the naming scheme seems backwards. hmmm...

  // tagcatIncl==0 --> 0ptag, !1ptag, !2ptag
  // tagcatIncl==1 --> 0ptag, 1ptag, !2ptag
  // tagcatIncl==2 --> 0ptag, 1ptag, 2ptag

  // set the output jets
  // sort the signal jets by pt
  // sort the vbf jets by pt
  // do nothing with any of the other jets
  vbfJets.clear();
  if (vbfjet1->pt() > vbfjet2->pt()) {
    vbfJets.push_back(vbfjet1);
    vbfJets.push_back(vbfjet2);
  } else {
    vbfJets.push_back(vbfjet2);
    vbfJets.push_back(vbfjet1);
  }
  if (signalJets.at(0)->pt() < signalJets.at(1)->pt()) {
    std::swap(signalJets.at(0), signalJets.at(1));
  }

  return true;
}

bool AnalysisReader_VBFHbb1Ph::jet_selection_vbf4(
    std::vector<const xAOD::Jet*> inputJets,
    std::vector<const xAOD::Jet*>& signalJets,
    std::vector<const xAOD::Jet*>& vbfJets,
    std::vector<const xAOD::Jet*>& vbfJets2, int& tagcatExcl, int& tagcatIncl) {
  string tagStrategy, tagAlgorithm;
  m_config->getif<std::string>("tagStrategy",
                               tagStrategy);  // SignalJets, AllJets
  m_config->getif<std::string>("tagAlgorithm",
                               tagAlgorithm);  // FlavLabel,FlavTag

  std::vector<const xAOD::Jet*> signalJets2;
  std::vector<const xAOD::Jet*> nonBtagJets;

  signalJets.clear();
  vbfJets.clear();
  vbfJets2.clear();  // list of jets not signal bb, not vbf JJ. and not photon
  tagcatExcl = 0;
  tagcatIncl = -1;

  signalJets2.clear();
  nonBtagJets.clear();

  int my_njetPt40 = 0;
  int my_nbtag = 0;
  int my_nbtag40 = 0;
  int my_nbtag_signalJets = 0;

  // count number of jet pT>40 GeV
  for (const auto* const jetPtr : inputJets) {
    if ((jetPtr->pt() * 0.001) > 40.0) {
      ++my_njetPt40;
    }
  }

  // Check number of b-tag
  m_bTagTool->setJetAuthor(m_jetReader->getContainerName());
  for (const xAOD::Jet* jet : inputJets) {
    // bool isTagged = BTagProps::isTagged.get(jet);
    bool isTagged = (m_bTagTool->isTagged(*jet));

    // if using truth label (MC only!)
    if (m_isMC && (tagAlgorithm == "FlavLabel"))
      isTagged = (Props::HadronConeExclTruthLabelID.get(jet) == 5);

    if (isTagged) {
      my_nbtag++;
      float pt = (jet->pt()) * 0.001;
      float eta = jet->eta();

      if (pt > 40.0) {
        if (fabs(eta) < 2.5) {
          my_nbtag40++;
        }
      }
    }
  }

  // sort jets by b-tagging probability
  struct bsort {
    bool operator()(const xAOD::Jet* jet1, const xAOD::Jet* jet2) {
      return (Props::MV2c10.get(jet1) > Props::MV2c10.get(jet2));
    }
  } m_bsort;  // bsort

  // sort jets by pt
  struct ptsort {
    bool operator()(const xAOD::Jet* jet1, const xAOD::Jet* jet2) {
      return (jet1->p4().Pt() > jet2->p4().Pt());
    }
  } m_ptsort;  // ptsort

  if (m_doTruthTagging) {
    //===========================
    // sort jet by btag weight
    //===========================
    std::sort(inputJets.begin(), inputJets.end(), m_bsort);
  } else {
    //===========================
    // sort jet by Pt
    //===========================
    std::sort(inputJets.begin(), inputJets.end(), m_ptsort);
  }

  // pick the 2 bb signal jets.
  // all others as vbf jets
  int icj = 0;
  int n_SgnJet_Btag = 0;
  m_bTagTool->setJetAuthor(m_jetReader->getContainerName());
  for (const auto* const jetPtr : inputJets) {
    // if(! Props::isVBFJet.get(jetPtr)) continue;
    if (!Props::isVetoJet.get(jetPtr)) continue;

    bool isTagged = (m_bTagTool->isTagged(*jetPtr));

    if ((jetPtr->pt() * 0.001) > 40.0) {  // Study A

      if (fabs(jetPtr->eta()) < 2.5) {
        signalJets2.push_back(jetPtr);
      }

      if (m_doTruthTagging) {
        isTagged = true;
      }

      if ((signalJets.size() < 2) && (fabs(jetPtr->eta()) < 2.5) &&
          (isTagged)) {
        signalJets.push_back(jetPtr);
        ++n_SgnJet_Btag;

      } else {
        nonBtagJets.push_back(jetPtr);
        // vbfJets.push_back(jetPtr);
      }
    }
    ++icj;
  }

  //========================================
  // Get non-btagged signal jets
  // =======================================

  for (const auto* const jetPtr : nonBtagJets) {
    if (signalJets.size() < 2 && fabs(jetPtr->eta()) < 2.5) {
      signalJets.push_back(jetPtr);
    } else {
      vbfJets.push_back(jetPtr);
    }
  }

  // check that we have enough of each type of jet
  if (signalJets.size() < 2 || vbfJets.size() < 2) {
    return false;
  }

  // find the pair of jets that yields the highest mjj
  const xAOD::Jet* vbfjet1(nullptr);
  const xAOD::Jet* vbfjet2(nullptr);
  double maxMjj(-1.);
  for (const auto* const jetPtr1 : vbfJets) {
    for (const auto* const jetPtr2 : vbfJets) {
      if (jetPtr1 == jetPtr2) continue;  // skip if they are the same jet
      // if(! (jetPtr1->eta()*jetPtr2->eta() < 0 )) continue; // require jets to
      // be in opposite hemispheres
      TLorentzVector jet1(jetPtr1->p4()), jet2(jetPtr2->p4());
      if ((jet1 + jet2).M() > maxMjj) {
        maxMjj = (jet1 + jet2).M();
        vbfjet1 = jetPtr1;
        vbfjet2 = jetPtr2;
      }
    }
  }

  // if we didn't find any vbf jets
  if (maxMjj < 0) return false;

  //   // select remaining jets
  //   for(const auto* const jetPtr : inputJets){
  //	  if ((jetPtr != signalJets.at(0)) && (jetPtr != signalJets.at(1)) &&
  //			(jetPtr != vbfjet1) && (jetPtr != vbfjet2)) {
  //
  //		 TLorentzVector sjet = jetPtr->p4();
  //		 float dRphjet = m_phVec.DeltaR(sjet);
  //
  //		 bool  passjet = true;
  //		 if (fabs(sjet.Eta()) > 2.5) {
  //			if ((sjet.Pt()*0.001) < 30.0) {passjet = false;}
  //		 }
  //		 if (dRphjet < 0.4) {  // not photon
  //			passjet = false;
  //		 }
  //
  //		 if ( passjet ) {
  //			vbfJets2.push_back(jetPtr);  // list of jets not signal
  // bb, not vbf JJ. and not photon
  //
  //			++m_numSoftJet1;
  //			m_SumEtSoftJet1 = m_SumEtSoftJet1 + (sjet.Pt()*0.001);
  //
  //			// within VBF jets
  //			bool ibtwVBF = false;
  //			if ( vbfjet1->eta() >  vbfjet2->eta() ) {
  //			   if ( (sjet.Eta() > vbfjet2->eta()) &&  (sjet.Eta() <
  // vbfjet1->eta()) ) { 				  ibtwVBF = true;
  //			   }
  //			} else {
  //			   if ( (sjet.Eta() > vbfjet1->eta()) &&  (sjet.Eta() <
  // vbfjet2->eta()) ) { 				  ibtwVBF = true;
  //			   }
  //			}
  //
  //			if (ibtwVBF) {
  //			   ++m_numSoftJet2;
  //			   m_SumEtSoftJet2 = m_SumEtSoftJet2 +
  //(sjet.Pt()*0.001);
  //
  //			   if (fabs(sjet.Eta()) < 2.5) {
  //				  ++m_numSoftJet4;
  //				  m_SumEtSoftJet4 = m_SumEtSoftJet4 +
  //(sjet.Pt()*0.001);
  //			   }
  //			}
  //
  //			if (fabs(sjet.Eta()) < 2.5) {
  //			   ++m_numSoftJet3;
  //			   m_SumEtSoftJet3 = m_SumEtSoftJet3 +
  //(sjet.Pt()*0.001);
  //			}
  //
  //		 }
  //	  }
  //   }

  m_bTagTool->setJetAuthor(m_jetReader->getContainerName());

  if ((m_bTagTool->isTagged(*(signalJets.at(0)))) || (m_doTruthTagging)) {
    tagcatExcl++;
    BTagProps::isTagged.set(signalJets.at(0), 1);
  }
  if ((m_bTagTool->isTagged(*(signalJets.at(1)))) || (m_doTruthTagging)) {
    tagcatExcl++;
    BTagProps::isTagged.set(signalJets.at(1), 1);
  }

  my_nbtag_signalJets = 0;
  // check number of signal jet is b-tagged
  for (unsigned i = 0; i < signalJets2.size(); ++i) {
    if (m_bTagTool->isTagged(*(signalJets2.at(i)))) {
      ++my_nbtag_signalJets;
    }
  }

  //   m_my_njetPt40 = my_njetPt40;
  //   // m_my_nbtag    = my_nbtag;
  //   m_my_nbtag    = my_nbtag_signalJets;

  // set the output jets
  // sort the signal jets by pt
  // sort the vbf jets by pt
  // do nothing with any of the other jets
  vbfJets.clear();
  if (vbfjet1->pt() > vbfjet2->pt()) {
    vbfJets.push_back(vbfjet1);
    vbfJets.push_back(vbfjet2);
  } else {
    vbfJets.push_back(vbfjet2);
    vbfJets.push_back(vbfjet1);
  }
  if (signalJets.at(0)->pt() < signalJets.at(1)->pt()) {
    std::swap(signalJets.at(0), signalJets.at(1));
  }

  return true;
}

bool AnalysisReader_VBFHbb1Ph::jet_selection_vbfincl4cen(
    std::vector<const xAOD::Jet*> inputJets) {
  std::vector<const xAOD::Jet*> signalJets;
  signalJets.clear();
  int njet = 0;

  for (const auto* const jetPtr :
       inputJets) {  // select signal jets; veto fwd jet event
    if (!Props::isVetoJet.get(jetPtr)) continue;
    if (jetPtr->pt() < 55000) continue;
    ++njet;
    if (jetPtr->pt() > 60000 && fabs(jetPtr->eta()) > 3.2 &&
        fabs(jetPtr->eta()) < 4.4)
      return false;  // veto fwd jet event
    bool isTagged = Props::MV2c10.get(jetPtr) > 0.8244273;  // 70%
    if (isTagged && signalJets.size() < 2) signalJets.push_back(jetPtr);
  }
  if (njet < 4) return false;               // 4 jets with pT>55
  if (signalJets.size() < 2) return false;  // 2 b-tagged jets 70%
  if ((signalJets.at(0)->p4() + signalJets.at(1)->p4()).Pt() < 150000)
    return false;  // pTBB>150 GeV

  return true;
}

bool AnalysisReader_VBFHbb1Ph::jet_selection_vbfincl2cen(
    std::vector<const xAOD::Jet*> inputJets) {
  std::vector<const xAOD::Jet*> signalJets1;
  std::vector<const xAOD::Jet*> signalJets2;
  signalJets1.clear();
  signalJets2.clear();
  int njet = 0;
  int nfwd = 0;

  for (const auto* const jetPtr : inputJets) {  // count number of jets
    if (!Props::isVetoJet.get(jetPtr)) continue;
    if (jetPtr->pt() < 20000) continue;
    bool isTagged70 = Props::MV2c10.get(jetPtr) > 0.8244273;  // 70%
    bool isTagged85 = Props::MV2c10.get(jetPtr) > 0.1758475;  // 85%
    ++njet;
    if (jetPtr->pt() > 90000 && isTagged70)
      signalJets1.push_back(jetPtr);
    else if (jetPtr->pt() > 70000 && isTagged85)
      signalJets2.push_back(jetPtr);
    if (jetPtr->pt() > 60000 && fabs(jetPtr->eta()) > 3.2 &&
        fabs(jetPtr->eta()) < 4.4)
      ++nfwd;
  }
  if (signalJets1.size() < 1) return false;  // 1 jet with pT>90 btag 70%
  if (signalJets2.size() < 1) return false;  // 2 jets with pT>70, btag 85%
  if (nfwd < 1) return false;                // 1 fwd jet
  if (njet < 4) return false;                // 4 jets with pT>20

  float pTBB = 0;
  if (signalJets1.size() >= 2)
    pTBB = (signalJets1.at(0)->p4() + signalJets1.at(1)->p4()).Pt();
  else
    pTBB = (signalJets1.at(0)->p4() + signalJets2.at(0)->p4()).Pt();
  if (pTBB < 160000) return false;  // pTBB>160 GeV

  return true;
}

std::pair<std::pair<float, float>, float>
AnalysisReader_VBFHbb1Ph::getCenterSlopeOfEllipse(const xAOD::Jet* jet1,
                                                  const xAOD::Jet* jet2) {
  float eta0, eta1, eta2, phi0, phi1, phi2, m;
  if (jet1->phi() > jet2->phi()) {
    phi1 = jet1->phi();
    eta1 = jet1->eta();
    phi2 = jet2->phi();
    eta2 = jet2->eta();
  } else {
    phi1 = jet2->phi();
    eta1 = jet2->eta();
    phi2 = jet1->phi();
    eta2 = jet1->eta();
  }

  eta0 = 0.5 * (eta1 + eta2);
  float dphi = phi1 - phi2;
  if (dphi > TMath::Pi()) phi2 = phi2 + 2. * TMath::Pi();

  phi0 = 0.5 * (phi1 + phi2);

  m = (phi1 - phi0) / (eta1 - eta0);

  if (phi0 > TMath::Pi()) phi0 = phi0 - 2. * TMath::Pi();

  // std::printf("jet1 eta/phi : %f/%f\n jet2 eta/phi : %f/%f\n\n",
  // jet1->eta(), jet1->phi(), jet2->eta(), jet2->phi());

  // std::printf("Ellipse eta0/phi0/m: %f/%f/%f\n", eta0, phi0, m);
  std::pair<float, float> C(eta0, phi0);
  std::pair<std::pair<float, float>, float> El(C, m);

  return El;
}

bool AnalysisReader_VBFHbb1Ph::trackJetIsInEllipse(
    std::pair<std::pair<float, float>, float> El, const xAOD::Jet* jet,
    float dRBB, float r) {
  float eta0 = El.first.first;
  float phi0 = El.first.second;
  float m = El.second;
  float m_p = -1. / m;

  float etaj = jet->eta();
  float phij = jet->phi();

  if (fabs(phi0 - phij) > TMath::Pi()) {
    if (phi0 > phij)
      phij = phij + 2 * TMath::Pi();
    else
      phij = phij - 2 * TMath::Pi();
  }

  float eta_I1 = (1. / (m - m_p)) * (m * eta0 - m_p * etaj - phi0 + phij);
  float phi_I1 = m * (eta_I1 - eta0) + phi0;

  float eta_I2 = (1. / (m_p - m)) * (m_p * eta0 - m * etaj - phi0 + phij);
  float phi_I2 = m_p * (eta_I2 - eta0) + phi0;

  // not sure below here!!!
  float dEta1 = eta_I1 - etaj;
  float dPhi1 = phi_I1 - phij;

  float dEta2 = eta_I2 - etaj;
  float dPhi2 = phi_I2 - phij;

  float d1sq = dPhi1 * dPhi1 + dEta1 * dEta1;
  float d2sq = dPhi2 * dPhi2 + dEta2 * dEta2;

  float b = r;
  float a = 0.5 * dRBB + r;

  bool inEllipse = ((d1sq / (b * b)) + (d2sq / (a * a))) < 1;

  return inEllipse;
}

//=========================================================
double AnalysisReader_VBFHbb1Ph::GetBjetTriggerWeightEta(double eta,
                                                         int variation) {
  double sf = m_BJetTrig_EFF_Event->Eval(eta);
  double var = GetBjetSFVariation(eta, hist_BJetTrig_EFF_Event,
                                  m_BJetTrig_EFF_Event, variation);

  sf += var;
  if (sf > 1) {  // protection because this SF is actually eff/1.0
    Warning("GetBjetTriggerWeightEta()", "SF>1. Force it to 1.");
    sf = 1.0;
  }
  return sf;
}

double AnalysisReader_VBFHbb1Ph::GetBjetTriggerWeightPt(double pt1, double pt2,
                                                        bool match1,
                                                        bool match2,
                                                        int variation) {
  double b1SF = 1.0;
  double b2SF = 1.0;
  double totWt = 1.0;

  // SF is only upto 700 GeV
  if (pt1 >= 700) pt1 = 699;
  if (pt2 >= 700) pt2 = 699;

  double perJetSFb1 = m_BJetTrig_SF_Jet->Eval(pt1);
  double perJetSFb2 = m_BJetTrig_SF_Jet->Eval(pt2);
  double varb1 = GetBjetSFVariation(pt1, hist_BJetTrig_EFF_Jet,
                                    m_BJetTrig_SF_Jet, variation);
  double varb2 = GetBjetSFVariation(pt2, hist_BJetTrig_EFF_Jet,
                                    m_BJetTrig_SF_Jet, variation);
  perJetSFb1 += varb1;
  perJetSFb2 += varb2;

  // calculate event level SF
  // (assume data eff has no syst. Propogate the per-jet SF variation directly
  // to the ineffSF
  if (match1) {
    b1SF = perJetSFb1;
  } else {
    b1SF = getPerJetIneffSF(m_BJetTrig_EFF_Jet->Eval(pt1), perJetSFb1);
  }

  if (match2) {
    b2SF = perJetSFb2;
  } else {
    b2SF = getPerJetIneffSF(m_BJetTrig_EFF_Jet->Eval(pt2), perJetSFb2);
  }

  totWt = b1SF * b2SF;
  //  std::cout<<"variation"<<variation<<" pt1"<<pt1<<" pt2"<<pt2<<"
  //  b1SF"<<b1SF<<" b2SF"<<b2SF<<" totWt"<<totWt<<std::endl;
  return totWt;
}

double AnalysisReader_VBFHbb1Ph::getPerJetIneffSF(double eff_data,
                                                  double eff_sf) {
  double eff_mc = eff_data / eff_sf;
  double ineff_sf = (1 - eff_data) / (1 - eff_mc);
  //   std::cout<<"effdt"<<eff_data<<" eff_mc"<<eff_mc<<"
  //   ineff_sf"<<ineff_sf<<std::endl;
  return ineff_sf;
}
double AnalysisReader_VBFHbb1Ph::GetBjetSFVariation(double x, TH1F* hist,
                                                    TGraphAsymmErrors* graph,
                                                    int variation) {
  if (variation == 0) {  // nominal
    return 0;
  } else {
    double var = 0;

    // get bin index
    int binx = hist->GetXaxis()->FindBin(x) -
               1;  // histogram bin index starts from 1, while graph point index
                   // starts from 0.
    int nbin = graph->GetN();
    if (binx < 0) {
      Warning("GetBjetSFVariation()",
              "Bin index %d out of range. Use the value in the first bin.",
              binx);
      binx = 0;
    } else if (binx > nbin - 1) {
      Warning("GetBjetSFVariation()",
              "Bin index %d out of range. Use the value in the last bin.",
              binx);
      binx = nbin - 1;
    }

    // get variation
    if (variation == 1) {
      var = graph->GetErrorYhigh(binx);
      // std::cout<<x<<" "<<binx<<" "<<var<<std::endl;
    } else if (variation == -1) {  // down
      var = -graph->GetErrorYlow(binx);
    } else {  // when variation is not 0, 1, or -1
      Warning("GetBjetSFVariation()", "Invalid variation. Return nominal SF.");
    }
    return var;
  }
}
//=========================================================
