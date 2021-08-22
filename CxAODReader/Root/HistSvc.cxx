#include <TH1D.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TTree.h>

#include <iostream>
#include <stdexcept>

#include "CxAODReader/HistSvc.h"
#include "EventLoop/IWorker.h"

HistSvc::HistSvc()
    : m_bootstraphists(),
      m_fillHists(true),
      m_fillAllSysts(true),
      m_nameSvc(nullptr),
      m_bootstrap(new BootstrapGenerator("gen", "gen", 1000)),
      m_weightSysts(nullptr),
      m_weightSystsCS(nullptr) {
  TH1::AddDirectory(false);
  gROOT->SetBatch(true);

  m_trees = new unordered_map<string, TTree*>;
}

HistSvc::~HistSvc() {
  //  if (m_hists) {delete m_hists; m_hists = 0;}
}

void HistSvc::Reset() {
  // Clear all hists from store, freeing up memory
  for (auto entry : m_hists) {
    if (entry.second) delete entry.second;
  }
  m_hists.clear();

  for (auto& entry : m_bootstraphists) {
    if (entry.second) delete entry.second;
  }
  m_bootstraphists.clear();

  if (m_trees) {
    treestore_t::const_iterator treeItr(m_trees->begin());
    treestore_t::const_iterator treeEnd(m_trees->end());

    for (; treeItr != treeEnd; ++treeItr) {
      if (treeItr->second) delete treeItr->second;
    }

    m_trees->clear();
    delete m_trees;
    m_trees = 0;
  }

  m_trees = new unordered_map<string, TTree*>;
}

// Bootstrap functions
void HistSvc::GenerateBootstrapWeights(float weight, UInt_t RunNumber,
                                       UInt_t EventNumber,
                                       UInt_t mc_channel_number) {
  m_bootstrap->Generate(weight, RunNumber, EventNumber, mc_channel_number);
}

TH1DBootstrap* HistSvc::FindBootstrapHistInMap(const string& fullname) {
  TH1DBootstrap* hist = nullptr;
  try {
    hist = m_bootstraphists.at(fullname);
  } catch (const std::out_of_range& oor) {
    return nullptr;
  }
  return hist;
}

TH1DBootstrap* HistSvc::BookBootstrapHist(const string& name, int nbinsx,
                                          float xlow, float xup) {
  string fullname = name;
  if (m_nameSvc) {
    // so much hack to make it work !
    int useEvtFlav = m_nameSvc->get_useEventFlav();
    m_nameSvc->set_useEventFlav(false);
    std::string samplename = m_nameSvc->get_sample();
    m_nameSvc->set_sample("data");

    fullname = m_nameSvc->getFullHistName(name);

    // hack again !
    m_nameSvc->set_useEventFlav(useEvtFlav);
    m_nameSvc->set_sample(samplename);
  }

  TH1DBootstrap* hist = FindBootstrapHistInMap(fullname);
  if (hist == 0) {
    string hname = histName(fullname);
    hist = new TH1DBootstrap(hname.c_str(), hname.c_str(), nbinsx, xlow, xup,
                             1000, m_bootstrap);
    m_bootstraphists[fullname] = hist;
  }
  return hist;
}

void HistSvc::BookFillBootstrapHist(const string& name, int nbinsx, float xlow,
                                    float xup, float value, float weight) {
  if (!m_fillHists) return;
  TH1DBootstrap* hist = BookBootstrapHist(name, nbinsx, xlow, xup);
  if (fabs(weight) > 1.e-9) {
    hist->Fill(value, weight / fabs(weight));
  }
}

TH1* HistSvc::FindHistInMap(const string& fullname) {
  TH1* hist = nullptr;
  try {
    hist = m_hists.at(fullname);
  } catch (const std::out_of_range& oor) {
    return nullptr;
  }
  return hist;
}

TH1* HistSvc::BookHist(const string& name, int nbinsx, float xlow, float xup) {
  string fullname = name;
  if (m_nameSvc) {
    fullname = m_nameSvc->getFullHistName(name);
  }

  TH1* hist = FindHistInMap(fullname);
  if (hist == 0) {
    string hname = histName(fullname);
    hist = new TH1F(hname.c_str(), hname.c_str(), nbinsx, xlow, xup);
    hist->Sumw2();
    m_hists[fullname] = hist;
  }
  return hist;
}

