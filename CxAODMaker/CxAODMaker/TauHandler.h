// Dear emacs, this is -*-c++-*-

#ifndef CxAODMaker_TauHandler_H
#define CxAODMaker_TauHandler_H

#include "CxAODMaker/ObjectHandler.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

class TruthProcessor;

namespace TauAnalysisTools {
  class TauSmearingTool;
  class TauSelectionTool;
  class TauEfficiencyCorrectionsTool;
  class TauTruthMatchingTool;
}

namespace Trig {
  class TrigDecisionTool;
  class TrigTauMatchingTool;
}

class TauHandler : public ObjectHandler<xAOD::TauJetContainer> {

public:

  TauHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
              EventInfoHandler & eventInfoHandler);
  
  virtual ~TauHandler();
  
  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode clearEvent() override;
  virtual void setTruthProcessor(TruthProcessor* truthproc ) { m_truthproc=truthproc; }
  virtual EL::StatusCode fillOutputLinks();
  
  // Setting tau mass to 0 for now as it was done in Run 1
  void setP4(const xAOD::TauJet * in, xAOD::TauJet * out) override { out->setP4(in->pt(), in->eta(), in->phi(), 0); }

  //for fake control    
  bool m_antitau;

protected:
  // Tau-ID configuration
  int m_TAT_IDLevel;
  int m_TAT_IDLevel_AntiTau;
  xAOD::TauJetParameters::IsTauFlag m_xAOD_IDLevel;
  xAOD::TauJetParameters::IsTauFlag m_xAOD_IDLevel_AntiTau;

  // calibration tools
  std::unique_ptr<TauAnalysisTools::TauSmearingTool> m_tauSmearingTool;
  std::unique_ptr<TauAnalysisTools::TauSelectionTool> m_tauSelectionTool;
  std::unique_ptr<TauAnalysisTools::TauTruthMatchingTool> m_tauTruthMatchingTool;
  std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool> m_tauEffCorr;
  std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool> m_tauTriggerEffTool25;
  std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool> m_tauTriggerEffTool35;
  std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool> m_tauTriggerEffTool80;
  std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool> m_tauTriggerEffTool125;
  std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool> m_tauTriggerEffTool160;

  virtual EL::StatusCode addCPVariations(const std::vector<TString> &variations, 
                                         const bool filterByTools = true, const bool skipWeightVar = false) override;

  // selection functions
  bool passLooseTau(xAOD::TauJet * tau);

  virtual EL::StatusCode decorateOriginParticle(const xAOD::TauJet * tau) override;
  virtual EL::StatusCode decorate(xAOD::TauJet * tau) override; 
  
  virtual EL::StatusCode calibrateCopies(xAOD::TauJetContainer * particles, const CP::SystematicSet & sysSet) override;

  /** @brief Sets certain variables for the outTau.
    * @param inTau Tau from m_inContainer, from which values are read.
    * @param outTau Tau from m_outContainer for which values are set.
    * @param isSysVar Bool deciding if container holding outTau is a shallow copy.
    *
    *Called in function copyContainer that is implemented in the ObjectHandler.
    *In case isSysVar is true, variation dependent object variables are overwritten.
   */
  virtual EL::StatusCode writeOutputVariables(xAOD::TauJet * inTau, xAOD::TauJet * outTau, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  virtual EL::StatusCode writeAllVariations(xAOD::TauJet* inTau, xAOD::TauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeKinematicVariations(xAOD::TauJet* inTau, xAOD::TauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeWeightVariations(xAOD::TauJet* inTau, xAOD::TauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeNominal(xAOD::TauJet* inTau, xAOD::TauJet* outTau, const TString& sysName);
  virtual EL::StatusCode writeCustomVariables(xAOD::TauJet * inTau, xAOD::TauJet * outTau, bool isKinVar, bool isWeightVar, const TString& sysName) override;
  
  TruthProcessor* m_truthproc;
  EL::StatusCode TruthMatch(xAOD::TauJet * tau);
  EL::StatusCode TruthMatchTAT(xAOD::TauJet * tau);
  EL::StatusCode FindTrueTaus();
  bool m_SearchedTrueTaus;
  bool m_runTruthMatchTool;
  std::vector<const xAOD::TruthParticle*> m_truetaus;
  std::vector<const xAOD::TruthParticle*> m_truetaus_had;
  std::vector<const xAOD::TruthParticle*> m_truetaus_lep;
  std::vector<const xAOD::TruthParticle*> m_trueeles;
  std::vector<const xAOD::TruthParticle*> m_truemuons;
  std::vector<const xAOD::TruthParticle*> m_truepartons;

  //for including taus in MET calculation
  bool m_useTausInMET;

  Trig::TrigDecisionTool *m_trigDecTool; 

  std::string m_tauIDWP;
  bool m_writeRNNVars;
  bool m_useRNNTaus;
  bool m_suppressTauCalibration;
  bool m_doTrigMatch;
  bool m_SkipTruthMatch;
  std::unordered_map<std::string, PROP<int>*> m_triggersToMatch;

#ifndef __MAKECINT__
  Trig::TrigTauMatchingTool *m_match_Tool; //!
#endif // not __MAKECINT__


};

#endif

