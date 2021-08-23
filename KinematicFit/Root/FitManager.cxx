/**
 * @class KF::FitManager
 * @brief A class to manage the fit configuration
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

// Local includes
#include "KinematicFit/FitManager.h"

// Default constructor
//_____________________________________________________________________________________________________________
KF::FitManager::FitManager() : me(new LogDef("FitManager", INFO)) {}

// Default destructor
//_____________________________________________________________________________________________________________
KF::FitManager::~FitManager() {
  // Log(me).Write(VERBOSE) << "destructor";

  if (me) delete me;
}  // destructor

// Initialize manager
//_____________________________________________________________________________________________________________
bool KF::FitManager::Initialize(std::string datadir, std::string year) {
  // Log(me).Write(VERBOSE) << "Initialize()";

  // Basic settings
  SetDataDir(datadir);
  fAnalysisType = AnalysisType::UNDEF;

  fIsEventClean = true;
  fIsFitSuccess = false;

  // Setup resolutions
  fFitResolutions = new FitResolutions();
  if (!fFitResolutions->Init(datadir, year)) return false;

  std::string TFFileName =
      "bjet_tf_160804.root";  // This was used for EPS2017 and ICHEP2018
  m_doGetSumPxyWidth = 0;
  if (year == "2018") {
    TFFileName = "bjet_tf_181118.root";
    m_doGetSumPxyWidth = 1;
  } else if (year == "2019b") {
    TFFileName = "bjet_tf_190509.root";
    m_doGetSumPxyWidth = 1;
  } else if (year == "2019c") {
    TFFileName = "cjet_tf_190510.root";
    m_doGetSumPxyWidth = 1;
  } else {
    Log(me).Write(WARNING) << "unknown year --> using bjet_tf_160804.root";
    // return false;
  }

  // Load jet response
  return LoadJetResponse(TFFileName);

}  // Initialize

// Define the type of analysis to run (this tag version only supports llbb!)
//_____________________________________________________________________________________________________________
void KF::FitManager::DefineAnalysisType(AnalysisType::Analysis anaType) {
  // Log(me).Write(VERBOSE) << "DefineAnalysisType()";

  // Check analysis type
  if (anaType == AnalysisType::UNDEF) {
    Log(me).Write(WARNING) << "AnalysisType has been set to UNDEF!";
  }
  fAnalysisType = anaType;
}  // DefineAnalysisType

// Add particle object to fit
//_____________________________________________________________________________________________________________
bool KF::FitManager::AddParticle(
    const ParticleType::Particle& type, const std::string& name,
    const double& pt, const double& eta, const double& phi, const double& E,
    const int& /*userIndex*/)  // not supported in this version
{
  // Log(me).Write(VERBOSE) << "AddParticle()";

  // Check particle type
  if (type == ParticleType::Unknown) {
    Log(me).Write(ERROR)
        << "ParticleType has been set to Unknown; please set type. Abort!";
    return Status::ERROR;
  }

  // Check particle ID
  if (GetNParticles() > 0) {
    if (FindParticle(name)) {
      Log(me).Write(ERROR)
          << "Particle <" << name
          << "> is already defined. Please choose another name. Abort!";
      return Status::ERROR;
    }
  }

  // Retrieve current position in particle set for a given type
  int typePos = 0;
  if (type >= ParticleType::Jet)
    typePos = GetNJets();
  else
    typePos = GetNLeptons();

  // Make new particle
  FitParticle* particle = new FitParticle();

  // Set Properties
  particle->SetType(type);
  particle->SetName(name);
  particle->SetTypePosition(typePos + 1);
  particle->SetFourVector(pt, eta, phi, E);

  // Add particle to container
  fFitParticleSet->push_back(particle);

  // Set event flag
  fIsEventClean = false;

  // No error
  return Status::SUCCESS;
}  // AddParticle

