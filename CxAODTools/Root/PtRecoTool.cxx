#include "CxAODTools/PtRecoTool.h"

PtRecoTool::PtRecoTool():
  m_debug(false),
  m_ptrecoFile(nullptr),
  m_ptrecoHistSemileptonic(nullptr),
  m_ptrecoHistHadronic(nullptr)
{
}

PtRecoTool::~PtRecoTool()
{
}

EL::StatusCode PtRecoTool::initializePtRecoHistograms() {
  if (m_debug) std::cout<<"Start PtRecoTool::initializePtRecoHistograms()"<<std::endl;

  std::map<std::string,TFile*> map_name_TFile;

  // scope 18 Oct 2018 using 31-10 and periods ad
  {
    std::vector<std::string> processes={"qqZllHbb_345055"};
    std::vector<std::string> periods={"ad"};
    std::vector<std::string> inputs={"OneMu","OneMu2","OneMu2MuR"};
    std::vector<std::string> targets={"TruthWZ"};
    std::vector<std::string> slds={"hadronic","semileptonic"};
    std::vector<std::string> fits={"None"};    
    for(const auto& process : processes) {
      for(const auto& period : periods) {
	for(const auto& input : inputs) {
	  for(const auto& target : targets) {
	      // e.g. PtReco_histos_2_qqZllHbb_345055_mc16ad_all_OneMu2MuR_TruthWZ_CorrectionFactor.root
	      std::string nameTFile=process+"_mc16"+period+"_all_"+input+"_"+target+"_CorrectionFactor";
	      if (m_debug) std::cout<<"PtRecoTool::initializePtRecoHistograms() nameTFile="<<nameTFile<<std::endl;
	      map_name_TFile[nameTFile]=new TFile(("$WorkDir_DIR/data/CxAODTools/PtReco_histos_2_"+nameTFile+".root").c_str(),"read");
	      if (m_debug) std::cout<<"PtRecoTool::initializePtRecoHistograms() "<<"map_name_TFile[nameTFile]="<<map_name_TFile[nameTFile]<<std::endl<<" map_name_TFile[nameTFile]->IsZombie()="<<map_name_TFile[nameTFile]->IsZombie()<<std::endl;
	      for(const auto& sld : slds) {
		for(const auto& fit : fits) {
		  std::string nameTH1F="CorrectionFactor_"+process+"_mc16"+period+"_all_"+input+"_"+sld+"_"+fit;
		  if (m_debug) std::cout<<"PtRecoTool::initializePtRecoHistograms() "<<"nameTH1F="<<nameTH1F<<std::endl;
		  m_map_name_TH1F[nameTH1F]=(TH1F*) map_name_TFile[nameTFile]->Get(nameTH1F.c_str());
		  m_map_name_TH1F[nameTH1F]->SetDirectory(0);
		}//end loop over sld
	      }//end loop over fit
	      map_name_TFile[nameTFile]->Close();
	  }//end loop over target
	}//end loop over input
      }//end loop over period
    }//end loop over process
  }//end scop

  // scope 25 Feb 2019 using 32-07 and periods ade
  {
    std::vector<std::string> processes={"qqZllHbb_345055"};
    std::vector<std::string> periods={"ade"};
    std::vector<std::string> inputs={"OneMu"};
    std::vector<std::string> targets={"TruthWZ"};
    std::vector<std::string> slds={"hadronic","semileptonic"};
    std::vector<std::string> fits={"None"};    
    for(const auto& process : processes) {
      for(const auto& period : periods) {
	for(const auto& input : inputs) {
	  for(const auto& target : targets) {
	      // e.g. PtReco_histos_2_qqZllHbb_345055_mc16ade_all_OneMu_TruthWZ_CorrectionFactor.root
	      std::string nameTFile=process+"_mc16"+period+"_all_"+input+"_"+target+"_CorrectionFactor";
	      if (m_debug) std::cout<<"PtRecoTool::initializePtRecoHistograms() nameTFile="<<nameTFile<<std::endl;
	      map_name_TFile[nameTFile]=new TFile(("$WorkDir_DIR/data/CxAODTools/PtReco_histos_2_"+nameTFile+".root").c_str(),"read");
	      if (m_debug) std::cout<<"PtRecoTool::initializePtRecoHistograms() "<<"map_name_TFile[nameTFile]="<<map_name_TFile[nameTFile]<<std::endl<<" map_name_TFile[nameTFile]->IsZombie()="<<map_name_TFile[nameTFile]->IsZombie()<<std::endl;
	      for(const auto& sld : slds) {
		for(const auto& fit : fits) {
		  std::string nameTH1F="CorrectionFactor_"+process+"_mc16"+period+"_all_"+input+"_"+sld+"_"+fit;
		  if (m_debug) std::cout<<"PtRecoTool::initializePtRecoHistograms() "<<"nameTH1F="<<nameTH1F<<std::endl;
		  m_map_name_TH1F[nameTH1F]=(TH1F*) map_name_TFile[nameTFile]->Get(nameTH1F.c_str());
		  m_map_name_TH1F[nameTH1F]->SetDirectory(0);
		}//end loop over sld
	      }//end loop over fit
	      map_name_TFile[nameTFile]->Close();
	  }//end loop over target
	}//end loop over input
      }//end loop over period
    }//end loop over process
  }//end scop
  
  if(true){
    std::cout<<"PtRecoTool::initializePtRecoHistograms() printing out map_name_TH1F"<<std::endl;
    for(const auto& el : m_map_name_TH1F){
      std::cout<<"name="<<el.first<<" TH1F="<<el.second<<" integral="<<el.second->Integral()<<std::endl;
    }
  }

  // now ready to return success
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode PtRecoTool::initialize(bool debug)
{
  m_debug=debug;

  //new init
  std::string nameDir = "$WorkDir_DIR/data/CxAODTools/";
  std::string nameFile = "PtReco_histos_2_qqZllHbb_345055_mc16ade_all_OneMu_TruthWZ_CorrectionFactor.root";
  std::string nameHistSemileptonic = "CorrectionFactor_qqZllHbb_345055_mc16ade_all_OneMu_semileptonic_None";
  std::string nameHistHadronic = "CorrectionFactor_qqZllHbb_345055_mc16ade_all_OneMu_hadronic_None";

  m_ptrecoFile = TFile::Open((nameDir+nameFile).c_str());
  m_ptrecoHistSemileptonic = (TH1*)m_ptrecoFile->Get(nameHistSemileptonic.c_str());
  m_ptrecoHistHadronic = (TH1*)m_ptrecoFile->Get(nameHistHadronic.c_str());

  //this is old init, plan to remove it with DO_ICHEP
  return initializePtRecoHistograms();
}

//this is old one, plan to remove it with DO_ICHEP
float PtRecoTool::getFactor(float jetPt, const std::string& name)
{
  // 31-10 ad  name = qqZllHbb_ad_OneMu2_TruthWZ_True_hadronic_None
  // 31-10 ad  name = qqZllHbb_ad_OneMu2_TruthWZ_True_semileptonic_None
  // 32-07 ade name = qqZllHbb_ade_OneMu_TruthWZ_True_hadronic_None
  // 32-07 ade name = qqZllHbb_ade_OneMu_TruthWZ_True_semileptonic_None
  float factor=0.0;
  if (m_map_name_TH1F[name]) {
    factor=m_map_name_TH1F[name]->Interpolate(jetPt);
  } else {
    std::cout<<"PtRecoTool::getFactor() PtReco histogram not found for name="<<name<<". Will ABORT!!!"<<std::endl;
    assert(false);
  }
  if (m_debug) {
    std::cout<<"PtRecoTool::getFactor name="<<name<<" jetPt[GeV]="<<jetPt<<" factor="<<factor<<std::endl;
  }
  return factor;
}

//new one
float PtRecoTool::getFactor(const float& jetPt, const int& isSL)
{
  float factor=0.0;
  if (isSL==0) {
    factor = m_ptrecoHistHadronic->Interpolate(jetPt);
  } else if (isSL==1) {
    factor = m_ptrecoHistSemileptonic->Interpolate(jetPt);
  } else {
    std::cout<<"PtRecoTool::getFactor isSL "<<isSL<<" factor 1"<<std::endl;
    factor = 1;
  }
  return factor;
}
