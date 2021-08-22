#ifndef CxAODReader_MVATree_VBFHbb_H
#define CxAODReader_MVATree_VBFHbb_H

#include "CxAODReader/MVATree.h"

class MVATree_VBFHbb : public MVATree {
 protected:
  std::string m_analysisType;

  virtual void AddBranch(TString name, Float_t *address);
  virtual void AddBranch(TString name, Int_t *address);
  virtual void AddBranch(TString name, ULong64_t *address);
  virtual void AddBranch(TString name, std::string *address);
  virtual void SetBranches() override;
  virtual void TransformVars() override;

 public:
  MVATree_VBFHbb(bool persistent, bool readMVA, std::string analysisType,
                 EL::IWorker *wk, std::vector<std::string> variations,
                 bool nominalOnly, std::string MVAxmlFileName);

  ~MVATree_VBFHbb() {}

  virtual void Reset() override;
  virtual void ReadMVA();

  std::string m_xmlFileName;

  int passTrig;
  int matchTrig;
  int passMVAPreSel;

  std::string sample;
  int MCChannelNumber;
  int randomRunNumber;
  float EventWeight;
  float dEtaJJSF;
  float mindRBPhSF;
  float pTJJSF;
  float pTBalSF;
  float bjetTrigPt_SF;
  float bjetTrigEta_SF;
  int RunNumber;
  unsigned long long EventNumber;
  float AverageMu;
  float ActualMu;
  float AverageMuScaled;
  float ActualMuScaled;
  int Nvtx;
  float ZPV;

  int tagCatExcl;
  int tagCatExclDirect;

  //  int nJ;
  int nJets;
  float mJJ;
  float pTJJ;
  float dEtaJJ;
  float dRJJ;
  float dPhiJJ;
  float maxEta;

  float mBB;
  float mBBNoCorr;
  float dRBB;
  float dPhiBB;
  float dEtaBB;
  float pTBB;
  float dPhiPhBmin;
  float mBBPh;

  //  int nJetGap;
  //  float HT_gap;
  float HT_soft;
  //  float HT_soft_SCALEUP;
  //  float HT_soft_SCALEDOWN;
  //  float HT_soft_RESOPARA;
  //  float HT_soft_RESOPERP;
  float pTBal;
  //
  //  int nTrkJet2;
  //  int nTrkJet5;
  //  int nTrkJet10;
  float dEtaJJBB;
  float dPhiBBJJ;
  float pTTot;
  float pZTot;
  float HT;
  float pTJ5;
  //  float pTJ4;
  float etaJStar;
  //  float cosThetaA;
  float cosThetaC;
  //
  //  float cenHJJ;
  //  float cenHgJJ;
  float cenPhJJ;

  float pTJ1;
  float pTJ2;
  float pTB1;
  float pTB2;
  float pTPh;

  float etaJ1;
  float etaJ2;
  float etaB1;
  float etaB2;
  float etaPh;

  float MV2c10B1;
  float MV2c10B2;
  float WidthJ1;
  float WidthJ2;

  float dRB1Ph;
  float dRB2Ph;
  float dRJ1Ph;
  float dRJ2Ph;

  float dRB1J1;
  float dRB1J2;
  float dRB2J1;
  float dRB2J2;

  float nTrkPt5J1;
  float nTrkPt5J2;
  //  float nTrkPt10J1;
  //  float nTrkPt10J2;
  float SumPtExtraJets;
  float mindRBPh;
  float mindRJPh;
  //  float dR_BPhovJPh;
  //  float mindRJ1J;
  //  float mindRJ2J;
  //  float mindRB1J;
  //  float mindRB2J;
  float mindRJ1Ji;
  float mindRJ2Ji;

  //  int passVBF18;
  //  int passVBF20;
  //  int passVBF22;
  //  int passVBF0b;
  //  int passVBF1b;
  //  int passVBF1bs;
  //  int passVBF2b;
  //  int passVBF2bs;
  //  int passVBF25g4j;
  //  int passEM13VH;
  //  int passEM15VH;
  //  int passEM18VH;
  //  int passEM20VH;
  //  int passEM22VHI;
  //  int passg10;
  //  int passg15;
  //  int passg20;
  //  int passg20m;
  //  int passg25;
  //  int passg35;
  //  int passg40;
  //  int passg45;
  //  int passg50;
  //  int passg60;
  //  int passg70;
  //  int passg80;
  //  int passg100;
  //  int passg120;
  //
  //  float l1_emet;
  //  float hlt_phpt;
  //  float hlt_j1pt;
  //  float hlt_j2pt;
  //  float hlt_j3pt;
  //  float hlt_j4pt;
  //  float hlt_j5pt;
  //  float hlt_mjj;
  //  float hlt_phpt_matched;
  //  float hlt_j4pt_phor;
  //  float hlt_mjj_phor;

  float weight_forwardJetScale_up;
  float weight_forwardJetScale_down;
  float weight_pTBalScale_up;
  float weight_pTBalScale_down;
  float weight_pTBalPS_up;
  float weight_pTBalPS_down;
  float weight_TrigJetEff_up;
  float weight_TrigJetEff_down;
  float weight_TrigL1EMEffon;

  float BDT;

  // std::vector<float> track_jet_pt;
  // std::vector<float> track_jet_eta;
  // std::vector<int> track_jet_n;
  // float mBB_Regression;
  // float mBB_OneMu;
  // float mBB_PtRecollbbOneMuPartonBukinNew;
};

#endif
