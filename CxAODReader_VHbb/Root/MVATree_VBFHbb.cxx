#include "CxAODReader_VHbb/MVATree_VBFHbb.h"
#include "EventLoop/Worker.h"

MVATree_VBFHbb::MVATree_VBFHbb(bool persistent, bool readMVA, std::string analysisType, EL::Worker* wk, std::vector<std::string> variations, bool nominalOnly) 
 : MVATree(persistent, readMVA, wk, variations, nominalOnly),
   m_analysisType(analysisType) { SetBranches(); }

void MVATree_VBFHbb::AddBranch(TString name, Int_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) 
    tree.second -> Branch(name, address);
  m_reader.AddVariable(name, address);
}

void MVATree_VBFHbb::AddBranch(TString name, Float_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) 
    tree.second -> Branch(name, address);
  m_reader.AddVariable(name, address);
}

void MVATree_VBFHbb::AddBranch(TString name, ULong64_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) 
    tree.second -> Branch(name, address);
  m_reader.AddVariable(name, address);
}

void MVATree_VBFHbb::AddBranch(TString name, std::string* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) 
    tree.second -> Branch(name, address);
}

// Additional methods for storing std::vectors
void MVATree_VBFHbb::AddBranch(TString name, std::vector<int> *address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) 
    tree.second -> Branch(name, address);
}
void MVATree_VBFHbb::AddBranch(TString name, std::vector<float> *address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) 
    tree.second -> Branch(name, address);
}


