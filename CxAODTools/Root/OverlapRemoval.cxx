// OverlapRemovalToolLargeR and OverlapRemovalToolVBFGamma are not available in this branch !!!!
#include "CxAODTools/OverlapRemoval.h"
#include "AssociationUtils/OverlapRemovalInit.h"
#include "AssociationUtils/OverlapRemovalTool.h"
//#include "CxAODTools/OverlapRemovalToolLargeR.h"
//#include "CxAODTools/OverlapRemovalToolVBFGamma.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/PhJetOverlapTool.h"
#include "AssociationUtils/MuJetOverlapTool.h"
#include "CxxUtils/make_unique.h"
#include "AssociationUtils/DeltaROverlapTool.h"
#include "CxAODTools/InvDeltaROverlapTool.h"

OverlapRemoval::OverlapRemoval(ConfigStore & config) :
  m_config(config),
  m_applyOverlapRemoval(true),
  m_applyORlargeR(false),
  m_doJetLargeJetOR(false),
  m_applyVBFGammaOR(false),
  m_applyNewToolOR(true),
  m_applyHHbbtautauOR(false),
  m_useDeltaROR(false),
  m_useBTagOR(false),
  m_useTauOR(true),
  m_useTauJetOR(true),
  m_useSetORInputLables(true),
  m_useEleEleOR(true),
  m_doMuPFJetOR(true),
  m_useVarConeEleJetOR(false),
  m_useVarConeMuJetOR(true),
  m_dRMatcher(nullptr)
  //m_overlapRemovalTool(nullptr),
  //m_overlapRemovalToolVBFGamma(nullptr),
{
  //from config
  //------------
  m_config.getif<bool>("debug", m_debug);
  m_config.getif<bool>("applyOverlapRemoval", m_applyOverlapRemoval);
  m_config.getif<bool>("applyOverlapRemovalLargeR", m_applyORlargeR);
  m_config.getif<bool>("doJetLargeJetOR", m_doJetLargeJetOR);
  m_config.getif<bool>("applyNewToolOR", m_applyNewToolOR);
  m_config.getif<bool>("applyVBFGammaOR", m_applyVBFGammaOR);
  m_config.getif<bool>("useDeltaROR",m_useDeltaROR);
  m_config.getif<bool>("useBTagOR",m_useBTagOR);
  m_config.getif<bool>("useTauOR",m_useTauOR);
  m_config.getif<bool>("useTauJetOR",m_useTauJetOR);
  m_config.getif<bool>("useSetORInputLables",m_useSetORInputLables);
  m_config.getif<bool>("applyHHbbtautauOR",m_applyHHbbtautauOR);
  m_config.getif<bool>("useEleEleOR",m_useEleEleOR);
  m_config.getif<bool>("doMuPFJetOR",m_doMuPFJetOR);
  m_config.getif<bool>("useVarConeEleJetOR",m_useVarConeEleJetOR);
  m_config.getif<bool>("useVarConeMuJetOR",m_useVarConeMuJetOR);

}
  
