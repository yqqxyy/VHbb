/**
 * @class KF::FitParticle
 * @brief A generic class describing an universal fit particle
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

// Local includes
#include "KinematicFit/FitParticle.h"

// Default
static const double _DF_ = -999;

// Default constructor
//_____________________________________________________________________________________________________________
KF::FitParticle::FitParticle()
    : fIndex(0),
      fUserIndex(-1),
      fTypePosition(0),
      fName(""),
      fPt(_DF_),
      fEta(_DF_),
      fPhi(_DF_),
      fE(_DF_),
      fPx(_DF_),
      fPy(_DF_),
      fPz(_DF_),
      fM(_DF_),
      fFitParameter(0),
      fParamInitialValue(0),
      fParamResolution(0),
      fParamResolutionSquared(0),
      fParamStepSize(0),
      fParamLowerLimit(0),
      fParamUpperLimit(0),
      fParamMinuitIndex(0),
      fIsParamFixed(0),
      fNFitParameter(0),
      fFitPt(_DF_),
      fFitEta(_DF_),
      fFitPhi(_DF_),
      fFitE(_DF_),
      fFitPx(_DF_),
      fFitPy(_DF_),
      fFitPz(_DF_),
      fFitM(_DF_) {
  Initialize();
  // Log(me).Write(VERBOSE) << "constructor";
}  // constructor

// Initialize
//_____________________________________________________________________________________________________________
void KF::FitParticle::Initialize() {
  me = new LogDef("FitParticle", INFO);

  fParticleType = ParticleType::Unknown;
  fFitMode = FitMode::Unknown;
}  // Initialize

// Default destructor
//_____________________________________________________________________________________________________________
KF::FitParticle::~FitParticle() {
  // Log(me).Write(VERBOSE) << "destructor";

  if (me) delete me;
}  // destructor

// Reset measured 4-Vector components to default value
//_____________________________________________________________________________________________________________
void KF::FitParticle::ResetFourVector() {
  // Log(me).Write(VERBOSE) << "ResetFourVector()";

  fPt = _DF_;
  fEta = _DF_;
  fPhi = _DF_;
  fE = _DF_;
  fPx = _DF_;
  fPy = _DF_;
  fPz = _DF_;
  fM = _DF_;
}  // ResetFourVector

// Reset fitted 4-Vector components to default value
//_____________________________________________________________________________________________________________
void KF::FitParticle::ResetFitFourVector() {
  // Log(me).Write(VERBOSE) << "ResetFitFourVector()";

  fFitPt = _DF_;
  fFitEta = _DF_;
  fFitPhi = _DF_;
  fFitE = _DF_;
  fFitPx = _DF_;
  fFitPy = _DF_;
  fFitPz = _DF_;
  fFitM = _DF_;
}  // ResetFitFourVector

// Set measured 4-Vector of particle
//_____________________________________________________________________________________________________________
bool KF::FitParticle::SetFourVector(double pt, double eta, double phi,
                                    double E) {
  // Log(me).Write(VERBOSE) << "SetFourVector()";

  fPt = fabs(pt);
  fEta = eta;
  fPhi = phi;
  fE = E;

  fPx = pt * cos(phi);
  fPy = pt * sin(phi);
  fPz = pt * sinh(eta);

  fM = E * E - (fPx * fPx + fPy * fPy + fPz * fPz);
  fM = fM < 0.0 ? -sqrt(-fM) : sqrt(fM);

  // No error
  return Status::SUCCESS;
}  // SetFourVector

// Set fitted 4-Vector of particle provided Px, Py, Pz, E
//_____________________________________________________________________________________________________________
int KF::FitParticle::SetFitPxPyPzE(double px, double py, double pz, double E) {
  // Log(me).Write(VERBOSE) << "SetFitPxPyPzE()";

  // Reset
  ResetFitFourVector();

  // TODO variable protection against e.g. common TLV warnings
  // return Status::ERROR;

  fFitPt = sqrt(px * px + py * py);

  float cosTheta = (sqrt(px * px + py * py + pz * pz) == 0.0)
                       ? 1.0
                       : pz / sqrt(px * px + py * py + pz * pz);
  fFitEta = (cosTheta * cosTheta < 1)
                ? -0.5 * log((1.0 - cosTheta) / (1.0 + cosTheta))
                : 10e10;
  if (fFitEta == 10e10 && pz <= 0) fFitEta *= -1;

  fFitPhi = (px == 0 && py == 0.0) ? 0.0 : TMath::ATan2(py, px);
  fFitE = E;

  fFitPx = px;
  fFitPy = py;
  fFitPz = pz;

  fFitM = E * E - (fFitPx * fFitPx + fFitPy * fFitPy + fFitPz * fFitPz);
  fFitM = fFitM < 0.0 ? -sqrt(-fFitM) : sqrt(fFitM);

  // No error
  return Status::SUCCESS;
}  // SetFitPxPyPzE

// Set fitted 4-Vector of particle provided Pt, Eta, Phi, E
//_____________________________________________________________________________________________________________
int KF::FitParticle::SetFitPtEtaPhiE(double pt, double eta, double phi,
                                     double E) {
  // Log(me).Write(VERBOSE) << "SetFitPtEtaPhiE()";

  // Reset
  ResetFitFourVector();

  // TODO variable protection against e.g. common TLV warnings
  // return Status::ERROR;

  // Set fast access vars
  fFitPt = fabs(pt);
  fFitEta = eta;
  fFitPhi = phi;
  fFitE = E;

  fFitPx = pt * cos(phi);
  fFitPy = pt * sin(phi);
  fFitPz = pt * sinh(eta);

  fFitM = E * E - (fFitPx * fFitPx + fFitPy * fFitPy + fFitPz * fFitPz);
  fFitM = fFitM < 0.0 ? -sqrt(-fFitM) : sqrt(fFitM);

  // No error
  return Status::SUCCESS;
}  // SetFitPtEtaPhiE

// Add particle parameter to fit
//_____________________________________________________________________________________________________________
bool KF::FitParticle::AddFitParameter(const ParameterType::Parameter& paramType,
                                      const double& iniVal,
                                      const double& paramRes,
                                      const double& stepSize,
                                      const double& lowerLimit,
                                      const double& upperLimit) {
  // Log(me).Write(VERBOSE) << "AddFitParameter()";

  // Parameter type check
  if (paramType == ParameterType::Unknown) {
    Log(me).Write(ERROR)
        << "AddFitParameter(): parameter type is unknown. Abort!";
    return Status::ERROR;
  }

  // Check if parameter already defined
  if (GetParameterIndex(paramType) > -1) {
    Log(me).Write(ERROR)
        << "AddFitParameter(): parameter type already defined. Abort!";
    return Status::ERROR;
  }

  // Fill vectors
  fFitParameter.push_back(paramType);
  fParamInitialValue.push_back(iniVal);
  fParamResolution.push_back(paramRes);
  fParamResolutionSquared.push_back(paramRes * paramRes);
  fParamStepSize.push_back(stepSize);
  fParamLowerLimit.push_back(lowerLimit);
  fParamUpperLimit.push_back(upperLimit);
  fParamMinuitIndex.push_back(-1);
  fIsParamFixed.push_back(false);

  // Update counter
  fNFitParameter = int(fFitParameter.size());

  // Update fit mode
  UpdateFitMode();

  // No error
  return Status::SUCCESS;
}  // AddFitParameter

// Update fit mode based on currently defined fit parameters
//_____________________________________________________________________________________________________________
void KF::FitParticle::UpdateFitMode() {
  // Log(me).Write(VERBOSE) << "UpdateFitMode()";

  // Reset
  fFitMode = FitMode::Unknown;

  // Types (temp. version)
  bool Eta(false);
  bool Phi(false);
  bool E(false);

  // Set flags
  for (unsigned int i = 0; i < fFitParameter.size(); i++) {
    if (fFitParameter[i] == ParameterType::Eta) Eta = true;
    if (fFitParameter[i] == ParameterType::Phi) Phi = true;
    if (fFitParameter[i] == ParameterType::Energy) E = true;
    if (fFitParameter[i] == ParameterType::Pt) E = true;
  }

  // Determine mode
  if (Eta && Phi && E)
    fFitMode = FitMode::EtaPhiE;
  else if (Eta && Phi && !E)
    fFitMode = FitMode::EtaPhi;
  else if (Eta && !Phi && E)
    fFitMode = FitMode::EtaE;
  else if (!Eta && Phi && E)
    fFitMode = FitMode::PhiE;
  else if (Eta && !Phi && !E)
    fFitMode = FitMode::Eta;
  else if (!Eta && Phi && !E)
    fFitMode = FitMode::Phi;
  else if (!Eta && !Phi && E)
    fFitMode = FitMode::E;
}  // UpdateFitMode

// Fix added parameter from variation
//_____________________________________________________________________________________________________________
bool KF::FitParticle::FixParameter(ParameterType::Parameter paramType) {
  // Log(me).Write(VERBOSE) << "FixParameter()";

  // Sanity size check
  if (!(int(fFitParameter.size()) > 0)) {
    Log(me).Write(ERROR)
        << "FixParameter(): no fit parameters have been defined yet. Abort!";
    return Status::ERROR;
  }

  // Search and set fix flag
  bool paramFound(false);
  for (unsigned int i = 0; i < fFitParameter.size(); i++) {
    if (fFitParameter[i] == paramType) {
      paramFound = true;
      fIsParamFixed[i] = true;
      break;
    }
  }

  // Return status
  if (!paramFound) {
    Log(me).Write(ERROR)
        << "FixParameter(): requested fit parameter not found. Abort!";
    return Status::ERROR;
  }
  return Status::SUCCESS;
}  // FixParameter

// Provide total number of fit parameters defined
//_____________________________________________________________________________________________________________
int KF::FitParticle::GetNFitParameters() {
  // Log(me).Write(VERBOSE) << "GetNFitParameters()";

  // Obtain count
  int nParam = int(fFitParameter.size());

  // Sanity (usage) check
  if (nParam != fNFitParameter) {
    Log(me).Write(WARNING) << "GetNFitParameters(): obtained param count "
                              "doesn't equal fNFitParameter.";
    fNFitParameter = nParam;
  }

  return fNFitParameter;
}  // GetNFitParameters

// Retrieve index of given fit parameter type: (-1) if parameter not defined
//_____________________________________________________________________________________________________________
int KF::FitParticle::GetParameterIndex(ParameterType::Parameter paramType) {
  // Log(me).Write(VERBOSE) << "GetParameterIndex()";

  int index = -1;

  // Size check
  if (!(int(fFitParameter.size()) > 0)) return index;

  // Search for parameter
  for (int i = 0; i < int(fFitParameter.size()); i++) {
    if (fFitParameter[i] == paramType) {
      index = i;
      break;
    }
  }

  return index;
}  // GetParameterIndex
