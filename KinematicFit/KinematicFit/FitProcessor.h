/**
 * @class KF::FitProcessor
 * @brief A base class to process and run a kinematic likelihood fit
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

#ifndef KF_FITPROCESSOR_H
#define KF_FITPROCESSOR_H

//STL includes
#include <string>
#include <vector>
#include <algorithm>

//ROOT includes
#include "TMinuit.h"
#include "TH1F.h"

//Local includes
#include "FitParticle.h"

/**
 * @namespace KF
 * @brief The KF namespace
 */
namespace KF
{
  //Consts
  const double _DF_ = -999;
  const int GeV     = 1e3;

  class FitProcessor 
  {

  public:
    /** 
     * Default constructor 
     */
    FitProcessor();

    /** 
     * Default destructor 
     */
    virtual ~FitProcessor();

    /**
     * CINT
     */
#ifdef KINEMATICFIT_STANDALONE
    ClassDef(FitProcessor,1);
#endif

    /**
     * Initialization status
     * @return Status bool flag
     */
    inline bool IsInitialized() { return fIsInitialized; };

    /** 
     * Run kinematic likelihood fit
     * @return An error code
     */
    int Fit();


  private:
    /**
     * Initialize
     */
    void Initialize();

    /**
     * Init flag
     */
    bool fIsInitialized;

    /**
     * Path to local data directory
     */
    std::string fDatadir;

    /**
     * Jet response flag
     */
    bool fIsJetResponseLoaded;

    /**
     * Logger
     */
    LogDef* me;


  protected:
    /**
     * Set local data directory
     */
    inline void SetDataDir(std::string dir)
    { fDatadir = dir; }

    /**
     * Fit Particle object
     */
    FitParticle* fFitParticle;

    /**
     * Fit Particle object container
     */
    FitParticleSet* fFitParticleSet;

    /**
     * Find a particle with a given name
     * @return found(true), or not(false)
     */
    bool FindParticle(std::string name);

    /**
     * Return the container index of a particle
     * @return index
     */
    int GetParticleIndex(std::string name);

    /**
     * Return the total number of particles
     * @return Number of particles
     */ 	
    inline int GetNParticles() const
    { return int(fFitParticleSet->size()); };

    /**
     * Return the number of particles of a given type
     * @return Number of particles
     */
    int GetNParticles(ParticleType::Particle ptype) const;

    /**
     * Return the number of leptons
     * @return Number of particles
     */
    int GetNLeptons();

    /**
     * Return the number of jets
     * @return Number of particles
     */
    int GetNJets();

    /**
     * Return the total number of fit parameters
     * @return Number of fit parameters
     */
    int GetNFitParameters();

    /**
     * Return the number of fit parameters for given particle type
     * @return Number of fit parameters
     */
    int GetNFitParameters(ParticleType::Particle ptype);

    /**
     * Return the number of terms in the likelihood function
     * @return Number of LHD terms
     */
    int GetNLHDTerms();

    /**
     * Return RMS of jet probability histogram for given pT,Eta
     * @return RMS value
     */
    double GetJetResponseRMS(ParticleType::Particle ptype, int ptypePos, double pt, double eta);

    /**
     * Add constraint to fit
     * @return Standard error flag
     */
    bool AddConstraint(ConstraintType::Constraint type, double value); 

    /**
     * Remove all constraints
     */
    void RemoveConstraints();

    /**
     * Load jet response histograms to runtime vectors
     * @return Standard error flag
     */
    bool LoadJetResponse( const std::string TFFileName );

    /**
     * Toggle PtReco in Likelihood flag
     */
    //inline void SetLHDPtRecoCorr(bool flag) { fApplyLHDPtRecoCorr=flag; };

    /**
     * Return fit convergence status
     * @return bool flag
     */
    inline bool IsFitConverged() { return fIsFitConverged; };

    /**
     * Return global prefit array
     * @return An array
     */
    inline double* GetGlobPrefitValues() { return fGlobPrefitValues; };

