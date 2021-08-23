/**
 * @class KinematicFit
 * @brief A user class to configure and run a kinematic likelihood fit
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

// Local includes
#include "KinematicFit/KinematicFit.h"

// ROOT
#include "TLorentzVector.h"
#include "TSystem.h"

// Global DEF
static const float mpi = -TMath::Pi();
static const float pi = TMath::Pi();

// Default constructor
//_____________________________________________________________________________________________________________
KF::KinematicFit::KinematicFit()
    : me(new LogDef("KinematicFit", INFO)),
      isInitialized(false) {}  // constructor

// Initialize KinematicFit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::Init(std::string datadir, std::string year, bool isMC) {
  Log(me).Write(INFO) << "Initialization";

  // Set data directory
  if (datadir == "") {
    fDatadir = gSystem->WorkingDirectory();
    fDatadir += "/data/";
  } else {
    fDatadir = datadir;
  }
  gSystem->Setenv("KFDATADIR", fDatadir.c_str());

  // Setup Fit Manager
  fFitManager = new FitManager();
  isInitialized = fFitManager->Initialize(fDatadir, year);
  m_year = year;

  // Set processing default
  fEventProcessed = false;

  // Set data flag
  fIsMC = isMC;

  // Set particle order flag
  fOrderParticleSet = false;

  // Setup plotter [INTERNAL]
  fPlotter = NULL;

  /*
  //Print welcome message
  if(isInitialized) {
    std::string   vpath = fDatadir+"version.info";
    std::string   version("KinematicFit");
    std::string   vinfo(""), devinfo("");
    std::ifstream vfile(vpath.c_str());
    if(vfile.is_open()) {
      std::getline(vfile, version);
      std::getline(vfile, vinfo);
      std::getline(vfile, devinfo);
    }
    Log(me).Write(INFO) << "*******************************************";
    Log(me).Write(INFO) << " > RUNNING :: " << version;
    if(vinfo!="" && devinfo!="") {
      Log(me).Write(INFO) << "__________________________________________";
      Log(me).Write(INFO) << " > INFO    :: "   << vinfo;
      Log(me).Write(INFO) << " > Developed by " << devinfo;
    }
    Log(me).Write(INFO) << "*******************************************";
  }
  */
  return isInitialized;
}  // Init

// Default destructor
//_____________________________________________________________________________________________________________
KF::KinematicFit::~KinematicFit() {
  // Log(me).Write(VERBOSE) << "destructor";

  if (fFitManager) delete fFitManager;

  if (me) delete me;
}  // destructor

// Add an electron to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddElectron(const std::string& name,
                                   const TLorentzVector& tlv,
                                   const float& ERes) {
  // Log(me).Write(VERBOSE) << "AddElectron()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR) << "AddElectron(): Cannot perform request. Please "
                            "check your setup. Abort!";
    return false;
  }

  float pt = tlv.Pt();
  float eta = tlv.Eta();
  float phi = tlv.Phi();
  float E = tlv.E();
  float EResE = ERes * E;

  // this protection was used in reader getKF() until ICHEP2018
  if (pt * 0.001 < 1 || ERes < 0.001) {
    m_particlesOk = false;
  }

  // Event check
  NewEvent();

  // Add new electron to fit
  if (!fFitManager->AddParticle(ParticleType::Electron, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW    UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Energy, name, 0, E, EResE,
                                    EResE * 0.1, E * 0.2, E * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddElectron

// Add an electron to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddElectron(std::string name, float pt, float eta,
                                   float phi, float E, float ERes) {
  // Log(me).Write(VERBOSE) << "AddElectron()";

  ERes = ERes * E;

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR) << "AddElectron(): Cannot perform request. Please "
                            "check your setup. Abort!";
    return false;
  }

  // Event check
  NewEvent();

  // Add new electron to fit
  if (!fFitManager->AddParticle(ParticleType::Electron, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW    UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Energy, name, 0, E, ERes,
                                    ERes * 0.1, E * 0.2, E * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddElectron

// Add an electron to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddElectron(std::string name, float pt, float eta,
                                   float phi, float E) {
  // Log(me).Write(VERBOSE) << "AddElectron()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR) << "AddElectron(): Cannot perform request. Please "
                            "check your setup. Abort!";
    return false;
  }

  // Retrieve energy resolution
  float res =
      fFitManager->fFitResolutions->GetElectronEnergyResolution(E, eta, fIsMC);

  return AddElectron(name, pt, eta, phi, E, fabs(res));
}  // AddElectron

