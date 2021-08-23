/**
 * @class KF::FitParticle
 * @brief A generic class describing an universal fit particle
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

#ifndef KF_FITPARTICLE_H
#define KF_FITPARTICLE_H

//STL includes
#include <string>
#include <vector>

//ROOT includes

//Local includes
#include "FitResolutions.h"
#include "TMath.h"

/**
 * @namespace KF
 * @brief The KF namespace
 */
namespace KF
{
  class FitParticle 
  {

  public:
    /** 
     * Default constructor 
     */
    FitParticle();

    /**
     * Default copy constructor
     */
    //FitParticle(const FitParticle & particle);

    /** 
     * Default destructor 
     */
    virtual ~FitParticle();

    /**
     * CINT
     */
#ifdef KINEMATICFIT_STANDALONE
    ClassDef(FitParticle,1);
#endif

    /** 
     * Set global index of particle
     */
    inline void SetIndex(int index)
    { fIndex = index; }

    /**
     * Set global user index of particle (if needed)
     */
    inline void SetUserIndex(int index=-1)
    { fUserIndex = index; }

    /**
     * Set name of particle
     */
    inline void SetName(std::string name)
    { fName = name; }

    /**
     * Set type of particle
     */
    inline void SetType(ParticleType::Particle type)
    { fParticleType = type; }

    /**
     * Set type position of particle in container
     */
    inline void SetTypePosition(int pos)
    { fTypePosition = pos; }

    /**
     * Reset 4-Vector components to default value
     */
    void ResetFourVector();

    /**
     * Reset fitted 4-Vector components to default value
     */
    void ResetFitFourVector();

    /**
     * Set 4-Vector of particle
     * @return Standard error flag
     */
    bool SetFourVector(double pt, double eta, double phi, double E);

    /**
     * Set fitted 4-Vector of particle provided Px, Py, Pz, E
     * @return An error code (e.g. pT=0 warning)
     */
    int SetFitPxPyPzE(double px, double py, double pz, double E);

    /**
     * Set fitted 4-Vector of particle provided Pt, Eta, Phi, E
     * @return An error code (e.g. pT=0 warning)
     */
    int SetFitPtEtaPhiE(double pt, double eta, double phi, double E);

    /**
     * Add particle parameter to fit
     * @return Standard error flag
     */
    bool AddFitParameter(const ParameterType::Parameter& paramType,
			 const double& iniVal,
			 const double& paramRes,
			 const double& stepSize,
			 const double& lowerLimit,
			 const double& upperLimit);

    /**
     * Update fit mode based on currently defined fit parameters
     */
    void UpdateFitMode();

    /**
     * Fix parameter from variation
     * @return Standard error flag
     */
    bool FixParameter(ParameterType::Parameter paramType);

    /**
     * Set global Minuit index for a parameter
     */
    void SetParamMinuitIndex(int index, int minuitIdx)  { fParamMinuitIndex[index] = minuitIdx; }

    /**
     * @return Global index of particle
     */
    inline int GetIndex()
    { return fIndex; }

    /**
     * @return Global user index of particle
     */
    inline int GetUserIndex()
    { return fUserIndex; }

    /**
     * @return Name of particle
     */
    inline std::string GetName()
    { return fName; }

    /**
     * @return Type of particle
     */
    inline ParticleType::Particle GetType()
    { return fParticleType; }

    /**
     * @return Type position of particle in container
     */
    inline int GetTypePosition()
    { return fTypePosition; }
        
    /**
     * @return Fit configuration by index
     */
    inline ParameterType::Parameter GetParameterType(int index) { return fFitParameter[index]; }
    inline double GetParamInitialValue(int index) { return fParamInitialValue[index]; }
    inline double GetParamResolution(int index)   { return fParamResolution[index];   }
    inline double GetParamResolutionSquared(int index)   { return fParamResolutionSquared[index];   }
    inline double GetParamStepSize(int index)     { return fParamStepSize[index];     }
    inline double GetParamLowerLimit(int index)   { return fParamLowerLimit[index];   }
    inline double GetParamUpperLimit(int index)   { return fParamUpperLimit[index];   }
    inline int    GetParamMinuitIndex(int index)  { return fParamMinuitIndex[index];  }
    inline bool   IsParamFixed(int index)         { return fIsParamFixed[index];      }

    /**
     * @return Total number of fit parameters defined
     */
    int GetNFitParameters();

    /**
     * @return Mode of fit parameter permutation
     */
    inline FitMode::Mode GetFitMode() { return fFitMode; }

    /**
     * @return Index of defined fit parameter: (-1) if not defined
     */
    int GetParameterIndex(ParameterType::Parameter paramType);

    /**
     * @return Measured fast access variables from corresponding 4-Vector
     */
    inline double Pt()  { return fPt;  }
    inline double Eta() { return fEta; }
    inline double Phi() { return fPhi; }
    inline double E()   { return fE;   }
    inline double Px()  { return fPx;  }
    inline double Py()  { return fPy;  }
    inline double Pz()  { return fPz;  }
    inline double M()   { return fM;   }

    /**
     * @return Fitted fast access variables from corresponding 4-Vector
     */
    inline double FitPt()  { return fFitPt;  }
    inline double FitEta() { return fFitEta; }
    inline double FitPhi() { return fFitPhi; }
    inline double FitE()   { return fFitE;   }
    inline double FitPx()  { return fFitPx;  }
    inline double FitPy()  { return fFitPy;  }
    inline double FitPz()  { return fFitPz;  }
    inline double FitM()   { return fFitM;   }

    /**
     * Class sorting predicates
     */
    static bool PT_ORDER(  FitParticle* lhs, FitParticle* rhs) { return (lhs->fPt             > rhs->fPt);             }
    static bool E_ORDER(   FitParticle* lhs, FitParticle* rhs) { return (lhs->fE              > rhs->fE);              }
    static bool ETA_ORDER( FitParticle* lhs, FitParticle* rhs) { return (std::fabs(lhs->fEta) < std::fabs(rhs->fEta)); }
    static bool PHI_ORDER( FitParticle* lhs, FitParticle* rhs) { return (std::fabs(lhs->fPhi) < std::fabs(rhs->fPhi)); }

  private:
    /**
     * Initialize
     */
    void Initialize();

    /**
     * Logger
     */
    LogDef* me;

  private:
    //**** Object Identification ****/
    int fIndex;
    int fUserIndex;
    int fTypePosition;
    std::string fName;
    ParticleType::Particle fParticleType;

    //****** Object Properties ******/
    double fPt;
    double fEta;
    double fPhi;
    double fE;
    double fPx;
    double fPy;
    double fPz;
    double fM;

    //****** Fit Configuration ******/
    std::vector<ParameterType::Parameter> fFitParameter;
    std::vector<double> fParamInitialValue;
    std::vector<double> fParamResolution;
    std::vector<double> fParamResolutionSquared;
    std::vector<double> fParamStepSize;
    std::vector<double> fParamLowerLimit;
    std::vector<double> fParamUpperLimit;
    std::vector<int>    fParamMinuitIndex;
    std::vector<bool>   fIsParamFixed;

    //For fast access
    int fNFitParameter;
    FitMode::Mode fFitMode; 

    //** Fitted Object Properties ***/
    double fFitPt;
    double fFitEta;
    double fFitPhi;
    double fFitE;
    double fFitPx;
    double fFitPy;
    double fFitPz;
    double fFitM;

  };//@end class FitParticle

    /**
     * A vector of fit particle pointers
     */
  typedef std::vector<FitParticle*> FitParticleSet;

}//@end namespace KF

#endif //> !KF_FITPARTICLE_H
