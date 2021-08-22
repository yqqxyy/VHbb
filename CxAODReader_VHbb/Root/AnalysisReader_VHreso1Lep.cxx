#include <cfloat>
#include <type_traits>

#include "CxAODReader/EasyTree.h"
#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools_VHbb/TriggerTool_VHbb1lep.h"
#include "CxAODTools_VHbb/VHbb1lepEvtSelection.h"

#include <CxAODReader_VHbb/AnalysisReader_VHreso1Lep.h>
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

#define length(array) (sizeof(array) / sizeof(*(array)))

AnalysisReader_VHreso1Lep::EventVariables::EventVariables()
    : el(nullptr),
      mu(nullptr),
      met(nullptr),
      signalJets(),
      forwardJets(),
      fatJets(),
      trackJets(),
      selectedJets(),
      taus(),
      lepton(),
      triggerSF(1.),
      isTriggered(false),
      inmBBresolvedWindow(false),
      inmBBmergedWindow(false),
      inmBBresolvedrejWindow(false),
      inmBBmergedrejWindow(false),
      nLooseEl(0),
      nSigEl(0),
      nLooseMu(0),
      nSigMu(0),
      matchedLeadTrackJets(),
      unmatchedTrackJets(),
      trackJetsForLabelling(),
      trackJetsForBTagging(),
      selectedEventJets(),
      selectedEventFatJets(),
      metSig(),
      metSig_PU(),
      metSig_soft(),
      metSig_hard(),
      metOverSqrtSumET(),
      metOverSqrtHT(),
      resolvedMET(),
      mergedMET(),
      metVec(),
      mptVec(),
      resolvedH(),
      mergedH(),
      resolvedVH(),
      mergedVH(),
      HVec(),
      VHVec(),
      bbjVec(),
      bbjVec_Corr(),
      WVec(),
      isResolved(),
      isMerged(),
      leptonSF(1.),
      btagWeight(1.),
      Mtop(0.),
      sumpt(0.),
      dPhiMETMPT(0.),  // TODO remove?
      dPhiMETdijetResolved(0.),
      eventFlag(0),
      cuts_common_resolved(),
      cuts_SR_resolved(),
      cuts_jet_hists_resolved(),  // TODO remove?
      cuts_common_merged(),
      cuts_SR_merged()

{}

AnalysisReader_VHreso1Lep::AnalysisReader_VHreso1Lep()
    : AnalysisReader_VHQQ1Lep(),
      m_doBJetEnergyCorr(
          true),  // These needs to be revised, most are probably not necessary
      m_doMbbRescaling(true),
      m_doIsoInv(false),
      m_doBlindingData(true),
      m_doBlindingMC(false),
      m_applyFatJetMuOR(false),
      m_mVHvsPtvCut(false),
      m_doBasicPlots(true),  // A lot of plots ...
      m_doInputPlots(false),
      m_doMJEtree(false),
      m_doExtendedPlots(false),
      m_doTLVPlots(false),
      m_doMbbRejHist(false),
      m_reduceHistograms(false),
      m_writeMVATree(false),
      m_writeEasyTree(false),
      m_printEventWeightsForCutFlow(true),
      m_doCutflow(false),
      m_doMergeCR(false),
      m_nominalOnly(true),
      m_useTTBarPTWFiltered1LepSamples(false),
      m_applyPUWeight(false),
      m_writeObjectsInEasyTree(false),
      m_vars() {}

AnalysisReader_VHreso1Lep::~AnalysisReader_VHreso1Lep() {}

