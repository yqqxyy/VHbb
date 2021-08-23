/**
 * @class Plotter
 * @brief Class for internal KF plotting
 * @author Manuel Proissl <mproissl@cern.ch>
 */

#ifndef __PLOTTER_H__
#define __PLOTTER_H__

//Root
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include <TChain.h>
#include "TTree.h"
#include "TBranch.h"

//STL
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

class Variable
{
 public:
  Variable()
    {
      fN   = 0;
      fMin = 1e20;
      fMax = -1e20;
    };
  ~Variable(){};

  //Props
  double fMin;
  double fMax;

  //Iterative
  int fN;       /* Number of calls */
  double fSum;  /* Sum of calls    */
  double fMean; /* Mean of calls   */
};

class Tree
{
 public:
  Tree(TTree* tree, TString name)
    {
      fTree    = tree;
      fName    = name;
      fCurrent = -1;

      var      = -999;
      pt       = -999;
      eta      = -999;
      phi      = -999;
      E        = -999;
      weight   = -999;

      b_var    = NULL;
      b_pt     = NULL;
      b_eta    = NULL;
      b_phi    = NULL;
      b_E      = NULL;
      b_weight = NULL;
    };
  ~Tree(){};

  //Tree
  TTree          *fTree;    //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain
  TString         fName;

  //Vars
  Double_t         var;
  Double_t         pt;
  Double_t         eta;
  Double_t         phi;
  Double_t         E;
  Double_t         weight;  //Event weight

  //Branch
  TBranch        *b_var;
  TBranch        *b_pt;
  TBranch        *b_eta;
  TBranch        *b_phi;
  TBranch        *b_E;
  TBranch        *b_weight;
};

class Histogram
{
 public:
  Histogram() 
    {
      is1D = false;
      is2D = false;
      is3D = false;

      fType   = "DEF";
      fName   = "DEF";
      fXTitle = "DEF";
      fYTitle = "DEF";
      fNXBins = -999;
      fNYBins = -999;
      fXLow   = -999;
      fYLow   = -999;
      fXHigh  = -999;
      fYHigh  = -999;
    };
  ~Histogram(){};

  //Basics
  bool is1D;
  bool is2D;
  bool is3D;

  //Config
  TString fType;
  TString fName;
  TString fXTitle;
  TString fYTitle;
  Int_t   fNXBins;
  Int_t   fNYBins;
  Float_t fXLow;
  Float_t fYLow;
  Float_t fXHigh;
  Float_t fYHigh;
};

class Plotter
{
 public:
  Plotter();
  virtual ~Plotter();

  //Init
  void Init(TString filename);

  /************* CONTAINERS *************/

  //Variable Container
  std::vector<std::pair<TString, Variable*> > fVariableContainer;

  //Histogram Type Container
  std::vector<std::pair<TString, Histogram*> > fHistTypeContainer; //(TypeName, TypeObject)

  //Histogram Data Container
  std::vector<std::pair<TString, TList*> > fHistDataContainer; //(DataType, HistDataObject)
        
  //Tree Container
  std::vector<Tree*> fTreeContainer;

  /************* FUNCTIONS *************/

  //Define types of histograms
  //1D
  void DefineHistogramType(TString histtype, TString xtitle, Int_t xbins, Float_t xlow, Float_t xhigh, TString ytitle );
  //2D
  void DefineHistogramType(TString histtype, TString xtitle, Int_t xbins, Float_t xlow, Float_t xhigh, TString ytitle, Int_t ybins, Float_t ylow, Float_t yhigh );

  //Fill trees
  /*            unique ID         probe       4-vector --->                          <---  event weight */  
  bool FillTree(TString treename, double var, double pt, double eta, double phi, double E, double weight);

  //Fill histogram (and dynamically add if missing, given an existing data and hist type)
  bool Fill(TString datatype, TString histtype, TString histname, double value, double weight);
  bool Fill(TString datatype, TString histtype, TString histname, double xvalue, double yvalue, double weight);

  //Make new histogram for given datatype and histtype
  TH1F* Get1DHistogram(TString datatype, TString histname, Int_t histtype_idx);
  TH2F* Get2DHistogram(TString datatype, TString histname, Int_t histtype_idx);

