#ifndef CxAODReader_AnalysisReader_VBFHbb1Ph_H
#define CxAODReader_AnalysisReader_VBFHbb1Ph_H

#include "CxAODReader_VHbb/AnalysisReader_VHQQ.h"

class TH3D;
class TH1F;
class TGraphAsymmErrors;

class AnalysisReader_VBFHbb1Ph : public AnalysisReader_VHQQ {
 protected:
  // Used for MODEL systematics on V+Jets and TTbar
  // CAS::EventType m_evtType; //!
  // CAS::DetailEventType m_detailevtType; //!
  // CAS::Systematic m_Systematic; //!

 public:
  AnalysisReader_VBFHbb1Ph();
  ~AnalysisReader_VBFHbb1Ph();

  virtual EL::StatusCode initializeSelection() override;
  bool m_doCutBased;
  bool m_doMVAPreSel;
  bool m_doTrigStudy;
  bool m_doBlinding;
  bool m_checkOrthogonality;
  float m_bTagCut;
  string m_CxAODTag;

  TFile *m_BJetTrigEff_file;                //!
  TGraphAsymmErrors *m_BJetTrig_EFF_Jet;    //!
  TGraphAsymmErrors *m_BJetTrig_SF_Jet;     //!
  TGraphAsymmErrors *m_BJetTrig_EFF_Event;  //!
  TH1F *hist_BJetTrig_EFF_Jet;              //!
  TH1F *hist_BJetTrig_EFF_Event;            //!

  TFile *m_SoftTermSyst_file;        //!
  TH3D *m_shiftpara_pthard_njet_mu;  //!
  TH3D *m_resopara_pthard_njet_mu;   //!
  TH3D *m_resoperp_pthard_njet_mu;   //!

  TF1 *f_dEtaJJSF;    //!
  TF1 *f_mindRBPhSF;  //!
  TF1 *f_pTJJSF;      //!
  TF1 *f_pTBalSF;     //!
  TF1 *f_TrigL1EMSF;  //!

  EL::StatusCode fill_vbf1ph();
  //
  bool jet_selection_vbf(std::vector<const xAOD::Jet *> inputJets,
                         std::vector<const xAOD::Jet *> &signalJets,
                         std::vector<const xAOD::Jet *> &vbfJets,
                         int &tagcatExcl, int &tagcatIncl);
  bool jet_selection_vbf4(std::vector<const xAOD::Jet *> inputJets,
                          std::vector<const xAOD::Jet *> &signalJets,
                          std::vector<const xAOD::Jet *> &vbfJets,
                          std::vector<const xAOD::Jet *> &vbfJets2,
                          int &tagcatExcl, int &tagcatIncl);
  bool jet_selection_vbfincl4cen(std::vector<const xAOD::Jet *> inputJets);
  bool jet_selection_vbfincl2cen(std::vector<const xAOD::Jet *> inputJets);

  bool trackJetIsInEllipse(std::pair<std::pair<float, float>, float> El,
                           const xAOD::Jet *jet, float dRBB, float r);

  std::pair<std::pair<float, float>, float> getCenterSlopeOfEllipse(
      const xAOD::Jet *jet1, const xAOD::Jet *jet2);
  // bool pass1LepTrigger(double& triggerSF_nominal, ResultVHbb1lep
  // selectionResult);

  double GetBjetTriggerWeightPt(double pt1, double pt2, bool match1,
                                bool match2, int variation = 0);
  double GetBjetTriggerWeightEta(double eta, int variation = 0);
  double getPerJetIneffSF(double eff_data, double eff_sf);
  double GetBjetSFVariation(double x, TH1F *hist, TGraphAsymmErrors *graph,
                            int variation);

  // this is needed to distribute the algorithm to the workers
  ClassDefOverride(AnalysisReader_VBFHbb1Ph, 1);
};

#endif  // ifndef CxAODReader_AnalysisReader_VBFHbb1Ph_H