    /**
     * Return global postfit array
     * @return An array
     */
    inline double* GetGlobPostfitValues() { return fGlobPostfitValues; };


  private:
    //RUNTIME METHODS
    //_______________________________________________________________>

    /**
     * Verify minimal setup of fit
     * @return Standard error flag
     */
    bool VerifyFitSetup();

    /**
     * Apply jet scale correction from resolution
     * @return Standard error flag
     */
    bool ApplyJetScaleCorrection();

    /**
     * Reset all objects and variables for new event
     * @return Standard error flag
     */
    bool Reset();

    /**
     * Initialize Minuit
     * @return Standard error flag
     */
    bool InitMinuit();

    /**
     * Likelihood function used by Minuit
     */
    static void FCNLikelihood(int &npar, double * grad, double &fval, double * par, int flag);

    /**
     * Likelihood function to be minimized
     * @brief Called by FCNLikelihood
     * @return Function value
     */
    double LHDFunction();


    /**
     * Sets the fit vectors for leptons and jets 
     * given the current Minuit parameter list
     */
    void SetFitVectors(double * par);

    /**
     * Convert eta to theta
     * @return Theta
     */
    inline double GetTheta(double eta) { return ( 2 * atan( exp(-eta) ) ); };

    /**
     * Return the square of x (for code clarity)
     * @return x^2
     */
    inline double POW2(double x) { return (x * x); };

    /**
     * Retrieve jet probability histogram for given particle position, pT, Eta
     * @return Jet histogram
     */
    TH1F* GetJetProbabilityHist(ParticleType::Particle ptype, int ptypePos, double pt, double eta);

    /**
     * Retrieve jet probability from corresponding histogram
     * @param ptype    : jet semileptonic or not
     * @param ptypePos : position of particle in set
     * @param fit_pt   : "pTTruth" from fit
     * @param ini_pt   : pTReco from measurement
     * @param ini_eta  : jet eta from measurement
     * @return Probability
     */
    double GetJetProbability(ParticleType::Particle ptype, int ptypePos, double fit_pt, double ini_pt, double ini_eta);


    /**
     * Retrieve jet pTReco probability from corresponding histogram
     * @return Probability
     */
    //double GetJetPtRecoProbability(ParticleType::Particle ptype, int ptypePos, double fit_pt, double ini_eta);

    /**
     * Retrieve total measured lepton mass
     * @return Mass in MeV
     */
    double GetTotalLeptonMass();

    /**
     * Retrieve total fitted lepton mass
     * @return Mass in MeV
     */
    double GetTotalLeptonFitMass();

    /**
     * Retrieve total measured jet mass
     * @return Mass in MeV
     */
    double GetTotalJetMass();

    /**
     * Retrieve total fitted jet mass
     * @return Mass in MeV
     */
    double GetTotalJetFitMass();

    /**
     * Retrieve total Px/Py
     * @return Sum in MeV
     */
    std::pair<double,double> GetSumPxy();

    /**
     * Retrieve total fitted Px/Py
     * @return Sum in MeV
     */
    std::pair<double,double> GetSumFitPxy();

    /*
     * Apply baseline ptReco correction
     */
    void ApplyPtRecoBaseline();

    /** 
     * Publish fit results
     * @return Standard error flag
     */
    bool PublishFit();


    //RUNTIME FLAGS
    //_______________________________________________________________>

    /**
     * Require initial baseline check to be executed
     */
    bool fIsBaselineCheckRequired;

    /**
     * TMinuit error flag
     */
    int fMinuitErrorFlag;

    /**
     * Flag to apply pTReco correction through likelihood
     */
    //bool fApplyLHDPtRecoCorr;


    //RUNTIME VARIABLES
    //_______________________________________________________________>
	
    /* ######################### STAGE 1 ######################### */

    /**
     * Total number of particles
     */
    int fNParticles;

