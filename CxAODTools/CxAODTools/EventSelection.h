// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools__EventSelection_H
#define CxAODTools__EventSelection_H

#ifndef __MAKECINT__
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/CutFlowCounter.h"
#include "CxAODTools/ConfigStore.h"
#endif // not __MAKECINT__

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODTruth/TruthParticleContainer.h"	
#include "xAODTruth/TruthEventContainer.h"
#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

#include "EventLoop/StatusCode.h"

class TString;

// structure to hold containers for event selection
struct SelectionContainers {

public:
  
  SelectionContainers() :
    evtinfo  (nullptr),
    met      (nullptr),
    TruthMET (nullptr),
    met_soft (nullptr),
    mettrack (nullptr),
    metmjtight(nullptr),
    metfjvt(nullptr),
    metmjmutight(nullptr),
    metmjmiddle(nullptr),
    metmjloose(nullptr),
    electrons(nullptr),
    forwardelectrons(nullptr),
    photons  (nullptr),
    muons    (nullptr),
    taus     (nullptr),
    ditaus   (nullptr),
    jets     (nullptr),
    fatjets  (nullptr),
    fatjetsAlt  (nullptr),
    trackjets(nullptr),
    subjets(nullptr),
    truthEvent(nullptr),
    truthParticles(nullptr),
    truthMuons(nullptr),
    truthTaus(nullptr),
    truthElectrons(nullptr),
    truthNeutrinos(nullptr),
    truthAntiKt4TruthWZJets(nullptr),
    truthAntiKt10TruthWZJets(nullptr)
  {}

  const xAOD::EventInfo         * evtinfo;  
  const xAOD::MissingET         * met;	    
  const xAOD::MissingET         * TruthMET;
  const xAOD::MissingET         * met_soft;	    
  const xAOD::MissingET         * mettrack;
  const xAOD::MissingET         * metmjtight;	    
  const xAOD::MissingET         * metfjvt;	    
  const xAOD::MissingET         * metmjmutight;	    
  const xAOD::MissingET         * metmjmiddle;
  const xAOD::MissingET         * metmjloose;
  const xAOD::ElectronContainer * electrons;
  const xAOD::ElectronContainer * forwardelectrons;
  const xAOD::PhotonContainer   * photons;  
  const xAOD::MuonContainer     * muons;    
  const xAOD::TauJetContainer   * taus;	    
  const xAOD::DiTauJetContainer * ditaus;
  const xAOD::JetContainer      * jets;	    
  const xAOD::JetContainer      * fatjets;  
  const xAOD::JetContainer      * fatjetsAlt;  
  const xAOD::JetContainer      * trackjets;
  const xAOD::JetContainer      * subjets;
  const xAOD::TruthEventContainer     * truthEvent; 
  const xAOD::TruthParticleContainer     * truthParticles; 
  const xAOD::TruthParticleContainer     * truthMuons;
  const xAOD::TruthParticleContainer     * truthTaus;
  const xAOD::TruthParticleContainer     * truthElectrons;
  const xAOD::TruthParticleContainer     * truthNeutrinos;
  const xAOD::JetContainer     * truthAntiKt4TruthWZJets;
  const xAOD::JetContainer     * truthAntiKt10TruthWZJets;
};


class EventSelection
{
protected:
#ifndef __MAKECINT__
  CutFlowCounter m_cutFlow;
  TString m_sysName;
  int m_selectionFlag;
  int m_metMJCalc;
  bool m_RunIsoSF;
  float m_jetPtCut;
  float m_jetRapidityCut;
  float m_lepPtCut;
  ConfigStore *m_config;
  std::string m_RecoToTruthLinkName;
#endif // not __MAKECINT__
  
public:
  
  static bool sort_pt(const xAOD::Jet* jetA, const xAOD::Jet* jetB) {
    return jetA->pt() > jetB->pt();
  }

  static bool sortTaus_pt(const xAOD::TauJet* tauA, const xAOD::TauJet* tauB) {
    return tauA->pt() > tauB->pt();
  }


  EventSelection(const TString cutFlowName = "PreselectionCutFlow") ;
  EventSelection(ConfigStore* config, const TString cutFlowName = "PreselectionCutFlow") ;
  virtual ~EventSelection() = default;
  
  virtual bool passPreSelection(SelectionContainers & containers, bool isKinVar) = 0;
  
  virtual bool passSelection(SelectionContainers & containers, bool isKinVar) = 0;
  
  //explicitly define the truth methods to a sensible default, so that not all readers need to 
  //implement them if they are nto needed. If they are needed, these can be over-ridden later
  virtual bool passTruthPreSelection(SelectionContainers & /*containers*/, bool /*isKinVar*/) {return false;} ;
  virtual bool passTruthSelection(SelectionContainers & /*containers*/, bool /*isKinVar */) {return false;} ;
  

  virtual EL::StatusCode writeEventVariables(const xAOD::EventInfo* /* eventInfoIn */,
                                             xAOD::EventInfo* /* eventInfoOut */,
                                             bool /* isKinVar */,
                                             bool /* isWeightVar */,
                                             std::string /* sysName */,
                                             int /* rdm_RunNumber */,
                                               CP::MuonTriggerScaleFactors* /* m_trig_sfmuon */) { return EL::StatusCode::SUCCESS; };

  virtual void writeEventVariables(const xAOD::EventInfo* /*eventInfoIn*/,
                                   xAOD::EventInfo* /*eventInfoOut*/,
                                   bool /*isKinVar*/,
                                   bool /*isWeightVar*/) {return;};

  const CutFlowCounter& getCutFlowCounter() const { return m_cutFlow; }
  bool passJetCleaning(const xAOD::JetContainer* jets, const xAOD::EventInfo* evtinfo);
  virtual void setSysName(const TString& name) {m_sysName = name;}

  virtual void setSelectionFlag(const int newSelectionFlag) {m_selectionFlag = newSelectionFlag;}
  int getSelectionFlag() const { return m_selectionFlag;}

  virtual void setMetMJCalc(const int mj){ m_metMJCalc = mj; }
  int getMetMJCalc() { return m_metMJCalc; }

  virtual void setRunIsoSF(bool run_iso){ m_RunIsoSF =run_iso; }
  bool getRunIsoSF() { return m_RunIsoSF; }

  virtual void setJetPtCut(float jet_ptcut){ m_jetPtCut =jet_ptcut; }
  float getJetPtCut() { return m_jetPtCut; }
  virtual void setJetRapidityCut(float jet_rapiditycut){ m_jetRapidityCut =jet_rapiditycut; }
  float getJetRapidityCut() { return m_jetRapidityCut; }
  virtual void setLepPtCut(float lep_ptcut){ m_lepPtCut =lep_ptcut; }
  float getLepPtCut() { return m_lepPtCut; }

  /// \brief  Setter function for configuration store pointer. Called in EventSelection:: constructor
  /// \param  Configuration store pointer
  /// \return void
  void setConfig( ConfigStore * config ){ m_config = config; }

  /// \brief Setter function for the link name to be used
  ///        by the EvtSElTruthAllocator:: element linking process
  /// \param None
  /// \return void
  void setRecoTruthLinkName(){ m_config->getif<std::string>("RecoToTruthLinkName", m_RecoToTruthLinkName); }  
  

};

#endif
