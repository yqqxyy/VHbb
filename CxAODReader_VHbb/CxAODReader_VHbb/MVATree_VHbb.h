#ifndef CxAODReader_MVATree_VHbb_H
#define CxAODReader_MVATree_VHbb_H

#include "CxAODReader/MVATree.h"
#include "CxAODReader/StaticMVATree.h"

namespace CxAODReader_MVATree_VHbb_internal {
template <class Tree>
using STree = StaticMVATree<Tree, bool, int, unsigned, long, unsigned long,
                            float, double, std::string>;
}

class MVATree_VHbb
    : public MVATree,
      public CxAODReader_MVATree_VHbb_internal::STree<MVATree_VHbb> {
  friend CxAODReader_MVATree_VHbb_internal::STree<MVATree_VHbb>;

 protected:
  std::string m_analysisType;

  virtual void AddBranch(TString name, Float_t *address);
  virtual void AddBranch(TString name, Int_t *address);
  virtual void AddBranch(TString name, ULong64_t *address);
  virtual void AddBranch(TString name, std::string *address);

  template <class T>
  void AddBranch(const TString &name, T *address) {
    for (std::pair<std::string, TTree *> tree : m_treeMap) {
      tree.second->Branch(name, address);
    }
  }
  virtual void SetBranches() override;
  virtual void TransformVars() override;

 public:
  MVATree_VHbb(bool persistent, bool readMVA, std::string analysisType,
               EL::IWorker *wk, const std::vector<std::string> &variations,
               bool nominalOnly);
  //  MVATree_VHbb(bool persistent, bool readMVA, std::string analysisType,
  //  EL::IWorker* wk);//, std::vector<std::string> variations, bool
  //  nominalOnly);

  ~MVATree_VHbb() {}

  virtual void Reset() override;
  void Fill();

  TTree *getTree(const std::string treeName);

  // DO NOT ENTER NEW VARIABLES HERE IF NOT ABSOLUTELY NECESSARY FOR MVA
  // please use the EasyTree functionality for studies

  // event info
  std::string sample;
  float EventWeight;
  float bTagWeight;
  unsigned long long EventNumber;
  int ChannelNumber;

  // event variables
  int nSigJet;
  int nForwardJet;
  int nJ;
  int nTags;
  int nTaus;
  float MET;
  float MEff;
  float METSig;
  float dPhiVBB;
  float dEtaVBB;
  float sumPtJets;
  float softMET;
  float hasFSR;

  // vector boson
  float pTV;
  float etaV;
  float phiV;

  // higgs boson/jet system
  float mBB;
  float dRBB;
  float dEtaBB;
  float dPhiBB;
  float mBBJ;

  // jets
  float pTB1;
  float pTB2;
  float pTJ3;
  float etaB1;
  float etaB2;
  float etaJ3;
  float phiB1;
  float phiB2;
  float phiJ3;
  float mB1;
  float mB2;
  float mJ3;
  float bin_MV2c10B1;
  float bin_MV2c10B2;
  float bin_MV2c10J3;

  // special 1 lepton
  float Mtop;
  float dYWH;
  float dPhiLBmin;
  float mTW;
  float pTL;
  float etaL;
  float phiL;

  // special 2 lepton
  float mLL;
  float pTL1;
  float pTL2;
  float etaL1;
  float etaL2;
  float phiL1;
  float phiL2;
  float cosThetaLep;

  // BDT output of default MVA
  float BDT;
  float BDT_VZ;
};

#endif
