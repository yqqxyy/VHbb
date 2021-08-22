#include "EventLoop/Job.h"
#include "SampleHandler/DiskListEOS.h"
#include "SampleHandler/DiskListLocal.h"
#include "SampleHandler/SampleHandler.h"
#include "SampleHandler/ScanDir.h"
#include "SampleHandler/ToolsDiscovery.h"
#include "xAODRootAccess/Init.h"

#include "EventLoop/CondorDriver.h"
#include "EventLoop/DirectDriver.h"
#include "EventLoop/Driver.h"
#include "EventLoop/LSFDriver.h"
#include "EventLoop/SlurmDriver.h"
#include "EventLoop/TorqueDriver.h"

#include "SampleHandler/Sample.h"
#include "xAODRootAccess/tools/TFileAccessTracer.h"

#include <stdlib.h>
#include <fstream>
#include <vector>

#include <TFile.h>
#include <TPython.h>
#include <TSystem.h>

#include "CxAODTools/ConfigStore.h"

// TODO: REMOVE
#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"
//
#include "CxAODReader_VHbb/AnalysisReader_VBFHbb1Ph.h"
#include "CxAODReader_VHbb/AnalysisReader_VGamma.h"
#include "CxAODReader_VHbb/AnalysisReader_VHbb0Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHbb1Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHbb2Lep.h"

// Vhcc
#include "CxAODReader_VHbb/AnalysisReader_VHcc0Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHcc1Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHcc2Lep.h"

// VHreso
#include "CxAODReader_VHbb/AnalysisReader_VHreso0Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHreso1Lep.h"
#include "CxAODReader_VHbb/AnalysisReader_VHreso2Lep.h"

// TO RUN ON LYON BATCH
#include "EventLoop/GEDriver.h"
#include "SampleHandler/ToolsSplit.h"

void tag(SH::SampleHandler& sh, const std::string& tag);

