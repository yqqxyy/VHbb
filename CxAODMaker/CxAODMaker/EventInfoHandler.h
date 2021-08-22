// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_EventInfoHandler_H
#define CxAODMaker_EventInfoHandler_H

#include "CxAODTools/CommonProperties.h"

#include "AsgTools/AsgMessaging.h"

#include "xAODJet/JetContainer.h"
#include "xAODEventInfo/EventInfo.h"

// Standard Template Library includes
#include <utility>
#include <vector>
#include <string>
#include <unordered_map>

// Forward declarations
class ConfigStore;
class PUReweightingTool;
class GoodRunsListSelectionTool;
namespace TrigConf {
  class xAODConfigTool;
}
namespace Trig {
  class TrigDecisionTool;
}
class TrigObjects;

class EventShape;

class JetHandler;

class EventInfoHandler {

public:

  EventInfoHandler(ConfigStore & config, xAOD::TEvent * event);

  virtual ~EventInfoHandler();

  // initialize tools and stuff
  virtual EL::StatusCode initialize();
  // add systematic variations that affect the event info
  virtual EL::StatusCode addVariations(const std::vector<TString> &variations);
  // read the info from the event
  virtual EL::StatusCode executeEvent();
  // initialize output container for all variations
  // a preceeding call of executeEvent() is required
  virtual EL::StatusCode setOutputContainer();
  // fill/record output container
  virtual EL::StatusCode fillOutputContainer();
  // get all variations that affect the event info
  virtual std::vector<TString> getAllVariations();
  
  virtual void setJets(JetHandler* jets);

  // get / set info
  int   get_runNumber()       { return m_eventInfoIn->runNumber()                                ; }
  unsigned long long   get_eventNumber()     { return m_eventInfoIn->eventNumber()                              ; }
  unsigned long long   get_mcEventNumber()   { return m_isMC ? m_eventInfoIn->mcEventNumber()           : -1    ; }
  int   get_mcChannelNumber() { return m_isMC ? m_eventInfoIn->mcChannelNumber()         : -1    ; }
  float get_MCEventWeight()   { return m_isMC ? Props::MCEventWeight.get(m_eventInfoIn)  : -999. ; }
  float get_PileupReweight()    { return Props::PileupReweight.get(m_eventInfoIn); }
  float get_RandomRunNumber() { return Props::RandomRunNumber.get(m_eventInfoIn);}
  PUReweightingTool* get_PUReweightingTool() { return m_puReweightingTool; } 
  float get_averageInteractionsPerCrossing() {return m_eventInfoIn->averageInteractionsPerCrossing();}
  float get_ZPV()             { return Props::ZPV.get(m_eventInfoIn)                             ; }
  int   get_indexPV()         { return m_indexPV                                                 ; }
  int   get_NVtx3Trks()       { return Props::NVtx3Trks.get(m_eventInfoIn)                       ; }
  void  set_isMC(int isMC)    { m_isMC = isMC                                                    ; } 
  int   get_isMC()            { return m_isMC                                                    ; }

  // TODO random run number should be generated once within EventInfoHandler.cxx if needed
  int   get_TriggerStream()   { return m_triggerStream                                           ; }

  const std::unordered_map<std::string,PROP<int> *> & get_trigDecorators() { return m_trigDecorators; }

  Trig::TrigDecisionTool*   get_TrigDecTool()   { return m_trigDecTool                           ; }

  void  set_TriggerStream(int triggerStream)   { m_triggerStream = triggerStream; }
  
  void set_isDerivation(bool isDerivation){m_isDerivation = isDerivation;}
  bool get_isDerivation() {return m_isDerivation;}

  void set_derivationName(std::string derivationName){m_derivationName = derivationName;}
  std::string get_derivationName() {return m_derivationName;}
  
  void set_isAFII(bool isAFII){m_isAFII = isAFII;}
  bool get_isAFII() {return m_isAFII;}

  void set_pTag(std::string pTag){m_pTag = pTag;}
  std::string get_pTag() {return m_pTag;}

