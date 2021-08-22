#ifndef CxAODTools_TriggerScaleFactorsMET_H
#define CxAODTools_TriggerScaleFactorsMET_H

#include "TMath.h"

class TF1;

class TriggerScaleFactorsMET {

public:

  TriggerScaleFactorsMET ();
  virtual ~TriggerScaleFactorsMET ();

  virtual double get_metTrigEff(const Double_t met, const std::string whichTrigger);
  virtual double get_metTrigSF(const Double_t met, const Double_t sumpt, const std::string variation, const std::string whichTrigger);

protected:

  virtual double metSF_GetStatError(const Double_t* MET);
  virtual double metSF_GetMETTrigSumpt(const Double_t sumpt, const std::string whichTrigger);

  TF1 *trig_SF;
  Double_t trig_met[1];
  Double_t trig_pars_w[2]; //fit result parameters
  Double_t trig_pars_t[2];
  Double_t trig_pars_z[2];
  Double_t trig_errs_w[2]; //errors of the pars
  Double_t trig_matr[4]; //covariance matrix
  Double_t trig_Chisquare; //chisquare
  int trig_NDF; //number of degrees of freedom
  Double_t trig_cl; // confidence level of the fit

};

#endif // ifndef CxAODTools_TriggerScaleFactorsMET_H