// Add fit parameter associated to a pre-defined particle (via AddParticle)
//_____________________________________________________________________________________________________________
bool KF::FitManager::AddFitParameter(
    const ParameterType::Parameter& paramType, const std::string& particleName,
    const int& /*particleIndex*/,  // not supported in this version
    const double& iniVal, const double& paramRes, const double& stepSize,
    const double& lowerLimit, const double& upperLimit) {
  // Log(me).Write(VERBOSE) << "AddFitParameter()";

  // Check parameter type
  if (paramType == ParameterType::Unknown) {
    Log(me).Write(ERROR) << "AddFitParameter(): parameter type is unknown, "
                            "please set type. Abort!";
    return Status::ERROR;
  }

  // Check particle ID
  if (GetNParticles() == 0) {
    Log(me).Write(ERROR) << "AddFitParameter(): no particles have been defined "
                            "yet. Cannot assign fit parameter. Abort!";
    return Status::ERROR;
  } else {
    if (!FindParticle(particleName)) {
      Log(me).Write(ERROR) << "AddFitParameter(): particle " << particleName
                           << " could not be found. Abort!";
      return Status::ERROR;
    }
    // Note: user index is not supported in this version
  }

  // Get index and type of particle
  int pIdx = GetParticleIndex(particleName);
  if (pIdx < 0) {
    Log(me).Write(ERROR) << "AddFitParameter(): particle index for "
                         << particleName << " not found. Abort!";
    return Status::ERROR;
  }
  ParticleType::Particle pType = fFitParticleSet->at(pIdx)->GetType();

  double paramResFixed = paramRes;
  double stepSizeFixed = stepSize;

  // Jet Energy and step sizes
  if (paramType >= ParameterType::Energy && pType >= ParticleType::Jet) {
    //+ Energy
    if (paramResFixed == _DF_) paramResFixed = 0;  // not needed in this version

    //+ Step Size
    if (stepSizeFixed == _DF_) {
      stepSizeFixed = fabs(GetJetResponseRMS(
                          pType, fFitParticleSet->at(pIdx)->GetTypePosition(),
                          fFitParticleSet->at(pIdx)->Pt(),
                          fFitParticleSet->at(pIdx)->Eta())) *
                      0.1;
    }
  }

  // Validate parameter values [only default check in this version]
  if (iniVal == _DF_ || paramResFixed == _DF_ || stepSizeFixed == _DF_ ||
      lowerLimit == _DF_ || upperLimit == _DF_) {
    Log(me).Write(ERROR)
        << "AddFitParameter(): all parameter values have to be set. Abort!";
    return Status::ERROR;
  }

  // Add parameter settings to particle object
  (fFitParticleSet->at(pIdx))
      ->AddFitParameter(paramType, iniVal, paramResFixed, stepSizeFixed,
                        lowerLimit, upperLimit);

  // No error
  return Status::SUCCESS;
}  // AddFitParameter

// Exclude parameter from variation
//_____________________________________________________________________________________________________________
bool KF::FitManager::FixFitParameter(
    ParameterType::Parameter paramType, std::string particleName,
    int /*particleIndex*/)  // not supported in this version
{
  // Log(me).Write(VERBOSE) << "FixFitParameter()";

  // Get index of particle
  int idx = GetParticleIndex(particleName);
  if (idx < 0) {
    Log(me).Write(ERROR) << "FixFitParameter(): particle by the name "
                         << particleName << " not found. Abort!";
    return Status::ERROR;
  }

  return (fFitParticleSet->at(idx))->FixParameter(paramType);
}  // FixFitParameter

// Apply constraint to fit
//_____________________________________________________________________________________________________________
bool KF::FitManager::AddFitConstraint(ConstraintType::Constraint constraintType,
                                      double constraintValue) {
  // Log(me).Write(VERBOSE) << "AddFitConstraint()";
  return AddConstraint(constraintType, constraintValue);
}  // AddFitConstraint

