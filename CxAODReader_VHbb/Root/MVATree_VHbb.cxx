#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "EventLoop/IWorker.h"

MVATree_VHbb::MVATree_VHbb(bool persistent, bool readMVA,
                           std::string analysisType, EL::IWorker* wk,
                           const std::vector<std::string>& variations,
                           bool nominalOnly)
    : MVATree(persistent, readMVA, wk, variations, nominalOnly),
      CxAODReader_MVATree_VHbb_internal::STree<MVATree_VHbb>(),
      m_analysisType(analysisType),
      sample() {
  SetBranches();
}

void MVATree_VHbb::AddBranch(TString name, Int_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_VHbb::AddBranch(TString name, Float_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_VHbb::AddBranch(TString name, ULong64_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_VHbb::AddBranch(TString name, std::string* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
}

void MVATree_VHbb::SetBranches() {
  // DO NOT ENTER NEW VARIABLES HERE IF NOT ABSOLUTELY NECESSARY FOR MVA
  // please use the EasyTree functionality for studies

  // event info
  AddBranch("sample", &sample);
  AddBranch("EventWeight", &EventWeight);
  AddBranch("bTagWeight", &bTagWeight);
  AddBranch("EventNumber", &EventNumber);
  AddBranch("ChannelNumber", &ChannelNumber);

  // event variables
  AddBranch("nJ", &nJ);
  AddBranch("nSigJet", &nSigJet);
  AddBranch("nForwardJet", &nForwardJet);
  AddBranch("nTags", &nTags);
  AddBranch("nTaus", &nTaus);
  AddBranch("MET", &MET);
  AddBranch("MEff", &MEff);
  AddBranch("METSig", &METSig);
  AddBranch("dPhiVBB", &dPhiVBB);
  AddBranch("dEtaVBB", &dEtaVBB);
  AddBranch("sumPtJets", &sumPtJets);
  AddBranch("softMET", &softMET);
  AddBranch("hasFSR", &hasFSR);

  // vector boson
  AddBranch("pTV", &pTV);
  AddBranch("phiV", &phiV);

  // higgs boson/jet system
  AddBranch("dRBB", &dRBB);
  AddBranch("dPhiBB", &dPhiBB);
  AddBranch("dEtaBB", &dEtaBB);
  AddBranch("mBB", &mBB);
  AddBranch("mBBJ", &mBBJ);

  // jets
  AddBranch("pTB1", &pTB1);
  AddBranch("pTB2", &pTB2);
  AddBranch("pTJ3", &pTJ3);
  AddBranch("etaB1", &etaB1);
  AddBranch("etaB2", &etaB2);
  AddBranch("etaJ3", &etaJ3);
  AddBranch("phiB1", &phiB1);
  AddBranch("phiB2", &phiB2);
  AddBranch("phiJ3", &phiJ3);
  AddBranch("mB1", &mB1);
  AddBranch("mB2", &mB2);
  AddBranch("mJ3", &mJ3);
  AddBranch("bin_MV2c10B1", &bin_MV2c10B1);
  AddBranch("bin_MV2c10B2", &bin_MV2c10B2);
  AddBranch("bin_MV2c10J3", &bin_MV2c10J3);

  // special 1 lepton
  if (m_analysisType == "1lep") {
    AddBranch("Mtop", &Mtop);
    AddBranch("dYWH", &dYWH);
    AddBranch("dPhiLBmin", &dPhiLBmin);
    AddBranch("mTW", &mTW);
    AddBranch("pTL", &pTL);
    AddBranch("etaL", &etaL);
    AddBranch("phiL", &phiL);
  }

  // special 2 lepton
  if (m_analysisType == "2lep") {
    AddBranch("mLL", &mLL);
    AddBranch("etaV", &etaV);
    AddBranch("pTL1", &pTL1);
    AddBranch("pTL2", &pTL2);
    AddBranch("etaL1", &etaL1);
    AddBranch("etaL2", &etaL2);
    AddBranch("phiL1", &phiL1);
    AddBranch("phiL2", &phiL2);
    AddBranch("cosThetaLep", &cosThetaLep);
  }

  AddBranch("BDT", &BDT);
  AddBranch("BDT_VZ", &BDT_VZ);
}

void MVATree_VHbb::TransformVars() {}

void MVATree_VHbb::Fill() {
  StaticSetBranches(m_analysisType, this);
  MVATree::Fill();
}

void MVATree_VHbb::Reset() {
  // DO NOT ENTER NEW VARIABLES HERE IF NOT ABSOLUTELY NECESSARY FOR MVA
  // please use the EasyTree functionality for studies

  // event info
  sample = "Unknown";
  EventWeight = 0.;
  bTagWeight = -99;
  EventNumber = -99;
  ChannelNumber = -99;

  // event variables
  nJ = -99;
  nSigJet = -99;
  nForwardJet = -99;
  nTags = -99;
  nTaus = -99;
  MET = -99.;
  METSig = -99.;
  dPhiVBB = -99.;
  dEtaVBB = -99.;
  sumPtJets = -99.;
  softMET = -99.;
  hasFSR = -99.;

  // vector boson
  pTV = -99.;
  etaV = -99.;
  phiV = -99.;

  // higgs boson/jet system
  dRBB = -99.;
  dPhiBB = -99.;
  dEtaBB = -99.;
  mBB = -99.;
  mBBJ = -99.;

  // jets
  pTB1 = -99.;
  pTB2 = -99.;
  pTJ3 = -99.;
  etaB1 = -99.;
  etaB2 = -99.;
  etaJ3 = -99.;
  phiB1 = -99.;
  phiB2 = -99.;
  phiJ3 = -99.;
  mB1 = -99.;
  mB2 = -99.;
  mJ3 = -99.;
  bin_MV2c10B1 = -99.;
  bin_MV2c10B2 = -99.;
  bin_MV2c10J3 = -99.;

  // special 1 lepton
  Mtop = -99.;
  dYWH = -99.;
  dPhiLBmin = -99.;
  mTW = -99.;
  pTL = -99.;
  etaL = -99.;
  phiL = -99.;

  // special 2 lepton
  mLL = -99.;
  pTL1 = -99.;
  pTL2 = -99.;
  etaL1 = -99.;
  etaL2 = -99.;
  phiL1 = -99.;
  phiL2 = -99.;
  cosThetaLep = -99.;

  // BDT output
  BDT = -99.;
  BDT_VZ = -99.;

  StaticReset();
}

TTree* MVATree_VHbb::getTree(const std::string treeName) {
  if (m_treeMap.find(treeName) == m_treeMap.end()) {
    Error("MVATree_VHbb::getTree()", "Could not get tree with name '%s'!",
          treeName.c_str());
    exit(EXIT_FAILURE);
  }
  return m_treeMap[treeName];
}
