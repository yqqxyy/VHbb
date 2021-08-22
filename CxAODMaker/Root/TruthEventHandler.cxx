#include <iostream>

#include "CxAODMaker/TruthEventHandler.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "CxAODTools/ReturnCheck.h"

TruthEventHandler::TruthEventHandler(const std::string& name, ConfigStore & config,
                                     xAOD::TEvent * event, EventInfoHandler & eventInfoHandler) :
  ObjectHandlerBase(name, config, event),
  m_eventInfoHandler(eventInfoHandler)
{
}


TruthEventHandler::~TruthEventHandler()
{
}

EL::StatusCode TruthEventHandler::initializeTools()
{
  if (m_msgLevel == MSG::DEBUG) {
    Info("TruthEventHandler::initializeTools()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear(); // no systematics for TruthEvent

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TruthEventHandler::setObjects()
{
  if (m_msgLevel == MSG::DEBUG) {
    Info("TruthEventHandler::setObjects()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  if (!m_eventInfoHandler.get_isMC()) {
    // no TruthEvent for data.
    return EL::StatusCode::SUCCESS;
  }

  // define input container
  m_inContainer = nullptr;
  TOOL_CHECK("TruthEventHandler::setObjects()", m_event->retrieve(m_inContainer, "TruthEvents"));

  if (!m_inContainer) {
    Error("TruthEventHandler::setObjects()", "Could not load TruthEventContainer for event. Exiting.");
    return EL::StatusCode::FAILURE;
  }

  if ( m_inContainer->size() != 1 ) {
    Error("TruthEventHandler::setObjects()", "Number of truth events different from 1! Exiting.");
    return EL::StatusCode::FAILURE;
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TruthEventHandler::clearEvent()
{
  if (m_msgLevel == MSG::DEBUG) {
    Info("TruthEventHandler::clearEvent()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  m_inContainer = nullptr;
  m_outContainer = nullptr;
  m_outContainerAux = nullptr;

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode TruthEventHandler::fillOutputContainer() {

  if (!m_eventInfoHandler.get_isMC()) {
    return EL::StatusCode::SUCCESS;
  }

  if (m_msgLevel == MSG::DEBUG) {
    Info("TruthEventHandler::fillOutputContainer()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  // 1. Create the output containers and register them with the event store
  m_outContainer = new xAOD::TruthEventContainer();
  m_outContainerAux = new xAOD::TruthEventAuxContainer();
  m_outContainer->setStore(m_outContainerAux);

  if ( ! m_event->record(m_outContainer, "TruthEvents___Nominal").isSuccess() ) {
    delete m_outContainer;
    delete m_outContainerAux;
    m_outContainer = nullptr;
    m_outContainerAux = nullptr;
    return EL::StatusCode::FAILURE;
  }

  if ( ! m_event->record(m_outContainerAux, "TruthEvents___NominalAux.").isSuccess() ) {
    delete m_outContainerAux;
    m_outContainerAux = nullptr;
    return EL::StatusCode::FAILURE;
  }

  // 2. Fill the output containers
  return setTruthInfo();
}

EL::StatusCode TruthEventHandler::setTruthInfo() {

  if (!m_eventInfoHandler.get_isMC()) {
    return EL::StatusCode::SUCCESS;
  }

  if (m_msgLevel == MSG::DEBUG) {
    Info("TruthEventHandler::setTruthInfo()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  //extract relevant info from input container
  const xAOD::TruthEvent* inTruth = m_inContainer->at(0);

  float x1 = -9999, x2 = -9999, Q = -9999, xf1 = -9999, xf2 = -9999;
  int PDFID1 = -9999, PDFID2 = -9999, PDGID1 = -9999, PDGID2 = -9999;

  static SG::AuxElement::ConstAccessor<float> x1Acc("X1");
  static SG::AuxElement::ConstAccessor<float> x2Acc("X2");
  static SG::AuxElement::ConstAccessor<float> QAcc("Q");
  static SG::AuxElement::ConstAccessor<int> PDGID1Acc("PDGID1");
  static SG::AuxElement::ConstAccessor<int> PDGID2Acc("PDGID2");
  static SG::AuxElement::ConstAccessor<int> PDFID1Acc("PDFID1");
  static SG::AuxElement::ConstAccessor<int> PDFID2Acc("PDFID2");
  static SG::AuxElement::ConstAccessor<float> xf1Acc("XF1");
  static SG::AuxElement::ConstAccessor<float> xf2Acc("XF2");

  if ( x1Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(x1, xAOD::TruthEvent::PdfParam::X1) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::X1");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( x2Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(x2, xAOD::TruthEvent::PdfParam::X2) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::X2");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( QAcc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(Q, xAOD::TruthEvent::PdfParam::Q) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::Q");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( PDGID1Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(PDGID1, xAOD::TruthEvent::PdfParam::PDGID1) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::PDGID1");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( PDGID2Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(PDGID2, xAOD::TruthEvent::PdfParam::PDGID2) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::PDGID2");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( PDFID1Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(PDFID1, xAOD::TruthEvent::PdfParam::PDFID1) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::PDFID1");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( PDFID2Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(PDFID2, xAOD::TruthEvent::PdfParam::PDFID2) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::PDFID2");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( xf1Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(xf1, xAOD::TruthEvent::PdfParam::XF1) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::XF1");
      return EL::StatusCode::FAILURE;
    }
  }

  if ( xf2Acc.isAvailable( *inTruth ) ) {
    if ( ! inTruth->pdfInfoParameter(xf2, xAOD::TruthEvent::PdfParam::XF2) ) {
      Error("TruthEventHandler::setTruthInfo()","Failed to retrieve xAOD::TruthEvent::PdfParam::XF2");
      return EL::StatusCode::FAILURE;
    }
  }

  // define output info
  xAOD::TruthEvent* outTruth = new xAOD::TruthEvent();
  m_outContainer->push_back(outTruth);

  outTruth->setPdfInfoParameter( x1 , xAOD::TruthEvent::PdfParam::X1 );
  outTruth->setPdfInfoParameter( x2 , xAOD::TruthEvent::PdfParam::X2 );
  outTruth->setPdfInfoParameter( xf1 , xAOD::TruthEvent::PdfParam::XF1 );
  outTruth->setPdfInfoParameter( xf1 , xAOD::TruthEvent::PdfParam::XF2 );
  outTruth->setPdfInfoParameter( Q , xAOD::TruthEvent::PdfParam::Q );
  outTruth->setPdfInfoParameter( PDFID1 , xAOD::TruthEvent::PdfParam::PDFID1 );
  outTruth->setPdfInfoParameter( PDFID2 , xAOD::TruthEvent::PdfParam::PDFID2 );
  outTruth->setPdfInfoParameter( PDGID1 , xAOD::TruthEvent::PdfParam::PDGID1 );
  outTruth->setPdfInfoParameter( PDGID2 , xAOD::TruthEvent::PdfParam::PDGID2 );

  return EL::StatusCode::SUCCESS;
}

