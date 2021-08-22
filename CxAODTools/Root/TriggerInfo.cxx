#include "CxAODTools/TriggerInfo.h"
#include "TError.h"
#include "TObjString.h"
#include "TObjArray.h"

TriggerInfo::TriggerInfo(TString name) :
        m_name(name),
        m_passProp(nullptr),
        m_matchProp(nullptr),
        m_dataType(DataType::undefined),
        m_firstPeriod(DataPeriod::undefined),
        m_lastPeriod(DataPeriod::undefined),
        m_triggerObject(TriggerObject::undefined),
        m_triggerThreshold(-1) {
}

void TriggerInfo::setPassProperty(PROP<int>* property) {
  m_passProp = property;
}

void TriggerInfo::setMatchProperty(PROP<int>* property) {
  m_matchProp = property;
}

void TriggerInfo::setDataType(DataType type) {
  m_dataType = type;
}

void TriggerInfo::setDataRange(DataPeriod first, DataPeriod last) {
  m_firstPeriod = first;
  m_lastPeriod = last;
}

void TriggerInfo::setTriggerObject(TriggerObject object) {
  m_triggerObject = object;
}

TString TriggerInfo::getName() {
  return m_name;
}

PROP<int>* TriggerInfo::getPassProperty() {
  return m_passProp;
}

PROP<int>* TriggerInfo::getMatchProperty() {
  return m_matchProp;
}

TriggerInfo::TriggerObject TriggerInfo::getTriggerObject() {
  if (m_triggerObject == TriggerObject::undefined) {
    m_triggerObject = getTriggerObjectFromName(m_name);
  }
  return m_triggerObject;
}

float TriggerInfo::getThreshold() {
  if (m_triggerThreshold < 0) {
    m_triggerThreshold = getTriggerThresholdFromName(m_name);
  }
  return m_triggerThreshold;
}

bool TriggerInfo::requiresMatching() {
  return (m_matchProp != nullptr);
}

bool TriggerInfo::appliesToType(DataType type) {
  if (m_dataType == DataType::any) {
    return true;
  }
  return (m_dataType == type);
}

bool TriggerInfo::appliesToPeriod(DataPeriod period) {
  if (period >= m_firstPeriod && period <= m_lastPeriod) {
    return true;
  }
  return false;
}

bool TriggerInfo::appliesToRun(int runNumber) {
  DataPeriod period = getDataPeriodFromRun(runNumber);
  return appliesToPeriod(period);
}

TriggerInfo::DataPeriod TriggerInfo::getDataPeriodFromRun (int runNumber) {

  // run numbers taken from:
  // https://atlas-tagservices.cern.ch/tagservices/RunBrowser/runBrowserReport/rBR_Period_Report.php
  
  DataPeriod period = DataPeriod::undefined;
  
  if      (runNumber < 266904)
    period = DataPeriod::undefined;
  else if      (runNumber >= 266904 &&
		runNumber <= 284484) period = DataPeriod::data15;
  else if (runNumber <= 300287) period = DataPeriod::data16A;
  else if (runNumber <= 302872) period = DataPeriod::data16BD3;
  else if (runNumber <= 303892) period = DataPeriod::data16D4E3;
  else if (runNumber <= 304008) period = DataPeriod::data16F1;
  else if (runNumber <= 311481) period = DataPeriod::data16F2L11;
  else if (runNumber <= 341649) period = DataPeriod::data17; 
  else if (runNumber <= 364292) period = DataPeriod::data18;
  // last data run in data18 taken from the latest GRL of data18, namely the one below, careful they are in decreasing order
  // http://atlas.web.cern.ch/Atlas/GROUPS/DATABASE/GroupData/GoodRunsLists/data18_13TeV/20181111/

  if (period == DataPeriod::undefined) {
    Warning("TriggerInfo::getDataPeriodFromRun()",
            "Data period for run number '%i' is not defined!", runNumber);
  }
  
  return period;
}

// TODO maybe nice with a map or so?
TPRegexp TriggerInfo::m_regexpElectron("_([1-9]?)e([1-9][0-9]*)");
TPRegexp TriggerInfo::m_regexpPhoton  ("_([1-9]?)g([1-9][0-9]*)");
TPRegexp TriggerInfo::m_regexpMuon    ("_([1-9]?)mu([1-9][0-9]*)");
TPRegexp TriggerInfo::m_regexpJet     ("_([1-9]?)j([1-9][0-9]*)");
TPRegexp TriggerInfo::m_regexpMET     ("_()xe([1-9][0-9]*)");


TriggerInfo::TriggerObject TriggerInfo::getTriggerObjectFromName(TString triggerName) {
  
  // TODO what about mixed triggers? Examples:
  // HLT_j100_xe80_L1J40_DPHI
  // HLT_g20_loose_L1EM18VH_2j40_0eta490_3j25_0eta490_invm700
  // Currently we are giving the following priority:
  
  TriggerObject object = TriggerObject::undefined;
  if (triggerName.Contains(m_regexpElectron)) {
    object = TriggerObject::Electron;
  } else if (triggerName.Contains(m_regexpPhoton)) {
    object = TriggerObject::Photon;
  } else if (triggerName.Contains(m_regexpMuon)) {
    object = TriggerObject::Muon;
  } else if (triggerName.Contains(m_regexpMET)) {
    object = TriggerObject::MET;
  } else if (triggerName.Contains(m_regexpJet)) {
    object = TriggerObject::Jet;
  }

  if (object == TriggerObject::undefined) {
    Warning("TriggerInfo::getTriggerObjectFromName()",
            "Trigger object for trigger name '%s' is not defined!", triggerName.Data());
  }

  return object;
}

float TriggerInfo::getTriggerThresholdFromName(TString triggerName) {
  TriggerObject object = getTriggerObjectFromName(triggerName);
  TPRegexp* regexp;
  if (object == TriggerObject::Electron) regexp = &m_regexpElectron;
  if (object == TriggerObject::Photon  ) regexp = &m_regexpPhoton;
  if (object == TriggerObject::Muon    ) regexp = &m_regexpMuon;
  if (object == TriggerObject::MET     ) regexp = &m_regexpMET;
  if (object == TriggerObject::Jet     ) regexp = &m_regexpJet;
  
  TObjArray* tokens = regexp -> MatchS(triggerName);
  if (tokens -> GetSize() < 3) {
    Warning("TriggerInfo::getTriggerObjectFromName()",
            "Not able to extract treshold from trigger '%s'"
            "with regular expression '%s'! Returning 0.",
            triggerName.Data(), regexp -> GetPattern().Data());
    return 0;
  }
  TString thresholdStr = ((TObjString*) tokens -> At(2)) -> GetString();
  delete tokens;
  float threshold = atoi(thresholdStr.Data()) * 1e3;
  return threshold;
}