int main(int argc, char* argv[]) {
  // Take the submit directory from the input if provided:
  std::string submitDir = "submitDir";
  std::string configPath = "data/CxAODReader_VHbb/framework-read.cfg";
  if (argc > 1) submitDir = argv[1];
  if (argc > 2) configPath = argv[2];

  // read run config
  static ConfigStore* config = ConfigStore::createStore(configPath);

  // enable failure on unchecked status codes
  bool enableFailure = false;
  config->getif<bool>("failUncheckedStatusCodes", enableFailure);
  if (enableFailure) {
    xAOD::TReturnCode::enableFailure();
    StatusCode::enableFailure();
    CP::CorrectionCode::enableFailure();
    CP::SystematicCode::enableFailure();
  }
  // Set up the job for xAOD access:
  xAOD::Init().ignore();

  // Construct the samples to run on:
  std::vector<std::string> sample_names = {"data",
                                           "data15",
                                           "ZeeB",
                                           "ZeeC",
                                           "ZeeL",
                                           "ZmumuB",
                                           "ZmumuC",
                                           "ZmumuL",
                                           "ZtautauB",
                                           "ZtautauC",
                                           "ZtautauL",
                                           "ZnunuB",
                                           "ZnunuC",
                                           "ZnunuL",
                                           "WenuB",
                                           "WenuC",
                                           "WenuL",
                                           "WmunuB",
                                           "WmunuC",
                                           "WmunuL",
                                           "WtaunuB",
                                           "WtaunuC",
                                           "WtaunuL",
                                           "ttbar",
                                           "ttbar_dilep",
                                           "singletop_t",
                                           "singletop_s",
                                           "singletop_Wt",
                                           "ttbar_PwHerwigppEG",
                                           "ttbar_PwPyEG",
                                           "ttbar_aMcAtNloHerwigppEG",
                                           "ttH",
                                           "ttH_dilep",
                                           "ttV",
                                           "ZHvv125",
                                           "WH125",
                                           "ZHll125",
                                           "ZHll125J_MINLO",
                                           "bbA",
                                           "ggA",
                                           "HVT",
                                           "HbbjjaSM125",
                                           "HbbjjaHEFT125",
                                           "NonResbbjja",
                                           "ZbbjjaEWK",
                                           "ZbbjjaQCD",
                                           "Wenu_MG",
                                           "Wmunu_MG",
                                           "Wtaunu_MG",
                                           "Wenu_Pw",
                                           "Wmunu_Pw",
                                           "Wtaunu_Pw",
                                           "Zee_MG",
                                           "Zmumu_MG",
                                           "Ztautau_MG",
                                           "Znunu_MG",
                                           "Zee_Pw",
                                           "Zmumu_Pw",
                                           "Ztautau_Pw",
                                           "Znunu_Pw",
                                           "WW",
                                           "WZ",
                                           "ZZ",
                                           "WW_improved",
                                           "WZ_improved",
                                           "ZZ_improved",
                                           "ZeeB_v22",
                                           "ZeeC_v22",
                                           "ZeeL_v22",
                                           "ZmumuB_v22",
                                           "ZmumuC_v22",
                                           "ZmumuL_v22",
                                           "ZtautauB_v22",
                                           "ZtautauC_v22",
                                           "ZtautauL_v22",
                                           "ZnunuB_v22",
                                           "ZnunuC_v22",
                                           "ZnunuL_v22",
                                           "WenuB_v22",
                                           "WenuC_v22",
                                           "WenuL_v22",
                                           "WmunuB_v22",
                                           "WmunuC_v22",
                                           "WmunuL_v22",
                                           "WtaunuB_v22",
                                           "WtaunuC_v22",
                                           "WtaunuL_v22",
                                           "HVT_WH",
                                           "HVT_0500",
                                           "HVT_0600",
                                           "HVT_0700",
                                           "HVT_0800",
                                           "HVT_0900",
                                           "HVT_1000",
                                           "HVT_1100",
                                           "HVT_1200",
                                           "HVT_1300",
                                           "HVT_1400",
                                           "HVT_1500",
                                           "HVT_1600",
                                           "HVT_1700",
                                           "HVT_1800",
                                           "HVT_1900",
                                           "HVT_2000",
                                           "HVT_2200",
                                           "HVT_2400",
                                           "HVT_2600",
                                           "HVT_2800",
                                           "HVT_3000",
                                           "HVT_3500",
                                           "HVT_4000",
                                           "HVT_4500",
                                           "HVT_5000",
                                           "bbA_1000",
                                           "bbA_1100",
                                           "bbA_1200",
                                           "bbA_1300",
                                           "bbA_1400",
                                           "bbA_1500",
                                           "bbA_1600",
                                           "bbA_1700",
                                           "bbA_1800",
                                           "bbA_1900",
                                           "bbA_200",
                                           "bbA_2000",
                                           "bbA_280",
                                           "bbA_300",
                                           "bbA_320",
                                           "bbA_340",
                                           "bbA_360",
                                           "bbA_380",
                                           "bbA_400",
                                           "bbA_420",
                                           "bbA_440",
                                           "bbA_460",
                                           "bbA_480",
                                           "bbA_500",
                                           "bbA_550",
                                           "bbA_600",
                                           "bbA_650",
                                           "bbA_700",
                                           "bbA_750",
                                           "bbA_800",
                                           "bbA_850",
                                           "bbA_900",
                                           "bbA_950",
                                           "ggA_1000",
                                           "ggA_1100",
                                           "ggA_1200",
                                           "ggA_1300",
                                           "ggA_1400",
                                           "ggA_1500",
                                           "ggA_1600",
                                           "ggA_1700",
                                           "ggA_1800",
                                           "ggA_1900",
                                           "ggA_200",
                                           "ggA_2000",
                                           "ggA_280",
                                           "ggA_300",
                                           "ggA_320",
                                           "ggA_340",
                                           "ggA_360",
                                           "ggA_380",
                                           "ggA_400",
                                           "ggA_420",
                                           "ggA_440",
                                           "ggA_460",
                                           "ggA_480",
                                           "ggA_500",
                                           "ggA_550",
                                           "ggA_600",
                                           "ggA_650",
                                           "ggA_700",
                                           "ggA_750",
                                           "ggA_800",
                                           "ggA_850",
                                           "ggA_900",
                                           "ggA_950",
                                           "SinglePhoton"};

  // Override sample list from config
  config->getif<std::vector<std::string> >("samples", sample_names);

  // Possibility to skip some of the samples above
  // Read bools from config file and if false remove sample from vector
  std::vector<std::string>::iterator itr;
  for (itr = sample_names.begin(); itr != sample_names.end();) {
    bool includeSample = true;
    config->getif<bool>(*itr, includeSample);
    if (!includeSample)
      itr = sample_names.erase(itr);
    else
      ++itr;
  }

  std::string dataset_dir = config->get<std::string>("dataset_dir");

  bool eos;
  bool isu_xrootd;
  std::string prefix = "/eos/";
  std::string xrdprefix = "/local/xrootd/a/";
  if (dataset_dir.substr(0, prefix.size()).compare(prefix) == 0) {
    eos = true;
    isu_xrootd = false;
    std::cout << "Will read datasets from EOS directory " << dataset_dir
              << std::endl;
  } else if (dataset_dir.substr(0, xrdprefix.size()).compare(xrdprefix) == 0) {
    isu_xrootd = true;
    eos = false;
    std::cout << "Will read datasets from XRootD ISU directory " << dataset_dir
              << std::endl;
  } else {
    eos = false;
    isu_xrootd = false;
    std::cout << "Will read datasets from local directory " << dataset_dir
              << std::endl;
  }

  // Query - I had to put each background in a separate directory
  // for samplehandler to have sensible sample names etc.
  // Is it possible to define a search string for directories and assign all
  // those to a sample name? SH::scanDir (sampleHandler, list); // apparently it
  // will come

  // create the sample handler
  SH::SampleHandler sampleHandler;
  std::cout << "Scanning samples:" << std::endl;
  for (unsigned int isamp(0); isamp < sample_names.size(); isamp++) {
    std::string sample_name(sample_names.at(isamp));
    std::string sample_dir(dataset_dir + "/" + sample_name);

    if (!eos && !isu_xrootd) {
      bool direxists = gSystem->OpenDirectory(sample_dir.c_str());
      if (!direxists) {
        std::cout << " No sample exists: " << sample_name
                  << " , skipping: " << sample_dir << std::endl;
        continue;
      }
    }

    std::string inFiles = "";
    config->getif<std::string>("inFiles", inFiles);
    std::string RSE = "default";
    config->getif<string>("RSE", RSE);

    // eos, local disk or afs
    if (eos) {
      // SH::DiskListEOS list(sample_dir,"root://eosatlas/"+sample_dir );
      SH::DiskListEOS list(sample_dir,
                           "root://eosatlas.cern.ch:/" + sample_dir);
      // SH::DiskListEOS list("eosatlas.cern.ch", sample_dir);
      // tuples are downloaded to same directory as CxAOD so specify CxAOD root
      // file pattern
      if (inFiles.empty()) {
        SH::ScanDir()
            .samplePattern("*CxAOD*")
            .sampleName(sample_name)
            .scan(sampleHandler, list);
        // SH::scanSingleDir (sampleHandler, sample_name, list, "*CxAOD*") ;
      } else {
        SH::ScanDir()
            .samplePattern(inFiles)
            .sampleName(sample_name)
            .scan(sampleHandler, list);
        // SH::scanSingleDir (sampleHandler, sample_name, list, inFiles) ;
      }
      // SH::scanSingleDir (sampleHandler, sample_name, list, "*outputLabel*") ;
    } else if (RSE == "ISU") {
      std::string mnt_dir(sample_dir);
      mnt_dir.erase(0, xrdprefix.size());
      SH::DiskListLocal list("/mnt/xrootd/" + mnt_dir,
                             "root://head2/" + sample_dir);
      SH::ScanDir()
          .samplePattern(inFiles)
          .sampleName(sample_name)
          .scan(sampleHandler, list);
    } else {
      // Reading files from another storage element - find out which one
      // std::string RSE = "default";
      // config->getif<string>("RSE", RSE);
      SH::DiskListLocal list("");
      if (RSE == "BNL") {
        list = SH::DiskListLocal(
            sample_dir, "root://dcxrd.usatlas.bnl.gov:1096//" + sample_dir);
      } else {
        list = SH::DiskListLocal(sample_dir);
      }
      // tuples are downloaded to same directory as CxAOD so specify CxAOD root
      // file pattern
      if (inFiles.empty()) {
        SH::ScanDir()
            .samplePattern("*CxAOD*")
            .sampleName(sample_name)
            .scan(sampleHandler, list);
        // SH::scanSingleDir (sampleHandler, sample_name, list, "*CxAOD*") ;
      } else {
        SH::ScanDir()
            .samplePattern(inFiles)
            .sampleName(sample_name)
            .scan(sampleHandler, list);
        // SH::scanSingleDir (sampleHandler, sample_name, list, inFiles) ;
      }
      // SH::scanSingleDir (sampleHandler, sample_name, list, "*outputLabel*") ;
    }

    //
    SH::Sample* sample_ptr = sampleHandler.get(sample_name);
    // sample_ptr->setMetaString("SampleID", sample_name);
    sample_ptr->meta()->setString("SampleID", sample_name);
    int nsampleFiles = sample_ptr->numFiles();

    std::cout << "Sample name " << sample_name
              << " with nfiles : " << nsampleFiles << std::endl;
  }

  // Set the name of the input TTree. It's always "CollectionTree"
  // for xAOD files.
  sampleHandler.setMetaString("nc_tree", "CollectionTree");

  // Print what we found:
  std::cout << "Printing sample handler contents:" << std::endl;
  sampleHandler.print();

  // limit file size per job (sample-wise)
  int jobSizeLimitMB = -1;
  config->getif<int>("jobSizeLimitMB", jobSizeLimitMB);
  if (jobSizeLimitMB > 0) {
    long long int jobSizeLimitByte =
        (((long long int)jobSizeLimitMB) * 1024) * 1024;
    std::cout << "Limiting file size per job (sample-wise) to "
              << jobSizeLimitMB << " MB" << std::endl;
    SH::SampleHandler::iterator it = sampleHandler.begin();
    SH::SampleHandler::iterator it_end = sampleHandler.end();
    // loop over samples
    for (; it != it_end; it++) {
      SH::Sample* sample = *it;
      std::vector<long long int> fileSizes;
      // loop over files
      unsigned int nFiles = sample->numFiles();
      for (unsigned int i = 0; i < nFiles; i++) {
        TString fileName = sample->fileName(i);
        // Depending on wether the file is located on eos or locally, the script
        // will act differently.
        if (fileName.BeginsWith("file:///")) {
          fileName.ReplaceAll("file:///", "/");
        } else if (fileName.BeginsWith("root://eosatlas.cern.ch://")) {
          fileName.ReplaceAll("root://eosatlas.cern.ch://", "/");
        } else
          continue;
        // open file with cursor at the end:
        std::ifstream fileStream(fileName.Data(),
                                 std::ios::binary | std::ios::ate);
        long long int fileBytes = fileStream.tellg();
        fileSizes.push_back(fileBytes);
      }

      // go through list of sizes and estimate maximal nFilesPerJob
      unsigned int nFilesPerJob = 0;
      bool jobsAreSmaller = true;
      while (jobsAreSmaller) {
        nFilesPerJob++;
        long long int jobSize = 0;
        unsigned int filePerJobCounter = 0;
        for (unsigned int i = 0; i < fileSizes.size(); i++) {
          jobSize += fileSizes.at(i);
          filePerJobCounter++;
          if (jobSize > jobSizeLimitByte) {
            jobsAreSmaller = false;
            break;
          }
          if (filePerJobCounter > nFilesPerJob) {
            jobSize = 0;
            filePerJobCounter = 0;
          }
        }
        if (nFilesPerJob > nFiles) {
          break;
        }
      }

      // even out nFilesPerJob
      if (nFilesPerJob >= nFiles) {
        nFilesPerJob = nFiles;
      } else if (nFilesPerJob > nFiles / 2 + 1) {
        nFilesPerJob = nFiles / 2 + 1;
      }
      // set number of files per job
      std::cout << "Sample " << sample->name() << ": nFiles = " << nFiles
                << ", nFilesPerJob = " << nFilesPerJob << std::endl;
      sample->meta()->setDouble(EL::Job::optFilesPerWorker, nFilesPerJob);
    }
  }

  // generate yield file from input CxAODs
  std::string yieldFile = "";
  bool generateYieldFile = false;
  config->getif<bool>("generateYieldFile", generateYieldFile);
  if (generateYieldFile) {
    // write file list to temporary text file
    std::ofstream file;
    file.open("fileList_temp.txt");
    SH::SampleHandler::iterator it = sampleHandler.begin();
    SH::SampleHandler::iterator it_end = sampleHandler.end();
    for (; it != it_end; it++) {
      SH::Sample* sample = *it;
      for (unsigned int i = 0; i < sample->numFiles(); i++) {
        file << sample->fileName(i) << std::endl;
      }
    }
    file.close();
    // sort file list and generate yield file
    system("sort fileList_temp.txt > fileList.txt");
    TPython::Exec("import sys");
    TPython::Exec("sys.argv=['dummy','fileList.txt']");
    TPython::LoadMacro(
        "./CxAODOperations_VHbb/scripts/count_Nentry_SumOfWeight.py");
    yieldFile = (const char*)TPython::Eval("out_file_md5");
    system("rm fileList_temp.txt fileList.txt");
  }

  // Create an EventLoop job:
  EL::Job job;
  job.sampleHandler(sampleHandler);

  // turn off sending xAOD summary access report (to be discussed)
  bool m_enableDataSubmission = true;
  config->getif<bool>("enableDataSubmission", m_enableDataSubmission);
  if (!m_enableDataSubmission) {
    xAOD::TFileAccessTracer::enableDataSubmission(false);
  }

  // remove submit dir before running the job
  // job.options()->setDouble(EL::Job::optRemoveSubmitDir, 1);

  // create algorithm, set job options, maniplate members and add our analysis
  // to the job:
  AnalysisReader* algorithm = nullptr;

  std::string analysis = "VHbb";
  config->getif<std::string>("analysis", analysis);

  std::string modelType = "CUT";
  config->getif<std::string>("modelType", modelType);

  std::string analysisType = config->get<std::string>("analysisType");
  if (analysisType == "0lep") {
    if (analysis == "VHbb") {
      if (modelType == "HVT" || modelType == "AZh") {
        algorithm = new AnalysisReader_VHreso0Lep();
      } else {
        algorithm = new AnalysisReader_VHbb0Lep();
      }
    } else if (analysis == "VHcc") {
      algorithm = new AnalysisReader_VHcc0Lep();
    } else {
      Error("hsg5frameworkReadCxAOD",
            "Bad config: analysis = %s is not supported [valid choices: "
            "VHbb|VHcc].",
            analysis.c_str());
      return 1;
    }
  } else if (analysisType == "1lep") {
    if (analysis == "VHbb") {
      if (modelType == "HVT") {
        algorithm = new AnalysisReader_VHreso1Lep();
      } else {
        algorithm = new AnalysisReader_VHbb1Lep();
      }
    } else if (analysis == "VHcc") {
      algorithm = new AnalysisReader_VHcc1Lep();
    } else {
      Error("hsg5frameworkReadCxAOD",
            "Bad config: analysis = %s is not supported [valid choices: "
            "VHbb|VHcc].",
            analysis.c_str());
      return 1;
    }
  } else if (analysisType == "2lep") {
    if (analysis == "VHbb") {
      if (modelType == "HVT" || modelType == "AZh") {
        algorithm = new AnalysisReader_VHreso2Lep();
      } else {
        algorithm = new AnalysisReader_VHbb2Lep();
      }
    } else if (analysis == "VHcc") {
      algorithm = new AnalysisReader_VHcc2Lep();
    } else {
      Error("hsg5frameworkReadCxAOD",
            "Bad config: analysis = %s is not supported [valid choices: "
            "VHbb|VHcc].",
            analysis.c_str());
      return 1;
    }
  } else if (analysisType == "vbfa") {
    algorithm = new AnalysisReader_VBFHbb1Ph();
  } else if (analysisType == "vgamma") {
    algorithm = new AnalysisReader_VGamma();
  } else {
    algorithm = new AnalysisReader_VHQQ();
  }
  algorithm->setConfig(config);
  algorithm->setSumOfWeightsFile(yieldFile);

  // limit number of events to maxEvents - set in config
  job.options()->setDouble(EL::Job::optMaxEvents,
                           config->get<int>("maxEvents"));
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EventLoop#Access_the_Data_Through_xAOD_EDM
  bool nominalOnly = false;
  config->getif<bool>("nominalOnly", nominalOnly);
  if (nominalOnly) {
    // branch access shows better performance for CxAODReader with Nominal only
    job.options()->setString(EL::Job::optXaodAccessMode,
                             EL::Job::optXaodAccessMode_branch);
  } else {
    // branch access cannot read shallow copies, so we need class access in case
    // reading systematics
    job.options()->setString(EL::Job::optXaodAccessMode,
                             EL::Job::optXaodAccessMode_class);
  }

  // add algorithm to job
  job.algsAdd(algorithm);

  // Number of files to submit per job
  int nFilesPerJob = 20;
  config->getif<int>("nFilesPerJob", nFilesPerJob);

  // Run the job using the local/direct driver:
  std::string driver = "direct";
  config->getif<std::string>("driver", driver);
  if (driver == "direct") {
    EL::DirectDriver* eldriver = new EL::DirectDriver;
    eldriver->submit(job, submitDir);
  } else if (driver == "proof") {
    Error("hsg5framework",
          "The ProofDriver is no longer supported. See ATLASG-1376.");
    return EXIT_FAILURE;
  } else if (driver == "LSF") {
    EL::LSFDriver* eldriver = new EL::LSFDriver;
    eldriver->options()->setString(EL::Job::optSubmitFlags, "-L /bin/bash");
    eldriver->shellInit =
        "export "
        "ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase && "
        "source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh";
    job.options()->setDouble(EL::Job::optFilesPerWorker, nFilesPerJob);
    std::string bQueue = "1nh";
    config->getif<std::string>("bQueue", bQueue);
    job.options()->setString(EL::Job::optSubmitFlags,
                             ("-q " + bQueue).c_str());  // 1nh 8nm
    eldriver->submitOnly(job, submitDir);
  } else if (driver == "condor") {  // Spyros: add condor driver
    EL::CondorDriver* eldriver = new EL::CondorDriver;
    eldriver->shellInit =
        "export "
        "ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase && "
        "source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh";
    job.options()->setDouble(EL::Job::optFilesPerWorker, nFilesPerJob);
    std::string condor_config = "";
    std::string condor_queue = "none";
    config->getif<std::string>("condor_queue", condor_queue);
    if (condor_queue != "none")
      condor_config.append(("+JobFlavour = \"" + condor_queue + "\"").c_str());

    std::string condor_OSPreference = "none";
    config->getif<std::string>("condor_OSPreference", condor_OSPreference);
    if (condor_OSPreference != "none") {
      condor_config.append(" \n");
      condor_config.append(
          ("requirements = (OpSysAndVer =?= \"" + condor_OSPreference + "\")")
              .c_str());
      condor_config.append(" \n");
    }

    std::string accountingGroup = "none";
    config->getif<std::string>("accountingGroup", accountingGroup);
    if (accountingGroup != "none") {
      condor_config.append(" \n");
      condor_config.append(("accounting_group = " + accountingGroup).c_str());
    }

    job.options()->setString(EL::Job::optCondorConf, condor_config);
    eldriver->submitOnly(job, submitDir);
  } else if (driver ==
             "GE") {  // grid engine - Camilla (lyon batch), Felix (MPI batch)
    std::vector<std::string> vec_submit_flags =
        config->get<std::vector<std::string> >("submitFlags");
    std::string submit_flags("");
    for (auto str : vec_submit_flags) submit_flags += str + " ";
    EL::GEDriver* eldriver = new EL::GEDriver;
    job.options()->setDouble(EL::Job::optFilesPerWorker, nFilesPerJob);
    job.options()->setString(EL::Job::optSubmitFlags, submit_flags);
    eldriver->submit(job, submitDir);
    // eldriver->submitOnly(job, submitDir);
  } else if (driver == "slurm") {
    // slurm driver (Freiburg batch)
    std::string slurm_account = config->get<std::string>("slurm_account");
    std::string slurm_partition = config->get<std::string>("slurm_partition");
    std::string slurm_runTime = config->get<std::string>("slurm_runTime");
    std::string slurm_memory = config->get<std::string>("slurm_memory");
    EL::SlurmDriver* eldriver = new EL::SlurmDriver;
    eldriver->SetJobName("ReadCxAOD");
    eldriver->SetAccount(slurm_account);
    eldriver->SetPartition(slurm_partition);
    eldriver->SetRunTime(slurm_runTime);
    eldriver->SetMemory(slurm_memory);
    eldriver->SetConstrain("");
    eldriver->submitOnly(job, submitDir);
  } else if (driver == "torque") {
    EL::TorqueDriver* eldriver = new EL::TorqueDriver;
    eldriver->shellInit =
        "export "
        "ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase && "
        "source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh";
    std::string stbc_queue = "generic";
    config->getif<std::string>("stbc_queue", stbc_queue);
    job.options()->setString(
        EL::Job::optSubmitFlags,
        ("-q " + stbc_queue).c_str());  // generic queue default - 24h walltime
    eldriver->submitOnly(job, submitDir);
  } else {
    Error("hsg5framework", "Unknown driver '%s'", driver.c_str());
    return 0;
  }
  return 0;
}
