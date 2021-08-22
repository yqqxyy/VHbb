#include <iostream>
#include "CxAODTools/CommonProperties.h"

#include "CxAODMaker_VHbb/FatJetHandler_VHbb.h"

static SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer> > ungroomedLink("Parent");
static SG::AuxElement::ConstAccessor<int> partonTruthLabelID("PartonTruthLabelID");

FatJetHandler_VHbb::FatJetHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                             EventInfoHandler & eventInfoHandler) : 
  FatJetHandler(name, config, event, eventInfoHandler),
  m_writePartonTruthLabel(false)
{
  using std::placeholders::_1;
  // clear to get rid of ObjectHandler::passPreSel (all jets pass)
  m_selectFcns.clear();
  // add back in the functions from base class. passPreSel is set there with configurable pT and eta cuts !
  m_selectFcns.push_back(std::bind(&FatJetHandler::passPreSelection, this, _1));
  // add VHbb specific selection function. TODO probably useless
  m_selectFcns.push_back(std::bind( &FatJetHandler_VHbb::passFatJet, this, _1));

  if (m_isMC) {
    m_config.getif<bool>("writePartonTruthIDInFatJets", m_writePartonTruthLabel);
  }
}

bool FatJetHandler_VHbb::passFatJet(xAOD::Jet* jet)
{
  bool passFatCut = false;
  if(jet->pt() > 200000.){
    if(fabs(jet->eta()) < 2.0){
      passFatCut = true;
    }
  }
  Props::isFatJet.set(jet, passFatCut);

  return passFatCut;
}

EL::StatusCode FatJetHandler_VHbb::decorate(xAOD::Jet* jet)
{
  if (m_writePartonTruthLabel && m_isMC) {
    int ptlID = 0;
    auto parent = ungroomedLink(*jet);
    if (parent.isValid()) ptlID = partonTruthLabelID(**parent);
    Props::PartonTruthLabelIDFromUngrJet.set(jet, ptlID);
  }

  return FatJetHandler::decorate(jet);
}

EL::StatusCode FatJetHandler_VHbb::writeCustomVariables(xAOD::Jet* inJet, xAOD::Jet* outJet, bool isKinVar, bool isWeightVar, const TString& /*sysName*/) 
{
  Props::isFatJet.copy(inJet, outJet);
  
  if( !isKinVar && !isWeightVar ){
    Props::nGroomedTracks.copy(inJet, outJet);
    Props::NumTrkPt500PV.copy(inJet, outJet);
    Props::NumTrkPt1000PV.copy(inJet, outJet);
    if(m_saveNonCrucialVariables){
      Props::UngroomedTrackWidth500.copy(inJet, outJet);
      Props::UngroomedTrackWidth1000.copy(inJet, outJet);
    }

    if (m_writePartonTruthLabel) {
      Props::PartonTruthLabelIDFromUngrJet.copy(inJet, outJet);
    }
  }
  return EL::StatusCode::SUCCESS;
}

