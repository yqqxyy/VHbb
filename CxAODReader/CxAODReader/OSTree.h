#ifndef CxAODReader_OSTree_H
#define CxAODReader_OSTree_H

#include "CxAODReader/MVATree.h"
#include "CxAODReader/StaticMVATree.h"

namespace CxAODReader_OSTree_internal {
template <class Tree>
using STree = StaticMVATree<Tree, bool, int, unsigned, long, unsigned long,
                            float, double, std::string>;
}

class OSTree : public MVATree,
               public CxAODReader_OSTree_internal::STree<OSTree> {
  friend CxAODReader_OSTree_internal::STree<OSTree>;

 protected:
  std::string m_analysisType;

  virtual void AddBranch(TString name, Float_t* address);
  virtual void AddBranch(TString name, Int_t* address);
  virtual void AddBranch(TString name, ULong64_t* address);
  //virtual void AddBranch(TString name, std::string* address);
  template <class T>
  void AddBranch(TString name, T* address) {
    for (std::pair<std::string, TTree*> tree : m_treeMap) {
      tree.second->Branch(name, address);
    }
  }
  virtual void SetBranches() override;
  virtual void TransformVars() override;
  float getBinnedMV1c(float MV1c);

 public:
  OSTree(bool persistent, bool readMVA, std::string analysisType,
         EL::IWorker* wk, std::vector<std::string> variations, bool nominalOnly);
  //  OSTree(bool persistent, bool readMVA, std::string analysisType, EL::Worker* wk);//, std::vector<std::string> variations, bool nominalOnly);

  ~OSTree() {}

  virtual void Reset() override;
  virtual void ReadMVA();
  void Fill();

  std::string sample;
  int MC_Channel_Number;
  int EventNumber;
  int RunNumber;
  std::vector<int> Category;

  float BDT;
 
};

#endif
