/**
 * @class FitResolutions
 * @brief A class to provide experimental uncertainties of particles
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

#ifndef KF_FITRESOLUTIONS_H
#define KF_FITRESOLUTIONS_H

//RootCore
//#include "egammaAnalysisUtils/EnergyRescalerUpgrade.h"

//C++
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <functional>

//Local includes
#include "Log.h"
#include "Plotter.h"
#include "FitEnums.h"

/**
 * @namespace KF
 * @brief The KF namespace
 */
namespace KF
{
  class FitResolutions
  {

  public:
    /** 
     * Default constructor 
     */
    FitResolutions();

    /** 
     * Default destructor 
     */
    virtual ~FitResolutions();

    /**
     * CINT
     */
#ifdef KINEMATICFIT_STANDALONE
    ClassDef(FitResolutions,1);
#endif
        
    /**
     * Initialize FitResolutions
     * @brief Called during setup
     * @param datadir Path to data
     * @param year    Data period
     * @return Standard error flag
     */
    bool Init(std::string datadir, std::string year);

    /**
     * Provide electron energy resolution
     * @param E      Cluster energy
     * @param eta    Cluster eta
     * @param isMC   Flag for MC
     * @return Energy resolution
     */
    float GetElectronEnergyResolution(float E, float eta, bool isMC);

    /**
     * Provide muon resolution scale factor (to scale q/p covariance)
     * @param eta    Eta of muon
     * @param type   Detector type
     *               (a) combined muons
     *               (b) segment or calorimeter tagged muons
     *               (c) stand-alone muons
     * @return Resolution scale factor
     */
    float GetMuonResolutionSF(float eta, MuonType::Detector type);

  private:     
    /**
     * Logger
     */
    LogDef* me;

    /**
     * Init flag
     */
    bool isInitialized;
        
    /**
     * Electron energy resolutions
     * @return Standard error flag
     */
    //egRescaler::EnergyRescalerUpgrade* fEnergyRescaler;

    /**
     * Muon resolution and momentum scale factors
     * @brief SFs from MuonMomentumCorrections/MuonResolutionAndMomentumScaleFactors
     */
    std::vector<double> fEta_min,   fEta_max;
    std::vector<double> fRes_SF_CB, fRes_SF_CB_err;
    std::vector<double> fMom_SF_CB, fMom_SF_CB_err;
    std::vector<double> fRes_SF_ID, fRes_SF_ID_err;
    std::vector<double> fMom_SF_ID, fMom_SF_ID_err; 
    std::vector<double> fRes_SF_SA, fRes_SF_SA_err;
    std::vector<double> fMom_SF_SA, fMom_SF_SA_err; 

  };//@end class FitResolutions

}//@end namespace KF

#endif //> !KF_FITRESOLUTIONS_H
