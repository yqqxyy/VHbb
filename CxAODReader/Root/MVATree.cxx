#include "CxAODReader/MVATree.h"
#include "EventLoop/IWorker.h"
#include "TError.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>

//MVATree::MVATree(bool persistent, bool readMVA) :

MVATree::MVATree(bool persistent, bool readMVA, EL::IWorker* wk,
                 const std::vector<std::string>& variations, bool nominalOnly)
    : m_persistent(persistent),
      m_readMVA(readMVA),
      m_nominalOnly(nominalOnly),
      m_treeMap(),
      m_currentVar(""),
      m_reader() {
  //make map of trees for storing systematic variations
  std::vector<std::string> vars;
  if (nominalOnly) {
    vars = {"Nominal"};
  } else {
    vars = variations;
  }
  Info("MVATree::MVATree()",
       "Initialize tree for %lu variations and persistent = %u.", vars.size(),
       persistent);
  if (m_persistent) {
    for (std::string varName : vars) {
      //std::cout << "making tree called " << varName << std::endl;
      TFile* f = wk->getOutputFile("MVATree");
      m_treeMap[varName] = new TTree(varName.c_str(), varName.c_str());
      m_treeMap[varName]->SetDirectory(f);
    }
  } else {
    //do we want variations if we are not storing the trees?
    for (std::string varName : vars) {
      m_treeMap[varName] = new TTree(varName.c_str(), varName.c_str());
    }
  }
}

MVATree::MVATree(bool persistent, bool readMVA, EL::IWorker* /*wk*/)
    : m_persistent(persistent),
      m_readMVA(readMVA),
      m_nominalOnly(true),
      m_treeMap(),
      m_currentVar(""),
      m_reader() {}

void MVATree::SetVariation(const std::string& variation) {
  m_currentVar = variation;
}

MVATree::~MVATree() {
  std::map<std::string, TTree*>::iterator it;
  for (it = m_treeMap.begin(); it != m_treeMap.end(); it++) {
    delete it->second;
  }
  //delete m_treeMap;
}

void MVATree::Fill() {
  if (!m_persistent) {
    return;
  }
  // TODO fill different trees for regions (MVA training)?
  //      -> could use HistNameSvc to decide

  // TODO impose limits on variables here, e.g.
  // if (mBB > 175e3) mBB = 175e3;
  // or also transformations like
  // mBB = mBB / (mBB + mean(mBB))

  TransformVars();

  if (m_currentVar == "") {
    Error("MVATree::Fill()",
          "Variation is not set! Did you call SetVariation in your fill "
          "function?");
    exit(EXIT_FAILURE);
  }

  if (m_treeMap.find(m_currentVar) == m_treeMap.end()) {
    Error("MVATree::Fill()", "No tree registered for variation '%s'",
          m_currentVar.c_str());
    exit(EXIT_FAILURE);
  }

  m_treeMap[m_currentVar]->Fill();
}