void HistSvc::BookFillHist(const string& name, int nbinsx, double* xbins,
                           int nbinsy, double ylow, double yup, float xvalue,
                           float yvalue, float weight, doSysts ds) {
  if (!m_fillHists) return;
  string fullname = m_nameSvc ? m_nameSvc->getFullHistName(name) : name;
  if (m_fillAllSysts || m_nameSvc->get_isNominal() ||
      IsHistForSystematics(fullname))
    static_cast<TH2F*>(BookHist(name, nbinsx, xbins, nbinsy, ylow, yup))
        ->Fill(xvalue, yvalue, weight);

  // additionally fill weight systs in case of Nominal
  if (!m_nameSvc) return;
  if (!m_nameSvc->get_isNominal()) return;
  if (!m_weightSysts) return;
  if (ds == doSysts::no || (!m_fillAllSysts && !IsHistForSystematics(fullname)))
    return;
  for (unsigned int i = 0; i < m_weightSysts->size(); i++) {
    float weightSys = weight * m_weightSysts->at(i).factor;
    m_nameSvc->set_variation(m_weightSysts->at(i).name);
    static_cast<TH2F*>(BookHist(name, nbinsx, xbins, nbinsy, ylow, yup))
        ->Fill(xvalue, yvalue, weightSys);
  }
  for (unsigned int i = 0; i < m_weightSystsCS->size(); i++) {
    float weightSys = weight * m_weightSystsCS->at(i).factor;
    m_nameSvc->set_variation(m_weightSystsCS->at(i).name);
    static_cast<TH2F*>(BookHist(name, nbinsx, xbins, nbinsy, ylow, yup))
        ->Fill(xvalue, yvalue, weightSys);
  }
  m_nameSvc->set_isNominal();
}

TH1* HistSvc::BookHist(const string& name, int nbinsx, double* xbins,
                       int nbinsy, double ylow, double yup) {
  string fullname = name;
  if (m_nameSvc) {
    fullname = m_nameSvc->getFullHistName(name);
  }

  TH1* hist = FindHistInMap(fullname);
  if (hist == 0) {
    string hname = histName(fullname);
    hist = new TH2F(hname.c_str(), hname.c_str(), nbinsx, xbins, nbinsy, ylow,
                    yup);
    hist->Sumw2();
    m_hists[fullname] = hist;
  }

  return hist;
}

void HistSvc::BookFillHist(const string& name, int nbinsx, float xlow,
                           float xup, float value, float weight, doSysts ds) {
  if (!m_fillHists) return;
  //Fill the combined Electron+Muon histogram
  string fullname = m_nameSvc ? m_nameSvc->getFullHistName(name) : name;
  if (m_fillAllSysts || m_nameSvc->get_isNominal() ||
      IsHistForSystematics(fullname))
    BookHist(name, nbinsx, xlow, xup)->Fill(value, weight);

  // additionally fill weight systs in case of Nominal
  if (!m_nameSvc) return;
  if (!m_nameSvc->get_isNominal()) return;
  if (!m_weightSysts) return;
  if (ds == doSysts::no || (!m_fillAllSysts && !IsHistForSystematics(fullname)))
    return;
  for (unsigned int i = 0; i < m_weightSysts->size(); i++) {
    float weightSys = weight * m_weightSysts->at(i).factor;
    m_nameSvc->set_variation(m_weightSysts->at(i).name);
    BookHist(name, nbinsx, xlow, xup)->Fill(value, weightSys);
  }
  for (unsigned int i = 0; i < m_weightSystsCS->size(); i++) {
    float weightSys = weight * m_weightSystsCS->at(i).factor;
    m_nameSvc->set_variation(m_weightSystsCS->at(i).name);
    BookHist(name, nbinsx, xlow, xup)->Fill(value, weightSys);
  }
  m_nameSvc->set_isNominal();
}

