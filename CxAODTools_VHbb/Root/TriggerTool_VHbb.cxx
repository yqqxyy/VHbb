#include "CxAODTools_VHbb/TriggerTool_VHbb.h"
#include "CxAODTools/CommonProperties.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"

TriggerTool_VHbb::TriggerTool_VHbb(ConfigStore& config) 
  :TriggerTool(config),

    m_doMuonTriggerin1L(false),
    m_doMETTriggerin2L(false),
    m_doMETMuonTrigger(false),
    m_analysisType("2lep"),
    m_pTWcutVal(150e3),
    m_pTZcutVal(150e3)
{	
  m_config.getif<bool>("doMETMuonTrigger", m_doMETMuonTrigger); 
  m_config.getif<bool>("doMuonTriggerin1L", m_doMuonTriggerin1L);
  m_config.getif<bool>("doMETTriggerin2L", m_doMETTriggerin2L);
  m_config.getif<std::string>("analysisType", m_analysisType);
  m_config.getif<double>("Trig::pTWcutVal", m_pTWcutVal);
  m_config.getif<double>("Trig::pTZcutVal", m_pTZcutVal);

}

bool TriggerTool_VHbb::getDecisionAndSFwithMET(double& triggerSF) {
  
  bool useMETAndMuonAtHighPtV;
  if (m_doMETMuonTrigger) {
      useMETAndMuonAtHighPtV = true;
  } else {  
      useMETAndMuonAtHighPtV = false;
  }

  const xAOD::MissingET* met = m_met;

  // for electrons: ensure no MET trigger is allowed and return:
  if (m_electrons.size()) {
    setMET(nullptr);
    bool decision = getDecisionAndScaleFactor(triggerSF);
    setMET(met);
    return decision;
  }
  
  //Store the decision to not use the MET trigger in a boolean
  bool notUsingMETTrigger = false;

  // To decide on whether to use a single lepton trigger or MET trigger, construct pTV 
  const xAOD::Muon* muon1 = m_muons.at(0);
  const xAOD::Muon* muon2 = nullptr; //Declaration of muon here just in case 2L analysis calls this. Will just exist for the 1L one and not do anything. 
  TLorentzVector pTVVec, muVec1;
  muVec1.SetPtEtaPhiM(muon1->pt(), 0, muon1->phi(), 0);
  
  if (m_analysisType == "1lep"){ //Constructs PtV from met and the muon
    TLorentzVector metVec;
  	metVec.SetPtEtaPhiM(m_met->met(), 0, m_met->phi(), 0); //There will be actual met in the event
  	pTVVec = muVec1 + metVec; // WVecT
  	if (pTVVec.Pt() < m_pTWcutVal || m_doMuonTriggerin1L) {notUsingMETTrigger = true;}
  } 
  else if (m_analysisType == "2lep"){ //Constructs PtV from the two muons
    TLorentzVector muVec2;
    muon2 = m_muons.at(1);
    muVec2.SetPtEtaPhiM(muon2->pt(), 0, muon2->phi(), 0);
    pTVVec = muVec1 + muVec2; //ZVecT
    if (pTVVec.Pt() < m_pTZcutVal) {notUsingMETTrigger = true;}
  } 
  else { //Add instance to catch accidental calls from 0L analysis
  	Error("TriggerTool_VHbb::getDecisionAndSFwithMET()",
            "Currenlty this method cannot work with the 0L analysis");
    exit(1);
  }

  // for lower pT(V) use muon trigger only unless you force it (for 1L):
  if (notUsingMETTrigger) {
    setMET(nullptr);
    bool decision = getDecisionAndScaleFactor(triggerSF);
    setMET(met);
    return decision;
  }
  //If we get to this point we want to only use the MET trigger (on the muon object)

  // TODO easier way without containers? Maybe set MET value directly ...
  xAOD::MissingETContainer* metCont = new xAOD::MissingETContainer();
  xAOD::MissingETAuxContainer* metContAux = new xAOD::MissingETAuxContainer();
  metCont->setStore( metContAux );

  //Comment in this to fill a MET object with vector-added muon pt and reco met to trigger on :
  xAOD::MissingET* recoMETWithMu = new xAOD::MissingET();
  metCont->push_back(recoMETWithMu);

  double METx = pTVVec.Px();
  double METy = pTVVec.Py();

  // PtV object in 2L contains no true MET but there may be some in the event, for an object 
  // that is close to what the detector sees to be replicated here, the MET in the event 
  // (if there is any) should be subtracted from the resulting PtV (vectorally added)
  if (m_analysisType == "2lep"){ 
    METx += met->mpx();
    METy += met->mpy();
  }

  recoMETWithMu->setMpx(METx);
  recoMETWithMu->setMpy(METy);
  setMET(recoMETWithMu);

  if(useMETAndMuonAtHighPtV && m_analysisType == "1lep")   setMuons({muon1});
  else if(useMETAndMuonAtHighPtV && m_analysisType == "2lep")   setMuons({muon1, muon2});
  else  setMuons({});

  bool decision = getDecisionAndScaleFactor(triggerSF);

  setMET(met);
  if(m_analysisType == "1lep")   setMuons({muon1});
  else setMuons({muon1, muon2}); //2l case, as if analysisType is 0lep then the function should have exited. 


  delete metContAux;
  delete metCont;

  return decision;
}
