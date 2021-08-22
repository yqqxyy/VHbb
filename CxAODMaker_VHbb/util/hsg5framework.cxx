#include "xAODRootAccess/Init.h"
#include "SampleHandler/SampleHandler.h"
#include "SampleHandler/ToolsDiscovery.h"
#include "SampleHandler/DiskListLocal.h"
#include "SampleHandler/DiskListXRD.h"
#include "SampleHandler/ScanDir.h"
#include "SampleHandler/Sample.h"
#include "EventLoop/Job.h"
#include "EventLoop/DirectDriver.h"
#include "EventLoopGrid/PrunDriver.h"

#include <iostream>
#include <fstream>
#include <TSystem.h>
#include <stdlib.h>
#include <iomanip>

#include "CxAODMaker_VHbb/AnalysisBase_VHbb.h"
#include "CxAODTools/ConfigStore.h"

#include "AsgTools/StatusCode.h"
#include "EventLoop/StatusCode.h"
#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/SystematicCode.h"
#include "xAODRootAccess/tools/TFileAccessTracer.h"

#include <getopt.h>
#include <boost/lexical_cast.hpp>

void SetMetaDatasetIdForPrwCheck(SH::SampleHandler sampleHandler, bool grid=true);

void printUsage(std::ostream & os) {
	os << "Usage: " << std::endl;
	os << "hsg5framework [ options ]" << std::endl;
	os << "   Options: " << std::endl;
	os << "    " << std::setw(24) << "-h | --help : print this text" << std::endl;
	os << "    " << std::setw(24) << "-s | --sampleIn <path> : override the sampleIn value from the config file"  << std::endl;
	os << "    " << std::setw(24) << "-d | --submitDir <path> : override the submitDir value from the config file" << std::endl;
	os << "    " << std::setw(24) << "-n | --numEvents <int> : override the maxEvents value from the config file" << std::endl;
	os << "    " << std::setw(24) << "-k | --skipEvents <int> : skip events" << std::endl;
	os << "    " << std::setw(24) << "-c | --config <file> : override the config file" << std::endl;
	os << "    " << std::setw(24) << "-o | --higgs    : configure for official production for Higgs group" << std::endl;
	os << "    " << std::setw(24) << "-e | --exotics  : configure for official production for Exotics group" << std::endl;
	os << "    " << std::setw(24) << "-b | --hdbs     : configure for official production for Hdbs group" << std::endl;
	os << "    " << std::setw(24) << "-x | --noSubmit : turn off grid submission for testing before launching grid jobs" << std::endl;
}