TH1* HistSvc::BookHist(const string& name, int nbinsx, float* xbins) {
  string fullname = name;
  if (m_nameSvc) {
    fullname = m_nameSvc->getFullHistName(name);
  }

  TH1* hist = FindHistInMap(fullname);
  if (hist == 0) {
    string hname = histName(fullname);
    hist = new TH1F(hname.c_str(), hname.c_str(), nbinsx, xbins);
    hist->Sumw2();
    m_hists[fullname] = hist;
  }
  return hist;
}

void HistSvc::BookFillHist(const string& name, int nbinsx, float* xbins,
                           float value, float weight, doSysts ds) {
  if (!m_fillHists) return;
  string fullname = m_nameSvc ? m_nameSvc->getFullHistName(name) : name;
  if (m_fillAllSysts || m_nameSvc->get_isNominal() ||
      IsHistForSystematics(fullname))
    BookHist(name, nbinsx, xbins)->Fill(value, weight);

  // additionally fill weight systs in case of Nominal
  if (!m_nameSvc) return;
  if (!m_nameSvc->get_isNominal()) return;
  if (!m_weightSysts) return;
  if (ds == doSysts::no || (!m_fillAllSysts && !IsHistForSystematics(fullname)))
    return;
  for (unsigned int i = 0; i < m_weightSysts->size(); i++) {
    float weightSys = weight * m_weightSysts->at(i).factor;
    m_nameSvc->set_variation(m_weightSysts->at(i).name);
    BookHist(name, nbinsx, xbins)->Fill(value, weightSys);
  }
  for (unsigned int i = 0; i < m_weightSystsCS->size(); i++) {
    float weightSys = weight * m_weightSystsCS->at(i).factor;
    m_nameSvc->set_variation(m_weightSystsCS->at(i).name);
    BookHist(name, nbinsx, xbins)->Fill(value, weightSys);
  }
  m_nameSvc->set_isNominal();
}

TH1* HistSvc::BookHist(const string& name, int nbinsx, float xlow, float xup,
                       int nbinsy, float ylow, float yup) {
  string fullname = name;
  if (m_nameSvc) {
    fullname = m_nameSvc->getFullHistName(name);
  }

  TH1* hist = FindHistInMap(fullname);
  if (hist == 0) {
    string hname = histName(fullname);
    hist = new TH2F(hname.c_str(), hname.c_str(), nbinsx, xlow, xup, nbinsy,
                    ylow, yup);
    hist->Sumw2();
    m_hists[fullname] = hist;
  }

  return hist;
}

void HistSvc::BookFillHist(const string& name, int nbinsx, float xlow,
                           float xup, int nbinsy, float ylow, float yup,
                           float xvalue, float yvalue, float weight,
                           doSysts ds) {
  if (!m_fillHists) return;
  //Apply combined Electron+Muon histogram
  string fullname = m_nameSvc ? m_nameSvc->getFullHistName(name) : name;
  if (m_fillAllSysts || m_nameSvc->get_isNominal() ||
      IsHistForSystematics(fullname))
    static_cast<TH2F*>(BookHist(name, nbinsx, xlow, xup, nbinsy, ylow, yup))
        ->Fill(xvalue, yvalue, weight);

  // additionally fill weight systs in case of Nominal
  if (!m_nameSvc) return;
  if (!m_nameSvc->get_isNominal()) return;
  if (!m_weightSysts) return;
  if (ds == doSysts::no || (!m_fillAllSysts && !IsHistForSystematics(fullname)))
    return;
  for (unsigned int i = 0; i < m_weightSysts->size(); i++) {
    float weightSys = weight * m_weightSysts->at(i).factor;
    m_nameSvc->set_variation(m_weightSysts->at(i).name);
    static_cast<TH2F*>(BookHist(name, nbinsx, xlow, xup, nbinsy, ylow, yup))
        ->Fill(xvalue, yvalue, weightSys);
  }
  for (unsigned int i = 0; i < m_weightSystsCS->size(); i++) {
    float weightSys = weight * m_weightSystsCS->at(i).factor;
    m_nameSvc->set_variation(m_weightSystsCS->at(i).name);
    static_cast<TH2F*>(BookHist(name, nbinsx, xlow, xup, nbinsy, ylow, yup))
        ->Fill(xvalue, yvalue, weightSys);
  }
  m_nameSvc->set_isNominal();
}

