#include "CxAODTools/TriggerScaleFactorsMET.h"
#include "TF1.h"

TriggerScaleFactorsMET::TriggerScaleFactorsMET() :
trig_SF(nullptr) {
  trig_SF = new TF1("SFfunc", "0.5*(TMath::Erf((x-[0])/(TMath::Sqrt(2)*[1]))+1)");
}

TriggerScaleFactorsMET::~TriggerScaleFactorsMET() {
  delete trig_SF;
}

double TriggerScaleFactorsMET::get_metTrigEff(const Double_t met, const std::string whichTrigger) {
  double triggerEff = 1;
  
  if (whichTrigger.find("HLT_xe110_mht_L1XE50") != std::string::npos) { //updated 19/01/2017
      trig_pars_w[0] = 121.6; 
      trig_pars_w[1] = 29.75; 
  } else if (whichTrigger == "HLT_xe90_mht_L1XE50") { // updated 19/01/2017
      trig_pars_w[0] = 104; 
      trig_pars_w[1] = 27.49; 
  } else {
    //This appears for all events, making log files very large.
    //TriggerScaleFactorsMET... WARNING No SF for trigger HLT_xe110_pufit_L1XE55 available! Returning 1.
    //Creating a GitLab issue https://gitlab.cern.ch/CxAODFramework/CxAODTools/issues/12
    //and do not show this warning for this trigger any more
    if (whichTrigger != "HLT_xe110_pufit_L1XE55") {
      Warning("TriggerScaleFactorsMET::get_metTrigEff",
	      "No Efficiency for trigger %s available! Returning 1.", whichTrigger.c_str());
    }
    triggerEff = 1.;
    return triggerEff;
  }

  trig_met[0] = met / 1e3;
  triggerEff = trig_SF->EvalPar(trig_met, trig_pars_w);
  return triggerEff;
}

