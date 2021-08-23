#ifndef CxAODReader_MVATree_VBFHbb_H
#define CxAODReader_MVATree_VBFHbb_H

#include "CxAODReader/MVATree.h"
#include "vector"

class MVATree_VBFHbb : public MVATree {

 protected:
  
  std::string m_analysisType;

  virtual void AddBranch(TString name, Float_t* address);
  virtual void AddBranch(TString name, Int_t* address);
  virtual void AddBranch(TString name, ULong64_t * address);
  virtual void AddBranch(TString name, std::string* address);

  // Additional methods to store std::vectors
  virtual void AddBranch(TString name,std::vector<int> *address);
  virtual void AddBranch(TString name,std::vector<float> *address);

  virtual void SetBranches() override;
  virtual void TransformVars() override;

 public:

  MVATree_VBFHbb(bool persistent, bool readMVA, std::string analysisType, EL::Worker* wk, std::vector<std::string> variations, bool nominalOnly);

  ~MVATree_VBFHbb() {}

  virtual void Reset() override;
  virtual void ReadMVA();

  std::string sample;

  int RunNumber;
  int LumiBlock;
  unsigned long long EventNumber;
  int npv;
  float MCWeight;
  float EventWeight;
  float BJetTriggerWeight;
  float BJetSF;
  int MCChannelNumber;

  int nJets;
  int nJets20pt;
  int nJets30pt;
  int nJets40pt;
  int nJets50pt;
  int nJets60pt;

  int nTightMv2c20;
  int nMediumMv2c20;
  int nLooseMv2c20;

  int nTightMv2c10;
  int nMediumMv2c10;
  int nLooseMv2c10;

  std::vector<float> pT;
  std::vector<float> phi;
  std::vector<float> eta;
  std::vector<float> mv2c10;
  std::vector<float> mv2c20;
  std::vector<float> jetWidth;
  std::vector<int>   truthLabel;
  std::vector<int>   truthLabelID;
  std::vector<int>   coneTruthLabelID;
  std::vector<int>   hadronConeExclTruthLabelID;
  std::vector<int>   partonTruthLabelID;
  std::vector<float> weightSysts;
  std::vector<float> weightspdf4lhc;
  std::vector<float> weightspdfnnpdf30;
  std::vector<float> weightsqcdnnlops;
  std::vector<float> weightsqcdnnlops2np;
  std::vector<float> weightsqcdwg1;
  std::vector<float> weightsalternativepdf;

  float alpha_s_up;
  float alpha_s_dn;

  int whoIsJ1;
  int whoIsJ2;
  int whoIsB1;
  int whoIsB2;

  float mBB;
  float mBB_no_corr;
  float dRBB;
  float dRBB_no_corr;
  float dPhiBB;
  float dEtaBB;
  float pTBB;
  float pTBB_no_corr;

  float mJJ;
  float mJ1B1;
  float mJ1B2;
  float dRJJ;
  float dPhiJJ;
  float dEtaJJ;
  float pTJJ;

  float minEta;
  float maxEta;

  float pTJ1;
  float pTJ2;
  float pTB1;
  float pTB2;
  float pTB1_no_corr;
  float pTB2_no_corr;

  float J1FJVTLoose;
  float J2FJVTLoose;
  float J1FJVTTight;
  float J2FJVTTight;

  float etaJ1;
  float etaJ2;
  float etaB1;
  float etaB2;
  float etaB1_no_corr;
  float etaB2_no_corr;

  float dRB1J1;
  float dRB1J2;
  float dRB2J1;
  float dRB2J2;

  float TruthLabelB1;
  float TruthLabelB2;
  float TruthLabelPartonB1;
  float TruthLabelPartonB2;
  float HadronConeExclTruthLabelB1;
  float HadronConeExclTruthLabelB2;

  float MV2c20B1;
  float MV2c20B2;
  float MV2c20J1;
  float MV2c20J2;

  float MV2c10B1;
  float MV2c10B2;
  float MV2c10J1;
  float MV2c10J2;

  float WidthJ1;
  float WidthJ2;

  float NTrk1000PVJ1;
  float NTrk1000PVJ2;
  float NTrk500PVJ1;
  float NTrk500PVJ2;

  float QGTaggerJ1;
  float QGTaggerJ2;
  float QGTaggerWeightJ1;
  float QGTaggerWeightJ2;

  float pT_ballance;