EL::StatusCode OverlapRemoval::initialize()
{

  std::string isBTag = "";
  if(m_useBTagOR) isBTag = "isBTag";

  // new overlap removal tool
  // ------------------------
  //ORFlags
  ORUtils::ORFlags orFlags("OverlapRemovalTool","ORInputLabel","OROutputLabel");
  orFlags.bJetLabel = isBTag;
  //orFlags.boostedLeptons = m_applyVarOR; // Variable-cone for lep-jet OR is handled by 'm_useVarConeEleJetOR' and 'm_useVarConeMuJetOR'
  orFlags.outputPassValue = false;
  orFlags.doTaus = m_useTauOR;
  orFlags.doPhotons = m_applyVBFGammaOR; //orFlags.doPhotons = false;
  orFlags.doFatJets = m_applyORlargeR;
  orFlags.doEleEleOR = m_useEleEleOR;
  orFlags.doMuPFJetOR = m_doMuPFJetOR;
  //ToolBox initialization
  TOOL_CHECK("OverlapRemoval::initialize()", ORUtils::recommendedTools(orFlags, m_toolBox) );
  //Override Mu-Jet ORT propeties
  // "ApplyRelPt = false" in defalut, we'd like to set it true
  TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.muJetORT.setProperty("ApplyRelPt", true) );
  // use delta R overlap removal
  if(m_useDeltaROR) TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.muJetORT.setProperty("UseGhostAssociation", false) );
  
  //Override Ele-fJet, Jet-fJet, Tau-jet OR property
  if( m_applyORlargeR ){
	// Ele-FatJet OR property
  	TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.eleFatJetORT.setProperty("DR", 1.2) );
	// Jet-FatJet OR property
  	if( !m_doJetLargeJetOR ){
		//Disabled
		TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.jetFatJetORT.setProperty("DR", 0.) );
	}else{
		TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.jetFatJetORT.setProperty("DR", 1.4) );
	}
  }
  if( m_useTauOR && !m_useTauJetOR & m_applyNewToolOR) TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.tauJetORT.setProperty("DR", 0.0) );
  // Variable cone option for lepton-jet OR 
  TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.eleJetORT.setProperty("UseSlidingDR", m_useVarConeEleJetOR) );
  TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.muJetORT.setProperty("UseSlidingDR", m_useVarConeMuJetOR) );

  // Initialize OverlapRemoval tool 
  TOOL_CHECK("OverlapRemoval::initialize()", m_toolBox.initialize());

  // Lepton Only OR tool for b-jet corrections
  //ORFlags
  ORUtils::ORFlags orFlagsBjetCorr("OverlapRemovalToolLepOnly","LepOnlyORInputLabel","LepOnlyOROutputLabel");
  orFlagsBjetCorr.bJetLabel = isBTag;
  //orFlagsBjetCorr.boostedLeptons = m_applyVarOR; // Variable-cone for lep-jet OR is handled by 'm_useVarConeEleJetOR' and 'm_useVarConeMuJetOR'
  orFlagsBjetCorr.outputPassValue = false;
  orFlagsBjetCorr.doTaus = false;
  orFlagsBjetCorr.doPhotons = false; //orFlagsBjetCorr.doPhotons = false;
  orFlagsBjetCorr.doFatJets = false;
  orFlagsBjetCorr.doEleEleOR = m_useEleEleOR;
  orFlagsBjetCorr.doMuPFJetOR = m_doMuPFJetOR;
  orFlagsBjetCorr.doJets = false;
  //ToolBox initialization
  TOOL_CHECK("OverlapRemoval::initialize()", ORUtils::recommendedTools(orFlagsBjetCorr, m_toolBoxLepOnly) );
  TOOL_CHECK("OverlapRemoval::initialize()", m_toolBoxLepOnly.initialize());

/*
  if(!m_applyVBFGammaOR && !m_applyNewToolOR){
  //initialize overlap removal tool
  //-------------------------------
  // OverlapRemovalToolLargeR extends OverlapRemovalTool, does not modify existing functionality
  m_overlapRemovalTool = new OverlapRemovalToolLargeR("OverlapRemovalTool");
  m_overlapRemovalTool->msg().setLevel(MSG::INFO);
  if(m_debug) m_overlapRemovalTool->msg().setLevel(MSG::DEBUG);
  // Set name of decoration which tells the OR tool which objects to consider
  // if set to "false", the object automatically passes OR, i.e. the OROutputLabel is 0 (as in "do not remove this object").
  TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->setProperty("InputLabel", "ORInputLabel"));
  // Set name of decoration the OR tool will add as decoration
  // if decoration is "false" the object should be removed, while if "true" the object should be kept.
  TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->setProperty("OverlapLabel", "OROutputLabel"));
  TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->setProperty("TauElectronOverlapID", "ORInputLabel"));
  // ElectronJetDRCone = 0.4 -> only jets removed as in run1 VHbb
  //TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->setProperty("ElectronJetDRCone", 0.4));
  //this needs to be done in the CxAODMaker 
  bool useFlagForEleMuOR = false;
  m_config.getif<bool>("useFlagForEleMuOR",useFlagForEleMuOR);
  bool writeFlagForEleMuOR = !m_applyNewToolOR; // apply only with the legacy overlap removal tool
  if(writeFlagForEleMuOR && !useFlagForEleMuOR) TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->setProperty("WriteSharedTrackFlag", true));
  //this needs to be used in the CxAODReader
  if(useFlagForEleMuOR) TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->setProperty("UseSharedTrackFlag", true));

  if (m_applyORlargeR) TOOL_CHECK("OverlapRemoval::initialize()",((OverlapRemovalToolLargeR*) m_overlapRemovalTool)->setProperty("doJetLargeJetOR", m_doJetLargeJetOR));

  if (m_useTauOR&&!m_useTauJetOR) TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->setProperty("TauJetDRCone", 0));
 
  TOOL_CHECK("OverlapRemoval::initialize()", m_overlapRemovalTool->initialize());
  }
*/

  // initialise DeltaRMatcher for HHbbtautau OR
  double dR = 0.4;
  bool useRapidity = true;
  m_dRMatcher = new ORUtils::DeltaRMatcher(dR,useRapidity);
  
  return EL::StatusCode::SUCCESS;
}