  //Histogram styles
  void h1style(TH1F& h);
  void h2style(TH2F& h);

  //Write to File
  TFile* fOutfile;
  void WriteOutput();

  //Utils
  void FindRange(TString name, double value);
};


//Default constructor
//___________________________________________________________________________________
inline Plotter::Plotter() :
	       fVariableContainer(0),
	       fHistTypeContainer(0),
	       fHistDataContainer(0),
	       fTreeContainer(0),
	       fOutfile(0)
{
}//;


//Destructor
//___________________________________________________________________________________
inline Plotter::~Plotter()
{
}//;


//Init
//___________________________________________________________________________________
inline void Plotter::Init(TString filename)
{
  //Output file 
  fOutfile = new TFile(filename, "RECREATE");
  if(!fOutfile) { 
    std::cout << "Plotter: ( ERROR ) \t Cannot open " << filename << std::endl;
    exit(-1);
  }
    
  /* //Define histogram types ----> examples ---->
     DefineHistogramType("Truth-pt", "Truth pT [GeV]", 100, 0, 200, "Events");
     DefineHistogramType("Counter", "Count",        2,  0,    2,    "Events");
     DefineHistogramType("KF-mJJ", "Dijet invariant mass [GeV]", 100,   0, 300, "Arbitrary Units");
     DefineHistogramType("KF-LHD", "Kinematic Likelihood",       100,   0, 300, "Arbitrary Units");
     DefineHistogramType("KF-mLL", "m_{#ell#ell} [GeV]",         100,  80, 100, "Arbitrary Units");
     DefineHistogramType("KF-Px",  "Px of ZH system [GeV]",      100, -90,  90, "Arbitrary Units");
     DefineHistogramType("KF-Py",  "Py of ZH system [GeV]",      100, -90,  90, "Arbitrary Units");

     DefineHistogramType("KF-Pull",  "#frac{p^{-1}_{T,rec} - p^{-1}_{T,fit}}{#sigma_{q/p} #times SF}", 500, -3, 3, "Events");
     DefineHistogramType("KF-Pull2", "#frac{(p^{-1}_{T,rec} - p^{-1}_{T,fit})^2}{(#sigma_{q/p} #times SF)^2}", 500, 0, 0.5, "Events");

     DefineHistogramType("KF-QPCov",   "Muon #sigma_{q/p} [GeV]",          500, 0., 1.5e-09, "Events");
     DefineHistogramType("KF-ERes",    "Electron energy resolution [GeV]", 500, 0., 10.,     "Events");

     DefineHistogramType("KF-E-ERes",   "Electron energy resolution [GeV]",  500, 0., 15, "Energy [GeV]", 500, 0., 500.);
     DefineHistogramType("KF-Eta-ERes", "Electron energy resolution [GeV]",  500, 0., 15, "|#eta|", 500, 0., 2.5);
     DefineHistogramType("KF-PtInv-MuRes", "Muon #sigma_{q/p} [GeV]",           500, 0., 1.5e-09, "1/p [GeV]",    500, 0., 0.05);
     //DefineHistogramType("KF-PtInv-MuRes", "Muon 1/p^{2} #sigma_{q/p}", 300, 0., 2e-15, "p [MeV]", 300, 0., 1e+6); 
     */
}//;//Init


//Define new type of a 1D histogram
//___________________________________________________________________________________
inline void Plotter::DefineHistogramType(TString histtype, TString xtitle, Int_t xbins, Float_t xlow, Float_t xhigh, TString ytitle ) 
{    
  //Check if histtype already defined
  if(int(fHistTypeContainer.size()) != 0) {
    for(unsigned int i=0; i<fHistTypeContainer.size(); i++) {
      if(fHistTypeContainer[i].first == histtype) {
	std::cout << "Plotter: ( WARNING ) \t histtype " << histtype << " already defined." << std::endl;
	return;
      }
    }
  }

  //Make new histogram type object
  Histogram* newHistType = new Histogram();
    
  //Configure
  newHistType->is1D    = true;
  newHistType->fType   = histtype;
  newHistType->fXTitle = xtitle;
  newHistType->fYTitle = ytitle;
  newHistType->fNXBins = xbins;
  newHistType->fXLow   = xlow;
  newHistType->fXHigh  = xhigh;

  //Store
  std::pair<TString, Histogram*> newPair(histtype, newHistType);
  fHistTypeContainer.push_back(newPair);

}//;//DefineHistogramType


