/**
 * @class KF::FitProcessor
 * @brief A base class to process and run a kinematic likelihood fit
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

// Local includes
#include "KinematicFit/FitProcessor.h"
#include "TFile.h"

// The Global This
class KF::FitProcessor* gThis;

// Default constructor
//_____________________________________________________________________________________________________________
KF::FitProcessor::FitProcessor()
    : fIsInitialized(false),
      fDatadir(""),
      fIsJetResponseLoaded(false),
      fFitParticle(NULL),
      fFitParticleSet(new std::vector<FitParticle*>(0)),
      fIsBaselineCheckRequired(true),
      fMinuitErrorFlag(0),
      // fApplyLHDPtRecoCorr(false),//this was truth pT(j) distribution
      fNParticles(0),
      fNLeptons(0),
      fNJets(0),
      fNFitParameters(0),
      fMinuit(NULL),
      fMinuitPrintLevel(-1),
      fLHDIterStep(0),
      fJetTransferFunction_woutmu(0),
      fJetTransferFunction_withmu(0),
      fJetTransferFunction_worwmu(0),
      // fPtRecoCorrection_Lead2(0),
      // fPtRecoCorrection_Lead2_semilep(0),
      // fPtRecoCorrection_Lead(0),
      // fPtRecoCorrection_Lead_semilep(0),
      // fPtRecoCorrection_SubLead(0),
      // fPtRecoCorrection_SubLead_semilep(0),
      // fPtRecoCorrection_Other(0),
      // fPtRecoCorrection_Baseline(NULL),
      fPtRecoCorrection_Baseline_withmu(NULL),
      fPtRecoCorrection_Baseline_woutmu(NULL),
      fLHD(NULL),
      fNLHDTerms(0),
      fConstraintType(0),
      fConstraintValue(0),
      fLHD_initial(0),
      fLHD_final(0),
      fMinuit_LHD(_DF_),
      fMinuit_EDM(_DF_),
      fMinuit_ERRDEF(_DF_),
      fMinuit_NVPAR(0),
      fMinuit_NPARX(0),
      fMinuit_ICSTAT(0),
      fIsFitConverged(false),
      fGlobPrefitValues(NULL),
      fGlobPostfitValues(NULL),
      fNGlobFitValues(0) {
  Initialize();
}  // constructor

// Initialize
//_____________________________________________________________________________________________________________
void KF::FitProcessor::Initialize() {
  me = new LogDef("FitProcessor", INFO);

  fMinuitArglist[0] = 50000;
  fMinuitArglist[1] = 1;  // 0.01

  fIsInitialized = true;
  // Log(me).Write(VERBOSE) << "initialized";
}  // Initialize

// Default destructor
//_____________________________________________________________________________________________________________
KF::FitProcessor::~FitProcessor() {
  // Log(me).Write(VERBOSE) << "destructor";

  if (fGlobPrefitValues) delete fGlobPrefitValues;
  if (fGlobPostfitValues) delete fGlobPostfitValues;
  if (fMinuit) delete fMinuit;
  if (fLHD) delete fLHD;
  if (me) delete me;
}  // destructor

// Load jet response histograms to runtime vectors
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::LoadJetResponse(const std::string TFFileName) {
  // Log(me).Write(VERBOSE) << "LoadJetResponse()";

  // OPEN DATA FILES
  TFile* TFFile = new TFile((fDatadir + TFFileName).c_str(), "READ");
  if (TFFile->IsZombie()) {
    Log(me).Write(ERROR) << "LoadJetResponse(): Failed opening "
                            "JetTransferFunctions.root file. Abort!";
    return Status::ERROR;
  }

  // TFile* PtRecoFile = new
  // TFile((fDatadir+"JetPtRecoCorrection.root").c_str(),"READ");//this was
  // truth pT(j) distribution if(PtRecoFile -> IsZombie()) {
  // Log(me).Write(ERROR) << "LoadJetResponse(): Failed opening
  // JetPtRecoCorrection.root file. Abort!"; return Status::ERROR;
  //}

  // TFile* PtRecoBaseFile = new
  // TFile((fDatadir+"histos_llbb_OneMu_Parton.root").c_str(),"READ");//Moriond
  // 2016
  TFile* PtRecoBaseFile = new TFile(
      (fDatadir + "PtReco_histos_llbb_OneMu_TruthWZ_True.root").c_str(),
      "READ");  // ICHEP 2016
  if (PtRecoBaseFile->IsZombie()) {
    Log(me).Write(ERROR) << "LoadJetResponse(): Failed opening "
                            "histos_llbb_OneMu_Parton.root file. Abort!";
    return Status::ERROR;
  }

  // FILL JET TRANSFER FUNCTION VECTORS
  std::vector<TH1F*> TF_Eta0, TF_Eta1;

  TString TF_Type = "PDF";
  TString TF_PtRegion = "";
  int TF_PtBin[9] = {0, 30, 40, 50, 60, 70, 80, 100, 10000};
  TString TF_JetSel[2] = {"lead2", "other"};
  TString TF_SemilepSel[3] = {"woutmu", "withmu", "worwmu"};

  // Loop over TF_SemilepSel
  for (int iii = 0; iii < 3; iii++) {
    // Clear vectors
    TF_Eta0.clear();
    TF_Eta1.clear();

    // Loop over pT bins
    for (int low = 0; low <= 7; low++) {
      if (TF_SemilepSel[iii] == "other") break;

      int high = low + 1;
      TF_PtRegion.Form("pt%i_%i", TF_PtBin[low], TF_PtBin[high]);

      // TString basename =
      // TF_Type+"/H"+TF_Type+"_TF_"+TF_SemilepSel[iii]+"_"+TF_PtRegion;
      TString basename =
          "h1_pdf/h1_pdf_tf_wz_" + TF_SemilepSel[iii] + "_" + TF_PtRegion;
      TString n_eta0 = basename + "_eta0_12";
      TString n_eta1 = basename + "_eta12_ff";

      // Load histograms
      TH1F* HPDF_Eta0 = (TH1F*)TFFile->Get(n_eta0);
      TH1F* HPDF_Eta1 = (TH1F*)TFFile->Get(n_eta1);

      // Check pointers (explicit logging)
      if (!HPDF_Eta0)
        Log(me).Write(ERROR) << "LoadJetResponse(): Failed loading jet "
                                "transfer PDF histogram:\n> "
                             << n_eta0;
      if (!HPDF_Eta1)
        Log(me).Write(ERROR) << "LoadJetResponse(): Failed loading jet "
                                "transfer PDF histogram:\n> "
                             << n_eta1;

      // Fill vectors
      TF_Eta0.push_back(HPDF_Eta0);
      TF_Eta1.push_back(HPDF_Eta1);

      TF_PtRegion = "";
    }  // loop over pT bins

    // Fill TF vectors
    if (TF_SemilepSel[iii] == "woutmu") {
      fJetTransferFunction_woutmu.push_back(TF_Eta0);
      fJetTransferFunction_woutmu.push_back(TF_Eta1);
    } else if (TF_SemilepSel[iii] == "withmu") {
      fJetTransferFunction_withmu.push_back(TF_Eta0);
      fJetTransferFunction_withmu.push_back(TF_Eta1);
    } else if (TF_SemilepSel[iii] == "worwmu") {
      fJetTransferFunction_worwmu.push_back(TF_Eta0);
      fJetTransferFunction_worwmu.push_back(TF_Eta1);
    } else {
      std::cout << TF_SemilepSel[iii] << std::endl;
      Log(me).Write(ERROR) << "LoadJetResponse(): BUG report > unknown jet "
                              "selection type. Abort!";
      return Status::ERROR;
    }
  }  // loop over TF_SemilepSel

  /*
  //this was truth pT(j) distribution
  //FILL PTRECO VECTORS
  //Loop over jet selection
  for(int jSel=0; jSel < 2; jSel++) {

    TString basename =
  TF_Type+"/H"+TF_Type+"_KF_Jet-pt_"+TF_JetSel[jSel]+"_inclusive"; TString
  n_eta0         = basename + "_eta0-1.2"; TString n_eta1         = basename +
  "_eta1.2-ff"; TString n_eta0_semilep = basename + "_eta0-1.2_semilep"; TString
  n_eta1_semilep = basename + "_eta1.2-ff_semilep";

    //Load histograms
    TH1F* HPDF_Eta0         = (TH1F*) PtRecoFile -> Get(n_eta0);
    TH1F* HPDF_Eta1         = (TH1F*) PtRecoFile -> Get(n_eta1);
    TH1F* HPDF_Eta0_semilep = (TH1F*) PtRecoFile -> Get(n_eta0_semilep);
    TH1F* HPDF_Eta1_semilep = (TH1F*) PtRecoFile -> Get(n_eta1_semilep);

    //Check pointers (explicit logging)
    if(!HPDF_Eta0)             Log(me).Write(ERROR) << "LoadJetResponse():
  Failed loading PtReco PDF histogram:\n> " << n_eta0; if(!HPDF_Eta1)
  Log(me).Write(ERROR) << "LoadJetResponse(): Failed loading PtReco PDF
  histogram:\n> " << n_eta1; if(TF_JetSel[jSel]!="other") {
      if(!HPDF_Eta0_semilep) Log(me).Write(ERROR) << "LoadJetResponse(): Failed
  loading PtReco PDF histogram:\n> " << n_eta0_semilep; if(!HPDF_Eta1_semilep)
  Log(me).Write(ERROR) << "LoadJetResponse(): Failed loading PtReco PDF
  histogram:\n> " << n_eta1_semilep;
    }

    //Fill leading + subleading PtReco vectors
    if(TF_JetSel[jSel]=="lead2") {
      fPtRecoCorrection_Lead2.push_back(HPDF_Eta0);
      fPtRecoCorrection_Lead2.push_back(HPDF_Eta1);
      fPtRecoCorrection_Lead2_semilep.push_back(HPDF_Eta0_semilep);
      fPtRecoCorrection_Lead2_semilep.push_back(HPDF_Eta1_semilep);
    }
    //Fill leading PtReco vectors    --> disabled in this tag version
    else if(TF_JetSel[jSel]=="lead") {
      //fPtRecoCorrection_Lead.push_back(HPDF_Eta0);
      //fPtRecoCorrection_Lead.push_back(HPDF_Eta1);
      //fPtRecoCorrection_Lead_semilep.push_back(HPDF_Eta0_semilep);
      //fPtRecoCorrection_Lead_semilep.push_back(HPDF_Eta1_semilep);
    }
    //Fill subleading PtReco vectors --> disabled in this tag version
    else if(TF_JetSel[jSel]=="sub") {
      //fPtRecoCorrection_SubLead.push_back(HPDF_Eta0);
      //fPtRecoCorrection_SubLead.push_back(HPDF_Eta1);
      //fPtRecoCorrection_SubLead_semilep.push_back(HPDF_Eta0_semilep);
      //fPtRecoCorrection_SubLead_semilep.push_back(HPDF_Eta1_semilep);
    }
    //Fill other (3rd jet) PtReco vectors
    else if(TF_JetSel[jSel]=="other") {
      fPtRecoCorrection_Other.push_back(HPDF_Eta0);
      fPtRecoCorrection_Other.push_back(HPDF_Eta1);
    }
    else {
      Log(me).Write(ERROR) << "LoadJetResponse(): BUG report > unknown jet
  selection type. Abort!"; return Status::ERROR;
    }
  }//loop over jet selection
  */

  // PT RECO BASELINE

  // TString n_baseline = "Correction";
  // fPtRecoCorrection_Baseline = (TH1F*) PtRecoBaseFile -> Get(n_baseline);
  // if(!fPtRecoCorrection_Baseline) Log(me).Write(ERROR) << "LoadJetResponse():
  // Failed loading PtReco base histogram:\n> " << n_baseline;

  // fPtRecoCorrection_Baseline_withmu = (TH1F*) PtRecoBaseFile ->
  // Get("PtReco_mu_Bukin");//Moriond 2016
  fPtRecoCorrection_Baseline_withmu =
      (TH1F*)PtRecoBaseFile->Get("PtReco_semileptonic_None");  // ICHEP 2016
  if (!fPtRecoCorrection_Baseline_withmu) {
    Log(me).Write(ERROR)
        << "LoadJetResponse(): Failed loading PtReco base histogram:\n> ";
  }
  // fPtRecoCorrection_Baseline_woutmu = (TH1F*) PtRecoBaseFile ->
  // Get("PtReco_nosld_Bukin");//Moriond 2016
  fPtRecoCorrection_Baseline_woutmu =
      (TH1F*)PtRecoBaseFile->Get("PtReco_hadronic_None");  // ICHEP 2016
  if (!fPtRecoCorrection_Baseline_woutmu) {
    Log(me).Write(ERROR)
        << "LoadJetResponse(): Failed loading PtReco base histogram:\n> ";
  }

  // fPtRecoCorrection_Baseline_withmu = (TH1F*) TFFile ->
  // Get("h1_peak/h1_peak_wz_withmu_eta0_ff");
  // if(!fPtRecoCorrection_Baseline_withmu) {
  // Log(me).Write(ERROR) << "LoadJetResponse(): Failed loading PtReco base
  // histogram:\n> ";
  //}
  // fPtRecoCorrection_Baseline_woutmu = (TH1F*) TFFile ->
  // Get("h1_peak/h1_peak_wz_woutmu_eta0_ff");
  // if(!fPtRecoCorrection_Baseline_woutmu){
  // Log(me).Write(ERROR) << "LoadJetResponse(): Failed loading PtReco base
  // histogram:\n> ";
  //}

  // NO ERROR
  fIsJetResponseLoaded = true;
  return Status::SUCCESS;
}  // LoadJetResponse

