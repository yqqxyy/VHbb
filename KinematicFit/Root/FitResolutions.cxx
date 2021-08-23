/**
 * @class FitResolutions
 * @brief A class to provide experimental uncertainties of particles
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

// Local includes
#include "KinematicFit/FitResolutions.h"

// Default constructor
//_____________________________________________________________________________________________________________
KF::FitResolutions::FitResolutions()
    : me(new LogDef("FitResolutions", INFO))
// fEnergyRescaler(NULL)
{}  // constructor

// Initialize FitResolutions
//_____________________________________________________________________________________________________________
bool KF::FitResolutions::Init(std::string datadir, std::string /* year */) {
  // Log(me).Write(VERBOSE) << "Initialization";

  // Setup Electron EnergyRescaler tool
  // fEnergyRescaler = new egRescaler::EnergyRescalerUpgrade();
  // fEnergyRescaler->Init(datadir+"EnergyRescalerData_00-04-46.root",year,"es"+year);

  // Load MUID SF file
  std::string file =
      datadir +
      "final_scale_factors_MUID_2012.txt";  // in this tag version: only 2012
  std::ifstream infile(file.c_str());
  if (infile.fail()) {
    Log(me).Write(ERROR) << "Cannot open SF file for muons. Abort!";
    return false;
  }

  // Retrieve MUID SFs
  double dummy;
  while (!infile.eof()) {
    infile >> dummy;
    if (infile.eof()) break;

    fEta_min.push_back(dummy);
    infile >> dummy;
    fEta_max.push_back(dummy);

    infile >> dummy;
    fRes_SF_CB.push_back(dummy);
    infile >> dummy;
    fRes_SF_CB_err.push_back(dummy);
    infile >> dummy;
    fMom_SF_CB.push_back(dummy);
    infile >> dummy;
    fMom_SF_CB_err.push_back(dummy);

    infile >> dummy;
    fRes_SF_ID.push_back(dummy);
    infile >> dummy;
    fRes_SF_ID_err.push_back(dummy);
    infile >> dummy;
    fMom_SF_ID.push_back(dummy);
    infile >> dummy;
    fMom_SF_ID_err.push_back(dummy);

    infile >> dummy;
    fRes_SF_SA.push_back(dummy);
    infile >> dummy;
    fRes_SF_SA_err.push_back(dummy);
    infile >> dummy;
    fMom_SF_SA.push_back(dummy);
    infile >> dummy;
    fMom_SF_SA_err.push_back(dummy);
  }

  return true;
}  // Init

// Default destructor
//_____________________________________________________________________________________________________________
KF::FitResolutions::~FitResolutions() {
  // Log(me).Write(VERBOSE) << "destructor";

  // if(fEnergyRescaler) delete fEnergyRescaler;

  if (me) delete me;
}  // destructor

// Provide electron energy resolution [MeV input]
//_____________________________________________________________________________________________________________
float KF::FitResolutions::GetElectronEnergyResolution(float E, float /*eta*/,
                                                      bool /*isMC*/) {
  // Log(me).Write(VERBOSE) << "GetElectronEnergyResolution()";

  // float res = E*1e-3 * fEnergyRescaler->resolution(E*1e-3, eta, isMC);
  // //EnergyRescaler expects GeV input

  // return (res*1e3);
  return (0.05 * E);
}  // GetElectronEnergyResolution

// Provide muon resolution scale factor (to scale q/p covariance)
//_____________________________________________________________________________________________________________
float KF::FitResolutions::GetMuonResolutionSF(float eta,
                                              MuonType::Detector type) {
  // Log(me).Write(VERBOSE) << "GetMuonResolutionSF()";

  // Find bin
  unsigned int bin = 0;
  if (eta < fEta_min[0]) {
    bin = 0;
  }
  if (eta >= fEta_max[fEta_max.size() - 1]) {
    bin = fEta_max.size() - 1;
  }
  for (unsigned int k = 0; k < fEta_min.size(); k++) {
    if (eta >= fEta_min[k] && eta < fEta_max[k]) {
      bin = k;
      break;
    }
  }

  // Return resolution SF
  if (type == MuonType::CB) {
    return fRes_SF_CB[bin];
  }
  if (type == MuonType::ID || type == MuonType::Calo) {
    return fRes_SF_ID[bin];
  }
  return fRes_SF_SA[bin];
}  // GetMuonResolutionSF
