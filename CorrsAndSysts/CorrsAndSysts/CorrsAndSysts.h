#ifndef CorrsAndSysts_h
#define CorrsAndSysts_h

/*
 * CorrsAndSysts class
 *
 * supply functions to apply corrections
 * and systematics needed for the H->bb
 * analysis
 *
 *  G. Facini, N. Morange & D. Buescher
 *  Wed Dec 12 13:07:00 CET 2012
 */

//
// Take all corrections and systematics that affect shapes out of Heather's
// script
//
// You should apply:
//
// NLO EW Higgs corrections
//
// W, Z, and top backgrounds shape corrections.
//
// pT and Mbb shape systematics. Cut-based values are here, for first
// estimates/checks. Should be replaced with systematics better tailored for
// MVA.
//
// Most pT systematics are binned in pT, each bin should be varied
// independently. The other are continuous parametrizations.
//
#include "PMGTools/PMGSherpa22VJetsWeightTool.h"

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "TF1.h"
#include "TH1.h"
#include "TObject.h"
#include "TSpline.h"
#include "TString.h"

namespace CAS {

enum EventType {
  WHlvbb = 0,
  qqZHllbb,
  qqZHvvbb,
  ggZHllbb,  //**added by Lei: 2014Feb01
  ggZHvvbb,  //**added by Lei: 2014Feb01
  Z,         // inclusive Z - FLS
  W,         // inclusive W - FLS
  Wb,
  Wbb,  // sgargiul 26.10.2019 - for BDTr pTV shape
  Wbc,  // sgargiul 26.10.2019 - for BDTr pTV shape
  Wbl,  // sgargiul 26.10.2019 - for BDTr pTV shape
  Wc,
  Wcc,
  Wl,
  Zb,
  Zc,
  Zcc,
  Zl,
  ttbar,
  stop_Wt,
  stop_Wtbb,
  stop_Wtoth,
  stop_s,
  stop_t,
  WW,
  WZ,
  ZZ,
  multijet,
  NONAME
};
enum DetailEventType {
  WWincl = 0,
  WlnuZhad,
  WhadZll,
  WhadZnunu,
  ZnunuZhad,
  ZllZhad,
  WWHerwig,
  WZHerwig,
  ZZHerwig,
  topCR,
  topSR,
  Renorm,
  Fac,
  AlphaPDF,
  PDF,
  NODETAILNAME
};
enum SysVar { Do = 0, Up = 1 };
enum SysBin {
  None = -2,  // default
  Any = -1,   // means no binning -> used for continuous systematics
  Bin0 = 0,
  Bin1,
  Bin2,
  Bin3,
  Bin4,  // pT bins for binned systematics
  NOTDEFINED
};

enum Systematic  // only systematics affecting the shape are relevant to this
                 // tool
{ Nominal,
  // the following are binned in pT (5 bins, independent systematics)
  PTBINNED,
  // the following are continuous (thus correlated in pT)
  CONTINUOUS,
  SysStopWtMJ,      // gdigrego 19.11.19
  SysStoptPTV,      // yama 12.07.2016
  SysStoptMBB,      // yama 12.07.2016
  SysStopWtPTV,     // abell 29.04.18
  SysStopWtMBB,     // abell 29.04.18
  SysStopWtMTOP,    // abell 29.04.18
  SysStopWtbbACC,   // abell 29.04.18
  SysStopWtothACC,  // abell 29.04.18
  SysTTbarMJISR,    // mfoti 20.11.19
  SysTTbarMJPS,     // mfoti 21.11.19
  SysTTbarPt,
  SysTTbarPTV,      // dbuesche 29.06.16
  SysTTbarMBB,      // dbuesche 07.07.16
  SysTTbarMBB2Lep,  // eschopf 05.04.17
  SysTTbarPTVMBB,   // naoki 07.07.16
  SysWPtV,  // dbuesche 07.07.16 //sgargiul 15.4.19 //redefinition of SysWPtV ->
            // this corresponds to the shape syst derived in 2j or 3j region
  SysWPtV_BDTr,      // sgargiul 24.10.19 //pTV shape used to reweight Sherpa
  SysTTbarPtV_BDTr,  // kalkhour 14.11.19
                     // nominal when using BDTr shape systs
  SysZPtV,           // dbuesche 07.07.16
  SysWMbb,           // dbuesche 07.07.16
  SysZMbb,           // dbuesche 07.07.16
  SysWMTOP,          // sgargiul 19.06.18
  SysWPtVMbb,        // francav 15.07.16
  SysZPtVMbb,        // francav 15.07.16
  SysVVMbbME,        // francav 15.07.16
  SysVVPTVME,        // francav 15.07.16
  SysVVMbbPSUE,      // francav 15.07.16
  SysVVPTVPSUE,      // francav 15.07.16
  SysVHQCDscalePTV,  // francav 19.07.16
  SysVHQCDscaleMbb,  // francav 19.07.16
  SysVHPDFPTV,       // francav 19.07.16
  SysVHQCDscalePTV_ggZH,  // francav 19.07.16
  SysVHQCDscaleMbb_ggZH,  // francav 19.07.16
  SysVHPDFPTV_ggZH,       // francav 19.07.16
  SysVHUEPSPTV,           // francav 19.07.16
  SysVHUEPSMbb,           // francav 19.07.16
  SysVHNLOEWK,            // francav 19.07.16

  // Boosted VHbb
  SysZMbbBoosted,  // weitao 20.11.19
  SysWMbbBoosted,  // weitao 20.11.19

  //============== New PRSR TTbar Systematics=============
  SysTTbar_Boosted_shape_mVH_Pow_aMC_0lep,   // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_Pow_aMC_1lep,   // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_Pow_aMC_2lep,   // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_Her_Pyt_0lep,   // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_Her_Pyt_1lep,   // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_Her_Pyt_2lep,   // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_rLo_0lep,       // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_rLo_1lep,       // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_rLo_2lep,       // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_rHi_0lep,       // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_rHi_1lep,       // sjiggins @ 23/11/16
  SysTTbar_Boosted_shape_mVH_rHi_2lep,       // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_Pow_aMC_0lep,  // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_Pow_aMC_1lep,  // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_Pow_aMC_2lep,  // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_Her_Pyt_0lep,  // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_Her_Pyt_1lep,  // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_Her_Pyt_2lep,  // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_rLo_0lep,      // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_rLo_1lep,      // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_rLo_2lep,      // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_rHi_0lep,      // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_rHi_1lep,      // sjiggins @ 23/11/16
  SysTTbar_Resolved_shape_mVH_rHi_2lep,      // sjiggins @ 23/11/16
  //=====================================================

  //============== New PRSR Stop Systematics=============
  SysStop_Boosted_shape_mVH_Pow_aMC_1lep,   // nkondras @ 09/05/17
  SysStop_Boosted_shape_mVH_Her_Pyt_1lep,   // nkondras @ 09/05/17
  SysStop_Boosted_shape_mVH_rLo_1lep,       // nkondras @ 09/05/17
  SysStop_Boosted_shape_mVH_rHi_1lep,       // nkondras @ 09/05/17
  SysStop_Boosted_shape_mVH_DS_1lep,        // nkondras @ 13/05/17
  SysStop_Resolved_shape_mVH_Pow_aMC_1lep,  // nkondras @ 09/05/17
  SysStop_Resolved_shape_mVH_Her_Pyt_1lep,  // nkondras @ 09/05/17
  SysStop_Resolved_shape_mVH_rLo_1lep,      // nkondras @ 09/05/17
  SysStop_Resolved_shape_mVH_rHi_1lep,      // nkondras @ 09/05/17
  SysStop_Resolved_shape_mVH_DS_1lep,       // nkondras @ 13/05/17
  //=====================================================

  //============ Old HVT Moriond 2016 Systematics =============
  SysTTbar_shape_mVH_Pow_aMC_0lep,  // abell 160216
  SysTTbar_shape_mVH_Pow_aMC_2lep,  // abell 160216
  SysTTbar_shape_mVH_Her_Pyt_0lep,  // abell 160216
  SysTTbar_shape_mVH_Her_Pyt_2lep,  // abell 160216
  SysTTbar_shape_mVH_rLo_rHi_0lep,  // abell 160216
  SysTTbar_shape_mVH_rLo_rHi_2lep,  // abell 160216
  //===========================================================

  //=============== New PRSR V+Jets Modelling Systematics ============
  //----------- 0-Lepton -----------
  SysWbb_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysWbc_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysWbl_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysWcc_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysWcl_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysWl_0lep_Resolved_shape_mVH,   // sjiggins @ 13/12/16

  SysWbb_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysWbc_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysWbl_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysWcc_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysWcl_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysWl_0lep_Boosted_shape_mVH,   // sjiggins @ 13/12/16

  SysZbb_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZbc_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZbl_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZcc_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZcl_0lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZl_0lep_Resolved_shape_mVH,   // sjiggins @ 13/12/16

  SysZbb_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZbc_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZbl_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZcc_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZcl_0lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZl_0lep_Boosted_shape_mVH,   // sjiggins @ 13/12/16

  //------------ 1-Lepton ------------
  SysWbb_1lep_Resolved_shape_mVH,  // sjiggins @ 23/11/16
  SysWbc_1lep_Resolved_shape_mVH,  // sjiggins @ 23/11/16
  SysWbl_1lep_Resolved_shape_mVH,  // sjiggins @ 23/11/16
  SysWcc_1lep_Resolved_shape_mVH,  // sjiggins @ 23/11/16
  SysWcl_1lep_Resolved_shape_mVH,  // sjiggins @ 23/11/16
  SysWl_1lep_Resolved_shape_mVH,   // sjiggins @ 23/11/16

  SysWbb_1lep_Boosted_shape_mVH,  // sjiggins @ 23/11/16
  SysWbc_1lep_Boosted_shape_mVH,  // sjiggins @ 23/11/16
  SysWbl_1lep_Boosted_shape_mVH,  // sjiggins @ 23/11/16
  SysWcc_1lep_Boosted_shape_mVH,  // sjiggins @ 23/11/16
  SysWcl_1lep_Boosted_shape_mVH,  // sjiggins @ 23/11/16
  SysWl_1lep_Boosted_shape_mVH,   // sjiggins @ 23/11/16

  //------------- 2-lepton ------------
  SysZbb_2lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZbc_2lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZbl_2lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZcc_2lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZcl_2lep_Resolved_shape_mVH,  // sjiggins @ 13/12/16
  SysZl_2lep_Resolved_shape_mVH,   // sjiggins @ 13/12/16

  SysZbb_2lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZbc_2lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZbl_2lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZcc_2lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZcl_2lep_Boosted_shape_mVH,  // sjiggins @ 13/12/16
  SysZl_2lep_Boosted_shape_mVH,   // sjiggins @ 13/12/16
  //-----------------------------------
  //==================================================================

  //=========== Old HVT Moriond 2016 Model systematics =============
  SysVJets_0lep_shape_mVH,  // sjiggins
  SysVJets_2lep_shape_mVH,  // sjiggins
  //================================================================

  SysZNJet,
  LAST };
}  // end namespace CAS