OverlapRemoval::~OverlapRemoval()
{
  //delete m_overlapRemovalTool;
  //delete m_overlapRemovalToolVBFGamma;
  delete m_dRMatcher;
  
}

void OverlapRemoval::setORInputLabels(const xAOD::ElectronContainer* electrons,
                                             const xAOD::PhotonContainer* photons,
					     const xAOD::MuonContainer* muons,
					     const xAOD::TauJetContainer* taus,
                                             const xAOD::JetContainer* jets,
                                             const xAOD::JetContainer* fatjets,
                                             const xAOD::DiTauJetContainer* ditaus)
{

  // Set input flag for the OR tool (IntProp::ORInputLabel)  
  if (electrons) {
    for (const xAOD::Electron* elec : *electrons) {
      Props::ORInputLabel.set(elec,Props::passPreSel.get(elec));
      if(m_applyVBFGammaOR)
	Props::ORInputLabel.set(elec, Props::isWHSignalElectron.get(elec));// && elec->auxdata<char>("Tight"));
    }
  }

  if (muons) {
    for (const xAOD::Muon* muon : *muons) {
      Props::ORInputLabel.set(muon,Props::passPreSel.get(muon));
      if(m_applyVBFGammaOR)
	Props::ORInputLabel.set(muon, Props::isWHSignalMuon.get(muon));
    }
  }
  
  if (jets) {
    for (const xAOD::Jet* jet : *jets) {
      Props::ORInputLabel.set(jet,Props::passPreSel.get(jet));
      if(m_applyVBFGammaOR)
	Props::ORInputLabel.set(jet, Props::isVetoJet.get(jet));
    }
  }
  
  if (fatjets) {
    for (const xAOD::Jet* jet : *fatjets) {
      Props::ORInputLabel.set(jet,Props::passPreSel.get(jet));
    }
  }
  
  if (taus) {
    for (const xAOD::TauJet* tau : *taus) {
      Props::ORInputLabel.set(tau,Props::passPreSel.get(tau));
    }
  }

  if (ditaus) {
    for (const xAOD::DiTauJet* ditau : *ditaus) {
      Props::ORInputLabel.set(ditau,Props::passPreSel.get(ditau));
    }
  }
  
  if (photons) {
    for (const xAOD::Photon* photon : *photons) {
      Props::ORInputLabel.set(photon, Props::isVBFSignalPhoton.get(photon));
    }
  }
}