// Order particle set by given parameter type
//_____________________________________________________________________________________________________________
bool KF::FitManager::OrderParticleSet(ParameterType::Parameter paramType) {
  // Log(me).Write(VERBOSE) << "OrderParticleSet()";

  // Check parameter type
  if (paramType == ParameterType::Unknown) {
    Log(me).Write(ERROR) << "OrderParticleSet(): parameter type is unknown, "
                            "please set type. Abort!";
    return Status::ERROR;
  }

  // Check minimum size of particle set container
  if (fFitParticleSet->size() < 2) {
    Log(me).Write(ERROR)
        << "OrderParticleSet(): minimum size of container is 2. Abort!";
    return Status::ERROR;
  }

  // Ordering
  if (paramType == ParameterType::Pt) {
    std::sort(fFitParticleSet->begin(), fFitParticleSet->end(),
              KF::FitParticle::PT_ORDER);
  } else if (paramType == ParameterType::Energy) {
    std::sort(fFitParticleSet->begin(), fFitParticleSet->end(),
              KF::FitParticle::E_ORDER);
  } else if (paramType == ParameterType::Eta) {
    std::sort(fFitParticleSet->begin(), fFitParticleSet->end(),
              KF::FitParticle::ETA_ORDER);
  } else if (paramType == ParameterType::Phi) {
    std::sort(fFitParticleSet->begin(), fFitParticleSet->end(),
              KF::FitParticle::PHI_ORDER);
  } else {
    Log(me).Write(ERROR)
        << "OrderParticleSet(): order request not possible. Abort!";
    return Status::ERROR;
  }

  // Debug: test ordering with > std::random_shuffle(myvector.begin(),
  // myvector.end());

  // Correct type positions
  int typePos_lep(0), typePos_jet(0);

  for (int i = 0; i < GetNParticles(); i++) {
    if ((fFitParticleSet->at(i))->GetType() >= ParticleType::Jet) {
      typePos_jet++;
      (fFitParticleSet->at(i))->SetTypePosition(typePos_jet);
    } else {
      typePos_lep++;
      (fFitParticleSet->at(i))->SetTypePosition(typePos_lep);
    }
  }

  // No error
  return Status::SUCCESS;
}  // OrderParticleSet

int KF::FitManager::DoGetSumPxyWidth() { return m_doGetSumPxyWidth; }

// Get optimized SumPxy width
//_____________________________________________________________________________________________________________
float KF::FitManager::GetSumPxyWidth() {
  // Log(me).Write(VERBOSE) << "GetSumPxyWidth()";

  // Only llbb supported in this tag version
  if (!(fAnalysisType == AnalysisType::LLBB)) {
    Log(me).Write(ERROR) << "GetSumPxyWidth(): The version only supports llbb. "
                            "Change settings. Abort!";
    return 0;
  }

  // Sanity check
  if (GetNParticles() == 0) {
    Log(me).Write(ERROR)
        << "GetSumPxyWidth(): no particles have been defined yet. Abort!";
    return 0;
  }

  // Check particle container
  int el(0), mu(0), lep(0), jet(0);
  for (int i = 0; i < GetNParticles(); i++) {
    if (fFitParticleSet->at(i)->GetType() == ParticleType::Electron) el++;
    if (fFitParticleSet->at(i)->GetType() == ParticleType::Muon) mu++;
    if (fFitParticleSet->at(i)->GetType() >= ParticleType::Jet) jet++;
  }
  lep = el + mu;

  // LLBB check
  if (!(lep == 2 && jet >= 1)) {
    Log(me).Write(ERROR)
        << "GetSumPxyWidth(): 2 leptons and 1+ jets required. Abort!";
    return 0;
  }

  float SumPxyWidth = 0;
  if (jet == 2) {
    SumPxyWidth = 12.0;
  } else if (jet == 3) {
    SumPxyWidth = 15.0;
  }

  return SumPxyWidth;

}  // GetSumPxyWidth

// Validate settings for defined analysis type [only simple validation in this
// tag version]
//_____________________________________________________________________________________________________________
bool KF::FitManager::ValidateAnalysisType() {
  // Log(me).Write(VERBOSE) << "ValidateAnalysisType()";

  // Only llbb supported in this tag version
  if (!(fAnalysisType == AnalysisType::LLBB)) {
    Log(me).Write(ERROR) << "ValidateAnalysisType(): The version only supports "
                            "llbb. Change settings. Abort!";
    return Status::ERROR;
  }

  // Sanity check
  if (GetNParticles() == 0) {
    Log(me).Write(ERROR)
        << "ValidateAnalysisType(): no particles have been defined yet. Abort!";
    return Status::ERROR;
  }

  // Check particle container
  int el(0), mu(0), lep(0), jet(0);
  for (int i = 0; i < GetNParticles(); i++) {
    if (fFitParticleSet->at(i)->GetType() == ParticleType::Electron) el++;
    if (fFitParticleSet->at(i)->GetType() == ParticleType::Muon) mu++;
    if (fFitParticleSet->at(i)->GetType() >= ParticleType::Jet) jet++;
  }
  lep = el + mu;

  // LLBB check
  if (!(lep == 2 && jet >= 1)) {
    Log(me).Write(ERROR)
        << "ValidateAnalysisType(): 2 leptons and 1+ jets required. Abort!";
    return Status::ERROR;
  }

  // No error
  return Status::SUCCESS;
}  // ValidateAnalysisType