double TriggerScaleFactorsMET::get_metTrigSF(const Double_t met, const Double_t sumpt, const std::string variation, const std::string whichTrigger) {
  double triggerSF = 1;
  
  if (whichTrigger.find("HLT_xe110_mht_L1XE50") != std::string::npos) { //updated 25/04/2018
      trig_pars_w[0] = 75.3992; 
      trig_pars_w[1] = 38.1689; 
      trig_pars_t[0] = 77.4138;
      trig_pars_t[1] = 44.7784;
      trig_pars_z[0] = 76.8295;
      trig_pars_z[1] = 41.7536;
      trig_errs_w[0] = 6.9117; 
      trig_errs_w[1] = 4.44981; 
      trig_matr[0] = 47.7716;
      trig_matr[1] = -29.1064;
      trig_matr[2] = -29.1064;
      trig_matr[3] = 19.8008;
      trig_Chisquare = 11.6478;
      trig_NDF = 13 ;
      trig_cl = 0.68; 
  } else if (whichTrigger == "HLT_xe70") { //updated 25/04/2018
    trig_pars_w[0] = -194.086;
    trig_pars_w[1] = 145.438;
    trig_pars_t[0] = -205.551;
    trig_pars_t[1] = 179.143;
    trig_pars_z[0] = 35.4531;
    trig_pars_z[1] = 48.1835;
    trig_errs_w[0] = 211.324;
    trig_errs_w[1] = 72.3718;
    trig_matr[0] = 44657.7;
    trig_matr[1] = -15288.5;
    trig_matr[2] = -15288.5;
    trig_matr[3] = 5237.68;
    trig_Chisquare = 30.7878;
    trig_NDF = 13;
    trig_cl = 0.68;
  } else if (whichTrigger == "HLT_xe90_mht_L1XE50") { // updated 25/04/2018
      trig_pars_w[0] = 59.1995; 
      trig_pars_w[1] = 45.9814; 
      trig_pars_t[0] = 40.4239;
      trig_pars_t[1] = 60.6205;
      trig_pars_z[0] = 66.2916;
      trig_pars_z[1] = 43.2338;
      trig_errs_w[0] = 8.6119;
      trig_errs_w[1] = 4.95622; 
      trig_matr[0] = 74.1649;
      trig_matr[1] = -40.3242;
      trig_matr[2] = -40.3242;
      trig_matr[3] = 24.5641;
      trig_Chisquare = 6.67933;
      trig_NDF = 13 ;
      trig_cl = 0.68; 
  } else if (whichTrigger == "HLT_xe100_mht_L1XE50") { // updated ICHEP
    trig_pars_w[0] = 13.2286;
    trig_pars_w[1] = 83.9684;
    trig_pars_t[0] = 44.88;
    trig_pars_t[1] = 49.17;
    trig_pars_z[0] = 0.;
    trig_pars_z[1] = 0.;
    trig_errs_w[0] = 14.5565;
    trig_errs_w[1] = 7.61802;
    trig_matr[0] = 211.891;
    trig_matr[1] = -108.173;
    trig_matr[2] = -108.173;
    trig_matr[3] = 58.0342;
    trig_Chisquare = 9.31627;
    trig_NDF = 13;
    trig_cl = 0.68;
  } else if (whichTrigger == "HLT_xe110_pufit_L1XE55") { // updated 25/04/2018
      trig_pars_w[0] = 82.0811; 
      trig_pars_w[1] = 42.7931; 
      trig_pars_t[0] = 79.5308;
      trig_pars_t[1] = 49.5452;
      trig_pars_z[0] = 78.3896;
      trig_pars_z[1] = 47.1482;
      trig_errs_w[0] = 3.96122;
      trig_errs_w[1] = 2.12058; 
      trig_matr[0] = 15.6913;
      trig_matr[1] = -7.52044;
      trig_matr[2] = -7.52044;
      trig_matr[3] = 4.49686;
      trig_Chisquare = 55.0089;
      trig_NDF = 13 ;
      trig_cl = 0.68; 
  } else if (whichTrigger == "HLT_xe110_pufit_xe70_L1XE50") { // updated 04/02/2019
      trig_pars_w[0] = 86.4507; 
      trig_pars_w[1] = 31.3349; 
      trig_pars_t[0] = 87.7555;
      trig_pars_t[1] = 32.6289;
      trig_pars_z[0] = 87.4842;
      trig_pars_z[1] = 31.6966;
      trig_errs_w[0] = 5.47602;
      trig_errs_w[1] = 3.17065; 
      trig_matr[0] = 29.9868;
      trig_matr[1] = -16.1618;
      trig_matr[2] = -16.1618;
      trig_matr[3] = 10.053;
      trig_Chisquare = 28.9104;
      trig_NDF = 13 ;
      trig_cl = 0.68; 
  } else if (whichTrigger == "HLT_xe110_pufit_xe65_L1XE50") { // updated 04/02/2019
      trig_pars_w[0] = 84.9476; 
      trig_pars_w[1] = 30.8057; 
      trig_pars_t[0] = 86.5001;
      trig_pars_t[1] = 32.0305;
      trig_pars_z[0] = 86.6911;
      trig_pars_z[1] = 30.6275;
      trig_errs_w[0] = 5.71445;
      trig_errs_w[1] = 3.28403; 
      trig_matr[0] = 32.6549;
      trig_matr[1] = -17.4769;
      trig_matr[2] = -17.4769;
      trig_matr[3] = 10.7849;
      trig_Chisquare = 25.7147;
      trig_NDF = 13 ;
      trig_cl = 0.68; 
  } else {
    //This appears for all events, making log files very large.
    //TriggerScaleFactorsMET... WARNING No SF for trigger HLT_xe110_pufit_L1XE55 available! Returning 1.
    //Creating a GitLab issue https://gitlab.cern.ch/CxAODFramework/CxAODTools/issues/12
    //and do not show this warning for this trigger any more
    if (whichTrigger != "HLT_xe110_pufit_L1XE55") {
      Warning("TriggerScaleFactorsMET::get_metTrigEff",
              "No Efficiency for trigger %s available! Returning 1.", whichTrigger.c_str());
    }
    triggerSF = 1.;
    return triggerSF;
  }

  if (met < 500e3) {
    if (met < 100e3) {
      // if the met is below 100GeV, return the SF at 100GeV point. 
      trig_met[0] = 100.; 
    }
    else {
      trig_met[0] = met / 1e3;
    }
    triggerSF = trig_SF->EvalPar(trig_met, trig_pars_w);
    if (variation == "METTrigStat__1up") {
      triggerSF += metSF_GetStatError(trig_met);
    } else if (variation == "METTrigStat__1down") {
      triggerSF -= metSF_GetStatError(trig_met);
    } else if (variation == "METTrigTop__1up" && trig_pars_t[0]!=0) {
      triggerSF += fabs(trig_SF->EvalPar(trig_met, trig_pars_w) - trig_SF->EvalPar(trig_met, trig_pars_t));
    } else if (variation == "METTrigTop__1down" && trig_pars_t[0]!=0) {
      triggerSF -= fabs(trig_SF->EvalPar(trig_met, trig_pars_w) - trig_SF->EvalPar(trig_met, trig_pars_t));
    } else if (variation == "METTrigZ__1up" && trig_pars_z[0]!=0) {
      triggerSF += fabs(trig_SF->EvalPar(trig_met, trig_pars_w) - trig_SF->EvalPar(trig_met, trig_pars_z));
    } else if (variation == "METTrigZ__1down" && trig_pars_z[0]!=0) {
      triggerSF -= fabs(trig_SF->EvalPar(trig_met, trig_pars_w) - trig_SF->EvalPar(trig_met, trig_pars_z));
    } else if (variation == "METTrigSumpt__1down" && sumpt!=0) {
        triggerSF *= metSF_GetMETTrigSumpt(sumpt, whichTrigger);
    }
  } else triggerSF = 1.;

  return triggerSF;
}