void OverlapRemoval::setLepOnlyORInputLabels(const xAOD::ElectronContainer* electrons,
                                             const xAOD::PhotonContainer* photons,
					     const xAOD::MuonContainer* muons,
					     const xAOD::TauJetContainer* taus,
                                             const xAOD::JetContainer* jets,
                                             const xAOD::JetContainer* fatjets,
                                             const xAOD::DiTauJetContainer* ditaus)
{

  // Set input flag for the OR tool (IntProp::LepOnlyORInputLabel)  
  if (electrons) {
    for (const xAOD::Electron* elec : *electrons) {
      Props::LepOnlyORInputLabel.set(elec,true);
    }
  }

  if (muons) {
    for (const xAOD::Muon* muon : *muons) {
      Props::LepOnlyORInputLabel.set(muon,true);
    }
  }
  
  if (jets) {
    for (const xAOD::Jet* jet : *jets) {
      Props::LepOnlyORInputLabel.set(jet,true);
    }
  }
  
  if (fatjets) {
    for (const xAOD::Jet* jet : *fatjets) {
      Props::LepOnlyORInputLabel.set(jet,true);
    }
  }
  
  if (taus) {
    for (const xAOD::TauJet* tau : *taus) {
      Props::LepOnlyORInputLabel.set(tau,true);
    }
  }

  if (ditaus) {
    for (const xAOD::DiTauJet* ditau : *ditaus) {
      Props::LepOnlyORInputLabel.set(ditau,true);
    }
  }
  
  if (photons) {
    for (const xAOD::Photon* photon : *photons) {
      Props::LepOnlyORInputLabel.set(photon, true);
    }
  }

}