class CorrsAndSysts {
 public:
  ~CorrsAndSysts() = default;

 private:
  CorrsAndSysts(){};

  void Initialize();

  int m_debug;
  bool m_passCutBased = false;

  PMGTools::PMGSherpa22VJetsWeightTool* m_PMGSherpa22VJetsWeightTool;

  // Stores the WorkDir_DIR compiled location
  std::string m_WorkDir_DIR;

  // string to enums or enums to string
  std::map<std::string, CAS::EventType> m_typeNames;
  std::map<std::string, CAS::EventType> m_typeNames_BDTr;
  std::map<std::string, CAS::DetailEventType> m_detailtypeNames;
  std::map<CAS::Systematic, std::string> m_systNames;
  std::map<CAS::SysVar, std::string> m_varNames;
  std::map<CAS::SysBin, std::string> m_binNames;
  std::map<std::string, CAS::Systematic> m_systFromNames;
  std::map<std::string, CAS::SysVar> m_varFromNames;
  std::map<std::string, CAS::SysBin> m_binFromNames;

  Float_t pTbins[6];

  Float_t m_v_ZNJet0TagSherpaCorr[5];
  Float_t m_v_ZNJet1TagSherpaCorr[5];
  Float_t m_v_ZNJet2TagSherpaCorr[5];