// Run kinematic likelihood fit
//_____________________________________________________________________________________________________________
int KF::FitProcessor::Fit() {
  // Log(me).Write(VERBOSE) << "Fit()";

  // Check fit setup
  if (!VerifyFitSetup()) {
    Log(me).Write(ERROR) << "Fit(): fit setup verification failed. Abort!";
    return Status::ERROR;
  }

  // Apply jet scale correction
  // ApplyJetScaleCorrection();

  // Initialize minimizer
  if (!InitMinuit()) {
    Log(me).Write(ERROR) << "Fit(): Minuit initialization failed. Abort!";
    return Status::ERROR;
  }

  // Run LHD minimization with MIGRAD algorithm
  fMinuit->mnexcm("MIGRAD", fMinuitArglist, 2, fMinuitErrorFlag);

  // Publish fit results
  PublishFit();

  // No error
  return Status::SUCCESS;
}  // Fit

// Verify required minimal fit settings
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::VerifyFitSetup() {
  // Log(me).Write(VERBOSE) << "VerifyFitSetup()";

  // Retrieve counters
  fNParticles = GetNParticles();
  fNLeptons = GetNLeptons();
  fNJets = GetNJets();
  fNFitParameters = GetNFitParameters();
  fNLHDTerms = GetNLHDTerms();

  // Particle check
  if (!(fNParticles > 0)) {
    Log(me).Write(ERROR)
        << "VerifyFitSetup(): number of particles is 0. Abort!";
    return Status::ERROR;
  }

  // Parameter check
  if (!(fNFitParameters > 0)) {
    Log(me).Write(ERROR)
        << "VerifyFitSetup(): number of fit parameters is 0. Abort!";
    return Status::ERROR;
  }

  // Setup LHD
  if (fLHD) delete fLHD;
  fLHD = new double[fNLHDTerms];

  // Jet response
  if (!fIsJetResponseLoaded) {
    Log(me).Write(ERROR) << "VerifyFitSetup(): jet response not loaded. Abort!";
    return Status::ERROR;
  }

  if (fIsBaselineCheckRequired) {
    // Setup global fit results [limited in this tag version]
    fNGlobFitValues = 5;
    fGlobPrefitValues = new double[fNGlobFitValues];
    fGlobPostfitValues = new double[fNGlobFitValues];

    // Toggle global flag
    fIsBaselineCheckRequired = false;
  }

  // Reset runtime variables
  if (!Reset()) {
    Log(me).Write(ERROR)
        << "VerifyFitSetup(): runtime variable reset failed. Abort!";
    return Status::ERROR;
  }

  // No error
  return Status::SUCCESS;
}  // VerifyFitSetup