EL::StatusCode OverlapRemoval::removeOverlap(const xAOD::ElectronContainer* electrons,
                                             const xAOD::PhotonContainer* photons,
					     const xAOD::MuonContainer* muons,
					     const xAOD::TauJetContainer* taus,
                                             const xAOD::JetContainer* jets,
                                             const xAOD::JetContainer* fatjets,
                                             const xAOD::DiTauJetContainer* ditaus)
{

  if(m_useSetORInputLables){
    setORInputLabels(electrons,  photons, muons,  taus, jets, fatjets, ditaus);
    setLepOnlyORInputLabels(electrons,  photons, muons,  taus, jets, fatjets, ditaus);
  }
  
  // taus and photons can be nulptrs - do not apply photon OR, but use method below!
  // electrons, muons and jets need to be valid pointers
  if ( electrons && muons && jets ) {
/*
  if (!m_applyVBFGammaOR && !m_applyHHbbtautauOR && !m_applyNewToolOR) {
    if (!m_overlapRemovalTool) {
      Error("OverlapRemoval::removeOverlap()", "OverlapRemovalTool not initialized!");
      return EL::StatusCode::FAILURE;
    }
    if(m_useTauOR) {      
      if (m_applyVarOR) {
	EL_CHECK("OverlapRemoval::VariableConeOR()", VariableConeOR(electrons, muons, jets, taus, photons));
      }
      else {
	EL_CHECK("OverlapRemoval::removeOverlap()",m_overlapRemovalTool->removeOverlaps(electrons, muons, jets, taus, photons));
      }
    }
    else {
      if (m_applyVarOR) {
	EL_CHECK("OverlapRemoval::VariableConeOR()", VariableConeOR(electrons, muons, jets, nullptr, photons));
      }
      else {
	EL_CHECK("OverlapRemoval::removeOverlap()",m_overlapRemovalTool->removeOverlaps(electrons, muons, jets, nullptr,  photons));
      }
      // force taus to pass
      if (taus) {
        for( const xAOD::TauJet * tau : *taus) {
	  Props::OROutputLabel.set(tau,false);
        }
      }
    }

    if (m_applyORlargeR) {
      // apply large-R overlap removal
      EL_CHECK("OverlapRemoval::removeOverlap()",
              ((OverlapRemovalToolLargeR*) m_overlapRemovalTool)->removeOverlapsLargeR(
              electrons, nullptr, jets, fatjets, nullptr, nullptr));
    } else {
      // let all large-R jets pass
      if (fatjets) {
        for( const xAOD::Jet * jet : *fatjets) {
          Props::OROutputLabel.set(jet, false);
        }
      }
    }
  } // end !special analysis flag
*/

  if(m_applyNewToolOR && !m_applyVBFGammaOR && !m_applyHHbbtautauOR){
    
    // Lepton overlap removal for b-jet correction
    EL_CHECK("OverlapRemoval::removeOverlap()", m_toolBoxLepOnly.masterTool->removeOverlaps(electrons, muons, jets, taus, photons, fatjets)); 
    // new overlap removal
    EL_CHECK("OverlapRemoval::removeOverlap()", m_toolBox.masterTool->removeOverlaps(electrons, muons, jets, taus, photons, fatjets)); 



    if (!m_applyORlargeR && fatjets) {
      for( const xAOD::Jet * jet : *fatjets) {
	Props::OROutputLabel.set(jet, false);
      }
    }
    // force taus to pass
    if (taus && !m_useTauOR) {
      for( const xAOD::TauJet * tau : *taus) {
	Props::OROutputLabel.set(tau,false);
      }
    }

    if (ditaus) {
      for( const xAOD::DiTauJet*  ditau : *ditaus) {
        Props::OROutputLabel.set(ditau,false);
        Props::LepOnlyOROutputLabel.set(ditau,false);
      }
    }

  }
  
  if(m_applyVBFGammaOR) {
    // new overlap removal
    EL_CHECK("OverlapRemoval::removeOverlap()", m_toolBoxLepOnly.masterTool->removeOverlaps(electrons, muons, jets, taus, photons, nullptr)); 
    EL_CHECK("OverlapRemoval::removeOverlap()", m_toolBox.masterTool->removeOverlaps(electrons, muons, jets, taus, photons, nullptr)); 
  }
  }//end pass ele,muo, and jets

  if(m_applyHHbbtautauOR){

    // if(!m_applyNewToolOR){
    //   //EL_CHECK("OverlapRemoval::removeOverlap()",m_overlapRemovalTool->removeOverlaps(electrons, muons, jets, taus));
    // }
    // else{
      
      EL_CHECK("OverlapRemoval::removeOverlap()", m_toolBoxLepOnly.masterTool->removeOverlaps(electrons, muons, jets, taus, nullptr, nullptr)); 
      EL_CHECK("OverlapRemoval::removeOverlap()", m_toolBox.masterTool->removeOverlaps(electrons, muons, jets, taus, nullptr, nullptr)); 
      //}

    for(const auto jet : * jets){
      if(!Props::ORInputLabel.get(jet) || Props::OROutputLabel.get(jet)) continue;
      bool isBTagged = Props::isBJet.get(jet);

      for(const auto tau : * taus){
     	  if(!Props::ORInputLabel.get(tau) || Props::OROutputLabel.get(tau)) continue;
	     bool isAntiTau = Props::isAntiTau.get(tau);
	     bool isOverlap = false;
	     isOverlap = m_dRMatcher->objectsMatch(*jet,*tau);
	     
	     if(isBTagged  && isAntiTau && isOverlap){ Props::OROutputLabel.set(tau, true); }// b-jet and anti-tau overlapping

	     if(isOverlap){
	       bool tempTauOR = Props::OROutputLabel.get(tau);
	       bool tempJetOR = Props::OROutputLabel.get(jet);
	       if (m_debug)std::cout << "HH: BaseOR, " 
				 << "jpt= " << jet->pt()/1000.
				 << " tpt= " << tau->pt()/1000.
				 << " eta= " << tau->eta()
				 << " phi= " << tau->phi()
				 << " Overlapping: " << isOverlap << ", BTagJet: " << isBTagged <<  ", AntiTau: " << isAntiTau << ", TauOR: " << tempTauOR << ", JetOR: " << tempJetOR << std::endl;
	     }
      }
    }

   if (fatjets) {
     for( const xAOD::Jet*  jet : *fatjets) {
        Props::OROutputLabel.set(jet,false);
        Props::LepOnlyOROutputLabel.set(jet,false);
     }
   }

   if (ditaus) {
     for( const xAOD::DiTauJet*  ditau : *ditaus) {
        Props::OROutputLabel.set(ditau,false);
        Props::LepOnlyOROutputLabel.set(ditau,false);
     }
   }

  }
  getLepOnlyOROutputLabels(electrons,  photons, muons,  taus, jets, fatjets, ditaus);
  getOROutputLabels(electrons,  photons, muons,  taus, jets, fatjets, ditaus);

  return EL::StatusCode::SUCCESS;
}

