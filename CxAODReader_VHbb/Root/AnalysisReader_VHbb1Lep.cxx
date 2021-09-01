#include <cfloat>
#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader/OSTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"

#include "CxAODReader_VHbb/MVATree_VBFHbb.h"

#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb1lep.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

#include "CxAODTools_VHbb/VBFHbbInclEvtSelection.h"

#include <CxAODReader_VHbb/AnalysisReader_VHbb1Lep.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHbb1Lep::AnalysisReader_VHbb1Lep()
    : AnalysisReader_VHQQ1Lep(),
      m_ApplyFatJetMuOR(false),
      m_BDTSyst_debug(false),
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
      m_writeOSTree(false),
      m_doCoMtagging(false),
      m_forceTrackJetDRmatching(false),
      m_writeObjectsInEasyTree(false),
      m_doDYWFJCut(true),
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
      m_eventCountPassed_VBFHbb(0),

      m_truthLabeling("TrackJetHybrid"),
      m_RecoToTruthLinkName("") {}

AnalysisReader_VHbb1Lep::~AnalysisReader_VHbb1Lep() {}

EL::StatusCode AnalysisReader_VHbb1Lep::run_1Lep_analysis() {
 
  return EL::StatusCode::SUCCESS;

}  // run_1Lep_analysis

//added by wym
EL::StatusCode AnalysisReader_VHbb1Lep::fill_VBFCutFlow (std::string label) {
  std::string dir = "CutFlow/Nominal/";

  static std::string cuts[18] = {
    "All", "Pre-Selection", "Jet-Assignment"
  };

  m_histSvc->BookFillCutHist(dir + "Cuts", length(cuts), cuts, label, m_weight);
  m_histSvc->BookFillCutHist(dir + "CutsNoWeight", length(cuts), cuts, label, 1.);

  return EL::StatusCode::SUCCESS;
} // fill_VBFCutFlow

EL::StatusCode AnalysisReader_VHbb1Lep::fill_VBF (){

  EL_CHECK("fill_VBF ()", fill_VBFCutFlow("All"));

  if (m_debug) Info("fill_VBF()", "Beginning of the function...");
  m_tree_vbf->SetVariation(m_currentVar);
  m_tree_vbf->Reset();
  ResultVBFHbbIncl selectionResult = ((VBFHbbInclEvtSelection*)m_eventSelection)->result();

  if( !selectionResult.pass ) return EL::StatusCode::SUCCESS;

  if (m_debug) Info("fill_VBF()", "Passed Pre-Selection");
  EL_CHECK("fill_VBF ()", fill_VBFCutFlow("Pre-Selection"));

  std::vector<const xAOD::Jet*> jets        = selectionResult.jets;
  std::vector<const xAOD::Jet*> forwardJets = selectionResult.forwardJets;
  std::vector<const xAOD::Jet*> trackJets   = selectionResult.trackJets;

  // Jet Assignment Tool
  // -------------
  if (m_debug) Info("fill_VBF()", "entering jet assignment");
  if (! m_jetAssigner->assignJets(jets) ) return EL::StatusCode::SUCCESS; 
  if (m_debug) Info("fill_VBF()", "passed jet assignment");

  EL_CHECK("fill_VBF ()", fill_VBFCutFlow("Jet-Assignment"));

  std::string assType = m_jetAssigner->assignmentType();
  std::vector<const xAOD::Jet*> signalJets = m_jetAssigner->signalJets();
  std::vector<const xAOD::Jet*> vbfJets    = m_jetAssigner->vbfJets();

  if(!m_doTruthTagging) compute_btagging();


  // ===============
  TLorentzVector b1Vec(signalJets.at(0)->p4()), b2Vec(signalJets.at(1)->p4()),
                 j1Vec(vbfJets.at(0)->p4()), j2Vec(vbfJets.at(1)->p4());
  TLorentzVector HVec(b1Vec+b2Vec);


  // ====================
  // apply weights
  // ====================
  float btagSF = 1.;
  // THIS WILL CHANGE
  /*
  if(m_isMC){
    if (m_JetAssignmentStrategy.find("85") != std::string::npos){
      btagSF = computeBTagSFWeightPerJet (signalJets, m_jetReader->getContainerName(), "85");
    }
    else if (m_JetAssignmentStrategy.find("77") != std::string::npos){
      btagSF = computeBTagSFWeightPerJet (signalJets, m_jetReader->getContainerName(), "77");
    }
    else if (m_JetAssignmentStrategy.find("70") != std::string::npos){
      btagSF = computeBTagSFWeightPerJet (signalJets, m_jetReader->getContainerName(), "70");
    }
    else if (m_JetAssignmentStrategy.find("60") != std::string::npos){
      btagSF = computeBTagSFWeightPerJet (signalJets, m_jetReader->getContainerName(), "60");
    }
    else if (m_JetAssignmentStrategy.find("asym") != std::string::npos){
      btagSF = computeBTagSFWeightPerJet (signalJets, m_jetReader->getContainerName(), "70");
      btagSF *= computeBTagSFWeightPerJet (signalJets, m_jetReader->getContainerName(), "85");
    }
  }
  */

  // b-tagging weights
  m_weight *= btagSF;
  //m_tree_vbf->BJetSF = btagSF;

  // apply qg tagging weights 
  // TO BE REMOVED ? 
  m_weight *= 1;//Props::QGTaggerWeight.get(vbfJets.at(0));
  m_weight *= 1;//Props::QGTaggerWeight.get(vbfJets.at(1));

  m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::VBF); // use VBF naming convention
  m_histNameSvc->set_nTag(2); // set histogram name
  m_histNameSvc->set_nJet(4); // more histogram naming

  // ====================
  // fill event variables
  // ====================

  EL_CHECK("AnalysisReader_VBFHbb::save_meta_info" , save_meta_info(jets, signalJets) );
  //EL_CHECK("AnalysisReader_VBFHbb::save_trigger_info" , save_trigger_info() );

  // ====================
  // fill jet variables
  // ====================

  EL_CHECK("AnalysisReader_VBFHbb::fill_VBF", fill_nJetHistos(signalJets,  "Sig"));
  EL_CHECK("AnalysisReader_VBFHbb::fill_VBF", fill_nJetHistos(forwardJets, "Fwd"));
  EL_CHECK("AnalysisReader_VBFHbb::fill_VBF", fill_jetHistos(jets, signalJets,forwardJets));
  // We save all the information we need, for the events and the jets
  EL_CHECK("AnalysisReader_VBFHbb::save_jet_info" , save_jet_info(jets,signalJets,vbfJets,trackJets) );

  bool passCuts = true;

  // trigger cut
  //if(!m_tree_vbf->passTrig) passCuts = false;

  if(!passCuts) {
    std::printf("Failed kinematic cuts! Skipping this event...\n");
    return EL::StatusCode::SUCCESS;
  }

  // ====================
  // fill output ntup
  // ====================

  m_tree_vbf->Fill();// filling for all variations
  if (m_histNameSvc->get_isNominal()) {
    m_eventCountPassed_VBFHbb++;
  }

  return EL::StatusCode::SUCCESS;
}