// Find a particle with a given name
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::FindParticle(std::string name) {
  // Log(me).Write(VERBOSE) << "FindParticle()";

  bool found(false);

  // Sanity check
  if (GetNParticles() == 0) return found;

  // Loop over all particles
  for (int i = 0; i < GetNParticles(); i++) {
    if ((fFitParticleSet->at(i))->GetName() == name) {
      found = true;
      break;
    }
  }

  return found;
}  // FindParticle

// Retrieve the container index of a particle
//_____________________________________________________________________________________________________________
int KF::FitProcessor::GetParticleIndex(std::string name) {
  // Log(me).Write(VERBOSE) << "GetParticleIndex()";

  int idx(-1);

  // Sanity check
  if (!FindParticle(name)) {
    Log(me).Write(ERROR) << "GetParticleIndex(): particle <" << name
                         << "> not found. Return " << idx;
    return idx;
  }

  // Loop over all particles
  for (int i = 0; i < GetNParticles(); i++) {
    if ((fFitParticleSet->at(i))->GetName() == name) {
      idx = i;
      break;
    }
  }

  return idx;
}  // GetParticleIndex

// Retrieve the number of particles of a given type
//_____________________________________________________________________________________________________________
int KF::FitProcessor::GetNParticles(ParticleType::Particle ptype) const {
  // Log(me).Write(VERBOSE) << "GetNParticles()";

  int nParticles = 0;

  // Loop over all particles
  for (int i = 0; i < GetNParticles(); i++) {
    if ((fFitParticleSet->at(i))->GetType() == ptype) {
      nParticles++;
    }
  }

  return nParticles;
}  // GetNParticles

// Retrieve the number of leptons
//_____________________________________________________________________________________________________________
int KF::FitProcessor::GetNLeptons() {
  // Log(me).Write(VERBOSE) << "GetNLeptons()";

  int nLeps = 0;

  // Loop over all particles
  for (int i = 0; i < int(GetNParticles()); i++) {
    if ((fFitParticleSet->at(i))->GetType() <= ParticleType::Muon) {
      nLeps++;
    }
  }

  return nLeps;
}  // GetNLeptons

// Retrieve the number of jets
//_____________________________________________________________________________________________________________
int KF::FitProcessor::GetNJets() {
  // Log(me).Write(VERBOSE) << "GetNJets()";

  int nJets = 0;

  // Loop over all particles
  for (int i = 0; i < int(GetNParticles()); i++) {
    if ((fFitParticleSet->at(i))->GetType() >= ParticleType::Jet) {
      nJets++;
    }
  }

  return nJets;
}  // GetNJets

// Retrieve the number of fit parameters for a given particle type
//_____________________________________________________________________________________________________________
int KF::FitProcessor::GetNFitParameters(ParticleType::Particle ptype) {
  // Log(me).Write(VERBOSE) << "GetNFitParameters()";

  // Particle Check
  if (!(GetNParticles() > 0)) {
    Log(me).Write(ERROR)
        << "GetNFitParameters(): number of particles 0. Return 0!";
    return 0;
  }

  // Temp counter
  int nParam = 0;

  // Loop over all particles
  for (int i = 0; i < fNParticles; i++) {
    if ((fFitParticleSet->at(i))->GetType() == ptype) {
      nParam += (fFitParticleSet->at(i))->GetNFitParameters();
    }
  }

  return nParam;
}  // GetNFitParameters

// Retrieve the total number of fit parameters
//_____________________________________________________________________________________________________________
int KF::FitProcessor::GetNFitParameters() {
  // Log(me).Write(VERBOSE) << "GetNFitParameters()";

  // Particle Check
  if (!(GetNParticles() > 0)) {
    Log(me).Write(ERROR)
        << "GetNFitParameters(): number of particles 0. Return 0!";
    return 0;
  }

  // Temp counter
  int nParam = 0;

  // Loop over all particles
  for (int i = 0; i < fNParticles; i++) {
    nParam += (fFitParticleSet->at(i))->GetNFitParameters();
  }

  return nParam;
}  // GetNFitParameters

// Retrieve the number of terms in the likelihood function
//_____________________________________________________________________________________________________________
int KF::FitProcessor::GetNLHDTerms() {
  // Log(me).Write(VERBOSE) << "GetNLHDTerms()";

  int nTerms = 0;

  // Number of fit parameters
  nTerms = GetNFitParameters();

  // PtReco correction // this was truth pT(j) distribution
  // if(fApplyLHDPtRecoCorr) nTerms += fNJets;

  // Number of constraints
  int nConstraints = int(fConstraintType.size());
  if (nConstraints > 0) {
    int nBW_V = 0;
    int nBW_H = 0;
    for (int c = 0; c < nConstraints; c++) {
      if (fConstraintType[c] == ConstraintType::Vmass) nBW_V++;
      if (fConstraintType[c] == ConstraintType::Vwidth) nBW_V++;
      if (fConstraintType[c] == ConstraintType::Hmass) nBW_H++;
      if (fConstraintType[c] == ConstraintType::Hwidth) nBW_H++;
    }
    if (nBW_V == 2) nConstraints -= 1;
    if (nBW_H == 2) nConstraints -= 1;
  }

  // Total number of terms
  nTerms += nConstraints;

  return nTerms;
}  // GetNLHDTerms

// Retrieve RMS of jet probability histogram for given pT,Eta
//_____________________________________________________________________________________________________________
double KF::FitProcessor::GetJetResponseRMS(ParticleType::Particle ptype,
                                           int ptypePos, double pt,
                                           double eta) {
  // Log(me).Write(VERBOSE) << "GetJetResponseRMS()";

  // Obtain histogram
  TH1F* hist = GetJetProbabilityHist(ptype, ptypePos, pt, eta);

  // Check pointer
  if (!hist) {
    Log(me).Write(ERROR)
        << "GetJetResponseRMS(): jet histogram could not be obtained. Abort!";
    return 0;
  }

  // Return RMS
  return hist->GetRMS();
}  // GetJetResponseRMS

