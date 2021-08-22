//
// For comparing fit input histogram files 
//   -put all the analysis dependent stuff in here and
//   the library is in macros/limitFileCheckClass.C
//
#include "limitFileCheckClass.C"
limitFileCheckClass* limitfilecheck;

void run0Lep();
void run1Lep();
void run2Lep();

//
void runLimitFileCheck() {

  //  gInterpreter->AddIncludePath("macros/");
  
  
  bool do0Lep=false;
  bool do1Lep=false;
  bool do2Lep=true;

  limitfilecheck = new limitFileCheckClass();  
  
  if(do0Lep) run0Lep();
  if(do1Lep) run1Lep();
  if(do2Lep) run2Lep();
}


void run0Lep() {

  string fileNameRef="";
  string refName="ref";
  string fileNameTest="";
  string testName="test";

  string sample("data"); //define the sample to check  
  vector<string> regions;
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  limitfilecheck->setRegions(regions);

  limitfilecheck->setAllRegions(false);
  
  limitfilecheck->Run(fileNameRef, fileNameTest, refName, testName, sample);
  
  
}

void run1Lep() {

  string fileNameRef="";
  string refName="ref";
  string fileNameTest="";
  string testName="test";
  
  string sample("data"); // sample to check
  vector<string> regions;
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");
  regions.push_back("");

  limitfilecheck->setRegions(regions);
  
  // choose regions or all regions
  limitfilecheck->setAllRegions(false);
  
  limitfilecheck->Run(fileNameRef, fileNameTest, refName, testName, sample);
  
  
}


void run2Lep() {
  
  string fileNameRef="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/BoostedVHbb2019/TwoLep/32-10-SysTest/LimitHistograms.VH.llbb.13TeV.mc16e.UIOWAUSTC.SysTest.root";
  string refName="ref";
  string fileNameTest="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/BoostedVHbb2019/TwoLep/32-10-SysTest/LimitHistograms.VH.llbb.13TeV.mc16e.UIOWAUSTC.SysTest.root";
  string testName="test";


  string sample("qqZllH125"); // sample to check
  // define regions by hand or select all regions true below
  vector<string> regions;
  regions.push_back("2tag1pfat0pjet_250_400ptv_SR_noaddbjetsr_mva");
  regions.push_back("2tag1pfat0pjet_400ptv_SR_noaddbjetsr_mva");


  limitfilecheck->setRegions(regions);
  
  // choose regions or all regions
  limitfilecheck->setAllRegions(false);
  
  limitfilecheck->Run(fileNameRef, fileNameTest, refName, testName, sample);
  
  
}
  