  void set_hasNoFJDetectorEta(bool hasNoFJDetectorEta){m_hasNoFJDetectorEta = hasNoFJDetectorEta;}
  bool get_hasNoFJDetectorEta(){return m_hasNoFJDetectorEta;}
  
  void set_mcChanNr(int mcChanNr){m_mcChanNr = mcChanNr;}
  int get_mcChanNr(){return m_mcChanNr;}

  float get_beamPosUncX() {return m_eventInfoIn->beamPosSigmaX();}
  float get_beamPosUncY() {return m_eventInfoIn->beamPosSigmaY();}
  float get_beamPosCorrXY() {return m_eventInfoIn->beamPosSigmaXY();}

  // get input event info with full info
  const xAOD::EventInfo * getEventInfo() const { return m_eventInfoIn; }

  // get output event info variation (e.g. for writing event variables)
  xAOD::EventInfo * getOutEventInfoVariation(const TString &variation, bool fallbackToNominal = true);

  // get number of truth jets in MC, return 0 for data
  EL::StatusCode set_NTruthWZJets(const xAOD::JetContainer* truthWZJets);

  // get the per event JVT SF, return 1 for data
  EL::StatusCode set_JvtSF();

  // set codeTTBarDecay
  EL::StatusCode setCodeTTBarDecay(int code);
  
  // set TTBarpTW
  EL::StatusCode setTTbarpTW(float ptw);
  
  // set TTBarpTL
  EL::StatusCode setTTbarpTL(float ptl);

  // set TTBarEtaL
  EL::StatusCode setTTbarEtaL(float etal);

  // set SherpapTV
  EL::StatusCode setSherpapTV(float ptw);

  // set isVZbb
  EL::StatusCode isVZbb(bool isVZbb);

  // set SUSY MET
  EL::StatusCode setSUSYMET();

  // set ForHadHad
  EL::StatusCode setForHadHad(int isForHadHad);

  // set ForFourTauOnly
  EL::StatusCode setForFourTauOnly(int isForFourTauOnly);

  // clean shallow copy
  EL::StatusCode clearEvent();

  //MET trigger emulation
  int METtrigEmulation(const TString &triggerName);
  

protected:

  ConfigStore & m_config;
  bool m_debug;
  MSG::Level m_msgLevel;
  std::vector<std::string> m_grl;

  xAOD::TEvent * m_event;

  JetHandler* m_jetHandler;

  std::vector<TString> m_variations;
  
  virtual void writeOutputVariables(xAOD::EventInfo* eventInfoIn, xAOD::EventInfo* eventInfoOut);
  
  virtual EL::StatusCode writeVariation(xAOD::EventInfo* eventInfoIn, xAOD::EventInfo* eventInfoOut, TString sysName);

#ifndef __MAKECINT__

  // pointer to the Nominal EventInfo (shallow copy on input)
  xAOD::EventInfo      * m_eventInfoIn; //!
  // maps for all output variations
  std::map<TString, xAOD::EventInfo*> m_eventInfosOut; //!
  
  std::vector<std::string> m_triggerList;
  std::unordered_map<std::string,PROP<int> *> m_trigDecorators; //!
  GoodRunsListSelectionTool *m_grlSelectionTool; //!
  PUReweightingTool* m_puReweightingTool; //!
  TrigConf::xAODConfigTool *m_trigConfigTool; //!
  Trig::TrigDecisionTool *m_trigDecTool; //;
  #endif // not __MAKECINT__

  // Trigger objects
  bool m_useTrigObj;
  bool m_allowTrigObjFail;
  std::vector<TrigObjects *> m_trigObjects;
  bool m_METtrigEmul;

  std::string m_jetAlgoName;
  bool m_isPFlow;

  int m_firstEvent;
  int m_isMC;
  int m_triggerStream;
  int m_indexPV;
  bool m_isDerivation;
  std::string m_derivationName;
  bool m_isAFII;
  std::string m_pTag;
  bool m_hasNoFJDetectorEta;
  int m_mcChanNr;

};

#endif