// Add a muon to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddMuon(const std::string& name,
                               const TLorentzVector& tlv, const float& PRes) {
  // Log(me).Write(VERBOSE) << "AddMuon()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "AddMuon(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  float pt = tlv.Pt();
  float eta = tlv.Eta();
  float phi = tlv.Phi();
  float E = tlv.E();
  float PResPt = PRes * pt;

  // this protection was used in reader getKF() until ICHEP2018
  if (pt * 0.001 < 1 || PRes < 0.001) {
    m_particlesOk = false;
  }

  // Event check
  NewEvent();

  // Add new muon to fit
  if (!fFitManager->AddParticle(ParticleType::Muon, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW     UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Pt, name, 0, pt, PResPt,
                                    PResPt * 0.1, pt * 0.2, pt * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddMuon

// Add a muon to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddMuon(std::string name, float pt, float eta, float phi,
                               float E, float PRes) {
  // Log(me).Write(VERBOSE) << "AddMuon()";

  PRes = PRes * pt;

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "AddMuon(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  // Event check
  NewEvent();

  // Add new muon to fit
  if (!fFitManager->AddParticle(ParticleType::Muon, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW     UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Pt, name, 0, pt, PRes,
                                    PRes * 0.1, pt * 0.2, pt * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddMuon

// Add a muon to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddMuon(std::string name, float pt, float eta, float phi,
                               float E, float qp_cov, MuonType::Detector type) {
  // Log(me).Write(VERBOSE) << "AddMuon()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "AddMuon(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  // Input check
  if (qp_cov == 0.0) {
    Log(me).Write(ERROR) << "AddMuon(): Input parameter qp_cov is zero. Abort!";
    return false;
  }

  // Scale q/p error with resolution SF
  float res = sqrt(fabs(qp_cov)) *
              fFitManager->fFitResolutions->GetMuonResolutionSF(eta, type);

  return AddMuon(name, pt, eta, phi, E, res);
}  // AddMuon

// Add soft term to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddSoftTerm(const std::string& name,
                                   const TLorentzVector& tlv,
                                   const float& PRes) {
  // Log(me).Write(VERBOSE) << "AddSoftTerm()";

  float pt = tlv.Pt();
  float eta = tlv.Eta();
  float phi = tlv.Phi();
  float E = tlv.E();
  // PRes=PRes*pt; //not rel but abs 10 GeV

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR) << "AddSoftTerm(): Cannot perform request. Please "
                            "check your setup. Abort!";
    return false;
  }

  // Event check
  NewEvent();

  // Add new muon to fit
  if (!fFitManager->AddParticle(ParticleType::SoftTerm, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW     UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Pt, name, 0, pt, PRes,
                                    PRes * 0.1, pt * 0.2, pt * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddSoftTerm

// Add a jet to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddJet(const std::string& name,
                              const TLorentzVector& tlv, const float& ERes,
                              const std::string& JetType, const bool& semilep) {
  // Log(me).Write(VERBOSE) << "AddJet()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "AddJet(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  float pt = tlv.Pt();
  float eta = tlv.Eta();
  float phi = tlv.Phi();
  float E = tlv.E();
  float EResE = ERes * E;

  // this protection was used in reader getKF() until ICHEP2018
  if (pt * 0.001 < 1 || ERes < 0.001) {
    m_particlesOk = false;
  }

  /*
  //JER patch begin

  //Data jet resolution was not good in CxAOD v28.
  //Forward was still 0 and not yet updated.
  //So I implement copy of getRelResolutionMC here and use it for both MC and
  data.
  //https://svnweb.cern.ch/trac/atlasoff/browser/Reconstruction/Jet/JetResolution/trunk/Root/JERTool.cxx

  static const unsigned int nEtaBins = 7;
  double etaBins[nEtaBins+1] = {0, 0.8, 1.2, 2.1, 2.8, 3.2, 3.6, 4.5};
  TAxis etaAxis = TAxis(nEtaBins, etaBins);

  double noise[nEtaBins] = {};
  double stochastic[nEtaBins] = {};
  double constant[nEtaBins] = {};

  noise[0]=3.34;     stochastic[0]=0.627;     constant[0]=0.0234;
  noise[1]=3.05;     stochastic[1]=0.693;     constant[1]=0.0224;
  noise[2]=3.29;     stochastic[2]=0.658;     constant[2]=0.0300;
  noise[3]=2.56;     stochastic[3]=0.607;     constant[3]=0.0250;
  noise[4]=0.988;    stochastic[4]=0.753;     constant[4]=0.0228;
  noise[5]=2.74;     stochastic[5]=0.783;     constant[5]=0.0465;
  noise[6]=2.80;     stochastic[6]=0.623;     constant[6]=0.0000;

  double ptTrun = std::min( std::max(pt, (float)10000.), (float)1500000. );
  int bin = etaAxis.FindBin(fabs(eta));
  bin = std::min((int) nEtaBins, bin);
  int etaBin = bin - 1;
  double jerMC = 0.;
  jerMC = pow( pow(noise[etaBin]/(ptTrun*0.001), 2) +
               pow(stochastic[etaBin], 2)/(ptTrun*0.001) +
               pow(constant[etaBin], 2),
               0.5 );

  //std::cout<<"AddJet pt = "<<pt<<", eta = "<<eta<<", ERes = "<<ERes<<", jerMC
  = "<<jerMC<<std::endl;
  //MC jet resolution was the same and data jet resolution was modified.
  //This is as expected so I update ERes.

  ERes=jerMC;
  EResE=ERes*E;

  //JER patch end
  */

  // Event check
  NewEvent();

  // Jet type
  ParticleType::Particle ptype;
  ptype = semilep ? ParticleType::JetMu : ParticleType::Jet;
  if (JetType == "B") {
    ptype = semilep ? ParticleType::BJetMu : ParticleType::BJet;
  }
  if (JetType == "C") {
    ptype = semilep ? ParticleType::CJetMu : ParticleType::CJet;
  }

  // Add new jet to fit
  if (!fFitManager->AddParticle(ptype, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW    UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  // if(!fFitManager->AddFitParameter(ParameterType::Energy, name, 0, E,   _DF_,
  // _DF_,                 E*0.2, E*1.8 )) return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Energy, name, 0, E, EResE,
                                    EResE * 0.1, E * 0.2, E * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddJet

// Add a jet to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddJet(std::string name, float pt, float eta, float phi,
                              float E, float ERes, std::string JetType,
                              bool semilep) {
  // Log(me).Write(VERBOSE) << "AddJet()";

  ERes = ERes * E;

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "AddJet(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  // Event check
  NewEvent();

  // Jet type
  ParticleType::Particle ptype;
  ptype = semilep ? ParticleType::JetMu : ParticleType::Jet;
  if (JetType == "B") {
    ptype = semilep ? ParticleType::BJetMu : ParticleType::BJet;
  }

  // Add new jet to fit
  if (!fFitManager->AddParticle(ptype, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW    UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  // if(!fFitManager->AddFitParameter(ParameterType::Energy, name, 0, E,   _DF_,
  // _DF_,                 E*0.2, E*1.8 )) return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Energy, name, 0, E, ERes,
                                    ERes * 0.1, E * 0.2, E * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddJet

// Add a jet to the fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::AddJet(std::string name, bool semilep, float pt,
                              float eta, float phi, float E) {
  // Log(me).Write(VERBOSE) << "AddJet()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "AddJet(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  // Event check
  NewEvent();

  // Jet type
  // ParticleType::Particle ptype = semilep ? ParticleType::JetMu :
  // ParticleType::Jet;
  ParticleType::Particle ptype =
      semilep ? ParticleType::BJetMu : ParticleType::BJet;

  // Add new jet to fit
  if (!fFitManager->AddParticle(ptype, name, pt, eta, phi, E))
    return Status::ERROR;

  // Add fit parameters:            TYPE                   NAME IDX INI  RES
  // STEP                  LOW    UP
  if (!fFitManager->AddFitParameter(ParameterType::Eta, name, 0, eta,
                                    fabs(eta) * 0.01, (fabs(eta) * 0.01) * 0.1,
                                    -7.0, 7.0))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Phi, name, 0, phi, 0.01,
                                    0.01 * 0.1, mpi, pi))
    return Status::ERROR;
  if (!fFitManager->AddFitParameter(ParameterType::Energy, name, 0, E, _DF_,
                                    _DF_, E * 0.2, E * 1.8))
    return Status::ERROR;

  // Fix parameters (temp. only)
  if (!fFitManager->FixFitParameter(ParameterType::Eta, name, 0))
    return Status::ERROR;
  if (!fFitManager->FixFitParameter(ParameterType::Phi, name, 0))
    return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // AddJet

// Check if particles are ok
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::CheckParticlesOk() const {
  // Log(me).Write(VERBOSE) << "CheckParticlesOk()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR) << "CheckParticlesOk(): Cannot perform request. "
                            "Please check your setup. Abort!";
    return false;
  }

  return m_particlesOk;

}  // CheckParticlesOk

// Perform kinematic likelihood fit
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::RunFit() {
  // Log(me).Write(VERBOSE) << "RunFit()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "RunFit(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  // Pre-processing default
  fEventProcessed = false;

  // Configure fit
  if (!ConfigureFit()) {
    Log(me).Write(FATAL) << "RunFit(): Configure fit failed. Terminate fit!";

    // Event flush
    fFitManager->EventReset();

    return Status::ERROR;
  }

  // Particle set ordering
  if (fOrderParticleSet) fFitManager->OrderParticleSet(ParameterType::Pt);

  // Run fit
  if (!fFitManager->RunKF()) {
    Log(me).Write(FATAL) << "RunFit(): Running fit failed. Terminate fit!";
    return Status::ERROR;
  }

  // Post-processing default
  fEventProcessed = true;

  // Plot pulls
  // fFitManager->PlotPulls(fPlotter);

  // No error
  return Status::SUCCESS;
}  // RunFit

bool KF::KinematicFit::RunFit(double SumPxyWidth) {
  // Log(me).Write(VERBOSE) << "RunFit()";

  // Readout access
  if (!isInitialized) {
    Log(me).Write(ERROR)
        << "RunFit(): Cannot perform request. Please check your setup. Abort!";
    return false;
  }

  // Pre-processing default
  fEventProcessed = false;

  // Configure fit
  if (!ConfigureFit(SumPxyWidth)) {
    Log(me).Write(FATAL) << "RunFit(): Configure fit failed. Terminate fit!";

    // Event flush
    fFitManager->EventReset();

    return Status::ERROR;
  }

  // Particle set ordering
  if (fOrderParticleSet) fFitManager->OrderParticleSet(ParameterType::Pt);

  // Run fit
  if (!fFitManager->RunKF()) {
    Log(me).Write(FATAL) << "RunFit(): Running fit failed. Terminate fit!";
    return Status::ERROR;
  }

  // Post-processing default
  fEventProcessed = true;

  // Plot pulls
  // fFitManager->PlotPulls(fPlotter);

  // No error
  return Status::SUCCESS;
}  // RunFit

// Configure fit manager
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::ConfigureFit() {
  // Log(me).Write(VERBOSE) << "ConfigureFit()";

  // Define the analysis type (this tag version only supports llbb)
  fFitManager->DefineAnalysisType(AnalysisType::LLBB);

  float SumPxyWidth = 9.0;
  // This was used until ICHEP2018 and re-optimized after including MET soft
  // term
  if (fFitManager->DoGetSumPxyWidth() == 1) {
    SumPxyWidth = fFitManager->GetSumPxyWidth();
    if (SumPxyWidth == 0) {
      Log(me).Write(ERROR) << "GetSumPxyWidth() is 0 --> please check topology";
      return Status::ERROR;
    }
  } else {
    Log(me).Write(WARNING)
        << "DoGetSumPxyWidth() is not 1 --> using 9 GeV width";
  }

  // Add constraints to fit (values in MeV)
  if (!fFitManager->AddFitConstraint(ConstraintType::SumPx, SumPxyWidth))
    return Status::ERROR;
  if (!fFitManager->AddFitConstraint(ConstraintType::SumPy, SumPxyWidth))
    return Status::ERROR;
  if (!fFitManager->AddFitConstraint(ConstraintType::Vmass, 91187.6))
    return Status::ERROR;
  if (!fFitManager->AddFitConstraint(ConstraintType::Vwidth, 2495.2))
    return Status::ERROR;
  // if(!fFitManager->AddFitConstraint(ConstraintType::Hmass,  125e3))   return
  // Status::ERROR; if(!fFitManager->AddFitConstraint(ConstraintType::Hwidth,
  // 1e-20))   return Status::ERROR;

  // Validate particle definitions (llbb only)
  if (!fFitManager->ValidateAnalysisType()) return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // ConfigureFit

bool KF::KinematicFit::ConfigureFit(double SumPxyWidth) {
  // Log(me).Write(VERBOSE) << "ConfigureFit()";

  // Define the analysis type (this tag version only supports llbb)
  fFitManager->DefineAnalysisType(AnalysisType::LLBB);

  // Add constraints to fit (values in MeV)
  if (!fFitManager->AddFitConstraint(ConstraintType::SumPx, SumPxyWidth))
    return Status::ERROR;
  if (!fFitManager->AddFitConstraint(ConstraintType::SumPy, SumPxyWidth))
    return Status::ERROR;
  if (!fFitManager->AddFitConstraint(ConstraintType::Vmass, 91187.6))
    return Status::ERROR;
  if (!fFitManager->AddFitConstraint(ConstraintType::Vwidth, 2495.2))
    return Status::ERROR;
  // if(!fFitManager->AddFitConstraint(ConstraintType::Hmass,  125e3))   return
  // Status::ERROR; if(!fFitManager->AddFitConstraint(ConstraintType::Hwidth,
  // 1e-20))   return Status::ERROR;

  // Validate particle definitions (llbb only)
  if (!fFitManager->ValidateAnalysisType()) return Status::ERROR;

  // No error
  return Status::SUCCESS;
}  // ConfigureFit

// Return fit status flag
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::IsFitSuccess() {
  // Log(me).Write(VERBOSE) << "IsFitSuccess()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "IsFitSuccess(): Cannot perform request. Please "
                            "check your setup. Abort!";
    return false;
  }

  return fFitManager->IsFitSuccess();
}  // IsFitSuccess

// Return fit converge status
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::IsFitConverged() {
  // Log(me).Write(VERBOSE) << "IsFitConverged()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "IsFitConverged(): Cannot perform request. Please "
                            "check your setup. Abort!";
    return false;
  }

  return fFitManager->FitConvergeStatus();
}  // IsFitConverged

// Return fit results for given object
//_____________________________________________________________________________________________________________
TLorentzVector KF::KinematicFit::GetFitTLVector(std::string name) {
  // Log(me).Write(VERBOSE) << "GetFitFourVector()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "GetFitTLVector(): Cannot perform request. Please "
                            "check your setup. Abort!";
    // return NULL;
    TLorentzVector nll;
    return nll;
  }

  return fFitManager->GetTLVectors(Idx::Postfit, name);
}  // GetFitFourVector

// Return fit results for given object
//_____________________________________________________________________________________________________________
float* KF::KinematicFit::GetFitFourVector(std::string name) {
  // Log(me).Write(VERBOSE) << "GetFitFourVector()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "GetFitFourVector(): Cannot perform request. "
                            "Please check your setup. Abort!";
    return NULL;
  }

  return fFitManager->GetFourVectors(Idx::Postfit, name);
}  // GetFitFourVector

// Return fit results for given object by reference
//_____________________________________________________________________________________________________________
void KF::KinematicFit::GetFitFourVector(std::string name, float& pt, float& eta,
                                        float& phi, float& E) {
  // Log(me).Write(VERBOSE) << "GetFitFourVector()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "GetFitFourVector(): Cannot perform request. "
                            "Please check your setup. Abort!";

    pt = _DF_;
    eta = _DF_;
    phi = _DF_;
    E = _DF_;
  }

  float* vec = fFitManager->GetFourVectors(Idx::Postfit, name);
  pt = vec[Idx::Pt];
  eta = vec[Idx::Eta];
  phi = vec[Idx::Phi];
  E = vec[Idx::E];
}  // GetFitFourVector

// Return measured results for given object
//_____________________________________________________________________________________________________________
float* KF::KinematicFit::GetFourVector(std::string name) {
  // Log(me).Write(VERBOSE) << "GetFourVector()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "GetFourVector(): Cannot perform request. Please "
                            "check your setup. Abort!";
    return NULL;
  }

  return fFitManager->GetFourVectors(Idx::Prefit, name);
}  // GetFourVector