void AnalysisReader_VHbb1Lep::DefineVarBDTReweight_1lep(
    std::vector<const xAOD::Jet *> selectedJets, TLorentzVector &j1_corr,
    TLorentzVector &j2_corr, TLorentzVector &j3_corr,
    const TLorentzVector &leptonVec, const TLorentzVector &WvecT,
    const TLorentzVector &metVec, float Mtop, float dYWH) {
  if (selectedJets.size() > 1) {
    m_BDTSyst->m_Variables["mBB"] = (j1_corr + j2_corr).M() * 1e-03;
    m_BDTSyst->m_Variables["dRBB"] = (j1_corr).DeltaR(j2_corr);
    m_BDTSyst->m_Variables["dPhiVBB"] = fabs(WvecT.DeltaPhi(j1_corr + j2_corr));
    m_BDTSyst->m_Variables["pTB1"] = j1_corr.Pt() * 1e-03;
    m_BDTSyst->m_Variables["pTB2"] = j2_corr.Pt() * 1e-03;
    m_BDTSyst->m_Variables["pTV"] = WvecT.Pt() * 1e-03;
    m_BDTSyst->m_Variables["dPhiLBmin"] = std::min(
        fabs(leptonVec.DeltaPhi(j1_corr)), fabs(leptonVec.DeltaPhi(j2_corr)));
    m_BDTSyst->m_Variables["pTL"] = leptonVec.Pt() * 1e-03;
    m_BDTSyst->m_Variables["mTW"] = WvecT.M() * 1e-03;
    m_BDTSyst->m_Variables["mTop"] = Mtop * 1e-03;
    m_BDTSyst->m_Variables["MET"] = metVec.Pt() * 1e-03;
    m_BDTSyst->m_Variables["dYWH"] = dYWH;
  }

  if (selectedJets.size() > 2) {
    m_BDTSyst->m_Variables["pTJ3"] = j3_corr.Pt() * 1e-03;
    m_BDTSyst->m_Variables["mBBJ"] = (j1_corr + j2_corr + j3_corr).M() * 1e-03;
  }
}  // DefineVarBDTReweight_1lep


