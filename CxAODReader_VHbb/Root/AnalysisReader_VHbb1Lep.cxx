#include <cfloat>
#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader/OSTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
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

      m_truthLabeling("TrackJetHybrid"),
      m_RecoToTruthLinkName("") {}

AnalysisReader_VHbb1Lep::~AnalysisReader_VHbb1Lep() {}

EL::StatusCode AnalysisReader_VHbb1Lep::run_1Lep_analysis() {
 
  return EL::StatusCode::SUCCESS;

}  // run_1Lep_analysis

//added by wym
EL::StatusCode AnalysisReader_VHbb1Lep::fill_VBF (){

  //EL_CHECK("fill_VBF ()", fill_VBFCutFlow("All"));

  if (m_debug) Info("fill_VBF()", "Beginning of the function...");
  //m_tree_vbf->SetVariation(m_currentVar);
 // m_tree_vbf->Reset();
  ResultVBFHbbIncl selectionResult = ((VBFHbbInclEvtSelection*)m_eventSelection)->result();

  if( !selectionResult.pass ) return EL::StatusCode::SUCCESS;

  if (m_debug) Info("fill_VBF()", "Passed Pre-Selection");
  //EL_CHECK("fill_VBF ()", fill_VBFCutFlow("Pre-Selection"));

  std::vector<const xAOD::Jet*> jets        = selectionResult.jets;
  std::vector<const xAOD::Jet*> forwardJets = selectionResult.forwardJets;
  std::vector<const xAOD::Jet*> trackJets   = selectionResult.trackJets;

  // Jet Assignment Tool
  // -------------
  if (m_debug) Info("fill_VBF()", "entering jet assignment");
  if (! m_jetAssigner->assignJets(jets) ) return EL::StatusCode::SUCCESS; 
  if (m_debug) Info("fill_VBF()", "passed jet assignment");

  //EL_CHECK("fill_VBF ()", fill_VBFCutFlow("Jet-Assignment"));

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
  m_weight *= Props::QGTaggerWeight.get(vbfJets.at(0));
  m_weight *= Props::QGTaggerWeight.get(vbfJets.at(1));

  m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::VBF); // use VBF naming convention
  m_histNameSvc->set_nTag(2); // set histogram name
  m_histNameSvc->set_nJet(4); // more histogram naming

  // ====================
  // fill event variables
  // ====================

  //EL_CHECK("AnalysisReader_VBFHbb::save_meta_info" , save_meta_info(jets, signalJets) );
  //EL_CHECK("AnalysisReader_VBFHbb::save_trigger_info" , save_trigger_info() );

  // ====================
  // fill jet variables
  // ====================

  EL_CHECK("AnalysisReader_VBFHbb::fill_VBF", fill_nJetHistos(signalJets,  "Sig"));
  EL_CHECK("AnalysisReader_VBFHbb::fill_VBF", fill_nJetHistos(forwardJets, "Fwd"));
  EL_CHECK("AnalysisReader_VBFHbb::fill_VBF", fill_jetHistos(jets, signalJets,forwardJets));
  // We save all the information we need, for the events and the jets
  //EL_CHECK("AnalysisReader_VBFHbb::save_jet_info" , save_jet_info(jets,signalJets,vbfJets,trackJets) );

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

  //m_tree_vbf->Fill();// filling for all variations
  //if (m_histNameSvc->get_isNominal()) {
  //  m_eventCountPassed_VBFHbb++;
  //}

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

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode AnalysisReader_VHbb1Lep::save_trigger_info(){

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode AnalysisReader_VHbb1Lep::save_jet_info(
						    std::vector<const xAOD::Jet*>  const& inputJets,
						    std::vector<const xAOD::Jet*>  const& signalJets,
						    std::vector<const xAOD::Jet*>  const& vbfJets,
						    std::vector<const xAOD::Jet*>  const& trackJets) {



  return EL::StatusCode::SUCCESS;

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
