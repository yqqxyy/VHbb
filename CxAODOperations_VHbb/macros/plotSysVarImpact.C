//Script to plot systematic variations

//Brian Moser for the boosted VHbb analysis (brian.moser@cern.ch)
//This script needs AtlasStyle.* and AtlasLabels.* in the same directory

//The steering part to modify can be found at the end of the code

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TColor.h"
#include <vector>
#include "TPad.h"
#include "TAxis.h"
#include "AtlasStyle.C"
#include "AtlasLabels.C"
#include <sstream>

using namespace std;

struct var {
  string name;
  string tex;
  double x_min;
  double x_max;
  int rebin;
};

struct sumvec {
  string label_name;
  string save_name;
  vector<string> vect;
};

bool m_normalize = true;
string m_lepChannel = "";

//Helper functions
template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 3)
{
  ostringstream out;
  out.precision(n);
  out << fixed << a_value;
  return out.str();
}

//include OF and UF bin
void binsHist(TH1D* h){
  //UF treatement
  h->SetBinContent(1, h->GetBinContent(0)+h->GetBinContent(1));
  h->SetBinError(1, sqrt(pow(h->GetBinError(0),2)+pow(h->GetBinError(1),2)));
  h->SetBinContent(0, 0 );
  h->SetBinError(0, 0);
  //OF treatement
  h->SetBinContent(h->GetNbinsX(), h->GetBinContent(h->GetNbinsX())+h->GetBinContent(h->GetNbinsX()+1));
  h->SetBinError(h->GetNbinsX(), sqrt(pow(h->GetBinError(h->GetNbinsX()),2)+pow(h->GetBinError(h->GetNbinsX()+1),2)));
  h->SetBinContent(h->GetNbinsX()+1, 0);
  h->SetBinError(h->GetNbinsX()+1, 0);
}//binsHist


//ratio plots
TH1* getDataMCComparison(TH1 *h_data, TH1 *h_MC) {

  TH1 *h_comp = (TH1*)h_data->Clone();
  h_comp->SetDirectory(0);
  h_comp->Reset();

  for(int bin=1; bin<=h_comp->GetNbinsX(); bin++) {

    double nData = h_data->GetBinContent(bin);
    double eData = h_data->GetBinError(bin);
    double nMC = h_MC->GetBinContent(bin);

    if(nData > 1e-6 && eData > 1e-6 && nMC > 1e-6) {

      double nComp = nData / nMC;
      double eComp = eData / nMC;

      h_comp->SetBinContent(bin, nComp);
      h_comp->SetBinError(bin, eComp);
    }
  }
  return h_comp;
}//getDataMCComparison

//ratio plots
TH1* getDataMCComparisonNormalized(TH1 *h_data_tmp, TH1 *h_MC_tmp) {
  
  TH1 *h_data = (TH1*)h_data->Clone((string(h_data_tmp->GetName())+"_norm").c_str());
  TH1 *h_MC = (TH1*)h_MC->Clone((string(h_MC_tmp->GetName())+"_norm").c_str());

  h_data->Scale(h_data->Integral(0,-1));
  h_MC->Scale(h_MC->Integral(0,-1));

  TH1 *h_comp = (TH1*)h_data->Clone();
  h_comp->SetDirectory(0);
  h_comp->Reset();

  for(int bin=1; bin<=h_comp->GetNbinsX(); bin++) {

    double nData = h_data->GetBinContent(bin);
    double eData = h_data->GetBinError(bin);
    double nMC = h_MC->GetBinContent(bin);

    if(nData > 1e-6 && eData > 1e-6 && nMC > 1e-6) {

      double nComp = nData / nMC;
      double eComp = eData / nMC;

      h_comp->SetBinContent(bin, nComp);
      h_comp->SetBinError(bin, eComp);
    }
  }
  return h_comp;
}//getDataMCComparison