  Float_t m_v_ZNJet0TagMadgraphCorr[5];
  Float_t m_v_ZNJet1TagMadgraphCorr[5];
  Float_t m_v_ZNJet2TagMadgraphCorr[5];

  bool m_draw;
  bool m_zero;
  bool m_one;
  bool m_two;
  bool m_seven;
  bool m_eight;
  bool m_thirteen;

  bool m_HZZ;  // Flag for H->ZZ corrections

  // use TH* to store weights ; seems easier to maintain if ever we need e.g 2-d
  // corrections corrections
  std::shared_ptr<TH1> m_h_WHlvbbNLOEWKCorrection;
  std::shared_ptr<TH1> m_h_ZHllbbNLOEWKCorrection;
  std::shared_ptr<TH1> m_h_ZHvvbbNLOEWKCorrection;

  std::shared_ptr<TH1> m_h_WHlvbb_VpTEFTCorrection_kHdwR1Lambda1ca1;
  std::shared_ptr<TH1> m_h_ZHllbb_VpTEFTCorrection_kHdz1Lambda1ca1;
  std::shared_ptr<TH1> m_h_ZHvvbb_VpTEFTCorrection_kHdz1Lambda1ca1;
  std::shared_ptr<TH1> m_h_WHlvbb_VpTEFTCorrection_kSM1HdwR1Lambda1ca1;
  std::shared_ptr<TH1> m_h_ZHllbb_VpTEFTCorrection_kSM1Hdz1Lambda1ca1;
  std::shared_ptr<TH1> m_h_ZHvvbb_VpTEFTCorrection_kSM1Hdz1Lambda1ca1;

  std::shared_ptr<TH1> m_h_pTbins;

  std::shared_ptr<TH1> m_h_ttbarPtCorrection;

  std::shared_ptr<TF1> m_f_SysTTbarPTV_0lep;     // Peilong 20190703
  std::shared_ptr<TF1> m_f_SysTTbarPTV_0lep_2j;  // sjiggins + Peilong 09/08/19
  std::shared_ptr<TF1> m_f_SysTTbarPTV_0lep_3j;  // sjiggins + Peilong 09/08/19
  std::shared_ptr<TF1> m_f_SysTTbarPTV_1lep_2j;  // Peilong 20190703
  std::shared_ptr<TF1> m_f_SysTTbarPTV_1lep_3j;  // Peilong 20190703
  std::shared_ptr<TF1> m_f_SysTTbarPTV_2lep;     // dbuesche 29.06.16
  std::shared_ptr<TF1> m_f_SysTTbarMBB_0lep;     // Peilong 20190703
  std::shared_ptr<TF1> m_f_SysTTbarMBB_0lep_2j;  // sjiggins + Peilong 09/08/19
  std::shared_ptr<TF1> m_f_SysTTbarMBB_0lep_3j;  // sjiggins + Peilong 09/08/19
  std::shared_ptr<TF1> m_f_SysTTbarPtV_BDTr_0l_2j;  // kalkhour 14.11.19
  std::shared_ptr<TF1> m_f_SysTTbarPtV_BDTr_0l_3j;  // kalkhour 14.11.19
  std::shared_ptr<TF1> m_f_SysTTbarPtV_BDTr_1l_2j;  // kalkhour 14.11.19
  std::shared_ptr<TF1> m_f_SysTTbarPtV_BDTr_1l_3j;  // kalkhour 14.11.19
  // std::shared_ptr<TF1> m_f_SysTTbarMBB_MedPtv_0lep_2j;      // sjiggins +
  // Peilong 09/08/19 std::shared_ptr<TF1> m_f_SysTTbarMBB_MedPtv_0lep_3j; //
  // sjiggins + Peilong 09/08/19
  std::shared_ptr<TF1> m_f_SysTTbarMBB_1lep_2j;  // Peilong 20190703
  std::shared_ptr<TF1> m_f_SysTTbarMBB_1lep_3j;  // Peilong 2019070
  std::shared_ptr<TF1>
      m_f_SysTTbarMBB_MedPtv_1lep_2j;  // sjiggins + Peilong 09/08/19
  std::shared_ptr<TF1>
      m_f_SysTTbarMBB_MedPtv_1lep_3j;             // sjiggins + Peilong 09/08/19
  std::shared_ptr<TF1> m_f_SysTTbarMBB_2lep;      // dbuesche 07.07.16
  std::shared_ptr<TF1> m_f_SysTTbarMBB_2lep2jet;  // eschopf 05.04.17
  std::shared_ptr<TF1> m_f_SysTTbarMBB_2lep3jet;  // eschopf 05.04.17

