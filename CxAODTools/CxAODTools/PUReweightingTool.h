// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools_PUReweightingTool_H
#define CxAODTools_PUReweightingTool_H

#ifndef __MAKECINT__
#include "EventLoop/StatusCode.h"
#include "CxAODTools/ConfigStore.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/ReturnCheck.h"
#include "AsgAnalysisInterfaces/IPileupReweightingTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "xAODEventInfo/EventInfo.h"
#endif // __MAKECINT__

#include <vector>

// Forward declarations
namespace CP {
  class PileupReweightingTool;
}

class PUReweightingTool {

public:

  PUReweightingTool(ConfigStore & config);
  ~PUReweightingTool(); 

  EL::StatusCode initialize();
  EL::StatusCode decorate(const xAOD::EventInfo* eventInfo);
  void copySystWeights(const xAOD::EventInfo* eventInfoIn, xAOD::EventInfo * eventInfoOut);
  CP::SystematicSet affectingSystematics();
  CP::SystematicCode applySystematicVariation(CP::SystematicSet set);
  asg::AnaToolHandle<CP::IPileupReweightingTool> getPileupReweightingTool() { return m_pileupReweighting; } // needed by CP tools (muon, tau, etc)

protected:
  ConfigStore & m_config;
  bool m_debug;
  bool m_computePileupReweight;
  bool m_recomputePileupReweight;
  MSG::Level m_msgLevel;
  std::vector<std::string> m_ilumicalcFiles;
  std::vector<std::string> m_configFiles;
  bool m_initialized;

#ifndef __MAKECINT__
  asg::AnaToolHandle<CP::IPileupReweightingTool> m_pileupReweighting; //!
#endif // __MAKECINT__
 
};

#endif // ifndef CxAODTools_PUReweightingTool_H