void createCompPlots(string s_mcperiod, string s_inputFileName, vector<string> v_variations, vector<sumvec> v_processes, vector<sumvec> v_tags, vector<sumvec> v_jets, vector<sumvec> v_ptv, vector<sumvec> v_regions, vector<var> v_vars){
  TH1::SetDefaultSumw2();
  TGaxis::SetMaxDigits(4);

  //open the right input file
  TFile* f_input = new TFile(s_inputFileName.c_str(),"READ"); 

  for(auto variation : v_variations){	
    std::cout << "Doing Variation: " << variation <<  std::endl;
    bool b_isOneSided = false;    
    for(auto process_split : v_processes){
      for(auto tag_split : v_tags){
	for(auto ptv_split : v_ptv){
	  for(auto jet_split : v_jets){
	    for(auto region_split : v_regions){
	      for(auto variable : v_vars){
		TH1D *h_nom = NULL;
		TH1D *h_up = NULL;
		TH1D *h_down = NULL;
		string s_canvasName = "";

		//sum over all processes and create the histograms
		for(auto process : process_split.vect){
		  for(auto tag : tag_split.vect){
		    for(auto ptv : ptv_split.vect){
		      for(auto jet : jet_split.vect){
			for(auto region : region_split.vect){
			  //nominal name
			  string s_nom_name = "";
			  s_nom_name.append(process);
			  s_nom_name.append(("_"+tag+jet+"_"+ptv+"_"+region+"_").c_str());
			  s_nom_name.append(variable.name);
			  cout << "hist..." << s_nom_name.c_str() << endl;
			  TH1D *h_nom_tmp = (TH1D*) f_input->Get(s_nom_name.c_str()); 
			  if(h_nom_tmp==NULL){
			    continue;
			  }
			  h_nom_tmp->Clone();
			  h_nom_tmp->SetDirectory(0);
			  if(h_nom == NULL){
			    h_nom = h_nom_tmp;
			  }else{
			    h_nom->Add(h_nom_tmp);
			  }

			  //systematic variation - upward is always there, also for one-sided systematics
			  string s_up_name = "Systematics/";
			  s_up_name.append(process);
			  s_up_name.append(("_"+tag+jet+"_"+ptv+"_"+region+"_").c_str());
			  s_up_name.append(variable.name);
			  s_up_name.append("_Sys");
			  s_up_name.append(variation);
			  s_up_name.append("__1up");
			  s_canvasName = s_up_name; //for canvas naming, later
	
			  TH1D *h_up_tmp = (TH1D*) f_input->Get(s_up_name.c_str())->Clone();
			  h_up_tmp->SetDirectory(0);
			  if(h_up == NULL){
			    h_up = h_up_tmp;
			  }else{
			    h_up->Add(h_up_tmp);
			  }
	
			  //downward variation if two-sided systematic
			  string s_down_name = "Systematics/";
			  s_down_name.append(process);
			  s_down_name.append(("_"+tag+jet+"_"+ptv+"_"+region+"_").c_str());
			  s_down_name.append(variable.name);
			  s_down_name.append("_Sys");
			  s_down_name.append(variation);
			  s_down_name.append("__1down");
	
			  if(f_input->Get(s_down_name.c_str())!=NULL){
			    TH1D *h_down_tmp = (TH1D*) f_input->Get(s_down_name.c_str())->Clone();
			    h_down_tmp->SetDirectory(0);
			    if(h_down == NULL){
			      h_down = h_down_tmp;
			    }else{
			      h_down->Add(h_down_tmp);
			    }
			  }else{
			    b_isOneSided=true;
			  }	
			}
		      }
		    }
		  }
		}//process sum

		//histograms have been created, now do the plotting
		//--- rebinning
		h_nom->Rebin(variable.rebin);
		h_up->Rebin(variable.rebin);
		if(!b_isOneSided){
		  h_down->Rebin(variable.rebin);
		}
		//--- range setting
		h_nom->GetXaxis()->SetRangeUser(variable.x_min, variable.x_max);
		h_up->GetXaxis()->SetRangeUser(variable.x_min, variable.x_max);
		if(!b_isOneSided){
		  h_down->GetXaxis()->SetRangeUser(variable.x_min, variable.x_max);
		}
		//--- include OF,UF in plotting
		binsHist(h_nom);
		binsHist(h_up);
		if(!b_isOneSided){
		  binsHist(h_down);
		}
		//--- range setting
		h_nom->GetXaxis()->SetRangeUser(variable.x_min, variable.x_max);
		h_up->GetXaxis()->SetRangeUser(variable.x_min, variable.x_max);
		if(!b_isOneSided){
		  h_down->GetXaxis()->SetRangeUser(variable.x_min, variable.x_max);
		}
		//--- plot styles
		string yLabel = "a.u.";
		h_nom->SetMarkerStyle(8);
		h_nom->SetMarkerColor(kBlack);
		h_nom->SetMarkerSize(0.6); 
		h_nom->GetXaxis()->SetTitle(variable.tex.c_str());
		h_nom->GetYaxis()->SetTitle(yLabel.c_str());
		h_nom->GetXaxis()->SetLabelOffset(999);
		h_nom->GetXaxis()->SetLabelSize(0);

		h_up->SetLineColor(kAzure+1);
		h_up->SetLineWidth(2);
		h_up->SetMarkerStyle(8);
		h_up->SetMarkerColor(kAzure+1);
		h_up->SetMarkerSize(0.6); 
		h_up->GetXaxis()->SetTitle(variable.tex.c_str());
		h_up->GetYaxis()->SetTitle(yLabel.c_str());
		h_up->GetXaxis()->SetLabelOffset(999);
		h_up->GetXaxis()->SetLabelSize(0);

		if(!b_isOneSided){
		  h_down->SetLineColor(kSpring-1);
		  h_down->SetLineWidth(2);
		  h_down->SetMarkerStyle(8);
		  h_down->SetMarkerColor(kSpring-1);
		  h_down->SetMarkerSize(0.6); 
		  h_down->GetXaxis()->SetTitle(variable.tex.c_str());
		  h_down->GetYaxis()->SetTitle(yLabel.c_str());
		  h_down->GetXaxis()->SetLabelOffset(999);
		  h_down->GetXaxis()->SetLabelSize(0);
		}

		//--- canvas care taking
		TCanvas *c1 = new TCanvas(s_canvasName.c_str(),"",750,850);
		c1->cd();

		TPad *P_1 = new TPad((s_canvasName+"_1").c_str(), (s_canvasName+"_1").c_str(), 0, 0, 1, 0.3);
		TPad *P_2 = new TPad((s_canvasName+"_2").c_str(), (s_canvasName+"_2").c_str(), 0, 0.3, 1, 1);
		P_1->Draw();
		P_2->Draw();
		P_1->SetBottomMargin(0.35);
		P_1->SetTopMargin(0.02);
		P_1->SetRightMargin(0.05);
		P_2->SetTopMargin(0.05);
		P_2->SetBottomMargin(0.02);
		P_2->SetRightMargin(0.05);

		P_2->cd();
      
		double int_nom(0.0), int_up(0.0), int_down(0.0);
		int_nom = h_nom->Integral(0,-1);
		int_up = h_up->Integral(0,-1);
		if(!b_isOneSided) int_down = h_down->Integral(0,-1);

		if(m_normalize){
		  h_nom->Scale(1.0/int_nom);
		  h_up->Scale(1.0/int_up);
		  if(!b_isOneSided){
		    h_down->Scale(1.0/int_down);
		  }
		}

		//--- get some more space for plot labelling etc.; then DRAW
		double d_maximum = h_nom->GetMaximum();
		if(d_maximum<h_up->GetMaximum()){
		  d_maximum = h_up->GetMaximum();
		}
		if(!b_isOneSided){
		  if(d_maximum<h_down->GetMaximum()){
		    d_maximum = h_down->GetMaximum();
		  }	
		}		

		h_nom->GetYaxis()->SetRangeUser(0,d_maximum*1.55);
		h_nom->Draw("hist e0");

		h_up->GetYaxis()->SetRangeUser(0,d_maximum*1.55);
		h_up->Draw("hist e0 same");
 
		if(!b_isOneSided){
		  h_down->GetYaxis()->SetRangeUser(0,d_maximum*1.55);
		  h_down->Draw("hist e0 same");
		}

		h_nom->Draw("e0 hist same");

		P_2->RedrawAxis();
      
		//--- plot labeling
		TLatex l_LS;
		l_LS.SetNDC();
		l_LS.SetTextColor(1);
		l_LS.SetTextSize(0.055);
		l_LS.SetTextFont(72);
		l_LS.DrawLatex(0.20, 0.87, "ATLAS");
		l_LS.SetTextFont(42);
		l_LS.DrawLatex(0.35, 0.87, "work in progress");
		l_LS.SetTextSize(0.045);
		string s_channel = "";
		if(m_lepChannel == "0L"){
		  s_channel = "ZH #rightarrow #nu#nubb";
		}else if(m_lepChannel == "1L"){
		  s_channel = "WH #rightarrow l#nubb";
		}else if(m_lepChannel == "2L"){
		  s_channel = "ZH #rightarrow llbb";
		}

		l_LS.DrawLatex(0.20, 0.81, (s_channel+", "+process_split.label_name+" simulation").c_str());
		l_LS.DrawLatex(0.20, 0.76, (s_mcperiod + ", "+region_split.label_name+", "+tag_split.label_name+",").c_str());
		l_LS.DrawLatex(0.20, 0.71, (jet_split.label_name+", "+ptv_split.label_name).c_str());
		l_LS.DrawLatex(0.20, 0.66, ("#bf{" + variation + "}").c_str());

		if(m_normalize){
		  l_LS.DrawLatex(0.64, 0.61, "Yield change:");
		  if(!b_isOneSided){
		    double upimpact = int_up/int_nom;
		    double downimpact = int_down/int_nom;
		    l_LS.DrawLatex(0.64, 0.56, ("up var.: "+to_string_with_precision((upimpact-1)*100, 2)+" %").c_str());
		    l_LS.DrawLatex(0.64, 0.51, ("down var.: "+to_string_with_precision((downimpact-1)*100, 2)+" %").c_str());
		  }else{
		    double upimpact = int_up/int_nom;
		    l_LS.DrawLatex(0.64, 0.56, ("var.: "+to_string_with_precision((upimpact-1)*100, 2)+" %").c_str());		    
		  }
		}

		//--- plot legend
		TLegend *L = new TLegend(0.694, 0.779, 0.886, 0.911);
		L->SetFillColor(0);
		L->SetLineWidth(0);
		L->SetTextSize(0.035);
		L->SetBorderSize(1);
		L->SetLineColor(kWhite);
		L->AddEntry(h_nom,"nominal","lep");
		if(!b_isOneSided){
		  L->AddEntry(h_up,"up variation","lep");
		  L->AddEntry(h_down,"down variation","lep");
		}else{
		  L->AddEntry(h_up,"variation","lep");
		}
		L->Draw();
    
		//--- ratio plots
		//Ratio Pad 
		P_1->cd();
		TH1F* h_r_up = (TH1F*) getDataMCComparison(h_up,h_nom);
		h_r_up->GetXaxis()->SetTitleSize(0.12);
		h_r_up->GetXaxis()->SetTitleOffset(1.35);
		h_r_up->GetXaxis()->SetLabelSize(0.12);
		h_r_up->GetXaxis()->SetLabelOffset(0.03);
		h_r_up->GetYaxis()->SetTitleSize(0.12);
		h_r_up->GetYaxis()->SetTitleOffset(0.55);
		h_r_up->GetYaxis()->SetLabelSize(0.12);
		h_r_up->GetYaxis()->SetRangeUser(0.5, 1.5);
		if(m_normalize){
		  h_r_up->GetYaxis()->SetTitle("(shape) var./nom.");
		}else{
		  h_r_up->GetYaxis()->SetTitle("var./nom.");
		}
		h_r_up->GetXaxis()->SetTitle(variable.tex.c_str());

		h_r_up->SetLineColor(kAzure+1);
		h_r_up->SetLineWidth(2);
		h_r_up->SetMarkerStyle(8);
		h_r_up->SetMarkerColor(kAzure+1);
		h_r_up->SetMarkerSize(1.0);

		h_r_up->GetYaxis()->SetNdivisions(3,6,0,kTRUE);
		h_r_up->GetYaxis()->SetLabelOffset(999);  
  
		h_r_up->Draw("ep2");

		if(!b_isOneSided){
		  TH1F* h_r_down = (TH1F*) getDataMCComparison(h_down,h_nom);
		  h_r_down->GetXaxis()->SetTitleSize(0.12);
		  h_r_down->GetXaxis()->SetTitleOffset(1.35);
		  h_r_down->GetXaxis()->SetLabelSize(0.12);
		  h_r_down->GetXaxis()->SetLabelOffset(0.03);
		  h_r_down->GetYaxis()->SetTitleSize(0.12);
		  h_r_down->GetYaxis()->SetTitleOffset(0.55);
		  h_r_down->GetYaxis()->SetLabelSize(0.12);
		  h_r_down->GetYaxis()->SetRangeUser(0.5, 1.5);
		  if(m_normalize){
		    h_r_up->GetYaxis()->SetTitle("(shape) var./nom.");
		  }else{
		    h_r_up->GetYaxis()->SetTitle("var./nom.");
		  }
		  h_r_down->GetXaxis()->SetTitle(variable.tex.c_str());

		  h_r_down->SetLineColor(kSpring-1);
		  h_r_down->SetLineWidth(2);
		  h_r_down->SetMarkerStyle(8);
		  h_r_down->SetMarkerColor(kSpring+1);
		  h_r_down->SetMarkerSize(1.0);

		  h_r_down->GetYaxis()->SetNdivisions(3,6,0,kTRUE);
		  h_r_down->GetYaxis()->SetLabelOffset(999);  
  
		  h_r_down->Draw("ep2 same");
		}

		TLine *L_One = new TLine(variable.x_min, 1, variable.x_max, 1);
		L_One->SetLineWidth(2);
		L_One->SetLineColor(1);
		L_One->SetLineStyle(2);
		L_One->Draw();

		TText t;
		t.SetTextAlign(32);
		t.SetTextSize(0.12);
		t.SetTextAngle(0);
		t.SetNDC();
		t.DrawText(0.15,0.91,"1.4");
		t.DrawText(0.15,0.65,"1.0");
		t.DrawText(0.15,0.37,"0.6");

		c1->Modified();

		string s_saveName = "C_";
		s_saveName.append("syst_VHbb_");
		s_saveName.append(s_mcperiod);
		s_saveName.append("_");
		s_saveName.append(m_lepChannel);
		s_saveName.append("_");
		s_saveName.append(variable.name);
		s_saveName.append("_");
		s_saveName.append(process_split.save_name);
		s_saveName.append("_");
		s_saveName.append(tag_split.save_name);
		s_saveName.append("_");
		s_saveName.append(jet_split.save_name);
		s_saveName.append("_");
		s_saveName.append(ptv_split.save_name);
		s_saveName.append("_");
		s_saveName.append(region_split.save_name);
		s_saveName.append("_");
		s_saveName.append(variation);
		c1->SaveAs((s_saveName+".png").c_str());
		c1->SaveAs((s_saveName+".pdf").c_str());
	      }//variables for
	    }//region
	  }//jets
	}//ptv
      }//tag
    }//proceess
  }//systematics for
}//createCompPlots



