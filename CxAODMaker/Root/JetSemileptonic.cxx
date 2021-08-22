#include "PATInterfaces/ISystematicsTool.h" 
#include "EventLoop/StatusCode.h"
#include "CxAODTools/PtRecoTool.h"
#include "MuonSelectorTools/MuonSelectionTool.h"

#include "CxAODMaker/JetSemileptonic.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/ReturnCheck.h"

#include "PathResolver/PathResolver.h" 



JetSemileptonic::JetSemileptonic():
  m_PtRecoTool(nullptr),
  m_muonSelectionTool(nullptr),
  m_debug(false),
  m_msgLevel(MSG::WARNING)
{
}

JetSemileptonic::~JetSemileptonic() {
  delete m_muonSelectionTool;
}

//this initializes the tmva tools
EL::StatusCode JetSemileptonic::initialize(bool debug) {

  m_debug=debug;
  if (m_debug) {
    m_msgLevel = MSG::DEBUG;
  } else {
    m_msgLevel = MSG::INFO;
  }

  //PtRecoTool                                                                                                                                                                 
  m_PtRecoTool = new PtRecoTool();
  EL_CHECK("JetSemileptonic::initialize()", m_PtRecoTool->initialize(m_debug));
  
  // initialize the muon selector tool
  //---------------------------------------------------
  m_muonSelectionTool = new CP::MuonSelectionTool("MuonSelectionToolForJetSemileptonic");
  m_muonSelectionTool->msg().setLevel( m_msgLevel );
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionTool->setProperty("MaxEta", 2.7)); //2.7 should be default, but better to state it explicitly if later we want to change
  // Setting the tool accept flag to Loose, these muons will be used for sys calibration
  // Possible values: Tight, Medium, Loose, VeryLoose
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionTool->setProperty("MuQuality", (int) xAOD::Muon::Quality::Medium));
  TOOL_CHECK("MuonHandler::initializeTools()",m_muonSelectionTool->initialize());
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetSemileptonic::test() {
  if(m_debug) { std::cout<<"JetSemileptonic::test()"<<std::endl; }
  return EL::StatusCode::SUCCESS;
}

bool JetSemileptonic::passForJetElectron(const xAOD::Electron* electron)
{
  if(m_debug) { std::cout<<"JetSemileptonic::passForJetElectron"<<std::endl; }
  bool passSel = true;
  if (!Props::isLooseLH.get(electron)) passSel = false;
  if(electron->caloCluster()){ 
    if (!(fabs(electron->caloCluster()->etaBE(2)) < 2.47)) passSel = false; //do we want to exclude the crack region?
  }
  else{
    if (!(fabs(electron->eta()) < 2.47)) passSel = false; //do we want to exclude the crack region?
    Warning("JetSemileptonic::passForJetElectron","Did not find caloCluster, use eta() instead of caloCluster()->etaBE(2) to check eta range!");
  }
  if (!(electron->pt() > 4000.)) passSel = false;
  if (!(electron->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON))) passSel = false;
  return passSel;
}

bool JetSemileptonic::passForJetMuon(const xAOD::Muon* muon)
{
  if(m_debug) { std::cout<<"JetSemileptonic::passForJetMuon"<<std::endl; }
  bool passSel = true;
  bool accepted = m_muonSelectionTool->accept(*muon);
  if (!(accepted)) passSel = false;
  if (!(muon->pt() > 4000.)) passSel = false;
  return passSel;
}

TLorentzVector JetSemileptonic::getELossTLV(const xAOD::Muon* muon)
{
  float eLoss=0.0;
  muon->parameter(eLoss,xAOD::Muon::EnergyLoss);
  if(m_debug) {
    std::cout<<"JetSemileptonic::getELossTLV xAOD::Muon eLoss="<<eLoss<<std::endl;
  }
  TLorentzVector m=muon->p4();
  double theta=m.Theta();
  double phi=m.Phi();
  double eLossX=eLoss*sin(theta)*cos(phi);
  double eLossY=eLoss*sin(theta)*sin(phi);
  double eLossZ=eLoss*cos(theta);
  return TLorentzVector(eLossX,eLossY,eLossZ,eLoss);
}