//Define new type of a 2D histogram
//___________________________________________________________________________________
inline void Plotter::DefineHistogramType(TString histtype, TString xtitle, Int_t xbins, Float_t xlow, Float_t xhigh, TString ytitle, Int_t ybins, Float_t ylow, Float_t yhigh )
{ 
  //Check if histtype already defined
  if(int(fHistTypeContainer.size()) != 0) {
    for(unsigned int i=0; i<fHistTypeContainer.size(); i++) {
      if(fHistTypeContainer[i].first == histtype) {
	std::cout << "Plotter: ( WARNING ) \t histtype " << histtype << " already defined." << std::endl;
	return;
      }
    }
  }

  //Make new histogram type object
  Histogram* newHistType = new Histogram();
    
  //Configure
  newHistType->is2D    = true;
  newHistType->fType   = histtype;
  newHistType->fXTitle = xtitle;
  newHistType->fYTitle = ytitle;
  newHistType->fNXBins = xbins;
  newHistType->fXLow   = xlow;
  newHistType->fXHigh  = xhigh;
  newHistType->fNYBins = ybins;
  newHistType->fYLow   = ylow;
  newHistType->fYHigh  = yhigh;


  //Store
  std::pair<TString, Histogram*> newPair(histtype, newHistType);
  fHistTypeContainer.push_back(newPair);

}//;//DefineHistogramType


//Fill 4-vector tree
//___________________________________________________________________________________
inline bool Plotter::FillTree(TString treename, double var, double pt, double eta, double phi, double E, double weight)
{
  Tree* myTree = NULL;

  //Check if need to fill existing tree
  for(unsigned int i=0; i < fTreeContainer.size(); i++) {
    if(fTreeContainer.at(i)->fName == treename) {
      myTree = fTreeContainer.at(i);
      break;
    }
  }

  //Fill to existing tree
  if(myTree) {
    myTree -> var     = var;
    myTree -> pt     = pt;
    myTree -> eta    = eta;
    myTree -> phi    = phi;
    myTree -> E      = E;
    myTree -> weight = weight;

    myTree -> fTree -> Fill();
    myTree -> fCurrent = myTree -> fTree -> GetTreeNumber();
    return true;
  }

  //Make new tree and fill
  TTree* _myTree = new TTree(treename.Data(), treename.Data());
  myTree = new Tree(_myTree,treename);

  myTree -> fTree -> Branch("var",      &myTree->var,      "var/D"   );
  myTree -> fTree -> Branch("pt",       &myTree->pt,       "pt/D"    );
  myTree -> fTree -> Branch("eta",      &myTree->eta,      "eta/D"   );
  myTree -> fTree -> Branch("phi",      &myTree->phi,      "phi/D"   );
  myTree -> fTree -> Branch("E",        &myTree->E,        "E/D"     );
  myTree -> fTree -> Branch("weight",   &myTree->weight,   "weight/D");

  myTree -> var    = var;
  myTree -> pt     = pt;
  myTree -> eta    = eta;
  myTree -> phi    = phi;
  myTree -> E      = E;
  myTree -> weight = weight;

  myTree -> fTree -> Fill();
  myTree -> fCurrent = myTree -> fTree -> GetTreeNumber();
  fTreeContainer.push_back(myTree);

  return true;

}//;//FillTree


