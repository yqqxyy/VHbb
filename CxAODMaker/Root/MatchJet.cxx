#include "EventLoop/StatusCode.h" 

#include "CxAODTools/CommonProperties.h"

#include "CxAODMaker/MatchJet.h"
#include "CxAODMaker/JetHandler.h"
#include "CxAODMaker/FatJetHandler.h"

EL::StatusCode MatchJet::test() {
  std::cout<<" MatchJet::test()"<<std::endl;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MatchJet::matchJet(xAOD::Jet * truthjet, const TString& variation, JetHandler* jetHandler, double DRCut, bool m_debug) {
  if (m_debug) {
    std::cout<<"MatchJet::matchTruthJet with variation="<<variation<<" and DRCut="<<DRCut
	     <<" truthjet "<<truthjet->pt()<<" eta="<<truthjet->eta()<<" phi="<<truthjet->phi()<<" e="<<truthjet->e()<<" m="<<truthjet->m()
	     <<std::endl;
  }
  bool matched=false;
  for (xAOD::Jet * jet : *jetHandler->getInParticleVariation("Nominal")) {
    if (!(Props::isSignalJet.get(jet))) continue;
    double DR=truthjet->p4().DeltaR(jet->p4());
    if(!(DR<DRCut)) continue;
    if(m_debug) {
      std::cout<<"MatchJet::matchJet xAOD::Jet"
	       <<" pt="<<jet->pt()<<" eta="<<jet->eta()<<" phi="<<jet->phi()<<" e="<<jet->e()<<" m="<<jet->m()
	       <<" DR="<<DR
	       <<std::endl;
    }
    matched |= true;
  }
  Props::passPreSel.set(truthjet,matched);
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MatchJet::matchJet(xAOD::Jet * truthfatjet, const TString& variation, FatJetHandler* fatjetHandler, double DRCut, bool m_debug) {
  if (m_debug) {
    std::cout<<"MatchJet::matchFatJet with variation="<<variation<<" and DRCut="<<DRCut
	     <<" truthfatjet "<<truthfatjet->pt()<<" eta="<<truthfatjet->eta()<<" phi="<<truthfatjet->phi()<<" e="<<truthfatjet->e()<<" m="<<truthfatjet->m()
	     <<std::endl;
  }
  bool matched=false;
  for (xAOD::Jet * fatjet : *fatjetHandler->getInParticleVariation("Nominal")) {
    if (!(Props::isFatJet.get(fatjet))) continue;
    double DR=truthfatjet->p4().DeltaR(fatjet->p4());
    if(!(DR<DRCut)) continue;
    if(m_debug) {
      std::cout<<"MatchJet::matchFatJet xAOD::Jet"
	       <<" pt="<<fatjet->pt()<<" eta="<<fatjet->eta()<<" phi="<<fatjet->phi()<<" e="<<fatjet->e()<<" m="<<fatjet->m()
	       <<" DR="<<DR
	       <<std::endl;
    }
    matched |= true;
  }
  Props::passPreSel.set(truthfatjet,matched);
  return EL::StatusCode::SUCCESS;
}