///////////////////////////////////
// To modify
///////////////////////////////////
void plotSysVarImpact(){
  
  SetAtlasStyle();

  //configurable:

  //which lepton channel?
  m_lepChannel = "1L"; //0L, 1L or 2L
  
  //shape only?
  m_normalize = true;

  //full systematics vector
  vector<string> v_variations { "FATJET_Medium_JET_Comb_Baseline_Kin","FATJET_Medium_JET_Comb_Modelling_Kin","FATJET_Medium_JET_Comb_TotalStat_Kin","FATJET_Medium_JET_Comb_Tracking_Kin","FATJET_Medium_JET_Rtrk_Baseline_Sub","FATJET_Medium_JET_Rtrk_Modelling_Sub","FATJET_Medium_JET_Rtrk_TotalStat_Sub","FATJET_Medium_JET_Rtrk_Tracking_Sub","FATJET_JER","FATJET_JMR","FATJET_SubR" };

  //the sumvec struct is organized as {label name, save name, vector to merge}

  //processes to be considered
  vector<sumvec> v_processes;
  v_processes.push_back({"W+jets","Wjets",{"Wbb", "Wbc", "Wcc", "Wcl", "Wl"}});

  //tag categories
  vector<sumvec> v_tags;
  v_tags.push_back({"2 tags","2tag",{"2tag"}});

  //jets
  vector<sumvec> v_jets;
  v_jets.push_back({"#geq 1 FJ","1pfat0pjet",{"1pfat0pjet"}});

  //ptv ranges
  vector<sumvec> v_ptv;
  v_ptv.push_back({"p_{T}^{V} inclusive","150ptv",{"0_250ptv", "250_400ptv", "400ptv"}});
  
  //regions
  vector<sumvec> v_regions;
  v_regions.push_back({"SR","SR",{"SR_noaddbjetsr"}});
  v_regions.push_back({"CR","CR",{"SR_topaddbjetcr"}});

  //variable vector, to be plotted
  vector<var> v_vars; 
  v_vars.push_back({"mBB","leading large-R jet mass [GeV]",0.0,250.,2}); //variable name, x-axis label, xdown, xup, rebin

  //Main function to call
  //MC period and file name
  createCompPlots("mc16e", "hist-Wenu.root", v_variations, v_processes, v_tags, v_jets, v_ptv, v_regions, v_vars);
}