void MVATree_VBFHbb::SetBranches() {
  // prepare MVA reader
  m_reader.SetSplitVar(&EventNumber);
  m_reader.AddReader("reader", 2);

  //  AddBranch("sample", &sample);

  AddBranch("weightSysts",     &weightSysts);
  AddBranch("weightspdf4lhc",    &weightspdf4lhc);
  AddBranch("weightspdfnnpdf30", &weightspdfnnpdf30);
  AddBranch("weightsqcdnnlops",  &weightsqcdnnlops);
  AddBranch("weightsqcdnnlops2np",  &weightsqcdnnlops2np);
  AddBranch("weightsqcdwg1",     &weightsqcdwg1);
  AddBranch("weightsalternativepdf",  &weightsalternativepdf);

  AddBranch("runNumber"  ,     &RunNumber);
  AddBranch("lumiBlock"  ,     &LumiBlock);
  AddBranch("eventNumber",     &EventNumber);
  AddBranch("npv",     &npv);
  AddBranch("mcWeight"   ,     &MCWeight);
  AddBranch("eventWeight",     &EventWeight);
  AddBranch("BJetTriggerWeight", &BJetTriggerWeight);
  AddBranch("BJetSF", &BJetSF);
  AddBranch("alpha_s_up", &alpha_s_up);
  AddBranch("alpha_s_dn", &alpha_s_dn);
  AddBranch("mcChannelNumber", &MCChannelNumber);
  AddBranch("nJets"      ,     &nJets);
  AddBranch("nJets20pt"  ,     &nJets20pt);
  AddBranch("nJets30pt"  ,     &nJets30pt);
  AddBranch("nJets40pt"  ,     &nJets40pt);
  AddBranch("nJets50pt"  ,     &nJets50pt);
  AddBranch("nJets60pt"  ,     &nJets60pt);

  AddBranch("nTightMv2c20",    &nTightMv2c20);
  AddBranch("nMediumMv2c20",   &nMediumMv2c20);
  AddBranch("nLooseMv2c20",    &nLooseMv2c20);

  AddBranch("nTightMv2c10",    &nTightMv2c10);
  AddBranch("nMediumMv2c10",   &nMediumMv2c10);
  AddBranch("nLooseMv2c10",    &nLooseMv2c10);

  AddBranch("pT"         ,     &pT);
  AddBranch("phi"        ,     &phi);
  AddBranch("eta"        ,     &eta);
  AddBranch("mv2c10"     ,     &mv2c10);
  AddBranch("mv2c20"     ,     &mv2c20);
  AddBranch("jetWidth"   ,     &jetWidth);

  AddBranch("truthLabel" ,     &truthLabelID);
  AddBranch("coneTruthLabelID",&coneTruthLabelID);
  AddBranch("hadronConeExclTruthLabelID",&hadronConeExclTruthLabelID);
  AddBranch("partonTruthLabelID" ,     &partonTruthLabelID);



  AddBranch("whoIsJ1"    ,     &whoIsJ1);
  AddBranch("whoIsJ2"    ,     &whoIsJ2);
  AddBranch("whoIsB1"    ,     &whoIsB1);
  AddBranch("whoIsB2"    ,     &whoIsB2);

  AddBranch("mBB"        ,     &mBB);
  AddBranch("mBB_no_corr",     &mBB_no_corr);
  AddBranch("dRBB"       ,     &dRBB);
  AddBranch("dRBB_no_corr",    &dRBB_no_corr);
  AddBranch("dPhiBB"     ,     &dPhiBB);
  AddBranch("dEtaBB"     ,     &dEtaBB);
  AddBranch("pTBB"       ,     &pTBB);
  AddBranch("pTBB_no_corr",    &pTBB_no_corr);

  AddBranch("mJJ"        ,     &mJJ);
  AddBranch("mJ1B1"      ,     &mJ1B1);
  AddBranch("mJ1B2"      ,     &mJ1B2);
  AddBranch("dRJJ"       ,     &dRJJ);
  AddBranch("dPhiJJ"     ,     &dPhiJJ);
  AddBranch("dEtaJJ"     ,     &dEtaJJ);
  AddBranch("pTJJ"       ,     &pTJJ);

  AddBranch("minEta"    ,      &minEta);
  AddBranch("maxEta"    ,      &maxEta);

  // ====                                                                                                                                                               

  // AddBranch("passCURRENT",     &passCURRENT);
  // AddBranch("passATLAS",     &passATLAS);
  // AddBranch("passCMS",     &passCMS);
  // AddBranch("passHYBRID",     &passHYBRID);

  // AddBranch("whoIsB1_CURRENT" , &whoIsB1_CURRENT);
  // AddBranch("whoIsB2_CURRENT" , &whoIsB2_CURRENT);
  // AddBranch("whoIsB1_ATLAS", &whoIsB1_ATLAS);
  // AddBranch("whoIsB2_ATLAS", &whoIsB2_ATLAS);
  // AddBranch("whoIsB1_CMS", &whoIsB1_CMS );
  // AddBranch("whoIsB2_CMS", &whoIsB2_CMS );
  // AddBranch("whoIsB1_HYBRID", &whoIsB1_HYBRID );
  // AddBranch("whoIsB2_HYBRID", &whoIsB2_HYBRID );

  // =====   

  AddBranch("pTJ1",&pTJ1);
  AddBranch("pTJ2",&pTJ2);
  AddBranch("pTB1",&pTB1);
  AddBranch("pTB2",&pTB2);
  AddBranch("pTB1_no_corr",&pTB1_no_corr);
  AddBranch("pTB2_no_corr",&pTB2_no_corr);

  AddBranch("etaJ1",&etaJ1);
  AddBranch("etaJ2",&etaJ2);
  AddBranch("etaB1",&etaB1);
  AddBranch("etaB2",&etaB2);
  AddBranch("etaB1_no_corr",&etaB1_no_corr);
  AddBranch("etaB2_no_corr",&etaB2_no_corr);

  AddBranch("J1FJVTLoose",&J1FJVTLoose);
  AddBranch("J2FJVTLoose",&J2FJVTLoose);
  AddBranch("J1FJVTTight",&J1FJVTTight);
  AddBranch("J2FJVTTight",&J2FJVTTight);

  AddBranch("dRB1J1",&dRB1J1);
  AddBranch("dRB1J2",&dRB1J2);
  AddBranch("dRB2J1",&dRB2J1);
  AddBranch("dRB2J2",&dRB2J2);

  AddBranch("mindRJ1",&mindRJ1);
  AddBranch("mindRJ2",&mindRJ2);
  AddBranch("mindRJ1_Ex",&mindRJ1_Ex);
  AddBranch("mindRJ2_Ex",&mindRJ2_Ex);
  AddBranch("mindRB1",&mindRB1);
  AddBranch("mindRB2",&mindRB2);

  AddBranch("TruthLabelB1",&TruthLabelB1);
  AddBranch("TruthLabelB2",&TruthLabelB2);
  AddBranch("TruthLabelPartonB1",&TruthLabelPartonB1);
  AddBranch("TruthLabelPartonB2",&TruthLabelPartonB2);
  AddBranch("HadronConeExclTruthLabelB1", &HadronConeExclTruthLabelB1);
  AddBranch("HadronConeExclTruthLabelB2", &HadronConeExclTruthLabelB2);

  AddBranch("MV2c20B1",&MV2c20B1);
  AddBranch("MV2c20B2",&MV2c20B2);
  AddBranch("MV2c20J1",&MV2c20J1);
  AddBranch("MV2c20J2",&MV2c20J2);

  AddBranch("MV2c10B1",&MV2c10B1);
  AddBranch("MV2c10B2",&MV2c10B2);
  AddBranch("MV2c10J1",&MV2c10J1);
  AddBranch("MV2c10J2",&MV2c10J2);

  AddBranch("WidthJ1",&WidthJ1);
  AddBranch("WidthJ2",&WidthJ2);

  AddBranch("NTrk1000PVJ1",&NTrk1000PVJ1);
  AddBranch("NTrk1000PVJ2",&NTrk1000PVJ2);

  AddBranch("NTrk500PVJ1",&NTrk500PVJ1);
  AddBranch("NTrk500PVJ2",&NTrk500PVJ2);

  AddBranch("QGTaggerJ1",&QGTaggerJ1);
  AddBranch("QGTaggerJ2",&QGTaggerJ2);
  AddBranch("QGTaggerWeightJ1", &QGTaggerWeightJ1);
  AddBranch("QGTaggerWeightJ2", &QGTaggerWeightJ2);


  AddBranch("pT_ballance" ,    &pT_ballance );

  // ======                                                                    

  AddBranch("max_J1J2"   ,     &max_J1J2);
  AddBranch("eta_J_star" ,     &eta_J_star);
  AddBranch("cosTheta_MVA",    &cosTheta_MVA);
  AddBranch("cosTheta",    &cosTheta_CMS);
  AddBranch("cosTheta_boost",    &cosTheta_boost);
  AddBranch("HT_soft"    ,     &HT_soft);
  AddBranch("HT_MVA"    ,     &HT_MVA);
  //  AddBranch("BDT"        ,     &BDT);




  // ====================================================================
  // store trigger decisions
  AddBranch("pass_L1_2J25" , &pass_L1_2J25);
  AddBranch("pass_L1_J25_0ETA23" , &pass_L1_J25_0ETA23);
  AddBranch("pass_L1_HT150_JJ15_ETA49" , &pass_L1_HT150_JJ15_ETA49);
  AddBranch("pass_L1_2J15_31ETA49" , &pass_L1_2J15_31ETA49);
  AddBranch("pass_L1_HT150_J20s5_ETA31" , &pass_L1_HT150_J20s5_ETA31);
  AddBranch("pass_L1_MJJ_400" , &pass_L1_MJJ_400);
  AddBranch("pass_L1_J40_0ETA25" , &pass_L1_J40_0ETA25);
  AddBranch("pass_L1_MJJ_400_CF" , &pass_L1_MJJ_400_CF);
  AddBranch("pass_L1_J20_31ETA49" , &pass_L1_J20_31ETA49);
  AddBranch("pass_L1_J40_0ETA25_2J25_J20_31ETA49" , &pass_L1_J40_0ETA25_2J25_J20_31ETA49);
  AddBranch("pass_L1_J25_0ETA23_2J15_31ETA49" , &pass_L1_J25_0ETA23_2J15_31ETA49);
  AddBranch("pass_L1_HT150_J20s5_ETA31_MJJ_400_CF" , &pass_L1_HT150_J20s5_ETA31_MJJ_400_CF);
  AddBranch("pass_L1_J40_0ETA25_2J15_31ETA49" , &pass_L1_J40_0ETA25_2J15_31ETA49);
  AddBranch("pass_L1_HT150_JJ15_ETA49_MJJ_400" , &pass_L1_HT150_JJ15_ETA49_MJJ_400);
  AddBranch("pass_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490" , &pass_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490);
  AddBranch("pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49" , &pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49);
  AddBranch("pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split" , &pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split);
  AddBranch("pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split" , &pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split);
  AddBranch("pass_HLT_j80_0eta240_j60_j45_320eta490" , &pass_HLT_j80_0eta240_j60_j45_320eta490);
  AddBranch("pass_HLT_j80_0eta240_2j60_320eta490" , &pass_HLT_j80_0eta240_2j60_320eta490);
  AddBranch("pass_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49" , &pass_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49);

  // ====================================================================
  // If trigger is active
  AddBranch("isActive_L1_2J25" , &isActive_L1_2J25);
  AddBranch("isActive_L1_J25_0ETA23" , &isActive_L1_J25_0ETA23);
  AddBranch("isActive_L1_HT150_JJ15_ETA49" , &isActive_L1_HT150_JJ15_ETA49);
  AddBranch("isActive_L1_2J15_31ETA49" , &isActive_L1_2J15_31ETA49);
  AddBranch("isActive_L1_HT150_J20s5_ETA31" , &isActive_L1_HT150_J20s5_ETA31);
  AddBranch("isActive_L1_MJJ_400" , &isActive_L1_MJJ_400);
  AddBranch("isActive_L1_J40_0ETA25" , &isActive_L1_J40_0ETA25);
  AddBranch("isActive_L1_MJJ_400_CF" , &isActive_L1_MJJ_400_CF);
  AddBranch("isActive_L1_J20_31ETA49" , &isActive_L1_J20_31ETA49);
  AddBranch("isActive_L1_J40_0ETA25_2J25_J20_31ETA49" , &isActive_L1_J40_0ETA25_2J25_J20_31ETA49);
  AddBranch("isActive_L1_J25_0ETA23_2J15_31ETA49" , &isActive_L1_J25_0ETA23_2J15_31ETA49);
  AddBranch("isActive_L1_HT150_J20s5_ETA31_MJJ_400_CF" , &isActive_L1_HT150_J20s5_ETA31_MJJ_400_CF);
  AddBranch("isActive_L1_J40_0ETA25_2J15_31ETA49" , &isActive_L1_J40_0ETA25_2J15_31ETA49);
  AddBranch("isActive_L1_HT150_JJ15_ETA49_MJJ_400" , &isActive_L1_HT150_JJ15_ETA49_MJJ_400);
  AddBranch("isActive_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490" , &isActive_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490);
  AddBranch("isActive_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49" , &isActive_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49);
  AddBranch("isActive_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split" , &isActive_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split);
  AddBranch("isActive_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split" , &isActive_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split);
  AddBranch("isActive_HLT_j80_0eta240_j60_j45_320eta490" , &isActive_HLT_j80_0eta240_j60_j45_320eta490);
  AddBranch("isActive_HLT_j80_0eta240_2j60_320eta490" , &isActive_HLT_j80_0eta240_2j60_320eta490);
  AddBranch("isActive_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49" , &isActive_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49);

  // ====================================================================
  // Trigger Pre-Scales
  AddBranch("prescale_L1_2J25" , &prescale_L1_2J25);
  AddBranch("prescale_L1_J25_0ETA23" , &prescale_L1_J25_0ETA23);
  AddBranch("prescale_L1_HT150_JJ15_ETA49" , &prescale_L1_HT150_JJ15_ETA49);
  AddBranch("prescale_L1_2J15_31ETA49" , &prescale_L1_2J15_31ETA49);
  AddBranch("prescale_L1_HT150_J20s5_ETA31" , &prescale_L1_HT150_J20s5_ETA31);
  AddBranch("prescale_L1_MJJ_400" , &prescale_L1_MJJ_400);
  AddBranch("prescale_L1_J40_0ETA25" , &prescale_L1_J40_0ETA25);
  AddBranch("prescale_L1_MJJ_400_CF" , &prescale_L1_MJJ_400_CF);
  AddBranch("prescale_L1_J20_31ETA49" , &prescale_L1_J20_31ETA49);
  AddBranch("prescale_L1_J40_0ETA25_2J25_J20_31ETA49" , &prescale_L1_J40_0ETA25_2J25_J20_31ETA49);
  AddBranch("prescale_L1_J25_0ETA23_2J15_31ETA49" , &prescale_L1_J25_0ETA23_2J15_31ETA49);
  AddBranch("prescale_L1_HT150_J20s5_ETA31_MJJ_400_CF" , &prescale_L1_HT150_J20s5_ETA31_MJJ_400_CF);
  AddBranch("prescale_L1_J40_0ETA25_2J15_31ETA49" , &prescale_L1_J40_0ETA25_2J15_31ETA49);
  AddBranch("prescale_L1_HT150_JJ15_ETA49_MJJ_400" , &prescale_L1_HT150_JJ15_ETA49_MJJ_400);
  AddBranch("prescale_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490" , &prescale_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490);
  AddBranch("prescale_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49" , &prescale_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49);
  AddBranch("prescale_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split" , &prescale_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split);
  AddBranch("prescale_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split" , &prescale_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split);
  AddBranch("prescale_HLT_j80_0eta240_j60_j45_320eta490" , &prescale_HLT_j80_0eta240_j60_j45_320eta490);
  AddBranch("prescale_HLT_j80_0eta240_2j60_320eta490" , &prescale_HLT_j80_0eta240_2j60_320eta490);
  AddBranch("prescale_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49" , &prescale_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49);

  // ====================================================================

  // book MVA reader
  TString xmlFile = getenv("WorkDir_DIR");

  if (m_analysisType == "vbf" && m_readMVA) {
	 xmlFile += "/data/CxAODReader_VBFHbb/vbfgamma_BDT_0of2_weights.xml";
	 m_reader.BookReader("reader", xmlFile);
  }
	
}

