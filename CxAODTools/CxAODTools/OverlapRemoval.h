// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools__OverlapRemoval_H
#define CxAODTools__OverlapRemoval_H

#include "EventLoop/StatusCode.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/DiTauJetContainer.h"

#include "CxAODTools/ConfigStore.h"

#include "AssociationUtils/DeltaRMatcher.h"

#include "AssociationUtils/OverlapRemovalTool.h"
#include "AssociationUtils/OverlapRemovalInit.h"
#include "AssociationUtils/ToolBox.h"

class OverlapRemovalTool;
//class OverlapRemovalToolVBFGamma;
//class OverlapRemovalToolLargeR

class OverlapRemoval
{
  protected:
    
    ConfigStore m_config; //!
    bool m_debug;
    // if m_applyOverlapRemoval==false -> let all particles pass
    // (just copy input flag -> output flag)
    bool m_applyOverlapRemoval;
    bool m_applyORlargeR;
    bool m_doJetLargeJetOR;
    bool m_applyVBFGammaOR;
    bool m_applyNewToolOR;
    bool m_applyHHbbtautauOR;
    bool m_useDeltaROR;
    bool m_useBTagOR;
    bool m_useTauOR;
    bool m_useTauJetOR;
    bool m_useSetORInputLables;
    bool m_useEleEleOR;
    bool m_doMuPFJetOR;
    bool m_useVarConeEleJetOR;
    bool m_useVarConeMuJetOR;
    ORUtils::DeltaRMatcher * m_dRMatcher;
    OverlapRemovalTool* m_overlapRemovalTool;
    //OverlapRemovalToolVBFGamma* m_overlapRemovalToolVBFGamma;
    ORUtils::ToolBox m_toolBox; 
    ORUtils::ToolBox m_toolBoxLepOnly; 
    
  //ORUtils::ToolHandle<IOverlapRemovalTool>  m_orTool;



  public:
    OverlapRemoval(ConfigStore & config);
    virtual ~OverlapRemoval();

    virtual EL::StatusCode initialize();

    virtual EL::StatusCode removeOverlap(const xAOD::ElectronContainer* electrons,
                                         const xAOD::PhotonContainer* photons,
                                         const xAOD::MuonContainer* muons,
                                         const xAOD::TauJetContainer* taus,
                                         const xAOD::JetContainer* jets,
                                         const xAOD::JetContainer* fatjets,
                                         const xAOD::DiTauJetContainer* ditaus);
    
    virtual void setORInputLabels(const xAOD::ElectronContainer* electrons,
                                         const xAOD::PhotonContainer* photons,
                                         const xAOD::MuonContainer* muons,
                                         const xAOD::TauJetContainer* taus,
                                         const xAOD::JetContainer* jets,
                                         const xAOD::JetContainer* fatjets,
                                         const xAOD::DiTauJetContainer* ditaus);
    
    virtual void getOROutputLabels(const xAOD::ElectronContainer* electrons,
                                         const xAOD::PhotonContainer* photons,
                                         const xAOD::MuonContainer* muons,
                                         const xAOD::TauJetContainer* taus,
                                         const xAOD::JetContainer* jets,
                                         const xAOD::JetContainer* fatjets,
                                         const xAOD::DiTauJetContainer* ditaus);

    virtual void setLepOnlyORInputLabels(const xAOD::ElectronContainer* electrons,
                                         const xAOD::PhotonContainer* photons,
                                         const xAOD::MuonContainer* muons,
                                         const xAOD::TauJetContainer* taus,
                                         const xAOD::JetContainer* jets,
                                         const xAOD::JetContainer* fatjets,
                                         const xAOD::DiTauJetContainer* ditaus);

    virtual void getLepOnlyOROutputLabels(const xAOD::ElectronContainer* electrons,
                                          const xAOD::PhotonContainer* photons,
					  const xAOD::MuonContainer* muons,
                                          const xAOD::TauJetContainer* taus,
                                          const xAOD::JetContainer* jets,
                                          const xAOD::JetContainer* fatjets,
                                          const xAOD::DiTauJetContainer* ditaus);

  
  EL::StatusCode VariableConeOR(const xAOD::ElectronContainer * electrons,
				const xAOD::MuonContainer * muons,
				const xAOD::JetContainer * jets,
				const xAOD::TauJetContainer * taus,
				const xAOD::PhotonContainer * photons);
  
  EL::StatusCode removeEleJetOverlap_VarCone(const xAOD::ElectronContainer * electrons, 
					     const xAOD::JetContainer * jets);

  EL::StatusCode removeMuonJetOverlap_VarCone(const xAOD::MuonContainer * muons, 
					      const xAOD::JetContainer * jets);
  

  EL::StatusCode SetOROutputLabel(const xAOD::JetContainer* jets, 
			      const xAOD::ElectronContainer* Electrons,
			      const xAOD::MuonContainer* Muons);
  
  void SetUseTauJetOR(bool use) {m_useTauJetOR = use;}

};


#endif