// Add constraint to fit
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::AddConstraint(ConstraintType::Constraint type,
                                     double value) {
  // Log(me).Write(VERBOSE) << "AddConstraint()";

  // Check constraint vector
  int nConstraints = int(fConstraintType.size());
  if (nConstraints > 0) {
    for (int c = 0; c < nConstraints; c++) {
      if (fConstraintType[c] == type) {
        Log(me).Write(ERROR)
            << "AddConstraint(): constraint already defined. Abort!";
        return Status::ERROR;
      }
    }
  }

  // Add constraint
  fConstraintType.push_back(type);
  fConstraintValue.push_back(value);

  // No error
  return Status::SUCCESS;
}  // AddConstraint

// Remove all constraints
//_____________________________________________________________________________________________________________
void KF::FitProcessor::RemoveConstraints() {
  // Log(me).Write(VERBOSE) << "RemoveConstraints()";

  fConstraintType.clear();
  fConstraintValue.clear();
}  // RemoveConstraints

// Apply jet scale correction from resolution
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::ApplyJetScaleCorrection() {
  // Log(me).Write(VERBOSE) << "ApplyJetScaleCorrection()";

  // Check number of jets
  if (fNJets < 1) {
    Log(me).Write(WARNING)
        << "ApplyJetScaleCorrection(): number of jets is 0. Abort!";
    return Status::ERROR;
  }

  // Loop over all jets
  for (int i = 0; i < fNParticles; i++) {
    if ((fFitParticleSet->at(i))->GetType() >= ParticleType::BJet) {
      // Obtain information
      ParticleType::Particle ptype = (fFitParticleSet->at(i))->GetType();
      int ptypePos = (fFitParticleSet->at(i))->GetTypePosition();
      double pt = (fFitParticleSet->at(i))->Pt();
      double eta = (fFitParticleSet->at(i))->Eta();
      double E = (fFitParticleSet->at(i))->E();

      TH1F* hist = GetJetProbabilityHist(ptype, ptypePos, pt, eta);

      pt /= hist->GetMean();
      E /= hist->GetMean();

      (fFitParticleSet->at(i))
          ->SetFourVector(pt, eta, (fFitParticleSet->at(i))->Phi(), E);
    }
  }

  // No error
  return Status::SUCCESS;
}  // ApplyJetScaleCorrection

// Reset all runtime variables
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::Reset() {
  // Log(me).Write(VERBOSE) << "Reset()";

  // Note: particle container has to be flushed by FitManager!

  // if(fMinuit) delete fMinuit;
  fMinuitErrorFlag = 0;

  for (int q = 0; q < fNLHDTerms; q++) {
    fLHD[q] = 0.0;
  }
  fLHDIterStep = 0;
  fLHD_initial = 0;
  fLHD_final = 0;

  fMinuit_LHD = _DF_;
  fMinuit_EDM = _DF_;
  fMinuit_ERRDEF = _DF_;
  fMinuit_NVPAR = -999;
  fMinuit_NPARX = -999;
  fMinuit_ICSTAT = -999;
  fIsFitConverged = false;

  for (int q = 0; q < fNGlobFitValues; q++) {
    fGlobPrefitValues[q] = _DF_;
    fGlobPostfitValues[q] = _DF_;
  }

  // No error
  return Status::SUCCESS;
}  // Reset

// Initialize TMinuit
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::InitMinuit() {
  // Log(me).Write(VERBOSE) << "InitMinuit()";

  if (fIsBaselineCheckRequired) {
    Log(me).Write(ERROR) << "InitMinuit(): inproper fit setup! Please call "
                            "Fit() or VerifyFitSetup() to proceed. Abort!";
    return Status::ERROR;
  }

  // Set global this
  gThis = this;

  // Define Minuit
  if (fMinuit) delete fMinuit;
  fMinuit = new TMinuit(fNFitParameters);

  // Set print level
  if (fMinuitPrintLevel < -1 || fMinuitPrintLevel > 3) {
    Log(me).Write(ERROR) << "InitMinuit(): Minuit print level out of range. "
                            "Please reset to proceed. Abort!";
    return Status::ERROR;
  }
  fMinuit->SetPrintLevel(fMinuitPrintLevel);

  // Set LHD function
  fMinuit->SetFCN(&KF::FitProcessor::FCNLikelihood);

  // Set UP flag [chisquared fits UP=1, negative log likelihood UP=0.5]
  fMinuit->SetErrorDef(0.5);

  // Set Minimization strategy [arg: 0, 1 (default), 2 (precise, slow)]
  int flag;
  fMinuitArglist[0] = 2;
  fMinuit->mnexcm("SET STR", fMinuitArglist, 1, flag);

  // Set, Fix and Save parameters
  int index = 0;
  std::pair<int, int> lep, jet;
  for (int i = 0; i < fNParticles; i++) {
    // GLOBAL IDX
    (fFitParticleSet->at(i))->SetIndex(i);

    for (int j = 0; j < int((fFitParticleSet->at(i))->GetNFitParameters());
         j++) {
      // SET
      fMinuit->mnparm(index, (fFitParticleSet->at(i))->GetName(),
                      (fFitParticleSet->at(i))->GetParamInitialValue(j),
                      (fFitParticleSet->at(i))->GetParamStepSize(j),
                      (fFitParticleSet->at(i))->GetParamLowerLimit(j),
                      (fFitParticleSet->at(i))->GetParamUpperLimit(j), flag);

      // FIX
      if ((fFitParticleSet->at(i))->IsParamFixed(j)) {
        fMinuit->FixParameter(index);
      }

      // SAVE
      (fFitParticleSet->at(i))->SetParamMinuitIndex(j, index);

      index++;
    }  // loop over parameters
  }    // loop over particles

  // Set maximal number of iterations
  fMinuitArglist[0] = 500000;

  // No error
  return Status::SUCCESS;
}  // InitMinuit

// Likelihood function used by Minuit
//_____________________________________________________________________________________________________________
void KF::FitProcessor::FCNLikelihood(int& /*npar*/, double* /*grad*/,
                                     double& fval, double* par, int /*flag*/) {
  // Raise iteration counter
  gThis->fLHDIterStep++;

  // Update fit vectors
  gThis->SetFitVectors(par);

  // Calculate likelihood
  fval = gThis->LHDFunction();
}  // FCNLikelihood

