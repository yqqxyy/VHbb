#ifndef BDTSyst_h
#define BDTSyst_h

#include <TSystem.h>
#include <TSystemDirectory.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "TF1.h"
#include "TMVA/Config.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

class BDTSyst {
 private:
  std::map<TString, TF1 *> m_ShapeForm_tf1;
  std::map<TString, TH1 *> m_ShapeForm_th1;

  TString m_sample;
  std::map<TString, TMVA::Reader *> m_BDTFactories;
  void AddFactory(TString TF1Name);
  TString DetermineTF1Name(TString variation, int channel);
  TString DetermineTH1Name(TString variation, int channel);
  TString DetermineXMLName(TString variation, int channel);
  TString DetermineMVAToEval(int channel);

  int m_debug;
  bool m_useTF1;

 public:
  std::vector<TString> getExpressions(TString xmlFile,
                                      std::vector<TString> &spectators,
                                      std::vector<TString> &BDTs);
  void Initialise(int channel);
  void ShapeProvider(int channel);
  float DetermineWeight(TString variation, TString sample, int channel,
                        float &BDTScore);
  void ResetInputs();

  BDTSyst();
  ~BDTSyst();

  std::string m_WorkDir_DIR = gSystem->Getenv("WorkDir_DIR");

  std::map<TString, Float_t> m_Variables;
};

#endif  // BDTSyst_HPP_