//Fill histogram (and dynamically add if missing, given an existing data and hist type)
//___________________________________________________________________________________
inline bool Plotter::Fill(TString datatype, TString histtype, TString histname, double value, double weight)
{
  //Check if hist types are available
  if(int(fHistTypeContainer.size()) == 0) {
    std::cout << "Plotter: ( ERROR ) \t No hist types defined. Abort!" << std::endl;
    return false;
  }

  //--------------------------------------------------------------------------------------------------

  //Check if histtype already defined
  int histtype_Idx = -1;
  for(unsigned int i=0; i<fHistTypeContainer.size(); i++) {
    if(fHistTypeContainer[i].first == histtype) {
      histtype_Idx = i;
      break;
    }//search
  }//loop

  if(histtype_Idx<0) {
    std::cout << "Plotter: ( ERROR ) \t Requested histtype " << histtype << " for filling not found. Abort!" << std::endl;
    return false;
  }//exception
    
  //--------------------------------------------------------------------------------------------------

  //Check if histtype is 1D
  if(!fHistTypeContainer[histtype_Idx].second->is1D) {
    std::cout << "Plotter: ( ERROR ) \t Requested histtype " << histtype << " is not 1D. Abort!" << std::endl;
    return false;
  }

  //--------------------------------------------------------------------------------------------------

  //Check if datatype previously defined
  int datatype_Idx = -1;
  bool addNewDataType(false);
  if(int(fHistDataContainer.size()) != 0) {
    for(unsigned int i=0; i<fHistDataContainer.size(); i++) {
      if(fHistDataContainer[i].first == datatype) {
	datatype_Idx = i;
	break;
      }//search
    }//loop

    if(datatype_Idx<0) {
      addNewDataType = true;
    }
  }//data types available
  else {
    addNewDataType = true;
  }//no data types available

  //--------------------------------------------------------------------------------------------------

  //Add new data type branch, new histogram to branch and fill it
  TList* dataBranch = NULL;
  if(addNewDataType) {
    //Setup new branch
    dataBranch = new TList();

    //Declare new histogram
    TH1F* tmp = Get1DHistogram(datatype, histname, histtype_Idx);

    //Fill current event value
    tmp->Fill(value, weight);

    //Add histogram to branch
    dataBranch->Add(tmp);

    //Store in data container
    std::pair<TString, TList*> newPair(datatype, dataBranch);
    fHistDataContainer.push_back(newPair);
        
    return true;
  }

  //--------------------------------------------------------------------------------------------------

  //Fill existing histogram
  dataBranch = fHistDataContainer[datatype_Idx].second;
  TIter next(dataBranch);
  TH1 *iter;
  while( (iter=(TH1 *)next()) ) { 
    TString _histname(iter->GetName());
    if(_histname == (datatype+"_"+histtype+"_"+histname)) {
      iter->Fill(value, weight);
      return true;
    }//fill histogram
  }//search loop
    
  //--------------------------------------------------------------------------------------------------

  //Add and fill new histogram
  TH1F* tmp2 = Get1DHistogram(datatype, histname, histtype_Idx);
  tmp2->Fill(value, weight);
  dataBranch->Add(tmp2);
    
  return true;
}//;//Fill


