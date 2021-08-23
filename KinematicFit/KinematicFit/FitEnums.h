/**
 * @class KF::FitEnums
 * @brief A class to hold all global Enums
 * @author Manuel Proissl <mproissl@cern.ch>
 *
 */

#ifndef KF_FITENUMS_H
#define KF_FITENUMS_H

/**
 * @namespace KF
 * @brief The KF namespace
 */
namespace KF
{
  /**
   * @namespace Status
   * @brief Return status flag
   */
  namespace Status
  {
    enum Flag
    {
      ERROR,
      SUCCESS
    };
  }     

  /**
   * @namespace AnalysisType 
   * @brief Type of analysis
   */
  namespace AnalysisType
  {
    enum Analysis
    {
      UNDEF,
      VVBB,
      LVBB,
      LLBB
    };
  }//@end namespace Analysis

  /**
   * @namespace ParticleType 
   * @brief Type of particles
   */
  namespace ParticleType
  {
    enum Particle
    {
      Unknown,
      Neutrino,
      Electron,
      Muon,
      SoftTerm,
      //SoftTerm should be here, not to count nLep or nJet
      //There was kind of VH line here but now it is like Jet line
      Jet,
      JetMu,
      BJet,
      BJetMu,
      CJet,
      CJetMu
      //Be careful if you add something like FJet here
      //There are conditions like >= ParticleType::BJet
    };
  }//@end namespace ParticleType

  /**
   * @namespace MuonType 
   * @brief Type of Muon
   */
  namespace MuonType
  {
    enum Detector
    {
      Unknown,
      CB,
      ID,
      SA,
      Calo
    };
  }//@end namespace MuonType
    
  /**
   * @namespace ParameterType 
   * @brief Type of fit parameter
   */
  namespace ParameterType
  {
    enum Parameter
    {
      Unknown,
      Eta,
      Phi,
      Energy,
      Pt
    };
  }//@end namespace ParameterType

  /**
   * @namespace FitMode
   * @brief Mode for fit permutation
   * (temp. and for convenience only)
   */
  namespace FitMode
  {
    enum Mode
    {
      Unknown,
      EtaPhiE,
      EtaPhi,
      EtaE,
      PhiE,
      Eta,
      Phi,
      E
    };
  }

  /**
   * @namespace ConstraintType 
   * @brief Type of fit constraint
   */
  namespace ConstraintType
  {
    enum Constraint
    {
      Unknown,
      SumPx,
      SumPy,
      MET,
      Vmass,
      Vwidth,
      Hmass,
      Hwidth
    };
  }//@end namespace ConstraintType

  /**
   * @namespace Idx
   * @brief Index holders
   */
  namespace Idx
  {
    enum PtEtaPhiE
    {
      Pt,
      Eta,
      Phi,
      E
    };

    enum GlobParam
    {
      TotLHD,
      mJJ,
      mLL,
      SumPx,
      SumPy
    };

    enum PrePostFit
    {
      Prefit,
      Postfit
    };
  }//@end namespace Idx

}//@end namespace KF

#endif //> !KF_FITENUMS_H