EL::StatusCode AnalysisReader_VHbb1Lep::save_meta_info( std::vector<const xAOD::Jet*>  const& Jets,
						      std::vector<const xAOD::Jet*>  const& signalJets){

  // ------------------------
  // compute branch variables
  // ------------------------
  // Event Metadata
  // ------------------------
  /// apply the trigger weight scale factor 

  TLorentzVector b1Vec(signalJets.at(0)->p4()), b2Vec(signalJets.at(1)->p4());
  TLorentzVector LeadJetVec(Jets.at(0)->p4());

  // ===========================================================
  std::vector<float> weights_leadBEta = {1,0};
  std::vector<float> weights_jet0 = {1,0};
  std::vector<float> weights_jet1 = {1,0};

  /*
  if (m_isMC){
    if (b1Vec == LeadJetVec or b2Vec == LeadJetVec)
      weights_leadBEta = ReadOfflineBJetTriggerWeight(m_Eff_Event_leadingJet_jetEta, LeadJetVec.Eta(), false);
    
    if (m_selection.find("4central") != std::string::npos) {
	if (m_JetAssignmentStrategy.find("70") != std::string::npos){
 	  weights_jet0 = ReadOfflineBJetTriggerWeight(m_SF_offJets70_match_hlt70_jetPt, b1Vec.Pt());
	  weights_jet1 = ReadOfflineBJetTriggerWeight(m_SF_offJets70_match_hlt70_jetPt, b2Vec.Pt());
	}
	if (m_JetAssignmentStrategy.find("77") != std::string::npos){
	  weights_jet0 = ReadOfflineBJetTriggerWeight(m_SF_offJets77_match_hlt70_jetPt, b1Vec.Pt());
	  weights_jet1 = ReadOfflineBJetTriggerWeight(m_SF_offJets77_match_hlt70_jetPt, b2Vec.Pt());
	}
	if (m_JetAssignmentStrategy.find("85") != std::string::npos){
	  weights_jet0 = ReadOfflineBJetTriggerWeight(m_SF_offJets85_match_hlt70_jetPt, b1Vec.Pt());
	  weights_jet1 = ReadOfflineBJetTriggerWeight(m_SF_offJets85_match_hlt70_jetPt, b2Vec.Pt());
	}
    } else if (m_selection.find("2central") != std::string::npos ) {
      if (m_JetAssignmentStrategy.find("70") != std::string::npos){
	weights_jet0 = ReadOfflineBJetTriggerWeight(m_SF_offJets70_match_hlt70_jetPt, b1Vec.Pt());
	weights_jet1 = ReadOfflineBJetTriggerWeight(m_SF_offJets70_match_hlt85_jetPt, b2Vec.Pt());
      }
      if (m_JetAssignmentStrategy.find("77") != std::string::npos){
	weights_jet0 = ReadOfflineBJetTriggerWeight(m_SF_offJets77_match_hlt70_jetPt, b1Vec.Pt());
	weights_jet1 = ReadOfflineBJetTriggerWeight(m_SF_offJets77_match_hlt85_jetPt, b2Vec.Pt());
      }
      if (m_JetAssignmentStrategy.find("85") != std::string::npos){
	weights_jet0 = ReadOfflineBJetTriggerWeight(m_SF_offJets85_match_hlt70_jetPt, b1Vec.Pt());
	weights_jet1 = ReadOfflineBJetTriggerWeight(m_SF_offJets85_match_hlt85_jetPt, b2Vec.Pt());
      }
      if (m_JetAssignmentStrategy.find("asym") != std::string::npos){
	weights_jet0 = ReadOfflineBJetTriggerWeight(m_SF_offJets70_match_hlt70_jetPt, b1Vec.Pt());
	weights_jet1 = ReadOfflineBJetTriggerWeight(m_SF_offJets85_match_hlt85_jetPt, b2Vec.Pt());
      }
    }

    m_weightSysts.push_back({"BTrig_Eta_EFF__1up" ,    (weights_leadBEta[0]+weights_leadBEta[1])/weights_leadBEta[0]   });
    m_weightSysts.push_back({"BTrig_Eta_EFF__1down" ,  (weights_leadBEta[0]-weights_leadBEta[1])/weights_leadBEta[0]  });
    m_weightSysts.push_back({"BTrig_J1_SF__1up" ,   (weights_jet0[0]+weights_jet0[1])/weights_jet0[0] });
    m_weightSysts.push_back({"BTrig_J1_SF__1down" , (weights_jet0[0]-weights_jet0[1])/weights_jet0[0] });
    m_weightSysts.push_back({"BTrig_J2_SF__1up" ,   (weights_jet1[0]+weights_jet1[1])/weights_jet1[0] });
    m_weightSysts.push_back({"BTrig_J2_SF__1down" , (weights_jet1[0]-weights_jet1[1])/weights_jet1[0] });
  }

  */

  // =============================================================

  m_tree_vbf->RunNumber = m_eventInfo->runNumber();
  m_tree_vbf->LumiBlock = m_eventInfo->lumiBlock();
  m_tree_vbf->EventNumber = m_eventInfo->eventNumber();
  if (m_isMC) {
    m_tree_vbf->EventWeight = m_weight *weights_jet0.at(0) * weights_jet1.at(0) *weights_leadBEta.at(0);
    m_tree_vbf->BJetTriggerWeight = weights_jet0.at(0) * weights_jet1.at(0) *weights_leadBEta.at(0);
  }
  //m_tree_vbf->npv = Props::npv.get(m_eventInfo);

  if (!m_isMC) return StatusCode::SUCCESS;
  
  // THIS WILL CHANGE
  /*
  m_tree_vbf->MCChannelNumber = m_mcChannel;
  m_tree_vbf->MCWeight    = Props::MCEventWeight.get(m_eventInfo);
   
  if (m_mcChannel == 345338 or m_mcChannel == 345342){
    
    m_tree_vbf->alpha_s_up = Props::alpha_s_up.get(m_eventInfo);
    m_tree_vbf->alpha_s_dn = Props::alpha_s_dn.get(m_eventInfo);
    
    for(unsigned int i=0; i< Props::weight_pdf4lhc.get(m_eventInfo).size() ; i++){
      m_tree_vbf->weightspdf4lhc.push_back( Props::weight_pdf4lhc.get(m_eventInfo)[i]);
    }
    for(unsigned int i=0; i< Props::weight_pdfnnpdf30.get(m_eventInfo).size() ; i++){
      m_tree_vbf->weightspdfnnpdf30.push_back( Props::weight_pdfnnpdf30.get(m_eventInfo)[i]);
    }
    for(unsigned int i=0; i< Props::weight_qcd_nnlops.get(m_eventInfo).size() ; i++){
      m_tree_vbf->weightsqcdnnlops.push_back( Props::weight_qcd_nnlops.get(m_eventInfo)[i]);
    }
    for(unsigned int i=0; i< Props::weight_qcd_nnlops_2np.get(m_eventInfo).size() ; i++){
      m_tree_vbf->weightsqcdnnlops2np.push_back( Props::weight_qcd_nnlops_2np.get(m_eventInfo)[i]);
    }
    for(unsigned int i=0; i< Props::weight_qcd_WG1.get(m_eventInfo).size() ; i++){
      m_tree_vbf->weightsqcdwg1.push_back( Props::weight_qcd_WG1.get(m_eventInfo)[i]);
    }
    for(unsigned int i=0; i< Props::weight_alternative_pdf.get(m_eventInfo).size() ; i++){
      m_tree_vbf->weightsalternativepdf.push_back( Props::weight_alternative_pdf.get(m_eventInfo)[i]);
    }
  }
  */
  return EL::StatusCode::SUCCESS;
}


