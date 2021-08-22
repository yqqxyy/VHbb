// Dear emacs, this is -*-c++-*-

#ifndef CxAODMaker_TruthEventHandler_H
#define CxAODMaker_TruthEventHandler_H

#include "CxAODMaker/ObjectHandlerBase.h"

#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthEventAuxContainer.h"

class EventInfoHandler;

class TruthEventHandler : public ObjectHandlerBase {

public:

  TruthEventHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event, EventInfoHandler & eventInfoHandler);

  ~TruthEventHandler();

  virtual EL::StatusCode setObjects() override;
  virtual EL::StatusCode fillOutputContainer() override;
  virtual EL::StatusCode clearEvent() override;

  virtual EL::StatusCode initializeTools() override;

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

protected:
  virtual EL::StatusCode setTruthInfo(void);

  EventInfoHandler & m_eventInfoHandler;
  const xAOD::TruthEventContainer* m_inContainer;
  xAOD::TruthEventContainer* m_outContainer;
  xAOD::TruthEventAuxContainer* m_outContainerAux;

};

#endif