// Sets the fit vectors for leptons and jets given the current Minuit parameter
// list
//_____________________________________________________________________________________________________________
void KF::FitProcessor::SetFitVectors(double* par) {
  // Log(me).Write(VERBOSE) << "SetFitVectors()";

  // VARS
  FitMode::Mode mode;
  int idxEta, idxPhi, idxE, idxPt;  // Param. index
  int minEta, minPhi, minE, minPt;  // Minuit index
  bool doFit(false);

  for (int i = 0; i < fNParticles; i++) {
    // Get Mode
    mode = (fFitParticleSet->at(i))->GetFitMode();

    // Get Indices
    idxEta = (fFitParticleSet->at(i))->GetParameterIndex(ParameterType::Eta);
    idxPhi = (fFitParticleSet->at(i))->GetParameterIndex(ParameterType::Phi);
    idxE = (fFitParticleSet->at(i))->GetParameterIndex(ParameterType::Energy);
    idxPt = (fFitParticleSet->at(i))->GetParameterIndex(ParameterType::Pt);

    //+ ELECTRONS --------------------------------------------------->
    if ((fFitParticleSet->at(i))->GetType() == ParticleType::Electron) {
      switch (mode) {
        case FitMode::EtaPhiE: /* CURRENT DEFAULT */
        {
          // Sanity check
          doFit = (idxEta < 0 || idxPhi < 0 || idxE < 0) ? false : true;

          if (doFit) {
            minEta = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxEta);
            minPhi = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxPhi);
            minE = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxE);
            // Assume Energy = Magnitude for leptons *only
            (fFitParticleSet->at(i))
                ->SetFitPtEtaPhiE(par[minE] / cosh(par[minEta]), par[minEta],
                                  par[minPhi], par[minE]);
            break;
          }
          Log(me).Write(WARNING)
              << "SetFitVectors(): EtaPhiE mode for this particle not "
                 "defined.. setting initial values!";
          goto SET_INITIAL_EL;
        }

          /* OTHER MODES NOT SUPPORTED IN THIS VERSION */

        default: {
        SET_INITIAL_EL:
          Log(me).Write(WARNING)
              << "SetFitVectors(): Invalid mode: using initial values!";
          (fFitParticleSet->at(i))
              ->SetFitPtEtaPhiE((fFitParticleSet->at(i))->Pt(),
                                (fFitParticleSet->at(i))->Eta(),
                                (fFitParticleSet->at(i))->Phi(),
                                (fFitParticleSet->at(i))->E());
        } break;
      }  // switchboard
    }
    //+ MUONS ------------------------------------------------------->
    else if ((fFitParticleSet->at(i))->GetType() == ParticleType::Muon) {
      switch (mode) {
        case FitMode::EtaPhiE: /* CURRENT DEFAULT */
        {
          // Sanity check
          doFit = (idxEta < 0 || idxPhi < 0 || idxPt < 0) ? false : true;

          if (doFit) {
            minEta = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxEta);
            minPhi = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxPhi);
            minPt = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxPt);

            (fFitParticleSet->at(i))
                ->SetFitPtEtaPhiE(par[minPt], par[minEta], par[minPhi],
                                  par[minPt] * cosh(par[minEta]));
            break;
          }
          Log(me).Write(WARNING)
              << "SetFitVectors(): EtaPhiE mode for this particle not "
                 "defined.. setting initial values!";
          goto SET_INITIAL_MU;
        }

          /* OTHER MODES NOT SUPPORTED IN THIS VERSION */

        default: {
        SET_INITIAL_MU:
          Log(me).Write(WARNING)
              << "SetFitVectors(): Invalid mode: using initial values!";
          (fFitParticleSet->at(i))
              ->SetFitPtEtaPhiE((fFitParticleSet->at(i))->Pt(),
                                (fFitParticleSet->at(i))->Eta(),
                                (fFitParticleSet->at(i))->Phi(),
                                (fFitParticleSet->at(i))->E());
        } break;
      }  // switchboard
    }
    //+ SoftTerm ------------------------------------------------------->
    else if ((fFitParticleSet->at(i))->GetType() == ParticleType::SoftTerm) {
      switch (mode) {
        case FitMode::EtaPhiE: /* CURRENT DEFAULT */
        {
          // Sanity check
          doFit = (idxEta < 0 || idxPhi < 0 || idxPt < 0) ? false : true;

          if (doFit) {
            minEta = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxEta);
            minPhi = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxPhi);
            minPt = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxPt);

            (fFitParticleSet->at(i))
                ->SetFitPtEtaPhiE(par[minPt], par[minEta], par[minPhi],
                                  par[minPt] * cosh(par[minEta]));
            break;
          }
          Log(me).Write(WARNING)
              << "SetFitVectors(): EtaPhiE mode for this particle not "
                 "defined.. setting initial values!";
          goto SET_INITIAL_SOFT;
        }

          /* OTHER MODES NOT SUPPORTED IN THIS VERSION */

        default: {
        SET_INITIAL_SOFT:
          Log(me).Write(WARNING)
              << "SetFitVectors(): Invalid mode: using initial values!";
          (fFitParticleSet->at(i))
              ->SetFitPtEtaPhiE((fFitParticleSet->at(i))->Pt(),
                                (fFitParticleSet->at(i))->Eta(),
                                (fFitParticleSet->at(i))->Phi(),
                                (fFitParticleSet->at(i))->E());
        } break;
      }  // switchboard
    }
    //+ JETS -------------------------------------------------------->
    else {
      switch (mode) {
        case FitMode::EtaPhiE: /* CURRENT DEFAULT */
        {
          // Sanity check
          doFit = (idxEta < 0 || idxPhi < 0 || idxE < 0) ? false : true;

          if (doFit) {
            minEta = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxEta);
            minPhi = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxPhi);
            minE = (fFitParticleSet->at(i))->GetParamMinuitIndex(idxE);

            // Calculate Alpha and Magnitude for jets
            double jet_alpha = par[minE] / (fFitParticleSet->at(i))->E();
            double jet_p = POW2(par[minE]) -
                           POW2(jet_alpha * (fFitParticleSet->at(i))->M());
            jet_p = jet_p < 0.0 ? -sqrt(-jet_p) : sqrt(jet_p);

            (fFitParticleSet->at(i))
                ->SetFitPtEtaPhiE(jet_p / cosh(par[minEta]), par[minEta],
                                  par[minPhi], par[minE]);
            break;
          }
          Log(me).Write(WARNING)
              << "SetFitVectors(): EtaPhiE mode for this particle not "
                 "defined.. setting initial values!";
          goto SET_INITIAL_JET;
        }

          /* OTHER MODES NOT SUPPORTED IN THIS VERSION */

        default: {
        SET_INITIAL_JET:
          Log(me).Write(WARNING)
              << "SetFitVectors(): Invalid mode: using initial values!";
          (fFitParticleSet->at(i))
              ->SetFitPtEtaPhiE((fFitParticleSet->at(i))->Pt(),
                                (fFitParticleSet->at(i))->Eta(),
                                (fFitParticleSet->at(i))->Phi(),
                                (fFitParticleSet->at(i))->E());
        } break;
      }  // switchboard
    }    // Jets

    // Reset
    doFit = false;

  }  // loop over all particles

}  // SetFitVectors