  std::shared_ptr<TF1> m_f_SysStoptPTV;      // eschopf 10.05.17
  std::shared_ptr<TF1> m_f_SysStoptPTV2jet;  // yama 12.07.16
  std::shared_ptr<TF1> m_f_SysStoptPTV3jet;  // yama 12.07.16
  std::shared_ptr<TF1> m_f_SysStoptMBB;      // yama 12.07.16

  std::shared_ptr<TH1> m_h_SysStopWtMJ_M;  // gdigrego 19.11.19
  std::shared_ptr<TH1> m_h_SysStopWtMJ_H;  // gdigrego 19.11.19

  std::shared_ptr<TF1> m_f_SysStopWtbbPTV;    // abell 26.04.18
  std::shared_ptr<TF1> m_f_SysStopWtbbMBB;    // abell 26.04.18
  std::shared_ptr<TH1> m_h_SysStopWtbbMTOP;   // abell 26.04.18
  std::shared_ptr<TF1> m_f_SysStopWtothPTV;   // abell 26.04.18
  std::shared_ptr<TF1> m_f_SysStopWtothMBB;   // abell 26.04.18
  std::shared_ptr<TF1> m_f_SysStopWtothMTOP;  // abell 26.04.18

  std::shared_ptr<TF1> m_f_SysTTbarMJISR_M;  // mfoti 20.11.19
  std::shared_ptr<TF1> m_f_SysTTbarMJISR_H;  // mfoti 20.11.19
  std::shared_ptr<TH1> m_h_SysTTbarMJPS;     // mfoti 21.11.19

  std::shared_ptr<TF1> m_f_SysWPtV;             // sgargiul 15.04.19
  std::shared_ptr<TF1> m_f_SysWPtV_2j;          // sgargiul 15.04.19
  std::shared_ptr<TF1> m_f_SysWPtV_3j;          // sgargiul 15.04.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_2j_bb;  // sgargiul 24.10.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_2j_bc;  // sgargiul 24.10.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_2j_bl;  // sgargiul 24.10.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_2j_cc;  // sgargiul 24.10.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_3j_bb;  // sgargiul 24.10.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_3j_bc;  // sgargiul 24.10.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_3j_bl;  // sgargiul 24.10.19
  std::shared_ptr<TF1> m_f_SysWPtV_BDTr_3j_cc;  // sgargiul 24.10.19

  std::shared_ptr<TF1> m_f_SysZPtV;
  std::shared_ptr<TF1> m_f_SysWMbb;
  std::shared_ptr<TF1> m_f_SysWMbb_75_150_2j;  // sgargiul 08.08.19
  std::shared_ptr<TF1> m_f_SysWMbb_150_2j;     // sgargiul 08.08.19
  std::shared_ptr<TF1> m_f_SysWMbb_75_150_3j;  // sgargiul 08.08.19
  std::shared_ptr<TF1> m_f_SysWMbb_150_3j;     // sgargiul 08.08.19
  std::shared_ptr<TF1> m_f_SysZMbb;
  std::shared_ptr<TF1> m_f_SysWMTOP;  // sgargiul 19.06.18

  std::shared_ptr<TF1> m_f_SysVVMbbME_WZlvqq_2j;  // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVMbbME_ZZvvqq_2j;  // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVMbbME_ZZllqq_2j;  // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVMbbME_WZlvqq_3j;  // VD
  std::shared_ptr<TF1> m_f_SysVVMbbME_ZZvvqq_3j;  // VD
  std::shared_ptr<TF1> m_f_SysVVMbbME_ZZllqq_3j;  // VD

  std::shared_ptr<TH1> m_h_SysVVMbbPSUE_WZlvqq_2j;  // NM 17/06/15
  std::shared_ptr<TH1> m_h_SysVVMbbPSUE_ZZvvqq_2j;  // NM 17/06/15
  std::shared_ptr<TH1> m_h_SysVVMbbPSUE_ZZllqq_2j;  // NM 17/06/15
  std::shared_ptr<TH1> m_h_SysVVMbbPSUE_WZlvqq_3j;  // NM 17/06/15
  std::shared_ptr<TH1> m_h_SysVVMbbPSUE_ZZvvqq_3j;  // NM 17/06/15
  std::shared_ptr<TH1> m_h_SysVVMbbPSUE_ZZllqq_3j;  // NM 17/06/15