// Run the fitting algorithm using the above definitions
//_____________________________________________________________________________________________________________
int KF::FitManager::RunKF() {
  // Log(me).Write(VERBOSE) << "RunKF()";

  // Set prefit flag
  fIsFitSuccess = false;

  // Run Fitter
  if (!Fit()) {
    EventReset();
    return Status::ERROR;
  }

  // Set postfit flag
  fIsFitSuccess = true;

  // No error
  return Status::SUCCESS;
}  // RunKF

// Return fitted four vector of given particle
//_____________________________________________________________________________________________________________
TLorentzVector KF::FitManager::GetTLVectors(Idx::PrePostFit fitstage,
                                            std::string name) {
  // Log(me).Write(VERBOSE) << "GetFitFourVector()";

  // Setup
  TLorentzVector vec;
  for (size_t i = 0; i < 4; i++) {
    vec[i] = _DF_;
  }

  // Check fit status
  if (!IsFitSuccess()) {
    Log(me).Write(ERROR) << "GetFourVectors(): fit was unsuccessful; request "
                            "invalid. Returning NONSENSE!";
    return vec;
  }

  // Check request
  int idx = GetParticleIndex(name);
  if (idx < 0) {
    Log(me).Write(ERROR) << "GetFourVectors(): particle by the name " << name
                         << " not found. Returning NONSENSE!";
    return vec;
  }

  // Load (protection: return only converged fits)
  if (fitstage == Idx::Postfit && IsFitConverged()) {
    vec.SetPtEtaPhiE((fFitParticleSet->at(idx))->FitPt(),
                     (fFitParticleSet->at(idx))->FitEta(),
                     (fFitParticleSet->at(idx))->FitPhi(),
                     (fFitParticleSet->at(idx))->FitE());
    // vec[Idx::Pt]  = (fFitParticleSet->at(idx))->FitPt();
    // vec[Idx::Eta] = (fFitParticleSet->at(idx))->FitEta();
    // vec[Idx::Phi] = (fFitParticleSet->at(idx))->FitPhi();
    // vec[Idx::E]   = (fFitParticleSet->at(idx))->FitE();
  } else {
    vec.SetPtEtaPhiE(
        (fFitParticleSet->at(idx))->Pt(), (fFitParticleSet->at(idx))->Eta(),
        (fFitParticleSet->at(idx))->Phi(), (fFitParticleSet->at(idx))->E());
    // vec[Idx::Pt]  = (fFitParticleSet->at(idx))->Pt();
    // vec[Idx::Eta] = (fFitParticleSet->at(idx))->Eta();
    // vec[Idx::Phi] = (fFitParticleSet->at(idx))->Phi();
    // vec[Idx::E]   = (fFitParticleSet->at(idx))->E();
  }

  return vec;
}  // GetFourVectors

// Return fitted four vector of given particle
//_____________________________________________________________________________________________________________
float* KF::FitManager::GetFourVectors(Idx::PrePostFit fitstage,
                                      std::string name) {
  // Log(me).Write(VERBOSE) << "GetFitFourVector()";

  // Setup
  float* vec = new float[4];
  for (size_t i = 0; i < 4; i++) {
    vec[i] = _DF_;
  }

  // Check fit status
  if (!IsFitSuccess()) {
    Log(me).Write(ERROR) << "GetFourVectors(): fit was unsuccessful; request "
                            "invalid. Returning NONSENSE!";
    return vec;
  }

  // Check request
  int idx = GetParticleIndex(name);
  if (idx < 0) {
    Log(me).Write(ERROR) << "GetFourVectors(): particle by the name " << name
                         << " not found. Returning NONSENSE!";
    return vec;
  }

  // Load (protection: return only converged fits)
  if (fitstage == Idx::Postfit && IsFitConverged()) {
    vec[Idx::Pt] = (fFitParticleSet->at(idx))->FitPt();
    vec[Idx::Eta] = (fFitParticleSet->at(idx))->FitEta();
    vec[Idx::Phi] = (fFitParticleSet->at(idx))->FitPhi();
    vec[Idx::E] = (fFitParticleSet->at(idx))->FitE();
  } else {
    vec[Idx::Pt] = (fFitParticleSet->at(idx))->Pt();
    vec[Idx::Eta] = (fFitParticleSet->at(idx))->Eta();
    vec[Idx::Phi] = (fFitParticleSet->at(idx))->Phi();
    vec[Idx::E] = (fFitParticleSet->at(idx))->E();
  }

  return vec;
}  // GetFourVectors

