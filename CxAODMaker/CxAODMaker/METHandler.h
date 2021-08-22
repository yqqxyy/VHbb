// Dear emacs, this is -*-c++-*-

#ifndef CxAODMaker_METHandler_H
#define CxAODMaker_METHandler_H

#include "CxAODMaker/ObjectHandlerBase.h"
#include "AsgTools/AnaToolHandle.h"

#ifndef __MAKECINT__
#include "METInterface/IMETRebuilder.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#endif

//#include "METUtilities/METRebuilder.h"
#include "METInterface/IMETMaker.h"
#include "METInterface/IMETSignificance.h"
#include "METUtilities/METSystematicsTool.h"

class IMETRebuilder;

// Forward declarations
class JetHandler;
class MuonHandler;
class TauHandler;
class ElectronHandler;
class PhotonHandler;
class EventInfoHandler;

class METHandler : public ObjectHandlerBase {
public:

  METHandler(const std::string& name, ConfigStore & config, xAOD::TEvent *event, EventInfoHandler &eventInfoHandler);

  ~METHandler();

  virtual void setJets(JetHandler* jets) { m_jetHandler = jets; }
  virtual void setMuons(MuonHandler* muons) { m_muonHandler = muons; }
  virtual void setTaus(TauHandler* taus) { m_tauHandler = taus; }
  virtual void setElectrons(ElectronHandler* electrons) { m_electronHandler = electrons; }
  virtual void setPhotons(PhotonHandler* photons) { m_photonHandler = photons; }

  virtual EL::StatusCode initializeTools() override;
  virtual EL::StatusCode addParticleVariations();
  virtual EL::StatusCode setObjects() override;
  virtual EL::StatusCode setMET();
  virtual EL::StatusCode fillOutputContainer() override;
  virtual EL::StatusCode clearEvent() override;

  virtual xAOD::MissingET* getMET(const TString &variation);

  void printMET(const xAOD::MissingETContainer* METCont);

  // Some methods required by ObjectHandlerBase do not make sense for
  // METHandler -> implement empty methods here.
  EL::StatusCode calibrate() override       {return EL::StatusCode::SUCCESS;}
  EL::StatusCode select()    override       {return EL::StatusCode::SUCCESS;}
  EL::StatusCode fillOutputLinks() override {return EL::StatusCode::SUCCESS;}
  EL::StatusCode flagOutputLinks() override {return EL::StatusCode::SUCCESS;}
  EL::StatusCode addCutflows()     override {return EL::StatusCode::SUCCESS;}
  void setLinker(ParticleLinker*) {;}
  void countObjects() override {;}
  void printObjectCounts() override {;}
  const std::map<TString, CutFlowCounter*> getCutFlowCounter() const override
  { return std::map<TString, CutFlowCounter*>(); }

  // provide functionality to retrieve containers with same interface as for an ObjectHandler
  xAOD::MissingETContainer* getInParticleVariation(const TString & variation) {
    return getParticleVariation(m_inContainer, variation);
  }
  xAOD::MissingETContainer* getOutParticleVariation(const TString & variation) {
    return getParticleVariation(m_outContainer, variation);
  }


protected:

  virtual EL::StatusCode calibrateMET(xAOD::MissingETContainer* METCont, TString sysName, int isyst);
  virtual void copyContainer(xAOD::MissingETContainer* inCont, xAOD::MissingETContainer* outCont, bool isNominal);
  xAOD::MissingETContainer* getParticleVariation(std::map<TString, xAOD::MissingETContainer *> &container, const TString &variation);

  EventInfoHandler & m_eventInfoHandler;

  //for MET rebuilding
  std::string m_eleTerm;
  std::string m_gammaTerm;
  std::string m_tauTerm;
  std::string m_jetTerm;
  std::string m_jetTrkTerm;
  std::string m_muonTerm;
  std::string m_softTerm; //always set to "PVSoftTrk" for TST MET (only supported MET soft term in Run2)
  bool m_selObjForMET;    //steered by config
  bool m_rebuiltMETforOriginal; //steered by config
  std::string m_inputAssocMap;
  std::string m_inputMETCoreCont;
  std::string m_outMETTerm;

  JetHandler* m_jetHandler;
  MuonHandler* m_muonHandler;
  ElectronHandler* m_electronHandler;
  PhotonHandler* m_photonHandler;
  TauHandler* m_tauHandler;

  bool m_doRebuild;
  bool m_saveMETSoft;

  // MET significance tool
  asg::AnaToolHandle<IMETSignificance> m_metSignif;
  asg::AnaToolHandle<IMETSignificance> m_metSignif_PU;

  // MET utilities tool
  asg::AnaToolHandle<IMETMaker> m_metMaker;
  //asg::AnaToolHandle<IMETSystematicsTool> m_metSysTool;
  met::METSystematicsTool* m_metSysTool;

  // new MET containers for calibration
  std::map< TString, xAOD::MissingETContainer* > m_inContainer;
  // selected MET terms for output xAOD
  std::map< TString, xAOD::MissingETContainer* > m_outContainer;

  // MetSig containers
  std::map< TString, float > m_metSig;
  std::map< TString, float > m_metSig_PU;
  std::map< TString, float > m_metOverSqrtSumET;
  std::map< TString, float > m_metOverSqrtHT;

};
#endif