bool AnalysisReader_VHreso1Lep::ttbarDiLeptonVeto() {
  // remove overlap between dilep and inclusive / non-all had ttbar and single
  // top Wt samples
  //----------------------------------------------------------------------------------------
  // do overlap removal for 410472 ttbar_dilep_PwPy8
  if (m_debug) {
    Info("ttbarDiLeptonVeto()", "Beginning of function ttbarDiLeptonVeto()");
  }

  if (m_mcChannel == 410470) {  // 410470: PP8 non-all-had ttbar
    int veto = vetoDilepTtbarEvents();
    if (veto == -1) {
      Error("ttbarDiLeptonVeto()", "Props::codeTTBarDecay doesn't exist!");
      return EL::StatusCode::FAILURE;
    } else if (veto == 1) {
      Info("ttbarDiLeptonVeto()", "Found dileptons in ttbar, veto the event.");
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }

  // single top Wt apply to ttbar for the moment in 1L
  /*
  if ( m_mcChannel == 410646 ||  m_mcChannel == 410647 ){ // 410646(7): PP8
  inclusive single (anti-)top Wt int veto = vetoDilepWtEvents();
    // Event has to be vetoed
    if ( veto == 1 )  return EL::StatusCode::SUCCESS;
  }
  */
  //  }
}

EL::StatusCode AnalysisReader_VHreso1Lep::mergepTWFilteredttbar() {
  // Merge pTW filter ttbar samples with the nominal one

  if (m_debug) {
    Info("mergepTWFilteredttbar()",
         "Beginning of function mergepTWFilteredttbar()");
  }

  double pTW1 = -99;
  int PDGIDW1 = -99;
  double pTW2 = -99;
  double pTL = -99;
  double EtaL = -99;
  int PDGIDL = -99;
  int nW = 0;
  int nTau = 0;
  double pTW = -99;
  int index = 0;
  double DRMin = 10e5;
  if (m_useTTBarPTWFiltered1LepSamples) {
    // 410470: PP8 non-all-had ttbar ; 345951 : lepWpT100_200; 346031 :
    // lepWpT200;
    if (m_mcChannel == 410470) {
      if (!Props::codeTTBarDecay.exists(m_eventInfo)) {
        Error("mergepTWFilteredttbar()",
              "Props::codeTTBarDecay doesn't exist!");
        return EL::StatusCode::FAILURE;
      }
      // First W-->tau nu decay. Add weight only for events with tau undergo
      // leptonic decay
      if (Props::codeTTBarDecay.get(m_eventInfo) == 3) {
        for (auto *part : *m_truthParts) {
          if (part->isW() == true) {
            nW += 1;
            if (nW == 1) {
              pTW1 = part->pt();
              PDGIDW1 = part->pdgId();
            }
            if (nW == 2) {
              pTW2 = part->pt();
            }
            if (nW > 2) {
              Error("mergepTWFilteredttbar()", "Should have only 2W");
              return EL::StatusCode::FAILURE;
            }
          } else if (part->isTau()) {
            nTau += 1;
            if (nTau == 1) PDGIDL = part->pdgId();
          } else if (part->isElectron() || part->isMuon()) {
            TLorentzVector truthL;
            truthL.SetPtEtaPhiM(part->pt(), part->eta(), part->phi(),
                                part->m());
            if (m_vars.lepton.vec.DeltaR(truthL) < DRMin) {
              DRMin = m_vars.lepton.vec.DeltaR(truthL);
              index += 1;
              pTL = part->pt();
              EtaL = part->eta();
            }
          }
        }
      } else if (Props::codeTTBarDecay.get(m_eventInfo) == 1 ||
                 Props::codeTTBarDecay.get(m_eventInfo) ==
                     2) {                   // only e+jet or mu+jet
        for (auto *part : *m_truthParts) {  // so should have 2 W and 1 lepton
          if (part->isW() == true) {
            nW += 1;
            if (nW == 1) {
              pTW1 = part->pt();
              PDGIDW1 = part->pdgId();
            }
            if (nW == 2) {
              pTW2 = part->pt();
            }
            if (nW > 2) {
              Error("mergepTWFilteredttbar()", "Should have only 2W");
              return EL::StatusCode::FAILURE;
            }
          } else if (part->isElectron() || part->isMuon()) {
            TLorentzVector truthL;
            truthL.SetPtEtaPhiM(part->pt(), part->eta(), part->phi(),
                                part->m());
            if (m_vars.lepton.vec.DeltaR(truthL) < DRMin) {
              DRMin = m_vars.lepton.vec.DeltaR(truthL);
              index += 1;
              pTL = part->pt();
              EtaL = part->eta();
              PDGIDL = part->pdgId();
            }
          }
        }
      }

      if (PDGIDL < 0)
        pTW = (PDGIDW1 > 0) ? pTW1 : pTW2;  //+Lepton +W
      else if (PDGIDL > 0)
        pTW = (PDGIDW1 < 0) ? pTW1 : pTW2;  //-Lepton -W
      if (pTW > 100e3 && pTW < 200e3 && pTL > 20e3 && fabs(EtaL) < 3)
        m_weight *= 0.5;
      if (pTW > 200e3 && pTL > 20e3 && fabs(EtaL) < 3) m_weight *= 0.25;
    }

    if (m_mcChannel == 345951) m_weight *= 0.5;
    if (m_mcChannel == 346031) m_weight *= 0.75;
  }

  if (m_debug) {
    Info("mergepTWFilteredttbar()", "End of function mergepTWFilteredttbar()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::fill_VBF(){
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::run_1Lep_analysis() {
  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;
  if (m_debug) std::cout << " >>>>> Starting run_1Lep_analysis " << std::endl;
  if (m_debug) std::cout << " >>>>> VH Resonance Version <<<<< " << std::endl;
  if (m_debug) std::cout << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ " << std::endl;

  if (m_model != Model::HVT)
    Error("run_1Lep_analysis()",
          "Running VHreso1Lepton for model that isn't HVT!");

  // Veto Dilepton ttbar events if do doRemoveDilepOverlap
  if (m_doRemoveDilepOverlap && ttbarDiLeptonVeto())
    return EL::StatusCode::SUCCESS;

  m_histNameSvc->set_analysisType(HistNameSvc::AnalysisType::VHres);

  // Reset all event variables
  m_vars = {};
  //*********************************************************//
  //                     EVENT INITIALISATION                //
  //*********************************************************//

  // Select the objects
  EL_CHECK("run_1Lep_analysis()", selectObjects());

  // merge the samples if using pTW filtered ttbar samples
  EL_CHECK("run_1Lep_analysis()", mergepTWFilteredttbar());

  // Initialise lepton flavour category
  m_leptonFlavour = lepFlav::Combined;

  // Reset Meta data
  m_physicsMeta = PhysicsMetadata();
  m_physicsMeta.channel = PhysicsMetadata::Channel::OneLep;
  // Set Number of Jets - Extended to add track and fatjets to Meta data
  setevent_nJets(m_vars.signalJets, m_vars.forwardJets, m_vars.trackJets,
                 m_vars.fatJets);

  // Get selected jets and calualate number of tags
  EL_CHECK("run_1Lep_analysis()", computeJetsAndTagging());

  // Set all selected jets
  EL_CHECK("run_1Lep_analysis()", setJetVariables());

  // Set all selected fat jets
  EL_CHECK("run_1lep_analysis()", setFatJetVariables());

  // Compute a Higgs candidate from selected jets
  EL_CHECK("run_1Lep_analysis()", setHiggsCandidate());

  // Assign mBB-region for the Higgs candidate
  EL_CHECK("run_1Lep_analysis()", inmBBWindow());

  // Compute the rescaled yets (only resolved)
  EL_CHECK("run_1Lep_analysis()", rescale_jets());

  // Assign met related variables
  EL_CHECK("run_1Lep_analysis()", setMetAndMetSignificance());
  // Compute a VH candidate from a lepton, met, and the Higgs candidate
  EL_CHECK("run_1Lep_analysis()", setVHCandidate());

  // Set sumpt for the trigger
  // TODO trigger decision accessed earlier than this computation (like in
  // VHbb1Lep)
  EL_CHECK("run_1Lep_analysis()", setSumpt());

  // Determine event flag (do the cut flow)
  EL_CHECK("run_1Lep_analysis()", doEventFlag());

  // Set the cuts that we want to use
  EL_CHECK("run_1Lep_analysis()", setCuts());

  // Select resolved / merged depending on analysis mode
  select_regime();

  // Set the histogram names depending on regime / regions
  EL_CHECK("run_1Lep_analysis()", setHistNameSvcDescription());

  // Update event weights: leptonSF, btaggingSF, ...
  EL_CHECK("run_1Lep_analysis()", updateWeights());

  // Apply systematics
  EL_CHECK("run_1Lep_analysis()", doSystematics());

  // Decide whether to blind data / mc
  doBlinding();

  // Fill cutflows
  EL_CHECK("run_1Lep_analysis()", fill_1lepCutFlow("resolved"));
  EL_CHECK("run_1Lep_analysis()", fill_1lepCutFlow("merged"));

  EL_CHECK("run_1Lep_analysis()", selectCandidateVectors());

  // Quick veto of event before filling redundant histograms (for SM VH
  // analysis)
  if ((m_physicsMeta.nTags < 1 || m_physicsMeta.nJets > 3) &&
      m_reduceHistograms) {  // Added to reduce number of output histograms
                             // using reduceHistograms flag.  Just get > 0 tag
                             // and <  4 jet when true
    return EL::StatusCode::SUCCESS;
  }

  // Quick veto of events if they are in CR outter sidebands -->> Must change to
  // bit mask check. Should be faster than string comparison
  if (m_histNameSvc->get_description() == "RejCRmbb" && !m_doMbbRejHist) {
    return EL::StatusCode::SUCCESS;
  }
  // weight comparison for cutflow challenge
  EL_CHECK("run_1Lep_analysis()", printEventWeights());

  if (m_histNameSvc->get_description() == "") {
    if (m_debug) {
      Info("run_1Lep_analysis()",
           "Not histogram definition, moving on to the next event");
    }
    return EL::StatusCode::SUCCESS;
  }

  // Print cuts and stats if in debug mode
  EL_CHECK("run_1Lep_analysis()", printCuts());
  EL_CHECK("run_1Lep_analysis()", printStats());

  // Produce all plots for the CR and SR
  EL_CHECK("run_1Lep_analysis()", doInputPlots());
  EL_CHECK("run_1Lep_analysis()", doBasicPlots());
  EL_CHECK("run_1Lep_analysis()", doExtendedPlots());
  EL_CHECK("run_1Lep_analysis()", doTLVPlots());
  EL_CHECK("run_1Lep_analysis()", doSystPlots());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::initializeSelection() {
  // Read all variables that are relevant for the VHreso analysis from the
  // config and store the information in appropriate member variables.
  // The method consists of two parts. First, all default values are
  // initialised and then the information is read from the config.

  if (m_debug) {
    Info("initalizeSelection()", "Beginning of function initializeSelection()");
  }

  EL_CHECK("initializeSelection()",
           AnalysisReader_VHQQ1Lep::initializeSelection());

  m_config->getif<bool>("nominalOnly", m_nominalOnly);
  m_config->getif<bool>("doBJetenergyCorr", m_doBJetEnergyCorr);
  m_config->getif<bool>("doMbbRescaling", m_doMbbRescaling);
  m_config->getif<bool>("doIsoInv", m_doIsoInv);
  m_config->getif<bool>("doBlindingData", m_doBlindingData);
  m_config->getif<bool>("doBlindingMC", m_doBlindingMC);
  m_config->getif<bool>("applyFatJetMuOR", m_applyFatJetMuOR);
  m_config->getif<bool>("mVHvsPtvCut", m_mVHvsPtvCut);
  m_config->getif<bool>("doBasicPlots", m_doBasicPlots);  // A lot of plots ...
  m_config->getif<bool>("doInputPlots", m_doInputPlots);
  m_config->getif<bool>("doMJEtree", m_doMJEtree);
  m_config->getif<bool>("doExtendedPlots", m_doExtendedPlots);
  m_config->getif<bool>("doLTVPlots", m_doTLVPlots);
  m_config->getif<bool>("doMbbRejHist", m_doMbbRejHist);
  m_config->getif<bool>("reducHistograms", m_reduceHistograms);
  m_config->getif<bool>("writeMVATree", m_writeMVATree);
  m_config->getif<bool>("writeEasyTree", m_writeEasyTree);
  m_config->getif<bool>("printEventWeightsForCutFlow",
                        m_printEventWeightsForCutFlow);
  m_config->getif<bool>("doCutflow", m_doCutflow);
  m_config->getif<bool>("doMergeCR", m_doMergeCR);
  m_config->getif<bool>("useTTBarPTWFiltered1LepSamples",
                        m_useTTBarPTWFiltered1LepSamples);
  m_config->getif<bool>("applyPUWeight", m_applyPUWeight);
  m_config->getif<bool>("writeObjectsInEasyTree", m_writeObjectsInEasyTree);

  // Run the reader in inverted isolation mode?
  // (for multijet estimate)
  // default values

  if (m_debug) {
    Info("initalizeSelection()", "End of function initializeSelection()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::initializeEvent() {
  // Initialize the event
  //
  if (m_debug) {
    Info("initializeEvent()",
         "This is initializeEvent from the VHreso reader.");
  }

  if (m_debug) {
    Info("initializeEvent()", "Calling the base class initializeEvent now");
  }

  EL_CHECK("initializeEvent()", AnalysisReader::initializeEvent());

  if (m_debug) {
    Info("initalizeEvent()", "End of function initializeEvent()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::selectObjects() {
  // Get all objects from the VHbb0lepEvtSelection object and store objects in
  // member variables

  if (m_debug) Info("selectObjects()", "Beginning of function selectObjects()");

  ResultVHbb1lep selectionResult =
      ((VHbb1lepEvtSelection *)m_eventSelection)->result();

  m_vars.el = selectionResult.el;
  m_vars.mu = selectionResult.mu;
  m_vars.met = selectionResult.met;
  m_vars.signalJets = selectionResult.signalJets;
  m_vars.forwardJets = selectionResult.forwardJets;
  m_vars.fatJets = selectionResult.fatJets;
  m_vars.trackJets = selectionResult.trackJets;
  m_vars.taus = selectionResult.taus;

  m_vars.isTriggered = pass1LepTrigger(m_vars.triggerSF, selectionResult);

  // Assign lepton - Assume selectionResult only returns 1 lepton
  EL_CHECK("selectObjects()", setLeptonVariables());

  if (m_debug) Info("selectObjects()", "End of function SelectObjects()");

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHreso1Lep::setevent_nJets(
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::vector<const xAOD::Jet *> &forwardJets,
    const std::vector<const xAOD::Jet *> &trackJets,
    const std::vector<const xAOD::Jet *> &fatJets) {
  auto nSignalJet = signalJets.size();
  auto nForwardJet = forwardJets.size();
  auto nJet = nSignalJet + nForwardJet;
  auto nTrackJet = trackJets.size();
  auto nFatJet = fatJets.size();

  m_physicsMeta.nJets = nJet;
  m_physicsMeta.nSigJet = nSignalJet;
  m_physicsMeta.nForwardJet = nForwardJet;
  m_physicsMeta.nTrackJet = nTrackJet;
  m_physicsMeta.nFatJet = nFatJet;
}

void AnalysisReader_VHreso1Lep::fatJetMuOR() {
  // Apply Muon-Fat jet Overlap removal
  // TODO Why is this a vector???
  if (m_applyFatJetMuOR && m_vars.lepton.flav == mu) {
    std::vector<const xAOD::Muon *> Muons = {m_vars.mu};
    m_vars.fatJets = ApplyMuonFatJetOR(Muons, m_vars.fatJets);
    if (m_debug)
      Info("fatJetMuOR()", "Number of fat jets: " + m_vars.fatJets.size());
  }
}

EL::StatusCode AnalysisReader_VHreso1Lep::computeJetsAndTagging() {
  // Compute the tagging
  if (m_debug) {
    Info("computeJetsAndTagging()", "Computing jets and b-tagging.");
  }

  // Apply Muon-Fat jet overlap
  fatJetMuOR();

  // **Definition** : b-tagging**
  if (m_doTruthTagging) {
    compute_TRF_tagging(m_vars.signalJets);
  } else {
    compute_btagging();
  }
  // Fat jets
  compute_fatjetTags(m_vars.trackJets, m_vars.fatJets,
                     &m_vars.matchedLeadTrackJets, &m_vars.unmatchedTrackJets);
  if (m_debug) {
    Info("computeJetsAndTagging()", "Caluclated b-jet information");
  }

  if (m_vars.fatJets.size() >= 1 && m_debug) {
    Info("computeJetsAndTagging()",
         "Caluclated number of additional b-tags:  " +
             Props::nAddBTags.get(m_vars.fatJets.at(0)));
  }

  int tagcatExcl = -1;
  tagjet_selection(m_vars.signalJets, m_vars.forwardJets, m_vars.selectedJets,
                   tagcatExcl);
  m_physicsMeta.nTags = tagcatExcl;

  if (m_debug) {
    Info("computeJetsAndTagging()", "Computed jets and b-tagging.");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::setJetVariables() {
  // Initialize the jet variables
  if (m_debug) Info("setJetVariables()", "Setting all selected jet variables");

  Jet j1, j2, j3;  // small-R jets
  EL_CHECK("setJetVariables()", AnalysisReader_VHQQ::setJetVariables(
                                    j1, j2, j3, m_vars.selectedJets));

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

EL::StatusCode AnalysisReader_VHreso1Lep::setFatJetVariables() {
  // Initialize the fat jet variables

  if (m_debug) {
    Info("setFatJetVariables()", "Beginning of function setFatJetVariables()");
  }

  Jet j1, j2, j3;

  EL_CHECK("setFatJetVariables()",
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

EL::StatusCode AnalysisReader_VHreso1Lep::setHiggsCandidate() {
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
    j1 = m_vars.selectedEventJets.at(0);
  }
  if (m_vars.selectedEventJets.size() >= 2) {
    j2 = m_vars.selectedEventJets.at(1);
  }

  if (m_vars.selectedEventFatJets.size() >= 1) {
    fj = m_vars.selectedEventFatJets.at(0);
  }

  AnalysisReader_VHQQ::setHiggsCandidate(m_vars.resolvedH, m_vars.mergedH, j1,
                                         j2, fj);

  // Initiate the rescale vectors
  m_vars.resolvedH.vec_resc = m_vars.resolvedH.vec_corr;
  m_vars.mergedH.vec_resc = m_vars.mergedH.vec_corr;

  if (m_debug) {
    Info("setHiggsCandidate()", "End of function setHiggsCandidate()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::inmBBWindow() {
  // Check whether the Higgs candidate is inside the mBB window

  if (m_debug) {
    Info("inmBBWindow()", "Beginning of function inmBBWindow()");
  }

  // blinding window
  double mbbResolvedLowEdge = 110e3;
  double mbbResolvedHighEdge = 140e3;
  double mbbMergedLowEdge = 75e3;
  double mbbMergedHighEdge = 145e3;
  double mbbMergedLowerBound = 50e3;
  double mbbMergedUpperBound = 200e3;

  // mbb rejection
  double mbbResolvedLowerBound = 50e3;
  double mbbResolvedUpperBound = 200e3;

  // Resolved Regime
  if (m_vars.selectedEventJets.size() >= 2) {
    if ((m_vars.resolvedH.vec_corr.M() > mbbResolvedLowerBound) &&
        (m_vars.resolvedH.vec_corr.M() < mbbResolvedUpperBound))
      m_vars.inmBBresolvedrejWindow = true;

    if ((m_vars.resolvedH.vec_corr.M() > mbbResolvedLowEdge) &&
        (m_vars.resolvedH.vec_corr.M() < mbbResolvedHighEdge))
      m_vars.inmBBresolvedWindow = true;
    else if ((m_vars.resolvedH.vec_corr.M() > mbbResolvedLowerBound) &&
             (m_vars.resolvedH.vec_corr.M() < mbbResolvedLowEdge))
      m_physicsMeta.mbbSideBandResolved =
          PhysicsMetadata::MbbSideBandResolved::Low;
    else if ((m_vars.resolvedH.vec_corr.M() > mbbResolvedHighEdge) &&
             (m_vars.resolvedH.vec_corr.M() < mbbResolvedUpperBound))
      m_physicsMeta.mbbSideBandResolved =
          PhysicsMetadata::MbbSideBandResolved::High;
    else {
      m_physicsMeta.mbbSideBandResolved =
          PhysicsMetadata::MbbSideBandResolved::Outer;
    }
  } else {
    m_physicsMeta.mbbSideBandResolved =
        PhysicsMetadata::MbbSideBandResolved::undefined;
  }
  // Merged Regime
  if (m_vars.selectedEventFatJets.size() >= 1) {
    if ((m_vars.mergedH.vec_corr.M() > mbbMergedLowerBound) &&
        (m_vars.mergedH.vec_corr.M() < mbbMergedUpperBound))
      m_vars.inmBBmergedrejWindow = true;  // TODO what is this used for?

    if ((m_vars.mergedH.vec_corr.M() > mbbMergedLowEdge) &&
        (m_vars.mergedH.vec_corr.M() < mbbMergedHighEdge))
      m_vars.inmBBmergedWindow = true;
    else if (m_vars.mergedH.vec_corr.M() < mbbMergedLowEdge)
      m_physicsMeta.mbbSideBandMerged = PhysicsMetadata::MbbSideBandMerged::Low;
    else if (m_vars.mergedH.vec_corr.M() > mbbMergedHighEdge)
      m_physicsMeta.mbbSideBandMerged =
          PhysicsMetadata::MbbSideBandMerged::High;
    else {
      m_physicsMeta.mbbSideBandMerged =
          PhysicsMetadata::MbbSideBandMerged::Outer;
    }
  } else {
    m_physicsMeta.mbbSideBandMerged =
        PhysicsMetadata::MbbSideBandMerged::undefined;
  }
  if (m_debug) {
    Info("inmBBWindow()", "End of function inmBBWindow()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::rescale_jets() {
  // Overload to also update the higgs
  // TODO do we ever want this for fatjets? Only SM?
  if (m_debug) {
    Info("rescale_jets()", "Beginning of function rescale_jets()");
  }

  // TODO check the size for the first? Wasn't done previously
  if (m_doMbbRescaling && m_vars.inmBBresolvedWindow) {
    if (m_vars.selectedEventJets.size() >= 2) {
      AnalysisReader_VHQQ::rescale_jets(
          m_vars.resolvedH.vec_corr.M(),
          m_vars.selectedEventJets.at(0).vec_resc,
          m_vars.selectedEventJets.at(1).vec_resc);
      // Rescale Higgs
      m_vars.resolvedH.vec_resc = m_vars.selectedEventJets.at(0).vec_resc +
                                  m_vars.selectedEventJets.at(1).vec_resc;
    }
  }

  if (m_debug) {
    Info("rescale_jets()", "End of function rescale_jets()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::setMetAndMetSignificance() {
  // Get MET vector from MET object. Then get signifiances from
  // met container if specified in config

  if (m_debug) {
    Info("setMetAndMetSignificance()",
         "Beginning of function setMetAndMetSignificance()");
  }

  //  m_vars.metVec.SetPxPyPzE(m_vars.met->mpx(), m_vars.met->mpy(), 0,
  //                           m_vars.met->met());

  TLorentzVector metVec;
  metVec.SetPtEtaPhiM(m_vars.met->met(), 0, m_vars.met->phi(), 0);

  m_vars.metVec = metVec;

  if (!m_vars.met) {
    Error("setMetAndMetSignificance()", "No MET container set.");
    return EL::StatusCode::FAILURE;
  }

  // Need to be initalized??
  MET resolvedMET;
  MET mergedMET;

  resolvedMET.vec = m_vars.metVec;
  mergedMET.vec = m_vars.metVec;

  // Resolved regime
  if (m_vars.selectedEventJets.size() >= 2) {
    resolvedMET.vec_corr =
        getMETCorrTLV(m_vars.met,
                      {&m_vars.selectedEventJets.at(0).vec,
                       &m_vars.selectedEventJets.at(1).vec},
                      {&m_vars.selectedEventJets.at(0).vec_corr,
                       &m_vars.selectedEventJets.at(1).vec_corr});
    resolvedMET.vec_resc =
        getMETCorrTLV(m_vars.met,
                      {&m_vars.selectedEventJets.at(0).vec,
                       &m_vars.selectedEventJets.at(1).vec},
                      {&m_vars.selectedEventJets.at(0).vec_resc,
                       &m_vars.selectedEventJets.at(1).vec_resc});
  }

  // Merged regime
  if (m_vars.selectedEventFatJets.size() >= 1) {
    mergedMET.vec_corr =
        getMETCorrTLV(m_vars.met, {&m_vars.selectedEventFatJets.at(0).vec},
                      {&m_vars.selectedEventFatJets.at(0).vec_corr});
    mergedMET.vec_resc =
        getMETCorrTLV(m_vars.met, {&m_vars.selectedEventFatJets.at(0).vec},
                      {&m_vars.selectedEventFatJets.at(0).vec_resc});
  }
  m_vars.resolvedMET = resolvedMET;
  m_vars.mergedMET = mergedMET;

  // TODO could move this the MET structure?
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
    Info("setMetAndMetSignificance()",
         "End of function setMetAndMetSignificance()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::setLeptonVariables() {
  // initialize the lepton variables

  if (m_debug) {
    Info("setLeptonVariables()", "Beginning of function setLeptonVariables()");
  }
  m_vars.nLooseEl = 0;
  m_vars.nSigEl = 0;
  m_vars.nLooseMu = 0;
  m_vars.nSigMu = 0;

  Lepton lepton;
  lepton.vec = m_vars.el ? m_vars.el->p4() : m_vars.mu->p4();
  lepton.flav = m_vars.el ? el : mu;
  lepton.charge = m_vars.el ? m_vars.el->charge() : m_vars.mu->charge();

  lepton.vecT.SetPtEtaPhiM(lepton.vec.Pt(), 0, lepton.vec.Phi(), 0);
  m_vars.lepton = lepton;

  // **Definition** : lepton
  if (m_vars.el) {
    if (Props::isVHLooseElectron.get(m_vars.el) == 1) m_vars.nLooseEl = 1;
    if (Props::isWHSignalElectron.get(m_vars.el) == 1) m_vars.nSigEl = 1;
  } else if (m_vars.mu) {
    if (Props::isVHLooseMuon.get(m_vars.mu) == 1) m_vars.nLooseMu = 1;
    if (Props::isWHSignalMuon.get(m_vars.mu) == 1) m_vars.nSigMu = 1;
  } else {
    Error("setLeptonVariables()", "Missing lepton!");
    return EL::StatusCode::FAILURE;
  }

  if (m_debug) {
    Info("setLeptonVariables()", "End of function setLeptonVariables()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::setVHCandidate() {
  // Compute and set the VH candidates
  if (m_debug) {
    Info("setVHCandidate()", "Beginning of function setVHCandidate()");
  }

  EL_CHECK("setVHCandidate()", computeVHCandidate());

  if (m_debug) {
    Info("setVHCandidate()", "End of function setVHCandidate()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::setEventFlavour(
    std::vector<const xAOD::Jet *> jets) {
  // initialize the event flavour
  if (m_debug) {
    Info("setEventFlavour()", "Beginning of function setEventFlavour()");
  }

  setevent_flavour(
      jets);  // default from resolved analysis; is overwritten later

  if (m_debug) {
    Info("setEventFlavour()", "End of function setEventFlavour()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::setSumpt() {
  // Calculate jet pt sum
  // Use either 3 signal jets or 2 signal jets and 1 forward jet

  if (m_debug) {
    Info("setSumpt()", "Beginning of function setSumpt()");
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
      Info("setSumpt()", "Don't have enough jets to calculate sumpt");
      Info("setSumpt()",
           "m_vars.signalJets.size() = %lu, m_vars.forwardJets.size() = %lu, "
           "m_physicsMeta.nSigJet = %d, m_physicsMeta.nForwardJet = %d",
           m_vars.signalJets.size(), m_vars.forwardJets.size(),
           m_physicsMeta.nSigJet, m_physicsMeta.nForwardJet);
    }
    // Make sure that nothing went wrong with the physicsMeta
    if ((int(m_vars.signalJets.size()) != m_physicsMeta.nSigJet) ||
        (int(m_vars.forwardJets.size()) != m_physicsMeta.nForwardJet)) {
      Error("setSumpt()",
            "The jet vectors and the jet information in the metadata are "
            "mismatched.");
      Error("setSumppt()",
            "m_vars.signalJets.size() = %lu, m_vars.forwardJets.size() = %lu, "
            "m_physicsMeta.nSigJet = %d, m_physicsMeta.nForwardJet = %d",
            m_vars.signalJets.size(), m_vars.forwardJets.size(),
            m_physicsMeta.nSigJet, m_physicsMeta.nForwardJet);
      return EL::StatusCode::FAILURE;
    }
  }

  m_triggerTool->setSumpt(m_vars.sumpt);

  if (m_debug) {
    Info("setSumpt()", "End of function setSumpt()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::doEventFlag() {
  // Determine which cuts the event passes
  // Differences between different analyses and resolved / merged are taken into
  // account
  // The used updateFlag method should be called only once per cut, since by
  // using a logical OR once a bit is set to 1 it can not be reset to 0
  //
  // The CutFlow consists of
  //   1. All CxAOD
  //   2. Trigger (-> trigger scalefactor)
  //   3. 1 loose lepton
  //   4. 1 signal lepton
  //   5. >= 2 jets
  //   6. 2-3 signal jets
  //   7. >= 1 fatjet
  //   8. >= 2 trackjets
  //   9. mbbCorr
  //   10. leading jet pt > 45 GeV
  //   11. MET cut
  //   12. mTW cut
  //   13. 3rd b-jet veto
  //   14. Classify if mBB restriction or not
  //   15. pTW cut
  //   16. dRBB cut
  //   17. Additonal mTW cut
  //
  //   Resolved / Merged SR
  //
  // NOT ORDERED ANY MORE! Done in setCuts()
  // Remove depedency on passAllCutsUpTo(), change to passSpecificCuts()

  if (m_debug) {
    Info("doEventFlag()", "Beginning of function doEventFlag()");
  }

  // 1: All events (CxAOD)
  updateFlag(m_vars.eventFlag, OneLeptonCuts::AllCxAOD);

  // 2: trigger
  updateFlag(m_vars.eventFlag, OneLeptonCuts::Trigger, m_vars.isTriggered);
  // 3: 1 loose lepton
  updateFlag(m_vars.eventFlag, OneLeptonCuts::LooseLepton,
             (m_vars.nLooseEl + m_vars.nLooseMu == 1));

  // 4: 1 signal lepton  (HVT Lepton defintion raised to 27GeV du to new 2016
  // trigger menu) (SM VH(bb) Electron Channel and medium pTV region Muon
  // channel raised to 27GeV, High pTV region Muon Channel stay with 25GeV)
  // retrieve quantities on isolation and lepton quality to restore cuts
  // on reader level that were opened for the MJ CxAODs
  bool signalLeptons = signalLepton();
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SignalLepton, signalLeptons);

  // 5: require at least 2 jets
  updateFlag(m_vars.eventFlag, OneLeptonCuts::AtLeast2Jets,
             m_physicsMeta.nJets >= 2);

  // 6: require at least 2 and most 3 signal jets
  updateFlag(m_vars.eventFlag, OneLeptonCuts::AtLeast2SigJets,
             (m_physicsMeta.nSigJet >= 2 && m_physicsMeta.nSigJet <= 3));

  // 7: At least one fat jet
  updateFlag(m_vars.eventFlag, OneLeptonCuts::AtLeast1FatJet,
             m_physicsMeta.nFatJet >= 1);

  // 8: At least one matched track jets
  updateFlag(m_vars.eventFlag, OneLeptonCuts::AtLeast2TrackJets,
             m_vars.matchedLeadTrackJets.size() >= 1);

  // 9: in mBB window
  updateFlag(m_vars.eventFlag, OneLeptonCuts::mbbCorrResolved,
             m_vars.inmBBresolvedWindow);
  updateFlag(m_vars.eventFlag, OneLeptonCuts::mbbCorrMerged,
             m_vars.inmBBmergedWindow);

  // 10: leading jet pT > 45 GeV
  updateFlag(m_vars.eventFlag, OneLeptonCuts::Pt45,
             m_vars.selectedEventJets.size() > 0 &&
                 m_vars.selectedEventJets.at(0).vec.Pt() > 45e3);

  // 11: pass MET cut dependent on lepton flavor
  bool passMET = passMETCut("resolved");
  updateFlag(m_vars.eventFlag, OneLeptonCuts::METResolved, passMET);
  passMET = passMETCut("merged");
  updateFlag(m_vars.eventFlag, OneLeptonCuts::METMerged, passMET);

  // 12: pass mTW cut dependent on pT of W
  bool passmTW = passmTWCut("resolved");
  updateFlag(m_vars.eventFlag, OneLeptonCuts::mTWResolved, passmTW);
  passmTW = passmTWCut("merged");
  updateFlag(m_vars.eventFlag, OneLeptonCuts::mTWMerged, passmTW);

  // 13: 3rd b-jet veto
  updateFlag(m_vars.eventFlag, OneLeptonCuts::Veto3bjet,
             m_physicsMeta.nTags < 3);

  // 14: decide mBB restriction (for CR?)
  bool is_not_in_mbbRestrict =
      !m_doMbbRejHist ? m_vars.inmBBresolvedrejWindow : true;
  updateFlag(m_vars.eventFlag, OneLeptonCuts::mbbRestrictResolved,
             is_not_in_mbbRestrict);
  updateFlag(m_vars.eventFlag, OneLeptonCuts::mbbRestrictMerged,
             m_vars.inmBBmergedrejWindow);

  // 15: pTW cut
  bool passpTW = passpTVCut("resolved");
  updateFlag(m_vars.eventFlag, OneLeptonCuts::pTVResolved, passpTW);
  passpTW = passpTVCut("merged");
  updateFlag(m_vars.eventFlag, OneLeptonCuts::pTVMerged, passpTW);

  // TODO Are these needed by HVT?
  // 16: dRBB cut in Cut-based
  bool passdRBB = passdRBBCut();
  updateFlag(m_vars.eventFlag, OneLeptonCuts::dRBB, passdRBB);

  // 17: additional mTW cut in Cut-based
  updateFlag(m_vars.eventFlag, OneLeptonCuts::mTW_add,
             m_vars.resolvedVH.VvecT.M() < 120e3);

  // Resolved SR flags
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SR_0tag_2pjet,
             ((m_physicsMeta.nTags == 0) && (m_physicsMeta.nJets >= 2)));
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SR_1tag_2pjet,
             ((m_physicsMeta.nTags == 1) && (m_physicsMeta.nJets >= 2)));
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SR_2tag_2pjet,
             ((m_physicsMeta.nTags == 2) && (m_physicsMeta.nJets >= 2)));

  // Merged SR flags
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SR_0tag_1pfat0pjets,
             ((m_physicsMeta.nFatJet > 0 &&
               Props::nBTags.get(m_vars.fatJets.at(0)) == 0)));
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SR_1tag_1pfat0pjets,
             ((m_physicsMeta.nFatJet > 0 &&
               Props::nBTags.get(m_vars.fatJets.at(0)) == 1)));
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SR_2tag_1pfat0pjets,
             ((m_physicsMeta.nFatJet > 0 &&
               Props::nBTags.get(m_vars.fatJets.at(0)) == 2)));
  updateFlag(m_vars.eventFlag, OneLeptonCuts::SR_3ptag_1pfat0pjets,
             ((m_physicsMeta.nFatJet > 0 &&
               Props::nBTags.get(m_vars.fatJets.at(0)) >= 3)));

  if (m_debug) {
    Info("doEventFlag()", "End of function doEventFlag()");
  }

  return EL::StatusCode::SUCCESS;
}

bool AnalysisReader_VHreso1Lep::signalLepton() {
  // Checks if the selected lepton passes the signal cuts
  if (m_debug) {
    Info("signalLepton()", "Beginning of function signalLepton()");
  }
  bool passPtLep27 = m_vars.lepton.vec.Pt() > 27e3;

  float caloIso = -1.0;
  float trackIso = -1.0;
  bool passNom = false;
  bool passIsoInv = false;
  bool passLepQuality = false;

  if (m_vars.el) {
    caloIso = Props::topoetcone20.get(m_vars.el) / m_vars.lepton.vec.Pt();
    trackIso = Props::ptvarcone20.get(m_vars.el) / m_vars.lepton.vec.Pt();
    passNom = caloIso < 0.06 && trackIso < 0.06;
    passIsoInv = caloIso > 0.06 && trackIso < 0.06;
    passLepQuality = Props::isTightLH.get(m_vars.el) == 1;
  } else if (m_vars.mu) {
    caloIso = Props::ptvarcone30.get(m_vars.mu) / m_vars.lepton.vec.Pt();
    passNom = caloIso < 0.06;
    passIsoInv = caloIso > 0.06;
    passLepQuality = Props::isWHSignalMuon.get(m_vars.mu) == 1;
  }
  if (m_debug) {
    Info("signalLepton()", "End of function signalLepton()");
  }

  return (
      passPtLep27 && passLepQuality && (m_vars.nSigEl + m_vars.nSigMu == 1) &&
      ((!m_doIsoInv && passNom) || ((m_doIsoInv || m_doQCD) && passIsoInv)));
}

bool AnalysisReader_VHreso1Lep::passMETCut(std::string regime) {
  // Does the minimum MET cut for Mu and El be configurable?
  if (m_debug) {
    Info("passMETCut()", "Beginning of function passMETCut()");
  }

  bool passCut = false;

  if (regime == "resolved") {
    if (m_vars.nSigEl == 1 && m_vars.nSigMu == 0) {
      passCut = m_vars.metVec.Pt() > 30e3;
    } else if (m_vars.nSigEl == 0 && m_vars.nSigMu == 1) {
      passCut = m_vars.metVec.Pt() > 0e3;
    } else {
      Error("passMETCut()", "Invalid lepton type");
      return EL::StatusCode::FAILURE;
    }
  } else if (regime == "merged") {
    passCut = m_vars.metVec.Pt() > 100e3;
  }

  if (m_debug) {
    Info("passMETCut()", "End of function passMETCut()");
  }

  return passCut;
}

bool AnalysisReader_VHreso1Lep::passmTWCut(std::string regime) {
  // Does the mTW cuts need be configurable?

  if (m_debug) {
    Info("passmTWCut()", "Beginning of function passmTWCut()");
  }

  bool passCut = false;
  if (regime == "resolved") {
    if (m_vars.resolvedVH.VvecT.Pt() > 150e3) {
      passCut = (m_vars.resolvedVH.VvecT.M() > 0e3 &&
                 m_vars.resolvedVH.VvecT.M() < 300e3);
    } else {
      passCut = m_vars.resolvedVH.VvecT.M() > 20e3;
    }
  } else if (regime == "merged") {
    passCut = (m_vars.resolvedVH.VvecT.M() > 0e3 &&
               m_vars.resolvedVH.VvecT.M() < 300e3);
  } else {
    passCut = false;
  }

  if (m_debug) {
    Info("passmTWCut()", "End of function passmTWCut()");
  }

  return passCut;
}

bool AnalysisReader_VHreso1Lep::passpTVCut(std::string regime) {
  // Check if the pTVCut is passed, dependent on region and cut configuration
  if (m_debug) {
    Info("passpTVCut()", "Beginning of function passpTVCut()");
  }
  // pTV
  bool passCut = false;
  if (m_mVHvsPtvCut) {
    if (regime == "resolved") {
      //###############  mVH dependent pTV cut (optimised) ################
      // double low_Opt_grad = 249.88;    //(1+2-tag)   (log(x) version)
      // double low_Opt_inter = -1352.31;  //(1+2-tag)   (log(x) version)
      double low_Opt_grad = -326052;  //(1+2-tag)   (inv(x) version)
      double low_Opt_inter = 709.60;  //(1+2-tag)   (inv(x) version)
      double InvMassVH = m_vars.resolvedVH.vec_corr.M() / 1e3;
      // bool passLowerOpt = (WVecT.Pt() / 1e3) > (low_Opt_grad*log(InvMassVH))
      // + low_Opt_inter;  //(log(x) version)
      bool passLowerOpt =
          (m_vars.resolvedVH.VvecT.Pt() / 1e3) >
          (low_Opt_grad / (InvMassVH)) + low_Opt_inter;  //(inv(x) version)

      // Update the pTV flag for the resolved regime
      passCut = m_vars.resolvedVH.VvecT.Pt() / 1.e3 > 150 && passLowerOpt;
    } else if (regime == "merged") {
      if (m_vars.fatJets.size() > 0) {
        //############### mVH dependent pTV cut (optimised) ################
        double low_Opt_grad = 394.241;    //(1+2-tag)
        double low_Opt_inter = -2350.04;  //(1+2-tag)
        double InvMassVH = m_vars.mergedVH.vec_corr.M() / 1e3;
        bool passLowerOpt = (m_vars.resolvedVH.VvecT.Pt() / 1e3) >
                            (low_Opt_grad * log(InvMassVH)) + low_Opt_inter;
        passCut = passLowerOpt && m_vars.resolvedVH.VvecT.Pt() / 1.e3 > 150.0;
      }
    } else {
      passCut = m_vars.resolvedVH.VvecT.Pt() / 1.e3 > 150;
    }
  } else {
    if (regime == "resolved")
      passCut = m_vars.resolvedVH.VvecT.Pt() / 1.e3 > 150;
    else if (regime == "merged")
      passCut = m_vars.mergedVH.VvecT.Pt() / 1.e3 > 150;
  }

  if (m_debug) {
    Info("passpTVCut()", "End of function passpTVCut()");
  }

  return passCut;
}

bool AnalysisReader_VHreso1Lep::passdRBBCut() {
  // TODO Is this even used for the HVT?
  if (m_debug) {
    Info("passdRBBCut()", "Beginning of function passdRBBCut()");
  }

  bool passCut = false;

  if (m_vars.selectedEventJets.size() >= 2) {
    if (m_vars.resolvedVH.VvecT.Pt() <= 150e3) {
      passCut = m_vars.selectedEventJets.at(0).vec.DeltaR(
                    m_vars.selectedEventJets.at(1).vec) < 3.0;
    } else if (m_vars.resolvedVH.VvecT.Pt() > 150e3 &&
               m_vars.resolvedVH.VvecT.Pt() <= 200e3) {
      passCut = m_vars.selectedEventJets.at(0).vec.DeltaR(
                    m_vars.selectedEventJets.at(1).vec) < 1.8;
    } else {  // if (m_vars.resolvedVH.VvecT.Pt() > 200e3){
      passCut = m_vars.selectedEventJets.at(0).vec.DeltaR(
                    m_vars.selectedEventJets.at(1).vec) < 1.2;
    }
  }
  if (m_debug) {
    Info("passdRBBCut()", "End of function passdRBBCut()");
  }

  return passCut;
}

EL::StatusCode AnalysisReader_VHreso1Lep::setCuts() {
  // A seprate method to initialize the cuts for the different classes of events
  // One could think about defining these vectors as static or global constants
  // but the compiler probably takes care of that anyway and like this it is at
  // least tidy

  if (m_debug) {
    Info("setCuts()", "Beginning of function setCuts()");
  }

  // TODO Check if swapped cuts affect end results ... mbbCorr / pTVResolved
  m_vars.cuts_common_resolved = {OneLeptonCuts::AllCxAOD,
                                 OneLeptonCuts::Trigger,
                                 OneLeptonCuts::LooseLepton,
                                 OneLeptonCuts::SignalLepton,
                                 OneLeptonCuts::AtLeast2Jets,
                                 OneLeptonCuts::AtLeast2SigJets,
                                 OneLeptonCuts::Pt45,
                                 OneLeptonCuts::METResolved,
                                 OneLeptonCuts::mTWResolved,
                                 OneLeptonCuts::Veto3bjet,
                                 OneLeptonCuts::mbbRestrictResolved,
                                 OneLeptonCuts::mbbCorrResolved,
                                 OneLeptonCuts::pTVResolved};

  // resolved signal region - start from common cuts
  m_vars.cuts_SR_resolved = m_vars.cuts_common_resolved;

  // add additional cuts
  std::vector<unsigned long int> additional_SR_resolved_cuts = {
      OneLeptonCuts::SR_0tag_2pjet, OneLeptonCuts::SR_1tag_2pjet,
      OneLeptonCuts::SR_2tag_2pjet};

  m_vars.cuts_SR_resolved.insert(m_vars.cuts_SR_resolved.end(),
                                 additional_SR_resolved_cuts.begin(),
                                 additional_SR_resolved_cuts.end());

  // cuts for jet histograms
  m_vars.cuts_jet_hists_resolved = {};

  m_vars.cuts_common_merged = {
      OneLeptonCuts::AllCxAOD,          OneLeptonCuts::Trigger,
      OneLeptonCuts::SignalLepton,      OneLeptonCuts::METMerged,
      OneLeptonCuts::AtLeast1FatJet,    OneLeptonCuts::mbbCorrMerged,
      OneLeptonCuts::AtLeast2TrackJets, OneLeptonCuts::mTWMerged,
      OneLeptonCuts::pTVMerged,         OneLeptonCuts::mbbRestrictMerged};

  // merged signal region - start from common cuts
  m_vars.cuts_SR_merged = m_vars.cuts_common_merged;

  // add additional cuts
  std::vector<unsigned long int> additional_SR_merged_cuts = {
      OneLeptonCuts::SR_0tag_1pfat0pjets, OneLeptonCuts::SR_1tag_1pfat0pjets,
      OneLeptonCuts::SR_2tag_1pfat0pjets, OneLeptonCuts::SR_3ptag_1pfat0pjets};
  m_vars.cuts_SR_merged.insert(m_vars.cuts_SR_merged.end(),
                               additional_SR_merged_cuts.begin(),
                               additional_SR_merged_cuts.end());

  if (m_debug) {
    Info("setCuts()", "End of function setCuts()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::selectCandidateVectors() {
  // The vectors that we want to use depend on the regime and the model
  // Here we select the correct object from our Higgs and VH candidates

  if (m_debug) {
    Info("selectCandidateVectors()",
         "Beginning of function selectCandidateVectors()");
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
      if (m_doMbbRescaling)
        m_vars.VHVec = m_vars.resolvedVH.vec_resc;
      else {
        m_vars.VHVec = m_vars.resolvedVH.vec_corr;
      }
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
    Info("selectCandidateVectors()",
         "End of function selectCandidateVectors()");
  }

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHreso1Lep::select_regime() {
  // Select the regime and set physicsMetaData
  if (m_debug) {
    Info("select_regime()", "Beginning of function select_regime()");
  }

  bool passResolvedSR =
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_resolved, {});
  bool passResolvedCR =
      !passResolvedSR &&
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_resolved,
                       {OneLeptonCuts::mbbCorrResolved});

  // TODO is mbbRestricted only applied for the SR?
  bool passMergedSR =
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged,
                       {OneLeptonCuts::mbbRestrictMerged});
  bool passMergedCR =
      !passMergedSR &&
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged,
                       {OneLeptonCuts::mbbCorrMerged});

  bool passMerged = (m_physicsMeta.nFatJet >= 1);
  bool passResolved = (m_physicsMeta.nSigJet >= 2);
  if (m_debug)
    std::cout << "  ####################### passResolvedSR :" << passResolvedSR
              << std::endl;
  if (m_debug)
    std::cout << "  ####################### passMergedSR :" << passMergedSR
              << std::endl;

  if (m_debug) {
    std::cout << " >>>>> Resolved/Merged Analysis Decision" << std::endl;
    std::cout << "       passResolved : " << passResolved << std::endl;
    std::cout << "       passMerged : " << passMerged << std::endl;
  }

  if (m_analysisStrategy == "Resolved") {
    if (passResolved) m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "Merged") {
    if (passMerged) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "RecyclePtV") {
    // Give priority to merged above 500 GeV
    if (m_vars.resolvedVH.VvecT.Pt() / 1.e3 > 500.) {
      if (passMerged)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
      else if (passResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    } else {
      if (passResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
      else if (passMerged)
        m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    }
  } else if (m_analysisStrategy == "SimpleMerge500") {
    // Only merged above 500 GeV
    if (m_vars.resolvedVH.VvecT.Pt() / 1.e3 > 500.) {
      if (passMerged) m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    } else {
      if (passResolved)
        m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    }
  } else if (m_analysisStrategy == "PriorityResolved") {
    if (passResolved)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMerged)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "PriorityResolvedSR") {
    if (passResolvedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "PriorityResolvedSRbtag") {
    if (passResolvedSR && (m_physicsMeta.nTags == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR && (Props::nBTags.get(m_vars.fatJets.at(0)) == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR && (m_physicsMeta.nTags == 1))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR && (Props::nBTags.get(m_vars.fatJets.at(0)) == 1))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
  } else if (m_analysisStrategy == "PriorityMerged") {
    if (passMerged)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolved)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "PriorityMergedSR") {
    if (passMergedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  } else if (m_analysisStrategy == "PriorityMergedSRbtag") {
    if (passMergedSR && (Props::nBTags.get(m_vars.fatJets.at(0)) == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR && (m_physicsMeta.nTags == 2))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedSR && (Props::nBTags.get(m_vars.fatJets.at(0)) == 1))
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedSR && (m_physicsMeta.nTags == 1))
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
    else if (passMergedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::merged;
    else if (passResolvedCR)
      m_physicsMeta.regime = PhysicsMetadata::Regime::resolved;
  }

  m_vars.isResolved =
      (m_physicsMeta.regime == PhysicsMetadata::Regime::resolved);
  m_vars.isMerged = (m_physicsMeta.regime == PhysicsMetadata::Regime::merged);

  if (m_debug) {
    std::cout << " >>>>> Resolved/Merged Analysis Decision" << std::endl;
    std::cout << "       isResolved : " << m_vars.isResolved << std::endl;
    std::cout << "       isMerged : " << m_vars.isMerged << std::endl;
  }

  if (m_debug) {
    Info("select_regime()", "End of function select_regime()");
  }
}

EL::StatusCode AnalysisReader_VHreso1Lep::setHistNameSvcDescription() {
  // Set the properties of HistNameSvc
  if (m_debug) {
    Info("setHistNameSvcDescription()",
         "Beginning of function setHistNameSvcDescription()");
  }
  m_histNameSvc->set_description("");

  // leptons
  if (!m_doMergePtVBins) m_histNameSvc->set_pTV(m_vars.resolvedVH.VvecT.Pt());
  // here the difference is made between merged and resolved in histnames
  if (m_vars.isResolved) {
    setevent_flavour(m_vars.selectedJets);
    m_histNameSvc->set_nTag(m_physicsMeta.nTags);
    m_histNameSvc->set_nJet(m_physicsMeta.nJets);
    if (m_doMergeJetBins)
      m_histNameSvc->set_nJet(m_physicsMeta.nJets >= 2 ? -2
                                                       : m_physicsMeta.nJets);
    else
      m_histNameSvc->set_nJet(m_physicsMeta.nJets);
  } else if (m_vars.isMerged) {
    // Flavor labeling option: TrackJetCone
    setevent_flavour(m_vars.matchedLeadTrackJets);

    m_histNameSvc->set_nTag(Props::nBTags.get(m_vars.fatJets.at(0)));
    m_histNameSvc->set_nFatJet(m_physicsMeta.nFatJet);
    m_histNameSvc->set_nBTagTrackJetUnmatched(
        Props::nAddBTags.get(m_vars.fatJets.at(0)));
    m_physicsMeta.nAddBTrkJets = Props::nAddBTags.get(m_vars.fatJets.at(0));
  }  // end of if ( m_vars.isMerged )

  m_histNameSvc->set_eventFlavour(m_physicsMeta.b1Flav, m_physicsMeta.b2Flav);
  // Region
  if (m_vars.isResolved) {
    if (passSpecificCuts(
            m_vars.eventFlag, m_vars.cuts_common_resolved,
            {OneLeptonCuts::mbbCorrResolved})) {  // all signal cuts; ignore mbb
      if (passSpecificCuts(m_vars.eventFlag, {OneLeptonCuts::mbbCorrResolved},
                           {})) {
        m_histNameSvc->set_description("SR");
        m_physicsMeta.region = PhysicsMetadata::Region::SR;
      } else if (passSpecificCuts(m_vars.eventFlag,
                                  {OneLeptonCuts::mbbRestrictResolved}, {})) {
        // Set the Control Region name
        std::string CR_Name =
            (m_physicsMeta.mbbSideBandResolved ==
             PhysicsMetadata::MbbSideBandResolved::Low)
                ? (m_doMergeCR ? "mBBcr" : "lowmBBcr")
                : ((m_physicsMeta.mbbSideBandResolved ==
                    PhysicsMetadata::MbbSideBandResolved::High)
                       ? (m_doMergeCR ? "mBBcr" : "highmBBcr")
                       : "RejCRmbb");

        // Set the PhysicsMetadata:: Merged side band definition (used later
        // in model systematics definition)
        m_physicsMeta.region = PhysicsMetadata::Region::mbbCR;
        m_histNameSvc->set_description(CR_Name);
      }
    } else {
      // Skip the event
      if (!m_doCutflow) return EL::StatusCode::SUCCESS;
    }

  } else if (m_vars.isMerged) {
    if (passSpecificCuts(
            m_vars.eventFlag, m_vars.cuts_common_merged,
            {OneLeptonCuts::mbbCorrMerged,
             OneLeptonCuts::mbbRestrictMerged})) {  // all signal cuts; ignore
                                                    // mbb
      if (passSpecificCuts(m_vars.eventFlag, {OneLeptonCuts::mbbCorrMerged},
                           {})) {
        m_histNameSvc->set_description("SR");
        m_physicsMeta.region = PhysicsMetadata::Region::SR;
      } else if (passSpecificCuts(m_vars.eventFlag,
                                  {OneLeptonCuts::mbbRestrictMerged}, {})) {
        std::string CR_Name =
            m_doMergeCR ? "mBBcr"
                        : (m_physicsMeta.mbbSideBandMerged ==
                                   PhysicsMetadata::MbbSideBandMerged::Low
                               ? "lowmBBcr"
                               : "highmBBcr");
        m_physicsMeta.region = PhysicsMetadata::Region::mbbCR;
        m_histNameSvc->set_description(CR_Name);
      }
      if (m_debug)
        std::cout << " >>>>> EventCategory : "
                  << m_histNameSvc->getFullHistName("mVH") << std::endl;
    } else {
      // Skip the event
      if (!m_doCutflow) return EL::StatusCode::SUCCESS;
    }
  }

  m_vars.Mtop = m_vars.selectedEventJets.size() >= 2
                    ? calculateMtop(m_vars.lepton.vec, m_vars.metVec,
                                    m_vars.selectedEventJets.at(0).vec_corr,
                                    m_vars.selectedEventJets.at(1).vec_corr)
                    : 0.;  // Moved these here to be used for W+hf CR definition

  if (m_debug) {
    Info("setHistNameSvcDescription()",
         "End of function setHistNameSvcDescription()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::updateWeights() {
  // Produce the event weights

  if (m_debug) {
    Info("updateWeights()", "Beginning of function updateWeights()");
  }

  // **Weight**: Lepton
  m_vars.leptonSF = 1.;
  if (m_isMC) {
    m_vars.leptonSF = Props::leptonSF.get(m_eventInfo);
  }

  m_weight *= m_vars.leptonSF;

  // **Weight** : Trigger
  // double triggerSF = 1.;
  // bool isTriggered =  pass1LepTrigger(triggerSF,selectionResult);
  // if (m_isMC) m_weight *= triggerSF; <- is done already inside
  // pass1LepTrigger

  // **Weight** : JVT
  if (m_isMC) m_weight *= compute_JVTSF(m_vars.signalJets);

  // **Weight** : b-tagging
  float btagWeight = 1.;  // for later use
  // btagWeight is truth_tag_weight _with_ SFs if m_doTruthTagging
  // (getEfficiency() includes SFs)
  if (m_isMC && m_vars.isResolved) {
    // We set the bTagTool's jet author to the authorName argument passed to the
    // computeBTagSFWeight now just in case
    btagWeight =
        computeBTagSFWeight(m_vars.signalJets, m_jetReader->getContainerName());
    if (btagWeight <= 0) {
      Info("updateWeights()", "btagWeight = %f, be warned", btagWeight);
    }
  } else if (m_isMC && m_vars.isMerged) {
    // Need to use the jets inside the fatjet and the unmatched trackjets
    std::vector<const xAOD::Jet *> trackJetsForBTagging =
        m_vars.matchedLeadTrackJets;
    trackJetsForBTagging.insert(trackJetsForBTagging.end(),
                                m_vars.unmatchedTrackJets.begin(),
                                m_vars.unmatchedTrackJets.end());
    btagWeight = computeBTagSFWeight(trackJetsForBTagging,
                                     m_trackJetReader->getContainerName());
    if (btagWeight <= 0) {
      Info("updateWeights()", "btagWeight = %f, be warned", btagWeight);
    }
  }
  m_vars.btagWeight = btagWeight;
  m_weight *= btagWeight;

  if (m_isMC && m_doTruthTagging)
    BTagProps::truthTagEventWeight.set(m_eventInfo, btagWeight);
  // **Weight** : Pileup
  if (m_isMC) {
    if (!m_applyPUWeight) {
      m_weightSysts.push_back({"_withPU", (float)(m_pileupReweight)});
    }
  }

  // **Weight** : FakeFactor method for QCD
  if (m_doQCD) {
    m_histNameSvc->set_sample("multijet");
    if (m_vars.el && m_vars.nSigEl == 1) {
      EL_CHECK("updateWeights()", applyFFweight(m_vars.resolvedVH.VvecT,
                                                m_vars.metVec, m_vars.el));
    }
    if (m_vars.mu && m_vars.nLooseMu == 1) {
      EL_CHECK("updateWeights()", applyFFweight(m_vars.resolvedVH.VvecT,
                                                m_vars.metVec, m_vars.mu));
    }
    // applyFFweight might set event weight to zero in some cases,
    // like e.g. bad track isolation for electrons
    // (DBL_DIG: number of double digits, defined in <cfloat>)
    if (abs(m_weight) < (1. / (DBL_DIG - 1))) {
      if (m_debug) {
        Info("updateWeights()",
             "abs(m_weight) < 1e-%d after applyFFweight. Reject event.",
             (DBL_DIG - 1));
      }
      return EL::StatusCode::SUCCESS;
    }
  }

  if (m_debug) {
    Info("updateWeights()", "End of function updateWeights()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::doSystematics() {
  // Produce everything related to systematics
  // TODO could split into many functions
  if (m_debug) {
    Info("doSystematics()", "Beginning of function doSystematics()");
  }
  if (m_isMC &&
      ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    // apply CorrsAndSysts to m_weight and m_weightSysts (relies on
    // m_histNameSvc): added the MJ_TrigEff systematic from the template method
    // on 23-Jan-2017 this should be moved to the TriggerTool_VHbb eventually
    if (m_vars.isResolved) {
      applyCorrections();
      applySystematics(m_vars.resolvedVH.VvecT.Pt(), m_vars.metVec.Pt(),
                       m_physicsMeta.nJets, m_physicsMeta.nTags,
                       m_vars.selectedEventJets.at(0).vec_corr,
                       m_vars.selectedEventJets.at(1).vec_corr,
                       m_model == Model::CUT, m_vars.Mtop);
    }
  }

  if (m_doIsoInv &&
      (m_csCorrections.size() != 0 || m_csVariations.size() != 0)) {
    EL_CHECK("doSystematics()", applyCS_MJ(m_vars.lepton.vec));
  }

  //-------------------------
  // TTbar & Stop systematics histos
  //--------------------------
  // NOTE :: mVH/1e3 !!!!!!!!!!!!!!!!!!!
  if ((m_histNameSvc->get_sample() == "ttbar" ||
       m_histNameSvc->get_sample().find("stop") != std::string::npos) &&
      m_currentVar == "Nominal" && m_model == Model::HVT &&
      (m_vars.isResolved || m_vars.isMerged) && !m_nominalOnly) {
    double mBB = m_vars.isResolved
                     ? m_vars.resolvedH.vec_corr.M()
                     : m_vars.selectedEventFatJets.at(0).vec_corr.M();
    float mVH = m_vars.isResolved
                    ? (m_doMbbRescaling ? m_vars.resolvedVH.vec_resc.M()
                                        : m_vars.resolvedVH.vec_corr.M())
                    : m_vars.mergedVH.vec_corr.M();
    std::string Regime = m_vars.isResolved ? "Resolved" : "Boosted";
    float nAddBTags =
        m_vars.isResolved ? 0 : Props::nAddBTags.get(m_vars.fatJets.at(0));

    fillTopSystslep_PRSR(Regime, nAddBTags, mBB,
                         mVH / 1e3);  // mVH= GeV, mBB = MeV
  }

  //*********************************************************//
  //              ttbar  BDT Based Systematics               //
  //                                                         //
  //*********************************************************//
  //-----------------------------
  // VJets Systematic Histograms
  //-----------------------------
  // NOTE :: mBB is in MeV because CorrsAndSysts uses MeV !!!!!!!!!!!!!!!!!!!
  if ((m_histNameSvc->get_sample() == "W" ||
       m_histNameSvc->get_sample() == "Wv22") &&
      m_currentVar == "Nominal" && m_model == Model::HVT &&
      (m_vars.isResolved || m_vars.isMerged) && !m_nominalOnly) {
    double mBB = m_vars.isResolved
                     ? m_vars.resolvedH.vec_corr.M()
                     : m_vars.selectedEventFatJets.at(0).vec_corr.M();
    float mVH = m_vars.isResolved
                    ? (m_doMbbRescaling ? m_vars.resolvedVH.vec_resc.M()
                                        : m_vars.resolvedVH.vec_corr.M())
                    : m_vars.mergedVH.vec_corr.M();
    // fillVjetsSystslep( mBB, mVH/1e3);
    std::string Regime = m_vars.isResolved ? "Resolved" : "Boosted";
    float nAddBTags =
        m_vars.isResolved ? 0 : Props::nAddBTags.get(m_vars.fatJets.at(0));
    fillVjetsSystslep_PRSR(Regime, mBB, mVH / 1e3,
                           nAddBTags);  // mVH= GeV, mBB = MeV
  }

  //-----------------------------------------------
  // Internal weight variations Histograms
  //-----------------------------------------------
  if (m_evtWeightVarMode != -1 && m_currentVar == "Nominal" && m_isMC) {
    EL_CHECK("applyEvtWeights()", apply_EvtWeights());
  }

  if (m_debug) {
    Info("doSystematics()", "End of function doSystematics()");
  }

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader_VHreso1Lep::doBlinding() {
  bool isBlindingRegion =
      (m_vars.isResolved &&
       passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_resolved, {}) &&
       ((m_histNameSvc->get_nTag() == 1) ||
        (m_histNameSvc->get_nTag() == 2))) ||
      (m_vars.isMerged &&
       passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_merged, {}) &&
       ((m_histNameSvc->get_nTag() == 1) || m_histNameSvc->get_nTag() == 2));

  m_doBlinding = isBlindingRegion &&
                 ((m_isMC && m_doBlindingMC) || (!m_isMC && m_doBlindingData));
}

EL::StatusCode AnalysisReader_VHreso1Lep::fillEasyTree() {
  if (m_debug) {
    Info("fillEasyTree()", "Beginning of function fillEasyTree()");
  }
  // fill the ntuple
  if (m_writeObjectsInEasyTree && !m_writeEasyTree) m_writeEasyTree = true;

  if (m_writeEasyTree) {
    // dphi
    double mindPhi1 =
        fabs(m_vars.selectedEventJets.at(0).vec.DeltaPhi(m_vars.metVec));
    double mindPhi2 =
        fabs(m_vars.selectedEventJets.at(1).vec.DeltaPhi(m_vars.metVec));
    double mindPhi3 = 1000;
    if (m_physicsMeta.nJets >= 3)
      mindPhi3 =
          fabs(m_vars.selectedEventJets.at(2).vec.DeltaPhi(m_vars.metVec));
    double mindPhi = std::min(mindPhi1, std::min(mindPhi2, mindPhi3));
    // What is thiiis used for

    // EasyTree reset
    m_etree->Reset();
    m_etree->SetVariation(m_currentVar);

    if ((m_vars.selectedEventJets.size() >= 2 || m_physicsMeta.nFatJet >= 1)) {
      // event info
      m_etree->SetBranchAndValue<std::string>(
          "Description", m_histNameSvc->get_description(), "");
      m_etree->SetBranchAndValue<std::string>(
          "Sample", m_histNameSvc->getFullSample(), "");
      m_etree->SetBranchAndValue<std::string>(
          "EventFlavor", m_histNameSvc->getEventFlavour(), "");
      m_etree->SetBranchAndValue<int>("EventNumber", m_eventInfo->eventNumber(),
                                      -1);
      m_etree->SetBranchAndValue<float>("AverageMu", m_averageMu, -99);
      m_etree->SetBranchAndValue<float>("ActualMu", m_actualMu, -99);
      m_etree->SetBranchAndValue<float>("AverageMuScaled", m_averageMuScaled,
                                        -99);
      m_etree->SetBranchAndValue<float>("ActualMuScaled", m_actualMuScaled,
                                        -99);
      // weights
      m_etree->SetBranchAndValue<float>("EventWeight", m_weight, -1.);
      m_etree->SetBranchAndValue<float>("PUWeight", m_pileupReweight, -1.);
      m_etree->SetBranchAndValue<float>("BTagSF", m_vars.btagWeight, -1.);
      m_etree->SetBranchAndValue<float>("TriggerSF", m_vars.triggerSF, -1.);
      m_etree->SetBranchAndValue<float>("LeptonSF", m_vars.leptonSF, -1.);

      // objects counts
      m_etree->SetBranchAndValue<int>("nJet", m_physicsMeta.nJets, -1);
      m_etree->SetBranchAndValue<int>("nFats", m_physicsMeta.nFatJet, -1);
      m_etree->SetBranchAndValue<int>("nTaus", m_vars.taus.size(), -1);
      m_etree->SetBranchAndValue<int>("nTags", m_physicsMeta.nTags, -1);
      m_etree->SetBranchAndValue<int>("nElectrons", m_vars.nSigEl, -1);
      m_etree->SetBranchAndValue<int>("nMuons", m_vars.nSigMu, -1);

      // event categorisation
      //     m_etree->SetBranchAndValue<int>("m_vars.isMerged", m_vars.isMerged,
      //     -1); m_etree->SetBranchAndValue<int>("m_vars.isResolved",
      //     m_vars.isResolved, -1);
      m_etree->SetBranchAndValue<int>(
          "EventRegime", static_cast<int>(m_physicsMeta.regime), -1);
      m_etree->SetBranchAndValue<unsigned long>(
          "m_vars.eventFlag/l", m_vars.eventFlag,
          0);  // "/l" in the end to determine type
      m_etree->SetBranchAndValue<unsigned long>("eventFlagMerged/l",
                                                m_vars.eventFlag, 0);

      // general 1-lepton quantities
      m_etree->SetBranchAndValue<float>("lPt", m_vars.lepton.vec.Pt() / 1e3,
                                        -1.);
      m_etree->SetBranchAndValue<float>("lEta", m_vars.lepton.vec.Eta(), -10.);
      m_etree->SetBranchAndValue<float>("lPhi", m_vars.lepton.vec.Phi(), -10.);
      m_etree->SetBranchAndValue<float>("lM", m_vars.lepton.vec.M() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("WMt",
                                        m_vars.resolvedVH.VvecT.M() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>(
          "WPt", m_vars.resolvedVH.VvecT.Pt() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("WPhi", m_vars.resolvedVH.VvecT.Phi(),
                                        -10);
      m_etree->SetBranchAndValue<float>("WM", m_vars.resolvedVH.VvecT.M() / 1e3,
                                        -1);
      m_etree->SetBranchAndValue<float>("met", m_vars.metVec.Pt() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("metSig", m_vars.metSig, -1.);
      m_etree->SetBranchAndValue<float>("metSig_soft", m_vars.metSig_soft, -1.);
      m_etree->SetBranchAndValue<float>("metSig_hard", m_vars.metSig_hard, -1.);
      m_etree->SetBranchAndValue<float>("metOverSqrtSumET",
                                        m_vars.metOverSqrtSumET, -1.);
      m_etree->SetBranchAndValue<float>("metOverSqrtHT", m_vars.metOverSqrtHT,
                                        -1.);

      float j1MV2c10 = -1.;
      if (m_vars.selectedEventJets.size() > 0)
        j1MV2c10 = Props::MV2c10.get(m_vars.selectedJets.at(0));
      m_etree->SetBranchAndValue<float>(
          "j1Pt", m_vars.selectedEventJets.at(0).vec_corr.Pt() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>(
          "j1Eta", m_vars.selectedEventJets.at(0).vec_corr.Eta(), -10.);
      m_etree->SetBranchAndValue<float>(
          "j1Phi", m_vars.selectedEventJets.at(0).vec_corr.Phi(), -10.);
      m_etree->SetBranchAndValue<float>(
          "j1M", m_vars.selectedEventJets.at(0).vec_corr.M() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("j1MV2c10", j1MV2c10, -1.);
      m_etree->SetBranchAndValue<int>("j1Flav", m_physicsMeta.b1Flav, -1);
      float j2MV2c10 = -1.;
      if (m_vars.selectedEventJets.size() > 1)
        j2MV2c10 = Props::MV2c10.get(m_vars.selectedJets.at(1));
      m_etree->SetBranchAndValue<float>(
          "j2Pt", m_vars.selectedEventJets.at(1).vec_corr.Pt() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>(
          "j2Eta", m_vars.selectedEventJets.at(1).vec_corr.Eta(), -10.);
      m_etree->SetBranchAndValue<float>(
          "j2Phi", m_vars.selectedEventJets.at(1).vec_corr.Phi(), -10.);
      m_etree->SetBranchAndValue<float>(
          "j2M", m_vars.selectedEventJets.at(1).vec_corr.M() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("j2MV2c10", j2MV2c10, -1.);
      m_etree->SetBranchAndValue<int>("j2Flav", m_physicsMeta.b2Flav, -1);
      m_etree->SetBranchAndValue<float>(
          "jjPt", m_vars.resolvedH.vec_corr.Pt() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>("jjEta",
                                        m_vars.resolvedH.vec_corr.Eta(), -10.);
      m_etree->SetBranchAndValue<float>("jjPhi",
                                        m_vars.resolvedH.vec_corr.Phi(), -10.);
      m_etree->SetBranchAndValue<float>(
          "jjM", m_vars.resolvedH.vec_corr.M() / 1e3, -1.);
      m_etree->SetBranchAndValue<float>(
          "jjdR",
          m_vars.selectedEventJets.at(0).vec_corr.DeltaR(
              m_vars.selectedEventJets.at(1).vec_corr),
          -1.);
      m_etree->SetBranchAndValue<float>(
          "jjdPhi",
          m_vars.selectedEventJets.at(0).vec_corr.DeltaPhi(
              m_vars.selectedEventJets.at(1).vec_corr),
          -10.);
      m_etree->SetBranchAndValue<float>(
          "jjdEta",
          fabs(m_vars.selectedEventJets.at(0).vec_corr.Eta() -
               m_vars.selectedEventJets.at(1).vec_corr.Eta()),
          -1.);
      m_etree->SetBranchAndValue<float>(
          "WjjdPhi",
          fabs(m_vars.resolvedVH.VvecT.DeltaPhi(m_vars.resolvedH.vec_corr)),
          -1.);
      m_etree->SetBranchAndValue<float>(
          "ljmindPhi",
          std::min(fabs(m_vars.lepton.vec.DeltaPhi(
                       m_vars.selectedEventJets.at(0).vec_corr)),
                   fabs(m_vars.lepton.vec.DeltaPhi(
                       m_vars.selectedEventJets.at(1).vec_corr))),
          -1.);
      m_etree->SetBranchAndValue<float>(
          "WjjM", m_vars.resolvedH.vec_corr.M() / 1e3, -1.);

      // 3-jet events
      if (m_vars.selectedEventJets.size() > 2) {
        m_etree->SetBranchAndValue<float>(
            "j3Pt", m_vars.selectedEventJets.at(2).vec_corr.Pt() / 1e3, -1.);
        m_etree->SetBranchAndValue<float>(
            "j3Eta", m_vars.selectedEventJets.at(2).vec_corr.Eta(), -10.);
        m_etree->SetBranchAndValue<float>(
            "j3Phi", m_vars.selectedEventJets.at(2).vec_corr.Phi(), -10.);
        m_etree->SetBranchAndValue<float>(
            "j3M", m_vars.selectedEventJets.at(2).vec_corr.M() / 1e3, -1.);
        m_etree->SetBranchAndValue<float>("jjjM", m_vars.bbjVec_Corr.M() / 1e3,
                                          -1.);
      }

      // fat jets
      if (m_physicsMeta.nFatJet >= 1) {
        int J1nTags = -1;
        if (m_physicsMeta.nFatJet > 0)
          J1nTags = Props::nBTagsAll.get(m_vars.fatJets.at(0));
        m_etree->SetBranchAndValue<float>(
            "J1Pt", m_vars.selectedEventFatJets.at(0).vec_corr.Pt() / 1e3, -1.);
        m_etree->SetBranchAndValue<float>(
            "J1Eta", m_vars.selectedEventFatJets.at(0).vec_corr.Eta(), -10.);
        m_etree->SetBranchAndValue<float>(
            "J1Phi", m_vars.selectedEventFatJets.at(0).vec_corr.Phi(), -10.);
        m_etree->SetBranchAndValue<float>(
            "J1M", m_vars.selectedEventFatJets.at(0).vec_corr.M() / 1e3, -1.);
        m_etree->SetBranchAndValue<int>("J1nTags", J1nTags, -1);
        m_etree->SetBranchAndValue<float>(
            "WJdPhi",
            fabs(m_vars.resolvedVH.VvecT.DeltaPhi(
                m_vars.selectedEventFatJets.at(0).vec_corr)),
            -1.);
        m_etree->SetBranchAndValue<float>(
            "WJM", m_vars.mergedVH.vec_corr.M() / 1e3, -1.);
      }

      if (m_doMJEtree) {  // MJ study variables - save if told to in config file
        float ptCone30 = -5;
        float ptCone20 = -5;
        float topoCone20 = -5;
        if (m_vars.nSigMu == 1) {
          ptCone30 = Props::ptvarcone30.get(m_vars.mu);
          topoCone20 = Props::topoetcone20.get(m_vars.mu);
        } else if (m_vars.nSigEl == 1) {
          ptCone20 = Props::ptvarcone20.get(m_vars.el);
          topoCone20 = Props::topoetcone20.get(m_vars.el);
        }

        m_etree->SetBranchAndValue<float>("topoetcone20", topoCone20, -5);
        m_etree->SetBranchAndValue<float>("ptvarcone30", ptCone30, -5);
        m_etree->SetBranchAndValue<float>("ptvarcone20", ptCone20, -5);
        m_etree->SetBranchAndValue<float>(
            "lmetdPhi", fabs(m_vars.lepton.vec.DeltaPhi(m_vars.metVec)), -99);
        m_etree->SetBranchAndValue<double>("jjmindPhi", mindPhi, -99);
        m_etree->SetBranchAndValue<float>(
            "jmetdPhi",
            fabs(m_vars.selectedEventJets.at(0).vec_corr.DeltaPhi(
                m_vars.metVec)),
            -99);
        m_etree->SetBranchAndValue<float>(
            "jldPhi",
            fabs(m_vars.selectedEventJets.at(0).vec_corr.DeltaPhi(
                m_vars.lepton.vec)),
            -99);
      }

      if (m_writeObjectsInEasyTree) {
        EL_CHECK("fillEasyTree()",
                 AnalysisReader_VHQQ::fillObjectBranches(
                     m_vars.signalJets, m_electrons, m_muons, m_taus, m_met));
      }

      if (m_histNameSvc->get_isNominal() &&
          (passAllCutsUpTo(
               m_vars.eventFlag, OneLeptonCuts::pTVMerged,
               {OneLeptonCuts::pTVMerged, OneLeptonCuts::mbbCorrMerged}) ||
           passAllCutsUpTo(
               m_vars.eventFlag, OneLeptonCuts::pTVResolved,
               {OneLeptonCuts::pTVResolved, OneLeptonCuts::mbbCorrResolved}))) {
        m_etree->Fill();
      }
    }
  }
  if (m_debug) {
    Info("fillEasyTree()", "End of function fillEasyTree()");
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::printEventWeights() {
  // print events for cutflow comparison
  if (m_isMC && m_printEventWeightsForCutFlow &&
      passSpecificCuts(m_vars.eventFlag, m_vars.cuts_common_resolved, {}) &&
      m_histNameSvc->get_isNominal()) {
    printf(
        "Run: %7u  ,  Event: %8llu  -  PU = %5.3f  ,  Lepton = %5.3f  ,  "
        "Trigger = %5.3f ,  BTag = %5.3f\n",
        m_eventInfo->runNumber(), m_eventInfo->eventNumber(), m_pileupReweight,
        m_vars.leptonSF, m_vars.triggerSF, m_vars.btagWeight);
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::fill_1lepCutFlow(std::string regime) {
  // New fill method for 1Lepton cutflow - moved from two versions to one
  // Relies on m_vars.cuts vectors
  // systematics variations are skipped via histNameSvc

  std::string dir = "CutFlow/Nominal/";

  if (m_debug) {
    Info("fill_1lepCutFlow()", "Beginning of function fill_1lepCutFlow()");
  }

  static std::string cutsResolved[16] = {
      "AllCxAOD",     "Trigger",         "LooseLepton",   "SignalLepton",
      "AtLeast2Jets", "AtLeast2SigJets", "Pt45",          "MET",
      "mTW",          "Veto3bjet",       "mbbRestrict",   "mbbCorr",
      "pTV",          "SR_0tag_2pjet",   "SR_1tag_2pjet", "SR_2tag_2pjet"};

  static std::string cutsMerged[14] = {"AllCxAOD",
                                       "Trigger",
                                       "SignalLepton",
                                       "MET",
                                       "AtLeast1FatJet",
                                       "mbbCorr",
                                       "AtLeast2TrackJets",
                                       "mTW",
                                       "pTV",
                                       "mbbRestrictMerged",
                                       "SR_0tag_1pfat0pjets",
                                       "SR_1tag_1pfat0pjets",
                                       "SR_2tag_1pfat0pjets",
                                       "SR_3ptag_1pfat0pjets"};

  if (m_vars.cuts_SR_resolved.size() < length(cutsResolved) ||
      m_vars.cuts_SR_merged.size() < length(cutsMerged)) {
    Info("fill_1lepCutFlow()",
         "Size of resolved/merged hardcoded cuts differ from cuts used! Check "
         "it!");
    return EL::StatusCode::FAILURE;
  }

  // Cuts to exclude from cutflow
  std::vector<unsigned long int> excludeCuts = {
      OneLeptonCuts::mbbRestrictMerged};

  if (regime == "resolved") {
    if (m_debug) Info("fill_1lepCutFlow()", "Resolved Regime");
    // Keep track if one cut is failed
    // Want to have the print-outs for all cuts but only fill Cutflow for passed
    // cuts - does not count for the signal regions
    bool failedCut = false;
    for (unsigned long int icut = 0; icut < m_vars.cuts_SR_resolved.size();
         icut++) {
      // Skip excluded cuts
      if (std::find(excludeCuts.begin(), excludeCuts.end(),
                    m_vars.cuts_SR_resolved.at(icut)) != excludeCuts.end())
        continue;
      if (!passSpecificCuts(m_vars.eventFlag,
                            {m_vars.cuts_SR_resolved.at(icut)}, {})) {
        if (m_debug)
          Info("fill_1lepCutFlow()", ">>>>> %s : %d",
               cutsResolved[icut].c_str(), 0);
        if (icut <= 12) failedCut = true;
        continue;
      } else {
        if (m_debug)
          Info("fill_1lepCutFlow()", ">>>>> %s : %d",
               cutsResolved[icut].c_str(), 1);
      }
      if (!failedCut) {
        std::string label = cutsResolved[icut];
        m_histSvc->BookFillCutHist(dir + "CutsResolved", length(cutsResolved),
                                   cutsResolved, label, m_weight);
        m_histSvc->BookFillCutHist(dir + "CutsResolvedNoWeight",
                                   length(cutsResolved), cutsResolved, label,
                                   1);
      }
    }
  } else if (regime == "merged") {
    if (m_debug) Info("fill_1lepCutFlow()", "Merged Regime");
    // Keep track if one cut is failed
    // Want to have the print-outs for all cuts but only fill Cutflow for passed
    // cuts
    bool failedCut = false;
    for (unsigned long int icut = 0; icut < m_vars.cuts_SR_merged.size();
         icut++) {
      // Skip excluded cuts
      if (std::find(excludeCuts.begin(), excludeCuts.end(),
                    m_vars.cuts_SR_merged.at(icut)) != excludeCuts.end()) {
        continue;
      }
      if (!passSpecificCuts(m_vars.eventFlag, {m_vars.cuts_SR_merged.at(icut)},
                            {})) {
        if (m_debug)
          Info("fill_1lepCutFlow()", ">>>>> %s : %d", cutsMerged[icut].c_str(),
               0);
        if (icut <= 9) failedCut = true;
        continue;
      } else {
        if (m_debug)
          Info("fill_1lepCutFlow()", ">>>>> %s : %d", cutsMerged[icut].c_str(),
               1);
      }
      if (!failedCut) {
        std::string label = cutsMerged[icut];
        m_histSvc->BookFillCutHist(dir + "CutsMerged", length(cutsMerged),
                                   cutsMerged, label, m_weight);
        m_histSvc->BookFillCutHist(dir + "CutsMergedNoWeight",
                                   length(cutsMerged), cutsMerged, label, 1);
      }
    }
  }

  if (m_debug) {
    Info("fill_1lepCutFlow()", "End of function fill_1lepCutFlow()");
  }

  return EL::StatusCode::SUCCESS;
}

bool AnalysisReader_VHreso1Lep::passSpecificCuts(
    const unsigned long int flag, const std::vector<unsigned long int> &cuts,
    const std::vector<unsigned long int> &excludeCuts) {
  // Add 1Lepton specific version to simplify things
  // Require to skip intermediate cuts defined in excludedCuts
  // Only consider cuts defined in cuts

  unsigned long int mask = 0;
  const unsigned long int onBit = 1;
  const unsigned long int offBit = 0;
  // Make the bit mask
  for (unsigned long int cut = 0; cut < cuts.size(); ++cut) {
    if (std::find(excludeCuts.begin(), excludeCuts.end(), cuts.at(cut)) !=
        excludeCuts.end()) {
      // if a cut should be excluded set the corresponding bit to 0
      mask = mask | (offBit << cuts.at(cut));
    } else {
      // otherwise set the bit to 1
      mask = mask | (onBit << cuts.at(cut));
    }
  }

  return (flag & mask) == mask;
}

EL::StatusCode AnalysisReader_VHreso1Lep::doInputPlots() {
  // Could potential sort all histograms better

  if (m_debug) {
    Info("doInputPlots()", "Beginning of function doInputPlots()");
  }
  if (!m_doBlinding && m_doInputPlots) {
    // Determine if we need the _El_ & _Mu_ systematics for the mVH, mBB and
    // MET distributions
    SystLevel ApplySysts =
        m_doIsoInv ? SystLevel::All : SystLevel::CombinedOnly;

    //================== Generic plots for basic input generation
    //===================
    BookFillHist_VHQQ1Lep("pTV", 200, 0, 2000,
                          m_vars.resolvedVH.VvecT.Pt() / 1e3, m_weight,
                          ApplySysts);
    if (!m_doBasicPlots) {  // TODO Are these needed?
      SystLevel ApplyMETSysts = m_doIsoInv ? SystLevel::All : SystLevel::None;
      BookFillHist_VHQQ1Lep("MET", 100, 0, 1000, m_vars.metVec.Pt() / 1e3,
                            m_weight, ApplyMETSysts);
      BookFillHist_VHQQ1Lep("METSig", 300, 0., 30., m_vars.metSig, m_weight,
                            ApplySysts);
      if (m_vars.metSig_PU != -1) {
        BookFillHist_VHQQ1Lep("METSig_PU", 300, 0., 30., m_vars.metSig_PU,
                              m_weight, ApplySysts);
      } else {
        BookFillHist_VHQQ1Lep("METSig_soft", 300, 0., 30., m_vars.metSig_soft,
                              m_weight, ApplySysts);
        BookFillHist_VHQQ1Lep("METSig_hard", 300, 0., 30., m_vars.metSig_hard,
                              m_weight, ApplySysts);
      }
      BookFillHist_VHQQ1Lep("METOverSqrtSumET", 300, 0., 30.,
                            m_vars.metOverSqrtSumET, m_weight, ApplySysts);
      BookFillHist_VHQQ1Lep("METOverSqrtHT", 300, 0., 30., m_vars.metOverSqrtHT,
                            m_weight, ApplySysts);
    }

    //===============================================================================

    // =========== Resolved Regime Inputs Plots ============
    if (m_vars.isResolved) {
      BookFillHist_VHQQ1Lep("mBB", 100, 0, 500, m_vars.HVec.M() / 1e3, m_weight,
                            ApplySysts);
      BookFillHist_VHQQ1Lep("mVH", 600, 0, 6000, m_vars.VHVec.M() / 1e3,
                            m_weight, ApplySysts);
    }
    //======================================================

    //=========== Merged Regime Input Plots ============
    if (m_vars.isMerged) {
      BookFillHist_VHQQ1Lep("mVH", 600, 0, 6000, m_vars.VHVec.M() / 1e3,
                            m_weight, SystLevel::CombinedOnly);
      BookFillHist_VHQQ1Lep("mBB", 100, 0, 500, m_vars.HVec.M() / 1e3, m_weight,
                            SystLevel::CombinedOnly);
    }
    //===================================================
  }

  if (m_debug) {
    Info("doInputPlots()", "End of function doInputPlots()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::doBasicPlots() {
  // Do basic plots

  if (m_debug) {
    Info("doBasicPlots()", "Beginning of function doBasicPlots()");
  }

  if (m_doBasicPlots) {
    // Delta Phi between lepton and MET
    double deltaPhi_LepMET = m_vars.lepton.vec.DeltaPhi(m_vars.metVec);
    BookFillHist_VHQQ1Lep("DeltaPhiLepMet", 160, -4, 4, deltaPhi_LepMET,
                          m_weight);  // replace

    ////Scalar sum of final state pT
    // Derive the Ht value
    double Ht = 0;
    for (auto SigJet : m_vars.signalJets) {
      Ht += SigJet->p4().Pt();
    }
    for (auto ForwardJet : m_vars.forwardJets) {
      Ht += ForwardJet->p4().Pt();
    }
    BookFillHist_VHQQ1Lep("Ht", 500, 0, 5000, Ht / 1e3, m_weight);

    // event quantities
    BookFillHist_VHQQ1Lep("MET", 100, 0, 1000, m_vars.metVec.Pt() / 1e3,
                          m_weight);  // replace
    BookFillHist_VHQQ1Lep("mTW", 100, 0, 500, m_vars.resolvedVH.VvecT.M() / 1e3,
                          m_weight);
    BookFillHist_VHQQ1Lep("METSig", 300, 0., 30., m_vars.metSig, m_weight);
    if (m_vars.metSig_PU != -1) {
      BookFillHist_VHQQ1Lep("METSig_PU", 300, 0., 30., m_vars.metSig_PU,
                            m_weight);
    } else {
      BookFillHist_VHQQ1Lep("METSig_soft", 300, 0., 30., m_vars.metSig_soft,
                            m_weight);
      BookFillHist_VHQQ1Lep("METSig_hard", 300, 0., 30., m_vars.metSig_hard,
                            m_weight);
    }
    BookFillHist_VHQQ1Lep("METOverSqrtSumET", 300, 0., 30.,
                          m_vars.metOverSqrtSumET, m_weight);
    BookFillHist_VHQQ1Lep("METOverSqrtHT", 300, 0., 30., m_vars.metOverSqrtHT,
                          m_weight);
    // Lepton Vector quantities
    fill_TLV("lepVec", m_vars.lepton.vec, m_weight, m_vars.lepton.flav == el,
             m_vars.lepton.flav == mu);

    // Lepton and MET
    m_histSvc->BookFillHist("dPhiLepMET", 200, -3.15, 3.15,
                            m_vars.lepton.vec.DeltaPhi(m_vars.metVec),
                            m_weight);

    // Jet quantities
    if (m_vars.isResolved && !m_doExtendedPlots) {
      fill_TLV("j1VecCorr", m_vars.selectedEventJets.at(0).vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("j2VecCorr", m_vars.selectedEventJets.at(1).vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_vars.selectedEventJets.size() > 2) {
        fill_TLV("j3VecCorr", m_vars.selectedEventJets.at(2).vec_corr, m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      }
    } else if (m_vars.isMerged && !m_doExtendedPlots) {
      fill_TLV("HVecFatCorr", m_vars.selectedEventFatJets.at(0).vec_corr,
               m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_vars.matchedLeadTrackJets.size() >= 1)
        fill_TLV("TrkJet1", m_vars.matchedLeadTrackJets.at(0)->p4(), m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_vars.matchedLeadTrackJets.size() >= 2)
        fill_TLV("TrkJet2", m_vars.matchedLeadTrackJets.at(1)->p4(), m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
    }

    if (!m_doIsoInv) {  // if doIsoInv these plots are part of doInputPlots
      // Plot the ptvarcone and topetcome values for the isolation of the
      // leptons
      if (m_vars.lepton.flav == el) {
        BookFillHist_VHQQ1Lep(
            "ptvarcone20_pT", 100, 0, 0.5,
            Props::ptvarcone20.get(m_vars.el) / m_vars.lepton.vec.Pt(),
            m_weight);
        BookFillHist_VHQQ1Lep("ptvarcone20", 100, 0, 5000,
                              Props::ptvarcone20.get(m_vars.el), m_weight);
        BookFillHist_VHQQ1Lep(
            "topoetcone20_pT", 100, 0, 0.5,
            Props::topoetcone20.get(m_vars.el) / m_vars.lepton.vec.Pt(),
            m_weight);
        BookFillHist_VHQQ1Lep("topoetcone20", 100, 0, 5000,
                              Props::topoetcone20.get(m_vars.el), m_weight);
      }
      if (m_vars.lepton.flav == mu) {
        BookFillHist_VHQQ1Lep(
            "ptvarcone30_pT", 100, 0, 0.5,
            Props::ptvarcone30.get(m_vars.mu) / m_vars.lepton.vec.Pt(),
            m_weight);
        BookFillHist_VHQQ1Lep("ptvarcone30", 100, 0, 5000,
                              Props::ptvarcone30.get(m_vars.mu), m_weight);
        BookFillHist_VHQQ1Lep(
            "topoetcone20_pT", 100, 0, 0.5,
            Props::topoetcone20.get(m_vars.mu) / m_vars.lepton.vec.Pt(),
            m_weight);
        BookFillHist_VHQQ1Lep("topoetcone20", 100, 0, 5000,
                              Props::topoetcone20.get(m_vars.mu), m_weight);
      }
    }
  }
  if (m_debug) {
    Info("doBasicPlots()", "End of function doBasicPlots()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::doExtendedPlots() {
  // Produce an extended number of plots

  if (m_debug) {
    Info("doExtendedPlots()", "Beginning of function doExtendedPlots()");
  }

  if (m_doExtendedPlots) {
    // Plot the average number of  interactions per crossing for PU.
    {
      BookFillHist_VHQQ1Lep("AverMu", 180, 0, 90, m_averageMuScaled, m_weight);
    }

    // B-Tagging Variables for AntiKt4EMtopo dR = 0.4
    if (m_vars.selectedEventJets.size() >= 1) {
      BookFillHist_VHQQ1Lep("MV2c10B1", 100, -1, 1,
                            Props::MV2c10.get(m_vars.selectedJets.at(0)),
                            m_weight);
    }
    if (m_vars.selectedEventJets.size() >= 2) {
      BookFillHist_VHQQ1Lep("MV2c10B2", 100, -1, 1,
                            Props::MV2c10.get(m_vars.selectedJets.at(1)),
                            m_weight);
    }

    if (m_vars.isMerged) {
      // 1D histogram of MV2c00, MV2c10 & MV2c100 for each trackjet
      if (m_vars.matchedLeadTrackJets.size() >= 1) {
        BookFillHist_VHQQ1Lep(
            "TrkMV2c10B1", 100, -1, 1,
            Props::MV2c10.get(m_vars.matchedLeadTrackJets.at(0)), m_weight);
      }
      if (m_vars.matchedLeadTrackJets.size() >= 2) {
        BookFillHist_VHQQ1Lep(
            "TrkMV2c10B2", 100, -1, 1,
            Props::MV2c10.get(m_vars.matchedLeadTrackJets.at(1)), m_weight);
      }
    }

    // dijet system (Higgs)
    if (m_vars.isResolved) {
      // jets
      fill_TLV("j1VecCorr", m_vars.selectedEventJets.at(0).vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("j2VecCorr", m_vars.selectedEventJets.at(1).vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("HVecJetCorr", m_vars.resolvedH.vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      BookFillHist_VHQQ1Lep("dRBB", 100, 0., 6.,
                            m_vars.selectedEventJets.at(0).vec_corr.DeltaR(
                                m_vars.selectedEventJets.at(1).vec_corr),
                            m_weight);
      BookFillHist_VHQQ1Lep(
          "dPhiVBB", 315, 0., 3.15,
          fabs(m_vars.resolvedVH.VvecT.DeltaPhi(m_vars.resolvedH.vec_corr)),
          m_weight);

      BookFillHist_VHQQ1Lep("dPhiBB", 200, -3.15, 3.15,
                            m_vars.selectedEventJets.at(0).vec_corr.DeltaPhi(
                                m_vars.selectedEventJets.at(1).vec_corr),
                            m_weight);
      BookFillHist_VHQQ1Lep("dEtaBB", 100, 0., 6.,
                            fabs(m_vars.selectedEventJets.at(0).vec_corr.Eta() -
                                 m_vars.selectedEventJets.at(1).vec_corr.Eta()),
                            m_weight);
      BookFillHist_VHQQ1Lep(
          "dPhiLBmin", 100, 0., 6.,
          std::min(fabs(m_vars.lepton.vec.DeltaPhi(
                       m_vars.selectedEventJets.at(0).vec_corr)),
                   fabs(m_vars.lepton.vec.DeltaPhi(
                       m_vars.selectedEventJets.at(1).vec_corr))),
          m_weight);

      if (m_vars.selectedEventJets.size() >= 1)
        BookFillHist_VHQQ1Lep("MV2c10B1", 100, -1, 1,
                              Props::MV2c10.get(m_vars.selectedJets.at(0)),
                              m_weight);
      if (m_vars.selectedEventJets.size() >= 2)
        BookFillHist_VHQQ1Lep("MV2c10B2", 100, -1, 1,
                              Props::MV2c10.get(m_vars.selectedJets.at(1)),
                              m_weight);

      // 2D Histos
      m_histSvc->BookFillHist(
          "dPhiBB_dEtaBB", 100, 0., 3.15, 100, 0., 6.,
          fabs(m_vars.selectedEventJets.at(0).vec_corr.DeltaPhi(
              m_vars.selectedEventJets.at(1).vec_corr)),
          fabs(m_vars.selectedEventJets.at(0).vec_corr.Eta() -
               m_vars.selectedEventJets.at(1).vec_corr.Eta()),
          m_weight);
      m_histSvc->BookFillHist(
          "dPhiBB_dRBB", 100, 0., 3.15, 100, 0., 6.,
          fabs(m_vars.selectedEventJets.at(0).vec_corr.DeltaPhi(
              m_vars.selectedEventJets.at(1).vec_corr)),
          m_vars.selectedEventJets.at(0).vec_corr.DeltaR(
              m_vars.selectedEventJets.at(1).vec_corr),
          m_weight);
      m_histSvc->BookFillHist(
          "dEtaBB_dRBB", 100, 0., 6., 100, 0., 6.,
          fabs(m_vars.selectedEventJets.at(0).vec_corr.Eta() -
               m_vars.selectedEventJets.at(1).vec_corr.Eta()),
          m_vars.selectedEventJets.at(0).vec_corr.DeltaR(
              m_vars.selectedEventJets.at(1).vec_corr),
          m_weight);

      // pTB1 and pTB2, redundant, this information is already stored by the
      // lines above under a different name BookFillHist_VHQQ1Lep("pTB1",
      // 100,  0,  500,  j1VecCorr.Pt()/1e3, m_weight);
      // BookFillHist_VHQQ1Lep("pTB2", 100,  0,  500,  j2VecCorr.Pt()/1e3,
      // m_weight);

      // 3rd jet variables
      if (m_vars.selectedEventJets.size() > 2) {
        BookFillHist_VHQQ1Lep(
            "pTJ3", 500, 0, 500,
            m_vars.selectedEventJets.at(2).vec_corr.Pt() / 1e3, m_weight);
        BookFillHist_VHQQ1Lep("mBBJ", 1000, 0, 1000,
                              m_vars.bbjVec_Corr.M() / 1e3, m_weight);
      }
    }
    // fat jet (Higgs)
    if (m_vars.isMerged) {
      fill_TLV("HVecFatCorr", m_vars.selectedEventFatJets.at(0).vec_corr,
               m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      BookFillHist_VHQQ1Lep("dPhiVFatCorr", 100, 0., 3.15,
                            fabs(m_vars.resolvedVH.VvecT.DeltaPhi(
                                m_vars.selectedEventFatJets.at(0).vec_corr)),
                            m_weight);
      BookFillHist_VHQQ1Lep("dPhiLFatCorr", 100, 0., 3.15,
                            fabs(m_vars.lepton.vec.DeltaPhi(
                                m_vars.selectedEventFatJets.at(0).vec_corr)),
                            m_weight);
    }

    // W system
  }

  if (m_debug) {
    Info("doBasicPlots()", "End of function doBasicPlots()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::doTLVPlots() {
  // Produce plots from all TLorentzVectors
  if (m_debug) {
    Info("doTLVPlots()", "Beginning of function doTLVPlots()");
  }

  if (m_doTLVPlots) {
    if (m_vars.isResolved) {
      fill_TLV("fwdjet1Vec", (TLorentzVector)m_vars.forwardJets.at(0)->p4(),
               m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("fwdjet2Vec", (TLorentzVector)m_vars.forwardJets.at(1)->p4(),
               m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_physicsMeta.nForwardJet > 2)
        fill_TLV("fwdjet3Vec", (TLorentzVector)m_vars.forwardJets.at(2)->p4(),
                 m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

      fill_TLV("sigjet1Vec", (TLorentzVector)m_vars.signalJets.at(0)->p4(),
               m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("sigjet2Vec", (TLorentzVector)m_vars.signalJets.at(1)->p4(),
               m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_physicsMeta.nSigJet > 2)
        fill_TLV("sigjet3Vec", (TLorentzVector)m_vars.signalJets.at(2)->p4(),
                 m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

      fill_TLV("j1VecSel", m_vars.selectedEventJets.at(0).vec, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("j2VecSel", m_vars.selectedEventJets.at(1).vec, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_vars.selectedEventJets.size() > 2) {
        fill_TLV("j3VecSel", m_vars.selectedEventJets.at(2).vec, m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
        fill_TLV("bbjVec", m_vars.bbjVec, m_weight, m_vars.lepton.flav == el,
                 m_vars.lepton.flav == mu);

        fill_TLV("j3VecCorr", m_vars.selectedEventJets.at(2).vec_corr, m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
        fill_TLV("bbjVecCorr", m_vars.bbjVec_Corr, m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      }
      fill_TLV("j1VecRW", m_vars.selectedEventJets.at(0).vec_resc, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("j2VecRW", m_vars.selectedEventJets.at(1).vec_resc, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

      fill_TLV("HVecJet", m_vars.resolvedH.vec, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("HVecJetRW", m_vars.resolvedH.vec_resc, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

      fill_TLV("VHVecJet", m_vars.resolvedVH.vec, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("VHVecJetCorr", m_vars.resolvedVH.vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("VHVecJetRW", m_vars.resolvedVH.vec_resc, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
    }

    if (m_vars.isMerged) {
      fill_TLV("fatjet1Vec", m_vars.selectedEventFatJets.at(0).vec, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_physicsMeta.nFatJet > 1)
        fill_TLV("fatjet2Vec", m_vars.selectedEventFatJets.at(1).vec, m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_physicsMeta.nFatJet > 2)
        fill_TLV("fatjet3Vec", m_vars.selectedEventFatJets.at(2).vec, m_weight,
                 m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

      if (m_physicsMeta.nFatJet > 1)
        fill_TLV("fj2VecCorr", m_vars.selectedEventFatJets.at(1).vec_corr,
                 m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      if (m_physicsMeta.nFatJet > 2)
        fill_TLV("fj3VecCorr", m_vars.selectedEventFatJets.at(2).vec_corr,
                 m_weight, m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

      fill_TLV("fatJetRW", m_vars.selectedEventFatJets.at(0).vec_resc, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

      fill_TLV("VHVecFat", m_vars.mergedVH.vec, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("VHVecFatCorr", m_vars.mergedVH.vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
      fill_TLV("VHVecFatRW", m_vars.mergedVH.vec_resc, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
    }

    fill_TLV("metVec", m_vars.metVec, m_weight, m_vars.lepton.flav == el,
             m_vars.lepton.flav == mu);
    if (m_vars.isResolved)
      fill_TLV("metVecJetRW", m_vars.resolvedMET.vec_corr, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
    if (m_vars.isMerged)
      fill_TLV("metVecFJRW", m_vars.mergedMET.vec_resc, m_weight,
               m_vars.lepton.flav == el, m_vars.lepton.flav == mu);

    fill_TLV("lepVecT", m_vars.lepton.vecT, m_weight, m_vars.lepton.flav == el,
             m_vars.lepton.flav == mu);
    fill_TLV("WVecT", m_vars.resolvedVH.VvecT, m_weight,
             m_vars.lepton.flav == el, m_vars.lepton.flav == mu);
  }

  if (m_debug) {
    Info("doTLVPlots()", "End of function doTLVPlots()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::doSystPlots() {
  // redo the mBB after applying systematics
  if (m_debug) {
    Info("doSysPlots()", "Beginning of function doSysPlots()");
  }
  if (m_isMC &&
      ((m_csCorrections.size() != 0) || (m_csVariations.size() != 0))) {
    if (m_vars.isResolved) {
      applySystematics(
          m_vars.resolvedVH.VvecT.Pt(), m_vars.metVec.Pt(), m_physicsMeta.nJets,
          m_physicsMeta.nTags, m_vars.selectedEventJets.at(0).vec_corr,
          m_vars.selectedEventJets.at(1).vec_corr, true, m_vars.Mtop);
    }

    BookFillHist_VHQQ1Lep("mBBRedo", 100, 0, 500,
                          m_vars.resolvedH.vec_corr.M() / 1e3, m_weight,
                          SystLevel::CombinedOnly);
  }

  if (m_debug) {
    Info("doSysPlots()", "End of function doSysPlots()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::computeVHCandidate() {
  // Construct W from neutrino + lepton vectors
  // Creates the VH Candidate from input Higgs higgs and W

  if (m_debug) {
    Info("computeVHCandidate()", "Beginning of function computeVHCandidate()");
  }

  // **Definition** : neutrino using W mass constraint
  TLorentzVector nuVec =
      getNeutrinoTLV(m_vars.resolvedMET.vec, m_vars.lepton.vec, true);
  TLorentzVector nuVecJetCorr =
      getNeutrinoTLV(m_vars.resolvedMET.vec_corr, m_vars.lepton.vec, true);
  TLorentzVector nuVecJetRW =
      getNeutrinoTLV(m_vars.resolvedMET.vec_resc, m_vars.lepton.vec, true);
  TLorentzVector nuVecFJCorr =
      getNeutrinoTLV(m_vars.mergedMET.vec_corr, m_vars.lepton.vec, true);
  TLorentzVector nuVecFJRW =
      getNeutrinoTLV(m_vars.mergedMET.vec_resc, m_vars.lepton.vec, true);

  // Make the W candidates
  TLorentzVector WVecT = m_vars.lepton.vecT + m_vars.resolvedMET.vec;
  TLorentzVector WVec = m_vars.lepton.vec + nuVec;
  TLorentzVector WVecJetCorr = m_vars.lepton.vec + nuVecJetCorr;
  TLorentzVector WVecJetRW = m_vars.lepton.vec + nuVecJetRW;
  TLorentzVector WVecFJCorr = m_vars.lepton.vec + nuVecFJCorr;
  TLorentzVector WVecFJRW = m_vars.lepton.vec + nuVecFJRW;

  VHCand resolvedVH;
  VHCand mergedVH;
  // **Definition** : VH resonance
  if (m_vars.selectedEventJets.size() >= 2) {
    resolvedVH.vec = WVec + m_vars.resolvedH.vec;
    resolvedVH.vec_corr = WVecJetCorr + m_vars.resolvedH.vec_corr;
    resolvedVH.vec_resc = WVecJetRW + m_vars.resolvedH.vec_resc;
  }
  if (m_vars.selectedEventFatJets.size() >= 1) {
    mergedVH.vec = WVec + m_vars.mergedH.vec;
    mergedVH.vec_corr = WVecFJCorr + m_vars.mergedH.vec_corr;
    mergedVH.vec_resc = WVecFJRW + m_vars.mergedH.vec_resc;
    mergedVH.Vvec = WVecFJCorr;
  }

  resolvedVH.Vvec = WVec;
  resolvedVH.VvecT = WVecT;
  mergedVH.Vvec = WVec;
  mergedVH.VvecT = WVecT;

  m_vars.resolvedVH = resolvedVH;
  m_vars.mergedVH = mergedVH;

  if (m_debug) {
    Info("computeVHCandidate()", "End of function computeVHCandidate()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::printCuts() {
  // Loops through all the cuts for resolved and merged
  // Prints whether a cut is passed or not

  if (m_debug) {
    Info("printCuts()", "Beginning of function printCuts()");
  } else {
    return EL::StatusCode::SUCCESS;
  }

  static std::string cutsResolved[16] = {
      "AllCxAOD",     "Trigger",         "LooseLepton",   "SignalLepton",
      "AtLeast2Jets", "AtLeast2SigJets", "Pt45",          "MET",
      "mTW",          "Veto3bjet",       "mbbRestrict",   "mbbCorr",
      "pTV",          "SR_0tag_2pjet",   "SR_1tag_2pjet", "SR_2tag_2pjet"};

  static std::string cutsMerged[14] = {"AllCxAOD",
                                       "Trigger",
                                       "SignalLepton",
                                       "MET",
                                       "AtLeast1FatJet",
                                       "mbbCorr",
                                       "AtLeast2TrackJets",
                                       "mTW",
                                       "pTV",
                                       "mbbRestrictMerged",
                                       "SR_0tag_1pfat0pjets",
                                       "SR_1tag_1pfat0pjets",
                                       "SR_2tag_1pfat0pjets",
                                       "SR_3ptag_1pfat0pjets"};

  std::cout << "--------------- Resolved Cuts---------------" << std::endl;
  for (unsigned long int icut = 0; icut < m_vars.cuts_common_resolved.size();
       icut++) {
    std::cout << cutsResolved[icut] << ": "
              << passSpecificCuts(m_vars.eventFlag,
                                  {m_vars.cuts_common_resolved.at(icut)}, {})
              << std::endl;
  }
  std::cout << "--------------- Merged Cuts---------------" << std::endl;
  for (unsigned long int icut = 0; icut < m_vars.cuts_common_merged.size();
       icut++) {
    std::cout << cutsMerged[icut] << ": "
              << passSpecificCuts(m_vars.eventFlag,
                                  {m_vars.cuts_common_merged.at(icut)}, {})
              << std::endl;
  }
  std::cout << "------------------------------------------" << std::endl;

  if (m_debug) {
    Info("printCuts()", "End of function printCuts()");
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader_VHreso1Lep::printStats() {
  // Print Stats of the main analysis variable to provide debug output
  // Check if data is blinded
  if (!m_debug || m_doBlinding) return EL::StatusCode::SUCCESS;
  // Print Resolved Statistics
  std::cout << "--------------- Resolved -------------------" << std::endl;
  std::cout << "Selected Jets: " << std::endl;
  for (unsigned int ijet = 0; ijet < 3; ijet++) {
    if (m_vars.selectedEventJets.size() >= ijet + 1) {
      std::cout << "Jet " << ijet + 1 << ":   "
                << printVector(m_vars.selectedEventJets.at(ijet).vec)
                << std::endl;
      std::cout << "Corrected Jet " << ijet + 1 << ":   "
                << printVector(m_vars.selectedEventJets.at(ijet).vec_corr)
                << std::endl;
      std::cout << "Rescaled Jet " << ijet + 1 << ":    "
                << printVector(m_vars.selectedEventJets.at(ijet).vec_resc)
                << std::endl;
    }
  }
  std::cout << "Lepton: " << std::endl;
  std::cout << "Lepton:   " << printVector(m_vars.lepton.vec) << std::endl;
  std::cout << "MET:" << std::endl;
  std::cout << "MET    " << printVector(m_vars.resolvedMET.vec) << std::endl;
  std::cout << "Corrected MET    " << printVector(m_vars.resolvedMET.vec_corr)
            << std::endl;
  std::cout << "Rescaled MET    " << printVector(m_vars.resolvedMET.vec_resc)
            << std::endl;
  std::cout << "Higgs: " << std::endl;
  std::cout << "Higgs    " << printVector(m_vars.resolvedH.vec) << std::endl;
  std::cout << "Corrected Higgs   " << printVector(m_vars.resolvedH.vec_corr)
            << std::endl;
  std::cout << "Resolved Higgs   " << printVector(m_vars.resolvedH.vec_resc)
            << std::endl;
  std::cout << "VH: " << std::endl;
  std::cout << "VH    " << printVector(m_vars.resolvedVH.vec) << std::endl;
  std::cout << "Corrected VH   " << printVector(m_vars.resolvedVH.vec_corr)
            << std::endl;
  std::cout << "Resolved VH   " << printVector(m_vars.resolvedVH.vec_resc)
            << std::endl;

  std::cout << "--------------- Merged -----------------" << std::endl;
  std::cout << "Fat Jets: " << std::endl;
  for (unsigned int ijet = 0; ijet < 3; ijet++) {
    if (m_vars.selectedEventFatJets.size() >= ijet + 1) {
      std::cout << "Fat Jet " << ijet + 1 << ":   "
                << printVector(m_vars.selectedEventFatJets.at(ijet).vec)
                << std::endl;
      std::cout << "Corrected Fat Jet " << ijet + 1 << ":   "
                << printVector(m_vars.selectedEventFatJets.at(ijet).vec_corr)
                << std::endl;
      std::cout << "Rescaled Fat Jet " << ijet + 1 << ":    "
                << printVector(m_vars.selectedEventFatJets.at(ijet).vec_resc)
                << std::endl;
    }
  }
  std::cout << "Lepton: " << std::endl;
  std::cout << "Lepton:   " << printVector(m_vars.lepton.vec) << std::endl;
  std::cout << "MET:" << std::endl;
  std::cout << "MET    " << printVector(m_vars.mergedMET.vec) << std::endl;
  std::cout << "Corrected MET    " << printVector(m_vars.mergedMET.vec_corr)
            << std::endl;
  std::cout << "Rescaled MET    " << printVector(m_vars.mergedMET.vec_resc)
            << std::endl;
  std::cout << "Higgs: " << std::endl;
  std::cout << "Higgs    " << printVector(m_vars.mergedH.vec) << std::endl;
  std::cout << "Corrected Higgs   " << printVector(m_vars.mergedH.vec_corr)
            << std::endl;
  std::cout << "Resolved Higgs   " << printVector(m_vars.mergedH.vec_resc)
            << std::endl;
  std::cout << "VH: " << std::endl;
  std::cout << "VH    " << printVector(m_vars.mergedVH.vec) << std::endl;
  std::cout << "Corrected VH   " << printVector(m_vars.mergedVH.vec_corr)
            << std::endl;
  std::cout << "Resolved VH   " << printVector(m_vars.mergedVH.vec_resc)
            << std::endl;
  std::cout << "------------------------------------------" << std::endl;

  return EL::StatusCode::SUCCESS;
}

std::string AnalysisReader_VHreso1Lep::printVector(TLorentzVector v) {
  // Prints useful information from the tlorentzvectors
  // Different formate than the standard "root" Print()
  char buffer[100];
  int n = sprintf(buffer, "(Pt,eta,phi,M) = (%f,%f,%f,%f)", v.Pt(), v.Eta(),
                  v.Phi(), v.M());
  if (n == 0) return "";
  std::string output = buffer;
  return output;
}