EL::StatusCode JetSemileptonic::countElectronInJet(xAOD::Jet* jet, const TString& variation, const xAOD::ElectronContainer* electrons) {
  if (m_debug) {
    std::cout<<"JetSemileptonic::countElectronInJet with variation="<<variation
	     <<" jet "<<jet->pt()<<" eta="<<jet->eta()<<" phi="<<jet->phi()<<" e="<<jet->e()<<" m="<<jet->m()
	     <<std::endl;
  }
  unsigned counter04=0;
  unsigned counter=0;
  unsigned index_smallestDR=99;
  double smallestDR=99.9;
  TLorentzVector one;
  // loop over all the electrons
  for (unsigned i=0; i!=electrons->size(); i++) {
    const xAOD::Electron* electron = (*electrons)[i];
    if(m_debug) {
      std::cout<<"JetSemileptonic::countElectronInJet xAOD::Electron"
               <<" pt="<<electron->pt()<<" eta="<<electron->eta()<<" phi="<<electron->phi()<<" e="<<electron->e()<<" m="<<jet->m()
               <<std::endl;
    }
    bool good=passForJetElectron(electron);
    if(m_debug) {
      std::cout<<"JetSemileptonic::countElectronInJet xAOD::Electron pass electron selection="<<good<<std::endl;
    }
    if(!good) continue;
    // electron must be within 0.4 of the jet axis                                                                                                                             
    double DR=jet->p4().DeltaR(electron->p4());
    if(m_debug) {
      std::cout<<"JetSemileptonic::countElectronInJet xAOD::Electron electron passed selection, and DR="<<DR<<std::endl;
    }
    //apply the a DRCut changing with electron pT, just as in the Overlap Removal procedure                                                                                    
    double DRCut=std::min(0.4,0.04+10000.0/electron->pt());
    if(m_debug) {
      std::cout<<"JetSemileptonic::countElectronInJet xAOD::Electron"
               << " DRCut varying with electron pt="<<DRCut
               <<std::endl;
    }
    //if DR>DRCut (but smaller than 0.4), then the electron has deposited energy in the jet               
    //so remove it from the jet (though some calorimeter part would be more correct)
    if(!(DR<DRCut)) continue;
    if(m_debug) {
      std::cout<<"JetSemileptonic::correctElectronInJet xAOD::Electron DR<DRCutForVariablePt."<<std::endl;
    }
    counter++;
    if(DR<smallestDR)
      {
	index_smallestDR=i;
	smallestDR=DR;
      }
  }//end loop over electrons 

  if (counter>0) {
    const xAOD::Electron* electron = (*electrons)[index_smallestDR];
    one=electron->p4();
  }

  if(m_debug) {
    std::cout<<"JetSemileptonic::countElectronInJet one Electron"
	     <<" pt="<<one.Pt()<<" eta="<<one.Eta()<<" phi="<<one.Phi()<<" e="<<one.E()<<" m="<<one.M()
	     <<" DR="<<jet->p4().DeltaR(one)<<std::endl;
  }

  Props::nrElectronInJet04.set(jet,counter04);
  Props::nrElectronInJet.set(jet,counter);
  //Props::PtRatioOneElectronInJet.set(jet,one.Pt()/jet->pt());
  //Props::ERatioOneElectronInJet.set(jet,one.E()/jet->e());

  if(m_debug) {
    std::cout<<"JetSemileptonic::countElectronInJet jet pt="<<jet->pt()
	     <<" nrElectronInJet="<<Props::nrElectronInJet.get(jet)
      //<<" PtRatioOneElectronInJet="<<Props::PtRatioOneElectronInJet.get(jet)
      //<<" ERatioOneElectronInJet="<<Props::ERatioOneElectronInJet.get(jet)
	     <<std::endl;
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode JetSemileptonic::correctMuonInJet(xAOD::Jet* jet, const TString& variation, const xAOD::MuonContainer* muons, bool doPtReco) {
  // this is the new version from Oct 2018 to be used for the end of Run-2, where we store as OneMu only the muons within the variable-R cone radius
  // so no more OneMu, OneMu2; the previous OneMu2 is now OneMu
  // nrMuonInJet remains as before for VR, and nrMuonInJet04 disappears
  // all references to electrons disappear
  // store only one PtReco corresponding to this OneMu (but made from histograms that still have OneMu2 in their name)
  // created a new function after renamed the old one, so that we still have the old one just in case we want to do some extra studies later
  // and make this one short with only the minimum steps we need
  // we do not even compute things we will not want to store later on, to reduce CPU size as well
  if (m_debug) {
    std::cout<<"JetSemileptonic::correctMuonInJet with variation="<<variation
             <<" jet "<<jet->pt()<<" eta="<<jet->eta()<<" phi="<<jet->phi()<<" e="<<jet->e()<<" m="<<jet->m()
	     <<std::endl;
  }
  TLorentzVector j=jet->p4();
  unsigned counter=0;
  unsigned index_smallestDR=99;
  double smallestDR=99.9;
  TLorentzVector one;
  TLorentzVector one_eLoss;
  TLorentzVector ptreco;
  
  // loop over all the muons                                                                                                                                                                                                                                   
  for (unsigned i=0; i!=muons->size(); i++) {
    const xAOD::Muon* muon = (*muons)[i];
    if(m_debug) {
      std::cout<<"JetSemileptonic::countMuonInJet xAOD::Muon"
               <<" pt="<<muon->pt()<<" eta="<<muon->eta()<<" phi="<<muon->phi()<<" e="<<muon->e()<<" m="<<jet->m()
               <<std::endl;
    }
    bool good=passForJetMuon(muon);
    if(m_debug) {
      std::cout<<"JetSemileptonic::countMuonInJet xAOD::Muon pass muon selection="<<good<<std::endl;
    }
    if(!good) continue;
    // muon must be within some DR of the jet axis
    double DR=jet->p4().DeltaR(muon->p4());
    if(m_debug) {
      std::cout<<"JetSemileptonic::countMuonInJet xAOD::Muon muon passed selection, and DR="<<DR<<std::endl;
    }
    //apply the a DRCut changing with muon pT, just as in the Overlap Removal procedure
    double DRCut=std::min(0.4,0.04+10000.0/muon->pt());
    if(m_debug) {
      std::cout<<"JetSemileptonic::countMuonInJet xAOD::Muon"
               << " DRCut varying with muon pt="<<DRCut
               <<std::endl;
    }
    if(!(DR<DRCut)) continue;
    if(m_debug) {
      std::cout<<"JetSemileptonic::correctMuonInJet xAOD::Muon DR<DRCutForVariablePt. We remove the 4-vector of the energy deposited by the muon in the calorimeter and we add the 4-vector of the muon"<<std::endl;
    }
    counter++;
    if(DR<smallestDR)
      {
	index_smallestDR=i;
	smallestDR=DR;
      }
  } // done loop over muons
  if (counter>0) {
    const xAOD::Muon* muon = (*muons)[index_smallestDR];
    one=muon->p4();
    one_eLoss+=getELossTLV(muon);
  }
  if(m_debug) {
    std::cout<<"JetSemileptonic::correctMuonInJet one       Muon"
	     <<" pt="<<one.Pt()<<" eta="<<one.Eta()<<" phi="<<one.Phi()<<" e="<<one.E()<<" m="<<one.M()
	     <<" DR="<<jet->p4().DeltaR(one)<<std::endl;
    std::cout<<"JetSemileptonic::correctMuonInJet one_eLoss Muon"
	     <<" pt="<<one_eLoss.Pt()<<" eta="<<one_eLoss.Eta()<<" phi="<<one_eLoss.Phi()<<" e="<<one_eLoss.E()<<" m="<<one_eLoss.M()
	     <<" DR="<<jet->p4().DeltaR(one_eLoss)<<std::endl;
  }
  Props::nrMuonInJet.set(jet,counter);
  if(m_debug) {
    std::cout<<"JetSemileptonic::correctMuonInJet jet pt="<<jet->pt()
	     <<" nrMuonInJet="<<Props::nrMuonInJet.get(jet)
	     <<std::endl;
  }
  //create the TLorentzVector for the different lepton-in-jet
  std::map<std::string, TLorentzVector> map_name_tlv;
  map_name_tlv["OneMu"]=j-one_eLoss+one;
  if(m_debug) {
    std::cout<<"JetSemileptonic::correctMuonInJet     j       ="<<j.Pt()<<std::endl; j.Print();
    std::cout<<"JetSemileptonic::correctMuonInJet OneMu       ="<<map_name_tlv["OneMu"].Pt()<<std::endl; map_name_tlv["OneMu"].Print();
  }
  // insert the 4-vector of all these lepton-in-jet corrections into the jet
  for(auto const & item : map_name_tlv) {
    EL_CHECK("setJetTLV",setJetTLV(jet,item.first,item.second-j));//subtracting j, which is Nominal, already in MeV
    if(m_debug) {
      std::cout<<"JetSemileptonic::correctMuonInJet saved in CxAOD "<<item.first<<"="<<jet->jetP4(item.first).pt()<<std::endl;
    }
  }

  //only if we want PtReco, i.e. for resolved case, but not for the boosted jets
  if(doPtReco) {
    if(m_debug) {
      std::cout<<"Start doPtReco:"<<std::endl;
    }
    int nrMu=counter;
    std::string currentsld="";
    if(nrMu>0){
      currentsld="semileptonic";
    } else {
      currentsld="hadronic";
    }
    std::string input="OneMu";
    TLorentzVector tlv=map_name_tlv[input];//this is in MeV the proper value, not the difference to nominal
    float jetPt=(tlv.Pt())*0.001;//convert MeV->GeV needed as input to PtReco correction factor calculation
    // since 25 Feb 2019 when we retrained from 32-07 ade, instead of as before from 31-10 ad
    std::string PtRecoHistoName="CorrectionFactor_qqZllHbb_345055_mc16ade_all_OneMu_"+currentsld+"_None";
    float factorPtReco=m_PtRecoTool->getFactor(jetPt,PtRecoHistoName);
    // store the correction factor
    Props::factorPtReco.set(jet,factorPtReco);
  } // done if doPtReco

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode JetSemileptonic::setJetTLV(xAOD::Jet * jet, const std::string& name, const TLorentzVector& tlv) {
  if(m_debug) {
    std::cout<<"JetSemileptonic::setJetTLV before setting for xAOD::Jet pt="<<jet->pt()<<" eta="<<jet->eta()<<" phi="<<jet->phi()<<" e="<<jet->e()<<" m="<<jet->m()
	     <<" the following TLorentzVector with name \""<<name<<"\" and values"
	     <<" pt="<<tlv.Pt()<<" eta="<<tlv.Eta()<<" phi="<<tlv.Phi()<<" e="<<tlv.E()<<" m="<<tlv.M()
	     <<std::endl;
  }
  JetFourMom_t jetVec;
  jetVec.SetPt(tlv.Pt());
  jetVec.SetEta(tlv.Eta());
  jetVec.SetPhi(tlv.Phi());
  jetVec.SetM(tlv.M());
  if(m_debug) {
    std::cout<<"JetSemileptonic::setJetTLV We constructed JetFourMom_t jetVe with"
	     <<" pt="<<jetVec.Pt()<<" eta="<<jetVec.Eta()<<" phi="<<jetVec.Phi()<<" e="<<jetVec.E()<<" m="<<jetVec.M()
	     <<std::endl;
  }
  if (name=="") {
    //change the jet main value
    jet->setJetP4(jetVec); 
  } else {
    //set a new signal state
    jet->setJetP4(name,jetVec);
  }

  if(m_debug) {
    JetFourMom_t jet2=jet->jetP4(name);
    std::cout<<"JetSemileptonic::setJetTLV after setting for xAOD::Jet pt="<<jet->pt()<<" eta="<<jet->eta()<<" phi="<<jet->phi()<<" e="<<jet->e()<<" m="<<jet->m()
	     <<" the following TLorentzVector with name \""<<name<<"\" and values"
	     <<" pt="<<jet2.Pt()<<" eta="<<jet2.Eta()<<" phi="<<jet2.Phi()<<" e="<<jet2.E()<<" m="<<jet2.M()
	     <<std::endl;
  }
  //alternatively we can write this as a new state of the signal jet, as explained in the xAOD::Jet, especially for CaloJet
  //http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Event/xAOD/xAODJet/xAODJet/versions/Jet_v1.h
  //Signal States : Jets can hold several other 4-momentum representing various calibration states (primaliry used for calo jets). See also P4SignalState
  //void setJetP4(const std::string & sname, const JetFourMom_t & p4);
  //jet->setJetP4("EMJESGSMu",jetVec);
  //to validate this signal state
  //JetFourMom_t jet2=jet->jetP4("EMJESGSMu");
  //if(m_debug) {
  //std::cout<<"after xAOD::Jet Signal stateEMJESGSMu  pt="<<jet2.Pt()<<" eta="<<jet2.Eta()<<" phi="<<jet2.Phi()<<" e="<<jet2.E()<<" m="<<jet2.M()<<std::endl;
  //}
  
  return EL::StatusCode::SUCCESS;
}

