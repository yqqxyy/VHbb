#include <iostream>

// photon specific includes
#include "CxAODMaker_VHbb/PhotonHandler_VGamma.h"

PhotonHandler_VGamma::PhotonHandler_VGamma(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                             EventInfoHandler & eventInfoHandler) :
  PhotonHandler(name, config, event, eventInfoHandler)
{
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind( &PhotonHandler_VGamma::passVBFLoosePhoton, this, _1));
  m_selectFcns.push_back(std::bind( &PhotonHandler_VGamma::passVBFSignalPhoton, this, _1));
}

bool PhotonHandler_VGamma::passVBFLoosePhoton(xAOD::Photon * photon)
{

  int loose = Props::isLoose.get(photon);

  bool passSel = true;
  if (passSel) m_cutflow->count("VBFLoosePhoton input", 100);

  if(! ( (photon->author() & xAOD::EgammaParameters::AuthorPhoton) ||
	 (photon->author() & xAOD::EgammaParameters::AuthorAmbiguous) ) ) passSel = false;
  if (passSel) m_cutflow->count("author");

  if( ! photon->isGoodOQ(xAOD::EgammaParameters::BADCLUSPHOTON) ) passSel = false;
  if (passSel) m_cutflow->count("OQ");

  // photon clearning required by https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaIdentificationRun2#Photon_cleaning
  //if(  (photon->OQ()&134217728)!=0 &&
	//	   (photon->showerShapeValue(xAOD::EgammaParameters::Reta) >0.98
	//		||photon->showerShapeValue(xAOD::EgammaParameters::Rphi) >1.0
	//		||(photon->OQ()&67108864)!=0) ) passSel = false;

  passSel = static_cast<bool>(photon->auxdata<char>("DFCommonPhotonsCleaning")); 

  if (passSel) m_cutflow->count("photon cleaning");

  if(!loose) passSel = false;
  if(passSel) m_cutflow->count("loose");

  double eta = fabs(photon->caloCluster()->etaBE(2));
  if(! (eta < 2.37) ) passSel = false;
  if( (eta > 1.37) && (eta < 1.52) ) passSel = false;
  if (passSel) m_cutflow->count("|eta|");

  if(!(photon->pt() > 15000.)) passSel = false; //dummy value for testing
  if (passSel) m_cutflow->count("pt loose");

  Props::isVBFLoosePhoton.set(photon, passSel);
  Props::passPreSel.set(photon, passSel);
  if (passSel) m_cutflow->count("VBFLoosePhoton selected");

  return passSel;

}


bool PhotonHandler_VGamma::passVBFSignalPhoton(xAOD::Photon * photon)
{

  bool passSel = true;

  m_ptcut = 250000;
  m_etacut = 2.5;

  m_config.getif<double>("Photons::Pt", m_ptcut);
  m_config.getif<double>("Photons::Eta", m_etacut);

  if (!Props::isVBFLoosePhoton.get(photon)) passSel = false;
  if (passSel) m_cutflow->count("VBFSignalPhoton input", 200);
 
  if(!(Props::isLoose.get(photon))) passSel = false;
  if(passSel) m_cutflow->count("loose");

  if(!(photon->pt() > m_ptcut)) passSel = false;
  if (passSel) m_cutflow->count("pt signal");
  double eta = fabs(photon->caloCluster()->etaBE(2));
  if(!(eta < m_etacut)) passSel = false;

  //if(!(Props::isFixedCutTightCaloOnlyIso.get(photon))) passSel = false;

  Props::isVBFSignalPhoton.set(photon, passSel);

  if (passSel) m_cutflow->count("VBFSignalPhoton selected");
  return passSel;

}


EL::StatusCode PhotonHandler_VGamma::writeCustomVariables(xAOD::Photon * inPhoton, xAOD::Photon * outPhoton, bool /*isKinVar*/, bool isWeightVar, const TString& /*sysName*/)
{
  if(!isWeightVar) {
    Props::isVBFLoosePhoton.copy(inPhoton, outPhoton);
    Props::isVBFSignalPhoton.copy(inPhoton, outPhoton);
  }

  return EL::StatusCode::SUCCESS;

}