    /**
     * Total number of leptons
     */
    int fNLeptons;

    /**
     * Total number of jets
     */
    int fNJets;

    /**
     * Total number of fit parameters
     */
    int fNFitParameters;

    /**
     * TMinuit for LHD minimization
     */
    TMinuit* fMinuit;

    /**
     * Minuit output print level
     */
    int fMinuitPrintLevel;

    /**
     * TMinuit list of arguments
     */
    double fMinuitArglist[2];

    /* ######################### STAGE 2 ######################### */

    /**
     * Likelihood iteration count
     */
    int fLHDIterStep;

    std::vector< std::vector< TH1F* > > fJetTransferFunction_woutmu;
    std::vector< std::vector< TH1F* > > fJetTransferFunction_withmu;
    std::vector< std::vector< TH1F* > > fJetTransferFunction_worwmu;

    /**
     * PtReco corrections: leading two jets
     * @brief For in-situ jet ptReco correction in likelihood
     */
    //std::vector< TH1F* > fPtRecoCorrection_Lead2;

    /**
     * PtReco corrections: leading two jets [semileptonic]
     * @brief For in-situ jet ptReco correction in likelihood
     */
    //std::vector< TH1F* > fPtRecoCorrection_Lead2_semilep;

    /**
     * PtReco corrections: leading jet
     * @brief For in-situ jet ptReco correction in likelihood
     */
    //std::vector< TH1F* > fPtRecoCorrection_Lead;

    /**
     * PtReco corrections: leading jet [semileptonic]
     * @brief For in-situ jet ptReco correction in likelihood
     */
    //std::vector< TH1F* > fPtRecoCorrection_Lead_semilep;

    /**
     * PtReco corrections: sub-leading jet
     * @brief For in-situ jet ptReco correction in likelihood
     */
    //std::vector< TH1F* > fPtRecoCorrection_SubLead;

    /**
     * PtReco corrections: sub-leading jet [semileptonic]
     * @brief For in-situ jet ptReco correction in likelihood
     */
    //std::vector< TH1F* > fPtRecoCorrection_SubLead_semilep;
        
    /**
     * PtReco corrections: 3rd jet, FSR, etc.
     * @brief For jet energy probability in likelihood
     */
    //std::vector< TH1F* > fPtRecoCorrection_Other;

    /**
     * PtReco correction: current VH baseline
     * @brief For jet pt and E scaling if KF did not converge
     */
    //TH1F* fPtRecoCorrection_Baseline;

    TH1F* fPtRecoCorrection_Baseline_withmu;
    TH1F* fPtRecoCorrection_Baseline_woutmu;

    /* ######################### STAGE 3 ######################### */

    /**
     * Current individual likelihood values
     */
    double* fLHD;

    /**
     * Number of terms in likelihood function
     */
    int fNLHDTerms;

    /**
     * Fit constraint types to be applied
     */
    std::vector<ConstraintType::Constraint> fConstraintType;

    /**
     * Fit constraint value to be applied
     */
    std::vector<double> fConstraintValue;

    /**
     * Initial total likelihood value
     */
    double fLHD_initial;

    /**
     * Final total likelihood value after fit
     */
    double fLHD_final;

    /* ######################### STAGE 4 ######################### */
        
    /**
     * Minuit status flags
     */
    double fMinuit_LHD;
    double fMinuit_EDM;
    double fMinuit_ERRDEF;
    int    fMinuit_NVPAR;
    int    fMinuit_NPARX;
    int    fMinuit_ICSTAT;

    /**
     * Fit Convergence flag
     */
    bool fIsFitConverged;

    /**
     * Published prefit array
     */
    double* fGlobPrefitValues;

    /**
     * Published postfit array
     */
    double* fGlobPostfitValues;

    /** 
     * Number of fit results we publish
     */
    int fNGlobFitValues;

  };//@end class FitProcessor

}//@end namespace KF

#endif //> !KF_FITPROCESSOR_H