TH1* HistSvc::BookHist(const string& name, int nbinsx, float* xbins, int nbinsy,
                       float* ybins) {
  string fullname = name;
  if (m_nameSvc) {
    fullname = m_nameSvc->getFullHistName(name);
  }

  TH1* hist = FindHistInMap(fullname);
  if (hist == 0) {
    string hname = histName(fullname);
    hist = new TH2F(hname.c_str(), hname.c_str(), nbinsx, xbins, nbinsy, ybins);
    hist->Sumw2();
    m_hists[fullname] = hist;
  }

  return hist;
}

void HistSvc::BookFillHist(const string& name, int nbinsx, float* xbins,
                           int nbinsy, float* ybins, float xvalue, float yvalue,
                           float weight, doSysts ds) {
  if (!m_fillHists) return;
  string fullname = m_nameSvc ? m_nameSvc->getFullHistName(name) : name;
  if (m_fillAllSysts || m_nameSvc->get_isNominal() ||
      IsHistForSystematics(fullname))
    static_cast<TH2F*>(BookHist(name, nbinsx, xbins, nbinsy, ybins))
        ->Fill(xvalue, yvalue, weight);

  // additionally fill weight systs in case of Nominal
  if (!m_nameSvc) return;
  if (!m_nameSvc->get_isNominal()) return;
  if (!m_weightSysts) return;
  if (ds == doSysts::no || (!m_fillAllSysts && !IsHistForSystematics(fullname)))
    return;
  for (unsigned int i = 0; i < m_weightSysts->size(); i++) {
    float weightSys = weight * m_weightSysts->at(i).factor;
    m_nameSvc->set_variation(m_weightSysts->at(i).name);
    static_cast<TH2F*>(BookHist(name, nbinsx, xbins, nbinsy, ybins))
        ->Fill(xvalue, yvalue, weightSys);
  }
  for (unsigned int i = 0; i < m_weightSystsCS->size(); i++) {
    float weightSys = weight * m_weightSystsCS->at(i).factor;
    m_nameSvc->set_variation(m_weightSystsCS->at(i).name);
    static_cast<TH2F*>(BookHist(name, nbinsx, xbins, nbinsy, ybins))
        ->Fill(xvalue, yvalue, weightSys);
  }
  m_nameSvc->set_isNominal();
}

TTree* HistSvc::BookTree(const string& name, const string& branchName,
                         void* address, const std::string& leaves) {
  TTree* tree;
  string fullname = name;
  string tname = histName(name);
  treestore_t::const_iterator treeItr = m_trees->find(fullname);
  if (treeItr == m_trees->end()) {
    tree = new TTree(tname.c_str(), tname.c_str());
    tree->SetDirectory(0);
    tree->Branch(branchName.c_str(), address, leaves.c_str());
    (*m_trees)[fullname] = tree;
  } else {
    tree = (*m_trees)[fullname];
  }

  return tree;
}

void HistSvc::BookFillTree(const string& name, const string& branchName,
                           void* address, const std::string& leaves) {
  BookTree(name, branchName, address, leaves)->Fill();
}

TTree* HistSvc::BookTree(const string& name, const string& branchName,
                         double* address) {
  TTree* tree;
  string fullname = name;
  string tname = histName(name);
  treestore_t::const_iterator treeItr = m_trees->find(fullname);
  if (treeItr == m_trees->end()) {
    tree = new TTree(tname.c_str(), tname.c_str());
    tree->SetDirectory(0);

    char* names = const_cast<char*>(branchName.c_str());
    char* pch = strtok(names, ":");
    int i(0);

    while (pch) {
      tree->Branch(pch, address + i++);
      pch = strtok(0, ":");
    }

    (*m_trees)[fullname] = tree;
  } else {
    tree = (*m_trees)[fullname];
  }

  return tree;
}

void HistSvc::BookFillTree(const string& name, const string& branchName,
                           double* address) {
  BookTree(name, branchName, address)->Fill();
}