//Declare and return 1D histogram
//___________________________________________________________________________________
inline TH1F* Plotter::Get1DHistogram(TString datatype, TString histname, Int_t histtype_Idx)
{
  TH1F* tmp = NULL;

  //Sanity check
  if(histtype_Idx<0) {
    std::cout << "Plotter: ( ERROR ) \t histtype_idx is negative. Abort!" << std::endl;
    return tmp;
  }

  if(fHistTypeContainer[histtype_Idx].second->is1D) {
    /*
      Double_t ptBins[7] = {20,35,45,60,70,80,100};
      Double_t etaBins[12] = {-2.5,-2,-1.52,-1.37,-0.8,-0.1,0.1,0.8,1.37,1.52,2,2.5};
       
      if(fHistTypeContainer[histtype_Idx].second->fType.Contains("Pt")) {
      tmp = new TH1F(TString(datatype+"_"+fHistTypeContainer[histtype_Idx].second->fType+"_"+histname),
      fHistTypeContainer[histtype_Idx].second->fType,
      6,
      ptBins);
      } else if(fHistTypeContainer[histtype_Idx].second->fType.Contains("Eta")) {
      tmp = new TH1F(TString(datatype+"_"+fHistTypeContainer[histtype_Idx].second->fType+"_"+histname),
      fHistTypeContainer[histtype_Idx].second->fType,
      11,
      etaBins);
      } */
        
    tmp = new TH1F(TString(datatype+"_"+fHistTypeContainer[histtype_Idx].second->fType+"_"+histname),
		   fHistTypeContainer[histtype_Idx].second->fType,
		   fHistTypeContainer[histtype_Idx].second->fNXBins,
		   fHistTypeContainer[histtype_Idx].second->fXLow,
		   fHistTypeContainer[histtype_Idx].second->fXHigh);
        
    tmp->GetXaxis()->SetTitle(fHistTypeContainer[histtype_Idx].second->fXTitle);
    tmp->GetYaxis()->SetTitle(fHistTypeContainer[histtype_Idx].second->fYTitle);
    tmp->Sumw2();
    h1style(*tmp);

    //Temp. Hack for custom binning for eta and pt
    //Double_t ptBins[7] = {20,35,45,60,70,80,100};
    //Double_t etaBins[13] = {-2.5,-2,-1.52,-1.37,-0.8,-0.1,0,0.1,0.8,1.37,1.52,2,2.5};
    //if(fHistTypeContainer[histtype_Idx].second->fType.Contains("Pt")) tmp = (TH1F*) tmp->Rebin(6, tmp->GetName(),ptBins);
    //if(fHistTypeContainer[histtype_Idx].second->fType.Contains("Eta"))tmp = (TH1F*) tmp->Rebin(12,tmp->GetName(),etaBins);
  }
  else {
    std::cout << "Plotter: ( ERROR ) \t Requested histtype is not 1D. Abort!" << std::endl;
  }

  return tmp;
}//;//Get1DHistogram


//Fill histogram (and dynamically add if missing, given an existing data and hist type)
//___________________________________________________________________________________
inline bool Plotter::Fill(TString datatype, TString histtype, TString histname, double xvalue, double yvalue, double weight)
{
  //Check if hist types are available
  if(int(fHistTypeContainer.size()) == 0) {
    std::cout << "Plotter: ( ERROR ) \t No hist types defined. Abort!" << std::endl;
    return false;
  }

  //--------------------------------------------------------------------------------------------------

  //Check if histtype already defined
  int histtype_Idx = -1;
  for(unsigned int i=0; i<fHistTypeContainer.size(); i++) {
    if(fHistTypeContainer[i].first == histtype) {
      histtype_Idx = i;
      break;
    }//search
  }//loop

  if(histtype_Idx<0) {
    std::cout << "Plotter: ( ERROR ) \t Requested histtype " << histtype << " for filling not found. Abort!" << std::endl;
    return false;
  }//exception
    
  //--------------------------------------------------------------------------------------------------

  //Check if histtype is 2D
  if(!fHistTypeContainer[histtype_Idx].second->is2D) {
    std::cout << "Plotter: ( ERROR ) \t Requested histtype " << histtype << " is not 2D. Abort!" << std::endl;
    return false;
  }

  //--------------------------------------------------------------------------------------------------

  //Check if datatype previously defined
  int datatype_Idx = -1;
  bool addNewDataType(false);
  if(int(fHistDataContainer.size()) != 0) {
    for(unsigned int i=0; i<fHistDataContainer.size(); i++) {
      if(fHistDataContainer[i].first == datatype) {
	datatype_Idx = i;
	break;
      }//search
    }//loop

    if(datatype_Idx<0) {
      addNewDataType = true;
    }
  }//data types available
  else {
    addNewDataType = true;
  }//no data types available

  //--------------------------------------------------------------------------------------------------

  //Add new data type branch, new histogram to branch and fill it
  TList* dataBranch = NULL;
  if(addNewDataType) {
    //Setup new branch
    dataBranch = new TList();

    //Declare new histogram
    TH2F* tmp = Get2DHistogram(datatype, histname, histtype_Idx);

    //Fill current event values
    tmp->Fill(xvalue, yvalue, weight);

    //Add histogram to branch
    dataBranch->Add(tmp);

    //Store in data container
    std::pair<TString, TList*> newPair(datatype, dataBranch);
    fHistDataContainer.push_back(newPair);

    return true;
  }

  //--------------------------------------------------------------------------------------------------

  //Fill existing histogram
  dataBranch = fHistDataContainer[datatype_Idx].second;
  TIter next(dataBranch);
  TH2 *iter;
  while( (iter=(TH2 *)next()) ) { 
    TString _histname(iter->GetName());
    if(_histname == (datatype+"_"+histtype+"_"+histname)) {
      iter->Fill(xvalue, yvalue, weight);
      return true;
    }//fill histogram
  }//search loop

  //--------------------------------------------------------------------------------------------------

  //Add and fill new histogram
  TH2F* tmp2 = Get2DHistogram(datatype, histname, histtype_Idx);
  tmp2->Fill(xvalue, yvalue, weight);
  dataBranch->Add(tmp2);
 
  return true;
}//;//Fill


