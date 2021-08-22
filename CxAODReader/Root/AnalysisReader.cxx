#include <EventLoop/Job.h>

#include <CxAODReader/AnalysisReader.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include "EventLoop/OutputStream.h"

#include "xAODBTagging/BTagging.h"
#include "xAODEventInfo/EventInfo.h"

#include "CxAODTools/DummyEvtSelection.h"
#include "CxAODTools/TriggerTool.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TSystem.h"

//Regular expression include
#include <regex>

#include <SampleHandler/DiskOutputXRD.h>

#include "xAODRootAccess/tools/TFileAccessTracer.h"

// this is needed to distribute the algorithm to the workers
ClassImp(AnalysisReader)

    AnalysisReader::AnalysisReader()
    : m_eventInfoReader(nullptr),
      m_METReader(nullptr),
      m_MPTReader(nullptr),
      m_truthMETReader(nullptr),
      m_electronReader(nullptr),
      m_forwardElectronReader(nullptr),
      m_photonReader(nullptr),
      m_muonReader(nullptr),
      m_tauReader(nullptr),
      m_ditauReader(nullptr),
      m_jetReader(nullptr),
      m_fatJetReader(nullptr),
      m_fatJetAltReader(nullptr),
      m_trackJetReader(nullptr),
      m_subJetReader(nullptr),
      m_truthEventReader(nullptr),
      m_truthParticleReader(nullptr),
      m_truthTauReader(nullptr),
      m_truthMuonReader(nullptr),
      m_truthElectronReader(nullptr),
      m_truthNeutrinoReader(nullptr),
      m_truthWZJetReader(nullptr),
      m_truthWZFatJetReader(nullptr),
      m_mmcReader(nullptr),
      m_eventInfo(nullptr),
      m_mmc(nullptr),
      m_electrons(nullptr),
      m_forwardElectrons(nullptr),
      m_photons(nullptr),
      m_muons(nullptr),
      m_taus(nullptr),
      m_ditaus(nullptr),
      m_jets(nullptr),
      m_fatJets(nullptr),
      m_fatJetsAlt(nullptr),
      m_trackJets(nullptr),
      m_subJets(nullptr),
      m_metCont(nullptr),
      m_met(nullptr),
      m_met_soft(nullptr),
      m_mptCont(nullptr),
      m_mpt(nullptr),
      m_truthMET(nullptr),
      m_truthParts(nullptr),
      m_truthMuons(nullptr),
      m_truthTaus(nullptr),
      m_truthElectrons(nullptr),
      m_truthNeutrinos(nullptr),
      m_truthWZJets(nullptr),
      m_truthWZFatJets(nullptr),
      m_bTagTool(nullptr),
      m_EvtWeightVar(nullptr),
      m_eventCounter(0),
      m_eventCountPassed(0),
      m_isMC(false),
      m_period("a"),
      m_mcChannel(-999),
      m_mcChannelFromInputFile(-999),
      m_qgtruthmatch(false),
      m_qg_truth_map_status(0),
      m_weight(1.),
      m_maxEvents(-1),
      m_luminosity(-999),
      m_isSherpaVJets(false),
      m_isSherpaPt0WJets(false),
      m_isSherpaPt0ZJets(false),
      m_isSherpa(false),
      m_lowerShMCWeightThreshold(false),
      m_isHighWeight(false),
      m_randomRunNumber(-1),
      m_pileupReweight(-999.0),
      m_averageMu(-999.0),
      m_actualMu(-999.0),
      m_averageMuScaled(-999.0),
      m_actualMuScaled(-999.0),
      m_config(nullptr),
      m_debug(false),
      m_validation(false),
      m_applyEventPreSelection(false),
      m_allowORwithTruthSelection(false),
      m_applySherpaTruthPtCut(false),
      m_applyPowhegTruthMttCut(false),
      m_usePowhegInclFraction(-1),
      m_computePileupReweight(false),
      m_recomputePileupReweight(false),
      m_CxAODTag("CxAODTag31"),
      m_doICHEP(false),
      m_checkEventDuplicates(false),
      m_failEventDuplicates(false),
      m_putAllSysInOneDir(false),
      m_recomputeMuTrigSF(false),
      m_doQCD(false),
      m_doFakeFactor(false),
      m_applyVZbbWeight(""),
      m_doCombMass(false),      //combMass
      m_doCombMassMuon(false),  //combMass
      m_sumOfWeightsFile(""),
      m_histSvc(nullptr),
      m_truthHistSvc(nullptr),
      m_histNameSvc(nullptr),
      m_truthHistNameSvc(nullptr),
      m_eventSelection(nullptr),
      m_eventPostSelection(nullptr),
      m_xSectionProvider(nullptr),
      m_sumOfWeightsProvider(nullptr),
      m_triggerTool(nullptr),
      m_puReweightingTool(nullptr),
      m_PtRecoTool(nullptr),
      m_overlapRegAcc(nullptr),
      m_doLargeMCEventWeightsRemoval(false),
      m_evtWeightVarMode(-1) {
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}

EL::StatusCode AnalysisReader::setupJob(EL::Job &job) {
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  if (m_debug) Info("setupJob()", "Setting up job.");

  job.useXAOD();

  // check if ConfigStore is set
  if (!m_config) {
    Error("setupJob()",
          "ConfigStore not setup! Remember to set it via setConfig()!");
    return EL::StatusCode::FAILURE;
  }

  bool writeMVATree = false;
  bool writeEasyTree = false;
  string m_eosPATH = "none";
  m_config->getif<std::string>("eosFolderUser", m_eosPATH);

  EL::OutputStream out("MVATree");
  m_config->getif<bool>("writeMVATree", writeMVATree);
  m_config->getif<bool>("writeEasyTree", writeEasyTree);

  if (writeMVATree || writeEasyTree) {
    if (m_eosPATH != "none") {
      out.output(new SH::DiskOutputXRD("root://eosuser.cern.ch/" + m_eosPATH));
    }
    job.outputAdd(out);
  }

  bool writeOSTree = false;
  EL::OutputStream outOS("OSTree");
  m_config->getif<bool>("writeOSTree", writeOSTree);
  if (writeOSTree) {
    if (m_eosPATH != "none") {
      outOS.output(
          new SH::DiskOutputXRD("root://eosuser.cern.ch/" + m_eosPATH));
    }
    job.outputAdd(outOS);
  }
  // let's initialize the algorithm to use the xAODRootAccess package
  xAOD::Init("MyxAODAnalysis").ignore();  // call before opening first file

  return EL::StatusCode::SUCCESS;
}  // setupJob

EL::StatusCode AnalysisReader::histInitialize() {
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  Info("histInitialize()", "Initializing histograms.");

  // histogram manager
  m_histSvc = new HistSvc();
  m_histNameSvc = new HistNameSvc();
  m_histSvc->SetNameSvc(m_histNameSvc);
  m_histSvc->SetWeightSysts(&m_weightSysts);
  m_histSvc->SetWeightSystsCS(&m_weightSystsCS);
  m_truthHistSvc = new HistSvc();
  m_truthHistNameSvc = new HistNameSvc();
  m_truthHistSvc->SetNameSvc(m_truthHistNameSvc);
  m_truthHistSvc->SetWeightSysts(&m_weightSysts);
  m_truthHistSvc->SetWeightSystsCS(&m_weightSystsCS);

  bool fillHists = true;
  m_config->getif<bool>("writeHistograms", fillHists);
  m_histSvc->SetFillHists(fillHists);
  m_truthHistSvc->SetFillHists(fillHists);

  m_config->getif<bool>("doICHEP", m_doICHEP);

  return EL::StatusCode::SUCCESS;
}  // histInitialize

EL::StatusCode AnalysisReader::fileExecute() {
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  // std::cout << "fileExecute Input file is " << wk()->inputFile()->GetName() << std::endl;
  //QG Tagging Calibration Truth Match Files need to switch
  TString filename = wk()->inputFile()->GetName();
  filename = filename(filename.Last('/') + 1, filename.Length() + 1);
  std::cout << filename << std::endl;
  filename = filename(filename.First('.') + 1, filename.Length() + 1);
  filename = filename(filename.First('.') + 1, filename.Length() + 1);
  filename = filename(0, filename.First('C'));

  //Calculate DSID bc sometimes m_mcChannelFromInputFile not filled

  TString filename2 = wk()->inputFile()->GetName();
  TString temp(filename2);
  temp = temp(0, temp.Last('/'));
  std::cout << temp << std::endl;
  temp = temp(temp.Last('/') + 1, temp.Length() + 1);
  std::cout << temp << std::endl;
  temp = temp(temp.First('.') + 1, temp.Length() + 1);
  std::cout << temp << std::endl;
  temp = temp(temp.First('.') + 1, temp.Length() + 1);
  std::cout << temp << std::endl;
  temp = temp(temp.First('.') + 1, temp.Length() + 1);

  std::cout << temp << std::endl;

  temp = temp(0, temp.First('.'));
  std::cout << temp << std::endl;
  int mydsid = temp.Atoi();

  m_qgtruthmatch = false;
  m_config->getif<bool>("qgtruthmatch", m_qgtruthmatch);
  std::cout << "QG TRUTH MATCH IS: " << m_qgtruthmatch << std::endl;
  if (m_qgtruthmatch && m_isMC) {
    string m_qgtruthfile = "DNE";
    m_config->getif<string>("qgtruthfile", m_qgtruthfile);
    //   std::string this_dsid_truth_file = m_qgtruthfile + "truth_" + to_string(m_mcChannelFromInputFile) +"_" +  filename.Data() + ".root";
    std::string this_dsid_truth_file = m_qgtruthfile + "truth_" +
                                       std::to_string(mydsid) + "_" +
                                       filename.Data() + "root";
    std::cout << "QG TRUTH FILE: " << this_dsid_truth_file << std::endl;
    m_qg_truth_map_status = computeQGTruthMap(this_dsid_truth_file);
  }

  return EL::StatusCode::SUCCESS;
}  // fileExecute

int AnalysisReader ::computeQGTruthMap(std::string inputFile) {
  TFile *m_f;
  TTree *m_truthtree;
  m_f = new TFile(inputFile.c_str());

  //Check if QG Truth File Valid
  if (m_f->IsZombie()) {
    std::cout << "QG Calib: truth file dne, weights set to 1" << std::endl;
    return -10;
  }
  if (!m_f->GetListOfKeys()->Contains("nominal")) {
    std::cout << "QG Calib: nominal tree dne, weights set to 1" << std::endl;
    return -10;
  }

  TTree *testtree;
  testtree = (TTree *)m_f->Get("nominal");
  TBranch *br1 =
      (TBranch *)testtree->GetListOfBranches()->FindObject("eventNumber");
  TBranch *br2 =
      (TBranch *)testtree->GetListOfBranches()->FindObject("mcChannelNumber");
  TBranch *br3 = (TBranch *)testtree->GetListOfBranches()->FindObject(
      "truthJet_truthpdgid");
  TBranch *br4 = (TBranch *)testtree->GetListOfBranches()->FindObject(
      "truthJet_nChargedConstituents");
  TBranch *br5 =
      (TBranch *)testtree->GetListOfBranches()->FindObject("truthJet_pt");
  TBranch *br6 =
      (TBranch *)testtree->GetListOfBranches()->FindObject("truthJet_eta");
  TBranch *br7 =
      (TBranch *)testtree->GetListOfBranches()->FindObject("truthJet_phi");
  if (!br1 || !br2 || !br3 || !br4 || !br5 || !br6 || !br7) {
    std::cout << "QG Calib: branch(s) dne, weights set to 1" << std::endl;
    return -10;
  }

  //Create QG Truth Map
  m_truthtree = (TTree *)m_f->Get("nominal");
  int mcChannelNumber;
  Long64_t eventNumber;
  std::vector<int> *truthJet_truthpdgid = 0;
  std::vector<int> *truthJet_nChargedConstituents = 0;
  std::vector<float> *truthJet_pt = 0;
  std::vector<float> *truthJet_phi = 0;
  std::vector<float> *truthJet_eta = 0;
  //std::vector<float> *truthJet_E = 0;
  m_truthtree->SetBranchAddress("eventNumber", &eventNumber);
  m_truthtree->SetBranchAddress("mcChannelNumber", &mcChannelNumber);
  m_truthtree->SetBranchAddress("truthJet_truthpdgid", &truthJet_truthpdgid);
  m_truthtree->SetBranchAddress("truthJet_nChargedConstituents",
                                &truthJet_nChargedConstituents);
  m_truthtree->SetBranchAddress("truthJet_pt", &truthJet_pt);
  m_truthtree->SetBranchAddress("truthJet_eta", &truthJet_eta);
  m_truthtree->SetBranchAddress("truthJet_phi", &truthJet_phi);
  //  m_truthtree->SetBranchAddress("truthJet_E", &truthJet_E);

  for (int i = 0; i < m_truthtree->GetEntriesFast();
       i++) {  //loop over entries in truth tree
    m_truthtree->GetEntry(i);
    int dsid = mcChannelNumber;
    int runno = eventNumber;
    for (unsigned int j = 0; j < truthJet_pt->size(); j++) {
      TLorentzVector truthjet;
      truthjet.SetPtEtaPhiE((*truthJet_pt)[j], (*truthJet_eta)[j],
                            (*truthJet_phi)[j], 0);
      m_truthjetsmap[dsid][runno][(*truthJet_truthpdgid)[j]] =
          make_pair(truthjet, (*truthJet_nChargedConstituents)[j]);
    }  //end truth jet loop
  }    //end loop over entries in truth tree
       // m_qg_truth_map_status = 1;
  return 1;
}

