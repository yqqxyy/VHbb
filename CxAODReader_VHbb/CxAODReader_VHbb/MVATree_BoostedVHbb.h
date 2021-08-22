#ifndef CxAODReader_MVATree_BoostedVHbb_H
#define CxAODReader_MVATree_BoostedVHbb_H

#include "CxAODReader/MVATree.h"

class MVATree_BoostedVHbb : public MVATree {
 protected:
  std::string m_analysisType;

  virtual void AddBranch(TString name, Float_t *address);
  virtual void AddBranch(TString name, Int_t *address);
  virtual void AddBranch(TString name, std::string *address);
  virtual void AddBranch(TString name, ULong64_t *address);
  virtual void SetBranches() override;
  virtual void TransformVars() override;
  float getBinnedMV1c(float MV1c);

 public:
  MVATree_BoostedVHbb(bool persistent, bool readMVA, std::string analysisType,
                      EL::IWorker *wk, std::vector<std::string> variations,
                      bool nominalOnly);
  //  MVATree_BoostedVHbb(bool persistent, bool readMVA, std::string
  //  analysisType, EL::IWorker* wk);//, std::vector<std::string> variations,
  //  bool nominalOnly);

  ~MVATree_BoostedVHbb() {}

  virtual void Reset() override;
  virtual void ReadMVA();

  std::string sample;

  float MV1cB1;
  float MV1cB2;

  // EVENT INFO //
  float EventWeight;
  unsigned long long EventNumber;
  // COUNTERS //
  float nSigJets;
  float pTSigJet1;
  float pTSigJet2;
  float pTSigJet3;
  float etaSigJet1;
  float etaSigJet2;
  float etaSigJet3;
  float nFwdJets;
  float pTFwdJet1;
  float pTFwdJet2;
  float pTFwdJet3;
  float etaFwdJet1;
  float etaFwdJet2;
  float etaFwdJet3;
  float nFatJets;
  float nMatchTrkJetLeadFatJet;
  float nBTagMatchTrkJetLeadFatJet;
  float nBTagUnmatchTrkJetLeadFatJet;
  // VECTOR BOSON //
  float MPT;
  float MET;
  // HIGGS: LEAD FAT JET //
  float pTLeadFatJet;
  float etaLeadFatJet;
  float phiLeadFatJet;
  float C2LeadFatJet;
  float mJ;
  // SUB-LEAD FAT JET //
  // float pTSubLeadFatJet;
  // float etaSubLeadFatJet;
  // float phiSubLeadFatJet;
  // float MSubLeadFatJet;
  // DPHIS //
  float dPhiMETMPT;
  float dPhiMETLeadFatJet;
  float mindPhiMETJets;
  float mindPhiMETJets_3leadjets;
  // VH //
  float mVH;
  float mVH_rescaled;
  float pTVH;
  float MEff;

  float BDT;
};

#endif