// Likelihood function to be minimized
//_____________________________________________________________________________________________________________
double KF::FitProcessor::LHDFunction() {
  // Log(me).Write(VERBOSE) << "LHDFunction()";

  /**
   * This version uses the simplified model.
   */

  //+ VARS
  double LHD_sum = 0.0;
  int LHD_idx = 0;
  double sumPx = 0.0;
  double sumPy = 0.0;

  double Vmass_constraint = -1;
  double Vwidth_constraint = -1;
  double Hmass_constraint = -1;
  double Hwidth_constraint = -1;

  //+ RESET fLHD (sanity)
  for (int q = 0; q < fNLHDTerms; q++) {
    fLHD[q] = 0.0;
  }

  //+ PARAMETER TERMS
  for (int i = 0; i < fNParticles; i++) {
    for (int j = 0; j < int((fFitParticleSet->at(i))->GetNFitParameters());
         j++) {
      if ((fFitParticleSet->at(i))->GetParameterType(j) == ParameterType::Eta &&
          !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        fLHD[LHD_idx++] =
            POW2((fFitParticleSet->at(i))->FitEta() -
                 (fFitParticleSet->at(i))->Eta()) /
            (fFitParticleSet->at(i))->GetParamResolutionSquared(j);
      } else if ((fFitParticleSet->at(i))->GetParameterType(j) ==
                     ParameterType::Phi &&
                 !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        fLHD[LHD_idx++] =
            POW2((fFitParticleSet->at(i))->FitPhi() -
                 (fFitParticleSet->at(i))->Phi()) /
            (fFitParticleSet->at(i))->GetParamResolutionSquared(j);
      } else if ((fFitParticleSet->at(i))->GetParameterType(j) ==
                     ParameterType::Energy &&
                 (fFitParticleSet->at(i))->GetType() ==
                     ParticleType::Electron &&
                 !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        fLHD[LHD_idx++] =
            POW2((fFitParticleSet->at(i))->FitE() -
                 (fFitParticleSet->at(i))->E()) /
            (fFitParticleSet->at(i))->GetParamResolutionSquared(j);
      } else if ((fFitParticleSet->at(i))->GetParameterType(j) ==
                     ParameterType::Pt &&
                 (fFitParticleSet->at(i))->GetType() == ParticleType::Muon &&
                 !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        fLHD[LHD_idx++] =
            POW2((fFitParticleSet->at(i))->FitPt() -
                 (fFitParticleSet->at(i))->Pt()) /
            (fFitParticleSet->at(i))->GetParamResolutionSquared(j);
      } else if ((fFitParticleSet->at(i))->GetParameterType(j) ==
                     ParameterType::Pt &&
                 (fFitParticleSet->at(i))->GetType() ==
                     ParticleType::SoftTerm &&
                 !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        fLHD[LHD_idx++] =
            POW2((fFitParticleSet->at(i))->FitPt() -
                 (fFitParticleSet->at(i))->Pt()) /
            (fFitParticleSet->at(i))->GetParamResolutionSquared(j);
      } else if ((fFitParticleSet->at(i))->GetParameterType(j) ==
                     ParameterType::Energy &&
                 ((fFitParticleSet->at(i))->GetType() == ParticleType::Jet ||
                  (fFitParticleSet->at(i))->GetType() == ParticleType::JetMu) &&
                 !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        fLHD[LHD_idx++] =
            POW2((fFitParticleSet->at(i))->FitE() -
                 (fFitParticleSet->at(i))->E()) /
            (fFitParticleSet->at(i))->GetParamResolutionSquared(j);
      } else if ((fFitParticleSet->at(i))->GetParameterType(j) ==
                     ParameterType::Energy &&
                 (fFitParticleSet->at(i))->GetType() >= ParticleType::BJet &&
                 !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        fLHD[LHD_idx++] = -2 * log(GetJetProbability(
                                   (fFitParticleSet->at(i))->GetType(),
                                   (fFitParticleSet->at(i))->GetTypePosition(),
                                   (fFitParticleSet->at(i))->FitPt(),
                                   (fFitParticleSet->at(i))->Pt(),
                                   (fFitParticleSet->at(i))->Eta()));
      }

    }  // loop over defined fit parameters

    ///*PTRECO*/ if(fApplyLHDPtRecoCorr/*false*/ &&
    ///(fFitParticleSet->at(i))->GetType() >= ParticleType::BJet) {
    // fLHD[LHD_idx++] = -2 * log(
    // GetJetPtRecoProbability((fFitParticleSet->at(i))->GetType(),
    //(fFitParticleSet->at(i))->GetTypePosition(),
    //(fFitParticleSet->at(i))->FitPt(),
    //(fFitParticleSet->at(i))->Eta()) );
    //}

  }  // loop over all particles

  //+ CONSTRAINT TERMS
  for (unsigned int c = 0; c < fConstraintType.size(); c++) {
    if (fConstraintType[c] == ConstraintType::SumPx) {
      for (int i = 0; i < fNParticles; i++) {
        sumPx += (fFitParticleSet->at(i))->FitPx();
      }
      fLHD[LHD_idx++] = POW2(sumPx - 0.0) / POW2(fConstraintValue[c] * GeV);
    } else if (fConstraintType[c] == ConstraintType::SumPy) {
      for (int i = 0; i < fNParticles; i++) {
        sumPy += (fFitParticleSet->at(i))->FitPy();
      }
      fLHD[LHD_idx++] = POW2(sumPy - 0.0) / POW2(fConstraintValue[c] * GeV);
    } else if (fConstraintType[c] == ConstraintType::Vmass) {
      Vmass_constraint = fConstraintValue[c];
    } else if (fConstraintType[c] == ConstraintType::Vwidth) {
      Vwidth_constraint = fConstraintValue[c];
    } else if (fConstraintType[c] == ConstraintType::Hmass) {
      Hmass_constraint = fConstraintValue[c];
    } else if (fConstraintType[c] == ConstraintType::Hwidth) {
      Hwidth_constraint = fConstraintValue[c];
    }
  }  // loop over all constraints

  // Vmass Breit-Wigner
  if (Vmass_constraint > 0 && Vwidth_constraint > 0) {
    if (fNLeptons == 2) {
      fLHD[LHD_idx] =
          2 * log(POW2(POW2(GetTotalLeptonFitMass()) - POW2(Vmass_constraint)) +
                  POW2(Vmass_constraint) * POW2(Vwidth_constraint));
    } else {
      Log(me).Write(WARNING)
          << "LHDFunction(): Vmass Breit-Wigner constraint will be ignored; "
             "the number of leptons must be 2 in this tag version";
    }
  }

  // Hmass Breit-Wigner
  if (Hmass_constraint > 0 && Hwidth_constraint > 0) {
    if (fNJets > 1) {
      fLHD[LHD_idx] =
          2 * log(POW2(POW2(GetTotalJetFitMass()) - POW2(Hmass_constraint)) +
                  POW2(Hmass_constraint) * POW2(Hwidth_constraint));
    } else {
      Log(me).Write(WARNING)
          << "LHDFunction(): Hmass Breit-Wigner constraint will be ignored; "
             "the number of jets must be >1 in this tag version";
    }
  }

  //+ MAKE LHD SUM
  for (int q = 0; q < fNLHDTerms; q++) {
    LHD_sum += fLHD[q];
  }

  //+ SAVE INITIAL AND CURRENT LHD SUM
  if (fLHDIterStep == 1) fLHD_initial = LHD_sum;
  fLHD_final = LHD_sum;

  return LHD_sum;
}  // LHDFunction