// Return global fit result value for given type
//_____________________________________________________________________________________________________________
float KF::FitManager::GetGlobFitValue(Idx::PrePostFit fitstage,
                                      Idx::GlobParam param) {
  // Log(me).Write(VERBOSE) << "GetGlobFitValue()";

  // Sanity check
  if (!GetGlobPostfitValues() || !GetGlobPrefitValues()) {
    Log(me).Write(ERROR)
        << "GetGlobFitValue(): fit values not available. Abort!";
    return _DF_;
  }

  // Protection: return only converged fits
  if (fitstage == Idx::Postfit && IsFitConverged())
    return GetGlobPostfitValues()[param];
  return GetGlobPrefitValues()[param];
}  // GetGlobFitValue

// Event Clean-Up: flush containers
//_____________________________________________________________________________________________________________
void KF::FitManager::EventReset() {
  // Log(me).Write(VERBOSE) << "EventReset()";

  // Remove particles
  for (int i = 0; i < GetNParticles(); i++) {
    delete fFitParticleSet->at(i);
  }
  fFitParticleSet->clear();

  // Remove constraints
  RemoveConstraints();

  // Set flags
  fIsEventClean = true;
  fIsFitSuccess = false;
}  // EventReset

// Retrieve and plot pulls
//_____________________________________________________________________________________________________________
void KF::FitManager::PlotPulls(Plotter* fPlotter) {
  // Log(me).Write(VERBOSE) << "PlotPulls()";

  for (int i = 0; i < GetNParticles(); i++) {
    for (int j = 0; j < int((fFitParticleSet->at(i))->GetNFitParameters());
         j++) {
      // Muons
      if ((fFitParticleSet->at(i))->GetParameterType(j) == ParameterType::Pt &&
          (fFitParticleSet->at(i))->GetType() == ParticleType::Muon &&
          !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        double pull = (1.0 / (fFitParticleSet->at(i))->FitPt() -
                       1.0 / (fFitParticleSet->at(i))->Pt()) /
                      sqrt((fFitParticleSet->at(i))->GetParamResolution(j));

        double x = (1.0 / (fFitParticleSet->at(i))->FitPt() -
                    1.0 / (fFitParticleSet->at(i))->Pt());
        x = x * x;
        x = x / (fFitParticleSet->at(i))->GetParamResolution(j);

        if (IsFitConverged()) {
          fPlotter->FindRange("Pull_" + fFitParticleSet->at(i)->GetName(),
                              pull);
          fPlotter->Fill("Pulls", "KF-Pull", fFitParticleSet->at(i)->GetName(),
                         pull, 1);
          fPlotter->Fill("Pulls", "KF-Pull2", fFitParticleSet->at(i)->GetName(),
                         x, 1);
        }
      }
      // Electrons
      else if ((fFitParticleSet->at(i))->GetParameterType(j) ==
                   ParameterType::Energy &&
               (fFitParticleSet->at(i))->GetType() == ParticleType::Electron &&
               !(fFitParticleSet->at(i))->IsParamFixed(j)) {
        double pull =
            ((fFitParticleSet->at(i))->FitE() - (fFitParticleSet->at(i))->E()) /
            (fFitParticleSet->at(i))->GetParamResolution(j);

        double x =
            ((fFitParticleSet->at(i))->FitE() - (fFitParticleSet->at(i))->E());
        x = x * x;
        x = x / ((fFitParticleSet->at(i))->GetParamResolution(j) *
                 (fFitParticleSet->at(i))->GetParamResolution(j));

        if (IsFitConverged()) {
          fPlotter->FindRange("Pull_" + fFitParticleSet->at(i)->GetName(),
                              pull);
          fPlotter->Fill("Pulls", "KF-Pull", fFitParticleSet->at(i)->GetName(),
                         pull, 1);
          fPlotter->Fill("Pulls", "KF-Pull2", fFitParticleSet->at(i)->GetName(),
                         x, 1);
        }
      }  // Muons

    }  // parameters
  }    // particles

}  // PlotPulls
