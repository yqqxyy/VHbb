// Dear emacs, this is -*-c++-*-

#ifndef CxAODMaker_DiTauJetHandler_H
#define CxAODMaker_DiTauJetHandler_H

#include "CxAODMaker/ObjectHandler.h"
#include "CxAODTools/CommonProperties.h"

#include "xAODTau/DiTauJetContainer.h"
//#include "CxAODMaker/TruthParticleHandler.h"
//#include "TrigTauMatching/TrigTauMatching.h"
//#include <TauAnalysisTools/TauSmearingTool.h>
//#include <AsgTools/AnaToolHandle.h>
//#include <vector>

/*namespace xAOD {
#ifndef XAODDITAU_DITAUJETCONTAINER_H 
  class DiTauJetContainer;
#endif
#ifndef XAODDITAU_DITAUJET_H 
  class DiTauJet;
#endif
}

namespace xAOD {
#ifndef XAODDITAUJETCONTAINER_H
  class DiTauJetJetContainer;
#endif
#ifndef XAODDITAU_DITAUJET_H
  class DiTauJet;
#endif
}*/

namespace tauRecTools {
  class DiTauIDVarCalculator;
  class DiTauDiscriminantTool;
}

class DiTauJetHandler : public ObjectHandler<xAOD::DiTauJetContainer> {

public:

  DiTauJetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
              EventInfoHandler & eventInfoHandler);
  
  virtual ~DiTauJetHandler();
  
  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode clearEvent() override;
  //virtual void setTruthHandler(TruthParticleHandler* truthhandler ) { m_truthhandler=truthhandler; }
  
  // Setting tau mass to 0 for now as it was done in Run 1
  void setP4(const xAOD::DiTauJet * in, xAOD::DiTauJet * out) override { out->setP4(in->pt(), in->eta(), in->phi(), in->m()); }

  //for fake control    
  //bool m_antitau;

protected:
  
  // calibration to ols
  //asg::AnaToolHandle<TauAnalysisTools::ITauSmearingTool> * m_tauSmearingTool;
  /*TauAnalysisTools::TauSmearingTool * m_tauSmearingTool;
  TauAnalysisTools::TauSelectionTool  * m_tauSelectionTool;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauEffCorrLoose;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauEffCorrMedium;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolMedium25;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolMedium35;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolMedium80;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolMedium125;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolLoose25;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolLoose35;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolLoose80;
  TauAnalysisTools::TauEfficiencyCorrectionsTool * m_tauTriggerEffToolLoose125;

  TauAnalysisTools::TauTruthMatchingTool * m_tauTruthMatchingTool;*/

  tauRecTools::DiTauIDVarCalculator * m_IDVarCalculator;
  tauRecTools::DiTauDiscriminantTool * m_DiscrTool;

  virtual EL::StatusCode addCPVariations(const std::vector<TString> &variations, 
                                         const bool filterByTools = true, const bool skipWeightVar = false) override;

  // selection functions
  //bool passLooseTau(xAOD::DiTauJet * tau);
  virtual EL::StatusCode decorateOriginParticle(const xAOD::DiTauJet * tau) override;
  virtual EL::StatusCode decorate(xAOD::DiTauJet * tau) override;
  
  virtual EL::StatusCode calibrateCopies(xAOD::DiTauJetContainer * particles, const CP::SystematicSet & sysSet) override;

 /** @brief Sets certain variables for the outTau.
    * @param inTau Tau from m_inContainer, from which values are read.
    * @param outTau Tau from m_outContainer for which values are set.
    * @param isSysVar Bool deciding if container holding outTau is a shallow copy.
    *
    *Called in function copyContainer that is implemented in the ObjectHandler.
    *In case isSysVar is true, variation dependent object variables are overwritten.
   */
  virtual EL::StatusCode writeOutputVariables(xAOD::DiTauJet * inTau, xAOD::DiTauJet * outTau, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeAllVariations(xAOD::DiTauJet* inTau, xAOD::DiTauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeKinematicVariations(xAOD::DiTauJet* inTau, xAOD::DiTauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::DiTauJet* inTau, xAOD::DiTauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::DiTauJet* inTau, xAOD::DiTauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::DiTauJet * inTau, xAOD::DiTauJet * outTau, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  
  //TruthParticleHandler* m_truthhandler;
  //EL::StatusCode TruthMatch(xAOD::DiTauJet * tau);
  //EL::StatusCode TruthMatchTAT(xAOD::DiTauJet * tau);
  //EL::StatusCode FindTrueTaus();
  //bool m_SearchedTrueTaus;
  //bool m_runTruthMatchTool;
  //std::vector<xAOD::TruthParticle*> m_truetaus;
  //std::vector<xAOD::TruthParticle*> m_truetaus_had;
  //std::vector<xAOD::TruthParticle*> m_truetaus_lep;
  //std::vector<xAOD::TruthParticle*> m_trueeles;
  //std::vector<xAOD::TruthParticle*> m_truemuons;
  //std::vector<xAOD::TruthParticle*> m_truepartons;

  //for including taus in MET calculation
  //bool m_useTausInMET;

  //Trig::TrigDecisionTool *m_trigDecTool; 
  //PUReweightingTool *m_PUReweightingTool;

  //bool m_suppressTauCalibration;
  //bool m_doTrigMatch;
  
  //std::unordered_map<std::string, PROP<int>*> m_triggersToMatch;

//#ifndef __MAKECINT__
//  Trig::TrigTauMatchingTool *m_match_Tool; //!
//#endif // not __MAKECINT__


};

#endif