  float mindRJ1;
  float mindRJ2;
  float mindRJ1_Ex;
  float mindRJ2_Ex;
  float mindRB1;
  float mindRB2;

  // =======                                                                   

  float max_J1J2;
  float eta_J_star;
  float HT_soft;
  float HT_MVA;
  float cosTheta_MVA;
  float cosTheta_CMS;
  float cosTheta_boost;

  // =======                                                                                                                                                             
  /* int passCURRENT; */
  /* int passATLAS; */
  /* int passCMS; */
  /* int passHYBRID; */

  /* int whoIsB1_CURRENT; */
  /* int whoIsB2_CURRENT; */
  /* int whoIsB1_ATLAS; */
  /* int whoIsB2_ATLAS; */
  /* int whoIsB1_CMS; */
  /* int whoIsB2_CMS; */
  /* int whoIsB1_HYBRID; */
  /* int whoIsB2_HYBRID; */

  // =======

  std::vector<float> track_jet_pt;
  std::vector<float> track_jet_eta;
  std::vector<int> track_jet_n;

  //  float BDT;

  // ==================================================================== 
  // trigger decisions
  int pass_L1_2J25;
  int pass_L1_J25_0ETA23;
  int pass_L1_HT150_JJ15_ETA49;
  int pass_L1_2J15_31ETA49;
  int pass_L1_HT150_J20s5_ETA31;
  int pass_L1_MJJ_400;
  int pass_L1_J40_0ETA25;
  int pass_L1_MJJ_400_CF;
  int pass_L1_J20_31ETA49;
  int pass_L1_J40_0ETA25_2J25_J20_31ETA49;
  int pass_L1_J25_0ETA23_2J15_31ETA49;
  int pass_L1_HT150_J20s5_ETA31_MJJ_400_CF;
  int pass_L1_J40_0ETA25_2J15_31ETA49;
  int pass_L1_HT150_JJ15_ETA49_MJJ_400;
  int pass_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490;
  int pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49;
  int pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split;
  int pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split;
  int pass_HLT_j80_0eta240_j60_j45_320eta490;
  int pass_HLT_j80_0eta240_2j60_320eta490;
  int pass_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49;

  // ==================================================================== 
  // Trigger is Active
  int isActive_L1_2J25;
  int isActive_L1_J25_0ETA23;
  int isActive_L1_HT150_JJ15_ETA49;
  int isActive_L1_2J15_31ETA49;
  int isActive_L1_HT150_J20s5_ETA31;
  int isActive_L1_MJJ_400;
  int isActive_L1_J40_0ETA25;
  int isActive_L1_MJJ_400_CF;
  int isActive_L1_J20_31ETA49;
  int isActive_L1_J40_0ETA25_2J25_J20_31ETA49;
  int isActive_L1_J25_0ETA23_2J15_31ETA49;
  int isActive_L1_HT150_J20s5_ETA31_MJJ_400_CF;
  int isActive_L1_J40_0ETA25_2J15_31ETA49;
  int isActive_L1_HT150_JJ15_ETA49_MJJ_400;
  int isActive_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490;
  int isActive_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49;
  int isActive_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split;
  int isActive_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split;
  int isActive_HLT_j80_0eta240_j60_j45_320eta490;
  int isActive_HLT_j80_0eta240_2j60_320eta490;
  int isActive_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49;

  // ==================================================================== 
  // Trigger Pre-Scales
  float prescale_L1_2J25;
  float prescale_L1_J25_0ETA23;
  float prescale_L1_HT150_JJ15_ETA49;
  float prescale_L1_2J15_31ETA49;
  float prescale_L1_HT150_J20s5_ETA31;
  float prescale_L1_MJJ_400;
  float prescale_L1_J40_0ETA25;
  float prescale_L1_MJJ_400_CF;
  float prescale_L1_J20_31ETA49;
  float prescale_L1_J40_0ETA25_2J25_J20_31ETA49;
  float prescale_L1_J25_0ETA23_2J15_31ETA49;
  float prescale_L1_HT150_J20s5_ETA31_MJJ_400_CF;
  float prescale_L1_J40_0ETA25_2J15_31ETA49;
  float prescale_L1_HT150_JJ15_ETA49_MJJ_400;
  float prescale_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490;
  float prescale_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49;
  float prescale_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split;
  float prescale_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split;
  float prescale_HLT_j80_0eta240_j60_j45_320eta490;
  float prescale_HLT_j80_0eta240_2j60_320eta490;
  float prescale_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49;

  // ==================================================================== 
};

#endif