void OverlapRemoval::getOROutputLabels(const xAOD::ElectronContainer* electrons,
                                             const xAOD::PhotonContainer* photons,
					     const xAOD::MuonContainer* muons,
					     const xAOD::TauJetContainer* taus,
                                             const xAOD::JetContainer* jets,
                                             const xAOD::JetContainer* fatjets,
                                             const xAOD::DiTauJetContainer* ditaus)
{
  // set passOR flags
  if (electrons) {
    for (const xAOD::Electron* elec : *electrons) {
      bool passOR = Props::ORInputLabel.get(elec);
      if (m_applyOverlapRemoval)
        passOR &= ! Props::OROutputLabel.get(elec);
      Props::passOR.set(elec, passOR);
    }
  }

  if (muons) {
    for (const xAOD::Muon* muon : *muons) {
      bool passOR = Props::ORInputLabel.get(muon);
      if (m_applyOverlapRemoval)
        passOR &= ! Props::OROutputLabel.get(muon);
      Props::passOR.set(muon, passOR);
    }
  }
 
  if (jets) {
    for (const xAOD::Jet* jet : *jets) {
      bool passOR = Props::ORInputLabel.get(jet);
      if (m_applyOverlapRemoval)
        passOR &= ! Props::OROutputLabel.get(jet);
      Props::passOR.set(jet, passOR);
    }
  }
 
  if (fatjets) {
    for (const xAOD::Jet* jet : *fatjets) {
      bool passOR = Props::ORInputLabel.get(jet);
      if (m_applyOverlapRemoval)
        passOR &= ! Props::OROutputLabel.get(jet);
      Props::passOR.set(jet, passOR);
    }
  }
 
  if (taus){
    for (const xAOD::TauJet* tau : *taus) {
      bool passOR = Props::ORInputLabel.get(tau);
      if (m_applyOverlapRemoval)
        passOR &= ! Props::OROutputLabel.get(tau);
      Props::passOR.set(tau, passOR);
    }
  } 

  if (ditaus){
    for (const xAOD::DiTauJet* ditau : *ditaus) {
      bool passOR = Props::ORInputLabel.get(ditau);
      if (m_applyOverlapRemoval)
        passOR &= ! Props::OROutputLabel.get(ditau);
      Props::passOR.set(ditau, passOR);
    }
  } 

  if (photons){
    for(const xAOD::Photon* ph : *photons) {
      bool passOR = Props::ORInputLabel.get(ph);
      if (m_applyOverlapRemoval)
	passOR &= ! Props::OROutputLabel.get(ph);
      Props::passOR.set(ph, passOR);
    }
  }
}

void OverlapRemoval::getLepOnlyOROutputLabels(const xAOD::ElectronContainer* electrons,
                                             const xAOD::PhotonContainer* photons,
					     const xAOD::MuonContainer* muons,
					     const xAOD::TauJetContainer* taus,
                                             const xAOD::JetContainer* jets,
                                             const xAOD::JetContainer* fatjets,
                                             const xAOD::DiTauJetContainer* ditaus)
{
  // set passOR flags
  if (electrons) {
    for (const xAOD::Electron* elec : *electrons) {
      bool passOR = true;
      if (m_applyOverlapRemoval)
        passOR &= ! Props::LepOnlyOROutputLabel.get(elec);
      Props::passLepOnlyOR.set(elec, passOR);
    }
  }

  if (muons) {
    for (const xAOD::Muon* muon : *muons) {
      bool passOR = true;
      if (m_applyOverlapRemoval)
        passOR &= ! Props::LepOnlyOROutputLabel.get(muon);
      Props::passLepOnlyOR.set(muon, passOR);
    }
  }
 
  if (jets) {
    for (const xAOD::Jet* jet : *jets) {
      bool passOR = true;
      if (m_applyOverlapRemoval)
        passOR &= ! Props::LepOnlyOROutputLabel.get(jet);
      Props::passLepOnlyOR.set(jet, passOR);
    }
  }
 
  if (fatjets) {
    for (const xAOD::Jet* jet : *fatjets) {
      bool passOR = true;
      if (m_applyOverlapRemoval)
        passOR &= ! Props::LepOnlyOROutputLabel.get(jet);
      Props::passLepOnlyOR.set(jet, passOR);
    }
  }
 
  if (taus){
    for (const xAOD::TauJet* tau : *taus) {
      bool passOR = true;
      if (m_applyOverlapRemoval)
        passOR &= ! Props::LepOnlyOROutputLabel.get(tau);
      Props::passLepOnlyOR.set(tau, passOR);
    }
  } 

  if (ditaus){
    for (const xAOD::DiTauJet* ditau : *ditaus) {
      bool passOR = true;
      if (m_applyOverlapRemoval)
        passOR &= ! Props::LepOnlyOROutputLabel.get(ditau);
      Props::passLepOnlyOR.set(ditau, passOR);
    }
  } 

  if (photons){
    for(const xAOD::Photon* ph : *photons) {
      bool passOR = true;
      if (m_applyOverlapRemoval)
	passOR &= ! Props::LepOnlyOROutputLabel.get(ph);
      Props::passLepOnlyOR.set(ph, passOR);
    }
  }
}