int main(int argc, char* argv[]) {
  // flags for command line overrides
  bool overrideInput = false;
  bool overrideSubmitDir = false;
  bool overrideEvents = false;
  bool overrideConfig = false;
  bool officialProductionHiggs   = false;
  bool officialProductionExotics = false;
  bool officialProductionHdbs    = false;
  bool overrideSubmit = false;
  // command line override values
  std::string sampleIn_override="";
  std::string submitDir_override="";
  std::string configFile_override="";
  int nEvents_override = -1;
  int skipEvents = 0;

  static struct option long_options[] = {
    {"help",       no_argument,       0, 'h'},
    {"config",     required_argument, 0, 'c'},
    {"sampleIn",   required_argument, 0, 's'},
    {"submitDir",  required_argument, 0, 'd'},
    {"numEvents",  required_argument, 0, 'n'},
    {"skipEvents", required_argument, 0, 'k'},
    {"higgs",      no_argument,       0, 'o'}, // o as h is already taken
    {"exotics",    no_argument,       0, 'e'},
    {"hdbs",       no_argument,       0, 'b'}, // b as h, o are already taken
    {"noSubmit",   no_argument,       0, 'x'},
    {0,0,0,0}
  };

  int option_index =0;
  int c = 0;
  while ( (c = getopt_long( argc, argv, "c:s:d:n:k:ohxe", long_options, &option_index))!=-1) {
    switch(c)
      {
      case 'h':
	printUsage(std::cout);
	return 0;
      case 'c':
	//std::cout << "configFile : " << optarg << std::endl;
	configFile_override = std::string(optarg);
	overrideConfig = true;
	break;
      case 's':
	//std::cout << "sampleIn : " << optarg << std::endl;
	sampleIn_override = std::string(optarg);
	overrideInput = true;
	break;
      case 'd':
	//std::cout << "submitDir : " << optarg << std::endl;
	submitDir_override = std::string(optarg);
	overrideSubmitDir = true;
	break;
      case 'n':
	//std::cout << "numEvents : " << optarg << std::endl;
	nEvents_override = boost::lexical_cast<int>(optarg);
	overrideEvents = true;
	break;
      case 'k':
	skipEvents = boost::lexical_cast<int>(optarg);
	break;
      case 'o':
	//std::cout << "Enabling official production mode for Higgs group" << std::endl;
	officialProductionHiggs   = true;
	break;
      case 'e':
	//std::cout << "Enabling official production mode for Exotics group" << std::endl;
	officialProductionExotics = true;
	break;
      case 'b':
	//std::cout << "Enabling official production mode for Hdbs group" << std::endl;
	officialProductionHdbs = true;
	break;
      case 'x':
	//std::cout << "Disabling grid submission for testing" << std::endl;
	overrideSubmit = true;
	break;
      default:
	printUsage(std::cout);
	return EXIT_FAILURE;
      }
    //std::cout << "End of option loop..."  << optarg << std::endl;
  }
  // steer file has to put here: CxAODMaker_VHbb/data/framework-run.cfg
  // RootCore will copy it to $WorkDir_DIR/data/CxAODMaker_VHbb/framework-run.cfg
  // ConfigStore::createStore() will prepend "$WorkDir_DIR/" to the path given
  std::string configPath = "data/CxAODMaker_VHbb/framework-run.cfg";
  if(overrideConfig) {
    configPath = configFile_override;
    Info("hsg5framework","Overriding configPath : %s", configPath.c_str());
  }

  ConfigStore * config = ConfigStore::createStore( configPath );
  if (!config) {
    Error("hsg5framework","Couldn't instantiate ConfigStore");
    return EXIT_FAILURE;
  }

  // enable failure on unchecked status codes
  bool enableFailure = false;
  config->getif<bool>("failUncheckedStatusCodes", enableFailure);
  if (enableFailure) {
    xAOD::TReturnCode::enableFailure();
    StatusCode::enableFailure();
    CP::CorrectionCode::enableFailure();
    CP::SystematicCode::enableFailure();
  }

  bool runCxAODMaker = true;
  config->getif<bool>("runCxAODMaker",runCxAODMaker);

  // output directory and file names (if not specified in steer file, default names are used)
  //  - output directory (submitDir)
  //  - output file name (sample_out) - in grid mode this is overridden below
  std::string submitDir  = "submitDir";
  std::string sample_out = "CxAOD";
  config->getif<std::string>("submitDir",submitDir);
  config->getif<std::string>("sample_out",sample_out);

  // overrides from command line
  if(overrideSubmitDir) {
    submitDir = submitDir_override;
    Info("hsg5framework","Overriding submitDir : %s",submitDir.c_str());
  }
  //
  // check that output directory does not exist (otherwise EventLoop will crash)
  if (!gSystem->AccessPathName(submitDir.c_str())) {
    Error("hsg5framework","Output directory already exists, please change the value of submitDir in the configuration file");
    return EXIT_FAILURE;
  }

  // input sample name
  // - local running : the path to the directory required by SampleHandler
  // - grid running  : the path to a text file containing sample names (sample_in must contain the word "grid" to activate grid running)
  std::string sample_in  = "";
  sample_in = config->get<std::string>("sample_in");
  //
  if(overrideInput) {
    sample_in = sampleIn_override;
    Info("hsg5framework","Overriding sampleIn : %s",sample_in.c_str());
  }
  //
  // for grid running - contains sample names from text file
  std::vector<std::string> sample_name_in;

  // set up the job for xAOD access:
  xAOD::Init().ignore();

  // define if interactive or grid job, taken from 'grid' existing in name of submission text file
  // see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/CxAODFramework#Grid_running
  bool grid = sample_in.find("grid") != std::string::npos;
  Info("hsg5framework","Run on grid = %s", grid ? "true" : "false");

  // instantiate SampleHandler
  SH::SampleHandler sampleHandler;

  // set input samples
  if (!grid) {
    std::string prefix = "/eos/";
    // read from local disk
    SH::DiskListLocal list(sample_in);
    if(runCxAODMaker){
      SH::ScanDir()
	.sampleName(sample_out)
	.scan(sampleHandler,list);
    } else {
      // for creating tuples from CxAOD
      SH::ScanDir()
	.samplePattern("*CxAOD.root")
	.scan(sampleHandler,list);
    }
  } else {
    // read input file
    std::ifstream infile;
    infile.open(sample_in);
    if ( infile.fail() ) {
      Error("hsg5framework","Sample list file '%s' not found!", sample_in.c_str());
      return EXIT_FAILURE;
    }
    std::string line;
    while (!infile.eof()) {
      getline(infile, line);
      // remove all spaces from line
      line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
      // skip empty lines to avoid crash
      if (line.length() == 0) continue;
      // don't run over commented lines
      if (line.find("#") != std::string::npos) continue;
      // add sample name to vector
      sample_name_in.push_back(line);
    }
    infile.close();
    // check if text file contains sample names
    if ( sample_name_in.size() == 0) {
      Error("hsg5framework","No samples specified in file '%s'!", sample_in.c_str());
      return EXIT_FAILURE;
    }
    // declare the jobs
    for (unsigned int isam = 0; isam < sample_name_in.size(); ++isam) {
      SH::scanRucio (sampleHandler, sample_name_in[isam]);
    }
  }

  // Set the name of the input TTree. It's always "CollectionTree" for xAOD files.
  sampleHandler.setMetaString("nc_tree", "CollectionTree");
  //sampleHandler.setMetaString("nc_grid_filter", "AOD.01512594._000099.pool.root.1"); // use only 1 file to run on

  // copy MC datasetid from sample/filename to sample metadata for pileup reweight checks
  SetMetaDatasetIdForPrwCheck(sampleHandler, grid);

  // print what we found:
  sampleHandler.print();

  // create an EventLoop job:
  EL::Job job;
  job.sampleHandler(sampleHandler);

  // turn off sending xAOD summary access report (to be discussed)
  bool m_enableDataSubmission=true;
  config->getif<bool>("enableDataSubmission", m_enableDataSubmission);
  if( !m_enableDataSubmission ){
    xAOD::TFileAccessTracer::enableDataSubmission(false);
  }

  // limit number of events to maxEvents - set in config
  int eventMax = -1;
  config->getif<int>("maxEvents",eventMax);
  if(overrideEvents) {
    eventMax = nEvents_override;
    Info("hsg5framework","Overriding maxEvents : %i",eventMax);
  }

  // run nominal only for "alternative" sample lists
  if (grid && sample_in.find("alternative") != std::string::npos) {
    Info("hsg5framework", "Running nominal only for 'alternative' sample.");
    config -> put<bool>("nominalOnly", true, true);
  }

  job.options()->setDouble (EL::Job::optMaxEvents, eventMax);
  if( skipEvents > 0) {
    job.options()->setDouble( EL::Job::optSkipEvents, skipEvents);
  }
  // Set the xAOD access mode of the job:
  job.options()->setString( EL::Job::optXaodAccessMode, EL::Job::optXaodAccessMode_class );

  // setup CxAODMaker
  if(runCxAODMaker){
   AnalysisBase_VHbb* algorithm = new AnalysisBase_VHbb();
   algorithm->m_maxEvent = static_cast<int>(job.options()->castDouble(EL::Job::optMaxEvents));
   algorithm->setConfig(config);
   job.algsAdd(algorithm);
  }

  // run the job using the direct driver (local) or the prun driver (grid)
  if (!grid) {
    std::string driver = "direct";
    config->getif<std::string>("driver", driver);
    if (driver == "direct"){
      EL::DirectDriver*  eldriver = new EL::DirectDriver;
      eldriver->submit(job, submitDir);
    } else if (driver == "proof"){
      Error("hsg5framework", "The ProofDriver is no longer supported. See ATLASG-1376.");
      return EXIT_FAILURE;
    } else {
      Error("hsg5framework", "Unknown driver '%s'", driver.c_str());
      return EXIT_FAILURE;
    }
  } else {
    EL::PrunDriver driver;
    // determine derivation name
    // default value if it cannot be determined below
    std::string derivation = "derivation";
    if      (sample_name_in[0].find("HIGG5D1") != std::string::npos) derivation = "HIGG5D1";
    else if (sample_name_in[0].find("HIGG5D2") != std::string::npos) derivation = "HIGG5D2";
    else if (sample_name_in[0].find("HIGG2D4") != std::string::npos) derivation = "HIGG2D4";
    bool autoDetermineSelection = false;
    config->getif<bool>("autoDetermineSelection",autoDetermineSelection);
    if (!autoDetermineSelection && derivation == "derivation") {
      std::string selectionName = "";
      config->getif<std::string>("selectionName",selectionName);
      if      (selectionName == "0lep") derivation = "HIGG5D1";
      else if (selectionName == "1lep") derivation = "HIGG5D2";
      else if (selectionName == "2lep") derivation = "HIGG2D4";
    }
    // form needed names
    // default in case it is not specified in steer file
    std::string vtag = "vtag";
    config->getif<std::string>("vtag",vtag);

    bool haveOfficialSample = false;
    bool haveGroupSample = false;
    for (std::string name : sample_name_in ) {
      if (name.find("group.") != std::string::npos ||
          name.find("user.") != std::string::npos) {
        haveGroupSample = true;
      } else {
        haveOfficialSample = true;
      }
    }

    if (haveGroupSample && haveOfficialSample) {
      Error("hsg5framework","Found official and group production samples in input files! Cannot determine output sample names.");
      return EXIT_FAILURE;
    }

    // position of tag "mc16_13TeV" or "data17_13TeV" or similar in input sample
    // 1 = beginning (for official production)
    // 3 = for group/user production
    int tag_offset = 1;
    if (haveGroupSample) {
      tag_offset = 3;
    }

    // output dataset name definition
    std::ostringstream ostr_prefix; // user or group + type of data or MC + DSID
    if(officialProductionHiggs) { // use Higgs group name for official production
      Info("hsg5framework","Configuring for official production with Higgs production role");
      ostr_prefix << "group.phys-higgs";
    } else if(officialProductionExotics) { // use Exotics group name for official production
      Info("hsg5framework","Configuring for official production with Exotics production role");
      ostr_prefix << "group.phys-exotics";
    } else if(officialProductionHdbs) { // use Hdbs group name for official production
      Info("hsg5framework","Configuring for official production with Hdbs production role");
      ostr_prefix << "group.phys-hdbs";
    } else { // use user name for non-official production
      Info("hsg5framework","Configuring for regular user production");
      ostr_prefix << "user.%nickname%";
    }
    //add the rest of dataset name
    ostr_prefix << ".%in:name["
		<< tag_offset + 0 << "]%.%in:name["
		<< tag_offset + 1 << "]%.";
    std::string prefix = ostr_prefix.str();

    // loop on all input samples
    for (unsigned int isam = 0; isam < sample_name_in.size(); ++isam) {
       std::string inpDS = sample_name_in[isam];
       int count_dots = 0;
       std::string generator = "";
       std::string ami_tags = "";
       for(unsigned int i = 0 ; i < inpDS.length(); i++) {
         if ( inpDS[i]=='.' ) count_dots++;
	 // keep generator part of the name
	 else if (count_dots==(tag_offset+1) ) generator += inpDS[i];
         // keep the tags
         else if (count_dots>(tag_offset+3) ) {
	   ami_tags += inpDS[i];
         }
       }
       std::string outDS=prefix+"CAOD_"+derivation+"."+ami_tags+"."+vtag;
       sampleHandler.setMetaString (".*"+generator+".*"+ami_tags+".*","nc_outputSampleName",outDS);
       std::cout<<"inpDS="<<inpDS<<std::endl;
       std::cout<<"outDS="<<outDS<<std::endl;
    } // done loop over input samples

    //submit flags
    std::string stringOptSubmitFlags=" --cmtConfig=x86_64-slc6-gcc62-opt";
    if (officialProductionHiggs || officialProductionExotics || officialProductionHdbs ) { // for official production - set the official command line option
      stringOptSubmitFlags+=" --official";
    }
    driver.options()->setString(EL::Job::optSubmitFlags, stringOptSubmitFlags.c_str());
    // other options
    double nFilesPerJob = -1.;
    config->getif<double>("nFilesPerJob",nFilesPerJob);
    if (nFilesPerJob > 0) driver.options()->setDouble("nc_nFilesPerJob", nFilesPerJob);
    //
    double nGBPerJob = -1.;
    config->getif<double>("nGBPerJob",nGBPerJob);
    if (nGBPerJob > 0) {
      if (nGBPerJob == 1000.) driver.options()->setString("nc_nGBPerJob", "MAX");
      else driver.options()->setDouble("nc_nGBPerJob", nGBPerJob);
    }
    driver.options()->setDouble("nc_nGBPerMergeJob", 1.5);
    //
    std::string site = "";
    config->getif<std::string>("site",site);
    if (!site.empty() && site != "all") driver.options()->setString("nc_site", site);
    //
    std::string excludedSite = "none";
    config->getif<std::string>("excludedSite",excludedSite);
    if (excludedSite != "none") driver.options()->setString("nc_excludedSite", excludedSite);
    //
    std::string destSE = "none";
    config->getif<std::string>("destSE",destSE);
    if (destSE!="none") driver.options()->setString("nc_destSE", destSE);
    //
    bool useNewCode = false;
    config->getif<bool>("useNewCode",useNewCode);
    if (useNewCode) driver.options()->setDouble("nc_useNewCode",1);
    //
    bool allowTaskDuplication =false;
    config->getif<bool>("allowTaskDuplication",allowTaskDuplication);
    driver.options()->setBool("nc_allowTaskDuplication", allowTaskDuplication);
    //
    bool submit = false;
    if(overrideSubmit) {
      Info("hsg5framework","Grid submission disabled from command line");
    }
    config->getif<bool>("submit",submit);
    if (!submit || overrideSubmit) { // don't submit the job if the config file says not to - or if overridden from the command line
      driver.options()->setDouble("nc_noSubmit", 1);
      driver.options()->setDouble("nc_showCmd", 1);
    }
    driver.options()->setString("nc_mergeOutput", "true"); // turn on/off the merging on the grid of CxAOD output
    // run
    // driver.submit(job, submitDir); // with monitoring
    driver.submitOnly(job, submitDir); // no monitoring
  }
  return 0;
}

