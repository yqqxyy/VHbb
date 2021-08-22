#ifndef CxAODTools_TriggerTool_H
#define CxAODTools_TriggerTool_H

#include <map>

#include "EventLoop/StatusCode.h"

#include "CxAODTools/TriggerInfo.h"
#include "CxAODTools/ConfigStore.h"
#include "CxAODTools/ReturnCheck.h"

class TriggerScaleFactorsMET;

#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODMissingET/MissingET.h"

#include "PATInterfaces/SystematicSet.h"

namespace CP {
  class MuonTriggerScaleFactors;
}

// Compiler script for adding a trigger _without_ matching:
#define ADD_TRIG(name, dataType, firstPeriod, lastPeriod) \
  TriggerInfo* triggerInfo_##name = new TriggerInfo(#name); \
  m_triggers.insert(triggerInfo_##name); \
  triggerInfo_##name -> setPassProperty(&Props::pass##name); \
  triggerInfo_##name -> setDataType(TriggerInfo::DataType::dataType); \
  triggerInfo_##name -> setDataRange(TriggerInfo::DataPeriod::firstPeriod, \
                                     TriggerInfo::DataPeriod::lastPeriod); \

// Compiler script for adding a trigger _with_ matching:
#define ADD_TRIG_MATCH(name, dataType, firstPeriod, lastPeriod) \
  ADD_TRIG(name, dataType, firstPeriod, lastPeriod) \
  triggerInfo_##name -> setMatchProperty(&Props::match##name);

class TriggerTool {

public:

  TriggerTool(ConfigStore& config);
  virtual ~TriggerTool();

  virtual EL::StatusCode initialize();

  void setEventInfo(const xAOD::EventInfo* eventInfo, int randomRunNumber);
  // Make 2 separate functions of above to make it clearer
  void setEventInfo(const xAOD::EventInfo* eventInfo){m_eventInfo = eventInfo;}
  void setRandomRunNumber(int randomRunNumber) {m_randomRunNumber = randomRunNumber;} // only used in case of recomputed pileup 
      
  // Setters for trigger objects influence the trigger decision,
  // matching and scale-factor. By default only triggers are considered
  // for which the corresponding trigger object is set:
  void setElectrons(std::vector<const xAOD::Electron*> electrons);
  void setPhotons  (std::vector<const xAOD::Photon*>   photons);
  void setMuons    (std::vector<const xAOD::Muon*>     muons);
  void setJets     (std::vector<const xAOD::Jet*>      jets);
  void setMET      (const xAOD::MissingET*             met);
  void setSumpt    (double                       sumpt);
  
  virtual void clearTriggerObjects();
  
  EL::StatusCode applySystematicVariation(CP::SystematicSet set);

  // main getter for trigger decision and scale factor:
  bool getDecisionAndScaleFactor(double& triggerSF);

  // alternative getters (somewhat slower and less checks):
  bool getDecision();
  double getScaleFactor();
  double getScaleFactor(std::set<TriggerInfo*> passedTriggers);
  std::set<TriggerInfo*> getPassedTriggers();



protected:
  
  virtual TString getMuonTriggerName(std::set<TriggerInfo*> triggersmu, int runNumber); //Get muon trigger name which depends on run number

  void validateIneff(double& ineff, double& ineffScaled);

  virtual double getInefficiencyElectron(double& ineffScaled) ;
  virtual double getInefficiencyMuon(double& totalIneffScaled, std::set<TriggerInfo*> triggersmu) ;
  virtual double getInefficiencyMET(double& totalIneffScaled, std::set<TriggerInfo*> triggersMET);

  // initialization methods, intended to be overridden:
  virtual EL::StatusCode initTools();
  virtual EL::StatusCode initProperties();
  virtual EL::StatusCode initTriggers() = 0;
  
  virtual void addLowestUnprescaledElectron();
  virtual void addLowestUnprescaledMuon();
  virtual void addLowestUnprescaledMET();
  
  EL::StatusCode initMuonSFTool(TString muonQuality);
  
  virtual bool checkRequirements(TriggerInfo* trigger);
  virtual bool getMatch(TriggerInfo* trigger);
  template <class partType>
  bool getMatch(TriggerInfo* trigger, std::vector<const partType*> particles);
  template <class partType>
  std::vector<const partType*> getValidParticles(std::vector<const partType*> particles);
  
  std::set<TriggerInfo*> getTriggersOfObject(std::set<TriggerInfo*> triggers, TriggerInfo::TriggerObject object);
  
  virtual int getRunNumber();
  
  virtual double getScaleFactorElectron(std::set<TriggerInfo*> triggers);
  virtual double getScaleFactorMuon(std::set<TriggerInfo*> triggers);
  //virtual double getScaleFactorElectronMuon(std::set<TriggerInfo*> triggersel,std::set<TriggerInfo*> triggersmu) ; //triggersel not used, so commenting it out
  virtual double getScaleFactorElectronMuon(std::set<TriggerInfo*> triggersmu) ;
  virtual double getScaleFactorMET(std::set<TriggerInfo*> triggers);
  virtual double getScaleFactorMETMuon(std::set<TriggerInfo*> triggersMET,std::set<TriggerInfo*> triggersmu) ;

  ConfigStore m_config;
  bool m_debug;
  bool m_recomputePUWeight;
  bool m_applyMatching;
  float m_thresholdFactor;
  std::string m_eventInfoContNameMuonSF;  // to use Muon trigger tool at reader level
  
  CP::SystematicSet m_currentVariation;

  CP::MuonTriggerScaleFactors* m_muonSFTool;

  std::set<TriggerInfo*> m_triggers;

  PROP<float>* m_electronEffProp;
  PROP<float>* m_electronSFProp;

  std::vector<const xAOD::Electron*> m_electrons;
  std::vector<const xAOD::Photon*>   m_photons;
  std::vector<const xAOD::Muon*>     m_muons;
  std::vector<const xAOD::Jet*>      m_jets;
  const xAOD::MissingET*             m_met;
  double                       m_sumpt;
  
  const xAOD::EventInfo* m_eventInfo;
  int m_randomRunNumber;
  
  TriggerScaleFactorsMET* m_METscaleFactorTool;

  int m_lastNonZeroRun;
  int m_nRuns;
  int m_nZeroRuns;
  int m_nResetZeroRuns;
  int m_nHardResetZeroRuns;

};

#endif // ifndef CxAODTools_TriggerTool_H