  std::shared_ptr<TF1> m_f_SysVVPTVME_WZlvqq_2j;    // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVME_ZZvvqq_2j;    // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVME_ZZllqq_2j;    // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVPSUE_WZlvqq_2j;  // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVPSUE_ZZvvqq_2j;  // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVPSUE_ZZllqq_2j;  // francav 15.07.16

  std::shared_ptr<TF1> m_f_SysVVPTVME_WZlvqq_3j;    // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVME_ZZvvqq_3j;    // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVME_ZZllqq_3j;    // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVPSUE_WZlvqq_3j;  // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVPSUE_ZZvvqq_3j;  // francav 15.07.16
  std::shared_ptr<TF1> m_f_SysVVPTVPSUE_ZZllqq_3j;  // francav 15.07.16

  std::shared_ptr<TF1> m_f_SysVHQCDscalePTV_WlvH_2j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHQCDscalePTV_ZvvH_2j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHQCDscalePTV_ZllH_2j;  // francav 19.07.16

  std::shared_ptr<TF1> m_f_SysVHQCDscalePTV_WlvH_3j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHQCDscalePTV_ZvvH_3j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHQCDscalePTV_ZllH_3j;  // francav 19.07.16

  std::shared_ptr<TF1> m_f_SysVHQCDscaleMbb_WlvH;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHQCDscaleMbb_ZvvH;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHQCDscaleMbb_ZllH;  // francav 19.07.16

  std::shared_ptr<TF1> m_f_SysVHPDFPTV_WlvH_2j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHPDFPTV_ZvvH_2j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHPDFPTV_ZllH_2j;  // francav 19.07.16

  std::shared_ptr<TF1> m_f_SysVHPDFPTV_WlvH_3j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHPDFPTV_ZvvH_3j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHPDFPTV_ZllH_3j;  // francav 19.07.16

  std::shared_ptr<TF1> m_f_SysVHUEPSPTV_WlvH_2j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHUEPSPTV_ZllH_2j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHUEPSPTV_ZvvH_2j;  // francav 19.07.16

  std::shared_ptr<TF1> m_f_SysVHUEPSPTV_WlvH_3j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHUEPSPTV_ZllH_3j;  // francav 19.07.16
  std::shared_ptr<TF1> m_f_SysVHUEPSPTV_ZvvH_3j;  // francav 19.07.16

  std::shared_ptr<TH1> m_h_SysVHUEPSMbb;  // NM 17/06/15

  // Boosted VHbb
  // Z+jets MUR
  std::shared_ptr<TF1> m_f_SysZMbbBoosted_Zvv_HP_250_400;
  std::shared_ptr<TF1> m_f_SysZMbbBoosted_Zvv_HP_400;
  std::shared_ptr<TF1> m_f_SysZMbbBoosted_Zvv_LP_250_400;
  std::shared_ptr<TF1> m_f_SysZMbbBoosted_Zvv_LP_400;
  std::shared_ptr<TF1> m_f_SysZMbbBoosted_Zll_250_400;
  std::shared_ptr<TF1> m_f_SysZMbbBoosted_Zll_400;  // weitao 20.11.19
  // W+jets MUR
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_0L_HP_250_400;
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_0L_HP_400;
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_0L_LP_250_400;
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_0L_LP_400;
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_1L_HP_250_400;
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_1L_HP_400;
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_1L_LP_250_400;
  std::shared_ptr<TF1> m_f_SysWMbbBoosted_1L_LP_400;  // weitao 20.11.19

  //============== HVT & AZH VH resonance ttbar systematics - sjiggins @
  // 23/11/16 =================

  //--- 0-lepton ---

  // Resolved analysis systematics
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_0lep_pretag_topCR_rHi_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1>
      m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_SRmbb_Her_Pyt_mVH;
  std::shared_ptr<TF1>
      m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_CRmbb_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1>
      m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_SRmbb_Pow_aMC_mVH;
  std::shared_ptr<TF1>
      m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_CRmbb_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_SRmbb_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_CRmbb_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_topCR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_SRmbb_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_0lep_pretag_addtopCR_CRmbb_rHi_mVH;

  // Resolved analysis systematics
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_0lep_pretag_topCR_rHi_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_0lep_pretag_topCR_rHi_mVH;

  //--- 1-lepton ---

  //################ Smoothed TF1 systematics ###################
  // Resolved analysis systematics
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_1lep_pretag_topCR_rHi_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_CR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_CR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_CR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_CR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_rHi_mVH;
  //##############################################################

  //#################### Binned TH1 Systematics ##################
  // Resolved analysis systematics
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_1lep_pretag_topCR_rHi_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_CR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TH1>
      m_h1_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_CR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TH1>
      m_h1_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_CR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_CR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_topCR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_1lep_pretag_topCR_CRmbb_rHi_mVH;
  //##############################################################

  //--- 2-lepton ---

  // Resolved analysis systematics
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_emu_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_emu_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_emu_SR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_emu_SR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Resolved_2lep_pretag_topCR_rHi_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_Boosted_2lep_pretag_topCR_rHi_mVH;

