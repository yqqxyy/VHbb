#include "CxAODReader/OSTree.h"
#include "EventLoop/IWorker.h"
#include "TFile.h"

OSTree::OSTree(bool persistent, bool readMVA, std::string analysisType,
               EL::IWorker* wk, std::vector<std::string> variations,
               bool nominalOnly)
    : MVATree(persistent, readMVA, wk), m_analysisType(analysisType) {
  if (nominalOnly) variations = {"physics"};
  if (persistent) {
    for (std::string varName : variations) {
      std::cout << "making OStree called " << varName << std::endl;
      TFile* f = wk->getOutputFile("OSTree");
      m_treeMap[varName] = new TTree(varName.c_str(), varName.c_str());
      m_treeMap[varName]->SetDirectory(f);
    }
  } else {
    //do we want variations if we are not storing the trees?
    for (std::string varName : variations) {
      m_treeMap[varName] = new TTree(varName.c_str(), varName.c_str());
    }
  }
  SetBranches();
}

void OSTree::AddBranch(TString name, Int_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void OSTree::AddBranch(TString name, Float_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

void OSTree::AddBranch(TString name, ULong64_t* address) {
  for (std::pair<std::string, TTree*> tree : m_treeMap) {
    tree.second->Branch(name, address);
  }
  m_reader.AddVariable(name, address);
}

// void OSTree::AddBranch(TString name, std::string* address)
// {
//   for (std::pair<std::string, TTree*> tree : m_treeMap) {
//     tree.second -> Branch(name, address);
//   }
// }

void OSTree::SetBranches() {
  AddBranch("EventNumber", &EventNumber);
  AddBranch("RunNumber", &RunNumber);
  AddBranch("category", &Category);
  AddBranch("BDT", &BDT);
}

void OSTree::ReadMVA() {}

void OSTree::TransformVars() {}

void OSTree::Fill() {
  StaticSetBranches(m_analysisType, this);
  SetVariation("physics");
  MVATree::Fill();
}

void OSTree::Reset() {
  EventNumber = -1;
  RunNumber = -1;
  BDT = -99;
  Category.clear();

  StaticReset();
}