void MVATree_VBFHbb::TransformVars() {}


void MVATree_VBFHbb::Reset() {
  //  sample = "Unknown";

  RunNumber       = 0;
  LumiBlock       = 0;
  EventNumber     = 0;
  npv             = 0;
  MCWeight        = 0;
  EventWeight     = 0;
  BJetTriggerWeight =0;
  BJetSF =0;
  MCChannelNumber = 0;
  nJets           = 0;
  nJets20pt       = 0;
  nJets30pt       = 0;
  nJets40pt       = 0;
  nJets50pt       = 0;
  nJets60pt       = 0;

  nTightMv2c20    = 0;
  nMediumMv2c10   = 0;
  nLooseMv2c10    = 0;

  nTightMv2c10    = 0;
  nMediumMv2c10   = 0;
  nLooseMv2c10    = 0;

  pT.clear();
  phi.clear();
  eta.clear();
  mv2c10.clear();
  mv2c20.clear();
  jetWidth.clear();
  truthLabelID.clear();
  coneTruthLabelID.clear();
  hadronConeExclTruthLabelID.clear();
  partonTruthLabelID.clear();
  weightSysts.clear();

  weightspdf4lhc.clear();
  weightspdfnnpdf30.clear();
  weightsqcdnnlops.clear();
  weightsqcdnnlops2np.clear();
  weightsqcdwg1.clear();
  weightsalternativepdf.clear();


  alpha_s_up      = 0;
  alpha_s_dn      = 0;
  whoIsJ1         = 0;
  whoIsJ2         = 0;
  whoIsB1         = 0;
  whoIsB2         = 0;

  mBB              = 0;
  mBB_no_corr      = 0;
  dRBB             = 0;
  dRBB_no_corr     = 0;
  dPhiBB           = 0;
  dEtaBB           = 0;
  pTBB             = 0;
  pTBB_no_corr      = 0;

  mJJ              = 0;
  dRJJ             = 0;
  dPhiJJ           = 0;
  dEtaJJ           = 0;
  pTJJ             = 0;

  minEta           = 0;
  maxEta           = 0;

  // ======                                                                                                                                                              
  // passCURRENT = 0;
  // passATLAS = 0;
  // passCMS = 0;
  // passHYBRID = 0;

  // whoIsB1_CURRENT = 0;
  // whoIsB2_CURRENT = 0;
  // whoIsB1_ATLAS = 0;
  // whoIsB2_ATLAS = 0;
  // whoIsB1_CMS = 0;
  // whoIsB2_CMS = 0;
  // whoIsB1_HYBRID = 0;
  // whoIsB2_HYBRID = 0;

  // ======  

  pTJ1 = 0;
  pTJ2 = 0;
  pTB1 = 0;
  pTB2 = 0;
  pTB1_no_corr = 0;
  pTB2_no_corr = 0;

  J1FJVTLoose = 0;
  J2FJVTLoose = 0;
  J1FJVTTight = 0;
  J2FJVTTight = 0;

  etaJ1 = 0;
  etaJ2 = 0;
  etaB1 = 0;
  etaB2 = 0;
  etaB1_no_corr = 0;
  etaB2_no_corr = 0;

  dRB1J1 = 0;
  dRB1J2 = 0;
  dRB2J1 = 0;
  dRB2J2 = 0;

  mindRJ1 = 0;
  mindRJ2 = 0;
  mindRJ1_Ex = 0;
  mindRJ2_Ex = 0;
  mindRB1 = 0;
  mindRB2 = 0;

  TruthLabelB1 = 0;
  TruthLabelB2 = 0;
  TruthLabelPartonB1 = 0;
  TruthLabelPartonB2 = 0;
  HadronConeExclTruthLabelB1 =0;
  HadronConeExclTruthLabelB2 =0;

  MV2c20B1 = 0;
  MV2c20B2 = 0;
  MV2c20J1 = 0;
  MV2c20J2 = 0;

  MV2c10B1 = 0;
  MV2c10B2 = 0;
  MV2c10J1 = 0;
  MV2c10J2 = 0;

  WidthJ1 = 0;
  WidthJ2 = 0;

  NTrk1000PVJ1 =0;
  NTrk1000PVJ2 =0;
  NTrk500PVJ1 =0;
  NTrk500PVJ2 =0;

  pT_ballance = 0;

  // ======                                                                    

  max_J1J2         = 0;
  eta_J_star       = 0;
  HT_soft          = 0;
  HT_MVA           = 0;
  cosTheta_MVA     = 0;
  cosTheta_CMS     = 0;
  cosTheta_boost     = 0;
  //  BDT              = -2;

  // Triggers
  pass_L1_2J25 = -1;
  pass_L1_J25_0ETA23 = -1;
  pass_L1_HT150_JJ15_ETA49 = -1;
  pass_L1_2J15_31ETA49 = -1;
  pass_L1_HT150_J20s5_ETA31 = -1;
  pass_L1_MJJ_400 = -1;
  pass_L1_J40_0ETA25 = -1;
  pass_L1_MJJ_400_CF = -1;
  pass_L1_J20_31ETA49 = -1;
  pass_L1_J40_0ETA25_2J25_J20_31ETA49 = -1;
  pass_L1_J25_0ETA23_2J15_31ETA49 = -1;
  pass_L1_HT150_J20s5_ETA31_MJJ_400_CF = -1;
  pass_L1_J40_0ETA25_2J15_31ETA49 = -1;
  pass_L1_HT150_JJ15_ETA49_MJJ_400 = -1;
  pass_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490 = -1;
  pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = -1;
  pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split = -1;
  pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split = -1;
  pass_HLT_j80_0eta240_j60_j45_320eta490 = -1;
  pass_HLT_j80_0eta240_2j60_320eta490 = -1;
  pass_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = -1;

  isActive_L1_2J25 = -1;
  isActive_L1_J25_0ETA23 = -1;
  isActive_L1_HT150_JJ15_ETA49 = -1;
  isActive_L1_2J15_31ETA49 = -1;
  isActive_L1_HT150_J20s5_ETA31 = -1;
  isActive_L1_MJJ_400 = -1;
  isActive_L1_J40_0ETA25 = -1;
  isActive_L1_MJJ_400_CF = -1;
  isActive_L1_J20_31ETA49 = -1;
  isActive_L1_J40_0ETA25_2J25_J20_31ETA49 = -1;
  isActive_L1_J25_0ETA23_2J15_31ETA49 = -1;
  isActive_L1_HT150_J20s5_ETA31_MJJ_400_CF = -1;
  isActive_L1_J40_0ETA25_2J15_31ETA49 = -1;
  isActive_L1_HT150_JJ15_ETA49_MJJ_400 = -1;
  isActive_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490 = -1;
  isActive_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = -1;
  isActive_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split = -1;
  isActive_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split = -1;
  isActive_HLT_j80_0eta240_j60_j45_320eta490 = -1;
  isActive_HLT_j80_0eta240_2j60_320eta490 = -1;
  isActive_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = -1;

  prescale_L1_2J25 = -1;
  prescale_L1_J25_0ETA23 = -1;
  prescale_L1_HT150_JJ15_ETA49 = -1;
  prescale_L1_2J15_31ETA49 = -1;
  prescale_L1_HT150_J20s5_ETA31 = -1;
  prescale_L1_MJJ_400 = -1;
  prescale_L1_J40_0ETA25 = -1;
  prescale_L1_MJJ_400_CF = -1;
  prescale_L1_J20_31ETA49 = -1;
  prescale_L1_J40_0ETA25_2J25_J20_31ETA49 = -1;
  prescale_L1_J25_0ETA23_2J15_31ETA49 = -1;
  prescale_L1_HT150_J20s5_ETA31_MJJ_400_CF = -1;
  prescale_L1_J40_0ETA25_2J15_31ETA49 = -1;
  prescale_L1_HT150_JJ15_ETA49_MJJ_400 = -1;
  prescale_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490 = -1;
  prescale_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = -1;
  prescale_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split = -1;
  prescale_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split = -1;
  prescale_HLT_j80_0eta240_j60_j45_320eta490 = -1;
  prescale_HLT_j80_0eta240_2j60_320eta490 = -1;
  prescale_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49 = -1;
}

void MVATree_VBFHbb::ReadMVA() {
  if (!m_readMVA) return;
  //  BDT = m_reader.EvaluateMVA("reader");
}