///////////////////////////////////
// end of to modify
///////////////////////////////////

// VHbb systematics
//  vector<string> v_variations { "PRW_DATASF","JET_CR_JET_BJES_Response","JET_CR_JET_EffectiveNP_Detector1","JET_CR_JET_EffectiveNP_Detector2","JET_CR_JET_EffectiveNP_Mixed1","JET_CR_JET_EffectiveNP_Mixed2","JET_CR_JET_EffectiveNP_Mixed3","JET_CR_JET_EffectiveNP_Modelling1","JET_CR_JET_EffectiveNP_Modelling2","JET_CR_JET_EffectiveNP_Modelling3","JET_CR_JET_EffectiveNP_Modelling4","JET_CR_JET_EffectiveNP_Statistical1","JET_CR_JET_EffectiveNP_Statistical2","JET_CR_JET_EffectiveNP_Statistical3","JET_CR_JET_EffectiveNP_Statistical4","JET_CR_JET_EffectiveNP_Statistical5","JET_CR_JET_EffectiveNP_Statistical6","JET_CR_JET_EtaIntercalibration_Modelling","JET_CR_JET_EtaIntercalibration_NonClosure_highE","JET_CR_JET_EtaIntercalibration_NonClosure_negEta","JET_CR_JET_EtaIntercalibration_NonClosure_posEta","JET_CR_JET_EtaIntercalibration_TotalStat","JET_CR_JET_Flavor_Composition","JET_CR_JET_Flavor_Response","JET_CR_JET_JER_DataVsMC","JET_CR_JET_JER_EffectiveNP_1","JET_CR_JET_JER_EffectiveNP_2","JET_CR_JET_JER_EffectiveNP_3","JET_CR_JET_JER_EffectiveNP_4","JET_CR_JET_JER_EffectiveNP_5","JET_CR_JET_JER_EffectiveNP_6","JET_CR_JET_JER_EffectiveNP_7restTerm","JET_CR_JET_Pileup_OffsetMu","JET_CR_JET_Pileup_OffsetNPV","JET_CR_JET_Pileup_PtTerm","JET_CR_JET_Pileup_RhoTopology","JET_CR_JET_PunchThrough_MC16","JET_CR_JET_SingleParticle_HighPt","JET_JvtEfficiency","MET_JetTrk_Scale","MET_SoftTrk_ResoPara","MET_SoftTrk_ResoPerp","MET_SoftTrk_Scale","EG_RESOLUTION_ALL","EG_SCALE_ALL","EL_EFF_ID_TOTAL_1NPCOR_PLUS_UNCOR","EL_EFF_Iso_TOTAL_1NPCOR_PLUS_UNCOR","EL_EFF_Reco_TOTAL_1NPCOR_PLUS_UNCOR","EL_EFF_Trigger_TOTAL_1NPCOR_PLUS_UNCOR","MUON_ID","MUON_MS","MUON_SAGITTA_RESBIAS","MUON_SAGITTA_RHO","MUON_SCALE","MUON_EFF_ISO_STAT","MUON_EFF_ISO_SYS","MUON_EFF_RECO_STAT","MUON_EFF_RECO_STAT_LOWPT","MUON_EFF_RECO_SYS","MUON_EFF_RECO_SYS_LOWPT","MUON_EFF_TTVA_STAT","MUON_EFF_TTVA_SYS","TAUS_TRUEHADTAU_SME_TES_DETECTOR","TAUS_TRUEHADTAU_SME_TES_INSITU","TAUS_TRUEHADTAU_SME_TES_MODEL","FATJET_Medium_JET_Comb_Baseline_Kin","FATJET_Medium_JET_Comb_Modelling_Kin","FATJET_Medium_JET_Comb_TotalStat_Kin","FATJET_Medium_JET_Comb_Tracking_Kin","FATJET_Medium_JET_Rtrk_Baseline_Sub","FATJET_Medium_JET_Rtrk_Modelling_Sub","FATJET_Medium_JET_Rtrk_TotalStat_Sub","FATJET_Medium_JET_Rtrk_Tracking_Sub","FATJET_JER","FATJET_JMR","FATJET_SubR"};