EL::StatusCode OverlapRemoval::VariableConeOR(const xAOD::ElectronContainer * electrons,
					      const xAOD::MuonContainer * muons,
					      const xAOD::JetContainer * jets,
					      const xAOD::TauJetContainer * taus,
					      const xAOD::PhotonContainer * photons) {
  
  // Check input
  if ( ! jets || ! muons || ! electrons ) {
    Error("OverlapRemoval::VariableConeOR()","One or more inputs are null pointers!");
    return EL::StatusCode::FAILURE;
  }
  
  // Initialize output decorations
  // set all to 'keep object', except for electrons where the ele-mu OR is taken into account
  for (const auto jet : *jets) {
    Props::OROutputLabel.set(jet, false);
  }
  for (const auto elec : *electrons) {
    Props::OROutputLabel.set(elec, false);
  }
  for (const auto muon : *muons) {
    Props::OROutputLabel.set(muon, false);
  }
  if ( taus ) {
    for (const auto tau : *taus) {
      Props::OROutputLabel.set(tau, false);
    }
  }
  if ( photons ) {
    for (const auto photon : *photons) {
      Props::OROutputLabel.set(photon, false);
    }
  }
  
  // The sequence of call below are the same as in the legacy tool, defined in 'removeOverlaps()', only replacing ele/mu-jet OR with a variable cone OR.
  // https://svnweb.cern.ch/trac/atlasoff/browser/PhysicsAnalysis/AnalysisCommon/AssociationUtils/tags/AssociationUtils-01-01-17/Root/OverlapRemovalToolLegacy.cxx

  // tau and e/mu OR (legacy tool)
  if (taus) {
    //EL_CHECK("VariableConeOR()", m_overlapRemovalTool->removeTauEleOverlap(*taus, *electrons) );
    //EL_CHECK("VariableConeOR()", m_overlapRemovalTool->removeTauMuonOverlap(*taus, *muons) );
  }
  // e-mu OR (legacy tool)
  //EL_CHECK("VariableConeOR()", m_overlapRemovalTool->removeEleMuonOverlap(*electrons, *muons) );
  // photon and e/mu OR (legacy tool)
  if (photons) {
    //EL_CHECK("VariableConeOR()", m_overlapRemovalTool->removePhotonEleOverlap(*photons, *electrons) );
    //EL_CHECK("VariableConeOR()", m_overlapRemovalTool->removePhotonMuonOverlap(*photons, *muons) );
  }
  // jet and e/mu variable cone OR
  EL_CHECK("VariableConeOR()", removeEleJetOverlap_VarCone(electrons, jets) );
  EL_CHECK("VariableConeOR()", removeMuonJetOverlap_VarCone(muons, jets) );
  // tau-jet OR (legacy tool)
  //if (taus) EL_CHECK("VariableConeOR()", m_overlapRemovalTool->removeTauJetOverlap(*taus, *jets) );
  // photon-jet OR (legacy tool)
  //if (photons) EL_CHECK("VariableConeOR()", m_overlapRemovalTool->removePhotonJetOverlap(*photons, *jets) );
  
  return EL::StatusCode::SUCCESS;
  
}