// Retrieve jet histogram for given pT,Eta
//_____________________________________________________________________________________________________________
TH1F* KF::FitProcessor::GetJetProbabilityHist(ParticleType::Particle ptype,
                                              int ptypePos, double pt,
                                              double eta) {
  // Log(me).Write(VERBOSE) << "GetJetProbabilityHist()";

  TH1F* hist = NULL;

  // Sanity check
  if (!(ptype >= ParticleType::BJet)) {
    Log(me).Write(ERROR) << "GetJetProbabilityHist(): invalid request for "
                            "non-jet type particle. Abort!";
    return hist;
  }

  // Unrolled pt bin loop
  int ptBin = -1;
  if (pt < 30 * GeV)
    ptBin = 0;
  else if (pt < 40 * GeV)
    ptBin = 1;
  else if (pt < 50 * GeV)
    ptBin = 2;
  else if (pt < 60 * GeV)
    ptBin = 3;
  else if (pt < 70 * GeV)
    ptBin = 4;
  else if (pt < 80 * GeV)
    ptBin = 5;
  else if (pt < 100 * GeV)
    ptBin = 6;
  else
    ptBin = 7;

  // Unrolled eta bin loop
  int etaBin = -1;
  if (fabs(eta) < 1.2)
    etaBin = 0;
  else
    etaBin = 1;

  if (ptype == ParticleType::BJet) {
    hist = fJetTransferFunction_woutmu[etaBin][ptBin];
  } else if (ptype == ParticleType::BJetMu) {
    hist = fJetTransferFunction_withmu[etaBin][ptBin];
  } else if (ptype == ParticleType::CJet || ptype == ParticleType::CJetMu) {
    // low stat in c-jet with muons so use inclusive
    hist = fJetTransferFunction_worwmu[etaBin][ptBin];
  } else {
    Log(me).Write(ERROR) << "GetJetProbabilityHist(): invalid request"
                         << " pt " << pt << " eta " << eta << " ptype " << ptype
                         << " ptypePos " << ptypePos;
    return hist;
  }

  return hist;
}  // GetJetProbabilityHist

// Retrieve jet probability from corresponding histogram
//_____________________________________________________________________________________________________________
double KF::FitProcessor::GetJetProbability(ParticleType::Particle ptype,
                                           int ptypePos, double fit_pt,
                                           double ini_pt, double ini_eta) {
  // Log(me).Write(VERBOSE) << "GetJetProbability()";

  double prob = 1e-20;

  // Take ratio with initial jet pT
  if (fit_pt <= 0) {
    Log(me).Write(ERROR) << "GetJetProbability(): Fit pT = " << fit_pt
                         << ". Return 1e-20!";
    return prob;
  }
  double ratio = ini_pt / fit_pt; /* pTReco over pTTruth */

  // Determine corresponding histogram
  TH1F* hist = GetJetProbabilityHist(ptype, ptypePos, ini_pt, ini_eta);

  // Check pointer
  if (!hist) {
    Log(me).Write(ERROR)
        << "GetJetProbability(): Hist NULL pointer. Return 1e-20!";
    return prob;
  }

  // Get mean
  // double mean = hist->GetMean();

  // Given the varied jet pT, return approximated value via linear interpolation
  // based on the two nearest bin centers.
  prob =
      hist->Interpolate(ratio);  // used if no scale correction as been applied
  // prob = hist->Interpolate(ratio - 1 + mean); // used if scale correction as
  // been applied

  return ((prob != 0) ? prob : 1e-20);
}  // GetJetProbability

/*
//this was truth pT(j) distribution
//Retrieve jet pTReco probability from corresponding histogram
//_____________________________________________________________________________________________________________
double KF::FitProcessor::GetJetPtRecoProbability(ParticleType::Particle ptype,
int ptypePos, double fit_pt, double ini_eta)
{
  //Log(me).Write(VERBOSE) << "GetJetPtRecoProbability()";

  double prob = 1e-20;

  //Unrolled eta bin loop
  int etaBin = -1;
  if(fabs(ini_eta) < 1.2) etaBin = 0;
  else                    etaBin = 1;

  //Determine corresponding histogram
  TH1F* hist = NULL;
  if(ptypePos==1 || ptypePos==2) {
    if(ptype == ParticleType::BJet)   hist = fPtRecoCorrection_Lead2[etaBin];
    if(ptype == ParticleType::BJetMu) hist =
fPtRecoCorrection_Lead2_semilep[etaBin];
  }
  else {
    if(ptype >= ParticleType::BJet)   hist = fPtRecoCorrection_Other[etaBin];
  }
  //--> Disabled in this tag version:
  //if(ptypePos==1) {
  //if(ptype == ParticleType::BJet)   hist = fPtRecoCorrection_Lead[etaBin];
  //if(ptype == ParticleType::BJetMu) hist =
fPtRecoCorrection_Lead_semilep[etaBin];
  //}
  //else if(ptypePos==2) {
  //if(ptype == ParticleType::BJet)   hist = fPtRecoCorrection_SubLead[etaBin];
  //if(ptype == ParticleType::BJetMu) hist =
fPtRecoCorrection_SubLead_semilep[etaBin];
  //}

  //Check pointer
  if(!hist) {
    Log(me).Write(ERROR) << "GetJetPtRecoProbability(): Hist NULL pointer.
Return 1e-20!"; return prob;
  }

  //Given the varied jet pT, return approximated value via linear interpolation
  //based on the two nearest bin centers. Note: hist in GeV
  prob = hist->Interpolate(fit_pt*1e-3);

  return ((prob != 0) ? prob : 1e-20);
}//GetJetPtRecoProbability
*/

// Retrieve total measured lepton mass
//_____________________________________________________________________________________________________________
double KF::FitProcessor::GetTotalLeptonMass() {
  // Log(me).Write(VERBOSE) << "GetTotalLeptonMass()";

  double mass = 0.0;

  // Check lepton count
  if (fNLeptons == 0) {
    Log(me).Write(ERROR)
        << "GetTotalLeptonMass(): number of leptons is 0. Return 0!";
    return mass;
  }

  if (fNLeptons > 2) {
    Log(me).Write(WARNING)
        << "GetTotalLeptonMass(): more than 2 leptons defined.";
  }

  // Loop over all particles
  double E(0), Px(0), Py(0), Pz(0);
  for (int i = 0; i < fNParticles; i++) {
    if ((fFitParticleSet->at(i))->GetType() <= ParticleType::Muon) {
      E += (fFitParticleSet->at(i))->E();
      Px += (fFitParticleSet->at(i))->Px();
      Py += (fFitParticleSet->at(i))->Py();
      Pz += (fFitParticleSet->at(i))->Pz();
    }
  }

  // Calculate mass
  mass = E * E - (Px * Px + Py * Py + Pz * Pz);
  mass = mass < 0.0 ? -sqrt(-mass) : sqrt(mass);

  return mass;
}  // GetTotalLeptonMass

// Retrieve total fitted lepton mass
//_____________________________________________________________________________________________________________
double KF::FitProcessor::GetTotalLeptonFitMass() {
  // Log(me).Write(VERBOSE) << "GetTotalLeptonFitMass()";

  double mass = 0.0;

  // Check lepton count
  if (fNLeptons == 0) {
    Log(me).Write(ERROR)
        << "GetTotalLeptonFitMass(): number of leptons is 0. Return 0!";
    return mass;
  }

  if (fNLeptons > 2) {
    Log(me).Write(WARNING)
        << "GetTotalLeptonFitMass(): more than 2 leptons defined.";
  }

  // Loop over all particles
  double E(0), Px(0), Py(0), Pz(0);
  for (int i = 0; i < fNParticles; i++) {
    if ((fFitParticleSet->at(i))->GetType() <= ParticleType::Muon) {
      E += (fFitParticleSet->at(i))->FitE();
      Px += (fFitParticleSet->at(i))->FitPx();
      Py += (fFitParticleSet->at(i))->FitPy();
      Pz += (fFitParticleSet->at(i))->FitPz();
    }
  }

  // Calculate mass
  mass = E * E - (Px * Px + Py * Py + Pz * Pz);
  mass = mass < 0.0 ? -sqrt(-mass) : sqrt(mass);

  return mass;
}  // GetTotalLeptonFitMass

