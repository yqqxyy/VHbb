#include "CxAODMaker/ElectronHandler.h"
#include "CxAODMaker/PhotonHandler.h"
#include "CxAODMaker/MuonHandler.h"
#include "CxAODMaker/JetHandler.h"
#include "CxAODMaker/FatJetHandler.h"
#include "CxAODMaker/TauHandler.h"
#include "CxAODMaker/DiTauJetHandler.h"

#include "EventLoop/StatusCode.h"   
#include "CxAODTools/ConfigStore.h"
#include "CxAODTools/OverlapRemoval.h"

#include "CxAODMaker/OverlapRemover.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/ReturnCheck.h"

OverlapRemover::OverlapRemover(ConfigStore & config) :
  m_config(config),
  m_jets(nullptr),
  m_fatjets(nullptr),
  m_muons(nullptr),
  m_taus(nullptr),
  m_ditaus(nullptr),
  m_electrons(nullptr),
  m_photons(nullptr),
  m_OR(nullptr)
{
 m_config.getif<bool>("debug", m_debug);
}


OverlapRemover::~OverlapRemover()
{
  if (m_OR) {
    delete m_OR;
  }
}


EL::StatusCode OverlapRemover::removeOverlap(const TString& sysName)
{
  if(m_debug) Info("OverlapRemover::removeOverlap", "...");

  if (!m_OR) {
    Error("OverlapRemover::removeOverlap", "OverlapRemoval was not initialized!");
    return EL::StatusCode::FAILURE;
  }
  
  // first, retrieve the containers. Always check for nullptr, in case some analyses don't use some
  // containers
  xAOD::JetContainer* jets = nullptr;
  if(m_jets) { jets = m_jets->getInParticleVariation(sysName); }
  xAOD::JetContainer* fatjets = nullptr;
  if(m_fatjets) { fatjets = m_fatjets->getInParticleVariation(sysName); }
  xAOD::ElectronContainer* electrons = nullptr;
  if(m_electrons) { electrons = m_electrons->getInParticleVariation(sysName); }
  xAOD::PhotonContainer* photons = nullptr;
  if(m_photons) { photons = m_photons->getInParticleVariation(sysName); }
  xAOD::MuonContainer* muons = nullptr;
  if(m_muons) { muons = m_muons->getInParticleVariation(sysName); }
  xAOD::TauJetContainer* taus = nullptr;
  if(m_taus) { taus = m_taus->getInParticleVariation(sysName); }
  xAOD::DiTauJetContainer* ditaus = nullptr;
  if(m_ditaus) { ditaus = m_ditaus->getInParticleVariation(sysName); }
  // next, perform the actual OR on the objects.
  // This step set the decorators "passOR" on the objects

  EL_CHECK("OverlapRemover",m_OR->removeOverlap(electrons, photons, muons, taus, jets, fatjets, ditaus));
 
  // Now, set the "passORGlob" flags which track if at least once the OR has been passed for the
  // different cases with the same container (e.g Electron Nominal container used for all jet systematics)

  if(electrons) {
    for(xAOD::Electron* el : *electrons) {
      int passORGlob = 0;
      Props::passORGlob.get(el, passORGlob);
      Props::passORGlob.set(el, passORGlob | Props::passOR.get(el));
    }
  }

  if(photons) {
    for(xAOD::Photon* ph : *photons) {
      int passORGlob = 0;
      Props::passORGlob.get(ph, passORGlob);
      Props::passORGlob.set(ph, passORGlob | Props::passOR.get(ph));
    }
  }

  if(muons) {
    for(xAOD::Muon* mu : *muons) {
      int passORGlob = 0;
      Props::passORGlob.get(mu, passORGlob);
      Props::passORGlob.set(mu, passORGlob | Props::passOR.get(mu));
    }
  }

  if(taus) {
    for(xAOD::TauJet* tau : *taus) {
      int passORGlob = 0;
      Props::passORGlob.get(tau, passORGlob);
      Props::passORGlob.set(tau, passORGlob | Props::passOR.get(tau));
    }
  }

  if(jets) {
    for(xAOD::Jet* jet : *jets) {
      int passORGlob = 0;
      Props::passORGlob.get(jet, passORGlob);
      Props::passORGlob.set(jet, passORGlob | Props::passOR.get(jet));
    }
  }

  if(fatjets) {
    for(xAOD::Jet* jet : *fatjets) {
      int passORGlob = 0;
      Props::passORGlob.get(jet, passORGlob);
      Props::passORGlob.set(jet, passORGlob | Props::passOR.get(jet));
    }
  }

  if(ditaus) {
    for(xAOD::DiTauJet* ditau : *ditaus) {
      int passORGlob = 0;
      Props::passORGlob.get(ditau, passORGlob);
      Props::passORGlob.set(ditau, passORGlob | Props::passOR.get(ditau));
    }
  }

  // if we managed to reach this, everything went fine
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode OverlapRemover::setAllObjectsFail(const TString& sysName)
{
  if(m_debug) Info("OverlapRemover::selAllObjectsFail", "...");
  
  // first, retrieve the containers. Always check for nullptr, in case some analyses don't use some
  // containers
  xAOD::JetContainer* jets = nullptr;
  if(m_jets) { jets = m_jets->getInParticleVariation(sysName); }
  xAOD::JetContainer* fatjets = nullptr;
  if(m_fatjets) { fatjets = m_fatjets->getInParticleVariation(sysName); }
  xAOD::ElectronContainer* electrons = nullptr;
  if(m_electrons) { electrons = m_electrons->getInParticleVariation(sysName); }
  xAOD::PhotonContainer* photons = nullptr;
  if(m_photons) { photons = m_photons->getInParticleVariation(sysName); }
  xAOD::MuonContainer* muons = nullptr;
  if(m_muons) { muons = m_muons->getInParticleVariation(sysName); }
  xAOD::TauJetContainer* taus = nullptr;
  if(m_taus) { taus = m_taus->getInParticleVariation(sysName); }
  xAOD::DiTauJetContainer* ditaus = nullptr;
  if(m_ditaus) { ditaus = m_ditaus->getInParticleVariation(sysName); }
  // next, perform the actual OR on the objects.
  // This step set the decorators "passOR" on the objects
 
  // Now, set the "passORGlob" flags which track if at least once the OR has been passed for the
  // different cases with the same container (e.g Electron Nominal container used for all jet systematics)

  if(electrons) {
    for(xAOD::Electron* el : *electrons) {
      Props::passOR.set(el, false);
      int passORGlob = 0;
      Props::passORGlob.get(el, passORGlob);
      Props::passORGlob.set(el, passORGlob | Props::passOR.get(el));
    }
  }

  if(photons) {
    for(xAOD::Photon* ph : *photons) {
      Props::passOR.set(ph, false);
      int passORGlob = 0;
      Props::passORGlob.get(ph, passORGlob);
      Props::passORGlob.set(ph, passORGlob | Props::passOR.get(ph));
    }
  }

  if(muons) {
    for(xAOD::Muon* mu : *muons) {
      Props::passOR.set(mu, false);
      int passORGlob = 0;
      Props::passORGlob.get(mu, passORGlob);
      Props::passORGlob.set(mu, passORGlob | Props::passOR.get(mu));
    }
  }

  if(taus) {
    for(xAOD::TauJet* tau : *taus) {
      Props::passOR.set(tau, false);
      int passORGlob = 0;
      Props::passORGlob.get(tau, passORGlob);
      Props::passORGlob.set(tau, passORGlob | Props::passOR.get(tau));
    }
  }

  if(jets) {
    for(xAOD::Jet* jet : *jets) {
      Props::passOR.set(jet, false);
      int passORGlob = 0;
      Props::passORGlob.get(jet, passORGlob);
      Props::passORGlob.set(jet, passORGlob | Props::passOR.get(jet));
    }
  }

  if(fatjets) {
    for(xAOD::Jet* jet : *fatjets) {
      Props::passOR.set(jet, false);
      int passORGlob = 0;
      Props::passORGlob.get(jet, passORGlob);
      Props::passORGlob.set(jet, passORGlob | Props::passOR.get(jet));
    }
  }

  if(ditaus) {
    for(xAOD::DiTauJet* ditau : *ditaus) {
      int passORGlob = 0;
      Props::passORGlob.get(ditau, passORGlob);
      Props::passORGlob.set(ditau, passORGlob | Props::passOR.get(ditau));
    }
  }

  // if we managed to reach this, everything went fine
  return EL::StatusCode::SUCCESS;
}

