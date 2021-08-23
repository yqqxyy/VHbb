/**
 * @class KF::FitManager
 * @brief A class to manage the fit configuration
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

#ifndef KF_FITMANAGER_H
#define KF_FITMANAGER_H

//Local includes
#include "FitProcessor.h"

//ROOT includes
#include "TLorentzVector.h"

/**
 * @namespace KF
 * @brief The KF namespace
 */
namespace KF
{
  class FitManager : public FitProcessor
  {

  public:
    /** 
     * Default constructor 
     */
    FitManager();
        
    /** 
     * Default destructor 
     */
    virtual ~FitManager();

    /**
     * CINT
     */
#ifdef KINEMATICFIT_STANDALONE
    ClassDef(FitManager,1);
#endif

    /**
     * Initialize
     * @param datadir Path to data dir
     * @param year    Data taking period
     * @return Standard error flag
     */
    bool Initialize(std::string datadir,
		    std::string year);
        
    /** 
     * Define the type of analysis to run
     * @param Fixed Analysis type
     */
    void DefineAnalysisType(AnalysisType::Analysis anaType); 

    /**
     * Add particle object to fit
     * @param type      Defines type of particle
     * @param name      Custom particle name
     * @param pt        pT of particle
     * @param eta       Eta of particle
     * @param phi       Phi of particle
     * @param E         Energy of particle
     * @param userindex [OPTIONAL] Custom particle index 
     * @return Standard error flag
     */
    bool AddParticle(const ParticleType::Particle& type,
		     const std::string& name,
		     const double& pt,
		     const double& eta,
		     const double& phi,
		     const double& E,
		     const int& userIndex = -1 );
        
    /**
     * Add a fit parameter associated to a pre-defined particle
     * @param paramType     Type of fit parameter
     * @param particleName  AddParticle: @param name
     * @param particleIndex [OPTIONAL] AddParticle: @param userindex
     * @param iniVal        Initial parameter value
     * @param paramRes      Resolution of parameter
     * @param stepSize      Size of (initial) parameter variation
     * @param lowerLimit    Lower boundary of parameter
     * @param upperLimit    Upper boundary of parameter
     * @return Standard error flag
     */
    bool AddFitParameter(const ParameterType::Parameter& paramType,
			 const std::string& particleName,
			 const int& particleIndex = -1,
			 const double& iniVal     = _DF_,
			 const double& paramRes   = _DF_,
			 const double& stepSize   = _DF_,
			 const double& lowerLimit = _DF_,
			 const double& upperLimit = _DF_ );

    /**
     * Exclude parameter from variation
     * @param paramType     Type of fit parameter
     * @param particleName  AddParticle: @param name
     * @param particleIndex [OPTIONAL] AddParticle: @param userindex
     * @return Standard error flag
     */
    bool FixFitParameter(ParameterType::Parameter paramType,
			 std::string particleName,
			 int particleIndex = -1 );

    /**
     * Apply constraint to fit
     * @param Type of fit constraint
     * @param Fixed value of constraint
     * @return Standard error flag
     */
    bool AddFitConstraint(ConstraintType::Constraint constraintType, 
			  double constraintValue);

    /**
     * Order particle set by given parameter type
     * @param paramType Type of fit parameter
     * @return Standard error flag
     */
    bool OrderParticleSet(ParameterType::Parameter paramType);

    float GetSumPxyWidth();
    int DoGetSumPxyWidth();

    /**
     * Validate settings for defined analysis type
     * @return Standard error flag
     */
    bool ValidateAnalysisType();

    /**
     * Run the fitting algorithm using the above definitions
     * @return An error code
     */
    int RunKF();

    /**
     * Return fit status flag
     * @return bool
     */
    inline bool IsFitSuccess() { return fIsFitSuccess; };
       
    /**
     * Return fit converge status flag (EDM<1)
     * @return bool
     */
    inline bool FitConvergeStatus() { return IsFitConverged(); };

    /**
     * Return fitted four vector of given particle
     * @return A 1D-array
     */
    TLorentzVector GetTLVectors(Idx::PrePostFit fitstage, std::string name);
    float* GetFourVectors(Idx::PrePostFit fitstage, std::string name);

    /**
     * Return global fit value for given type
     * @return Fit parameter value
     */
    float GetGlobFitValue(Idx::PrePostFit fitstage, Idx::GlobParam param);

    /**
     * Event Clean-Up: flush containers
     */
    void EventReset();

    /**
     * Fit resolutions (KF internal access)
     */
    FitResolutions* fFitResolutions;

    /**
     * Retrieve and plot pulls (event-level)
     */
    void PlotPulls(Plotter* fPlotter);

  private:
    /**
     * Logger
     */
    LogDef* me;

    /**
     * Analysis type defined for fit
     */
    AnalysisType::Analysis fAnalysisType;
        
    /**
     * Event reset flag
     */
    bool fIsEventClean;

    /**
     * Fit success flag
     */
    bool fIsFitSuccess;

    int m_doGetSumPxyWidth;

  };//@end class FitManager

}//@end namespace KF

#endif //> !KF_FITMANAGER_H
