#ifndef CxAODReader_MVATree_H
#define CxAODReader_MVATree_H

#include "CxAODReader/MVAReader.h"
#include "TTree.h"

namespace EL {
class IWorker;
}

class MVATree {
 protected:
  bool m_persistent;
  bool m_readMVA;
  bool m_nominalOnly;
  std::map<std::string, TTree*> m_treeMap;
  std::string m_currentVar = "";
  MVAReader m_reader;

  virtual void SetBranches() = 0;
  virtual void TransformVars() = 0;

 public:
  MVATree(bool persistent, bool readMVA, EL::IWorker* wk,
          const std::vector<std::string>& variations, bool nominalOnly);
  MVATree(bool persistent, bool readMVA, EL::IWorker* wk);
  void SetVariation(const std::string& variation);

  virtual ~MVATree();

  virtual void Reset() = 0;
  void Fill();
};

#endif