  // Resolved analysis systematics
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_emu_SR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_emu_SR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_emu_SR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_emu_SR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Resolved_2lep_pretag_topCR_rHi_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_SR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_topCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_SR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysTtbar_Boosted_2lep_pretag_topCR_rHi_mVH;

  //=====================================================================================

  //============== HVT VH resonance stop systematics - nkondras @ 09/05/17
  //=================

  //--- 1-lepton ---

  //################ Smoothed TF1 systematics ###################
  // Resolved analysis systematics
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbSR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbSR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbSR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbSR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbCR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbSR_DS_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Resolved_1lep_pretag_mbbCR_DS_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbSR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbSR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbSR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbSR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbSR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbSR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbCR_rLo_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbSR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbCR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbSR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbCR_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbSR_DS_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_mbbCR_DS_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbSR_DS_mVH;
  std::shared_ptr<TF1> m_h_SysStop_Boosted_1lep_pretag_topCR_mbbCR_DS_mVH;
  //##############################################################

  //#################### Binned TH1 Systematics ##################
  // Resolved analysis systematics
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbSR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbSR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbSR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbSR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbCR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbSR_DS_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Resolved_1lep_pretag_mbbCR_DS_mVH;

  // Boosted analysis systematics
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbSR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbSR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbCR_Her_Pyt_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbSR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbSR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbCR_Pow_aMC_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbSR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbSR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbCR_rLo_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbSR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbCR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbSR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbCR_rHi_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbSR_DS_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_mbbCR_DS_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbSR_DS_mVH;
  std::shared_ptr<TH1> m_h1_SysStop_Boosted_1lep_pretag_topCR_mbbCR_DS_mVH;
  //##############################################################

  //=====================================================================================

  //=========== VH resonance ttbar systematics: added by Andrew Bell 160216
  //=============
  std::shared_ptr<TF1> m_h_SysTtbar_0lep_pretag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_0lep_pretag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_0lep_pretag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_0lep_pretag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_0lep_pretag_SR_rLo_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_0lep_pretag_topCR_rLo_rHi_mVH;

  std::shared_ptr<TF1> m_h_SysTtbar_2lep_0tag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_0tag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_0tag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_0tag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_0tag_SR_rLo_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_0tag_topCR_rLo_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_1tag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_1tag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_1tag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_1tag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_1tag_SR_rLo_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_1tag_topCR_rLo_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_2tag_SR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_2tag_topCR_Her_Pyt_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_2tag_SR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_2tag_topCR_Pow_aMC_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_2tag_SR_rLo_rHi_mVH;
  std::shared_ptr<TF1> m_h_SysTtbar_2lep_2tag_topCR_rLo_rHi_mVH;

  //========== New PRSR HVT/AZH modelling systematics ============

  //----- 0-Lepton -----
  // sjiggins @ 13/12/16

  // Resolved
  std::shared_ptr<TF1> m_f_SysWbb_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_0lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysWbb_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_0lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TF1> m_f_SysWbb_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_0lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysWbb_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_0lep_Boosted_SR_SherpaMG5_mVH;

  // Resolved
  std::shared_ptr<TF1> m_f_SysZbb_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_0lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysZbb_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_0lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TF1> m_f_SysZbb_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_0lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysZbb_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_0lep_Boosted_SR_SherpaMG5_mVH;

  // h1

  // Resolved
  std::shared_ptr<TH1> m_h1_SysWbb_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWl_0lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h1_SysWbb_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWl_0lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TH1> m_h1_SysWbb_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWl_0lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h1_SysWbb_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWbl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWcl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysWl_0lep_Boosted_SR_SherpaMG5_mVH;

  // Resolved
  std::shared_ptr<TH1> m_h1_SysZbb_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_0lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_0lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h1_SysZbb_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_0lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_0lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TH1> m_h1_SysZbb_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_0lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_0lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h1_SysZbb_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_0lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_0lep_Boosted_SR_SherpaMG5_mVH;

  //---------------------

  //----- 1-Lepton -----
  // sjiggins @ 23/11/16

  //############## Smoothed TF1 Systematics ##################
  // Resolved
  std::shared_ptr<TF1> m_f_SysWbb_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_1lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysWbb_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_1lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TF1> m_f_SysWbb_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_1lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysWbb_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_1lep_Boosted_SR_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysWbb_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_1lep_Boosted_TopCR_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysWbb_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbc_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWbl_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcc_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWcl_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;

  //###################### TSpline3 Systematics ########################
  // Resolved + Boosted
  std::map<std::string, std::shared_ptr<TH1>>
      m_s_VHExotics_Sherpa221_VJetsSysts;

  //###################### Binned TH1 Systematics ######################
  // Resolved
  std::shared_ptr<TH1> m_h_SysWbb_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbc_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbl_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcc_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcl_1lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWl_1lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h_SysWbb_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbc_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbl_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcc_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcl_1lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWl_1lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TH1> m_h_SysWbb_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbc_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbl_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcc_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcl_1lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWl_1lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h_SysWbb_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbc_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbl_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcc_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcl_1lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWl_1lep_Boosted_SR_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h_SysWbb_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbc_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbl_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcc_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcl_1lep_Boosted_TopCR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWl_1lep_Boosted_TopCR_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h_SysWbb_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbc_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWbl_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcc_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWcl_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h_SysWl_1lep_Boosted_TopCR_CRmbb_SherpaMG5_mVH;
  //---------------------

