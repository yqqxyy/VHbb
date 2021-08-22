#ifndef CxAODMaker_OverlapRemover_H
#define CxAODMaker_OverlapRemover_H

class ElectronHandler;
class PhotonHandler;
class MuonHandler;
class JetHandler;
class FatJetHandler;
class TauHandler;
class DiTauJetHandler;

class ConfigStore;
class OverlapRemoval;

class OverlapRemover {

protected:

  ConfigStore & m_config;
  bool m_debug;

  JetHandler      * m_jets;
  FatJetHandler   * m_fatjets;
  MuonHandler     * m_muons;
  TauHandler     * m_taus;
  DiTauJetHandler * m_ditaus;
  ElectronHandler * m_electrons;
  PhotonHandler   * m_photons;

  OverlapRemoval* m_OR;

public:

  OverlapRemover(ConfigStore & config);

  virtual ~OverlapRemover();
  
  virtual void setOverlapRemoval(OverlapRemoval* OR) { m_OR = OR; }

  virtual void setJets(JetHandler* jets) { m_jets = jets; }

  virtual void setFatJets(FatJetHandler* jets) { m_fatjets = jets; }

  virtual void setMuons(MuonHandler* muons) { m_muons = muons; }

  virtual void setElectrons(ElectronHandler* electrons) { m_electrons = electrons; }

  virtual void setPhotons(PhotonHandler* photons) { m_photons = photons; }

  virtual void setTaus(TauHandler* taus) { m_taus = taus; }

  virtual void setDiTaus(DiTauJetHandler* ditaus){ m_ditaus = ditaus; }

  virtual EL::StatusCode removeOverlap(const TString& sysName);

  virtual EL::StatusCode setAllObjectsFail(const TString& sysName);

};

#endif