void SetMetaDatasetIdForPrwCheck(SH::SampleHandler sampleHandler, bool grid) {

  // set "datasetid" in SampleHandler sample's metadata for pileup reweight checks
  // The array of MCCampaigns is for mc16_13TeV - more can be added later
  // For data or in case of being unable to work out datasetid the metadata is set to -1.
  for (SH::SampleHandler::iterator iter = sampleHandler.begin(); iter != sampleHandler.end(); ++ iter) {
    SH::Sample *sample = *iter;
    TString datasetid_str("None");
    bool foundDsid=false;
    // for the grid the samples are the input datasets - just iterate once per sample
    unsigned int nFiles = (grid) ? 1 : sample->numFiles();

    for (unsigned int i = 0; !foundDsid && i < nFiles; i++) {
      TString fileName = (grid) ? sample->name() : sample->fileName(i);
      std::cout << "filename is " << fileName << std::endl;
      std::vector<TString> campaigns={"mc16_13TeV."};
      //      campaigns.push_back("mc16_13TeV.");
      for (auto campaign : campaigns) {
	short campaignInd=fileName.Index(campaign);
	std::cout << "campaign " << campaign << " " << campaignInd << std::endl;
	if (campaignInd>-1) {
	  // dataset id length 6 from format mcXX_YYTeV.123456.
	  TSubString datasetID=fileName(campaignInd+campaign.Length(),6);
	  datasetid_str=datasetID;
	  foundDsid=true;
	}
      }
      int datasetid = (datasetid_str != "None")? datasetid_str.Atoi() : -1;
      Info("hsg5framework","Setting datasetid for pileupreweight(used for MC only) : %i",datasetid);
      sample->meta()->setInteger("datasetid",datasetid);
    } // "files" - for grid is just 1 loop
  } // samples
}