  //----- 2-Lepton -----
  // sjiggins @ 13/12/16

  // Resolved
  std::shared_ptr<TF1> m_f_SysZbb_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_2lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysZbb_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_2lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TF1> m_f_SysZbb_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_2lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TF1> m_f_SysZbb_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbc_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZbl_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcc_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZcl_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_2lep_Boosted_SR_SherpaMG5_mVH;

  // Resolved
  std::shared_ptr<TH1> m_h1_SysZbb_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_2lep_Resolved_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_2lep_Resolved_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h1_SysZbb_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_2lep_Resolved_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_2lep_Resolved_SR_SherpaMG5_mVH;

  // Boosted
  std::shared_ptr<TH1> m_h1_SysZbb_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_2lep_Boosted_CRmbb_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_2lep_Boosted_CRmbb_SherpaMG5_mVH;

  std::shared_ptr<TH1> m_h1_SysZbb_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbc_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZbl_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcc_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZcl_2lep_Boosted_SR_SherpaMG5_mVH;
  std::shared_ptr<TH1> m_h1_SysZl_2lep_Boosted_SR_SherpaMG5_mVH;

  //---------------------
  //==============================================================

  //========== Old HVT V+Jet mVH systematics ===========
  // 0-Lepton
  std::shared_ptr<TF1> m_f_SysZb_0lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZc_0lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_0lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZb_0lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZc_0lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_0lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZb_0lep_highMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZc_0lep_highMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_0lep_highMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWb_0lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWc_0lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_0lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWb_0lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWc_0lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_0lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWb_0lep_highMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWc_0lep_highMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysWl_0lep_highMH_SherpaMG5_mVH;

  // 2-Lepton
  std::shared_ptr<TF1> m_f_SysZb_2lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZc_2lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_2lep_lowMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZb_2lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZc_2lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_2lep_SR_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZb_2lep_highMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZc_2lep_highMH_SherpaMG5_mVH;
  std::shared_ptr<TF1> m_f_SysZl_2lep_highMH_SherpaMG5_mVH;
  //============================================

  std::map<TString, std::shared_ptr<TObject>> m_allHists;

 public:
  CorrsAndSysts(TString name, bool draw = true,
                bool HZZ = false);  // e.g OneLepton_8TeV
  CorrsAndSysts(
      int channel, int year, bool draw = true,
      bool HZZ = false);  // channel: 0->0lepton, 1->1lepton, 2->2leptons

  inline void SetDebug(int i) { m_debug = i; }

  CAS::DetailEventType GetDetailEventType(TString name);
  CAS::EventType GetEventType(TString name);
  CAS::EventType GetEventType_BDTr(TString name);
  CAS::SysBin GetSysBin(float vpt);

  void WriteHistsToFile(TString fname);
  void SetCutBased(bool passCutBased);
  // all values (VpT, Mbb) in MeV !

  // Higgs pT reweighting (NLO EW corrections)
  float Get_HiggsNLOEWKCorrection(CAS::EventType type, float VpT);
  inline float Get_HiggsNLOEWKCorrection(TString evtType, float VpT) {
    return Get_HiggsNLOEWKCorrection(m_typeNames[evtType.Data()], VpT);
  }

  // VpT EFT reweighting for SM VH
  float Get_VpTEFTCorrection_kSM1HdvR1Lambda1ca1(CAS::EventType type,
                                                 float VpT);
  inline float Get_VpTEFTCorrection_kSM1HdvR1Lambda1ca1(TString evtType,
                                                        float VpT) {
    return Get_VpTEFTCorrection_kSM1HdvR1Lambda1ca1(m_typeNames[evtType.Data()],
                                                    VpT);
  }

  float Get_VpTEFTCorrection_kHdvR1Lambda1ca1(CAS::EventType type, float VpT);
  inline float Get_VpTEFTCorrection_kHdvR1Lambda1ca1(TString evtType,
                                                     float VpT) {
    return Get_VpTEFTCorrection_kHdvR1Lambda1ca1(m_typeNames[evtType.Data()],
                                                 VpT);
  }

  float Get_BkgNJetCorrection(CAS::EventType type, int njet, int ntag,
                              bool sherpa);

  // Sherpa 2.2 generation scale multiplicity fix
  float Get_BkgNJetCorrection(CAS::EventType type, int njet);

  // Systematics on distributions
  float Get_SystematicWeight(CAS::EventType type, float VpT, float Mbb,
                             float truthPt, float DeltaPhi, float deltaR,
                             float pTB1, float pTB2, float met, int njet,
                             int ntag, int mc_channel_number,
                             CAS::Systematic sys = CAS::Nominal,
                             CAS::SysVar var = CAS::Up,
                             CAS::SysBin bin = CAS::None);
  float Get_SystematicWeight(CAS::EventType type, float VpT, float Mbb,
                             float truthPt, float DeltaPhi, float deltaR,
                             float pTB1, float pTB2, float met, int njet,
                             int ntag, int mc_channel_number, TString sysName);