EL::StatusCode AnalysisReader::changeInput(bool /*firstFile*/) {
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.

  // It seems PROOF-lite needs the file initialization here

  TFile *inputfile = wk()->inputFile();
  TString filename(inputfile->GetName());

  Info("changeInput()", "Processing file '%s'", filename.Data());

  // retrieve the DSID, i.e. from the file below retrieve 366019
  //file:///data06/abuzatu/data/CxAOD/ToUseInReader/HIGG5D1_13TeV/CxAOD_32-07_e/ZnunuC_Sh221_PTV/group.phys-higgs.mc16_13TeV.366019.CAOD_HIGG5D1.e7033_s3126_r10724_p3717.32-07-P-A_CxAOD.root/group.phys-higgs.16852182._000001.CxAOD.root
  Info("changeInput()", "From fileName retrieve the DSID");
  TString temp(filename);
  Info("changeInput()", "tempDSID=%s", temp.Data());
  //find last / and keep from beginning until just before /
  temp = temp(0, temp.Last('/'));
  Info("changeInput()", "tempDSID=%s", temp.Data());
  // find the last / and keep from just after it to the end
  temp = temp(temp.Last('/') + 1, temp.Length() + 1);
  //temp3=group.phys-higgs.mc16_13TeV.366019.CAOD_HIGG5D1.e7033_s3126_r10724_p3717.32-07-P-A_CxAOD.root
  Info("changeInput()", "tempDSID=%s", temp.Data());
  // we need to remove three
  temp = temp(temp.First('.') + 1, temp.Length() + 1);
  temp = temp(temp.First('.') + 1, temp.Length() + 1);
  temp = temp(temp.First('.') + 1, temp.Length() + 1);
  //temp4=366019.CAOD_HIGG5D1.e7033_s3126_r10724_p3717.32-07-P-A_CxAOD.root
  Info("changeInput()", "tempDSID=%s", temp.Data());
  temp = temp(0, temp.First('.'));
  //temp5=366019
  Info("changeInput()", "tempDSID=%s", temp.Data());
  m_mcChannelFromInputFile = temp.Atoi();
  Info("changeInput()", "m_mcChannelFromInputFile=%i",
       m_mcChannelFromInputFile);
  // done evaluate m_mcChannelFromInputFile

  m_isSherpaPt0WJets =
      ((filename.Contains("Sherpa_CT10_W") || filename.Contains("Sh_CT10_W")) &&
       (filename.Contains("Pt0") && !filename.Contains("Pt70")));
  m_isSherpaPt0ZJets =
      ((filename.Contains("Sherpa_CT10_Z") || filename.Contains("Sh_CT10_Z")) &&
       (filename.Contains("Pt0") && !filename.Contains("Pt70")));

  // general Sherpa flag
  m_isSherpaVJets =
      ((filename.Contains("Sherpa") || filename.Contains("Sh_CT10")) &&
       filename.Contains("Pt"));

  //Check for Sherpa/sh/sherpa
  static const std::regex GenRegex("(.*)([Ss][h])(.*)");
  TString fullfilepath = filename.Data();
  TObjArray *filepath_tokens = fullfilepath.Tokenize("/");

  std::string foldername = (std::string)(
      ((TObjString *)(filepath_tokens->At(filepath_tokens->GetEntries() - 3)))
          ->String());

  //Perform Regular Expression match
  if (std::regex_match(foldername, GenRegex)) {
    m_isSherpa = true;
  }

  /*
    //QG Tagging Calibration Truth Match Files need to switch
    std::cout << "we are about to check if we are suppose to truthmatch"
              << std::endl;
    m_qgtruthmatch = false;
    m_config->getif<bool>("qgtruthmatch", m_qgtruthmatch);
    std::cout << "QG TRUTH MATCH IS: " << m_qgtruthmatch << std::endl;
    if (m_qgtruthmatch && m_isMC) {
      string m_qgtruthfile = "DNE";
      m_config->getif<string>("qgtruthfile", m_qgtruthfile);
      std::string this_dsid_truth_file = m_qgtruthfile + "luser.woodsn." +
                                         to_string(m_mcChannelFromInputFile) +
                                         "_all.root";
      std::cout << "QG TRUTH FILE: " << this_dsid_truth_file << std::endl;
      m_qg_truth_map_status = computeQGTruthMap(this_dsid_truth_file);
    }
  */
  return EL::StatusCode::SUCCESS;
}  // changeInput

EL::StatusCode AnalysisReader::initialize() {
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  Info("initialize()", "Dumping config...");
  m_config->print();

  // turn off sending xAOD summary access report (to be discussed)
  bool m_enableDataSubmission = true;
  m_config->getif<bool>("enableDataSubmission", m_enableDataSubmission);
  if (!m_enableDataSubmission) {
    Info("initialize()", "Turning off xAOD access report.");
    xAOD::TFileAccessTracer::enableDataSubmission(false);
  }

  m_config->getif<bool>("doCombMass", m_doCombMass);          //combMass
  m_config->getif<bool>("doCombMassMuon", m_doCombMassMuon);  //combMass

  EL_CHECK("initialize()", initializeCxAODTag());
  EL_CHECK("initialize()", initializeEvent());
  EL_CHECK("initialize()", initializeReader());
  EL_CHECK("initialize()", initializeIsMC());
  EL_CHECK("initialize()", initializeVariations());
  EL_CHECK("initialize()", initializeSelection());
  EL_CHECK("initialize()", initializeTools());
  EL_CHECK("initialize()", initializeSumOfWeights());

  m_config->getif<bool>("validation", m_validation);
  m_config->getif<int>("maxEvents", m_maxEvents);

  if (m_validation) {
    EL_CHECK("initialize()", initializeValidationSelection());
  }

  EL_CHECK("initialize()", initializeChannel());

  //if boosted VHbb analysis, lower MCWeight threshold for Sherpa large weight protection
  std::string ana = "";
  m_config->getif<std::string>("analysis", ana);

  std::string anaStrategy = "";
  m_config->getif<std::string>("analysisStrategy", anaStrategy);

  if ((ana == "VHbb") && (anaStrategy == "Merged")) {
    m_lowerShMCWeightThreshold = true;
  }

  return EL::StatusCode::SUCCESS;
}  // initialize

EL::StatusCode AnalysisReader::initializeCxAODTag() {
  Info("initializeCxAODTag()", "Initialize CxAODTag");
  m_CxAODTag = "CxAODTag26";  // note earlier than 26 is not supported any more
  //TTree *metaDataCxAODTree = nullptr;
  //metaDataCxAODTree = dynamic_cast<TTree*>(wk()->inputFile()->Get("MetaData_CxAOD"));
  //if(metaDataCxAODTree) {
  //  m_CxAODTag="CxAODTag28";
  // to do: later to read the value of the brank in the tree to distinguish tag 28 and tag 30, etc.
  //}
  // read it from the config directly
  // CxAODTag31 from ICHEP 2018
  // CxAODTag32 for full dataset Run-2, with new derivations and data18/mc16e
  m_config->getif<string>("CxAODTag", m_CxAODTag);

  Info("AnalysisReader::initializeCxAODTag()",
       "We assume this file is of CxAODTag=%s. AnalysisReader supports only "
       "tag 26 or later.",
       m_CxAODTag.c_str());
  return EL::StatusCode::SUCCESS;
}  // initializeCxAODTag

EL::StatusCode AnalysisReader::initializeEvent() {
  Info("initializeEvent()", "Initialize event.");

  m_event = wk()->xaodEvent();

  Info("initializeEvent()", "Number of events on first file = %lli",
       m_event->getEntries());  // print long long int

  m_eventCounter = 0;

  m_config->getif<bool>("debug", m_debug);

  // luminosity (for rescaling of MC to desired lumi, in fb-1)
  // default is 0.001 fb-1, which means no rescaling of the MC inputs (already scaled to 1 pb-1)
  m_luminosity = 0.001;
  m_config->getif<float>("luminosity", m_luminosity);
  Info("initializeEvent()", "Luminosity for normalisation of MC = %f fb-1",
       m_luminosity);

  m_config->getif<bool>("applyEventPreSelection", m_applyEventPreSelection);
  m_config->getif<bool>("applySherpaTruthPtCut", m_applySherpaTruthPtCut);
  m_config->getif<bool>("applyPowhegTruthMttCut", m_applyPowhegTruthMttCut);
  m_config->getif<bool>("allowORwithTruthSelection",
                        m_allowORwithTruthSelection);
  m_config->getif<float>("usePowhegInclFraction", m_usePowhegInclFraction);
  m_config->getif<bool>("computePileupReweight", m_computePileupReweight);
  m_config->getif<bool>("recomputePileupReweight", m_recomputePileupReweight);
  m_config->getif<bool>("checkEventDuplicates", m_checkEventDuplicates);
  m_config->getif<bool>("failEventDuplicates", m_failEventDuplicates);
  m_config->getif<bool>("putAllSysInOneDir", m_putAllSysInOneDir);
  m_config->getif<std::string>("applyVZbbWeight", m_applyVZbbWeight);

  std::string whichData = "combined";
  m_config->getif<std::string>("whichData", whichData);
  if (whichData != "combined" &&
      !(m_computePileupReweight || m_recomputePileupReweight)) {
    Error("PUReweightingTool::initialize()",
          "Running on %s data only -> need to recompute PU weight. Set "
          "recomputePUWeight to true in config! Exiting.",
          whichData.c_str());
    return EL::StatusCode::FAILURE;
  }

  return EL::StatusCode::SUCCESS;
}  // initializeEvent

EL::StatusCode AnalysisReader::initializeReader() {
  Info("initializeReader()", "Initialize object reader.");

  // note: the names of the readers determine which collections are
  //       read, see also framework-read.cfg

  m_eventInfoReader = registerReader<xAOD::EventInfo>("eventInfo");
  m_METReader = registerReader<xAOD::MissingETContainer>("MET");
  m_MPTReader = registerReader<xAOD::MissingETContainer>("MPT");
  m_truthMETReader = registerReader<xAOD::MissingETContainer>("METTruth");
  m_METMJTightReader = registerReader<xAOD::MissingETContainer>("METMJTight");
  m_METMJMuTightReader =
      registerReader<xAOD::MissingETContainer>("METMJMuTight");
  m_electronReader = registerReader<xAOD::ElectronContainer>("electron");
  m_forwardElectronReader =
      registerReader<xAOD::ElectronContainer>("forwardElectron");
  m_photonReader = registerReader<xAOD::PhotonContainer>("photon");
  m_ditauReader = registerReader<xAOD::DiTauJetContainer>("diTauJet");
  m_muonReader = registerReader<xAOD::MuonContainer>("muon");
  m_tauReader = registerReader<xAOD::TauJetContainer>("tau");
  m_jetReader = registerReader<xAOD::JetContainer>("jet");
  m_fatJetReader = registerReader<xAOD::JetContainer>("fatJet");
  m_fatJetAltReader = registerReader<xAOD::JetContainer>("fatJetAlt");
  m_trackJetReader = registerReader<xAOD::JetContainer>("trackJet");
  m_subJetReader = registerReader<xAOD::JetContainer>("subJet");
  m_truthEventReader = registerReader<xAOD::TruthEventContainer>("truthEvent");
  m_truthParticleReader =
      registerReader<xAOD::TruthParticleContainer>("truthParticle");
  m_truthTauReader = registerReader<xAOD::TruthParticleContainer>("truthTau");
  m_truthMuonReader = registerReader<xAOD::TruthParticleContainer>("truthMuon");
  m_truthElectronReader =
      registerReader<xAOD::TruthParticleContainer>("truthElectron");
  m_truthNeutrinoReader =
      registerReader<xAOD::TruthParticleContainer>("truthNeutrino");
  m_truthWZJetReader = registerReader<xAOD::JetContainer>("truthWZJet");
  m_truthWZFatJetReader = registerReader<xAOD::JetContainer>("truthFatWZJet");
  m_mmcReader = registerReader<xAOD::EventInfo>("ditau");
  return EL::StatusCode::SUCCESS;
}  // initializeReader

EL::StatusCode AnalysisReader::initializeSelection() {
  Info("initializeSelection()", "Initialize selection.");

  // set event selection: use a dummy here
  m_eventSelection = new DummyEvtSelection();
  m_eventPostSelection = new DummyEvtSelection();
  m_fillFunction = std::bind(&AnalysisReader::fill_example, this);

  return EL::StatusCode::SUCCESS;
}  // initializeSelection

EL::StatusCode AnalysisReader::initializeTools() {
  Info("initializeTools()", "Initialize tools.");

  //Xsec Provider
  //------------------
  std::string comEnergy = m_config->get<std::string>("COMEnergy");
  std::string xSectionFile = gSystem->Getenv("WorkDir_DIR");
  int releaseSeries = atoi(getenv("ROOTCORE_RELEASE_SERIES"));
  if (releaseSeries >= 25) xSectionFile = gSystem->Getenv("WorkDir_DIR");

  xSectionFile += "/data/CxAODOperations/XSections_";
  xSectionFile += comEnergy;
  xSectionFile += ".txt";
  // if xSectionFile is given in config file, replace all we just did
  m_config->getif<string>("xSectionFile", xSectionFile);
  m_xSectionProvider = new XSectionProvider(xSectionFile);

  if (!m_xSectionProvider) {
    Error("initializeTools()", "XSection provider not initialized!");
    return EL::StatusCode::FAILURE;
  }

  // overlap removal
  // ---------------
  m_overlapRegAcc = new OverlapRegisterAccessor(OverlapRegisterAccessor::READ);

  m_metMJCalc = 0;
  m_config->getif<int>("metMJCalc", m_metMJCalc);
  m_jetPtCut = 20.0e3;
  m_config->getif<float>("jetPtCut", m_jetPtCut);
  m_jetRapidityCut = 2.5;
  m_config->getif<float>("jetRapidityCut", m_jetRapidityCut);
  m_lepPtCut = 25.0e3;
  m_config->getif<float>("lepPtCut", m_lepPtCut);

  // b-tagging
  // ---------
  bool use2DbTagCut = false;
  m_config->getif<bool>("use2DbTagCut", use2DbTagCut);
  bool uncorrelate_run1_to_run2 = false;
  m_config->getif<bool>("uncorrelate_run1_to_run2_btagging",
                        uncorrelate_run1_to_run2);

  std::string btaggingCDIfilename;
  m_config->getif<std::string>("btaggingCDIfilename", btaggingCDIfilename);
  std::vector<std::string> bTagToolConfigs;
  if (use2DbTagCut)
    m_config->getif<std::vector<std::string> >("bTagToolConfigs2D",
                                               bTagToolConfigs);
  else
    m_config->getif<std::vector<std::string> >("bTagToolConfigs",
                                               bTagToolConfigs);

  m_useContinuous = false;
  if (std::find(bTagToolConfigs.begin(), bTagToolConfigs.end(),
                std::string("Continuous")) != bTagToolConfigs.end())
    m_useContinuous = true;

  std::vector<std::string> bTagVariationConfigs;
  m_config->getif<std::vector<std::string> >("weightVariations",
                                             bTagVariationConfigs);
  std::string reductionScheme = "Medium";
  bool doWeightVar = false;
  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);
  if (!nominalOnly) {
    for (auto var : bTagVariationConfigs) {
      if (var.find("BTAG") != std::string::npos) {
        doWeightVar = true;

        if (var.find("LOOSE") != std::string::npos) reductionScheme = "Loose";

        if (var.find("MEDIUM") != std::string::npos) reductionScheme = "Medium";

        if (var.find("TIGHT") != std::string::npos) reductionScheme = "Tight";
      }
    }
  }

  bool useQuntile = false;
  int maxTruthTag = 2;
  string ExcludeBTagEVlist = "";
  m_config->getif<bool>("UseQuantile", useQuntile);
  m_config->getif<int>("maxTruthTag", maxTruthTag);
  m_config->getif<std::string>("ExcludeBTagEVlist", ExcludeBTagEVlist);
  if (bTagToolConfigs.size() >= 4) {
    BTaggingTool::Config_t args{{"TaggerName", bTagToolConfigs[0]},
                                {"OperatingPoint", bTagToolConfigs[1]},
                                {"JetAuthor", bTagToolConfigs[2]},
                                {"Scheme", bTagToolConfigs[3]},
                                {"rScheme", reductionScheme}};

    if (!m_bTagTool) m_bTagTool = new BTaggingTool();
    if (btaggingCDIfilename.size() == 0) {
      EL_CHECK(
          "initializeTools()",
          m_bTagTool->initialize(args, use2DbTagCut, uncorrelate_run1_to_run2,
                                 useQuntile, maxTruthTag));
    } else {
      EL_CHECK(
          "initializeTools()",
          m_bTagTool->initialize(args, use2DbTagCut, uncorrelate_run1_to_run2,
                                 useQuntile, maxTruthTag, btaggingCDIfilename));
    }
    if (ExcludeBTagEVlist != "none")
      m_bTagTool->setExcludedBTagEV(ExcludeBTagEVlist);

    m_bTagTool->setWeightVar(doWeightVar);
  } else {
    Warning("initializeTools()",
            "Could not initialize BTaggingTool due to invalid bTagToolConfigs "
            "in config!");
  }

  //Multiple event weight tool for Sherpa 2.2.1
  //NOTE:: EvtWeightVariations() constructor initialises all class internal member variables
  //       No need for manual call of any initialise methods
  m_evtWeightVarMode = m_config->get<int>("evtWeightVarMode");
  // const string detailedName = m_xSectionProvider->getSampleDetailName( (int)eventInfo->mcChannelNumber() );
  if (m_evtWeightVarMode != -1) {
    m_EvtWeightVar = new EvtWeightVariations(m_evtWeightVarMode);
  };

  // pileup reweighting tool
  // ---------------------------
  m_puReweightingTool = new PUReweightingTool(*m_config);
  if (m_computePileupReweight || m_recomputePileupReweight)
    EL_CHECK("AnalysisReader::initializeTools()",
             m_puReweightingTool->initialize());

  //PtRecoTool
  m_PtRecoTool = new PtRecoTool();
  EL_CHECK("AnalysisReader::initializeTools()",
           m_PtRecoTool->initialize(m_debug));

  return EL::StatusCode::SUCCESS;
}  // initializeTools

