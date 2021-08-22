// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_MatchJet_H
#define CxAODMaker_MatchJet_H

#include "xAODJet/Jet.h"

class JetHandler;
class FatJetHandler;

class MatchJet {

 public:
  MatchJet() {};
  ~MatchJet() {};

  EL::StatusCode test();
  EL::StatusCode matchJet(xAOD::Jet * truthjet, const TString& variation, JetHandler* jetHandler, double DRCut, bool m_debug);
  EL::StatusCode matchJet(xAOD::Jet * truthfatjet, const TString& variation, FatJetHandler* fatjetHandler, double DRCut, bool m_debug);

  private:
};

#endif