  float Get_SystematicWeight(TString evtType, float VpT, float Mbb,
                             float truthPt, float DeltaPhi, float deltaR,
                             float pTB1, float pTB2, float met, int njet,
                             int ntag, int mc_channel_number,
                             CAS::Systematic sys, CAS::SysVar var,
                             CAS::SysBin bin);
  float Get_SystematicWeight(TString evtType, float VpT, float Mbb,
                             float truthPt, float DeltaPhi, float deltaR,
                             float pTB1, float pTB2, float met, int njet,
                             int ntag, int mc_channel_number, TString sysName);

  float Get_SystematicWeight(CAS::EventType type, float Mtop,
                             CAS::Systematic sys, CAS::SysVar var);

  // same without mc_channel_number
  float Get_SystematicWeight(CAS::EventType type, float VpT, float Mbb,
                             float truthPt, float DeltaPhi, float deltaR,
                             float pTB1, float pTB2, float met, int njet,
                             int ntag, CAS::DetailEventType detailtype,
                             CAS::Systematic sys = CAS::Nominal,
                             CAS::SysVar var = CAS::Up,
                             CAS::SysBin bin = CAS::None, bool print = false);
  float Get_SystematicWeight(CAS::EventType type, float VpT, float Mbb,
                             float truthPt, float DeltaPhi, float deltaR,
                             float pTB1, float pTB2, float met, int njet,
                             int ntag, CAS::DetailEventType detailtype,
                             TString sysName);

  inline float Get_SystematicWeight(TString evtType, float VpT, float Mbb,
                                    float truthPt, float DeltaPhi, float deltaR,
                                    float pTB1, float pTB2, float met, int njet,
                                    int ntag, CAS::DetailEventType detailtype,
                                    CAS::Systematic sys, CAS::SysVar var,
                                    CAS::SysBin bin) {
    return Get_SystematicWeight(m_typeNames[evtType.Data()], VpT, Mbb, truthPt,
                                DeltaPhi, deltaR, pTB1, pTB2, met, njet, ntag,
                                detailtype, sys, var, bin);
  }

  inline float Get_SystematicWeight(TString evtType, float VpT, float Mbb,
                                    float truthPt, float DeltaPhi, float deltaR,
                                    float pTB1, float pTB2, float met, int njet,
                                    int ntag, CAS::DetailEventType detailtype,
                                    TString sysName) {
    return Get_SystematicWeight(m_typeNames[evtType.Data()], VpT, Mbb, truthPt,
                                DeltaPhi, deltaR, pTB1, pTB2, met, njet, ntag,
                                detailtype, sysName);
  }

  // forge the normalized syste name from the enums
  inline std::string GetSystName(CAS::Systematic sys, CAS::SysBin bin,
                                 CAS::SysVar var) {
    return m_systNames[sys] + m_binNames[bin] + m_varNames[var];
  }

  // inverse function
  void GetSystFromName(TString name, CAS::Systematic& sys, CAS::SysBin& bin,
                       CAS::SysVar& var);

  // faster method, which does not allow to determine bin or direction
  void GetSystFromName(const TString& name, CAS::Systematic& sys);
};  // close CorrsAndSysts class

namespace Utils {

// utility
std::shared_ptr<TH1F> BuildTH1(
    std::vector<Double_t> contents, TString hname, float min, float max,
    std::map<TString, std::shared_ptr<TObject>>& hists);
void FillTH1(std::vector<Float_t> contents, std::shared_ptr<TH1> h,
             std::map<TString, std::shared_ptr<TObject>>& hists);
void FillTH1(int len, Float_t* contents, std::shared_ptr<TH1> h,
             std::map<TString, std::shared_ptr<TObject>>& hists);
inline float GetScale(float value, std::shared_ptr<TH1> h);

template <typename T>
void SaveHist(std::shared_ptr<T> h,
              std::map<TString, std::shared_ptr<TObject>>& hists) {
  if (hists.find(h->GetName()) != hists.end()) {
    std::cout << "CorrsAndSysts::ERROR - non-unique name of histogram/function "
                 "is being used - please correct"
              << std::endl;
    exit(-1);
  }
  std::shared_ptr<TObject> objPtr = std::shared_ptr<TObject>(h);
  hists[h->GetName()] = objPtr;
}

void ArraySubstractOne(float* array, unsigned int length);
CAS::DetailEventType GetDibosonType(int mc_channel_number, CAS::EventType type);

// map<K,V> => map<V,K>
template <typename T, typename U>
std::map<U, T> reverseMap(const std::map<T, U>& m_in);

// Implementation
//
template <typename T, typename U>
std::map<U, T> reverseMap(const std::map<T, U>& m_in) {
  typedef typename std::map<T, U>::const_iterator map_it;
  map_it it = m_in.begin();
  std::map<U, T> m_out;
  while (it != m_in.end()) {
    m_out[it->second] = it->first;
    it++;
  }
  return m_out;
}

// Template function for deleting pointers inside a map whereby the map has the
// format
// FORMAT ::     std::map< typename A, typename B > m_Map
//        Where:  A = any object used as a key
//                B = Pointer to object needed for deletion.
template <typename Iter>
void DeleteMapPointers(Iter begin, Iter end) {
  // Loop through and delete pointed objects
  for (; begin != end; begin++) {
    delete begin->second;
  }
}

}  // namespace Utils

#endif  // CorrsAndSysts_HPP_