//Declare and return 2D histogram
//___________________________________________________________________________________
inline TH2F* Plotter::Get2DHistogram(TString datatype, TString histname, Int_t histtype_Idx)
{
  TH2F* tmp = NULL;

  //Sanity check
  if(histtype_Idx<0) {
    std::cout << "Plotter: ( ERROR ) \t histtype_idx is negative. Abort!" << std::endl;
    return tmp;
  }

  if(fHistTypeContainer[histtype_Idx].second->is2D) {
    /*//Temp. Hack for custom binning for eta and pt
      Double_t ptBins[7] = {20,35,45,60,70,80,100};
      Double_t etaBins[12] = {-2.5,-2,-1.52,-1.37,-0.8,-0.1,0.1,0.8,1.37,1.52,2,2.5};
    
      tmp = new TH2F(TString(datatype+"_"+fHistTypeContainer[histtype_Idx].second->fType+"_"+histname),
      fHistTypeContainer[histtype_Idx].second->fType,
      6,
      ptBins,
      11,
      etaBins);
    */
    tmp = new TH2F(TString(datatype+"_"+fHistTypeContainer[histtype_Idx].second->fType+"_"+histname),
		   fHistTypeContainer[histtype_Idx].second->fType,
		   fHistTypeContainer[histtype_Idx].second->fNXBins,
		   fHistTypeContainer[histtype_Idx].second->fXLow,
		   fHistTypeContainer[histtype_Idx].second->fXHigh,
		   fHistTypeContainer[histtype_Idx].second->fNYBins,
		   fHistTypeContainer[histtype_Idx].second->fYLow,
		   fHistTypeContainer[histtype_Idx].second->fYHigh);
        
    tmp->GetXaxis()->SetTitle(fHistTypeContainer[histtype_Idx].second->fXTitle);
    tmp->GetYaxis()->SetTitle(fHistTypeContainer[histtype_Idx].second->fYTitle);
    tmp->Sumw2();
    h2style(*tmp);
  }
  else {
    std::cout << "Plotter: ( ERROR ) \t Requested histtype is not 2D. Abort!" << std::endl;
  }

  return tmp;
}//;//Get2DHistogram


//Style settings for 1D histograms
//___________________________________________________________________________________
inline void Plotter::h1style(TH1F &h)
{
  h.SetLineWidth(2);
  h.SetLineStyle(1);
  h.SetFillStyle(1001);
  h.GetXaxis()->SetNdivisions(510);
  h.GetYaxis()->SetNdivisions(510);
  h.SetMarkerStyle(1);
  h.SetMarkerColor(1);
  h.SetMarkerSize(1);
  //h.SetStats(0);
  h.SetLabelFont(62,"X");       // 42
  h.SetLabelFont(62,"Y");       // 42
  h.SetLabelOffset(0.000,"X");  // D=0.005
  h.SetLabelOffset(0.005,"Y");  // D=0.005
  h.SetLabelSize(0.055,"X");
  h.SetLabelSize(0.055,"Y");
  h.SetTitleOffset(0.8,"X");
  h.SetTitleOffset(0.9,"Y");
  h.SetTitleSize(0.06,"X");
  h.SetTitleSize(0.06,"Y");
  h.SetTitle(0);
}//; // h1style


