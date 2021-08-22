#ifndef CxAODReader_MVAReader_H
#define CxAODReader_MVAReader_H

#include <TMVA/Reader.h>

class MVAReader {
 protected:
  // ReaderFold is holds one TMVA::Reader with k (usually 2) registered MVAs.
  // This is for kFolding: all MVAs need to use the same set of variables.
  class ReaderFold {
   public:
    const int k;
    bool isRegression;
    TMVA::Reader* reader;
    ReaderFold(int kFold)
        : k(kFold), isRegression(false), reader(new TMVA::Reader("Silent=0")) {}
    ~ReaderFold() { delete reader; }
  };

  // The split variable determines the samples splitting for the kFolds.
  // Usually one can use the EventNumber.
  // Evaluation of reader i: m_splitVar % k == i
  // Assuming training of i: m_splitVar % k != i
  const unsigned long long* m_splitVar;

  // The MVAReader can hold multiple kFolds, e.g. for various regions.
  // Each kFold might have its one variable set.
  std::map<TString, ReaderFold*> m_reader;

  // The MVAReader holds its own list of variables.
  // Based on the XML weight files they are filtered and ordered,
  // then added to the TMVA::Reader(s).
  std::map<TString, Int_t*> m_varsInt;
  std::map<TString, Float_t*> m_varsFloat;
  std::map<TString, ULong64_t*> m_varsULong64;

  // Get the "Expression" fields for the variables from a TMVA XML file.
  std::vector<TString> getExpressions(TString xmlFile);

  // Test if any word in expression is identical to varName.
  bool matchExpr(TString expression, TString varName);

  // Add variables that can be found in expressions to reader.
  // The match is done by matchExpr().
  void addVariables(TMVA::Reader* reader, std::vector<TString> expressions);

 public:
  MVAReader();

  ~MVAReader();

  void AddReader(TString name, int kFold);
  void AddVariable(TString name, Int_t* branch);
  void AddVariable(TString name, Float_t* branch);
  void AddVariable(TString name, ULong64_t* branch);
  void BookReader(TString name, TString xmlFile);
  float EvaluateMVA(TString name);
  void inline SetSplitVar(const unsigned long long* splitVar) {
    m_splitVar = splitVar;
  }
};

#endif