// Return measured results for given object by reference
//_____________________________________________________________________________________________________________
void KF::KinematicFit::GetFourVector(std::string name, float& pt, float& eta,
                                     float& phi, float& E) {
  // Log(me).Write(VERBOSE) << "GetFourVector()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "GetFourVector(): Cannot perform request. Please "
                            "check your setup. Abort!";

    pt = _DF_;
    eta = _DF_;
    phi = _DF_;
    E = _DF_;
  }

  float* vec = fFitManager->GetFourVectors(Idx::Prefit, name);
  pt = vec[Idx::Pt];
  eta = vec[Idx::Eta];
  phi = vec[Idx::Phi];
  E = vec[Idx::E];
}  // GetFourVector

// Return prefit parameter of given type
//_____________________________________________________________________________________________________________
float KF::KinematicFit::GetPrefitParameter(Idx::GlobParam param) {
  // Log(me).Write(VERBOSE) << "GetPrefitParameter()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "GetPrefitParameter(): Cannot perform request. "
                            "Please check your setup. Abort!";
    return -999;
  }

  return fFitManager->GetGlobFitValue(Idx::Prefit, param);
}  // GetPrefitParameter

// Return postfit parameter of given type
//_____________________________________________________________________________________________________________
float KF::KinematicFit::GetPostfitParameter(Idx::GlobParam param) {
  // Log(me).Write(VERBOSE) << "GetPostfitParameter()";

  // Readout access
  if (!isInitialized || !fEventProcessed) {
    Log(me).Write(ERROR) << "GetPostfitParameter(): Cannot perform request. "
                            "Please check your setup. Abort!";
    return -999;
  }

  return fFitManager->GetGlobFitValue(Idx::Postfit, param);
}  // GetPostfitParameter

// Call manager event reset
//_____________________________________________________________________________________________________________
void KF::KinematicFit::EventReset() {
  // Log(me).Write(VERBOSE) << "EventReset()";

  // Reset processing flag
  fEventProcessed = false;
  m_particlesOk = true;

  // Reset event
  fFitManager->EventReset();
}  // EventReset

// New event check
//_____________________________________________________________________________________________________________
void KF::KinematicFit::NewEvent() {
  // Log(me).Write(VERBOSE) << "NewEvent()";

  // Reset processing flag
  fEventProcessed = false;

  // Reset event
  if (fFitManager->IsFitSuccess()) fFitManager->EventReset();
}  // NewEvent

// Setup KF Plotter
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::SetupPlotter(TString filename) {
  // Log(me).Write(VERBOSE) << "SetupPlotter()";

  if (fPlotter) return false;
  fPlotter = new Plotter();
  fPlotter->Init(filename);

  return true;
}  // SetupPlotter

// Publish internal analysis histograms
//_____________________________________________________________________________________________________________
bool KF::KinematicFit::WriteHistograms() {
  // Log(me).Write(VERBOSE) << "WriteHistograms()";

  if (!fPlotter) return false;
  fPlotter->WriteOutput();

  return true;
}  // WriteHistograms