EL::StatusCode OverlapRemoval::removeEleJetOverlap_VarCone(const xAOD::ElectronContainer * electrons, 
							   const xAOD::JetContainer * jets) {

  // Remove jets that overlap with surviving electrons
  // this is a hybrid between old and new - should also check b-tagging
  for (const auto jet : *jets) {
    if ( ! Props::ORInputLabel.get(jet) || Props::OROutputLabel.get(jet) ) continue;
    TLorentzVector jetTLV = jet->p4();
    for (const auto elec : *electrons) {
      if ( ! Props::ORInputLabel.get(elec) || Props::OROutputLabel.get(elec) ) continue;
      TLorentzVector elecTLV = elec->p4();
      if (jetTLV.DeltaR(elecTLV) < 0.2) {
	Props::OROutputLabel.set(jet, true);
      } 
    }
  }
     
  // Remove electrons that overlap with surviving jets
  for (const auto jet : *jets) {
    if ( ! Props::ORInputLabel.get(jet) || Props::OROutputLabel.get(jet) ) continue;
    if(Props::PassJvtMedium.exists(jet)) {
      if(!(Props::PassJvtMedium.get(jet))) continue;
    } else {
      if( (jet->pt() < 60e3 && fabs(Props::DetectorEta.get(jet)) < 2.4) && Props::Jvt.get(jet) < 0.59 ) continue;
    }
    TLorentzVector jetTLV = jet->p4();
    for (const auto elec : *electrons) {
      if ( ! Props::ORInputLabel.get(elec) || Props::OROutputLabel.get(elec) ) continue;
      TLorentzVector elecTLV = elec->p4();
      float dR = jetTLV.DeltaR(elecTLV);
      float dRcut = std::min(0.4, 0.04 + 10.e3/elecTLV.Pt());
      if ( dR < dRcut ) {
	Props::OROutputLabel.set(elec, true);
      }
    }
  }

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode OverlapRemoval::removeMuonJetOverlap_VarCone(const xAOD::MuonContainer * muons, 
							    const xAOD::JetContainer * jets) {

  // Remove jets that overlap with muons
  // this a hybrid between old and new - should also check b-tagging and track pointers
  for (const auto jet : *jets) {
    if ( ! Props::ORInputLabel.get(jet) || Props::OROutputLabel.get(jet) ) continue;
    TLorentzVector jetTLV = jet->p4();
    for (const auto muon : *muons) {
      if ( ! Props::ORInputLabel.get(muon) || Props::OROutputLabel.get(muon) ) continue;
      TLorentzVector muonTLV = muon->p4();
      if ( ! (Props::NumTrkPt500PV.get(jet) < 3 || (muonTLV.Pt()/Props::SumPtTrkPt500PV.get(jet) > 0.7 && jetTLV.Pt()/muonTLV.Pt() < 2)) ) continue;
      if (jetTLV.DeltaR(muonTLV) < 0.2) { 
	Props::OROutputLabel.set(jet, true);
      } 
    }
  }
     
  // Remove muons that overlap with surviving jets
  for (const auto jet : *jets) {
    if ( ! Props::ORInputLabel.get(jet) || Props::OROutputLabel.get(jet) ) continue;
    if(Props::PassJvtMedium.exists(jet)) {
      if(!(Props::PassJvtMedium.get(jet))) continue;
    } else {
      if( (jet->pt() < 60e3 && fabs(Props::DetectorEta.get(jet)) < 2.4) && Props::Jvt.get(jet) < 0.59 ) continue;
    }
    TLorentzVector jetTLV = jet->p4();
    for (const auto muon : *muons) {
      if ( ! Props::ORInputLabel.get(muon) || Props::OROutputLabel.get(muon) ) continue;
      TLorentzVector muonTLV = muon->p4();
      float dR = jetTLV.DeltaR(muonTLV);
      float dRcut = std::min(0.4, 0.04 + 10.e3/muonTLV.Pt());
      if ( dR < dRcut ) {
	Props::OROutputLabel.set(muon, true);
      } 
    }
  }

  return EL::StatusCode::SUCCESS;
}

