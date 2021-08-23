/**
 * @class KinematicFit
 * @brief A user class to configure and run a kinematic likelihood fit
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

#ifndef KF_KINEMATICFIT_H
#define KF_KINEMATICFIT_H

//Local includes
#include "FitManager.h"

/**
 * @namespace KF
 * @brief The KF namespace
 */
namespace KF
{
  class KinematicFit
  {

  public:
    /** 
     * Default constructor 
     */
    KinematicFit();

    /** 
     * Default destructor 
     */
    virtual ~KinematicFit();

    /**
     * CINT
     */
#ifdef KINEMATICFIT_STANDALONE
    ClassDef(KinematicFit,1);
#endif

    /**
     * Initialize KinematicFit
     * @brief Called during setup
     * @param Path to data dir of package
     * @param Data taking period
     * @param Flag for MC
     * @return Standard error flag
     */
    bool Init(std::string datadir="", std::string year="2012", bool isMC=true);

    /**
     * Enable or disable particle set ordering
     * @brief Called during setup
     */
    inline void DoPtOrdering(bool order) { fOrderParticleSet = order; }
        
    /**
     * Add a muon to the fit
     * @brief Called in event loop
     * @param Unique name of muon (e.g. MU1)
     * @param Pt of muon
     * @param Eta of muon
     * @param Phi of muon
     * @param Energy of muon
     * @param Inverse momentum resolution
     * @return Standard error flag
     */
    bool AddSoftTerm(const std::string& name, const TLorentzVector& tlv, const float& PRes);
    bool AddMuon(const std::string& name, const TLorentzVector& tlv, const float& PRes);
    bool AddMuon(std::string name, float pt, float eta, float phi, float E, float PRes);

    /**
     * Add a muon to the fit
     * @brief Called in event loop
     * @param Unique name of muon (e.g. MU1)
     * @param Pt of muon
     * @param Eta of muon
     * @param Phi of muon
     * @param Energy of muon
     * @param Covariance of q/p
     * @param Muon type
     *        (a) combined muons
     *        (b) segment or calorimeter tagged muons
     *        (c) stand-alone muons
     * @return Standard error flag
     */
    bool AddMuon(std::string name, float pt, float eta, float phi, float E, float qp_cov, MuonType::Detector type);

    /**
     * Add an electron to the fit
     * @brief Called in event loop
     * @param Unique name of electron (e.g. EL1)
     * @param Pt of electron
     * @param Eta of electron
     * @param Phi of electron
     * @param Energy of electron
     * @param Energy resolution
     * @return Standard error flag
     */
    bool AddElectron(const std::string& name, const TLorentzVector& tlv, const float& ERes);
    bool AddElectron(std::string name, float pt, float eta, float phi, float E, float ERes);

    /**
     * Add an electron to the fit
     * @brief Called in event loop
     * @param Unique name of electron (e.g. EL1)
     * @param Pt of electron
     * @param Eta of electron
     * @param Phi of electron
     * @param Energy of electron
     * @return Standard error flag
     */
    bool AddElectron(std::string name, float pt, float eta, float phi, float E);
        
    /**
     * Add a jet to the fit
     * @brief Called in event loop
     * @param Unique name of jet (e.g. JET1)
     * @param semileptonic or not
     * @param Pt of jet
     * @param Eta of jet
     * @param Phi of jet
     * @param Energy of jet
     * @return Standard error flag
     */
    bool AddJet(const std::string& name, const TLorentzVector& tlv, const float& ERes, const std::string& JetType, const bool& semilep);
    bool AddJet(std::string name, float pt, float eta, float phi, float E, float ERes, std::string JetType, bool semilep);
    bool AddJet(std::string name, bool semilep, float pt, float eta, float phi, float E);

    bool CheckParticlesOk() const;
    void EventReset();

    /**
     * Perform kinematic likelihood fit
     * @brief Called in event loop
     * @return Standard error flag
     */
    bool RunFit();
    bool RunFit(double SumPxyWidth);

    /* ---------------- Published Fit Results ---------------->
       Note: limited results in this tag version only        */

    /**
     * Return fit status flag
     * @brief Flag indicating any abort during runtime
     * @return Standard error flag
     */
    bool IsFitSuccess();

    /**
     * Return fit converge status
     * @return Convergence flag
     */
    bool IsFitConverged();

    /**
     * Return fit results for given object
     * @return Fitted (Pt,Eta,Phi,E) array
     */
    TLorentzVector GetFitTLVector(std::string name);
    float* GetFitFourVector(std::string name);

    /**
     * Return fit results for given object by reference
     */
    void GetFitFourVector(std::string name, float &pt, float &eta, float &phi, float &E);

    /**
     * Return measured results for given object
     * @return Measured (Pt,Eta,Phi,E) array
     */
    float* GetFourVector(std::string name);

    /**
     * Return measured results for given object by reference
     */
    void GetFourVector(std::string name, float &pt, float &eta, float &phi, float &E);

    /**
     * Return prefit parameter of given type
     * @return fit parameter value
     */
    float GetPrefitParameter(Idx::GlobParam param);

    /**
     * Return postfit parameter of given type
     * @return fit parameter value
     */
    float GetPostfitParameter(Idx::GlobParam param);
      
    /**
     * Setup KF Plotter
     * @return Standard error flag
     */
    bool SetupPlotter(TString filename);    

    /**
     * Access KF Plotter
     * @return Pointer to plotter
     */
    inline Plotter* GetPlotter() { return fPlotter; };
      
    /**
     * Plot internal analysis histograms
     * @return Standard error flag
     */
    bool WriteHistograms();

  private:     
    /**
     * Logger
     */
    LogDef* me;

    /**
     * Plotter
     */
    Plotter* fPlotter;

    /**
     * Init flag
     */
    bool isInitialized;

    /**
     * MC flag
     */
    bool fIsMC;
        
    /**
     * Path to data dir
     */
    std::string fDatadir;

    /**
     * Kinematic fit manager
     */
    FitManager* fFitManager;

    /**
     * Particle set order flag
     */
    bool fOrderParticleSet;

    /**
     * Event processing flag
     */
    bool fEventProcessed;

    /**
     * New event check
     * @return Standard error flag
     */
    void NewEvent();

    /**
     * Make all fit settings
     * @return Standard error flag
     */
    bool ConfigureFit();
    bool ConfigureFit(double SumPxyWidth);

    std::string m_year;
    bool m_particlesOk=true;

  };//@end class KinematicFit

}//@end namespace KF

#endif //> !KF_KINEMATICFIT_H