double TriggerScaleFactorsMET::metSF_GetStatError(const Double_t* MET) {

  // what is done here is essentially equal to the TVirtualFitter::GetConfidenceIntervals()
  trig_SF->SetParameters(trig_pars_w);
  trig_SF->SetParError(0, trig_errs_w[0]);
  trig_SF->SetParError(1, trig_errs_w[1]);
  Double_t t = TMath::StudentQuantile(0.5 + trig_cl / 2, trig_NDF);
  Double_t chidf = TMath::Sqrt(trig_Chisquare / trig_NDF);

  Double_t grad[2];
  Double_t sum_vector[2];
  Double_t c = 0;
  trig_SF->GradientPar(MET, grad);
  // multiply the covariance matrix by gradient
  sum_vector[0] = trig_matr[0] * grad[0] + trig_matr[1] * grad[1];
  sum_vector[1] = trig_matr[2] * grad[0] + trig_matr[3] * grad[1];
  c = TMath::Sqrt(grad[0] * sum_vector[0] + grad[1] * sum_vector[1]);

  return c * t*chidf;
}

double TriggerScaleFactorsMET::metSF_GetMETTrigSumpt(const Double_t sumpt, const std::string whichTrigger) {
    double sys_weight = 1.;
    if(sumpt<0){
      Warning("TriggerScaleFactorsMET::metSF_GetMETTrigSumpt",
              "Sumpt is not set for computing systematics METTrigSumpt, returning 1. Please call m_triggerTool->setSumpt(sumpt) before trigger decision.");
    }
    else {
        if(whichTrigger == "HLT_xe110_pufit_L1XE55" ){
            if(sumpt<160e3) sys_weight =0.79;
            else if(sumpt<170e3) sys_weight =0.87;
            else if(sumpt<180e3) sys_weight =0.94;
            else if(sumpt<190e3) sys_weight =0.94;
            else if(sumpt<200e3) sys_weight =0.99;
        }
        else if( whichTrigger == "HLT_xe110_pufit_xe70_L1XE50"){
            if(sumpt<160e3) sys_weight =0.83;
            else if(sumpt<170e3) sys_weight =0.92;
            else if(sumpt<180e3) sys_weight =0.97;
            else if(sumpt<190e3) sys_weight =0.98;
        }
        else if (whichTrigger == "HLT_xe110_pufit_xe65_L1XE50"){
            if(sumpt<160e3) sys_weight =0.83;
            else if(sumpt<170e3) sys_weight =0.93;
            else if(sumpt<180e3) sys_weight =0.99;
        }
        else sys_weight = 1;
    }
    return sys_weight;
}
