#ifndef CxAODReader_EasyTree_H
#define CxAODReader_EasyTree_H

#include <xAODBase/IParticle.h>

#include "CxAODReader/MVATree.h"
#include "CxAODReader/StaticMVATree.h"

namespace EL {
class IWorker;
}

namespace CxAODReader_EasyTree_internal {
template <class Tree>
using STree = StaticMVATree<Tree, bool, int, unsigned, long, unsigned long,
                            unsigned long long, float, double, std::string,
                            xAOD::IParticle::FourMom_t, std::vector<int>,
                            std::vector<float>, std::vector<double>,
                            std::vector<xAOD::IParticle::FourMom_t>>;
}

class EasyTree : public MVATree,
                 public CxAODReader_EasyTree_internal::STree<EasyTree> {
  friend CxAODReader_EasyTree_internal::STree<EasyTree>;

 protected:
  std::string m_analysisType;

  template <class T>
  void AddBranch(TString name, T *address) {
    for (std::pair<std::string, TTree *> tree : m_treeMap) {
      tree.second->Branch(name, address);
    }
  }

  virtual void SetBranches() override {}

  virtual void TransformVars() override {}

 public:
  EasyTree(bool persistent, std::string analysisType, EL::IWorker *wk,
           std::vector<std::string> variations, bool nominalOnly);
  ~EasyTree() {}

  virtual void Reset() override { StaticReset(); }

  void Fill();
};

#endif  // ifndef CxAODReader_EasyTree_H
