#include "CxAODTools/EventSelection.h"

EventSelection::EventSelection(const TString cutFlowName ) : 
  m_cutFlow(cutFlowName),
  m_metMJCalc(0),
  m_jetPtCut(25.0e3),
  m_jetRapidityCut(2.5),
  m_lepPtCut(25.0e3),
  m_config(nullptr),
  m_RecoToTruthLinkName("")
{}

EventSelection::EventSelection( ConfigStore *config, const TString cutFlowName ) : 
  m_cutFlow(cutFlowName),
  m_metMJCalc(0),
  m_jetPtCut(25.0e3),
  m_jetRapidityCut(2.5),
  m_lepPtCut(25.0e3),
  m_RecoToTruthLinkName("")
{
  //Initialise configuration store member variable
  if( config ){
    EventSelection::setConfig(config);
    EventSelection::setRecoTruthLinkName(); //If set to none "" value, then the element linking is performed
  }
  
}

bool EventSelection::passJetCleaning(const xAOD::JetContainer* jets, const xAOD::EventInfo* evtinfo){
  if(Props::DFCommonJets_eventClean_LooseBad.exists(evtinfo)){
      //https://twiki.cern.ch/twiki/bin/view/AtlasProtected/HowToCleanJets2017
      if(!Props::DFCommonJets_eventClean_LooseBad.get(evtinfo)) return false;
  }
  else{
      //https://twiki.cern.ch/twiki/bin/view/AtlasProtected/HowToCleanJets2015
      for (const xAOD::Jet * jet : *jets) {
          if (!Props::goodJet.get(jet) && Props::passOR.get(jet)){    
              if (Props::PassJvtMedium.get(jet)) return false;
              if(jet->pt() > 20e3 && fabs(Props::DetectorEta.get(jet)) >= 2.4) return false;
              if(jet->pt() > 60e3 ) return false;
          }//end check if jet is not good and passed the OR 
      }//end loop over jets in the event
  }
  return true; 
}