EL::StatusCode AnalysisReader::initializeValidationSelection() {
  Info("initializeValidationSelection()", "Initialize validation selection");

  m_validationFillFunction = std::bind(&AnalysisReader::fill_validation, this);
  return EL::StatusCode::SUCCESS;
}  // initializeValidationSelection

EL::StatusCode AnalysisReader::initializeIsMC() {
  Info("initializeIsMC()", "Initialize isMC.");

  // get nominal event info
  // -------------------------------------------------------------
  const xAOD::EventInfo *eventInfo = m_eventInfoReader->getObjects("Nominal");

  if (!eventInfo) return EL::StatusCode::FAILURE;

  // get MC flag - different info on data/MC files
  // -----------------------------------------------
  m_isMC = Props::isMC.get(eventInfo);
  Info("initializeIsMC()", "isMC = %i", m_isMC);

  return EL::StatusCode::SUCCESS;
}  // initializeIsMC

EL::StatusCode AnalysisReader::initializeChannel() {
  //set MC channel number
  //----------------------
  const xAOD::EventInfo *eventInfo = m_eventInfoReader->getObjects("Nominal");

  if (!eventInfo) return EL::StatusCode::FAILURE;

  if (!m_xSectionProvider) {
    Error("initializeChannel()", "XSection provider not initialized!");
    return EL::StatusCode::FAILURE;
  }

  if (m_isMC) {
    //MC
    m_mcChannel = (int)eventInfo->mcChannelNumber();
    if (m_debug) {
      Info("initializeChannel()", "before m_mcChannel=%i", m_mcChannel);
    }
    // temporary hack for ZnunuB_pTV, ZnunuC_pTV and ZnunuL_pTV
    // due to an EVNT-EVNT error in MC generation for some samples (https://gitlab.cern.ch/CxAODFramework/CxAODReader/issues/18)
    // from the various DSID from the xAOD/DxAOD/CxAOD we have the same other DSID in the metadata
    // e.g. 366010 (B), 366019 (C), 366028 (L) all have the same metadata mcChannel of 366001
    // the mcChannel is used for the cross section file, PRW, etc
    // we need here to overwrite the 366001 with either 366010, 366019, or 366028
    // initially we had a hard coded only for the B filter
    // but now we need to add also C and L filter
    // so in the same file in the function changeInput() we compute m_mcChannelFromInputFile
    // and we replace m_mcChannel with m_mcChannelFromInputFile for those DSID that need it
    if (false
        // ZnunuX_PTV, X=B, C, L
        || (m_mcChannel == 366001) || (m_mcChannel == 366002) ||
        (m_mcChannel == 366003) || (m_mcChannel == 366004) ||
        (m_mcChannel == 366005) || (m_mcChannel == 366006) ||
        (m_mcChannel == 366007) || (m_mcChannel == 366008)
        // we can add here more in the future if they have the same problem as ZnunuX_PTV
    ) {
      m_mcChannel = m_mcChannelFromInputFile;
    }
    if (m_debug) {
      Info("initializeChannel()", "after  m_mcChannel=%i", m_mcChannel);
    }
    // done for ZnunuB pTV slice

    //set histNameSvc
    m_histNameSvc->set_sample(m_xSectionProvider->getSampleName(m_mcChannel));
    m_truthHistNameSvc->set_sample(
        m_xSectionProvider->getSampleName(m_mcChannel));
  } else {
    // data
    // set histNameSvc
    m_truthHistNameSvc->set_sample("data");
    m_histNameSvc->set_sample("data");
  }  // done if

  if (m_debug)
    Info("initializeChannel()", Form("Initialize channel %d", m_mcChannel));

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::initializeSumOfWeights() {
  Info("initializeSumOfWeights()", "Initialize sum of weights.");

  m_config->getif<std::string>("period", m_period);

  if (!m_isMC) {
    return EL::StatusCode::SUCCESS;
  }

  std::string sumOfWeightsFile;
  bool generateYieldFile = false;
  std::string CxAODYieldDir = "CxAODOperations_VHbb/CxAOD/info";
  std::string analysisType = m_config->get<std::string>("analysisType");
  std::string comEnergy = m_config->get<std::string>("COMEnergy");
  m_config->getif<bool>("generateYieldFile", generateYieldFile);
  m_config->getif<std::string>("CxAODYieldDir", CxAODYieldDir);

  if (generateYieldFile && (m_sumOfWeightsFile != "")) {
    sumOfWeightsFile = m_sumOfWeightsFile;
  } else {
    sumOfWeightsFile = gSystem->Getenv("WorkDir_DIR");
    int releaseSeries = atoi(getenv("ROOTCORE_RELEASE_SERIES"));
    if (releaseSeries >= 25) sumOfWeightsFile = gSystem->Getenv("WorkDir_DIR");
    sumOfWeightsFile += "/data/" + CxAODYieldDir + "/yields.";
    sumOfWeightsFile += analysisType;
    sumOfWeightsFile += ".";
    sumOfWeightsFile += comEnergy;
    sumOfWeightsFile += "_mc16" + m_period;
    sumOfWeightsFile += ".txt";
  }
  // if yieldFile is given in config file, replace all we just did
  m_config->getif<string>("yieldFile", sumOfWeightsFile);
  m_sumOfWeightsProvider = new sumOfWeightsProvider(sumOfWeightsFile);

  if (!m_sumOfWeightsProvider) {
    Error("initializeSumOfWeights()", "sumOfWeightsProvider not initialized!");
    return EL::StatusCode::FAILURE;
  }
  return EL::StatusCode::SUCCESS;
}  // initializeSumOfWeights

EL::StatusCode AnalysisReader::initializeVariations() {
  Info("initializeVariations()", "Initialize systematic variations.");

  // global list of variation, which is looped over in execute():
  m_variations.clear();

  // retrieve variations from config if set
  std::vector<std::string>
      variations;  //experimental systematic stored in CxAODs
  bool nominalOnly = false;
  bool autoDiscoverVariations = true;
  m_config->getif<std::vector<std::string> >("variations", variations);
  m_config->getif<bool>("nominalOnly", nominalOnly);
  m_config->getif<bool>("autoDiscoverVariations", autoDiscoverVariations);
  m_config->getif<bool>("doQCD", m_doQCD);
  if (m_doQCD) {
    m_config->getif<std::vector<std::string> >("FakeFactorSyst", variations);
  }

  // read available variations in input file (discoverVariations has to be called for each reader)
  TTree *collectionTree =
      dynamic_cast<TTree *>(wk()->inputFile()->Get("CollectionTree"));
  for (ObjectReaderBase *reader : m_objectReader) {
    reader->discoverVariations(collectionTree);
  }

  // Nominal has to be processed first (bug in EDM? maybe not true anymore)
  m_variations.push_back("Nominal");

  // if nominalOnly we are done
  if (nominalOnly) {
    Info("initializeVariations()", "Running Nominal only!");
    return EL::StatusCode::SUCCESS;
  }

  // add variations found in input to global list
  for (ObjectReaderBase *reader : m_objectReader) {
    for (const std::string &varName : reader->getVariations()) {
      // ensure to use each systematic once
      bool found = (std::find(m_variations.begin(), m_variations.end(),
                              varName) != m_variations.end());
      if (!found) {
        m_variations.push_back(varName);
      }
    }
  }

  // add QCD systs TODO: this seems not to be a consistent treatment
  if (m_doQCD) {
    for (const std::string &varName : variations) {
      bool found = (std::find(m_variations.begin(), m_variations.end(),
                              varName) != m_variations.end());
      if (!found) {
        m_variations.push_back(varName);
      }
    }
  }

  // print variations found in input file to screen
  Info("initializeVariations()", "Variations found in input file:");
  for (const std::string &varName : m_variations) {
    Info("initializeVariations()", varName.c_str());
  }

  // process all variations in input if autoDiscoverVariations
  if (autoDiscoverVariations) {
    Info("initializeVariations()",
         "Running all variations found in input file.");
    return EL::StatusCode::SUCCESS;
  }

  // filter variations using the list from the config file
  Info("initializeVariations()",
       "Running only variations requested in config file (plus Nominal and "
       "weight variations):");
  vector<std::string> filteredVars;
  filteredVars.push_back("Nominal");
  // loop over systs specified in config
  for (std::string varName : variations) {
    // check if available in input
    bool found = false;
    for (std::string varNameIn : m_variations) {
      if ((varName + "__1up") == varNameIn ||
          (varName + "__1down") == varNameIn) {
        Info("initializeVariations()", varNameIn.c_str());
        filteredVars.push_back(varNameIn);
        found = true;
      }
    }
    if (!found) {
      Warning("initializeVariations()",
              "Did not find requested variation '%s' in input. Please check "
              "config!",
              varName.c_str());
    }
  }
  m_variations = filteredVars;

  return EL::StatusCode::SUCCESS;
}  // initializeVariations

EL::StatusCode AnalysisReader::finalizeTools() {
  Info("finalizeTools()", "Finalizing tools.");

  // b-tagging
  // ---------
  // EL_CHECK("AnalysisReader::finalizeTools()", m_bTagTool->release());
  if (m_bTagTool) {
    delete m_bTagTool;
    m_bTagTool = nullptr;
  }

  // pileup reweighting
  // --------------------
  if (m_puReweightingTool) {
    delete m_puReweightingTool;
    m_puReweightingTool = nullptr;
  }

  // trigger tool
  // -------------
  if (m_triggerTool) {
    delete m_triggerTool;
    m_triggerTool = nullptr;
  }

  // overlapRegisterAccessor
  if (m_overlapRegAcc) {
    delete m_overlapRegAcc;
    m_overlapRegAcc = nullptr;
  }

  return EL::StatusCode::SUCCESS;
}  // finalizeTools

PROPERTY(PropsEmptyContFix, float, pt)
PROPERTY(PropsEmptyContFix, float, px)
EL::StatusCode AnalysisReader::clearEmptyContainersWithNonZeroSize() {
  // empty containers for replacement
  static const xAOD::ElectronContainer empty_electrons;
  static const xAOD::ElectronContainer empty_forwardElectrons;
  static const xAOD::DiTauJetContainer empty_ditaus;
  static const xAOD::PhotonContainer empty_photons;
  static const xAOD::MuonContainer empty_muons;
  static const xAOD::TauJetContainer empty_taus;
  static const xAOD::JetContainer empty_jets;
  static const xAOD::TruthParticleContainer empty_truthParts;
  static const xAOD::JetContainer empty_truthWZJets;
  static const xAOD::JetContainer empty_truthWZFatJets;

  // replace invalid particle containers with empty ones
  if (m_electrons)
    if (m_electrons->size())
      if (!PropsEmptyContFix::pt.exists(m_electrons->at(0)))
        m_electrons = &empty_electrons;

  if (m_forwardElectrons)
    if (m_forwardElectrons->size())
      if (!PropsEmptyContFix::pt.exists(m_forwardElectrons->at(0)))
        m_forwardElectrons = &empty_forwardElectrons;

  if (m_photons)
    if (m_photons->size())
      if (!PropsEmptyContFix::pt.exists(m_photons->at(0)))
        m_photons = &empty_photons;

  if (m_ditaus)
    if (m_ditaus->size())
      if (!PropsEmptyContFix::pt.exists(m_ditaus->at(0)))
        m_ditaus = &empty_ditaus;

  if (m_muons)
    if (m_muons->size())
      if (!PropsEmptyContFix::pt.exists(m_muons->at(0))) m_muons = &empty_muons;
  if (m_truthWZFatJets)
    if (m_truthWZFatJets->size())
      if (!PropsEmptyContFix::pt.exists(m_truthWZFatJets->at(0)))
        m_truthWZFatJets = &empty_truthWZFatJets;

  if (m_truthWZJets)
    if (m_truthWZJets->size())
      if (!PropsEmptyContFix::pt.exists(m_truthWZJets->at(0)))
        m_truthWZJets = &empty_truthWZJets;

  if (m_taus)
    if (m_taus->size())
      if (!PropsEmptyContFix::pt.exists(m_taus->at(0))) m_taus = &empty_taus;

  if (m_jets)
    if (m_jets->size())
      if (!PropsEmptyContFix::pt.exists(m_jets->at(0))) m_jets = &empty_jets;

  if (m_fatJets)
    if (m_fatJets->size())
      if (!PropsEmptyContFix::pt.exists(m_fatJets->at(0)))
        m_fatJets = &empty_jets;

  if (m_fatJetsAlt)
    if (m_fatJetsAlt->size())
      if (!PropsEmptyContFix::pt.exists(m_fatJetsAlt->at(0)))
        m_fatJetsAlt = &empty_jets;

  if (m_trackJets)
    if (m_trackJets->size())
      if (!PropsEmptyContFix::pt.exists(m_trackJets->at(0)))
        m_trackJets = &empty_jets;

  if (m_subJets)
    if (m_subJets->size())
      if (!PropsEmptyContFix::pt.exists(m_subJets->at(0)))
        m_subJets = &empty_jets;

  if (m_truthParts)
    if (m_truthParts->size())
      if (!PropsEmptyContFix::px.exists(m_truthParts->at(0)))
        m_truthParts = &empty_truthParts;

  return EL::StatusCode::SUCCESS;
}  // clearEmptyContainersWithNonZeroSize

EL::StatusCode AnalysisReader::execute() {
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  if (((m_eventCounter % 1000) == 0) || m_debug) {
    Info("execute()", "Event number = %li", m_eventCounter);
  }
  m_eventCounter++;
  // b-tagging requires special index for MC-to-MC efficiencies
  // if (m_isMC) m_bTagTool->setMCIndex(m_bTagTool->indexMCEfficiencyFromFileName(wk()->inputFile()->GetName()));
  // if (m_isMC) m_bTagTool->setMCIndex(m_bTagTool->indexMCEfficiencyFromChannel(m_mcChannel, *m_xSectionProvider), *m_xSectionProvider);

  EL_CHECK("changeInput()", initializeChannel());

  if (m_isMC) m_bTagTool->setMCIndex(m_mcChannel, *m_xSectionProvider);

  // store random number from pu tool -> working with nominal is enough
  if (m_isMC) {
    // if statement for backword compatibility, in CxAOD31 present with dummy value also for data
    m_randomRunNumber =
        Props::RandomRunNumber.get(m_eventInfoReader->getObjects("Nominal"));
  }

  // ----------------------------------------------------------
  // loop on systematic variations - or at least run "Nominal"!
  // ----------------------------------------------------------
  for (std::string varName : m_variations) {
    bool isNominal = (varName == "Nominal");

    // --------------------
    // HistNameSvc settings
    // ---------------------
    m_histNameSvc->reset();
    m_histNameSvc->set_variation(varName);
    m_histNameSvc->set_doOneSysDir(
        m_putAllSysInOneDir);  //put all syst in one directory
    m_truthHistNameSvc->reset();
    m_truthHistNameSvc->set_variation(varName);
    m_truthHistNameSvc->set_doOneSysDir(
        m_putAllSysInOneDir);  //put all syst in one directory
    m_weightSysts.clear();     // prepare list of weight variations
    m_weightSystsCS.clear();   // prepare list of weight variations

    m_currentVar = varName;
    if (m_debug) Info("execute ()", "Running variation %s", varName.c_str());

    EL_CHECK("execute()", applyVariationToTools(varName.c_str()));

    //combMass begin
    bool isCombMassSyst(false);
    if (m_doCombMass) {
      EL_CHECK("execute()", combMassTmp1(isCombMassSyst, varName));
      //std::cout<<"isCombMassSyst="<<isCombMassSyst<<", varName="<<varName<<endl;
    }
    //combMass end

    // --------------------
    // retrieve containers
    // --------------------
    if (m_debug) Info("execute ()", "Retrieving input containers...");
    if (m_eventInfoReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_eventInfoReader");
      m_eventInfo = m_eventInfoReader->getObjects(varName);
    }
    if (m_electronReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_electronReader");
      m_electrons = m_electronReader->getObjects(varName);
    }
    if (m_forwardElectronReader) {
      if (m_debug)
        Info("execute ()",
             "Retrieving input container m_forwardElectronReader");
      m_forwardElectrons = m_forwardElectronReader->getObjects(varName);
    }
    if (m_photonReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_photonReader");
      m_photons = m_photonReader->getObjects(varName);
    }
    if (m_ditauReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_ditauReader");
      m_ditaus = m_ditauReader->getObjects(varName);
    }
    if (m_muonReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_muonReader");
      m_muons = m_muonReader->getObjects(varName);
    }
    if (m_tauReader) {
      if (m_debug) Info("execute ()", "Retrieving input container m_tauReader");
      m_taus = m_tauReader->getObjects(varName);
    }
    if (m_jetReader) {
      if (m_debug) Info("execute ()", "Retrieving input container m_jetReader");
      m_jets = m_jetReader->getObjects(varName);
    }
    if (m_fatJetReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_fatJetReader");
      m_fatJets = m_fatJetReader->getObjects(varName);
    }
    if (m_fatJetAltReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_fatJetAltReader");
      m_fatJetsAlt = m_fatJetAltReader->getObjects(varName);
    }
    if (m_trackJetReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_trackJetReader");
      m_trackJets = m_trackJetReader->getObjects(varName);
    }
    if (m_subJetReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_subJetReader");
      m_subJets = m_subJetReader->getObjects(varName);
    }
    if (m_truthEventReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthEventReader");
      m_truthEvent = m_truthEventReader->getObjects(varName);
    }
    if (m_truthParticleReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthParticleReader");
      m_truthParts = m_truthParticleReader->getObjects(varName);
    }
    if (m_truthTauReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthTauReader");
      m_truthTaus = m_truthTauReader->getObjects(varName);
    }
    if (m_truthMuonReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthMuonReader");
      m_truthMuons = m_truthMuonReader->getObjects(varName);
    }
    if (m_truthElectronReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthElectronReader");
      m_truthElectrons = m_truthElectronReader->getObjects(varName);
    }
    if (m_truthNeutrinoReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthNeutrinoReader");
      m_truthNeutrinos = m_truthNeutrinoReader->getObjects(varName);
    }
    if (m_truthWZFatJetReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthWZFatJetReader");
      m_truthWZFatJets = m_truthWZFatJetReader->getObjects(varName);
    }
    if (m_truthWZJetReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthWZJetReader");
      m_truthWZJets = m_truthWZJetReader->getObjects(varName);
    }
    if (m_mmcReader) {
      if (m_debug) Info("execute ()", "Retrieving input container m_mmcReader");
      m_mmc = m_mmcReader->getObjects(varName);
    }
    m_met = nullptr;
    m_met_soft = nullptr;
    if (m_METReader) {
      if (m_doQCD && m_currentVar == "MJ_El_METstr") {
        if (m_debug)
          Info("execute ()",
               "Retrieving input container m_METMJTightReader since m_doQCD "
               "and m_currentVar == MJ_El_METstr");
        m_metCont = m_METMJTightReader->getObjects(varName);
        // else if(m_doQCD&&m_currentVar=="MJ_Mu_METstr") m_metCont = m_METMJMuTightReader->getObjects(varName);
      } else {
        if (m_debug)
          Info("execute ()",
               "Retrieving input container m_METReader as either m_doQCD is "
               "false or m_currentVar != MJ_El_METstr");
        m_metCont = m_METReader->getObjects(
            varName);  // but the METReader is read twice, as it is already read once above!
      }
      if (m_metCont == 0) {
        Error("execute ()", "m_metCont pointer is nullptr");
        return EL::StatusCode::FAILURE;
      }
      if (m_metCont->size() == 1) {
        m_met = m_metCont->at(0);
      } else if (m_metCont->size() == 2) {
        m_met = m_metCont->at(0);
        m_met_soft = m_metCont->at(1);
      } else {
        Error("execute ()", "m_metCont '%s' has size != 1 or 2",
              m_METReader->getContainerName().c_str());
        return EL::StatusCode::FAILURE;
      }
    }

    m_mpt = nullptr;
    if (m_MPTReader) {
      if (m_debug) Info("execute ()", "Retrieving input container m_MPTReader");
      m_mptCont = m_MPTReader->getObjects(varName);
      if (m_mptCont == 0) {
        Error("execute ()", "m_mptCont pointer is nullptr");
        return EL::StatusCode::FAILURE;
      }
      if (m_mptCont->size() > 0) {
        m_mpt = m_mptCont->at(0);
      } else {
        Error("execute ()", "m_mptCont '%s' has size != 1",
              m_MPTReader->getContainerName().c_str());
        return EL::StatusCode::FAILURE;
      }
    }

    m_truthMET = nullptr;
    if (m_truthMETReader) {
      if (m_debug)
        Info("execute ()", "Retrieving input container m_truthMETReader");
      const xAOD::MissingETContainer *m_truthCont =
          m_truthMETReader->getObjects(varName);
      if (m_truthCont == 0) {
        Error("execute ()", "m_truthCont pointer is nullptr");
        return EL::StatusCode::FAILURE;
      }
      if (m_truthCont->size() > 0) {
        m_truthMET = m_truthCont->at(0);
      } else {
        Error("execute ()", "m_truthMETReader '%s' has size < 1",
              m_truthMETReader->getContainerName().c_str());
        return EL::StatusCode::FAILURE;
      }
    }

    // -----------------------------------------------------------------------------
    // get event weight - fill m_weight, decorate eventInfo with individual weights
    // also get corrected mu in data
    // -----------------------------------------------------------------------------

    // First check if ggZllHbb / ggZvvHbb / ggZllHcc / ggZvvHcc weights are insanely large -> if it's the case, return SUCCESS to get out of the loop
    // Getting out of the loop here effectively means discarding events

    m_config->getif<bool>("doLargeMCEventWeightsRemoval",
                          m_doLargeMCEventWeightsRemoval);
    if (m_doLargeMCEventWeightsRemoval) {
      if (checkMCEventWeight()) {
        return EL::StatusCode::SUCCESS;
      }
    }

    if (m_debug) Info("execute ()", "Calculating event weight...");
    EL_CHECK("AnalysisReader::execute()", setEventWeight());

    // Event duplicates
    if (m_checkEventDuplicates) {
      EL_CHECK("execute()", checkEventDuplicates());
    }

    // -------------
    //combMass
    // -------------

    if (m_doCombMass) {
      EL_CHECK("execute()", combMassTmp2(isCombMassSyst));
    }

    // --------------
    // SherpaTruthPt
    // --------------
    if ((varName == "Nominal") && m_isMC && m_isSherpaVJets &&
        m_applySherpaTruthPtCut) {
      bool passSherpaTruthPt;
      float sherpaTruthPt;
      EL_CHECK("AnalysisReader::execute()",
               checkSherpaTruthPt(passSherpaTruthPt, sherpaTruthPt));

      if (!passSherpaTruthPt) return EL::StatusCode::SUCCESS;

      m_histSvc->BookFillHist("VptTruth", 100, 0, 1000, sherpaTruthPt / 1000.,
                              m_weight);
    }

    // --------------
    // PowhegTruthMtt
    // --------------
    if (m_isMC && m_applyPowhegTruthMttCut) {
      bool passPowhegTruthMtt;
      float powhegTruthMtt;
      EL_CHECK("AnalysisReader::execute()",
               checkPowhegTruthMtt(passPowhegTruthMtt, powhegTruthMtt));

      if (!passPowhegTruthMtt) return EL::StatusCode::SUCCESS;

      m_histSvc->BookFillHist("MttTruth", 100, 0, 5000, powhegTruthMtt / 1000.,
                              m_weight);
    }

    if (m_debug)
      Info("execute ()", "Event = %llu ; Run = %u", m_eventInfo->eventNumber(),
           m_eventInfo->runNumber());

    // protection against empty containers with size > 0
    // TODO how can this happen?
    if (m_debug) Info("execute ()", "Clearing empty containers...");
    EL_CHECK("execute()", clearEmptyContainersWithNonZeroSize());

    // ----------------------------
    // fill validation histograms
    // ----------------------------
    if (m_validation) {
      EL_CHECK("AnalysisReader::execute()", m_validationFillFunction());
    }

    EL_CHECK("AnalysisReader::execute()", doOverlapRemoval(varName));

    EL_CHECK("AnalysisReader::execute()", executePreEvtSel());

    // --------------------
    // event selection
    // --------------------
    if (m_debug) Info("execute ()", "Checking the event selection...");
    bool passSel = true;

    if (m_eventSelection) {
      // all pointers are zero initialised in default constructor of SelectionContainers
      // so leaving out unused containers is okay (the corresponding pointer will not be dangling)
      SelectionContainers containers;
      containers.evtinfo = m_eventInfo;
      containers.met = m_met;
      containers.met_soft = m_met_soft;
      containers.electrons = m_electrons;
      containers.forwardelectrons = m_forwardElectrons;
      containers.photons = m_photons;
      containers.muons = m_muons;
      containers.taus = m_taus;
      containers.ditaus = m_ditaus;
      containers.jets = m_jets;
      containers.fatjets = m_fatJets;
      containers.fatjetsAlt = m_fatJetsAlt;
      containers.trackjets = m_trackJets;
      containers.subjets = m_subJets;
      containers.truthEvent = m_truthEvent;
      containers.truthParticles = m_truthParts;
      containers.truthTaus = m_truthTaus;
      containers.truthMuons = m_truthMuons;
      containers.truthElectrons = m_truthElectrons;
      containers.truthNeutrinos = m_truthNeutrinos;
      containers.truthAntiKt4TruthWZJets = m_truthWZJets;
      containers.truthAntiKt10TruthWZJets = m_truthWZFatJets;

      if (m_applyEventPreSelection) {
        m_eventSelection->setJetPtCut(m_jetPtCut);
        m_eventSelection->setJetRapidityCut(m_jetRapidityCut);
        m_eventSelection->setLepPtCut(m_lepPtCut);
        bool passSel_tmp =
            m_eventSelection->passPreSelection(containers, !isNominal);
        if (m_isMC && m_allowORwithTruthSelection) {
          // the logic here is that an event should pass if it passes either the regular reo selection
          // OR the truth selection if m_allowORwithTruthSelection is activated
          passSel_tmp |=
              m_eventSelection->passTruthPreSelection(containers, !isNominal);
        }
        passSel &=
            passSel_tmp;  // so that passSel &= (passSelTruth || passSelReco)

        // Modify the met for the MJ
        m_eventSelection->setMetMJCalc(m_metMJCalc);
      }  // end preselection

      bool passSel_tmp =
          m_eventSelection->passSelection(containers, !isNominal);
      if (m_isMC && m_allowORwithTruthSelection) {
        // the logic here is that an event should pass if it passes either the regular reo selection
        // OR the truth selection if m_allowORwithTruthSelection is activated
        passSel_tmp |=
            m_eventSelection->passTruthSelection(containers, !isNominal);
      }
      passSel &=
          passSel_tmp;  // so that passSel &= (passSelTruth || passSelReco)
    }

    if (!passSel) continue;

    if (isNominal) m_eventCountPassed++;

    // --------------------
    // fill histograms
    // --------------------
    if (m_debug) Info("execute ()", "Calling the fill function...");
    EL_CHECK("AnalysisReader::execute()", m_fillFunction());
  }

  return EL::StatusCode::SUCCESS;
}  // execute

EL::StatusCode AnalysisReader::applyVariationToTools(TString sysName) {
  CP::SystematicVariation sysVar(sysName.Data());
  CP::SystematicSet sysSet;
  sysSet.insert(sysVar);

  if (m_puReweightingTool->applySystematicVariation(sysSet) !=
      CP::SystematicCode::Ok) {
    return EL::StatusCode::FAILURE;
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::setEventWeight() {
  // reset event weight and flag for highWeight correction
  m_weight = 1.0;
  m_isHighWeight = false;

  // generator weight
  EL_CHECK("AnalysisReader::applyMCEventWeight()", applyMCEventWeight());

  // lumi weight
  EL_CHECK("AnalysisReader::setEventWeight()", applyLumiWeight());

  // PU weight (and get (corrected) mu in data...)
  EL_CHECK("AnalysisReader::setEventWeight()", applyPUWeight());

  // VZqq - Zbb events weight
  EL_CHECK("AnalysisReader::setEventWeight()", applyVZqqZbbWeight(m_mcChannel));

  // ready to return
  return EL::StatusCode::SUCCESS;
}  // done setEventWeight

EL::StatusCode AnalysisReader::applyMCEventWeight() {
  // retrieve the MC event weight
  double MCEventWeight = Props::MCEventWeight.exists(m_eventInfo)
                             ? Props::MCEventWeight.get(m_eventInfo)
                             : -999.0;
  //Automated protection against pathologically large event weights for Sherpa MC generator:
  //       https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BosonJetsFocusGroup#Treating_pathological_high_weigh
  double weightThreshold = 100.;  //this is the recommended value
  if (m_lowerShMCWeightThreshold)
    weightThreshold = 20.;  //lower threshold (atm for VHbb boosted)

  if (m_isMC && m_isSherpa) {
    if (fabs(MCEventWeight) > weightThreshold) {
      MCEventWeight /= fabs(MCEventWeight);
      m_isHighWeight = true;
      Warning("setEventWeight()",
              "Large weight protection: the MCWeight of this Sherpa event is "
              "%f (which is larger than %f) set its weight to %f",
              Props::MCEventWeight.get(m_eventInfo), weightThreshold,
              MCEventWeight);
    }
  }

  // set "nominal" weight to correct valure for powheg+pythia8 top RadHigh samples:
  // twiki: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgTopProcesses#Process_specific_recipes
  if (m_isMC) {
    if (m_mcChannel == 410467 || m_mcChannel == 410480 ||
        m_mcChannel == 410481 || m_mcChannel == 410482)
      MCEventWeight = m_EvtWeightVar->GetTtbarRadHiNewNominal(
          m_eventInfo);  // "muR = 0.5, muF = 0.5"*"Var3cUp"/Nominal
  }

  if (m_debug) {
    std::cout << "   m_mcChannel=" << m_mcChannel
              << ",  MCEventWeight=" << MCEventWeight << std::endl;
  }
  if (m_isMC) m_weight *= MCEventWeight;
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::applyExtension0LepTTbarWeight() {
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // in 0L ttbar combine regular ttbar with the MET-filtered samples
  // from truth MET (called SUSYMET)
  // 100-200 GeV: use both 410470 weighted at half and 345935 weighted at half
  // 200-300: use 407345 with full weight
  // 300-400: use 407346 with full weight
  // 400-inf: use 407347 with full weight
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::string analysisType = m_config->get<std::string>("analysisType");
  // this can only impact 0lep MC, for the others return without doing anything
  if (!(analysisType == "0lep" && m_isMC)) {
    return EL::StatusCode::SUCCESS;
  }
  double factor = 1.0;
  bool useTTBarMETFiltered0LepSamples = false;
  m_config->getif<bool>("useTTBarMETFiltered0LepSamples",
                        useTTBarMETFiltered0LepSamples);
  if (useTTBarMETFiltered0LepSamples) {
    double SUSYMET = Props::SUSYMET.get(m_eventInfo) / 1e3;
    if (m_debug) {
      std::cout << "m_mcChannel=" << m_mcChannel << " SUSYMET=" << SUSYMET
                << std::endl;
    }
    if (m_mcChannel == 410470) {
      if (SUSYMET > 200) {
        factor = 0.0;  //Discard event from nominal sample with METSUSY > 200GeV
      } else if (SUSYMET > 100) {
        factor = 0.5;
      }
    }
    if (m_mcChannel == 345935) {
      factor = 0.5;
    }
  } else {
    // to avoid mistakes when wanting only the default 410470, set the weights for the other to zero
    if (m_mcChannel == 345935)
      factor = 0;
    else if (m_mcChannel == 407345)
      factor = 0;
    else if (m_mcChannel == 407346)
      factor = 0;
    else if (m_mcChannel == 407347)
      factor = 0;
  }  // done if useTTBarMETFiltered0LepSamples
  if (m_debug) {
    std::cout << "for analysisType=" << analysisType
              << " useTTBarMETFiltered0LepSamples="
              << useTTBarMETFiltered0LepSamples << " factor=" << factor
              << std::endl;
  }
  m_weight *= factor;
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::applyExtension0LepZnunuWeight() {
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Combine the ZnunuB and high pT samples between regular filtering of max(HT,PTV) new filtering of PTV
  // it must be done as a function of event flavor: bb, bc, bl only
  // how do we retrieve this info?
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::string analysisType = m_config->get<std::string>("analysisType");
  // this can only impact 0lep MC, for the others return without doing anything
  if (!(analysisType == "0lep" && m_isMC)) {
    return EL::StatusCode::SUCCESS;
  }
  double factor = 1.0;
  bool useZnunuExtension0LepSamples = false;
  m_config->getif<bool>("useZnunuExtension0LepSamples",
                        useZnunuExtension0LepSamples);
  bool isMAXHTPTVSample = ((m_mcChannel == 364144) ||  // ZnunuB_Sh221
                           (m_mcChannel == 364147) ||  // ZnunuB_Sh221
                           (m_mcChannel == 364150) ||  // ZnunuB_Sh221
                           (m_mcChannel == 364153) ||  // ZnunuB_Sh221
                           (m_mcChannel == 364154) ||  // Znunu_Sh221
                           (m_mcChannel == 364155) ||  // Znunu_Sh221
                           false);
  bool isPTVSample = ((m_mcChannel == 366010) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 366011) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 366012) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 366013) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 366014) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 366015) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 366016) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 366017) ||  // ZnunuB_Sh221_PTV
                      (m_mcChannel == 364222) ||  // Znunu_Sh221_PTV
                      (m_mcChannel == 364223) ||  // Znunu_Sh221_PTV
                      false);
  // applicable jet flavor should be only for Zbb, Zbc, Zbl
  // to do; for now done for all
  std::string flavor = m_histNameSvc->getEventFlavour();
  if (m_debug) {
    std::cout << "event jet pair flavor=" << flavor << std::endl;
  }
  bool isApplicableJetFlavor =
      (flavor == "bb" || flavor == "bc" || flavor == "bl");
  if (useZnunuExtension0LepSamples) {
    // use the extension
    if (isApplicableJetFlavor) {
      // flat to to apply to
      if (isMAXHTPTVSample) {
        factor = 0.5;
      }
      if (isPTVSample) {
        factor = 0.5;
      }
    } else {
      // flat to not apply to
      if (isMAXHTPTVSample) {
        factor = 1.0;
      }
      if (isPTVSample) {
        factor = 0.0;
      }
    }
  } else {
    // to not apply this extensions, so keep each sample at 1.000
    // note before we were asking in this case for the PTV slices to be at 0.000
    // but we can not do it any longer, as in period e we have only PTV slices
    // and we want when the flag is false for PTV alone to have its full weight
    if (isMAXHTPTVSample) {
      factor = 1.0;
    }
    if (isPTVSample) {
      factor = 1.0;
    }
  }
  m_weight *= factor;
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::applyLumiWeight() {
  double weight = 1.;

  if (!m_isMC) {
    Props::LumiWeight.set(m_eventInfo, 1.);
    return EL::StatusCode::SUCCESS;
  }

  // return if weight not requested
  bool applyWeight = true;
  m_config->getif<bool>("applyLumiWeight", applyWeight);

  if (!applyWeight) {
    Props::LumiWeight.set(m_eventInfo, weight);
    return EL::StatusCode::SUCCESS;
  }

  // Query - reading the cross section every event.
  // This is because on the first event fileExecute and changeInput are called before initialize
  // so there is no event available to read dataset
  // m_mcChannel = (int)m_eventInfo->mcChannelNumber();
  // get sum of weights from text file
  if (!m_sumOfWeightsProvider) {
    Error("applyLumiWeight()", "SumOfWeights provider not initialized!");
    return EL::StatusCode::FAILURE;
  }

  double sumOfWeights = m_sumOfWeightsProvider->getsumOfWeights(m_mcChannel);

  //If we have a max number of events to run on we should scale the sample weight by that over the total available entries
  if (m_maxEvents > 0) {
    double availableEntries =
        m_sumOfWeightsProvider->getNEntriesSelectedOut(m_mcChannel);
    sumOfWeights *= m_maxEvents / availableEntries;
  }

  if (!m_xSectionProvider) {
    Error("applyLumiWeight()", "XSection provider not initialized!");
    return EL::StatusCode::FAILURE;
  }

  float sigmaEff = m_xSectionProvider->getXSection(m_mcChannel);

  if (m_debug)
    Info("applyLumiWeight()", "Cross section times eff. for dataset id %i = %f",
         m_mcChannel, sigmaEff);

  // we are normalising to MC lumi, sumOfWeights calculated per m_mcChannel
  weight = (sumOfWeights) ? sigmaEff / sumOfWeights : 1.0;

  // scale to desired luminosity
  weight *= (m_luminosity *
             1e3);  // the MC is scaled to 1pb-1 but m_luminosity is in fb-1

  // decorate eventInfo with the lumi weight
  Props::LumiWeight.set(m_eventInfo, weight);

  // multiply with global event weight
  m_weight *= weight;

  return EL::StatusCode::SUCCESS;
}  // applyLumiWeight