TTree* HistSvc::BookTree(const string& name,
                         const map<string, double*>& branches) {
  //Check if the number of branches and addresses is the same
  if (branches.size() == 0) {
    std::cout << "BookTree() - You dont have any branches in your map.  "
                 "Resolve this.";
    exit(EXIT_FAILURE);
  }

  TTree* tree;
  string fullname = m_nameSvc->getFullHistName(name);
  string tname = histName(name);

  //Check if the tree exists in the map of trees
  treestore_t::const_iterator treeItr = m_trees->find(fullname);
  if (treeItr == m_trees->end()) {
    //if it does not, then make a new tree and initialize the addresses
    tree = new TTree(tname.c_str(), tname.c_str());
    tree->SetDirectory(0);

    for (auto const it : branches) {
      std::cout << "Adding: " << it.first.c_str() << std::endl;
      tree->Branch(it.first.c_str(), it.second);
    }

    (*m_trees)[fullname] = tree;
  } else {
    //if it does, then just get that tree
    tree = (*m_trees)[fullname];
  }

  return tree;
}

void HistSvc::BookFillTree(const string& name,
                           const map<string, double*>& branches) {
  BookTree(name, branches)->Fill();
}

TH1* HistSvc::BookCutHist(const string& name, int nbinsx, string cuts[]) {
  string fullname = name;

  // no automatic names for cut flow hists currently, only fill Nominal
  if (m_nameSvc) {
    if (!m_nameSvc->get_isNominal()) return nullptr;
  }

  TH1* hist = FindHistInMap(fullname);

  if (hist == 0) {
    string hname = histName(name);
    hist = new TH1D(hname.c_str(), hname.c_str(), nbinsx, -0.5, nbinsx - 0.5);
    hist->Sumw2();
    m_hists[fullname] = hist;
    for (int i = 0; i < nbinsx; i++) {
      hist->GetXaxis()->SetBinLabel(i + 1, cuts[i].c_str());
    }
  }

  return hist;
}

void HistSvc::BookFillCutHist(const string& name, int nbinsx, string cuts[],
                              const string& label, float weight) {
  TH1* hist = BookCutHist(name, nbinsx, cuts);
  if (!hist) return;
  int ibin = hist->GetXaxis()->FindBin(label.c_str());
  hist->Fill(ibin - 1, weight);
}

void HistSvc::BookFillCutHistDual(const string& name, int nbinsx, string cuts[],
                                  const string& label, float weight) {
  //Weighted
  TH1* hist = BookCutHist(name, nbinsx, cuts);
  if (!hist) return;
  int ibin = hist->GetXaxis()->FindBin(label.c_str());
  hist->Fill(ibin - 1, weight);

  //NoWeight
  string name_noweight = name;
  name_noweight.append("_NoWeight");
  TH1* hist_noweight = BookCutHist(name_noweight, nbinsx, cuts);
  if (!hist) return;
  ibin = hist_noweight->GetXaxis()->FindBin(label.c_str());
  hist_noweight->Fill(ibin - 1, 1.0);
}
void HistSvc::BookFillCutHistTripel(const string& name, int nbinsx,
                                    string cuts[], const string& label,
                                    double weight, double addWeight) {
  //   std::string dir = "CutFlow/Nominal/";
  std::string fullname;
  std::string tmp;
  if (m_nameSvc) {
    fullname = m_nameSvc->get_sample();
  }

  tmp = fullname + "_" + name;

  //Weighted
  TH1* hist = BookCutHist(tmp, nbinsx, cuts);
  if (!hist) return;
  int ibin = hist->GetXaxis()->FindBin(label.c_str());
  hist->Fill(ibin - 1, weight);

  //AddWeight
  string name_addweight = tmp;
  name_addweight.append("_AddWeight");
  TH1* hist_addweight = BookCutHist(name_addweight, nbinsx, cuts);
  if (!hist_addweight) return;
  ibin = hist_addweight->GetXaxis()->FindBin(label.c_str());
  hist_addweight->Fill(ibin - 1, weight * addWeight);

  //NoWeight
  string name_noweight = tmp;
  name_noweight.append("_NoWeight");
  TH1* hist_noweight = BookCutHist(name_noweight, nbinsx, cuts);
  if (!hist_noweight) return;
  ibin = hist_noweight->GetXaxis()->FindBin(label.c_str());
  hist_noweight->Fill(ibin - 1, 1.0);
}

