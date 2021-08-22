#include <type_traits>
//#include <EventLoop/IWorker.h>

#include <CxAODReader_VHbb/AnalysisReader_VGamma.h>
#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
/*
#include <CxAODReader_VHbb/AnalysisReader_VHQQ.h>
#include <EventLoop/StatusCode.h>
#include "CxAODTools_VHbb/VBFHbbInclEvtSelection.h"
#include "CxAODTools_VHbb/VGammaEvtSelection.h"
#include "TFile.h"
#include "TLorentzVector.h"
ClassImp(AnalysisReader)
*/
//#define length(array) (sizeof(array) / sizeof(*(array)))

bool doTruthJets = false;

AnalysisReader_VGamma::AnalysisReader_VGamma() : AnalysisReader_VHQQ() {}

AnalysisReader_VGamma::~AnalysisReader_VGamma() {}

EL::StatusCode AnalysisReader_VGamma::initializeSelection() {
  AnalysisReader_VHQQ::initializeSelection();

  m_eventSelection = new VGammaEvtSelection();
  // m_fillFunction = std::bind(&AnalysisReader_VGamma::fill_vgamma, this);
  m_fillFunction = std::bind(&AnalysisReader_VGamma::run_vgamma_analysis, this);
  ((VGammaEvtSelection*)m_eventSelection)->SetAnalysisType("m_analysisType");

  //*********************************************************//
  //                     READ CONFIGURATION                  //
  //*********************************************************//
  return EL::StatusCode::SUCCESS;
}
/*
//Test some of the substructure features for the leading jet in the event
std::vector<float> AnalysisReader_VGamma::getTrkJetMV2C10Vector(const
std::vector<const xAOD::Jet*> FatJets, const int n) { std::vector<float> result,
pt_weight; result.clear(); pt_weight.clear(); int iJet = 0;
  //if(FatJets->size()>0) {
  for (const auto& fatjet : FatJets) {
    const xAOD::Jet* jet = (const xAOD::Jet*) fatjet;//FatJets->at(0);
    //std::cout << "fat jet mass: " << jet->m()/1e3 << std::endl;
    // Test the links to the track jets associated to this fatjet
    std::vector<const xAOD::Jet*> trkJets;
    if ((jet)->getAssociatedObjects<xAOD::Jet>("GhostAntiKt2TrackJet",trkJets)
&& trkJets.size() > 0) { std::sort(trkJets.begin(), trkJets.end(),sort_pt);
      std::reverse(trkJets.begin(), trkJets.end());
      //if (trkJets.size()>2) std::cout<<"Trackjet size:
"<<trkJets.size()<<std::endl; for(const xAOD::Jet* trkJ : trkJets) {
        //std::cout<<"ntrackjet: "<<icount<<std::endl;
        if (!trkJ) continue; //if the trackjet is not valid then skip it
        if (!(Props::isTrackJet.get(trkJ))) continue; //pT > 7 GeV; |eta| < 2.5
        if (!(trkJ->pt() > 10e3)) continue;
        if (!(Props::TrkJetnumConstituents.get(trkJ) >= 2)) continue;
        pt_weight.push_back(trkJ->auxdata<float>("MV2c10"));
      }
      if (pt_weight.size()>(n-1)) result.push_back(pt_weight.at(n-1));
      else result.push_back(-999);
      //if (pt_weight.size()==0 || pt_weight.size()<n) result.push_back(-999);
      //int iter = 0;
      //for (unsigned int it=0; it<pt_weight.size(); it++) {
      //  if ((n-1)==iter) result.push_back(pt_weight.at(it));
      //  iter++;
      //}
      pt_weight.clear();
    }
    else result.push_back(-999);
    iJet++;
  }
  //}
  return result;
}
*/

std::vector<float> AnalysisReader_VGamma::getTrkJetMV2C10Vector(
    const std::vector<const xAOD::Jet*> FatJets, const int n,
    std::string trackLinkName) {
  std::vector<float> result;
  std::multimap<float, float> pt_weight;
  result.clear();
  pt_weight.clear();

  // if(FatJets->size()>0) {
  for (const auto& fatjet : FatJets) {
    const xAOD::Jet* jet = (const xAOD::Jet*)fatjet;  // FatJets->at(0);
    // std::cout << "fat jet mass: " << jet->m()/1e3 << std::endl;
    // Test the links to the track jets associated to this fatjet
    std::vector<const xAOD::Jet*> trkJets;
    // std::string trackLinkName = "GhostAntiKt2TrackJet";
    // m_config->getif<std::string>("trackLinkName", trackLinkName);
    if ((jet)->getAssociatedObjects<xAOD::Jet>(trackLinkName.c_str(),
                                               trkJets) &&
        trkJets.size() > 0) {
      // std::cout<<"Trackjet size: "<<trkJets.size()<<std::endl;
      for (const xAOD::Jet* trkJ : trkJets) {
        // std::cout<<"ntrackjet: "<<icount<<std::endl;
        if (!trkJ) continue;  // if the trackjet is not valid then skip it
        if (!(Props::isTrackJet.get(trkJ)))
          continue;  // pT > 7 GeV; |eta|
                     // < 2.5
        if (!(trkJ->pt() > 10e3)) continue;
        // if (!(Props::TrkJetnumConstituents.get(trkJ) >= 2)) continue;
        pt_weight.insert(std::pair<float, float>(
            trkJ->pt(), trkJ->auxdata<float>(
                            "MV2c10")));  // multimap is sorted by 1st element
      }
      if (pt_weight.size() == 0 || (signed int)pt_weight.size() < n)
        result.push_back(-999.);
      int iter = 0;
      for (auto it = pt_weight.rbegin(); it != pt_weight.rend();
           ++it) {  // due to sorting loop is inverted
        if ((n - 1) == iter) result.push_back(it->second);
        iter++;
      }
      pt_weight.clear();
    } else {
      result.push_back(-999.);
    }
  }
  //}
  return result;
}

