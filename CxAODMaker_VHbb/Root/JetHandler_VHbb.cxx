#include <iostream>

// jet specific includes
#include "CxAODMaker_VHbb/JetHandler_VHbb.h"

JetHandler_VHbb::JetHandler_VHbb(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                       EventInfoHandler & eventInfoHandler) :
  JetHandler(name, config, event, eventInfoHandler),
  m_btagCut(1.)

{
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind( &JetHandler_VHbb::passVetoJet, this, _1));
  m_selectFcns.push_back(std::bind( &JetHandler_VHbb::passSignalJet, this, _1));

  m_config.getif<float>("btagCut", m_btagCut);
  
}


bool JetHandler_VHbb::passVetoJet(xAOD::Jet* jet)
{
  bool passCentral = checkCentralJet(jet,true);
  
  bool passForward = true;
  if(passForward) m_cutflow->count("Forward jet input",200);
  if (!(jet->pt() > 30000.)) passForward = false;
  if (passForward) m_cutflow->count("Forward jet pt",201);
  if (!(fabs(jet->eta()) >= 2.5)) passForward = false;
  if (!(fabs(jet->eta()) < 4.5)) passForward = false;
  if (passForward) m_cutflow->count("Forward jet |eta|",202);

  if (passForward) m_cutflow->count("Forward jet selected",203);
  
  bool passSel = true;
  if (passSel) m_cutflow->count("VetoJet input", 300);

  // ok, I'll destroy the nice cutflow, but it's to fix a bug on badJets not being passed to OR
  //if (!(Props::goodJet.get(jet))) passSel = false;
  //if (passSel) m_cutflow->count("GoodJet");

  // JVT TWiki with recommendation: https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JVTCalibration
  if(!(Props::PassJvtMedium.get(jet))) passSel = false;
  if (passSel) m_cutflow->count("JVT");
  if (!(passCentral || passForward)) passSel = false;
  if (passSel) m_cutflow->count("CentralOrForwardJet");
  
  // passPreSel must be set BEFORE the goodJet requirement
  Props::passPreSel.set(jet, passSel);

  if (!(Props::goodJet.get(jet))) passSel = false;
  if (passSel) m_cutflow->count("GoodJet");

  Props::isVetoJet.set(jet, passSel);
  if (passSel) m_cutflow->count("VetoJet selected");
  Props::isVBFJet.set(jet, passSel);

  return passSel;
}

bool JetHandler_VHbb::passSignalJet(xAOD::Jet* jet)
{
  
  bool passSel = true;
  if (!(Props::isVetoJet.get(jet))) passSel = false;
  if (passSel) m_cutflow->count("SignalJet input", 400);
  if (!(checkCentralJet(jet,false))) passSel = false;
  if (passSel) m_cutflow->count("IsCentral");
  
  Props::isSignalJet.set(jet, passSel);
  Props::isVBFSignalJet.set(jet, passSel);
  // for vbf + photon OR-----------------
  if(m_btagCut<1.){
    if(passSel && jet->pt()>40000. && Props::MV2c10.get(jet) > m_btagCut)
      Props::isBTag.set(jet, true);
    else
      Props::isBTag.set(jet, false);
  }
  //-------------------------------------
  if (passSel) m_cutflow->count("SignalJet selected");
  return passSel;
}

bool JetHandler_VHbb::checkCentralJet(xAOD::Jet* jet,bool isInCutFlow)
{
  bool passSel = true;
  if (passSel && isInCutFlow ) m_cutflow->count("Central jet input",100);
  if (!(jet->pt() > 20000.)) passSel = false;  
  if (jet->pt() > 20000. && isInCutFlow ) m_cutflow->count("Central jet pt");
  if (!(fabs(jet->eta()) < 2.5)) passSel = false;
  if (fabs(jet->eta()) < 2.5 && isInCutFlow ) m_cutflow->count("Central jet |eta|");

  if (passSel&& isInCutFlow ) m_cutflow->count("Central jet selected");
  
  return passSel;
}
  
EL::StatusCode JetHandler_VHbb::writeCustomVariables(xAOD::Jet * inJet, xAOD::Jet * outJet, bool /*isKinVar*/, bool isWeightVar, const TString& /*sysName*/)
{

  if(!isWeightVar){
    Props::isVetoJet.copy(inJet, outJet);
    Props::isSignalJet.copy(inJet, outJet);
    if(m_saveNonCrucialVariables){
      Props::isVBFJet.copy(inJet, outJet);
      Props::isVBFSignalJet.copy(inJet, outJet);
      Props::isBTag.copyIfExists(inJet, outJet);
    }
  }
  
  return EL::StatusCode::SUCCESS;
 }