EL::StatusCode AnalysisReader::applyPUWeight() {
  if (m_computePileupReweight || m_recomputePileupReweight) {
    EL_CHECK("EventInfoHandler::executeEvent()",
             m_puReweightingTool->decorate(m_eventInfo));
  }
  if (Props::PileupReweight.exists(m_eventInfo)) {
    // CxAOD31 and newer
    if (m_recomputePileupReweight) {
      m_pileupReweight = Props::RecomputePileupReweight.get(m_eventInfo);
      m_averageMu = Props::RecomputeCorrectedAvgMu.get(m_eventInfo);
      m_actualMu = Props::RecomputeCorrectedMu.get(m_eventInfo);
      m_averageMuScaled =
          Props::RecomputeCorrectedAndScaledAvgMu.get(m_eventInfo);
      m_actualMuScaled = Props::RecomputeCorrectedAndScaledMu.get(m_eventInfo);
    } else {
      m_pileupReweight = Props::PileupReweight.get(m_eventInfo);
      m_averageMu = Props::CorrectedAvgMu.get(m_eventInfo);
      m_actualMu = Props::CorrectedMu.get(m_eventInfo);
      m_averageMuScaled = Props::CorrectedAndScaledAvgMu.get(m_eventInfo);
      m_actualMuScaled = Props::CorrectedAndScaledMu.get(m_eventInfo);
    }
  } else {
    // CxAOD30 and older
    m_pileupReweight = Props::Pileupweight.get(m_eventInfo);
    m_averageMu = Props::averageInteractionsPerCrossing.get(m_eventInfo);
    m_actualMu = m_averageMu;
    float OneOverSF = 1.0;     // MC
    if (!m_isMC) {             // data
      OneOverSF = 1.0 / 1.09;  // R21 (CxAOD29, CxAOD30);
      // careful but R20.7 CxAOD28 may have been different, if so, change here the value.
    }
    m_averageMuScaled = m_averageMu * OneOverSF;
    m_actualMuScaled = m_actualMu * OneOverSF;
  }
  // multiply with global event weight
  bool applyWeight = true;
  m_config->getif<bool>("applyPUWeight", applyWeight);
  if (applyWeight) m_weight *= m_pileupReweight;  //to bring back

  return EL::StatusCode::SUCCESS;
}  // applyPUWeight