EL::StatusCode AnalysisReader_VHbb1Lep::save_trigger_info(){

  // ------------------------
  // Collecting Trigger Information
  // ------------------------

  // trigger decisions
  //m_tree_vbf->pass_L1_2J25 = Props::pass_L1_2J25.get(m_eventInfo);
  //m_tree_vbf->pass_L1_J25_0ETA23 = Props::pass_L1_J25_0ETA23.get(m_eventInfo);
  //m_tree_vbf->pass_L1_HT150_JJ15_ETA49 = Props::pass_L1_HT150_JJ15_ETA49.get(m_eventInfo);
  m_tree_vbf->pass_L1_2J15_31ETA49 = Props::pass_L1_2J15_31ETA49.get(m_eventInfo);
  //m_tree_vbf->pass_L1_HT150_J20s5_ETA31 = Props::pass_L1_HT150_J20s5_ETA31.get(m_eventInfo);
  m_tree_vbf->pass_L1_MJJ_400 = Props::pass_L1_MJJ_400.get(m_eventInfo);
  m_tree_vbf->pass_L1_J40_0ETA25 = Props::pass_L1_J40_0ETA25.get(m_eventInfo);
  m_tree_vbf->pass_L1_MJJ_400_CF = Props::pass_L1_MJJ_400_CF.get(m_eventInfo);
  m_tree_vbf->pass_L1_J20_31ETA49 = Props::pass_L1_J20_31ETA49.get(m_eventInfo);
  m_tree_vbf->pass_L1_J40_0ETA25_2J25_J20_31ETA49 = Props::pass_L1_J40_0ETA25_2J25_J20_31ETA49.get(m_eventInfo);
  //m_tree_vbf->pass_L1_J25_0ETA23_2J15_31ETA49 = Props::pass_L1_J25_0ETA23_2J15_31ETA49.get(m_eventInfo);
  //m_tree_vbf->pass_L1_HT150_J20s5_ETA31_MJJ_400_CF = Props::pass_L1_HT150_J20s5_ETA31_MJJ_400_CF.get(m_eventInfo);
  m_tree_vbf->pass_L1_J40_0ETA25_2J15_31ETA49 = Props::pass_L1_J40_0ETA25_2J15_31ETA49.get(m_eventInfo);
  //m_tree_vbf->pass_L1_HT150_JJ15_ETA49_MJJ_400 = Props::pass_L1_HT150_JJ15_ETA49_MJJ_400.get(m_eventInfo);
  m_tree_vbf->pass_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490 = Props::pass_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490.get(m_eventInfo);
  m_tree_vbf->pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = Props::pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49.get(m_eventInfo);
  //m_tree_vbf->pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split = Props::pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split.get(m_eventInfo);
  m_tree_vbf->pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split = Props::pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split.get(m_eventInfo);
  m_tree_vbf->pass_HLT_j80_0eta240_j60_j45_320eta490 = Props::pass_HLT_j80_0eta240_j60_j45_320eta490.get(m_eventInfo);
  m_tree_vbf->pass_HLT_j80_0eta240_2j60_320eta490 = Props::pass_HLT_j80_0eta240_2j60_320eta490.get(m_eventInfo);
  m_tree_vbf->pass_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = Props::pass_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49.get(m_eventInfo);

  // Trigger is Active
  m_tree_vbf->isActive_L1_2J25 = Props::isActive_L1_2J25.get(m_eventInfo);
  m_tree_vbf->isActive_L1_J25_0ETA23 = Props::isActive_L1_J25_0ETA23.get(m_eventInfo);
  m_tree_vbf->isActive_L1_HT150_JJ15_ETA49 = Props::isActive_L1_HT150_JJ15_ETA49.get(m_eventInfo);
  m_tree_vbf->isActive_L1_2J15_31ETA49 = Props::isActive_L1_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->isActive_L1_HT150_J20s5_ETA31 = Props::isActive_L1_HT150_J20s5_ETA31.get(m_eventInfo);
  m_tree_vbf->isActive_L1_MJJ_400 = Props::isActive_L1_MJJ_400.get(m_eventInfo);
  m_tree_vbf->isActive_L1_J40_0ETA25 = Props::isActive_L1_J40_0ETA25.get(m_eventInfo);
  m_tree_vbf->isActive_L1_MJJ_400_CF = Props::isActive_L1_MJJ_400_CF.get(m_eventInfo);
  m_tree_vbf->isActive_L1_J20_31ETA49 = Props::isActive_L1_J20_31ETA49.get(m_eventInfo);
  m_tree_vbf->isActive_L1_J40_0ETA25_2J25_J20_31ETA49 = Props::isActive_L1_J40_0ETA25_2J25_J20_31ETA49.get(m_eventInfo);
  m_tree_vbf->isActive_L1_J25_0ETA23_2J15_31ETA49 = Props::isActive_L1_J25_0ETA23_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->isActive_L1_HT150_J20s5_ETA31_MJJ_400_CF = Props::isActive_L1_HT150_J20s5_ETA31_MJJ_400_CF.get(m_eventInfo);
  m_tree_vbf->isActive_L1_J40_0ETA25_2J15_31ETA49 = Props::isActive_L1_J40_0ETA25_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->isActive_L1_HT150_JJ15_ETA49_MJJ_400 = Props::isActive_L1_HT150_JJ15_ETA49_MJJ_400.get(m_eventInfo);
  m_tree_vbf->isActive_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490 = Props::isActive_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490.get(m_eventInfo);
  m_tree_vbf->isActive_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = Props::isActive_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->isActive_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split = Props::isActive_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split.get(m_eventInfo);
  m_tree_vbf->isActive_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split = Props::isActive_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split.get(m_eventInfo);
  m_tree_vbf->isActive_HLT_j80_0eta240_j60_j45_320eta490 = Props::isActive_HLT_j80_0eta240_j60_j45_320eta490.get(m_eventInfo);
  m_tree_vbf->isActive_HLT_j80_0eta240_2j60_320eta490 = Props::isActive_HLT_j80_0eta240_2j60_320eta490.get(m_eventInfo);
  m_tree_vbf->isActive_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = Props::isActive_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49.get(m_eventInfo);

  // Trigger Pre-Scales
  m_tree_vbf->prescale_L1_2J25 = Props::prescale_L1_2J25.get(m_eventInfo);
  m_tree_vbf->prescale_L1_J25_0ETA23 = Props::prescale_L1_J25_0ETA23.get(m_eventInfo);
  m_tree_vbf->prescale_L1_HT150_JJ15_ETA49 = Props::prescale_L1_HT150_JJ15_ETA49.get(m_eventInfo);
  m_tree_vbf->prescale_L1_2J15_31ETA49 = Props::prescale_L1_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->prescale_L1_HT150_J20s5_ETA31 = Props::prescale_L1_HT150_J20s5_ETA31.get(m_eventInfo);
  m_tree_vbf->prescale_L1_MJJ_400 = Props::prescale_L1_MJJ_400.get(m_eventInfo);
  m_tree_vbf->prescale_L1_J40_0ETA25 = Props::prescale_L1_J40_0ETA25.get(m_eventInfo);
  m_tree_vbf->prescale_L1_MJJ_400_CF = Props::prescale_L1_MJJ_400_CF.get(m_eventInfo);
  m_tree_vbf->prescale_L1_J20_31ETA49 = Props::prescale_L1_J20_31ETA49.get(m_eventInfo);
  m_tree_vbf->prescale_L1_J40_0ETA25_2J25_J20_31ETA49 = Props::prescale_L1_J40_0ETA25_2J25_J20_31ETA49.get(m_eventInfo);
  m_tree_vbf->prescale_L1_J25_0ETA23_2J15_31ETA49 = Props::prescale_L1_J25_0ETA23_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->prescale_L1_HT150_J20s5_ETA31_MJJ_400_CF = Props::prescale_L1_HT150_J20s5_ETA31_MJJ_400_CF.get(m_eventInfo);
  m_tree_vbf->prescale_L1_J40_0ETA25_2J15_31ETA49 = Props::prescale_L1_J40_0ETA25_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->prescale_L1_HT150_JJ15_ETA49_MJJ_400 = Props::prescale_L1_HT150_JJ15_ETA49_MJJ_400.get(m_eventInfo);
  m_tree_vbf->prescale_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490 = Props::prescale_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490.get(m_eventInfo);
  m_tree_vbf->prescale_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = Props::prescale_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49.get(m_eventInfo);
  m_tree_vbf->prescale_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split = Props::prescale_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split.get(m_eventInfo);
  m_tree_vbf->prescale_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split = Props::prescale_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split.get(m_eventInfo);
  m_tree_vbf->prescale_HLT_j80_0eta240_j60_j45_320eta490 = Props::prescale_HLT_j80_0eta240_j60_j45_320eta490.get(m_eventInfo);
  m_tree_vbf->prescale_HLT_j80_0eta240_2j60_320eta490 = Props::prescale_HLT_j80_0eta240_2j60_320eta490.get(m_eventInfo);
  m_tree_vbf->prescale_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = Props::prescale_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49.get(m_eventInfo);

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode AnalysisReader_VHbb1Lep::save_jet_info(
						    std::vector<const xAOD::Jet*>  const& inputJets,
						    std::vector<const xAOD::Jet*>  const& signalJets,
						    std::vector<const xAOD::Jet*>  const& vbfJets,
						    std::vector<const xAOD::Jet*>  const& trackJets) {


  m_tree_vbf->nJets = inputJets.size();

  TLorentzVector b1Vec(signalJets.at(0)->p4());
  TLorentzVector b2Vec(signalJets.at(1)->p4());
  TLorentzVector j1Vec(vbfJets.at(0)->p4())   ;
  TLorentzVector j2Vec(vbfJets.at(1)->p4())   ;
  TLorentzVector b1Vec_no_corr(signalJets.at(0)->p4());
  TLorentzVector b2Vec_no_corr(signalJets.at(1)->p4());

  // apply b-jet energy correction to signal jets
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/HSG5BJetEnergyCorr#Context  
  if (m_jetCorrType != "NoCorr"){
    EL_CHECK("AnalysisReader_VBFHbb::Fill() ",getBJetEnergyCorrTLV(signalJets.at(0), b1Vec, false, m_jetCorrType));
    EL_CHECK("AnalysisReader_VBFHbb::Fill() ",getBJetEnergyCorrTLV(signalJets.at(1), b2Vec, false, m_jetCorrType));
  }

  // ------------------------
  // Collecting Jets' Info
  // ------------------------
  m_tree_vbf->nJets20pt = 0;
  m_tree_vbf->nJets30pt = 0;
  m_tree_vbf->nJets40pt = 0;
  m_tree_vbf->nJets50pt = 0;
  m_tree_vbf->nJets60pt = 0;

  for (unsigned int iJet(0); iJet < inputJets.size(); iJet++) {
    if (inputJets.at(iJet)->pt() * 0.001 > 20) m_tree_vbf->nJets20pt++;
    if (inputJets.at(iJet)->pt() * 0.001 > 30) m_tree_vbf->nJets30pt++;
    if (inputJets.at(iJet)->pt() * 0.001 > 40) m_tree_vbf->nJets40pt++;
    if (inputJets.at(iJet)->pt() * 0.001 > 50) m_tree_vbf->nJets50pt++;
    if (inputJets.at(iJet)->pt() * 0.001 > 60) m_tree_vbf->nJets60pt++;

    m_tree_vbf->pT.push_back ( inputJets.at(iJet)->pt()  );
    m_tree_vbf->eta.push_back( inputJets.at(iJet)->eta() );
    m_tree_vbf->phi.push_back( inputJets.at(iJet)->phi() );
  }

  for (unsigned int iJet(0) ; iJet < inputJets.size(); ++iJet) {
    const xAOD::Jet * jet = inputJets.at(iJet);
    m_tree_vbf->mv2c10.push_back( Props::MV2c10.get(jet) );
    //    m_tree_vbf->jetWidth.push_back( Props::Width.get(jet) );
    int partonTruthLabelID = -999;
    if (m_isMC) partonTruthLabelID = Props::PartonTruthLabelID.get(jet);
    m_tree_vbf->partonTruthLabelID.push_back(partonTruthLabelID);
  }

  
  //m_tree_vbf->J1FJVTLoose = Props::PassFJvtLoose.get( vbfJets.at(0) );
  //m_tree_vbf->J2FJVTLoose = Props::PassFJvtLoose.get( vbfJets.at(1) );
  //m_tree_vbf->J1FJVTTight = Props::PassFJvtTight.get( vbfJets.at(0) );
  //m_tree_vbf->J2FJVTTight = Props::PassFJvtTight.get( vbfJets.at(1) );


  // === Get Index and mindR of J1, J2, B1 and B2
  double mindRB1 = 9999;
  double mindRB2 = 9999;
  double mindRJ1 = 9999;
  double mindRJ2 = 9999;

  double mindRJ1_Ex = 9999;
  double mindRJ2_Ex = 9999;

  double dRB1J = 9999;
  double dRB2J = 9999;
  double dRJ1J = 9999;
  double dRJ2J = 9999;

  for (unsigned int iJet(0); iJet < inputJets.size(); iJet++) {

    TLorentzVector tmpVect( inputJets.at(iJet)->p4() );
    dRJ1J = tmpVect.DeltaR(j1Vec);
    dRJ2J = tmpVect.DeltaR(j2Vec);
    dRB1J = tmpVect.DeltaR(b1Vec_no_corr);
    dRB2J = tmpVect.DeltaR(b2Vec_no_corr);

    bool eventJet = false;
    if (tmpVect == j1Vec or tmpVect == j2Vec or tmpVect == b1Vec_no_corr or tmpVect == b2Vec_no_corr){
      eventJet = true;
    }

    if (tmpVect == j1Vec) { m_tree_vbf->whoIsJ1 = iJet;}
    else { 
      if(dRJ1J<mindRJ1) mindRJ1 = dRJ1J; 
      if(not eventJet and dRJ1J<mindRJ1_Ex) mindRJ1_Ex = dRJ1J; 
    }

    if (tmpVect == j2Vec) { m_tree_vbf->whoIsJ2 = iJet;}
    else { 
      if(dRJ2J<mindRJ2) mindRJ2 = dRJ2J; 
      if(not eventJet and dRJ2J<mindRJ2_Ex) mindRJ2_Ex = dRJ2J; 
     }

    if (tmpVect == b1Vec_no_corr) { m_tree_vbf->whoIsB1 = iJet;}
    else { if(dRB1J<mindRB1) mindRB1 = dRB1J; }

    if (tmpVect == b2Vec_no_corr) { m_tree_vbf->whoIsB2 = iJet;}
    else { if(dRB2J<mindRB2) mindRB2 = dRB2J; }
  }

  // ============================================== 
  m_tree_vbf->mBB    =  (b1Vec + b2Vec).M();
  m_tree_vbf->dRBB   =  b1Vec.DeltaR(b2Vec);
  m_tree_vbf->dPhiBB =  fabs(b1Vec.DeltaPhi(b2Vec));
  m_tree_vbf->dEtaBB =  fabs(b1Vec.Eta() - b2Vec.Eta());
  m_tree_vbf->pTBB   =  (b1Vec + b2Vec).Pt();

  m_tree_vbf->mBB_no_corr    =  (b1Vec_no_corr + b2Vec_no_corr).M();
  m_tree_vbf->dRBB_no_corr   =  b1Vec_no_corr.DeltaR(b2Vec_no_corr);
  m_tree_vbf->pTBB_no_corr   =  (b1Vec_no_corr + b2Vec_no_corr).Pt();

  m_tree_vbf->mJJ     =  (j1Vec + j2Vec).M();
  m_tree_vbf->mJ1B1   =  (j1Vec + b1Vec).M();
  m_tree_vbf->mJ1B2   =  (j1Vec + b2Vec).M();

  m_tree_vbf->dRJJ   =  j1Vec.DeltaR(j2Vec);
  m_tree_vbf->dPhiJJ =  fabs(j1Vec.DeltaPhi(j2Vec));
  m_tree_vbf->dEtaJJ =  fabs(j1Vec.Eta() - j2Vec.Eta());
  m_tree_vbf->pTJJ   =  (j1Vec + j2Vec).Pt();

  m_tree_vbf->mindRB1    =  mindRB1;
  m_tree_vbf->mindRB2    =  mindRB2;
  m_tree_vbf->mindRJ1    =  mindRJ1;
  m_tree_vbf->mindRJ2    =  mindRJ2;
  m_tree_vbf->mindRJ1_Ex =  mindRJ1_Ex;
  m_tree_vbf->mindRJ2_Ex =  mindRJ2_Ex;

  // ============================================== 
  m_tree_vbf->pTJ1 = j1Vec.Pt();
  m_tree_vbf->pTJ2 = j2Vec.Pt();
  m_tree_vbf->pTB1 = b1Vec.Pt();
  m_tree_vbf->pTB2 = b2Vec.Pt();
  m_tree_vbf->pTB1_no_corr = b1Vec_no_corr.Pt();
  m_tree_vbf->pTB2_no_corr = b2Vec_no_corr.Pt();

  m_tree_vbf->etaJ1 = j1Vec.Eta();
  m_tree_vbf->etaJ2 = j2Vec.Eta();
  m_tree_vbf->etaB1_no_corr = b1Vec.Eta();
  m_tree_vbf->etaB2_no_corr = b2Vec.Eta();
  m_tree_vbf->etaB1_no_corr = b1Vec_no_corr.Eta();
  m_tree_vbf->etaB2_no_corr = b2Vec_no_corr.Eta();

  m_tree_vbf->dRB1J1 = b1Vec.DeltaR(j1Vec);
  m_tree_vbf->dRB1J2 = b1Vec.DeltaR(j2Vec);
  m_tree_vbf->dRB2J1 = b2Vec.DeltaR(j1Vec);
  m_tree_vbf->dRB2J2 = b2Vec.DeltaR(j2Vec);

  if (m_isMC) {
    //m_tree_vbf->TruthLabelB1 = ( Props::TruthLabelID.get( signalJets.at(0) ));
    //m_tree_vbf->TruthLabelB2 = ( Props::TruthLabelID.get( signalJets.at(1) ));
    m_tree_vbf->TruthLabelPartonB1 = ( Props::PartonTruthLabelID.get( signalJets.at(0) ));
    m_tree_vbf->TruthLabelPartonB2 = ( Props::PartonTruthLabelID.get( signalJets.at(1) ));
    m_tree_vbf->HadronConeExclTruthLabelB1 = ( Props::HadronConeExclTruthLabelID.get(signalJets.at(0) ));
    m_tree_vbf->HadronConeExclTruthLabelB2 = ( Props::HadronConeExclTruthLabelID.get(signalJets.at(1) ));
    
  }

  m_tree_vbf->MV2c10B1 = Props::MV2c10.get(signalJets.at(0));
  m_tree_vbf->MV2c10B2 = Props::MV2c10.get(signalJets.at(1));
  m_tree_vbf->MV2c10J1 = Props::MV2c10.get(vbfJets.at(0));
  m_tree_vbf->MV2c10J2 = Props::MV2c10.get(vbfJets.at(1));

  //  m_tree_vbf->WidthJ1 = Props::Width.get(vbfJets.at(0));
  //  m_tree_vbf->WidthJ2 = Props::Width.get(vbfJets.at(1));

  m_tree_vbf-> NTrk1000PVJ1 = Props::NumTrkPt1000PV.get(vbfJets.at(0));
  m_tree_vbf-> NTrk1000PVJ2 = Props::NumTrkPt1000PV.get(vbfJets.at(1));

  m_tree_vbf-> NTrk500PVJ1 = Props::NumTrkPt500PV.get(vbfJets.at(0));
  m_tree_vbf-> NTrk500PVJ2 = Props::NumTrkPt500PV.get(vbfJets.at(1));

  //m_tree_vbf-> QGTaggerJ1 = Props::QGTagger.get(vbfJets.at(0));
  //m_tree_vbf-> QGTaggerJ2 = Props::QGTagger.get(vbfJets.at(1));

  //m_tree_vbf-> QGTaggerWeightJ1 = (vbfJets.at(0))->auxdata<float>("qgTaggerWeight");//Props::QGTaggerWeight.get(vbfJets.at(0));
  //m_tree_vbf-> QGTaggerWeightJ2 = (vbfJets.at(1))->auxdata<float>("qgTaggerWeight");//Props::QGTaggerWeight.get(vbfJets.at(1));


  // ===== pT_ballance
  TLorentzVector sumSelectedJets = j1Vec + j2Vec + b1Vec + b2Vec;
  double sumPt = j1Vec.Pt() + j2Vec.Pt() + b1Vec.Pt() + b2Vec.Pt();
  m_tree_vbf->pT_ballance = sumSelectedJets.Pt() / sumPt;

  // ======
  m_tree_vbf->max_J1J2 = std::max( fabs(j1Vec.Eta()), fabs(j2Vec.Eta()) );
  m_tree_vbf->eta_J_star = ( fabs(j1Vec.Eta()) + fabs(j1Vec.Eta()) )/2 - ( fabs(b1Vec.Eta()) + fabs(b2Vec.Eta()) )/2;

  // Computation of HT_soft (taken from VBF+gamma) -- trunk version
  m_tree_vbf->minEta = (j1Vec.Eta() < j2Vec.Eta())? j1Vec.Eta() : j2Vec.Eta();
  m_tree_vbf->maxEta = (j1Vec.Eta() > j2Vec.Eta())? j1Vec.Eta() : j2Vec.Eta();

  m_tree_vbf->HT_soft = 0;
  std::pair<std::pair<float, float>, float> El(getCenterSlopeOfEllipse(signalJets.at(0), signalJets.at(1)));

  for (int iJet(0); iJet < (int)trackJets.size(); iJet++) {
    if ( trackJetIsInEllipse(El, trackJets.at(iJet), m_tree_vbf->dRBB, 0.4) ) continue;
    if (trackJets.at(iJet)->p4().DeltaR(vbfJets.at(0)->p4()) < 0.4) continue;
    if (trackJets.at(iJet)->p4().DeltaR(vbfJets.at(1)->p4()) < 0.4) continue;

    if (trackJets.at(iJet)->pt() * 0.001 < 2) continue;
    m_tree_vbf->HT_soft += trackJets.at(iJet)->pt();
  }


  m_tree_vbf->HT_MVA  = 0;
  for (int iJet(0); iJet < (int)inputJets.size(); iJet++) {
    // Check if the selected jet has not been selected as signal or vbf
    if (iJet == m_tree_vbf->whoIsJ1) continue;
    if (iJet == m_tree_vbf->whoIsJ2) continue;
    if (iJet == m_tree_vbf->whoIsB1) continue;
    if (iJet == m_tree_vbf->whoIsB2) continue;
    if (inputJets.at(iJet)->pt() * 0.001 < 20) continue;

    // Remove the region between the two b-jets
    double etaJet = inputJets.at(iJet)->eta();
    double etaJetBMin = inputJets.at(m_tree_vbf->whoIsB1)->eta() < inputJets.at(m_tree_vbf->whoIsB2)->eta() ? inputJets.at(m_tree_vbf->whoIsB1)->eta() : inputJets.at(m_tree_vbf->whoIsB2)->eta() ;
    double etaJetBMax = inputJets.at(m_tree_vbf->whoIsB1)->eta() < inputJets.at(m_tree_vbf->whoIsB2)->eta() ? inputJets.at(m_tree_vbf->whoIsB2)->eta() : inputJets.at(m_tree_vbf->whoIsB1)->eta() ;
    if ( etaJet > etaJetBMin && etaJet < etaJetBMax) continue;

    m_tree_vbf->HT_MVA += inputJets.at(iJet)->pt();
  }


  TLorentzVector Higgs = b1Vec + b2Vec;
  TVector3 boostingVector = Higgs.BoostVector();
  TVector3 CrossProduct = j1Vec.Vect().Cross(j2Vec.Vect());
  TLorentzVector CrossBoosted(CrossProduct,sqrt(CrossProduct*CrossProduct));
  CrossBoosted.Boost(boostingVector);
  m_tree_vbf->cosTheta_MVA = CrossBoosted.CosTheta();


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

  m_tree_vbf->cosTheta_CMS = cosTheta_CMS;

  TLorentzVector j1VecH = j1Vec;
  TLorentzVector j2VecH = j2Vec;
  TLorentzVector HVec(b1Vec+b2Vec);
  TVector3 boostVecH = -(HVec.BoostVector());
  j1VecH.Boost(boostVecH);
  j2VecH.Boost(boostVecH);
  TVector3 j1VecH3 = j1VecH.Vect();
  TVector3 j2VecH3 = j2VecH.Vect();
  TVector3 j1xj2p = j1VecH3.Cross(j2VecH3);
  TVector3 HVec3 = HVec.Vect();
  float cosTheta_boost = j1xj2p.Dot(HVec3)/ (j1xj2p.Mag()*HVec3.Mag());

  m_tree_vbf->cosTheta_boost = cosTheta_boost;

  // after filling the MVA tree, start to evaluate MVA
  m_tree_vbf->ReadMVA();



  return EL::StatusCode::SUCCESS;

}

std::pair<std::pair<float, float>, float>
AnalysisReader_VHbb1Lep::getCenterSlopeOfEllipse(const xAOD::Jet* jet1, const xAOD::Jet* jet2)
{
  float eta0;
  float eta1;
  float eta2;
  float phi0;
  float phi1;
  float phi2;
  float m;

  if(jet1->phi() > jet2->phi()){
    phi1 = jet1->phi();
    phi2 = jet2->phi();
    eta1 = jet1->eta();
    eta2 = jet2->eta();
  }
  else{
    phi1 = jet2->phi();
    phi2 = jet1->phi();
    eta1 = jet2->eta();
    eta2 = jet1->eta();
  }

  eta0 = (eta1 + eta2)/2;
  float dphi = phi1 - phi2;
  if(dphi > TMath::Pi()) phi2 = phi2 + 2.*TMath::Pi();

  phi0 = (phi1 + phi2)/2;
  m = (phi1 - phi0)/(eta1 - eta0);
  if (phi0 > TMath::Pi()) phi0 = phi0 - 2.*TMath::Pi();

  std::pair<float, float> C(eta0, phi0);
  std::pair<std::pair<float, float>, float> El(C, m);

  return El;
}


bool AnalysisReader_VHbb1Lep::trackJetIsInEllipse(std::pair<std::pair<float, float>, float> El,
						const xAOD::Jet* jet, float dRBB, float r){

  float eta0 = El.first.first;
  float phi0 = El.first.second;
  float m = El.second;
  float m_p = -1./m;

  float etaj = jet->eta();
  float phij = jet->phi();

  if ( fabs(phi0 - phij) > TMath::Pi() ){
    phij = (phi0 > phij)? phij + 2*TMath::Pi() : phij - 2*TMath::Pi();
  }

  float eta_I1 = ( 1. / (m - m_p) ) * ( m*eta0 - m_p*etaj - phi0 + phij);
  float phi_I1 = m * (eta_I1 - eta0) + phi0;

  float eta_I2 = ( 1. / (m_p - m) ) * ( m_p*eta0 - m*etaj - phi0 + phij);
  float phi_I2 = m_p * (eta_I2 - eta0) + phi0;


  // not sure below here!!!
  float dEta1 = eta_I1 - etaj;
  float dPhi1 = phi_I1 - phij;

  float dEta2 = eta_I2 - etaj;
  float dPhi2 = phi_I2 - phij;

  float d1sq = dPhi1*dPhi1 + dEta1*dEta1;
  float d2sq = dPhi2*dPhi2 + dEta2*dEta2;

  float b = r;
  float a = 0.5 * dRBB + r;

  bool inEllipse = ( (d1sq / (b*b)) + (d2sq / (a*a)) ) < 1;

  return inEllipse;
}

EL::StatusCode AnalysisReader_VHbb1Lep::initializeSelection() {
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
  m_config->getif<bool>("writeOSTree", m_writeOSTree);
  m_config->getif<bool>("doMergeCR", m_doMergeCR);
  m_config->getif<bool>("doCutflow", m_doCutflow);
  m_config->getif<bool>("nominalOnly", m_nominalOnly);
  m_config->getif<bool>("doCoMtagging", m_doCoMtagging);
  m_config->getif<bool>("forceTrackJetDRmatching", m_forceTrackJetDRmatching);
  m_config->getif<bool>("doRemoveDilepOverlap", m_doRemoveDilepOverlap);
  m_config->getif<bool>("useTTBarPTWFiltered1LepSamples",
                        m_useTTBarPTWFiltered1LepSamples);
  m_config->getif<bool>("applyPUWeight", m_applyPUWeight);
  m_config->getif<bool>("writeObjectsInEasyTree", m_writeObjectsInEasyTree);
  m_config->getif<bool>("GenerateSTXS", m_GenerateSTXS);
  m_config->getif<std::string>("RecoToTruthLinkName", m_RecoToTruthLinkName);
  m_config->getif<bool>("doDYWFJCut", m_doDYWFJCut);

  m_config->getif<std::vector<std::string>>("BDTSystVariations",
                                            m_BDTSystVariations);

  if (m_debug) {
    Info("initalizeSelection()", "End of function initializeSelection()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHbb1Lep::finalize ()
{
  AnalysisReader::finalize();
  Info("finalize()", "Passed nominal VBFHbb    = %li", m_eventCountPassed_VBFHbb);
  return EL::StatusCode::SUCCESS;
} // finalize