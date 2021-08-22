#ifndef CxAODTools_TriggerInfo_H
#define CxAODTools_TriggerInfo_H

#include "TString.h"
#include "TPRegexp.h"

template <typename T> class PROP;

class TriggerInfo {
  
public:
  
  enum class DataType {
    undefined = -1,
    any,
    data,
    MC
  };
  
  // TODO it might make more sense to define exclusive periods
  enum class DataPeriod {
    undefined = -1,
    data15,
    data16A,
    data16BD3,
    data16D4E3,
    data16F1,
    data16F2L11,
    data17,
    data18,
    future
  };
  
  enum class TriggerObject {
    undefined = -1,
    Electron,
    Photon,
    Muon,
    Jet,
    MET
  };
  
  TriggerInfo(TString name);
  virtual ~TriggerInfo() = default;
  
  void setPassProperty(PROP<int>* property);
  void setMatchProperty(PROP<int>* property);
  void setDataType(DataType type);
  void setDataRange(DataPeriod first, DataPeriod last);
  void setTriggerObject(TriggerObject object);
  
  TString getName();
  PROP<int>* getPassProperty();
  PROP<int>* getMatchProperty();
  TriggerObject getTriggerObject();
  float getThreshold();
  
  bool requiresMatching();
  bool appliesToType(DataType type);
  bool appliesToPeriod(DataPeriod period);
  bool appliesToRun(int runNumber);
  
  static DataPeriod getDataPeriodFromRun(int runNumber);
  static TriggerObject getTriggerObjectFromName(TString triggerName);
  static float getTriggerThresholdFromName(TString triggerName);
  
private:
  
  TString m_name;
  PROP<int>* m_passProp;
  PROP<int>* m_matchProp;
  DataType m_dataType;
  DataPeriod m_firstPeriod;
  DataPeriod m_lastPeriod;
  TriggerObject m_triggerObject;
  float m_triggerThreshold;
  
  static TPRegexp m_regexpElectron;
  static TPRegexp m_regexpPhoton;
  static TPRegexp m_regexpMuon;
  static TPRegexp m_regexpJet;
  static TPRegexp m_regexpMET;
};

#endif // ifndef CxAODTools_TriggerInfo_H
