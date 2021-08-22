#ifndef HistSvc_h
#define HistSvc_h

class TH1;
class TFile;
class TTree;

#include "CxAODReader/BootstrapGenerator.h"
#include "CxAODReader/HistNameSvc.h"
#include "CxAODReader/TH1DBootstrap.h"

#include <unordered_map>
using namespace std;

namespace EL {
class IWorker;
}

class HistSvc {
 public:
  enum class doSysts { no = 0, yes = 1 };

  struct WeightSyst {
    string name;
    float factor;
  };

  HistSvc();
  ~HistSvc();

  TH1DBootstrap* BookBootstrapHist(const string& name, int nbinsx, float xlow,
                                   float xup);
  void BookFillBootstrapHist(const string& name, int nbinsx, float xlow,
                             float xup, float value, float weight = 1);
  void GenerateBootstrapWeights(float weight, UInt_t RunNumber,
                                UInt_t EventNumber, UInt_t mc_channel_number);

  TH1* BookHist(const string& name, int nbinsx, float xlow, float xup);
  TH1* BookHist(const string& name, int nbinsx, float* xbins);
  TH1* BookHist(const string& name, int nbinsx, float xlow, float xup,
                int nbinsy, float ylow, float yup);
  TH1* BookHist(const string& name, int nbinsx, float* xbins, int nbinsy,
                float* ybins);

  TH1* BookHist(const string& name, int nbinsx, double* xbins, int nbinsy,
                double ylow, double yup);

  void BookFillHist(const string& name, int nbinsx, float xlow, float xup,
                    float value, float weight = 1,
                    doSysts ds = doSysts::yes);  //TH1 (1D)
  void BookFillHist(const string& name, int nbinsx, float* xbins, float value,
                    float weight = 1, doSysts ds = doSysts::yes);  //TH1 (1D)
  void BookFillHist(const string& name, int nbinsx, float xlow, float xup,
                    int nbinsy, float ylow, float yup, float xvalue,
                    float yvalue, float weight = 1,
                    doSysts ds = doSysts::yes);  //TH2 (2D)
  void BookFillHist(const string& name, int nbinsx, float* xbins, int nbinsy,
                    float* ybins, float xvalue, float yvalue, float weight = 1,
                    doSysts ds = doSysts::yes);  //TH2 (2D)

 void BookFillHist(const string& name, int nbinsx, double* xbins, int nbinsy,
		   double ylow, double yup, float xvalue, float yvalue, float weight = 1,
                    doSysts ds = doSysts::yes);  //TH2 (2D)


  TTree* BookTree(const string& name, const string& branchName, void* address,
                  const std::string& leaves);
  TTree* BookTree(const string& name, const string& branchName,
                  double* address);
  TTree* BookTree(const string& name, const map<string, double*>& branches);

  void BookFillTree(const string& name, const string& branchName, void* address,
                    const std::string& leaves);
  void BookFillTree(const string& name, const string& branchName,
                    double* address);
  void BookFillTree(const string& name, const map<string, double*>& branches);

  TH1* BookCutHist(const string& name, int nbinsx, string cuts[]);
  void BookFillCutHist(const string& name, int nbinsx, string cuts[],
                       const string& label, float weight);
  void BookFillCutHistDual(const string& name, int nbinsx, string cuts[],
                           const string& label, float weight);
  void BookFillCutHistTripel(const string& name, int nbinsx, string cuts[],
                             const string& label, double weight,
                             double addWeight);
  // 2D version of above
  TH1* BookCutHist(const string& name, int nbinsx, string cutsx[], int nbinsy,
                   string cutsy[]);
  void BookFillCutHist(const string& name, int nbinsx, string cutsx[],
                       int nbinsy, string cutsy[], const string& labelx,
                       const string& labely, float weight);

  // If a global m_fillAllSysts flag is false, the IsHistForSystematics function is called to determine if syst variations need to be stored for a histogram fullname.
  // It should be redefined in derived classes (e.g. to get a list of such histograms from a text file).
  virtual bool IsHistForSystematics(const string& fullname);

  void Write(TFile* file);
  void Write(EL::IWorker* wk);

  void Reset();
  TH1* FindHistInMap(const string& name);

  TH1DBootstrap* FindBootstrapHistInMap(const string& name);

  void SetNameSvc(HistNameSvc* nameSvc) { m_nameSvc = nameSvc; }

  void SetWeightSysts(const vector<WeightSyst>* weightSysts);
  void SetWeightSystsCS(const vector<WeightSyst>* weightSysts);

  void SetFillHists(bool fill) { m_fillHists = fill; };

  void SetFillAllSysts(bool fillAllSysts) { m_fillAllSysts = fillAllSysts; };

 private:
  unordered_map<string, TH1*> m_hists;
  unordered_map<string, TH1DBootstrap*> m_bootstraphists;

  typedef unordered_map<string, TTree*> treestore_t;
  treestore_t* m_trees;

  bool m_fillHists;
  bool m_fillAllSysts;

  HistNameSvc* m_nameSvc;
  BootstrapGenerator* m_bootstrap;

  string histName(const string name);
  string dirName(const string name);

  const vector<WeightSyst>* m_weightSysts;
  const vector<WeightSyst>* m_weightSystsCS;
};

#endif
