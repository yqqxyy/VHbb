#include "CxAODReader_VHbb/MVATree_BoostedVHbb.h"
#include "EventLoop/IWorker.h"

MVATree_BoostedVHbb::MVATree_BoostedVHbb(bool persistent, bool readMVA,
                                         std::string analysisType,
                                         EL::IWorker* wk,
                                         std::vector<std::string> variations,
                                         bool nominalOnly)
    : MVATree(persistent, readMVA, wk, variations, nominalOnly),
      m_analysisType(analysisType) {
  SetBranches();
}

void MVATree_BoostedVHbb::AddBranch(TString name, Int_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_BoostedVHbb::AddBranch(TString name, Float_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_BoostedVHbb::AddBranch(TString name, ULong64_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_BoostedVHbb::AddBranch(TString name, std::string* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
}

void MVATree_BoostedVHbb::SetBranches() {
  // prepare MVA reader
  m_reader.SetSplitVar(&EventNumber);
  m_reader.AddReader("reader", 2);

  AddBranch("sample", &sample);

  AddBranch("MV1cB1", &MV1cB1);
  AddBranch("MV1cB2", &MV1cB2);
  // EVENT INFO //
  AddBranch("EventWeight", &EventWeight);
  AddBranch("EventNumber", &EventNumber);
  // COUNTERS //
  AddBranch("nSigJets", &nSigJets);
  AddBranch("pTSigJet1", &pTSigJet1);
  AddBranch("pTSigJet2", &pTSigJet2);
  AddBranch("pTSigJet3", &pTSigJet3);
  AddBranch("etaSigJet1", &etaSigJet1);
  AddBranch("etaSigJet2", &etaSigJet2);
  AddBranch("etaSigJet3", &etaSigJet3);
  AddBranch("nFwdJets", &nFwdJets);
  AddBranch("pTFwdJet1", &pTFwdJet1);
  AddBranch("pTFwdJet2", &pTFwdJet2);
  AddBranch("pTFwdJet3", &pTFwdJet3);
  AddBranch("etaFwdJet1", &etaFwdJet1);
  AddBranch("etaFwdJet2", &etaFwdJet2);
  AddBranch("etaFwdJet3", &etaFwdJet3);
  AddBranch("nFatJets", &nFatJets);
  AddBranch("nMatchTrkJetLeadFatJet", &nMatchTrkJetLeadFatJet);
  AddBranch("nBTagMatchTrkJetLeadFatJet", &nBTagMatchTrkJetLeadFatJet);
  AddBranch("nBTagUnmatchTrkJetLeadFatJet", &nBTagUnmatchTrkJetLeadFatJet);
  // VECTOR BOSON //
  AddBranch("MET", &MET);
  AddBranch("MPT", &MPT);
  // HIGGS: LEAD FAT JET //
  AddBranch("pTLeadFatJet", &pTLeadFatJet);
  AddBranch("etaLeadFatJet", &etaLeadFatJet);
  AddBranch("phiLeadFatJet", &phiLeadFatJet);
  AddBranch("mJ", &mJ);
  // SUB-LEAD FAT JET //
  // AddBranch("pTSubLeadFatJet", &pTSubLeadFatJet);
  // AddBranch("etaSubLeadFatJet", &etaSubLeadFatJet);
  // AddBranch("phiSubLeadFatJet", &phiSubLeadFatJet);
  // AddBranch("MSubLeadFatJet", &MSubLeadFatJet);
  // DPHIS //
  AddBranch("dPhiMETMPT", &dPhiMETMPT);
  AddBranch("dPhiMETLeadFatJet", &dPhiMETMPT);
  AddBranch("mindPhiMETJets", &mindPhiMETJets);
  AddBranch("mindPhiMETJets_3leadjets", &mindPhiMETJets_3leadjets);
  // VH //
  AddBranch("mVH", &mVH);
  AddBranch("mVH_rescaled", &mVH_rescaled);
  AddBranch("pTVH", &pTVH);
  AddBranch("MEff", &MEff);

  // ADDED FOR BOOSTED ANALYSIS

  AddBranch("BDT", &BDT);
}

float MVATree_BoostedVHbb::getBinnedMV1c(float MV1c) {
  const int nbins = 5;
  float xbins[nbins + 1] = {0, 0.4050, 0.7028, 0.8353, 0.9237, 1.0};

  if (MV1c >= 1) MV1c = 1 - 1e-5;
  if (MV1c <= 0) MV1c = 1e-5;

  for (int i = 0; i < nbins; i++) {
    if (MV1c >= xbins[i] && MV1c < xbins[i + 1]) {
      return (xbins[i] + xbins[i + 1]) / 2;
    }
  }

  return 0;
}

void MVATree_BoostedVHbb::TransformVars() {
  MV1cB1 = getBinnedMV1c(MV1cB1);
  MV1cB2 = getBinnedMV1c(MV1cB2);
}

void MVATree_BoostedVHbb::Reset() {
  sample = "Unknown";

  MV1cB1 = -1;
  MV1cB2 = -1;

  // EVENT INFO //
  EventWeight = 0;
  EventNumber = -1;
  // COUNTERS //
  nSigJets = -1;
  pTSigJet1 = -1;
  pTSigJet2 = -1;
  pTSigJet3 = -1;
  etaSigJet1 = -1;
  etaSigJet2 = -1;
  etaSigJet3 = -1;
  nFwdJets = -1;
  pTFwdJet1 = -1;
  pTFwdJet2 = -1;
  pTFwdJet3 = -1;
  etaFwdJet1 = -1;
  etaFwdJet2 = -1;
  etaFwdJet3 = -1;
  nFatJets = -1;
  nMatchTrkJetLeadFatJet = -1;
  nBTagMatchTrkJetLeadFatJet = -1;
  nBTagUnmatchTrkJetLeadFatJet = -1;
  // VECTOR BOSON //
  MPT = -1;
  MET = -1;
  // HIGGS: LEAD FAT JET //
  pTLeadFatJet = -1;
  etaLeadFatJet = -1;
  phiLeadFatJet = -1;
  C2LeadFatJet = -1;
  mJ = -1;
  // SUB-LEAD FAT JET //
  // pTSubLeadFatJet = -1;
  // etaSubLeadFatJet = -1;
  // phiSubLeadFatJet = -1;
  // MSubLeadFatJet = -1;
  // DPHIS //
  dPhiMETLeadFatJet = -1;
  dPhiMETMPT = -1;
  mindPhiMETJets = -1;
  mindPhiMETJets_3leadjets = -1;
  // VH //
  mVH = -1;
  mVH_rescaled = -1;
  pTVH = -1;
  MEff = -1;

  BDT = -1;
}

void MVATree_BoostedVHbb::ReadMVA() {
  if (!m_readMVA) return;

  BDT = m_reader.EvaluateMVA("reader");
}
