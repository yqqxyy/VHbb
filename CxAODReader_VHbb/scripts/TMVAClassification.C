#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"


#include "TSystem.h"
#include "TROOT.h"

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

#include "TMVA/MethodCategory.h"

using namespace std;

int TMVAClassification(TString myMethodList = "")
{
  // gSystem->Exec("setupAtlas");
  // gSystem->Exec("localSetupROOT");

  TMVA::Tools::Instance();
  std::map<std::string,int> Use;
  Use["BDT"]             = 1; // uses Adaptive Boost

  // ---------------------------------------------------------------

  std::cout << std::endl;
  std::cout << "==> Start TMVAClassification" << std::endl;

  // Select methods (don't look at this code - not of interest)
  if (myMethodList != "") {
    for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) it->second = 0;

    std::vector<TString> mlist = TMVA::gTools().SplitString( myMethodList, ',' );
    for (UInt_t i=0; i<mlist.size(); i++) {
      std::string regMethod(mlist[i]);

      if (Use.find(regMethod) == Use.end()) {
	std::cout << "Method \"" << regMethod << "\" not known in TMVA under this name. Choose among the following:" << std::endl;
	for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) std::cout << it->first << " ";
	std::cout << std::endl;
	return 1;
      }
      Use[regMethod] = 1;
    }
  }

  // --------------------------------------------------------------------------------------------------

  // --- Here the preparation phase begins

  // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
  TString outfileName( "TMVA.root" );
  TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

  // Create the factory object.
  // TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
  //					      "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
  TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
					      "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification" );

  // Modify settings for the variable plotting
  // (TMVA::gConfig().GetVariablePlotting()).fTimesRMS = 8.0;
  // (TMVA::gConfig().GetVariablePlotting()).fNbins1D  = 60.0;
  // (TMVA::gConfig().GetVariablePlotting()).fNbins2D  = 300.0;
  // Modify the binning in the ROC curve (for classification only)
  // (TMVA::gConfig().GetVariablePlotting()).fNbinsXOfROCCurve = 100;
  // (TMVA::gConfig().GetIONames()).fWeightFileDir = "myWeightDirectory";

  // Define spectator variables that shall be used for events categorisation
  // These spectator variables should not be used for the MVA training
  factory->AddSpectator("EventWeight",   'F'); //
  factory->AddSpectator("nJet",          'I'); //

  // Apply additional cuts on the signal and background samples
  // Keep this in order to run alternative trainings and cross check
  // the MethodCategory trainings
  TCut mycut = "nJet==2"; // set the cut to nJet>=2 if you want to use MethodCategory

  // Define the input variables that shall be used for the MVA training
  factory->AddVariable("MET",            'F');
  factory->AddVariable("HT",             'F');
  factory->AddVariable("dPhiMETdijet",   'F');
  factory->AddVariable("pTB1",           'F');
  factory->AddVariable("pTB2",           'F');
  factory->AddVariable("mB1B2",          'F');
  factory->AddVariable("dRB1B2",         'F');
  factory->AddVariable("dEtaB1B2",       'F');

  // 3jets only
  if(mycut != "nJet==2")
    {
      factory->AddVariable("pTJ",            'F');
      factory->AddVariable("mbbj",           'F');
    }
  
  // Tagging information should wait for a nice calibration
  // factory->AddVariable("MV2c20b1",       'F');
  // factory->AddVariable("MV2c20b2",       'F');
  // factory->AddVariable("iMV2c20b1",      'I');
  // factory->AddVariable("iMV2c20b2",      'I');

  // factory->AddVariable("MV2c20J",        'F');
  // factory->AddVariable("iMV2c20J",       'F');

  

  // ---------------------------------------------------------------
  
  // Keep samples independent for further tests
  // Read training and test data
  TFile *f_WH125_0	  = new TFile("data-MVATree/WH125-0.root",        "READ");
  TFile *f_WW_0           = new TFile("data-MVATree/WW-0.root",           "READ");
  TFile *f_WZ_0	          = new TFile("data-MVATree/WZ-0.root",           "READ");
  TFile *f_WenuB_0	  = new TFile("data-MVATree/WenuB-0.root",        "READ");
  TFile *f_WenuB_1	  = new TFile("data-MVATree/WenuB-1.root",        "READ");
  TFile *f_WenuC_0	  = new TFile("data-MVATree/WenuC-0.root",        "READ");
  TFile *f_WenuC_1	  = new TFile("data-MVATree/WenuC-1.root",        "READ");
  TFile *f_WenuL_0	  = new TFile("data-MVATree/WenuL-0.root",        "READ");
  TFile *f_WenuL_1	  = new TFile("data-MVATree/WenuL-1.root",        "READ");
  TFile *f_WenuL_2	  = new TFile("data-MVATree/WenuL-2.root",        "READ");
  TFile *f_WmunuB_0	  = new TFile("data-MVATree/WmunuB-0.root",       "READ");
  TFile *f_WmunuB_1	  = new TFile("data-MVATree/WmunuB-1.root",       "READ");
  TFile *f_WmunuB_2	  = new TFile("data-MVATree/WmunuB-2.root",       "READ");
  TFile *f_WmunuC_0	  = new TFile("data-MVATree/WmunuC-0.root",       "READ");
  TFile *f_WmunuC_1	  = new TFile("data-MVATree/WmunuC-1.root",       "READ");
  TFile *f_WmunuC_2	  = new TFile("data-MVATree/WmunuC-2.root",       "READ");
  TFile *f_WmunuL_0	  = new TFile("data-MVATree/WmunuL-0.root",       "READ");
  TFile *f_WmunuL_1	  = new TFile("data-MVATree/WmunuL-1.root",       "READ");
  TFile *f_WmunuL_2	  = new TFile("data-MVATree/WmunuL-2.root",       "READ");
  TFile *f_WtaunuB_0	  = new TFile("data-MVATree/WtaunuB-0.root",      "READ");
  TFile *f_WtaunuB_1	  = new TFile("data-MVATree/WtaunuB-1.root",      "READ");
  TFile *f_WtaunuC_0	  = new TFile("data-MVATree/WtaunuC-0.root",      "READ");
  TFile *f_WtaunuC_1	  = new TFile("data-MVATree/WtaunuC-1.root",      "READ");
  TFile *f_WtaunuL_0	  = new TFile("data-MVATree/WtaunuL-0.root",      "READ");
  TFile *f_WtaunuL_1	  = new TFile("data-MVATree/WtaunuL-1.root",      "READ");
  TFile *f_ZHll125_0	  = new TFile("data-MVATree/ZHll125-0.root",      "READ");
  TFile *f_ZHvv125_0	  = new TFile("data-MVATree/ZHvv125-0.root",      "READ");
  TFile *f_ZZ_0	          = new TFile("data-MVATree/ZZ-0.root",           "READ");
  TFile *f_ZeeB_0	  = new TFile("data-MVATree/ZeeB-0.root",         "READ");
  TFile *f_ZeeB_1	  = new TFile("data-MVATree/ZeeB-1.root",         "READ");
  TFile *f_ZeeC_0	  = new TFile("data-MVATree/ZeeC-0.root",         "READ");
  TFile *f_ZeeC_1	  = new TFile("data-MVATree/ZeeC-1.root",         "READ");
  TFile *f_ZeeL_0	  = new TFile("data-MVATree/ZeeL-0.root",         "READ");
  TFile *f_ZeeL_1	  = new TFile("data-MVATree/ZeeL-1.root",         "READ");
  TFile *f_ZmumuB_0	  = new TFile("data-MVATree/ZmumuB-0.root",       "READ");
  TFile *f_ZmumuB_1	  = new TFile("data-MVATree/ZmumuB-1.root",       "READ");
  TFile *f_ZmumuC_0	  = new TFile("data-MVATree/ZmumuC-0.root",       "READ");
  TFile *f_ZmumuC_1	  = new TFile("data-MVATree/ZmumuC-1.root",       "READ");
  TFile *f_ZmumuL_0	  = new TFile("data-MVATree/ZmumuL-0.root",       "READ");
  TFile *f_ZmumuL_1	  = new TFile("data-MVATree/ZmumuL-1.root",       "READ");
  TFile *f_ZnunuB_0	  = new TFile("data-MVATree/ZnunuB-0.root",       "READ");
  TFile *f_ZnunuB_1	  = new TFile("data-MVATree/ZnunuB-1.root",       "READ");
  TFile *f_ZnunuC_0	  = new TFile("data-MVATree/ZnunuC-0.root",       "READ");
  TFile *f_ZnunuC_1	  = new TFile("data-MVATree/ZnunuC-1.root",       "READ");
  TFile *f_ZnunuC_2	  = new TFile("data-MVATree/ZnunuC-2.root",       "READ");
  TFile *f_ZnunuL_0	  = new TFile("data-MVATree/ZnunuL-0.root",       "READ");
  TFile *f_ZnunuL_1	  = new TFile("data-MVATree/ZnunuL-1.root",       "READ");
  TFile *f_ZnunuL_2	  = new TFile("data-MVATree/ZnunuL-2.root",       "READ");
  TFile *f_ZtautauB_0	  = new TFile("data-MVATree/ZtautauB-0.root",     "READ");
  TFile *f_ZtautauB_1	  = new TFile("data-MVATree/ZtautauB-1.root",     "READ");
  TFile *f_ZtautauC_0	  = new TFile("data-MVATree/ZtautauC-0.root",     "READ");
  TFile *f_ZtautauL_0	  = new TFile("data-MVATree/ZtautauL-0.root",     "READ");
  TFile *f_singletop_Wt_0 = new TFile("data-MVATree/singletop_Wt-0.root", "READ");
  TFile *f_singletop_Wt_1 = new TFile("data-MVATree/singletop_Wt-1.root", "READ");
  TFile *f_singletop_s_0  = new TFile("data-MVATree/singletop_s-0.root",  "READ");
  TFile *f_singletop_t_0  = new TFile("data-MVATree/singletop_t-0.root",  "READ");
  TFile *f_singletop_t_1  = new TFile("data-MVATree/singletop_t-1.root",  "READ");
  TFile *f_ttbar_0	  = new TFile("data-MVATree/ttbar-0.root",        "READ");
  TFile *f_ttbar_1        = new TFile("data-MVATree/ttbar-1.root",        "READ");
  
  TTree *t_WH125_0	  = (TTree*)f_WH125_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WW_0           = (TTree*)f_WW_0           ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WZ_0	          = (TTree*)f_WZ_0	     ->GetObjectChecked("Nominal", "TTree");    
  TTree *t_WenuB_0	  = (TTree*)f_WenuB_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WenuB_1	  = (TTree*)f_WenuB_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WenuC_0	  = (TTree*)f_WenuC_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WenuC_1	  = (TTree*)f_WenuC_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WenuL_0	  = (TTree*)f_WenuL_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WenuL_1	  = (TTree*)f_WenuL_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WenuL_2	  = (TTree*)f_WenuL_2	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuB_0	  = (TTree*)f_WmunuB_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuB_1	  = (TTree*)f_WmunuB_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuB_2	  = (TTree*)f_WmunuB_2	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuC_0	  = (TTree*)f_WmunuC_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuC_1	  = (TTree*)f_WmunuC_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuC_2	  = (TTree*)f_WmunuC_2	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuL_0	  = (TTree*)f_WmunuL_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuL_1	  = (TTree*)f_WmunuL_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WmunuL_2	  = (TTree*)f_WmunuL_2	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WtaunuB_0	  = (TTree*)f_WtaunuB_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WtaunuB_1	  = (TTree*)f_WtaunuB_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WtaunuC_0	  = (TTree*)f_WtaunuC_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WtaunuC_1	  = (TTree*)f_WtaunuC_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WtaunuL_0	  = (TTree*)f_WtaunuL_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_WtaunuL_1	  = (TTree*)f_WtaunuL_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZHll125_0	  = (TTree*)f_ZHll125_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZHvv125_0	  = (TTree*)f_ZHvv125_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZZ_0	          = (TTree*)f_ZZ_0	     ->GetObjectChecked("Nominal", "TTree");    
  TTree *t_ZeeB_0	  = (TTree*)f_ZeeB_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZeeB_1	  = (TTree*)f_ZeeB_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZeeC_0	  = (TTree*)f_ZeeC_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZeeC_1	  = (TTree*)f_ZeeC_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZeeL_0	  = (TTree*)f_ZeeL_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZeeL_1	  = (TTree*)f_ZeeL_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZmumuB_0	  = (TTree*)f_ZmumuB_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZmumuB_1	  = (TTree*)f_ZmumuB_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZmumuC_0	  = (TTree*)f_ZmumuC_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZmumuC_1	  = (TTree*)f_ZmumuC_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZmumuL_0	  = (TTree*)f_ZmumuL_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZmumuL_1	  = (TTree*)f_ZmumuL_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuB_0	  = (TTree*)f_ZnunuB_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuB_1	  = (TTree*)f_ZnunuB_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuC_0	  = (TTree*)f_ZnunuC_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuC_1	  = (TTree*)f_ZnunuC_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuC_2	  = (TTree*)f_ZnunuC_2	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuL_0	  = (TTree*)f_ZnunuL_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuL_1	  = (TTree*)f_ZnunuL_1	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZnunuL_2	  = (TTree*)f_ZnunuL_2	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ZtautauB_0	  = (TTree*)f_ZtautauB_0     ->GetObjectChecked("Nominal", "TTree");	  
  TTree *t_ZtautauB_1	  = (TTree*)f_ZtautauB_1     ->GetObjectChecked("Nominal", "TTree");	  
  TTree *t_ZtautauC_0	  = (TTree*)f_ZtautauC_0     ->GetObjectChecked("Nominal", "TTree");	  
  TTree *t_ZtautauL_0	  = (TTree*)f_ZtautauL_0     ->GetObjectChecked("Nominal", "TTree");	  
  TTree *t_singletop_Wt_0 = (TTree*)f_singletop_Wt_0 ->GetObjectChecked("Nominal", "TTree");
  TTree *t_singletop_Wt_1 = (TTree*)f_singletop_Wt_1 ->GetObjectChecked("Nominal", "TTree");
  TTree *t_singletop_s_0  = (TTree*)f_singletop_s_0  ->GetObjectChecked("Nominal", "TTree");
  TTree *t_singletop_t_0  = (TTree*)f_singletop_t_0  ->GetObjectChecked("Nominal", "TTree");
  TTree *t_singletop_t_1  = (TTree*)f_singletop_t_1  ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ttbar_0	  = (TTree*)f_ttbar_0	     ->GetObjectChecked("Nominal", "TTree");
  TTree *t_ttbar_1        = (TTree*)f_ttbar_1        ->GetObjectChecked("Nominal", "TTree");

  // cross sections were already taken into account in the framework
  Double_t signalWeight     = 1.0;
  Double_t backgroundWeight = 1.0;

  int threshold = 10; // missing statistic can result in empty categories for some sample, and inexisting branches
  if(t_WH125_0	      ->GetEntries() > threshold) factory->AddSignalTree(t_WH125_0,            signalWeight);
  if(t_WW_0           ->GetEntries() > threshold) factory->AddBackgroundTree(t_WW_0,           backgroundWeight);
  if(t_WZ_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WZ_0,           backgroundWeight);
  if(t_WenuB_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WenuB_0,        backgroundWeight);
  if(t_WenuB_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WenuB_1,        backgroundWeight);
  if(t_WenuC_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WenuC_0,        backgroundWeight);
  if(t_WenuC_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WenuC_1,        backgroundWeight);
  if(t_WenuL_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WenuL_0,        backgroundWeight);
  if(t_WenuL_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WenuL_1,        backgroundWeight);
  if(t_WenuL_2	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WenuL_2,        backgroundWeight);
  if(t_WmunuB_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuB_0,       backgroundWeight);
  if(t_WmunuB_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuB_1,       backgroundWeight);
  if(t_WmunuB_2	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuB_2,       backgroundWeight);
  if(t_WmunuC_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuC_0,       backgroundWeight);
  if(t_WmunuC_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuC_1,       backgroundWeight);
  if(t_WmunuC_2	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuC_2,       backgroundWeight);
  if(t_WmunuL_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuL_0,       backgroundWeight);
  if(t_WmunuL_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuL_1,       backgroundWeight);
  if(t_WmunuL_2	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WmunuL_2,       backgroundWeight);
  if(t_WtaunuB_0      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WtaunuB_0,      backgroundWeight);
  if(t_WtaunuB_1      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WtaunuB_1,      backgroundWeight);
  if(t_WtaunuC_0      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WtaunuC_0,      backgroundWeight);
  if(t_WtaunuC_1      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WtaunuC_1,      backgroundWeight);
  if(t_WtaunuL_0      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WtaunuL_0,      backgroundWeight);
  if(t_WtaunuL_1      ->GetEntries() > threshold) factory->AddBackgroundTree(t_WtaunuL_1,      backgroundWeight);
  if(t_ZHll125_0      ->GetEntries() > threshold) factory->AddSignalTree(t_ZHll125_0,          signalWeight);
  if(t_ZHvv125_0      ->GetEntries() > threshold) factory->AddSignalTree(t_ZHvv125_0,          signalWeight);
  if(t_ZZ_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZZ_0,           backgroundWeight);
  if(t_ZeeB_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZeeB_0,         backgroundWeight);
  if(t_ZeeB_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZeeB_1,         backgroundWeight);
  if(t_ZeeC_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZeeC_0,         backgroundWeight); // empty
  if(t_ZeeC_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZeeC_1,         backgroundWeight);
  if(t_ZeeL_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZeeL_0,         backgroundWeight);
  if(t_ZeeL_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZeeL_1,         backgroundWeight);
  if(t_ZmumuB_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZmumuB_0,       backgroundWeight);
  if(t_ZmumuB_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZmumuB_1,       backgroundWeight);
  if(t_ZmumuC_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZmumuC_0,       backgroundWeight);
  if(t_ZmumuC_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZmumuC_1,       backgroundWeight);
  if(t_ZmumuL_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZmumuL_0,       backgroundWeight);
  if(t_ZmumuL_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZmumuL_1,       backgroundWeight);
  if(t_ZnunuB_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuB_0,       backgroundWeight);
  if(t_ZnunuB_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuB_1,       backgroundWeight);
  if(t_ZnunuC_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuC_0,       backgroundWeight);
  if(t_ZnunuC_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuC_1,       backgroundWeight);
  if(t_ZnunuC_2	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuC_2,       backgroundWeight);
  if(t_ZnunuL_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuL_0,       backgroundWeight);
  if(t_ZnunuL_1	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuL_1,       backgroundWeight);
  if(t_ZnunuL_2	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZnunuL_2,       backgroundWeight);
  if(t_ZtautauB_0     ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZtautauB_0,     backgroundWeight);
  if(t_ZtautauB_1     ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZtautauB_1,     backgroundWeight);
  if(t_ZtautauC_0     ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZtautauC_0,     backgroundWeight);
  if(t_ZtautauL_0     ->GetEntries() > threshold) factory->AddBackgroundTree(t_ZtautauL_0,     backgroundWeight);
  if(t_singletop_Wt_0 ->GetEntries() > threshold) factory->AddBackgroundTree(t_singletop_Wt_0, backgroundWeight);
  if(t_singletop_Wt_1 ->GetEntries() > threshold) factory->AddBackgroundTree(t_singletop_Wt_1, backgroundWeight);
  if(t_singletop_s_0  ->GetEntries() > threshold) factory->AddBackgroundTree(t_singletop_s_0,  backgroundWeight);
  if(t_singletop_t_0  ->GetEntries() > threshold) factory->AddBackgroundTree(t_singletop_t_0,  backgroundWeight);
  if(t_singletop_t_1  ->GetEntries() > threshold) factory->AddBackgroundTree(t_singletop_t_1,  backgroundWeight);
  if(t_ttbar_0	      ->GetEntries() > threshold) factory->AddBackgroundTree(t_ttbar_0,        backgroundWeight);
  if(t_ttbar_1        ->GetEntries() > threshold) factory->AddBackgroundTree(t_ttbar_1,        backgroundWeight);
			      
           		     			      
  // Set individual event weights (the variable EventWeight must exist as branch in inputs)
  factory->SetWeightExpression("EventWeight");

  // Tell the factory how to use the training and testing events
  factory->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );
  // To also specify the number of testing events, use:
  // factory->PrepareTrainingAndTestTree( mycut, "NSigTrain=3000:NBkgTrain=3000:NSigTest=3000:NBkgTest=3000:SplitMode=Random:!V" );

  // ---- Book MVA methods
  if (Use["BDT"])  // Adaptive Boost
    factory->BookMethod(TMVA::Types::kBDT, "BDT",
			"!H:!V:NTrees=200:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.15:SeparationType=GiniIndex:nCuts=100:NEventsMin=100:PruneMethod=NoPruning");
  
  // --------------------------------------------------------------------------------------------------

  // TMVA::MethodCategory* mJetCategories = 0;
  // TMVA::MethodBase* bdtJetCategories = factory->BookMethod( TMVA::Types::kCategory, "BDTJetCategories","" );
  // mJetCategories = dynamic_cast<TMVA::MethodCategory*>(bdtJetCategories);
  // mJetCategories->AddMethod( "nJet==2", "MET:HT:dPhiMETdijet:pTB1:pTB2:mB1B2:dRB1B2:dEtaB1B2", TMVA::Types::kBDT, "Category_BDT_2j", "!H:!V:NTrees=200:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.15:SeparationType=GiniIndex:nCuts=100:NEventsMin=100:PruneMethod=NoPruning" );
  // mJetCategories->AddMethod( "nJet==3", "MET:HT:dPhiMETdijet:pTB1:pTB2:mB1B2:dRB1B2:dEtaB1B2:pTJ:mbbj", TMVA::Types::kBDT, "Category_BDT_3j", "!H:!V:NTrees=200:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.15:SeparationType=GiniIndex:nCuts=100:NEventsMin=100:PruneMethod=NoPruning");  
  
  // ---- Now you can optimize the setting (configuration) of the MVAs using the set of training events
  // ---- STILL EXPERIMENTAL and only implemented for BDT's ! 
  // factory->OptimizeAllMethods("SigEffAt001","Scan");
  // factory->OptimizeAllMethods("ROCIntegral","FitGA");

  // --------------------------------------------------------------------------------------------------

  // ---- Now you can tell the factory to train, test, and evaluate the MVAs
  // Train MVAs using the set of training events
  factory->TrainAllMethods();
   
  // ---- Evaluate all MVAs using the set of test events
  factory->TestAllMethods();

  // ----- Evaluate and compare performance of all configured MVAs
  factory->EvaluateAllMethods();

  // --------------------------------------------------------------

  // Save the output
  outputFile->Close();

  std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
  std::cout << "==> TMVAClassification is done!" << std::endl;

  delete factory;

  // Launch the GUI for the root macros
  if (!gROOT->IsBatch()) TMVA::TMVAGui( outfileName );

  return 0;
}