// Retrieve total measured jet mass
//_____________________________________________________________________________________________________________
double KF::FitProcessor::GetTotalJetMass() {
  // Log(me).Write(VERBOSE) << "GetTotalJetMass()";

  double mass = 0.0;

  // Check lepton count
  if (fNJets == 0) {
    Log(me).Write(ERROR) << "GetTotalJetMass(): number of jets is 0. Return 0!";
    return mass;
  }

  // Loop over all particles
  double E(0), Px(0), Py(0), Pz(0);
  for (int i = 0; i < fNParticles; i++) {
    if ((fFitParticleSet->at(i))->GetType() >= ParticleType::BJet &&
        ((fFitParticleSet->at(i))->GetTypePosition() == 1 ||
         (fFitParticleSet->at(i))->GetTypePosition() == 2)) {
      E += (fFitParticleSet->at(i))->E();
      Px += (fFitParticleSet->at(i))->Px();
      Py += (fFitParticleSet->at(i))->Py();
      Pz += (fFitParticleSet->at(i))->Pz();
    }
  }

  // Calculate mass
  mass = E * E - (Px * Px + Py * Py + Pz * Pz);
  mass = mass < 0.0 ? -sqrt(-mass) : sqrt(mass);

  return mass;
}  // GetTotalJetMass

// Retrieve total fitted jet mass
//_____________________________________________________________________________________________________________
double KF::FitProcessor::GetTotalJetFitMass() {
  // Log(me).Write(VERBOSE) << "GetTotalJetFitMass()";

  double mass = 0.0;

  // Check lepton count
  if (fNJets == 0) {
    Log(me).Write(ERROR)
        << "GetTotalJetFitMass(): number of jets is 0. Return 0!";
    return mass;
  }

  // Loop over all particles
  double E(0), Px(0), Py(0), Pz(0);
  for (int i = 0; i < fNParticles; i++) {
    if ((fFitParticleSet->at(i))->GetType() >= ParticleType::BJet &&
        ((fFitParticleSet->at(i))->GetTypePosition() == 1 ||
         (fFitParticleSet->at(i))->GetTypePosition() == 2)) {
      E += (fFitParticleSet->at(i))->FitE();
      Px += (fFitParticleSet->at(i))->FitPx();
      Py += (fFitParticleSet->at(i))->FitPy();
      Pz += (fFitParticleSet->at(i))->FitPz();
    }
  }

  // Calculate mass
  mass = E * E - (Px * Px + Py * Py + Pz * Pz);
  mass = mass < 0.0 ? -sqrt(-mass) : sqrt(mass);

  return mass;
}  // GetTotalJetFitMass

// Retrieve total Px/Py
//_____________________________________________________________________________________________________________
std::pair<double, double> KF::FitProcessor::GetSumPxy() {
  // Log(me).Write(VERBOSE) << "GetSumPxy()";

  std::pair<double, double> Pxy(0, 0);  // def: (x,y)

  for (int i = 0; i < fNParticles; i++) {
    Pxy.first += (fFitParticleSet->at(i))->Px();
    Pxy.second += (fFitParticleSet->at(i))->Py();
  }

  return Pxy;
}  // GetSumPxy

// Retrieve total fitted Px/Py
//_____________________________________________________________________________________________________________
std::pair<double, double> KF::FitProcessor::GetSumFitPxy() {
  // Log(me).Write(VERBOSE) << "GetSumFitPxy()";

  std::pair<double, double> FitPxy(0, 0);  // def: (x,y)

  for (int i = 0; i < fNParticles; i++) {
    FitPxy.first += (fFitParticleSet->at(i))->FitPx();
    FitPxy.second += (fFitParticleSet->at(i))->FitPy();
  }

  return FitPxy;
}  // GetSumFitPxy

// Apply baseline ptReco correction
//_____________________________________________________________________________________________________________
void KF::FitProcessor::ApplyPtRecoBaseline() {
  // Log(me).Write(VERBOSE) << "ApplyPtRecoBaseline()";

  // Loop over all jets
  for (int i = 0; i < fNParticles; i++) {
    if ((fFitParticleSet->at(i))->GetType() >= ParticleType::BJet) {
      double Pt = (fFitParticleSet->at(i))->Pt();
      double E = (fFitParticleSet->at(i))->E();
      double correction =
          fPtRecoCorrection_Baseline_woutmu->Interpolate(Pt / 1e3);
      Pt *= correction;
      E *= correction;
      (fFitParticleSet->at(i))
          ->SetFourVector(Pt, (fFitParticleSet->at(i))->Eta(),
                          (fFitParticleSet->at(i))->Phi(), E);
    }
    if ((fFitParticleSet->at(i))->GetType() >= ParticleType::BJetMu) {
      double Pt = (fFitParticleSet->at(i))->Pt();
      double E = (fFitParticleSet->at(i))->E();
      double correction =
          fPtRecoCorrection_Baseline_withmu->Interpolate(Pt / 1e3);
      Pt *= correction;
      E *= correction;
      (fFitParticleSet->at(i))
          ->SetFourVector(Pt, (fFitParticleSet->at(i))->Eta(),
                          (fFitParticleSet->at(i))->Phi(), E);
    }
  }
}  // ApplyPtRecoBaseline

// Publish fit results [limited in this tag version]
//_____________________________________________________________________________________________________________
bool KF::FitProcessor::PublishFit() {
  // Log(me).Write(VERBOSE) << "PublishFit()";

  /* LHD:    the best LHD function value found so far
   * EDM:    the estimated vertical distance remaining to minimum
   * ERRDEF: the value of UP defining parameter uncertainties
   * NVPAR:  the number of currently variable parameters
   * NPARX:  the highest (external) parameter number defined by user
   * ICSTAT: a status integer indicating how good is the covariance
   *///--- returned by reference --->
  fMinuit->mnstat(fMinuit_LHD, fMinuit_EDM, fMinuit_ERRDEF, fMinuit_NVPAR,
                  fMinuit_NPARX, fMinuit_ICSTAT);

  if (fMinuitErrorFlag == 0) {
    fIsFitConverged = true;
  } else {
    fIsFitConverged = false;
    // Log(me).Write(INFO)<<"fIsFitConverged=false; ApplyPtRecoBaseline();";
  }

  // NOT CONVERGED: APPLY BASELINE PT RECO
  if (!fIsFitConverged) ApplyPtRecoBaseline();

  // BUILD FIT RESULT ARRAY ~~~~~~~~~~~~~~~~~~~~~~~~~~>
  fGlobPrefitValues[Idx::TotLHD] = fLHD_initial;
  fGlobPostfitValues[Idx::TotLHD] = fLHD_final;

  fGlobPrefitValues[Idx::mJJ] = GetTotalJetMass();
  fGlobPostfitValues[Idx::mJJ] = GetTotalJetFitMass();

  fGlobPrefitValues[Idx::mLL] = GetTotalLeptonMass();
  fGlobPostfitValues[Idx::mLL] = GetTotalLeptonFitMass();

  fGlobPrefitValues[Idx::SumPx] = GetSumPxy().first;
  fGlobPostfitValues[Idx::SumPx] = GetSumFitPxy().first;

  fGlobPrefitValues[Idx::SumPy] = GetSumPxy().second;
  fGlobPostfitValues[Idx::SumPy] = GetSumFitPxy().second;

  // No error
  return Status::SUCCESS;
}  // PublishFit