EL::StatusCode AnalysisReader::checkEventDuplicates() {
  // test nominal only
  if (!m_histNameSvc->get_isNominal()) {
    return EL::StatusCode::SUCCESS;
  }

  // count the event
  long int run = 0;
  if (m_isMC) {
    run = m_mcChannel;
  } else {
    run = m_eventInfo->runNumber();
  }
  long int evt = m_eventInfo->eventNumber();
  int count = 1;
  if (m_eventCountDuplicates.count(run)) {
    if (m_eventCountDuplicates[run].count(evt)) {
      count += m_eventCountDuplicates[run][evt];
    }
  }
  m_eventCountDuplicates[run][evt] = count;

  // check it
  if (count > 1) {
    Warning("checkEventDuplicates()", "Have %i counts for run %li event %li.",
            count, run, evt);
    if (m_failEventDuplicates) {
      Error("checkEventDuplicates()", "This is unacceptable! Exiting.");
      return EL::StatusCode::FAILURE;
    }
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::checkSherpaTruthPt(bool &pass,
                                                  float &sherpaTruthPt) {
  // determine if it is a Sherpa Pt0 sample, and remove events overlapping with slice (flag set for 13TeV only)

  pass = true;
  sherpaTruthPt = -999;

  if (!m_isSherpaVJets) {
    return EL::StatusCode::SUCCESS;
  }

  const xAOD::TruthParticleContainer *truthParts =
      m_truthParticleReader->getObjects("Nominal");

  if (!truthParts) {
    Error("checkSherpaTruthPt", "Did not find truth particles!");
    return EL::StatusCode::FAILURE;
  }

  const xAOD::TruthParticle *lep0 = nullptr;
  const xAOD::TruthParticle *lep1 = nullptr;

  for (const xAOD::TruthParticle *part : *truthParts) {
    // assume leptons are the only status 3 particles in SherpaV+jets
    if (part->status() == 3) {
      if (!lep0)
        lep0 = part;
      else
        lep1 = part;
    }
  }

  if (!(lep0 && lep1)) {
    Warning("checkSherpaTruthPt", "Did not find truth leptons!");
    return EL::StatusCode::SUCCESS;
  }

  TLorentzVector V = lep0->p4() + lep1->p4();
  sherpaTruthPt = V.Pt();

  if (m_isSherpaPt0WJets && (sherpaTruthPt > 40000.)) pass = false;

  if (m_isSherpaPt0ZJets && (sherpaTruthPt > 70000.)) pass = false;

  return EL::StatusCode::SUCCESS;
}  // checkSherpaTruthPt

EL::StatusCode AnalysisReader::checkPowhegTruthMtt(bool &pass,
                                                   float &powhegTruthMtt) {
  // determine if it is a Powheg m(ttbar) inclusive sample and
  // remove events overlapping with slices

  pass = true;
  powhegTruthMtt = -1;

  if (!m_eventInfo) {
    Error("checkPowhegTruthMtt", "Did not find event info!");
    return EL::StatusCode::FAILURE;
  }

  const bool isPowhegTTbarInclusive =
      m_mcChannel == 410000 || m_mcChannel == 410009 || m_mcChannel == 410007 ||
      m_mcChannel == 410471;

  const bool isPowhegTTbarMttSlice =
      ((m_mcChannel >= 301528 && m_mcChannel <= 301532) ||
       (m_mcChannel >= 303722 && m_mcChannel <= 303726) ||
       (m_mcChannel >= 410284 && m_mcChannel <= 410288));

  const bool isPythia8 =
      m_mcChannel == 410471 || (m_mcChannel >= 410284 && m_mcChannel <= 410288);

  if (!(isPowhegTTbarInclusive || isPowhegTTbarMttSlice)) {
    return EL::StatusCode::SUCCESS;
  }

  if (!m_truthParts) {
    Error("checkPowhegTruthMtt", "Did not find truth particles!");
    return EL::StatusCode::FAILURE;
  }

  TLorentzVector ttbarVec(0., 0., 0., 0.);
  int nTopsFound = 0;

  for (auto *part : *m_truthParts) {
    // select correct top PDGID
    const bool isTop = part->pdgId() == 6 || part->pdgId() == -6;

    // select correct status code (before ISR/FSR)
    const bool statusCheck = (isPythia8 && part->status() == 62) ||
                             (!isPythia8 && part->status() == 3);

    if (isTop && statusCheck) {
      ttbarVec += part->p4();
      nTopsFound++;
    }
  }

  if (nTopsFound != 2) {
    Error("checkPowhegTruthMtt", "Found wrong number of tops: %i", nTopsFound);
    return EL::StatusCode::FAILURE;
  }

  powhegTruthMtt = ttbarVec.M();

  if (isPowhegTTbarInclusive && (powhegTruthMtt > 1.1e6)) pass = false;

  if ((!pass || isPowhegTTbarMttSlice) && (m_usePowhegInclFraction > 0) &&
      (m_usePowhegInclFraction < 1)) {
    float weight = m_usePowhegInclFraction;

    if (isPowhegTTbarMttSlice) weight = 1 - m_usePowhegInclFraction;
    m_weight *= weight;
    pass = true;
  }

  return EL::StatusCode::SUCCESS;
}  // checkPowhegTruthMtt

EL::StatusCode AnalysisReader::checkSherpaVZqqZbb(bool &pass,
                                                  std::string m_varName) {
  unsigned int nZll = 0;
  unsigned int nZantill = 0;
  unsigned int nZvv = 0;
  unsigned int nZantivv = 0;
  unsigned int nZqq = 0;
  unsigned int nZbb = 0;
  unsigned int nWlv_l = 0;
  unsigned int nWlv_v = 0;
  const xAOD::TruthParticleContainer *truthParts =
      m_truthParticleReader->getObjects(m_varName);
  for (auto *part : *truthParts) {
    if (part->status() == 11) {
      int id = part->pdgId();
      bool isZll = (id == 11 || id == 13 || id == 15);
      bool isZantill = (id == -11 || id == -13 || id == -15);
      bool isZvv = (id == 12 || id == 14 || id == 16);
      bool isZantivv = (id == -12 || id == -14 || id == -16);
      bool isWlv_l = (abs(id) == 11 || abs(id) == 13 || abs(id) == 15);
      bool isWlv_v = (abs(id) == 12 || abs(id) == 14 || abs(id) == 16);
      // to do: check the impact when adding also negative values for the id of quarks
      bool isZqq = (id == 1 || id == 2 || id == 3 || id == 4);
      bool isZbb = (id == 5);
      if (isZll) nZll++;
      if (isZantill) nZantill++;
      if (isZvv) nZvv++;
      if (isZantivv) nZantivv++;
      if (isZqq) nZqq++;
      if (isZbb) nZbb++;
      if (isWlv_l) nWlv_l++;
      if (isWlv_v) nWlv_v++;
    }
  }

  bool isZll = ((nZll == 1) && (nZantill == 1));
  bool isZvv = ((nZvv == 1) && (nZantivv == 1));
  bool isWlv = ((nWlv_l == 1) && (nWlv_v == 1));
  bool isVZbb = ((isZll || isZvv || isWlv) && nZbb == 1);
  pass = isVZbb;
  return EL::StatusCode::SUCCESS;
}  // checkSherpaVZqqZbb

EL::StatusCode AnalysisReader::applyVZqqZbbWeight(int DSID) {
  double weight = 1;
  int DSID_VZqq[3] = {363355, 363356, 363489};  //ZqqZvv, ZqqZll, ZqqWlv
  int DSID_VZbb[3] = {345043, 345044, 345045};  //ZbbZvv, ZbbZll, ZbbWlv
  double Nbb_bfliter, Nbb_inclusive;
  double Brbbqq = 0.2161;
  for (int ich = 0; ich < 3; ich++) {  // loop channels
    if (DSID == DSID_VZqq[ich]) {      //VZqq inclusive
      Nbb_inclusive =
          m_sumOfWeightsProvider->getNEntries(DSID_VZqq[ich]) * Brbbqq;
      Nbb_bfliter = m_sumOfWeightsProvider->getNEntries(DSID_VZbb[ich]);
      bool isVZbb = false;
      if (m_CxAODTag == "CxAODTag31") {
        std::string m_varName = "Nominal";
        EL_CHECK("AnalysisReader::applyVZqqZbbWeight()",
                 checkSherpaVZqqZbb(isVZbb, m_varName));
      } else if (m_CxAODTag >= "CxAODTag32") {
        isVZbb = Props::isVZbb.get(m_eventInfo);
      } else {
        Error("AnalysisReader::applyVZqqZbbWeight()",
              "For for DSID=%i the CxAODTag=%s is not known, so will ABORT!!!",
              DSID, m_CxAODTag.c_str());
        return EL::StatusCode::FAILURE;
      }
      if (isVZbb && m_applyVZbbWeight == "Weight")
        weight *= Nbb_inclusive / (Nbb_inclusive + Nbb_bfliter);
      if (isVZbb && m_applyVZbbWeight == "Veto") weight *= 0;
    }
    if (DSID == DSID_VZbb[ich]) {  //VZbb b-filter
      Nbb_inclusive =
          m_sumOfWeightsProvider->getNEntries(DSID_VZqq[ich]) * Brbbqq;
      Nbb_bfliter = m_sumOfWeightsProvider->getNEntries(DSID_VZbb[ich]);
      if (m_applyVZbbWeight == "Weight")
        weight *= Nbb_bfliter / (Nbb_inclusive + Nbb_bfliter);
      else if (m_applyVZbbWeight != "Veto")
        weight = 0;
    }
  }  // end loop channels
  m_weight *= weight;
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::fill_example() {
  // get selection results
  ResultDummy selectionResult =
      ((DummyEvtSelection *)m_eventSelection)->result();
  std::vector<const xAOD::Jet *> jets = selectionResult.jets;

  // there are also a global pointers to the current collections, e.g.
  // const xAOD::JetContainer*       m_jets;
  // but this one is not suspect to re-selection in the selection class

  // book and fill histograms
  m_histSvc->BookFillHist("jetN", 11, -0.5, 10.5, jets.size(), m_weight);

  for (const xAOD::Jet *jet : jets) {
    m_histSvc->BookFillHist("jetPt", 100, 0, 100, jet->pt() / 1e3, m_weight);
  }
  return EL::StatusCode::SUCCESS;
}  // fill_example

EL::StatusCode AnalysisReader::doOverlapRemoval(std::string varName) {
  // load overlap register container from TEvent
  EL_CHECK("AnalysisReader::execute()", m_overlapRegAcc->loadRegister(m_event));
  // decorate objects with OR decisions from register
  OverlapRegisterAccessor::Containers containers;
  //use for two exceptions the Nominal case
  //where the OR should anyways not be affected by
  //PRW_DATASF could be removed from v28 onwards
  //one more exception for METTrig uncertainty (still need with v28)
  if (m_doQCD || varName.find("PRW_DATASF") == 0 ||
      varName.find("METTrig") == 0)
    containers.variation = "Nominal";
  else
    containers.variation = varName;
  containers.jets = m_jets;
  containers.fatjets = m_fatJets;
  containers.muons = m_muons;
  containers.electrons = m_electrons;
  containers.taus = m_taus;
  containers.photons = m_photons;
  EL_CHECK("AnalysisReader::execute()",
           m_overlapRegAcc->decorateObjects(containers));
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::fill_validation() {
  m_histSvc->BookFillHist("ELECTRON_nelectrons", 11, 0, 10, m_electrons->size(),
                          m_weight);

  for (const xAOD::Electron *electron : *m_electrons) {
    m_histSvc->BookFillHist("ELECTRON_electronE", 100, 0, 200,
                            electron->e() / 1e3, m_weight);
    m_histSvc->BookFillHist("ELECTRON_electronPt", 100, 0, 200,
                            electron->pt() / 1e3, m_weight);
    m_histSvc->BookFillHist("ELECTRON_electronEta", 100, -5, 5, electron->eta(),
                            m_weight);
    m_histSvc->BookFillHist("ELECTRON_electronPhi", 100, 0, 3.15,
                            electron->phi(), m_weight);
  }

  m_histSvc->BookFillHist("MUON_nmuons", 11, 0, 10, m_muons->size(), m_weight);

  for (const xAOD::Muon *muon : *m_muons) {
    m_histSvc->BookFillHist("MUON_muonE", 100, 0, 200, muon->e() / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("MUON_muonPt", 100, 0, 200, muon->pt() / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("MUON_muonEta", 100, -5, 5, muon->eta(), m_weight);
    m_histSvc->BookFillHist("MUON_muonPhi", 100, 0, 3.15, muon->phi(),
                            m_weight);
  }

  m_histSvc->BookFillHist("TAU_ntaus", 11, 0, 10, m_taus->size(), m_weight);

  for (const xAOD::TauJet *tau : *m_taus) {
    m_histSvc->BookFillHist("TAU_tauE", 100, 0, 200, tau->e() / 1e3, m_weight);
    m_histSvc->BookFillHist("TAU_tauPt", 100, 0, 200, tau->pt() / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("TAU_tauEta", 100, -5, 5, tau->eta(), m_weight);
    m_histSvc->BookFillHist("TAU_tauPhi", 100, 0, 3.15, tau->phi(), m_weight);
  }

  m_histSvc->BookFillHist("JET_njets", 11, 0, 10, m_jets->size(), m_weight);

  for (const xAOD::Jet *jet : *m_jets) {
    m_histSvc->BookFillHist("JET_jetE", 100, 0, 200, jet->e() / 1e3, m_weight);
    m_histSvc->BookFillHist("JET_jetPt", 100, 0, 200, jet->pt() / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("JET_jetEta", 100, -5, 5, jet->eta(), m_weight);
    m_histSvc->BookFillHist("JET_jetPhi", 100, 0, 3.15, jet->phi(), m_weight);
  }

  m_histSvc->BookFillHist("MET_met", 100, 0, 200, m_met->met() / 1e3, m_weight);
  m_histSvc->BookFillHist("MET_metPx", 100, 0, 200, m_met->mpx() / 1e3,
                          m_weight);
  m_histSvc->BookFillHist("MET_metPy", 100, 0, 200, m_met->mpy() / 1e3,
                          m_weight);
  if (m_met_soft != nullptr) {
    m_histSvc->BookFillHist("METSoft_met", 100, 0, 200, m_met_soft->met() / 1e3,
                            m_weight);
    m_histSvc->BookFillHist("METSoft_metPx", 100, 0, 200,
                            m_met_soft->mpx() / 1e3, m_weight);
    m_histSvc->BookFillHist("METSoft_metPy", 100, 0, 200,
                            m_met_soft->mpy() / 1e3, m_weight);
  }
  return EL::StatusCode::SUCCESS;
}  // fill_validation

EL::StatusCode AnalysisReader::postExecute() {
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.

  // clear event in object readers
  for (ObjectReaderBase *reader : m_objectReader) {
    reader->clearEvent();
  }

  return EL::StatusCode::SUCCESS;
}  // postExecute

EL::StatusCode AnalysisReader::finalize() {
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.

  Info("finalize()", "Finalizing job.");

  EL_CHECK("finalize", finalizeTools());

  if (m_eventSelection) {
    CutFlowCounter counter = m_eventSelection->getCutFlowCounter();
    TH1D *cutflowHisto_preselection = counter.getCutFlow("CutFlow/");
    wk()->addOutput(cutflowHisto_preselection);
  }

  Info("finalize()", "Processed events         = %li", m_eventCounter);
  Info("finalize()", "Passed nominal selection = %li", m_eventCountPassed);

  return EL::StatusCode::SUCCESS;
}  // finalize

EL::StatusCode AnalysisReader::histFinalize() {
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.

  Info("histFinalize()", "Finalizing histograms.");

  // a Tree with the luminosity
  m_histSvc->BookFillTree("MetaData", "luminosity", &m_luminosity,
                          "m_luminosity");

  m_histSvc->Write(wk());
  m_truthHistSvc->Write(wk());

  return EL::StatusCode::SUCCESS;
}  // histFinalize

float AnalysisReader::computeBTagSFWeight(
    const std::vector<const xAOD::Jet *> &signalJets,
    const std::string &authorName) {
  if (m_debug) Info("computeBTagSFWeight()", "Called.");

  if (!m_isMC) {
    return 1;
  }

  bool nominalOnly = false;
  m_config->getif<bool>("nominalOnly", nominalOnly);

  bool truth_tag = false;
  m_config->getif<bool>("doTruthTagging", truth_tag);
  float weight = 1.0;
  m_bTagTool->setJetAuthor(authorName);
  //bool doWeightVar{m_bTagTool->doWeightVar()};

  bool isNominal{(m_currentVar == "Nominal")};
  bool doWeightVar = (!nominalOnly && isNominal) ? true : false;

  if (m_debug)
    std::cout << "isNominal " << isNominal << " m_currentVar " << m_currentVar
              << "  doWeightVar " << doWeightVar << std::endl;

  m_bTagTool->setWeightVar(doWeightVar);
  auto btagEffSFs =
      (truth_tag ? m_bTagTool->computeEventWeight_truthTag(m_config)
                 : m_bTagTool->computeEventWeight(signalJets));
  //  m_bTagTool->setWeightVar(doWeightVar);

  weight = btagEffSFs["Nominal"];

  //only write syst histos if nominalOnly is set to false
  if (!nominalOnly && isNominal && doWeightVar) {
    for (auto effSF : btagEffSFs) {
      std::string varName = effSF.first;
      if (varName == "Nominal") continue;
      if (authorName != "") {
        auto n = varName.rfind("__1up");
        if (n == std::string::npos) n = varName.rfind("__1down");
        if (n == std::string::npos)
          varName += "_" + authorName;
        else
          varName.insert(n, "_" + authorName);
      }
      m_weightSysts.push_back({varName, effSF.second / weight});

      // Info("computeBTagSFWeight", "Relative weight for %s = %f", effSF.first.c_str(), effSF.second/weight);
    }
  }

  // Info("computeBTagSFWeight()", "Event weight = %f", weight);
  return weight;
}  // computeBTagSFWeight

void AnalysisReader::getNeutrinoPz(double MET, double MET_phi,
                                   const TLorentzVector &lepVec, double &nu_pz1,
                                   double &nu_pz2) {
  // This code gives results identical (except using a slightly different mW) to:
  // https://svnweb.cern.ch/trac/atlasphys-exa/browser/Physics/Exotic/Analysis/DibosonResonance/
  // Data2015/VV_JJ/Code/trunk/CxAODReader_DB/Root/AnalysisReader_DB.cxx?rev=239154#L812
  double mW = 80385.;  // PDG 2014

  double el = lepVec.E();
  double ptl = lepVec.Pt();
  double pzl = lepVec.Pz();

  TLorentzVector metVec;

  metVec.SetPtEtaPhiM(MET, 0, MET_phi, 0);

  double mu = 0.5 * mW * mW + ptl * MET * cos(lepVec.DeltaPhi(metVec));
  double delta = (mu * mu * pzl * pzl / (pow(ptl, 4))) -
                 (el * el * MET * MET - mu * mu) / (ptl * ptl);

  if (delta < 0) {
    delta = 0;
  }

  nu_pz1 = (mu * pzl / (ptl * ptl)) + sqrt(delta);
  nu_pz2 = (mu * pzl / (ptl * ptl)) - sqrt(delta);
}  // getNeutrinoPz

double AnalysisReader::getNeutrinoPz(double MET, double MET_phi,
                                     const TLorentzVector &lepVec, bool min) {
  double nu_pz1;
  double nu_pz2;

  getNeutrinoPz(MET, MET_phi, lepVec, nu_pz1, nu_pz2);

  if (fabs(nu_pz1) < fabs(nu_pz2)) {
    if (min) return nu_pz1;

    return nu_pz2;
  }

  if (min) return nu_pz2;

  return nu_pz1;
}  // getNeutrinoPz

TLorentzVector AnalysisReader::getNeutrinoTLV(const TLorentzVector &metVec,
                                              const TLorentzVector &lepVec,
                                              bool min) {
  double pXNu = metVec.Px();
  double pYNu = metVec.Py();
  double pZNu = getNeutrinoPz(metVec.Pt(), metVec.Phi(), lepVec, min);
  double eNu = sqrt(pXNu * pXNu + pYNu * pYNu + pZNu * pZNu);
  TLorentzVector nuVec(pXNu, pYNu, pZNu, eNu);
  return nuVec;
}  // getNeutrinoTLV

int AnalysisReader::getGAHeavyFlavHadronLabels_leadPt(
    const xAOD::Jet *jet) const {
  std::vector<std::pair<int, float> > hadrons;
  getGAHeavyFlavHadronLabels_PtSort(jet, hadrons);

  if (hadrons.size() > 0) return hadrons[0].first;

  return 0;
}

void AnalysisReader::getGAHeavyFlavHadronLabels_PtSort(const xAOD::Jet *jet,
                                                       int &jetflav1,
                                                       int &jetflav2) const {
  std::vector<std::pair<int, float> > hadrons;
  getGAHeavyFlavHadronLabels_PtSort(jet, hadrons);

  if (hadrons.size() > 1) {
    jetflav1 = hadrons[0].first;
    jetflav2 = hadrons[1].first;
  } else if (hadrons.size() > 0) {
    jetflav1 = hadrons[0].first;
    jetflav2 = 0;
  } else {
    jetflav1 = 0;
    jetflav2 = 0;
  }
}

void AnalysisReader::getGAHeavyFlavHadronLabels_PtSort(
    const xAOD::Jet *jet, std::vector<std::pair<int, float> > &hadrons) const {
  // get hadrons
  const std::string labelB = "GhostBHadronsFinal";
  const std::string labelC = "GhostCHadronsFinal";
  const std::string labelTau = "GhostTausFinal";
  std::vector<const xAOD::IParticle *> ghostB;
  std::vector<const xAOD::IParticle *> ghostC;
  std::vector<const xAOD::IParticle *> ghostTau;
  jet->getAssociatedObjects<xAOD::IParticle>(labelB, ghostB);
  jet->getAssociatedObjects<xAOD::IParticle>(labelC, ghostC);
  jet->getAssociatedObjects<xAOD::IParticle>(labelTau, ghostTau);

  // fill vector
  hadrons.clear();
  hadrons.reserve(ghostB.size() + ghostC.size() + ghostTau.size());
  for (const xAOD::IParticle *had : ghostB)
    hadrons.push_back(std::make_pair(5, had->pt()));
  for (const xAOD::IParticle *had : ghostC)
    hadrons.push_back(std::make_pair(4, had->pt()));
  for (const xAOD::IParticle *had : ghostTau)
    hadrons.push_back(std::make_pair(15, had->pt()));

  // sort vector
  std::sort(hadrons.begin(), hadrons.end(), sortPtGAHeavyFlavHadrons);
}

bool AnalysisReader::sortPtGAHeavyFlavHadrons(std::pair<int, float> had1,
                                              std::pair<int, float> had2) {
  // sort by hadron pt
  // if two hadrons have same pt, sort by flavour (B before C before Tau)
  if (had1.second > had2.second)
    return true;
  else if (had1.second < had2.second)
    return false;
  else if (abs(had1.first) == 5)
    return true;
  else if (abs(had2.first) == 5)
    return false;
  else if (abs(had1.first) == 4)
    return true;
  else if (abs(had2.first) == 4)
    return false;
  return true;
}

EL::StatusCode AnalysisReader::getTLVFromJet(const xAOD::Jet *jet,
                                             TLorentzVector &tlv,
                                             const std::string &name4Vec) {
  xAOD::JetFourMom_t tmp = jet->jetP4(name4Vec);
  if (tmp.pt() == 0 || tmp.e() == 0) {
    //if there is no muon, 0 vector is set, so let's just skip it
    return EL::StatusCode::SUCCESS;
  }
  if (std::isnan(tmp.pt()) || std::isnan(tmp.e())) {
    Warning("getTLVFromJet", "skipping NaN vector in %s", name4Vec.c_str());
    return EL::StatusCode::SUCCESS;
  }
  if (tmp.pt() < 0 || tmp.e() < 0) {
    Warning("getTLVFromJet", "skipping negative vector in %s",
            name4Vec.c_str());
    return EL::StatusCode::SUCCESS;
  }
  tlv.SetPx(tmp.px());
  tlv.SetPy(tmp.py());
  tlv.SetPz(tmp.pz());
  tlv.SetE(tmp.e());
  // now ready to return success
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader::getBJetEnergyCorrTLV(
    const xAOD::Jet *jet, TLorentzVector &veccor, bool isFatJet,
    const std::string &bJetEnergyCorr) {
  if (m_debug) {
    std::cout << "getBJetEnergyCorrTLV jet pt=" << jet->p4().Pt()
              << " bJetEnergyCorr=" << bJetEnergyCorr << std::endl;
  }
  // We don't store the b-get energy corrected jets for systematic shifts
  // but store them only for nominal jets
  // muon-in-jet as 4-vector difference relative to GSC correction -> m
  // PtReco as factor p relative to muon-in-jet -> (j+m)*p
  // Regression as factor r relative to GSC -> j*r

  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  ////  History of what b-jet energy corrections were stored and how for different CxAODTags   //////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////

  // see here what b-jet energy corrections were stored in each CxAODTag and how
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/HSG5BJetEnergyCorr
  // History of CxAOD production: https://svnweb.cern.ch/trac/atlasoff/browser/PhysicsAnalysis/HiggsPhys/Run2/Hbb/CxAODFramework/FrameworkSub/tags
  // 26: *** development for Moriond 2017 with aim at December 2016 results ***
  // 26: Git link will come  here after tag
  // 26: Jet     OneMu, PtReco, Regression, TruthWZ
  // 26: FatJet  OneMu, AllMu, TrackMu, XbbMu, TruthWZ
  // 26: For *Mu, stored as differences with respect to Nominal 4-vectors; except TruthWZ, stored as it is; PtReco stored as factor from OneMu; Regressionas factor from Nominal
  // 28: *** development for Moriond 2017 ran in Feb 2017 with final Moriond CP recommendations results ***
  // 28: Git link will come  here after tag
  // 28: Jet     OneMu, PtReco, Regression
  // 28: FatJet  xbb
  // 28: el and mu 5 GeV instead of 4 GeV; For *Mu, stored as differences with respect to Nominal 4-vectors; PtReco stored as factor from OneMu; Regressionas factor from Nominal

  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  //// Check the correction name asked by the user is one stored for this CxAODTag //////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////

  //find what type the correction the user is asking for
  bool isNominalCorr =
      (bJetEnergyCorr == "Nominal" or bJetEnergyCorr == "GSC");  // GSC
  bool isTruth = (bJetEnergyCorr == "Truth" ||
                  bJetEnergyCorr == "TruthWZ");  // truth 4-vector only for MC
  bool isRegressionCorr = (bJetEnergyCorr == "Regression");  // Regression
  bool isMuonInJetCorr =
      ((bJetEnergyCorr.find("Mu") != string::npos) ||
       (bJetEnergyCorr == "xbb") || (bJetEnergyCorr == "hbbFR") ||
       (bJetEnergyCorr == "hbbVR"));  // muon-in-jet (one of various types)
  bool isPtRecoCorr = (bJetEnergyCorr.find("PtReco") !=
                       string::npos);  // PtReco (one of various types)
  if (isPtRecoCorr == true) {
    isMuonInJetCorr = false;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  //// find the nominal jet for this systematically variated jet ////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////

  // find jet from nominal container
  const xAOD::Jet *jetnom = 0;

  // retrieve nominal jets
  const xAOD::JetContainer *jets_nominal =
      isFatJet ? m_fatJetReader->getObjects("Nominal")
               : m_jetReader->getObjects("Nominal");
  // check if the current variation is in fact the nominal
  if ((isFatJet && m_fatJets == jets_nominal) ||
      (!isFatJet && m_jets == jets_nominal)) {
    jetnom = jet;
  } else {
    // the current jet container is not the nominal
    for (const xAOD::Jet *jettest : *jets_nominal) {
      if (Props::partIndex.get(jettest) == Props::partIndex.get(jet)) {
        jetnom = jettest;
        break;
      }
    }
  }
  // if we don't have a nominal jet, we return as failure
  if (!jetnom) return EL::StatusCode::FAILURE;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  ////  Start check as a function of correction, for each correction as a function of CxAODTag //////////
  ////  For tags 26 and higher, PtReco and Regression stored as factors /////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////

  if (m_CxAODTag >= "CxAODTag26") {
    veccor = jet->p4();
    if (isNominalCorr) {
      // if "Nominal", do nothing, we will return this
    } else if (isTruth) {
      TLorentzVector temp;
      if (m_isMC) {
        // for MC retrieve directly from the CxAOD
        EL_CHECK("getBJetEnergyCorrTLV()",
                 getTLVFromJet(jetnom, temp, bJetEnergyCorr));
      } else {
        // for data fill with an empty 4-vector, as no truth in data
      }
      veccor = temp;
    } else if (isRegressionCorr) {
      veccor *= Props::factorRegression.get(jetnom);
    } else if (isMuonInJetCorr || isPtRecoCorr) {
      // this will be the new code more generic and cleaner, so that the user passes any desired combination of type X_Y
      // where x means about the muon-in-jet correction, and y means about the PtReco
      // the _y can miss
      // we try to compute all cases of muon-in-jet and put any type of PtReco at the end
      // first step split the string by _

      TString bJetEnergyCorrTString(bJetEnergyCorr);

      if (m_debug) {
        std::cout << "bJetEnergyCorrTString before rewrite="
                  << bJetEnergyCorrTString << std::endl;
      }

      // overwrite PtReco depending on CxAOD31 (ICHEP 2018) or CxAOD32 (full Run-2)
      // so that we can run with the same Reader code on old CxAOD from ICHEP, or the new one
      // and reproduce the ICHEP result, meaning b-jet energy corrections from ICHEP
      // meaning FR and semileptonic (el or mu) and hadronic is not semileptonic
      // or the new style with VR muon non-muon
      if (bJetEnergyCorr == "PtReco") {
        if (m_CxAODTag == "CxAODTag31") {
          if (m_doICHEP) {
            bJetEnergyCorrTString = TString("OneMuFR0GeV_PtReco");
          } else {
            bJetEnergyCorrTString = TString("OneMuVR0GeV_PtRecoR21OneMu");
          }
        } else if (m_CxAODTag >= "CxAODTag32") {
          bJetEnergyCorrTString = TString("OneMuVR0GeV_PtRecoR21OneMu");
        } else {
          std::cout << "CxAODTag=" << m_CxAODTag
                    << " not known. Choose CxAODTag31, CxAODTag32 or later. "
                       "Will ABORT!!!!";
          return EL::StatusCode::FAILURE;
        }
      }

      //std::cout<<"bJetEnergyCorrTString after rewrite="<<bJetEnergyCorrTString<<std::endl;

      // split the name from before the _ (nameX), which should play the role of the muon-in-jet
      // and the name for after the _ (nameY), which should play the role of the PtReco
      // but nameX can also be empty, in which case below we will overwrite it with some default values
      // A_B -> name_X="A"  and name_Y="B"
      // B   -> name_X=""   and name_Y="B"
      TString nameX =
          bJetEnergyCorrTString(0, bJetEnergyCorrTString.First('_'));
      TString nameY =
          bJetEnergyCorrTString(bJetEnergyCorrTString.First('_') + 1,
                                bJetEnergyCorrTString.Length() + 1);
      if (m_debug)
        std::cout << "bJetEnergyCorrTString=" << bJetEnergyCorrTString
                  << " nameX=" << nameX << " nameY=" << nameY << std::endl;
      assert(nameY != "");
      TString nameL;  // lepton-in-jet
      TString nameP;  // PtReco
      if (nameX == "") {
        if (nameY.Contains("PtReco")) {
          if (m_CxAODTag == "CxAODTag31" && m_doICHEP) {
            nameL = "OneMuFR0GeV";
          } else {
            nameL = "OneMuVR0GeV";
          }
          nameP = nameY;
        } else if (nameY == "xbb") {
          nameL = "xbb";
          nameP = "";
        } else if (nameY == "hbbFR") {
          nameL = "hbbFR";
          nameP = "";
        } else if (nameY == "hbbVR") {
          nameL = "hbbVR";
          nameP = "";
        } else if (nameY.Contains("Mu")) {
          if (nameY == "OneMu") {
            if (m_CxAODTag == "CxAODTag31" && m_doICHEP) {
              nameL = "OneMuFR0GeV";
            } else {
              nameL = "OneMuVR0GeV";
            }
          } else if (nameY == "AllMu") {
            if (m_CxAODTag == "CxAODTag31") {
              nameL = "AllMuFR";
            } else {
              std::cout << "It is not CxAODTag31, so probably it is CxAODTag32 "
                           "or later which does not store AllMu, so we ABORT!!!"
                        << std::endl;
              return EL::StatusCode::FAILURE;
            }
          } else {
            nameL = nameY;
          }
          nameP = "";
        } else {
          std::cout
              << "nameX=" << nameX << " nameY=" << nameY
              << " Neither PtReco nor of known type muon-in-jet. Will ABORT!!!"
              << std::endl;
          return EL::StatusCode::FAILURE;
        }
      } else {
        assert(nameX.Contains("Mu"));
        assert(nameY.Contains("PtReco"));
        nameL = nameX;
        nameP = nameY;
      }
      if (m_debug)
        std::cout << "bJetEnergyCorrTString=" << bJetEnergyCorrTString
                  << " nameL=" << nameL << " nameP=" << nameP << std::endl;
      // calculate the lepton
      TLorentzVector leptonInJet;
      // for simple options, jet directly that from the CxAOD
      if (nameL == "xbb" || nameL == "hbbFR" || nameL == "hbbVR") {
        EL_CHECK("getBJetEnergyCorrTLV()",
                 getTLVFromJet(jetnom, leptonInJet, (string)nameL));
      } else if (nameL.Contains("OneMuFR")) {
        if (m_CxAODTag == "CxAODTag31") {
          EL_CHECK("getBJetEnergyCorrTLV()",
                   getTLVFromJet(jetnom, leptonInJet, "OneMu"));
        } else if (m_CxAODTag >= "CxAODTag32") {
          std::cout << "In CxAODTag32 or later we do not store the fixed "
                       "radius FR, so we abort!!!"
                    << std::endl;
          return EL::StatusCode::FAILURE;
        }
      } else if (nameL.Contains("OneMuVR")) {
        if (m_CxAODTag == "CxAODTag31") {
          EL_CHECK("getBJetEnergyCorrTLV()",
                   getTLVFromJet(jetnom, leptonInJet, "OneMu2"));
        } else if (m_CxAODTag >= "CxAODTag32") {
          EL_CHECK("getBJetEnergyCorrTLV()",
                   getTLVFromJet(jetnom, leptonInJet, "OneMu"));
        }
      } else if (nameL.Contains("AllMuFR")) {
        if (m_CxAODTag == "CxAODTag31") {
          EL_CHECK("getBJetEnergyCorrTLV()",
                   getTLVFromJet(jetnom, leptonInJet, "AllMu"));
        } else if (m_CxAODTag >= "CxAODTag32") {
          std::cout << "In CxAODTag32 or later we do not store the AllMuFR, so "
                       "we abort!!!"
                    << std::endl;
          return EL::StatusCode::FAILURE;
        }
      } else if (nameL.Contains("AllMuVR")) {
        if (m_CxAODTag == "CxAODTag31") {
          EL_CHECK("getBJetEnergyCorrTLV()",
                   getTLVFromJet(jetnom, leptonInJet, "AllMu2"));
        } else if (m_CxAODTag >= "CxAODTag32") {
          std::cout << "In CxAODTag32 or later we do not store the AllMuVR, so "
                       "we abort!!!"
                    << std::endl;
          return EL::StatusCode::FAILURE;
        }
      } else {
        std::cout << "name of the lepton-in-jet correction (nameL) is " << nameL
                  << ", but it should either be xbb (for fat jet) or for "
                     "regular jet it should contain eiter OneMuFR, OneMuVR, "
                     "AllMuFR, AllMuVR. Will ABORT!!!!"
                  << std::endl;
        return EL::StatusCode::FAILURE;
      }
      if (m_debug)
        std::cout << "leptonInJetPt=" << leptonInJet.Pt() << std::endl;
      // if in addition it contains a cut over several GeV, only then use it, else consider there is no muon, so set its 4-vector to zero
      if (m_debug)
        std::cout << "check if jet is semileptonic or hadronic" << std::endl;
      int nrMu = 0;
      if (Props::nrMuonInJet.exists(jetnom)) {
        nrMu = Props::nrMuonInJet.get(jetnom);
      }
      int nrEl = 0;
      if (Props::nrElectronInJet.exists(jetnom)) {
        nrEl = Props::nrElectronInJet.get(jetnom);
      }
      int nrLe = nrMu + nrEl;
      int nrMu04 = nrMu;
      if (Props::nrMuonInJet04.exists(jetnom)) {
        nrMu04 = Props::nrMuonInJet04.get(jetnom);
      }
      int nrEl04 = nrEl;
      if (Props::nrElectronInJet04.exists(jetnom)) {
        nrEl04 = Props::nrElectronInJet04.get(jetnom);
      }
      int nrLe04 = nrMu04 + nrEl04;
      if (m_debug) {
        std::cout << "Before the cut on the pT of the OneMu inside the jet, we "
                     "have nrMu="
                  << nrMu << " nrEl=" << nrEl << " nrLe=" << nrLe
                  << " leptons inside the jet, nrMu04=" << nrMu04
                  << " nrEl04=" << nrEl04 << " nrLe04=" << nrLe04
                  << " and the muon-in-jet Pt in MeV is " << leptonInJet.Pt()
                  << std::endl;
        if (nrMu == 0 && nrMu04 > 0) {
          std::cout
              << "This is an event with a grey muon if you run CxAODTag31."
              << std::endl;
        }
      }

      if (nameL.Contains("GeV")) {
        // assuming a naming convention OneMuFR12GeV, go from first R to first G to find the value
        TString cutString = nameL(nameL.First("R") + 1, nameL.Length() + 1);
        cutString = cutString(0, cutString.First("G"));
        if (m_debug) std::cout << "cutString=" << cutString << std::endl;
        float cutGeV = std::stof(cutString.Data());
        float cutMeV = cutGeV * 1000;
        if (leptonInJet.Pt() > 0 && leptonInJet.Pt() < cutMeV) {
          leptonInJet = TLorentzVector();
          nrLe -= 1;  // decrease the number of leptons by 1
        }
        if (m_debug) {
          std::cout << "After the cut on " << cutGeV
                    << " GeV on OneMu, we are left with " << nrLe
                    << " leptons inside the jet." << std::endl;
        }
      }
      assert(nrLe >= 0);

      // if in addition it has ElR, remove the electron found with DR in [VR,FR]
      if (nameL.Contains("ElR")) {
        TLorentzVector temp;
        EL_CHECK("getBJetEnergyCorrTLV()", getTLVFromJet(jetnom, temp, "ElR"));
        leptonInJet -= temp;
      }
      // if in addition it has MuR, remove the muon found with DR in [VR,FR]
      // OneMuFRMuR == OneMuVR (so you should not add MuR if you already have VR)
      if (nameL.Contains("MuR")) {
        TLorentzVector temp;
        EL_CHECK("getBJetEnergyCorrTLV()", getTLVFromJet(jetnom, temp, "MuR"));
        leptonInJet -= temp;
      }
      //do the lepton-in-jet correction
      veccor += leptonInJet;
      //if in addition we ask for PtReco, apply the PtReco correction
      if (nameP != "") {
        float jetPt = veccor.Pt() * 0.001;
        float factorPtReco = 0.0;
        if (m_debug) std::cout << "m_period=" << m_period << std::endl;
        if (nameP == "PtReco") {
          factorPtReco = Props::factorPtReco.get(jetnom);
        } else if (nameP == "PtRecoR21OneMu") {
          bool isSemileptonic = nrMu > 0;
          if (isSemileptonic)
            factorPtReco =
                m_PtRecoTool->getFactor(jetPt,
                                        "CorrectionFactor_qqZllHbb_345055_"
                                        "mc16ade_all_OneMu_semileptonic_None");
          else
            factorPtReco =
                m_PtRecoTool->getFactor(jetPt,
                                        "CorrectionFactor_qqZllHbb_345055_"
                                        "mc16ade_all_OneMu_hadronic_None");
        } else {
          std::cout << "factorPtReco of name " << nameP
                    << "  not known. Will ABORT!!!" << std::endl;
          return EL::StatusCode::FAILURE;
        }
        if (m_debug)
          std::cout << "veccor Pt=" << jetPt
                    << " GeV, factorPtReco=" << factorPtReco << std::endl;
        veccor *= factorPtReco;
      }  // done if asking for PtReco on the top of the lepton-in-jet
    } else {
      //important to make sure we updated these when we add new tags
      Error("AnalysisReader::getBJetEnergyCorrTLV(...)",
            "You are asking for %s correction that is not Nominal, not of type "
            "muon-in-jet, not of type PtReco nor of type Regression.",
            bJetEnergyCorr.c_str());
      return EL::StatusCode::FAILURE;
    }  // done if based on the correction name
  }    // end if if(m_CxAODTag>="CxAODTag26")
  //now ready to return success
  return EL::StatusCode::SUCCESS;
}  //end function

EL::StatusCode AnalysisReader::applyRtrkUncertFix(const xAOD::Jet *jet) {
  // This is a temporary fix to fat jet variations JET_Rtrk_baseline, JET_Rtrk_Modelling, JET_Rtrk_Tracking,
  // which were accidently applied twice in FatJetHandler since around January 2016 to May 11 2016.
  // This method will undo the second application of the variations by assuming that the same correction factor
  // was applied twice. There will be non-closure effects in cases where the jet migrates from one bin in the
  // correction map to another after the first correction is applied (which means that two different correction
  // factors were applied in CxAODMaker).

  // The correction done here is:
  // pT(variation) = pT(nominal) * sqrt( pT(2*variation) / pT(nominal) )
  // m(variation)  = m(nominal)  * sqrt( m(2*variation)  / m(nominal) )

  // The argument in this method is a jet from one of the problematic variations:
  // JET_Rtrk_Baseline
  // JET_Rtrk_Modelling
  // JET_Rtrk_Tracking
  // ====> The user needs to make sure only fat jets from these variations are passed to this method !!! <=====
  // (The method will check - and return an error if not.)

  // This method will attach the corrected TLorentzVector ("RtrkUncertFix_TLV") which can be retrieved later with:
  //
  // TLorentzVector jetP4 = jet->auxdata<TLorentzVector>("RtrkUncertFix_TLV");
  //
  // or:
  //
  // static SG::AuxElement::Accessor<TLorentzVector> acc("RtrkUncertFix_TLV");
  // TLorentzVector jetP4 = acc(*jet);

  // check the current variation
  if (m_currentVar.find("JET_Rtrk") == std::string::npos) {
    Error("applyRtrkUncertFix()",
          "Ooops, make sure to only pass fat jets from variations: "
          "JET_Rtrk_[Baseline/Modelling/Tracking] !!");
    return EL::StatusCode::FAILURE;
  }

  // retrieve nominal jets
  const xAOD::JetContainer *jets_nominal =
      m_fatJetReader->getObjects("Nominal");

  // find jet from nominal container
  const xAOD::Jet *jetnom = 0;
  for (const xAOD::Jet *jettest : *jets_nominal) {
    if (Props::partIndex.get(jettest) == Props::partIndex.get(jet)) {
      jetnom = jettest;
      break;
    }
  }

  // check if nominal jet is found
  if (!jetnom) {
    Error("applyRtrkUncertFix()",
          "Couldn't find nominal jet - this should never happen - something is "
          "wrong!!");
    return EL::StatusCode::FAILURE;
  }

  // pT correction squared
  double pTcorr2 = jet->pt() / jetnom->pt();
  if (pTcorr2 < 0.) {
    Error("applyRtrkUncertFix()",
          "pT correction factor squared is negative : %f", pTcorr2);
    return EL::StatusCode::FAILURE;
  }

  // mass correction squared
  double Mcorr2 = jet->m() / jetnom->m();
  if (Mcorr2 < 0.) {
    Error("applyRtrkUncertFix()",
          "mass correction factor squared is negative : %f", Mcorr2);
    return EL::StatusCode::FAILURE;
  }

  // store corrected values as decorations
  xAOD::JetFourMom_t jetP4_wrong = jet->jetP4();
  TLorentzVector jetP4_correct;
  jetP4_correct.SetPtEtaPhiM(jetnom->pt() * sqrt(pTcorr2), jetP4_wrong.Eta(),
                             jetP4_wrong.Phi(), jetnom->m() * sqrt(Mcorr2));
  static SG::AuxElement::Decorator<TLorentzVector> dec("RtrkUncertFix_TLV");
  dec(*jet) = jetP4_correct;

  // std::cout << "nominal : " << jetnom->pt() << std::endl;
  // std::cout << "before  : " << jet->pt() << std::endl;
  // std::cout << "after   : " << (jet->auxdata<TLorentzVector>("RtrkUncertFix_TLV")).Pt() << std::endl;
  // std::cout << "correction factors (Pt,M) : " << sqrt(pTcorr2) << " " << sqrt(Mcorr2) << std::endl;

  // all is good
  return EL::StatusCode::SUCCESS;
}

//combMass begin

EL::StatusCode AnalysisReader::combMassTmp1(bool & /*isCombMassSyst*/,
                                            std::string & /*varName*/) {
  return EL::StatusCode::SUCCESS;
}  // combMassTmp1

EL::StatusCode AnalysisReader::combMassTmp2(bool & /*isCombMassSyst*/) {
  return EL::StatusCode::SUCCESS;
}  // combMassTmp2

//combMass end

// checkMCEventWeight begin
bool AnalysisReader::checkMCEventWeight() {
  // Method to check the insanley large MC event weights affecting the ggZllHbb / ggZvvHbb / ggZllHcc / ggZvvHcc samples (see: https://indico.cern.ch/event/799812/contributions/3323526/attachments/1799161/2934022/intro_hbb_19-02-20.pdf )
  // Method check if MC sample DSID == ggZllHbb / ggZvvHbb / ggZllHcc / ggZvvHcc and if MCEventWeight is insanely large -> returns true
  // For all the rest -> returns false
  // Samples to be corrected
  // ggZllHbb MC16d -> scaled with MC16a
  // ggZvvHbb MC16e -> scaled with MC16a
  // ggZllHcc MC16e -> scaled with MC16a
  // ggZvvHcc MC16e -> scaled with MC16a
  if (m_isMC) {
    if ((m_mcChannel == 345057 || m_mcChannel == 345058 ||
         m_mcChannel == 345113 || m_mcChannel == 345114)) {
      double MCEventWeight = Props::MCEventWeight.exists(m_eventInfo)
                                 ? Props::MCEventWeight.get(m_eventInfo)
                                 : -999.0;
      if (m_debug) {
        std::cout << "   m_mcChannel=" << m_mcChannel
                  << ",  MCEventWeight=" << MCEventWeight << std::endl;
      }
      if (std::log(MCEventWeight) > 0) {
        Warning("checkMCEventWeight",
                "The MC Event weight %f for DSID %d is too large, discarding "
                "the event!",
                MCEventWeight, m_mcChannel);

        return true;
      }
    }
  }
  return false;
}

// checkMCEventWeight end
