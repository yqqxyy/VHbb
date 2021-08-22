#ifndef limitFileCheckClass_h
#define limitFileCheckClass_h
#include <fstream>
#include <iostream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TColor.h>
#include <TGaxis.h>
#include <TROOT.h>
#include <TKey.h>
#include <TClass.h>
#include <algorithm>
#include <functional>
#include <array>

double m_scale = 100; // scaling for systematic shifts (100 to have all in %)
// thresholds for flag printing
double m_maxYieldDev = 0.05; // maximum yield deviation (0.1 = 10%)
double m_maxSysNormDev = 0.2; // maximum deviation in normalization shift from systematics
double m_treshSmall = 0.005 * m_scale; // maximum normalization shift for being 'small' (0.1 * m_scale = 10%)
double m_treshZero = 1e-9 * m_scale; // maximum normalization shift for being 'zero' (0.1 * m_scale = 10%)
double m_treshShapeSmall = 0.001 * m_scale; // maximum shape shift for being 'small' (0.1 * m_scale = 10%)
double m_treshNormAsym = 0.05; // minimum difference in normalization shift for being 'asymmetric' (0.1 = 10%)
double m_treshShapeAsym = 0.001; // maximum chi2 in shape comparison for being 'shape asymmetric'

using namespace std;

class SysParam {
public:
  string name;
  double shiftSysUp;
  double shiftSysDo;
  double intSysCrossUp;
  double intSysCrossDo;
  double shapeChi2Ndf;
  TH1* h_nominal;
  TH1* h_sysUp;
  TH1* h_sysDo;
  TH1* h_sysUpNorm;
  TH1* h_sysDoNorm;

private:
  
public:
  SysParam() {
  }

  bool flagBothSmall() {
    return (fabs(shiftSysUp) < m_treshSmall && fabs(shiftSysDo) < m_treshSmall);
  }

  bool flagAnyZero() {
    return (fabs(shiftSysUp) < m_treshZero || fabs(shiftSysDo) < m_treshZero);
  }

  bool flagSameSign() {
    return (shiftSysUp * shiftSysDo > 0);
  }

  bool flagShapeSmall() {
    return (intSysCrossUp < m_treshShapeSmall && intSysCrossDo < m_treshShapeSmall);
  }

  bool flagNormAsym() {
    return (fabs(shiftSysUp / shiftSysDo + 1) > m_treshNormAsym);
  }

  bool flagShapeAsym() {
    return (shapeChi2Ndf > m_treshShapeAsym);
  }
};

class Yield {
public:
  string sample;
  double integral;
  double err;

  Yield() {
  }
};

class limitFileCheckClass {

  // Tool for comparing two VH limit input files. Run with:
  // root -b limitFileCheck.C+
  
  // To run on CxAODReader output, set m_isRun2 to true.
  //In general need to specify reference and test files, sample to run systematic on, names for lables, etc. below.

  
public:
  limitFileCheckClass();
  ~limitFileCheckClass();

  void setStyleAtlas();
  bool createOutputDir();
  void Run(string fileNameRef, string fileNameTest, string refName, string testName, string sample="data");
  void setAllRegions(bool allregions) {m_allRegions=allregions;} 
  void setRegions( vector<string> regions) {m_regions=regions;}
  
 private:
  void setEmptyBinsToOne(TH1* ratio);
  TH1* getHisto(TDirectory* source, string histoName, string subFolder = "", string systematic = "", string direction = "");
  bool pushUnique(vector<string> &vec, string &str);
  vector<string> getListOfSystematics(TDirectory* source, string subFolder = "");
  vector<string> getListOfRegions(TDirectory* source, string subFolder = "", string sample = "data");
  vector<string> getListOfSamples(TDirectory* source, string subFolder = "");
  void scaleAndShiftHist(TH1* hist, double delta, double minHist);
  double integralAndSysToShift(TH1* h_sys, TH1* h_nominal, bool normalize, double scale);
  double computeChi2Ndf(TH1* hist1, TH1* hist2);
  double getShapeChi2Ndf(TH1* h_sysUpNorm, TH1* h_sysDoNorm);
  string formatString(double number);
  void printFlag(string flag, bool print);
  TFile* getFile(string fileName);
  bool sampleAvailable(string fileName, string sample, string region, string subFolder);
  bool sampleZero(string fileName, string sample, string region, string subFolder);
  bool sampleLowStats(string fileName, string sample, string region, string subFolder);
  vector<Yield> readYields(string fileName, vector<string> samples, string region, string subFolder);
  vector<SysParam*> readSysFromLimitFile(string fileName, string sample, string region, string subFolder);
  vector<SysParam> readSysFromLogFile(string fileName, string sample, string region);
  void printSysParam(vector<SysParam*> sysParams);
  int findSys(string name, vector<SysParam*> sysParams);
  void checkForMissingSys(vector<SysParam*> sysParamsTest, vector<SysParam*> sysParamsRef);
  bool getRatioFlag(double ratio, double maxDev);
  void printCompare(double test, double ref, bool print);
  void compareYields(vector<Yield> yieldsTest, vector<Yield> yieldsRef);
  void compareSys(vector<SysParam*> sysParamsTest, vector<SysParam*> sysParamsRef);
  void drawHistograms(vector<TH1*> histos, bool drawErrorBars, double limitYrange = -1);  
  void plotSys(vector<SysParam*> sysParams, string fileTag);
  bool testGroup(string group, TString sysName);
  void plotGroupSys(vector<SysParam*> sysParamsAll, string fileTag);
  void plotCompareSys(vector<SysParam*> sysParamsTest, vector<SysParam*> sysParamsRef);
  
  // general configuration
  bool m_isRun2 = true;  //the naming conventions changed slightly this flag should take care to catch the differences
  bool m_allRegions = true; // loop on all regions found in input file
  bool m_nominalOnly = false; //produces only plots for nominal if m_drawCompare is set to true
  bool m_printToLogFile = true; // printing systematic tables to log files (WARNING: uses same directory as input files)
  bool m_drawSingle = true; // ploting and saving all systematics for each input file
  bool m_drawGroup = false; // ploting and saving all systematics in groups for each input file
  bool m_drawCompare = true; // ploting and saving all systematics comparing both input files
  string m_plotDir = "./plots/"; // directory for saving plots - will be automatically created
  string m_graphicsExt = ".eps"; // plot file format
  double m_tsize = 0.05; // text size for plots. 0.05 for single, 0.07 for compare
  double m_limitXup = 250; // if (m_limitXup > 0) set x-axis range to (0, m_limitXup)

  // regions
  vector<string> m_regions;
  
  // for printout
  int m_width = 10; // column width
  string m_testName = "test"; // table headers 
  string m_refName = "ref";
  bool m_quiet = false; // less printingin
  
  // file reading
  //int m_rebin = 25; // rebin for all histograms
  int m_rebin = 5;// if Run2
    
  // for reading norm files from WSMaker
  bool m_normFile = false;

  TFile* m_file = 0;
  string m_prevFileName = "";

};
#endif
