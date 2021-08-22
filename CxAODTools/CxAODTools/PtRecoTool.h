// Dear emacs, this is -*-c++-*-                                                                                                                                              
#ifndef CxAODTools_PtRecoTool_H
#define CxAODTools_PtRecoTool_H

#include "EventLoop/StatusCode.h"

#include <iostream>
#include <string>
#include <map>
#include "TFile.h"
#include "TH1F.h"

class TFile;
class TH1F;

class PtRecoTool {

public:
  PtRecoTool();
  ~PtRecoTool();
  EL::StatusCode initialize(bool debug);
  float getFactor(float jetPt, const std::string& name);
  float getFactor(const float& jetPt, const int& isSL);

protected:
  bool m_debug;
  std::map<std::string,TH1F*> m_map_name_TH1F;
  EL::StatusCode initializePtRecoHistograms();
  TFile* m_ptrecoFile;
  TH1* m_ptrecoHistSemileptonic;
  TH1* m_ptrecoHistHadronic;
};

#endif // ifndef CxAODTools_PtRecoTool_H 