std::vector<int> AnalysisReader_VGamma::getTrkJetTruthIDVector(
    const std::vector<const xAOD::Jet*> FatJets, const int n,
    std::string trackLinkName) {
  std::vector<int> result;
  std::multimap<float, int> pt_weight;
  result.clear();
  pt_weight.clear();

  // if(FatJets->size()>0) {
  for (const auto& fatjet : FatJets) {
    const xAOD::Jet* jet = (const xAOD::Jet*)fatjet;  // FatJets->at(0);
    // std::cout << "fat jet mass: " << jet->m()/1e3 << std::endl;
    // Test the links to the track jets associated to this fatjet
    std::vector<const xAOD::Jet*> trkJets;
    if ((jet)->getAssociatedObjects<xAOD::Jet>(trackLinkName.c_str(),
                                               trkJets) &&
        trkJets.size() > 0) {
      // std::cout<<"Trackjet size: "<<trkJets.size()<<std::endl;
      for (const xAOD::Jet* trkJ : trkJets) {
        // std::cout<<"ntrackjet: "<<icount<<std::endl;
        if (!trkJ) continue;  // if the trackjet is not valid then skip it
        if (!(Props::isTrackJet.get(trkJ)))
          continue;  // pT > 7 GeV; |eta|
                     // < 2.5
        if (!(trkJ->pt() > 10e3)) continue;
        // if (!(Props::TrkJetnumConstituents.get(trkJ) >= 2)) continue;
        pt_weight.insert(std::pair<float, int>(
            trkJ->pt(), Props::HadronConeExclTruthLabelID.get(
                            trkJ)));  // multimap is sorted by 1st element
      }
      if (pt_weight.size() == 0 || (signed int)pt_weight.size() < n)
        result.push_back(-999);
      int iter = 0;
      for (auto it = pt_weight.rbegin(); it != pt_weight.rend();
           ++it) {  // due to sorting loop is inverted
        if ((n - 1) == iter) result.push_back(it->second);
        iter++;
      }
      pt_weight.clear();
    } else {
      result.push_back(-999);
    }
  }
  //}
  return result;
}