//Style settings for 2D histograms
//___________________________________________________________________________________
inline void Plotter::h2style(TH2F &h)
{
  h.SetLineWidth(2);
  h.SetLineStyle(1);
  h.SetFillStyle(1001);
  h.GetXaxis()->SetNdivisions(510);
  h.GetYaxis()->SetNdivisions(510);
  h.SetMarkerStyle(20);
  h.SetMarkerColor(1);
  //h.SetMarkerSize(1); // looks a little too big
  //h.SetStats(0);
  h.SetLabelFont(62,"X");       // 42
  h.SetLabelFont(62,"Y");       // 42
  h.SetLabelFont(62,"Z");       // 42
  h.SetLabelOffset(0.005,"Y");  // D=0.005
  h.SetLabelOffset(0.005,"Z");  // D=0.005
  h.SetLabelSize(0.055,"X");
  h.SetLabelSize(0.055,"Y");
  h.SetLabelSize(0.055,"Z");
  h.SetTitleOffset(1.0,"X");
  h.SetTitleOffset(0.8,"Y");
  h.SetTitleOffset(0.5,"Z");
  h.SetTitleSize(0.06,"X");
  h.SetTitleSize(0.06,"Y");
  h.SetTitleSize(0.06,"Z");
  h.SetTitle(0);
}//; // h2style


//___________________________________________________________________________________
inline void Plotter::WriteOutput() 
{
  //Check if data available
  if(int(fHistDataContainer.size()) == 0) {
    std::cout << "Plotter: ( ERROR ) \t No data found to write to file. Abort!" << std::endl;
    return;
  }

  //Write in datatype directories [TDirectory]
  for(unsigned int i=0; i<fHistDataContainer.size(); i++) {
    //Make dir
    TDirectory* adir = fOutfile->mkdir(fHistDataContainer[i].first);
    adir->cd();

    //Loop over histograms in data branch
    TIter next(fHistDataContainer[i].second);
    TH1 *iter;
    std::cout << "Plotter: Writing hists ";
    while( (iter=(TH1 *)next()) ) { 
      TString savename(iter->GetName());
      //savename.ReplaceAll(fHistDataContainer[i].first+"_","");
      if(iter->GetEntries() == 0) { continue; }
      iter->Write(savename);
      std::cout << ".";
    } std::cout << "\nComplete!" << std::endl;
  }//loop

  //Write trees
  if(fTreeContainer.size()>0) {
    std::cout << "Plotter: Writing trees ";
    //Make dir
    TDirectory* adir = fOutfile->mkdir("Trees");
    adir->cd();
    
    for(unsigned int i=0; i < fTreeContainer.size(); i++) {
      //Store tree
      fTreeContainer.at(i)->fTree->Write();
      std::cout << ".";
    } std::cout << "\nComplete!" << std::endl;
  }

  //Print others
  for(unsigned int i=0; i<fVariableContainer.size(); i++) {
    std::cout << "VAR[" << fVariableContainer[i].first << "]: " 
	      << "Min=" << fVariableContainer[i].second->fMin   << " "
	      << "Max=" << fVariableContainer[i].second->fMax   << " "
	      << "Mean=" << fVariableContainer[i].second->fMean << std::endl;
  }
    
}//;


//___________________________________________________________________________________
inline void Plotter::FindRange(TString name, double value)
{
  //Update
  for(unsigned int i=0; i<fVariableContainer.size(); i++) {
    if(fVariableContainer[i].first==name) {
      if(value < fVariableContainer[i].second->fMin) fVariableContainer[i].second->fMin = value;
      if(value > fVariableContainer[i].second->fMax) fVariableContainer[i].second->fMax = value;

      fVariableContainer[i].second->fN++;
      fVariableContainer[i].second->fSum += value;
      fVariableContainer[i].second->fMean = fVariableContainer[i].second->fSum / fVariableContainer[i].second->fN;
      return;
    }
  }

  //Add new
  Variable* var = new Variable();
  std::pair<TString, Variable*> newVar;

  newVar.first = name;
  newVar.second = var;

  if(value < newVar.second->fMin) newVar.second->fMin = value;
  if(value > newVar.second->fMax) newVar.second->fMax = value;

  fVariableContainer.push_back(newVar);
}//;// FindRange

#endif //__PLOTTER_H__
