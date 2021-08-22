#include "CxAODReader_VHbb/MVATree_VBFHbb.h"
#include "EventLoop/IWorker.h"

MVATree_VBFHbb::MVATree_VBFHbb(bool persistent, bool readMVA,
                               std::string analysisType, EL::IWorker* wk,
                               std::vector<std::string> variations,
                               bool nominalOnly, std::string MVAxmlFileName)
    : MVATree(persistent, readMVA, wk, variations, nominalOnly),
      m_analysisType(analysisType) {
  m_xmlFileName = MVAxmlFileName;
  SetBranches();
}

void MVATree_VBFHbb::AddBranch(TString name, Int_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_VBFHbb::AddBranch(TString name, Float_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_VBFHbb::AddBranch(TString name, ULong64_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void MVATree_VBFHbb::AddBranch(TString name, std::string* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
}

void MVATree_VBFHbb::SetBranches() {
  // prepare MVA reader
  m_reader.SetSplitVar(&EventNumber);
  m_reader.AddReader("reader", 2);

  AddBranch("passTrig", &passTrig);
  AddBranch("matchTrig", &matchTrig);
  AddBranch("passMVAPreSel", &passMVAPreSel);

  AddBranch("sample", &sample);
  AddBranch("MCChannelNumber", &MCChannelNumber);
  AddBranch("randomRunNumber", &randomRunNumber);
  AddBranch("EventWeight", &EventWeight);
  AddBranch("dEtaJJSF", &dEtaJJSF);
  AddBranch("mindRBPhSF", &mindRBPhSF);
  AddBranch("pTJJSF", &pTJJSF);
  AddBranch("pTBalSF", &pTBalSF);
  AddBranch("bjetTrigPt_SF", &bjetTrigPt_SF);
  AddBranch("bjetTrigEta_SF", &bjetTrigEta_SF);
  AddBranch("RunNumber", &RunNumber);
  AddBranch("EventNumber", &EventNumber);
  AddBranch("AverageMu", &AverageMu);
  AddBranch("ActualMu", &ActualMu);
  AddBranch("AverageMuScaled", &AverageMuScaled);
  AddBranch("ActualMuScaled", &ActualMuScaled);
  AddBranch("Nvtx", &Nvtx);
  AddBranch("ZPV", &ZPV);

  AddBranch("tagCatExcl", &tagCatExcl);
  AddBranch("tagCatExclDirect", &tagCatExclDirect);

  //  AddBranch("nJ", &nJ);
  AddBranch("nJets", &nJets);
  AddBranch("mJJ", &mJJ);
  AddBranch("pTJJ", &pTJJ);
  AddBranch("dEtaJJ", &dEtaJJ);
  AddBranch("dRJJ", &dRJJ);
  AddBranch("dPhiJJ", &dPhiJJ);
  AddBranch("maxEta", &maxEta);

  AddBranch("mBB", &mBB);
  AddBranch("mBBNoCorr", &mBBNoCorr);
  AddBranch("dRBB", &dRBB);
  AddBranch("dPhiBB", &dPhiBB);
  AddBranch("dEtaBB", &dEtaBB);
  AddBranch("pTBB", &pTBB);
  AddBranch("dPhiPhBmin", &dPhiPhBmin);
  AddBranch("mBBPh", &mBBPh);

  //  AddBranch("nJetGap", &nJetGap);
  //  AddBranch("HT_gap", &HT_gap);
  AddBranch("HT_soft", &HT_soft);
  //  AddBranch("HT_soft_SCALEUP", &HT_soft_SCALEUP);
  //  AddBranch("HT_soft_SCALEDOWN", &HT_soft_SCALEDOWN);
  //  AddBranch("HT_soft_RESOPARA", &HT_soft_RESOPARA);
  //  AddBranch("HT_soft_RESOPERP", &HT_soft_RESOPERP);
  AddBranch("pTBal", &pTBal);
  //
  //  AddBranch("nTrkJet2", &nTrkJet2);
  //  AddBranch("nTrkJet5", &nTrkJet5);
  //  AddBranch("nTrkJet10", &nTrkJet10);
  AddBranch("dEtaJJBB", &dEtaJJBB);
  AddBranch("dPhiBBJJ", &dPhiBBJJ);
  AddBranch("pTTot", &pTTot);
  AddBranch("pZTot", &pZTot);
  AddBranch("HT", &HT);
  AddBranch("pTJ5", &pTJ5);
  //  AddBranch("pTJ4", &pTJ4);
  AddBranch("etaJStar", &etaJStar);
  //  AddBranch("cosThetaA", &cosThetaA);
  AddBranch("cosThetaC", &cosThetaC);
  //
  //  AddBranch("cenHJJ", &cenHJJ);
  //  AddBranch("cenHgJJ", &cenHgJJ);
  AddBranch("cenPhJJ", &cenPhJJ);

  AddBranch("pTJ1", &pTJ1);
  AddBranch("pTJ2", &pTJ2);
  AddBranch("pTB1", &pTB1);
  AddBranch("pTB2", &pTB2);
  AddBranch("pTPh", &pTPh);

  AddBranch("etaJ1", &etaJ1);
  AddBranch("etaJ2", &etaJ2);
  AddBranch("etaB1", &etaB1);
  AddBranch("etaB2", &etaB2);
  AddBranch("etaPh", &etaPh);

  AddBranch("MV2c10B1", &MV2c10B1);
  AddBranch("MV2c10B2", &MV2c10B2);
  AddBranch("WidthJ1", &WidthJ1);
  AddBranch("WidthJ2", &WidthJ2);

  AddBranch("dRB1Ph", &dRB1Ph);
  AddBranch("dRB2Ph", &dRB2Ph);
  AddBranch("dRJ1Ph", &dRJ1Ph);
  AddBranch("dRJ2Ph", &dRJ2Ph);

  AddBranch("dRB1J1", &dRB1J1);
  AddBranch("dRB1J2", &dRB1J2);
  AddBranch("dRB2J1", &dRB2J1);
  AddBranch("dRB2J2", &dRB2J2);

  AddBranch("nTrkPt5J1", &nTrkPt5J1);
  AddBranch("nTrkPt5J2", &nTrkPt5J2);
  //  AddBranch("nTrkPt10J1", &nTrkPt10J1);
  //  AddBranch("nTrkPt10J2", &nTrkPt10J2);
  AddBranch("SumPtExtraJets", &SumPtExtraJets);
  AddBranch("mindRBPh", &mindRBPh);
  AddBranch("mindRJPh", &mindRJPh);
  //  AddBranch("dR_BPhovJPh", &dR_BPhovJPh);
  //  AddBranch("mindRJ1J", &mindRJ1J);
  //  AddBranch("mindRJ2J", &mindRJ2J);
  //  AddBranch("mindRB1J", &mindRB1J);
  //  AddBranch("mindRB2J", &mindRB2J);
  AddBranch("mindRJ1Ji", &mindRJ1Ji);
  AddBranch("mindRJ2Ji", &mindRJ2Ji);

  //  AddBranch("passVBF18", &passVBF18);
  //  AddBranch("passVBF20", &passVBF20);
  //  AddBranch("passVBF22", &passVBF22);
  //  AddBranch("passVBF0b", &passVBF0b);
  //  AddBranch("passVBF1b", &passVBF1b);
  //  AddBranch("passVBF1bs", &passVBF1bs);
  //  AddBranch("passVBF2b", &passVBF2b);
  //  AddBranch("passVBF2bs", &passVBF2bs);
  //  AddBranch("passVBF25g4j", &passVBF25g4j);
  //  AddBranch("passEM13VH", &passEM13VH);
  //  AddBranch("passEM15VH", &passEM15VH);
  //  AddBranch("passEM18VH", &passEM18VH);
  //  AddBranch("passEM20VH", &passEM20VH);
  //  AddBranch("passEM22VHI", &passEM22VHI);
  //  AddBranch("passg10", &passg10);
  //  AddBranch("passg15", &passg15);
  //  AddBranch("passg20", &passg20);
  //  AddBranch("passg20m", &passg20m);
  //  AddBranch("passg25", &passg25);
  //  AddBranch("passg35", &passg35);
  //  AddBranch("passg40", &passg40);
  //  AddBranch("passg45", &passg45);
  //  AddBranch("passg50", &passg50);
  //  AddBranch("passg60", &passg60);
  //  AddBranch("passg70", &passg70);
  //  AddBranch("passg80", &passg80);
  //  AddBranch("passg100", &passg100);
  //  AddBranch("passg120", &passg120);
  //
  //  AddBranch("l1_emet", &l1_emet);
  //  AddBranch("hlt_phpt", &hlt_phpt);
  //  AddBranch("hlt_j1pt", &hlt_j1pt);
  //  AddBranch("hlt_j2pt", &hlt_j2pt);
  //  AddBranch("hlt_j3pt", &hlt_j3pt);
  //  AddBranch("hlt_j4pt", &hlt_j4pt);
  //  AddBranch("hlt_j5pt", &hlt_j5pt);
  //  AddBranch("hlt_mjj", &hlt_mjj);
  //  AddBranch("hlt_phpt_matched", &hlt_phpt_matched);
  //  AddBranch("hlt_j4pt_phor", &hlt_j4pt_phor);
  //  AddBranch("hlt_mjj_phor", &hlt_mjj_phor);

  //  AddBranch("weight_forwardJetScale_up", &weight_forwardJetScale_up);
  //  AddBranch("weight_forwardJetScale_down", &weight_forwardJetScale_down);
  //  AddBranch("weight_pTBalScale_up", &weight_pTBalScale_up);
  //  AddBranch("weight_pTBalScale_down", &weight_pTBalScale_down);
  //  AddBranch("weight_pTBalPS_up", &weight_pTBalPS_up);
  //  AddBranch("weight_pTBalPS_down", &weight_pTBalPS_down);
  //  AddBranch("weight_TrigJetEff_up", &weight_TrigJetEff_up);
  //  AddBranch("weight_TrigJetEff_down", &weight_TrigJetEff_down);
  //  AddBranch("weight_TrigL1EMEffon", &weight_TrigL1EMEffon);

  AddBranch("BDT", &BDT);

  // AddBranch("MCWeight",        &MCWeight);
  // AddBranch("mBB_Regression", &mBB_Regression);
  // AddBranch("mBB_OneMu", &mBB_OneMu);
  // AddBranch("mBB_PtRecollbbOneMuPartonBukinNew",
  // &mBB_PtRecollbbOneMuPartonBukinNew);

  // book MVA reader
  if (m_xmlFileName == "")
    m_xmlFileName =
        "vbfgamma_BDT_0of2_ICHEP2016.weights.xml";  // can be modified by
                                                    // setting MVAxmlFileName in
                                                    // framework-read-vbfa.cfg
  TString xmlFile = getenv("WorkDir_DIR");
  if (m_analysisType == "vbfa" && m_readMVA) {
    xmlFile += "/data/CxAODReader_VHbb/";
    xmlFile += m_xmlFileName;
    m_reader.BookReader("reader", xmlFile);
  }
}

void MVATree_VBFHbb::TransformVars() {}

void MVATree_VBFHbb::Reset() {
  passTrig = 0;
  matchTrig = 0;

  sample = "Unknown";
  MCChannelNumber = -1;
  EventWeight = 0;
  RunNumber = -1;
  EventNumber = -1;
  AverageMu = -99;
  ActualMu = -99;
  AverageMuScaled = -99;
  ActualMuScaled = -99;
  Nvtx = -99;
  ZPV = -99;

  tagCatExcl = -1;
  tagCatExclDirect = -1;

  //  nJ              = -1;
  nJets = -1;
  mJJ = -1;
  pTJJ = -1;
  dEtaJJ = -1;
  dRJJ = -1;
  dPhiJJ = -1;

  mBB = -1;
  mBBNoCorr = -1;
  dRBB = -1;
  dPhiBB = -1;
  dEtaBB = -1;
  pTBB = -1;
  dPhiPhBmin = -1;
  mBBPh = -1;

  //  nJetGap         = -1;
  //  HT_gap          = -1;
  HT_soft = -1;
  pTBal = -1;
  //
  //  nTrkJet2        = -1;
  //  nTrkJet5        = -1;
  //  nTrkJet10       = -1;
  //  dEtaJJBB        = -1;
  pTTot = -1;
  pZTot = -1;
  HT = -1;
  pTJ5 = -1;

  //  cenHJJ          = -1;
  //  cenHgJJ         = -1;
  cenPhJJ = -1;

  pTJ1 = -1;
  pTJ2 = -1;
  pTB1 = -1;
  pTB2 = -1;
  pTPh = -1;

  etaJ1 = -1;
  etaJ2 = -1;
  etaB1 = -1;
  etaB2 = -1;
  etaPh = -1;

  MV2c10B1 = -1;
  MV2c10B2 = -1;
  WidthJ1 = -1;
  WidthJ2 = -1;

  dRB1Ph = -1;
  dRB2Ph = -1;
  dRJ1Ph = -1;
  dRJ2Ph = -1;

  dRB1J1 = -1;
  dRB1J2 = -1;
  dRB2J1 = -1;
  dRB2J2 = -1;

  //  weight_forwardJetScale_up = -1;
  //  weight_forwardJetScale_down = -1;
  //  weight_pTBalScale_up = -1;
  //  weight_pTBalScale_down = -1;
  //  weight_pTBalPS_up = -1;
  //  weight_pTBalPS_down = -1;
  //
  BDT = -1;

  // MCWeight        = -1;
}

void MVATree_VBFHbb::ReadMVA() {
  if (!m_readMVA) return;

  BDT = m_reader.EvaluateMVA("reader");
}