EL::StatusCode AnalysisReader_VGamma::run_vgamma_analysis() {
  // m_debug = true;

  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;
  if (m_debug) std::cout << " >>>>>>>>>>> Starting run_vgamma " << std::endl;
  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;

  //*********************************************************//
  //                     EVENT INITIALISATION                //
  //*********************************************************//

  // float GeV = 1000.;
  // float invGeV = 0.001;
  // float pi = TMath::Pi();
  ResultVGamma selectionResult =
      ((VGammaEvtSelection*)m_eventSelection)->result();

  //*********************************************************//
  //      INIT ALL VARIABLES FROM THE START                  //
  //*********************************************************//

  std::vector<const xAOD::Photon*> photon = selectionResult.ph;
  std::vector<const xAOD::Jet*> fatjets = selectionResult.fatJets;
  std::vector<const xAOD::Jet*> jets = selectionResult.jets4;
  std::vector<const xAOD::Jet*> trackJets = selectionResult.trackJets;
  std::vector<const xAOD::Jet*> associatedTrackJets;
  std::vector<const xAOD::Jet*> unassociatedTrackJets;

  // Sort all objetcs by pt just in case
  std::sort(fatjets.begin(), fatjets.end(), sort_pt);
  std::sort(jets.begin(), jets.end(), sort_pt);
  std::sort(trackJets.begin(), trackJets.end(), sort_pt);
  std::sort(photon.begin(), photon.end(), sortPhotons_pt);

  //*********************************************************//
  //                    CALCULATE WEIGHTS                    //
  //*********************************************************//

  // **Weight**: Photon
  // double photonSF = 1.;
  // if (m_isMC) {
  //  photonSF =
  //  (Props::photonFixedCutTightEffSF.get(photon)*Props::tightEffSF.get(photon));
  //}
  // std::printf("Photon SF: %f\n", photonSF);

  ////m_weight *= photonSF;

  // **Weight** : Trigger
  double triggerSF = 1.;
  // bool passTrig = passVBF1PhTrigger();
  // m_weight *= triggerSF;

  // **Weight** : b-tagging
  double btagSF1 = 1., btagSF4 = 1.;
  compute_btagging();
  compute_fatjetTags(trackJets, fatjets, &associatedTrackJets,
                     &unassociatedTrackJets);
  if (m_isMC) {
    std::vector<const xAOD::Jet*> trackJetsForBTagging = associatedTrackJets;
    trackJetsForBTagging.insert(trackJetsForBTagging.end(),
                                unassociatedTrackJets.begin(),
                                unassociatedTrackJets.end());
    btagSF1 = computeBTagSFWeight(trackJetsForBTagging,
                                  m_trackJetReader->getContainerName());
    if (btagSF1 <= 0) {
      Info("AnalysisReader_VGamma::run_vgamma_analysis()",
           "btagSF1 = %f, be warned", btagSF1);
    }

    btagSF4 = computeBTagSFWeight(jets, m_jetReader->getContainerName());
    if (btagSF4 <= 0) {
      Info("AnalysisReader_VGamma::run_vgamma_analysis()",
           "btagSF4 = %f, be warned", btagSF4);
    }
    //  btagSF1 = computeBTagSFWeight(fatjets, m_jetReader->getContainerName());
    //  btagSF4 = computeBTagSFWeight(jets, m_jetReader->getContainerName());
  }

  // m_weight *= btagSF;

  // **Weight** : pileup
  double puWeight = m_pileupReweight;
  if (!m_config->get<bool>("applyPUWeight"))
    m_weightSysts.push_back({"_withPU", (float)(m_pileupReweight)});

  if (m_debug) {
    std::cout << " >>>>> Applied Weights" << std::endl;
    // std::printf("photonSF: %f\n", photonSF);
    std::printf("triggerSF: %f\n", triggerSF);
    std::printf("btagSF1: %f\n", btagSF1);
    std::printf("btagSF4: %f\n", btagSF4);
    std::printf("puWeight: %f\n", m_pileupReweight);
  }

  //*********************************************************//
  //               CALCULATE EVENT VARIABLES                 //
  //*********************************************************//

  int NVtx2Trks = 0, NVtx3Trks = 0;
  if (Props::NVtx2Trks.exists(m_eventInfo))
    NVtx2Trks = Props::NVtx2Trks.get(m_eventInfo);
  else if (Props::NVtx3Trks.exists(m_eventInfo))
    NVtx3Trks = Props::NVtx3Trks.get(m_eventInfo);

  //*********************************************************//
  //               CALCULATE TRIGGER VARIABLES               //
  //*********************************************************//

  // analysis trigger
  int passTrigger = Props::passHLT_g140_loose.get(m_eventInfo);
  int matchTrigger = Props::matchHLT_g140_loose.get(photon.at(0));

  // Calculate MVA Pre selction
  int m_passMVAPreSel = 1;
  if (!passTrigger) m_passMVAPreSel = 0;
  if (!matchTrigger) m_passMVAPreSel = 0;

  //*********************************************************//
  //                       MVA TREE                          //
  //*********************************************************//
  m_etree->Reset();
  m_etree->SetVariation(m_currentVar);

  int njets = 0, njets4 = 0, n_photons = 0, n_truth = 0, runN = 0, lumiB = 0;
  double sample_weight = 1., mc_weight = 1.;
  sample_weight = Props::LumiWeight.get(m_eventInfo);
  if (m_isMC) mc_weight = Props::MCEventWeight.get(m_eventInfo);
  runN = m_eventInfo->runNumber();
  lumiB = m_eventInfo->lumiBlock();

  m_etree->SetBranchAndValue("pileup_weight", puWeight, 1.);
  m_etree->SetBranchAndValue("sample_weight", sample_weight, 1.);
  m_etree->SetBranchAndValue("mc_weight", mc_weight, 1.);
  m_etree->SetBranchAndValue<int>("runNumber", runN, -999);
  m_etree->SetBranchAndValue<int>("lumiBlock", lumiB, -999);

  m_etree->SetBranchAndValue("passTrig", passTrigger, 0);
  m_etree->SetBranchAndValue("matchTrig", matchTrigger, 0);
  m_etree->SetBranchAndValue("passMVAPreSel", m_passMVAPreSel, 0);

  m_etree->SetBranchAndValue("MCChannelNumber", m_mcChannel, -999);
  m_etree->SetBranchAndValue("weight", m_weight, 1.);
  m_etree->SetBranchAndValue("BTagSF1", btagSF1, 1.);
  m_etree->SetBranchAndValue("BTagSF4", btagSF4, 1.);
  // m_etree->SetBranchAndValue("PhotonSF", photonSF, 1.);
  m_etree->SetBranchAndValue("eventNumber", m_eventInfo->eventNumber(),
                             (long long unsigned int)(-999));

  m_etree->SetBranchAndValue("avgIntPerX", m_averageMu, float(-999));
  m_etree->SetBranchAndValue("NVtx2Trks", NVtx2Trks, -999);
  m_etree->SetBranchAndValue("NVtx3Trks", NVtx3Trks, -999);
  m_etree->SetBranchAndValue("zPV", Props::ZPV.get(m_eventInfo), float(-999));

  njets = fatjets.size();

  std::vector<float> jet_pt, jet_eta, jet_phi, jet_m, jet_trkJetKt2MV2c10_1,
      jet_trkJetKt2MV2c10_2, jet_d2, jet_xbb_eta, jet_xbb_phi, jet_xbb_pt,
      jet_xbb_m, jet_truth_pt, jet_truth_eta, jet_truth_phi, jet_truth_m,
      jet_pt_check, jet_mjg, jet_tmp, dummy_double, jet_trkJetVRMV2c10_1,
      jet_trkJetVRMV2c10_2;
  std::vector<int> jet_nbtags, jet_ungrtrk500, jet_passedWSubstructure50,
      jet_passedZSubstructure50, jet_passedWSubstructure80,
      jet_passedZSubstructure80, jet_passedWMassCut50, jet_passedZMassCut50,
      jet_passedWMassCut80, jet_passedZMassCut80, jet_isWJet50, jet_isZJet50,
      jet_isWJet80, jet_isZJet80, jet_xbbResult, jet_trkJetKt2TruthID_1,
      jet_trkJetKt2TruthID_2, jet_trkJetVRTruthID_1, jet_trkJetVRTruthID_2,
      dummy_int;
  jet_pt.clear(), jet_eta.clear(), jet_phi.clear(), jet_m.clear(),
      jet_trkJetKt2MV2c10_1.clear(), jet_trkJetKt2MV2c10_2.clear(),
      jet_trkJetVRMV2c10_1.clear(), jet_trkJetVRMV2c10_2.clear(),
      jet_d2.clear(), dummy_double.clear(), jet_nbtags.clear(),
      jet_ungrtrk500.clear(), jet_passedWSubstructure50.clear(),
      jet_passedZSubstructure50.clear(), jet_passedWSubstructure80.clear(),
      jet_passedZSubstructure80.clear(), jet_passedWMassCut50.clear(),
      jet_passedZMassCut50.clear(), jet_passedWMassCut80.clear(),
      jet_passedZMassCut80.clear(), jet_isWJet50.clear(), jet_isZJet50.clear(),
      jet_isWJet80.clear(), jet_isZJet80.clear(), jet_xbb_eta.clear(),
      jet_xbbResult.clear(), jet_xbb_eta.clear(), jet_xbb_phi.clear(),
      jet_xbb_pt.clear(), jet_xbb_m.clear(), jet_trkJetKt2TruthID_1.clear(),
      jet_trkJetKt2TruthID_2.clear(), jet_trkJetVRTruthID_1.clear(),
      jet_trkJetVRTruthID_2.clear(), jet_truth_pt.clear(),
      jet_truth_eta.clear(), jet_truth_phi.clear(), jet_truth_m.clear(),
      jet_pt_check.clear(), jet_mjg.clear(), jet_tmp.clear(), dummy_int.clear();

  TLorentzVector fatjetVecCorr;
  xAOD::JetFourMom_t tJet;

  jet_trkJetKt2MV2c10_1 =
      getTrkJetMV2C10Vector(fatjets, 1, "GhostAntiKt2TrackJet");
  jet_trkJetKt2MV2c10_2 =
      getTrkJetMV2C10Vector(fatjets, 2, "GhostAntiKt2TrackJet");
  jet_trkJetVRMV2c10_1 =
      getTrkJetMV2C10Vector(fatjets, 1, "GhostVR30Rmax4Rmin02TrackJet");
  jet_trkJetVRMV2c10_2 =
      getTrkJetMV2C10Vector(fatjets, 2, "GhostVR30Rmax4Rmin02TrackJet");

  if (m_isMC) {
    jet_trkJetKt2TruthID_1 =
        getTrkJetTruthIDVector(fatjets, 1, "GhostAntiKt2TrackJet");
    jet_trkJetKt2TruthID_2 =
        getTrkJetTruthIDVector(fatjets, 2, "GhostAntiKt2TrackJet");
    jet_trkJetVRTruthID_1 =
        getTrkJetTruthIDVector(fatjets, 1, "GhostVR30Rmax4Rmin02TrackJet");
    jet_trkJetVRTruthID_2 =
        getTrkJetTruthIDVector(fatjets, 2, "GhostVR30Rmax4Rmin02TrackJet");
  }

  if (fatjets.size() > 1 && fatjets.at(0)->pt() < fatjets.at(1)->pt())
    std::cout << "WE GOT PROBLEM IN FATJETS " << fatjets.size() << "  "
              << fatjets.at(0)->pt() << "  " << fatjets.at(1)->pt()
              << std::endl;
  if (photon.size() > 1 && photon.at(0)->pt() < photon.at(1)->pt())
    std::cout << "WE GOT PROBLEM IN PHOTONS" << std::endl;
  for (unsigned int ij = 0; ij < fatjets.size(); ++ij) {
    fatjetVecCorr.Clear();
    fatjetVecCorr = fatjets.at(ij)->p4();
    jet_eta.push_back(fatjets.at(ij)->eta());
    jet_phi.push_back(fatjets.at(ij)->phi());
    jet_nbtags.push_back(Props::nBTags.get(fatjets.at(ij)));
    if (doTruthJets) {
      tJet = fatjets.at(ij)->jetP4("TruthWZ");
      jet_truth_pt.push_back(tJet.pt());
      jet_truth_eta.push_back(tJet.eta());
      jet_truth_phi.push_back(tJet.phi());
      jet_truth_m.push_back(tJet.mass());
    }
    // if (associatedTrackJets.size()>0)
    // jet_trkJetMV2c10_1.push_back(Props::MV2c10.get(associatedTrackJets.at(0)));
    // else jet_trkJetMV2c10_1.push_back(-999.);
    // if (associatedTrackJets.size()>1)
    // jet_trkJetMV2c10_2.push_back(Props::MV2c10.get(associatedTrackJets.at(1)));
    // else jet_trkJetMV2c10_2.push_back(-999.);
    /////jet_trkJetMV2c10_1.push_back(getTrkJetMV2C10Vector(fatjets,1)[ij]);
    /////jet_trkJetMV2c10_2.push_back(getTrkJetMV2C10Vector(fatjets,2)[ij]);
    jet_ungrtrk500.push_back(Props::NumTrkPt500PV.get(fatjets.at(ij)));
    jet_passedWSubstructure50.push_back(
        Props::passWSubstructure50.get(fatjets.at(ij)));
    jet_passedZSubstructure50.push_back(
        Props::passZSubstructure50.get(fatjets.at(ij)));
    jet_passedWSubstructure80.push_back(
        Props::passWSubstructure80.get(fatjets.at(ij)));
    jet_passedZSubstructure80.push_back(
        Props::passZSubstructure80.get(fatjets.at(ij)));
    jet_passedWMassCut50.push_back(
        Props::passWLowMassCut50.get(fatjets.at(ij)) &
        Props::passWHighMassCut50.get(fatjets.at(ij)));
    jet_passedZMassCut50.push_back(
        Props::passZLowMassCut50.get(fatjets.at(ij)) &
        Props::passZHighMassCut50.get(fatjets.at(ij)));
    jet_passedWMassCut80.push_back(
        Props::passWLowMassCut80.get(fatjets.at(ij)) &
        Props::passWHighMassCut80.get(fatjets.at(ij)));
    jet_passedZMassCut80.push_back(
        Props::passZLowMassCut80.get(fatjets.at(ij)) &
        Props::passZHighMassCut80.get(fatjets.at(ij)));
    jet_isWJet50.push_back(Props::passWSubstructure50.get(fatjets.at(ij)) &
                           Props::passWLowMassCut50.get(fatjets.at(ij)) &
                           Props::passWHighMassCut50.get(fatjets.at(ij)));
    jet_isZJet50.push_back(Props::passZSubstructure50.get(fatjets.at(ij)) &
                           Props::passZLowMassCut50.get(fatjets.at(ij)) &
                           Props::passZHighMassCut50.get(fatjets.at(ij)));
    jet_isWJet80.push_back(Props::passWSubstructure80.get(fatjets.at(ij)) &
                           Props::passWLowMassCut80.get(fatjets.at(ij)) &
                           Props::passWHighMassCut80.get(fatjets.at(ij)));
    jet_isZJet80.push_back(Props::passZSubstructure80.get(fatjets.at(ij)) &
                           Props::passZLowMassCut80.get(fatjets.at(ij)) &
                           Props::passZHighMassCut80.get(fatjets.at(ij)));
    jet_pt.push_back(fatjets.at(ij)->pt());
    jet_m.push_back(fatjets.at(ij)->m());
    jet_d2.push_back(Props::D2.get(fatjets.at(ij)));
    jet_tmp.push_back(-999.);
    jet_mjg.push_back((fatjets.at(ij)->p4() + photon.at(0)->p4()).M());
    // xbb jet p4 corrections: see
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BoostedXbbTagger#Tagger_algorithm
    getBJetEnergyCorrTLV(
        fatjets.at(ij), fatjetVecCorr, true,
        "xbb");  // Correct the fat-jet mass by putting the matched muon back
    jet_xbb_eta.push_back(fatjetVecCorr.Eta());
    jet_xbb_phi.push_back(fatjetVecCorr.Phi());
    jet_xbb_pt.push_back(fatjetVecCorr.Pt());
    jet_xbb_m.push_back(fatjetVecCorr.M());
    jet_xbbResult.push_back(Props::xbbResult.get(fatjets.at(ij)));
  }

  if (jet_eta.size() != jet_pt.size() || jet_eta.size() != jet_d2.size() ||
      jet_eta.size() != jet_m.size())
    std::cout << "WE GOT PROBLEM IN FATJETS CORRECTIONS " << jet_eta.size()
              << "  " << jet_pt.size() << "  " << jet_d2.size() << "  "
              << jet_m.size() << std::endl;

  m_etree->SetBranchAndValue<int>("n_jets", njets, -999);
  m_etree->SetBranchAndValue("jet_pt", jet_pt, dummy_double);
  m_etree->SetBranchAndValue("jet_pt_check", jet_pt_check, dummy_double);
  m_etree->SetBranchAndValue("jet_eta", jet_eta, dummy_double);
  m_etree->SetBranchAndValue("jet_phi", jet_phi, dummy_double);
  m_etree->SetBranchAndValue("jet_mass", jet_m, dummy_double);
  m_etree->SetBranchAndValue("jet_mjg", jet_mjg, dummy_double);
  m_etree->SetBranchAndValue("jet_tmp", jet_tmp, dummy_double);
  if (doTruthJets) {
    m_etree->SetBranchAndValue("truthjet_pt", jet_truth_pt, dummy_double);
    m_etree->SetBranchAndValue("truthjet_eta", jet_truth_eta, dummy_double);
    m_etree->SetBranchAndValue("truthjet_phi", jet_truth_phi, dummy_double);
    m_etree->SetBranchAndValue("truthjet_mass", jet_truth_m, dummy_double);
  }
  m_etree->SetBranchAndValue("jet_nBTags", jet_nbtags, dummy_int);
  m_etree->SetBranchAndValue("jet_Kt2mv2c10_1", jet_trkJetKt2MV2c10_1,
                             dummy_double);
  m_etree->SetBranchAndValue("jet_Kt2mv2c10_2", jet_trkJetKt2MV2c10_2,
                             dummy_double);
  m_etree->SetBranchAndValue("jet_VRmv2c10_1", jet_trkJetVRMV2c10_1,
                             dummy_double);
  m_etree->SetBranchAndValue("jet_VRmv2c10_2", jet_trkJetVRMV2c10_2,
                             dummy_double);
  m_etree->SetBranchAndValue("jet_Kt2TruthID_1", jet_trkJetKt2TruthID_1,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_Kt2TruthID_2", jet_trkJetKt2TruthID_2,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_VRTruthID_1", jet_trkJetVRTruthID_1,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_VRTruthID_2", jet_trkJetVRTruthID_2,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_d2", jet_d2, dummy_double);
  m_etree->SetBranchAndValue("jet_ungrtrk500", jet_ungrtrk500, dummy_int);
  m_etree->SetBranchAndValue("jet_passedWSubstructure50",
                             jet_passedWSubstructure50, dummy_int);
  m_etree->SetBranchAndValue("jet_passedZSubstructure50",
                             jet_passedZSubstructure50, dummy_int);
  m_etree->SetBranchAndValue("jet_passedWSubstructure80",
                             jet_passedWSubstructure80, dummy_int);
  m_etree->SetBranchAndValue("jet_passedZSubstructure80",
                             jet_passedZSubstructure80, dummy_int);
  m_etree->SetBranchAndValue("jet_passedWMassCut50", jet_passedWMassCut50,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_passedZMassCut50", jet_passedZMassCut50,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_passedWMassCut80", jet_passedWMassCut80,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_passedZMassCut80", jet_passedZMassCut80,
                             dummy_int);
  m_etree->SetBranchAndValue("jet_isWJet50", jet_isWJet50, dummy_int);
  m_etree->SetBranchAndValue("jet_isZJet50", jet_isZJet50, dummy_int);
  m_etree->SetBranchAndValue("jet_isWJet80", jet_isWJet80, dummy_int);
  m_etree->SetBranchAndValue("jet_isZJet80", jet_isZJet80, dummy_int);
  m_etree->SetBranchAndValue("jet_xbb_pt", jet_xbb_pt, dummy_double);
  m_etree->SetBranchAndValue("jet_xbb_eta", jet_xbb_eta, dummy_double);
  m_etree->SetBranchAndValue("jet_xbb_phi", jet_xbb_phi, dummy_double);
  m_etree->SetBranchAndValue("jet_xbb_mass", jet_xbb_m, dummy_double);
  m_etree->SetBranchAndValue("jet_xbbResult", jet_xbbResult, dummy_int);

  vector<float> jet4_pt, jet4_eta, jet4_phi, jet4_m, jet4_mv2c10, jet4_jvt,
      jet4_jvtSF, jet4_width;
  vector<int> jet4_truthID, jet4_ungrtrk, jet4_isBad, jet4_goodJet,
      jet4_passPreSel;

  jet4_pt.clear(), jet4_eta.clear(), jet4_phi.clear(), jet4_m.clear(),
      jet4_mv2c10.clear(), jet4_jvt.clear(), jet4_jvtSF.clear(),
      jet4_width.clear(), jet4_truthID.clear(), jet4_ungrtrk.clear(),
      jet4_isBad.clear(), jet4_goodJet.clear(), jet4_passPreSel.clear();

  int jet4_nBTags = 0;

  for (unsigned int ij = 0; ij < jets.size(); ++ij) {
    jet4_pt.push_back(jets.at(ij)->pt());
    jet4_eta.push_back(jets.at(ij)->eta());
    jet4_phi.push_back(jets.at(ij)->phi());
    jet4_m.push_back(jets.at(ij)->m());
    jet4_mv2c10.push_back(Props::MV2c10.get(jets.at(ij)));
    // jet4_jvt.push_back(Props::Jvt.get(jets.at(ij)));
    // jet4_jvtSF.push_back(Props::JvtSF.get(jets.at(ij)));
    jet4_width.push_back(Props::Width.get(jets.at(ij)));
    if (BTagProps::isTagged.get(jets.at(ij))) jet4_nBTags++;
    if (m_isMC)
      jet4_truthID.push_back(
          Props::HadronConeExclTruthLabelID.get(jets.at(ij)));
    jet4_ungrtrk.push_back(Props::NumTrkPt500PV.get(jets.at(ij)));
    jet4_isBad.push_back(Props::isVetoJet.get(jets.at(ij)));
    jet4_goodJet.push_back(Props::goodJet.get(jets.at(ij)));
    jet4_passPreSel.push_back(Props::PassJvtMedium.get(jets.at(ij)));
  }

  njets4 = jets.size();
  m_etree->SetBranchAndValue<int>("n_jets4", njets4, -999);
  m_etree->SetBranchAndValue("jet4_pt", jet4_pt, dummy_double);
  m_etree->SetBranchAndValue("jet4_eta", jet4_eta, dummy_double);
  m_etree->SetBranchAndValue("jet4_phi", jet4_phi, dummy_double);
  m_etree->SetBranchAndValue("jet4_m", jet4_m, dummy_double);
  m_etree->SetBranchAndValue("jet4_mv2c10", jet4_mv2c10, dummy_double);
  m_etree->SetBranchAndValue("jet4_nBTags", jet4_nBTags, -999);
  m_etree->SetBranchAndValue("jet4_truthID", jet4_truthID, dummy_int);
  m_etree->SetBranchAndValue("jet4_ungrtrk", jet4_ungrtrk, dummy_int);
  // m_etree->SetBranchAndValue("jet4_jvf0",
  // double(Props::jvf0.get(jets.at(0))), dummy_double);
  // m_etree->SetBranchAndValue("jet4_jvt", jet4_jvt, dummy_double);
  // m_etree->SetBranchAndValue("jet4_jvtSF", jet4_jvtSF, dummy_double);
  m_etree->SetBranchAndValue("jet4_isBad", jet4_isBad, dummy_int);
  m_etree->SetBranchAndValue("jet4_width", jet4_width, dummy_double);
  m_etree->SetBranchAndValue("jet4_goodJet", jet4_goodJet, dummy_int);
  m_etree->SetBranchAndValue("jet4_passPreSel", jet4_passPreSel, dummy_int);

  vector<float> photon_pt, photon_eta, photon_phi, photon_e, photon_ptcone20,
      photon_etcone20, photon_etcone40, photon_FixedCutTightEffSF,
      photon_FixedCutLooseEffSF, photon_looseEffSF, photon_mediumEffSF,
      photon_tightEffSF;
  vector<int> photon_isTight, photon_isMedium, photon_isLoose,
      photon_isTightIsoCalo, photon_isTightIso, photon_isLooseIso,
      photon_convType, photon_partIndex, photon_truthType, photon_truthOrigin,
      photon_isEM_Tight, photon_isEM_Medium, photon_isEM_Loose,
      photon_isAmbiguous, photon_OQ, photon_Author;

  photon_pt.clear(), photon_eta.clear(), photon_phi.clear(), photon_e.clear(),
      photon_ptcone20.clear(), photon_etcone20.clear(), photon_etcone40.clear(),
      photon_FixedCutTightEffSF.clear(), photon_FixedCutLooseEffSF.clear(),
      photon_isTight.clear(), photon_isMedium.clear(), photon_isLoose.clear(),
      photon_isTightIsoCalo.clear(), photon_isTightIso.clear(),
      photon_isLooseIso.clear(), photon_convType.clear(),
      photon_partIndex.clear(), photon_truthType.clear(),
      photon_truthOrigin.clear(), photon_isEM_Tight.clear(),
      photon_isEM_Medium.clear(), photon_isEM_Loose.clear(),
      photon_isAmbiguous.clear(), photon_OQ.clear(), photon_Author.clear(),
      photon_looseEffSF.clear(), photon_mediumEffSF.clear(),
      photon_tightEffSF.clear();

  for (unsigned int ip = 0; ip < photon.size(); ++ip) {
    photon_pt.push_back(photon.at(ip)->pt());
    photon_eta.push_back(photon.at(ip)->eta());
    photon_phi.push_back(photon.at(ip)->phi());
    photon_e.push_back(photon.at(ip)->e());
    // photon_ptcone20.push_back(photon.at(ip)->isolationValue(xAOD::Iso::ptcone20));
    photon_etcone20.push_back(
        photon.at(ip)->isolationValue(xAOD::Iso::topoetcone20));
    photon_etcone40.push_back(
        photon.at(ip)->isolationValue(xAOD::Iso::topoetcone40));
    photon_isTight.push_back(Props::isTight.get(photon.at(ip)));
    photon_isMedium.push_back(Props::isMedium.get(photon.at(ip)));
    photon_isLoose.push_back(Props::isLoose.get(photon.at(ip)));
    photon_isTightIsoCalo.push_back(
        Props::isFixedCutTightCaloOnlyIso.get(photon.at(ip)));
    // photon_isTightIso.push_back(Props::isFixedCutTightIso.get(photon.at(ip)));
    // photon_isLooseIso.push_back(Props::isFixedCutLooseIso.get(photon.at(ip)));
    photon_convType.push_back(Props::convType.get(photon.at(ip)));
    photon_partIndex.push_back(Props::partIndex.get(photon.at(ip)));
    // if (m_isMC)
    // photon_FixedCutTightEffSF.push_back(Props::photonFixedCutTightEffSF.get(photon.at(ip)));
    // if (m_isMC)
    // photon_FixedCutLooseEffSF.push_back(Props::photonFixedCutLooseEffSF.get(photon.at(ip)));
    if (m_isMC) photon_truthType.push_back(Props::truthType.get(photon.at(ip)));
    if (m_isMC)
      photon_truthOrigin.push_back(Props::truthOrigin.get(photon.at(ip)));
    photon_isEM_Tight.push_back(Props::isEM_Tight.get(photon.at(ip)));
    photon_isEM_Medium.push_back(Props::isEM_Medium.get(photon.at(ip)));
    photon_isEM_Loose.push_back(Props::isEM_Loose.get(photon.at(ip)));
    photon_isAmbiguous.push_back(Props::isAmbiguous.get(photon.at(ip)));
    photon_OQ.push_back(Props::OQ.get(photon.at(ip)));
    photon_Author.push_back(Props::Author.get(photon.at(ip)));
    if (m_isMC) {
      photon_looseEffSF.push_back(Props::looseEffSF.get(photon.at(ip)));
      photon_mediumEffSF.push_back(Props::mediumEffSF.get(photon.at(ip)));
      photon_tightEffSF.push_back(Props::tightEffSF.get(photon.at(ip)));
    }
  }

  n_photons = photon.size();
  m_etree->SetBranchAndValue<int>("n_photons", n_photons, -999);
  m_etree->SetBranchAndValue("photon_pt", photon_pt, dummy_double);
  m_etree->SetBranchAndValue("photon_eta", photon_eta, dummy_double);
  m_etree->SetBranchAndValue("photon_phi", photon_phi, dummy_double);
  m_etree->SetBranchAndValue("photon_e", photon_e, dummy_double);
  // m_etree->SetBranchAndValue("photon_ptcone20", photon_ptcone20,
  // dummy_double);
  m_etree->SetBranchAndValue("photon_etcone20", photon_etcone20, dummy_double);
  m_etree->SetBranchAndValue("photon_etcone40", photon_etcone40, dummy_double);
  m_etree->SetBranchAndValue("photon_isTight", photon_isTight, dummy_int);
  m_etree->SetBranchAndValue("photon_isMedium", photon_isMedium, dummy_int);
  m_etree->SetBranchAndValue("photon_isLoose", photon_isLoose, dummy_int);
  m_etree->SetBranchAndValue("photon_isTightIsoCalo", photon_isTightIsoCalo,
                             dummy_int);
  // m_etree->SetBranchAndValue("photon_isTightIso", photon_isTightIso,
  // dummy_int); m_etree->SetBranchAndValue("photon_isLooseIso",
  // photon_isLooseIso, dummy_int);
  m_etree->SetBranchAndValue("photon_convType", photon_convType, dummy_int);
  m_etree->SetBranchAndValue("photon_partIndex", photon_partIndex, dummy_int);
  m_etree->SetBranchAndValue("photon_FixedCutTightEffSF",
                             photon_FixedCutTightEffSF, dummy_double);
  m_etree->SetBranchAndValue("photon_FixedCutLooseEffSF",
                             photon_FixedCutLooseEffSF, dummy_double);
  m_etree->SetBranchAndValue("photon_truthType", photon_truthType, dummy_int);
  m_etree->SetBranchAndValue("photon_truthOrigin", photon_truthOrigin,
                             dummy_int);
  m_etree->SetBranchAndValue("photon_isEM_Tight", photon_isEM_Tight, dummy_int);
  m_etree->SetBranchAndValue("photon_isEM_Medium", photon_isEM_Medium,
                             dummy_int);
  m_etree->SetBranchAndValue("photon_isEM_Loose", photon_isEM_Loose, dummy_int);
  m_etree->SetBranchAndValue("photon_isAmbiguous", photon_isAmbiguous,
                             dummy_int);
  m_etree->SetBranchAndValue("photon_OQ", photon_OQ, dummy_int);
  m_etree->SetBranchAndValue("photon_Author", photon_Author, dummy_int);
  m_etree->SetBranchAndValue("photon_looseEffSF", photon_looseEffSF,
                             dummy_double);
  m_etree->SetBranchAndValue("photon_mediumEffSF", photon_mediumEffSF,
                             dummy_double);
  m_etree->SetBranchAndValue("photon_tightEffSF", photon_tightEffSF,
                             dummy_double);

  int truthCounter = 0;
  vector<float> truth_px, truth_py, truth_pz, truth_e, truth_m;
  vector<int> truth_barcode, truth_partIndex, truth_pdgId, truth_status;

  truth_px.clear(), truth_py.clear(), truth_pz.clear(), truth_e.clear(),
      truth_m.clear(), truth_barcode.clear(), truth_partIndex.clear(),
      truth_pdgId.clear(), truth_status.clear();

  if (m_isMC) {
    n_truth = m_truthParts->size();
    for (const xAOD::TruthParticle* part : *m_truthParts) {
      truth_px.push_back(part->px());
      truth_py.push_back(part->py());
      truth_pz.push_back(part->pz());
      truth_e.push_back(part->e());
      truth_m.push_back(part->m());
      truth_barcode.push_back(part->barcode());
      truth_partIndex.push_back(Props::partIndex.get(part));
      truth_pdgId.push_back(part->pdgId());
      truth_status.push_back(part->status());
      truthCounter++;
    }
    m_etree->SetBranchAndValue("truth_px", truth_px, dummy_double);
    m_etree->SetBranchAndValue("truth_py", truth_py, dummy_double);
    m_etree->SetBranchAndValue("truth_pz", truth_pz, dummy_double);
    m_etree->SetBranchAndValue("truth_e", truth_e, dummy_double);
    m_etree->SetBranchAndValue("truth_m", truth_m, dummy_double);
    m_etree->SetBranchAndValue("truth_barcode", truth_barcode, dummy_int);
    m_etree->SetBranchAndValue("truth_partIndex", truth_partIndex, dummy_int);
    m_etree->SetBranchAndValue("truth_pdgId", truth_pdgId, dummy_int);
    m_etree->SetBranchAndValue("truth_status", truth_status, dummy_int);
    m_etree->SetBranchAndValue<int>("truth_n", n_truth, -999);
  }

  //*********************************************************//
  //                       EASY TREE                         //
  //*********************************************************//
  // for(unsigned int i=0; i<m_weightSysts.size(); i++){
  // float weightSys = weight * m_weightSysts->at(i).factor;
  // m_nameSvc->set_variation(m_weightSysts->at(i).name);
  // std::cout << m_weightSysts.at(i).name << ": " << m_weightSysts.at(i).factor
  // << std::endl;
  //}

  ////std::cout << "Number of weight systs:" << m_weightSysts.size() <<
  /// std::endl;
  for (unsigned int i = 0; i < m_weightSysts.size(); ++i) {
    // float weightSys = weight * m_weightSysts->at(i).factor;
    // m_nameSvc->set_variation(m_weightSysts->at(i).name);
    m_etree->SetBranchAndValue(m_weightSysts.at(i).name,
                               m_weightSysts.at(i).factor, float(-1.));
  }

  m_etree->Fill();

  //*********************************************************//
  //                  CUT BASED / MVA SELECTION              //
  //*********************************************************//

  bool passCuts = true;
  // cut based analysis -- Based on Song-Mings study
  if (m_doCutBased) {
    if (!passTrigger) passCuts = false;
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
  return EL::StatusCode::SUCCESS;
}