// Let's create a 2D version of the above
TH1* HistSvc::BookCutHist(const string& name, int nbinsx, string cutsx[],
                          int nbinsy, string cutsy[]) {
  string fullname = name;

  // no automatic names for cut flow hists currently, only fill Nominal
  if (m_nameSvc) {
    if (!m_nameSvc->get_isNominal()) return nullptr;
  }

  TH1* hist = FindHistInMap(fullname);

  if (hist == 0) {
    string hname = histName(name);
    hist = new TH2F(hname.c_str(), hname.c_str(), nbinsx, -0.5, nbinsx - 0.5,
                    nbinsy, -0.5, nbinsy - 0.5);
    hist->Sumw2();
    m_hists[fullname] = hist;
    for (int i = 0; i < nbinsx; i++) {
      hist->GetXaxis()->SetBinLabel(i + 1, cutsx[i].c_str());
    }
    for (int j = 0; j < nbinsy; j++) {
      hist->GetYaxis()->SetBinLabel(j + 1, cutsy[j].c_str());
    }
  }

  return hist;
}

void HistSvc::BookFillCutHist(const string& name, int nbinsx, string cutsx[],
                              int nbinsy, string cutsy[], const string& labelx,
                              const string& labely, float weight) {
  TH1* hist = BookCutHist(name, nbinsx, cutsx, nbinsy, cutsy);
  if (!hist) return;
  int ibinx = hist->GetXaxis()->FindBin(labelx.c_str());
  int ibiny = hist->GetYaxis()->FindBin(labely.c_str());
  static_cast<TH2F*>(hist)->Fill(ibinx - 1, ibiny - 1, weight);
  /*
  std::cout<<" lable x: "<<labelx.c_str()<<std::endl;
  std::cout<<" ibinx  : "<<ibinx<<std::endl;
  std::cout<<" lable y: "<<labely.c_str()<<std::endl;
  std::cout<<" ibiny  : "<<ibiny<<std::endl;
  */
}

void HistSvc::Write(EL::IWorker* wk) {
  // EventLoop: add the histograms to the TList output objects of the Worker
  for (auto entry : m_hists) {
    TH1* hist = entry.second;
    hist->SetName(entry.first.c_str());
    wk->addOutput(hist);
  }

  // EventLoop: add the histograms to the TList output objects of the Worker
  for (auto entry : m_bootstraphists) {
    TH1DBootstrap* bootstraphist = entry.second;
    int pos = entry.first.find("data");
    for (int i = 0; i < 1000; i++) {
      TH1* hist = bootstraphist->GetReplica(i);
      std::string name = entry.first;
      name.insert(pos + 4, std::to_string(i));
      hist->SetName(name.c_str());
      wk->addOutput(hist);
    }
  }

  // EventLoop: add the trees to the TList output objects of the Worker
  //for (auto entry : m_trees) {
  for (auto entry = m_trees->begin(); entry != m_trees->end(); ++entry) {
    TTree* tree = entry->second;
    tree->SetName(entry->first.c_str());
    wk->addOutput(tree);
  }

  // EventLoop seems to take care of deleting histograms
}

string HistSvc::histName(const string name) {
  std::string::size_type idelim = name.find_last_of("/");
  if (idelim == std::string::npos) return name;
  return name.substr(idelim + 1);
}

string HistSvc::dirName(const string name) {
  std::string::size_type idelim = name.find_last_of("/");
  if (idelim == std::string::npos) return string("");
  return name.substr(0, idelim);
}

void HistSvc::SetWeightSysts(const vector<WeightSyst>* weightSysts) {
  m_weightSysts = weightSysts;
}

void HistSvc::SetWeightSystsCS(const vector<WeightSyst>* weightSysts) {
  m_weightSystsCS = weightSysts;
}

bool HistSvc::IsHistForSystematics(const string& /*fullname*/) { return false; }
