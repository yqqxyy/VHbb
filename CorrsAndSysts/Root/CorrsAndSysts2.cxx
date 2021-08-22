#include "CorrsAndSysts/CorrsAndSysts2.h"
#include <algorithm>
#include <cmath>
#include "TFile.h"

#include <TVector2.h>

/*
 * See header file for description
 *
 */

CorrsAndSysts2::CorrsAndSysts2(TString name, bool draw, bool HZZ)
    : m_debug(0),
      m_PMGSherpa22VJetsWeightTool(nullptr),
      m_zero(false),
      m_one(false),
      m_two(false),
      m_seven(false),
      m_eight(false),
      m_thirteen(false) {
  m_draw = draw;
  m_HZZ = HZZ;

  // set analysis basesd on contentes of name
  if (name.Contains("Zero") || name.Contains("zero") || name.Contains("ZERO")) {
    m_zero = true;
  }
  if (name.Contains("One") || name.Contains("one") || name.Contains("ONE")) {
    m_one = true;
  }
  if (name.Contains("Two") || name.Contains("two") || name.Contains("TWO")) {
    m_two = true;
  }

  // check lepton selection chosen correctly
  if ((m_zero && m_one) || (m_zero && m_two) || (m_one && m_two)) {
    std::cout << "CorrsAndSysts2 initialized for multiple analyses"
              << std::endl;
    exit(-1);
  }
  if (!m_zero && !m_one && !m_two) {
    std::cout << "CorrsAndSysts2 initialized for none of the analyses"
              << std::endl;
    exit(-1);
  }

  // set year
  if (name.Contains("7TeV")) {
    m_seven = true;
  }
  if (name.Contains("8TeV")) {
    m_eight = true;
  }
  if (name.Contains("13TeV")) {
    m_thirteen = true;
  }

  // check year is set correctly
  if (m_seven && m_eight) {
    std::cout << "CorrsAndSysts2 initialized for both 7 and 8 TeV" << std::endl;
    exit(-1);
  }
  /*
  if(!m_seven && !m_eight) {
    std::cout << "CorrsAndSysts2 initialized for neither 7 nor 8 TeV" <<
  std::endl; exit(-1);
  }
  */

  // write some stuff so users can be confident they did this correctly
  std::cout << "Initalize CorrsAndSysts2 for ";
  if (m_zero) {
    std::cout << "Zero";
  }
  if (m_one) {
    std::cout << "One";
  }
  if (m_two) {
    std::cout << "Two";
  }
  std::cout << " Lepton ";
  if (m_seven) {
    std::cout << "7";
  }
  if (m_eight) {
    std::cout << "8";
  }
  if (m_thirteen) {
    std::cout << "13";
  }
  std::cout << " TeV analysis" << std::endl;

  // initalize the corrections and systematics
  Initialize();

}  // CorrsAndSysts2

CorrsAndSysts2::CorrsAndSysts2(int channel, int year, bool draw, bool HZZ)
    : m_debug(0),
      m_zero(false),
      m_one(false),
      m_two(false),
      m_seven(false),
      m_eight(false),
      m_thirteen(false) {
  m_draw = draw;
  m_HZZ = HZZ;

  switch (channel) {
    case 0:
      m_zero = true;
      break;
    case 1:
      m_one = true;
      break;
    case 2:
      m_two = true;
      break;
    default:
      std::cout << "CorrsAndSysts2 initialized for none of the analyses"
                << std::endl;
      exit(-1);
  }
  switch (year) {
    case 2011:
      m_seven = true;
      break;
    case 2012:
      m_eight = true;
      break;
    case 2015:
      m_thirteen = true;
      break;
    case 2016:
      m_thirteen = true;
      break;
    default:
      std::cout
          << "CorrsAndSysts2 not initialized correctly for 7 nor 8 nor 13 TeV"
          << std::endl;
      exit(-1);
  }

  // write some stuff so users can be confident they did this correctly
  std::cout << "Initalize CorrsAndSysts2 for ";
  if (m_zero) {
    std::cout << "Zero";
  }
  if (m_one) {
    std::cout << "One";
  }
  if (m_two) {
    std::cout << "Two";
  }
  std::cout << " Lepton ";
  if (m_seven) {
    std::cout << "7";
  }
  if (m_eight) {
    std::cout << "8";
  }
  if (m_thirteen) {
    std::cout << "13";
  }
  std::cout << " TeV analysis" << std::endl;

  // initalize the corrections and systematics
  Initialize();

}  // CorrsAndSysts2

CorrsAndSysts2::~CorrsAndSysts2() {
  delete m_h_WHlvbbNLOEWKCorrection;
  delete m_h_ZHllbbNLOEWKCorrection;
  delete m_h_ZHvvbbNLOEWKCorrection;

  delete m_h_ggZllH110_2JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZllH110_3JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZllH125_2JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZllH125_3JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZllH140_2JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZllH140_3JetCorrection;  //** added by Lei: 2014jan31

  delete m_h_ggZvvH110_2JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZvvH110_3JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZvvH125_2JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZvvH125_3JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZvvH140_2JetCorrection;  //** added by Lei: 2014jan31
  delete m_h_ggZvvH140_3JetCorrection;  //** added by Lei: 2014jan31

  delete m_h_pTbins;

  delete m_h_topPtCorrection;

  delete m_h_SysHerwigPt;

  delete m_h_SysWW2JetInclScalePt;
  delete m_h_SysWW2JetScalePt;
  delete m_h_SysWW3JetScalePt;
  delete m_h_SysWW3JetFor2JetScalePt;
  delete m_h_SysWlnuZhad2JetInclScalePt;
  delete m_h_SysWlnuZhad2JetScalePt;
  delete m_h_SysWlnuZhad3JetScalePt;
  delete m_h_SysWlnuZhad3JetFor2JetScalePt;
  delete m_h_SysWhadZnunu2JetInclScalePt;
  delete m_h_SysWhadZnunu2JetScalePt;
  delete m_h_SysWhadZnunu3JetScalePt;
  delete m_h_SysWhadZnunu3JetFor2JetScalePt;
  delete m_h_SysWhadZll2JetInclScalePt;
  delete m_h_SysWhadZll2JetScalePt;
  delete m_h_SysWhadZll3JetScalePt;
  delete m_h_SysWhadZll3JetFor2JetScalePt;
  delete m_h_SysZllZhad2JetInclScalePt;
  delete m_h_SysZllZhad2JetScalePt;
  delete m_h_SysZllZhad3JetScalePt;
  delete m_h_SysZllZhad3JetFor2JetScalePt;
  delete m_h_SysZnunuZhad2JetInclScalePt;
  delete m_h_SysZnunuZhad2JetScalePt;
  delete m_h_SysZnunuZhad3JetScalePt;
  delete m_h_SysZnunuZhad3JetFor2JetScalePt;

  delete m_h_SysWW2JetPDFAlphaPt;
  delete m_h_SysWW3JetPDFAlphaPt;
  delete m_h_SysWlnuZhad2JetPDFAlphaPt;
  delete m_h_SysWlnuZhad3JetPDFAlphaPt;
  delete m_h_SysWhadZnunu2JetPDFAlphaPt;
  delete m_h_SysWhadZnunu3JetPDFAlphaPt;
  delete m_h_SysWhadZll2JetPDFAlphaPt;
  delete m_h_SysWhadZll3JetPDFAlphaPt;
  delete m_h_SysZllZhad2JetPDFAlphaPt;
  delete m_h_SysZllZhad3JetPDFAlphaPt;
  delete m_h_SysZnunuZhad2JetPDFAlphaPt;
  delete m_h_SysZnunuZhad3JetPDFAlphaPt;

  delete m_h_SysLepVeto2JetTop;
  delete m_h_SysLepVeto2JetStop;
  delete m_h_SysLepVeto2JetWbb;
  delete m_h_SysLepVeto2JetWW;
  delete m_h_SysLepVeto2JetWZ;
  delete m_h_SysLepVeto3JetTop;
  delete m_h_SysLepVeto3JetStop;
  delete m_h_SysLepVeto3JetWbb;
  delete m_h_SysLepVeto3JetWW;
  delete m_h_SysLepVeto3JetWZ;

  delete m_h_SysMJ_dRBB_2lltag;
  delete m_h_SysMJ_dRBB_2mmtag;
  delete m_h_SysMJ_dRBB_2tttag;

  delete m_h_SysMJ_pTV_2lltag;
  delete m_h_SysMJ_pTV_2mmtag;
  delete m_h_SysMJ_pTV_2tttag;
  //** added by Lei: 2014jan31
  delete m_f_SysTheoryqqVH2JetPtQCD;
  delete m_f_SysTheoryggZH2JetPtQCD;
  delete m_f_SysTheoryqqVH3JetPtQCD;
  delete m_f_SysTheoryggZH3JetPtQCD;

  delete m_h_SysTheoryWHlvbbPt;
  delete m_h_SysTheoryZHllbbPt;
  delete m_h_SysTheoryZHvvbbPt;

  delete m_f_SysZDPhi;
  delete m_f_SysZPtV;
  delete m_f_SysZMbb;

  delete m_f_SysWDPhi;
  delete m_f_SysWPtV;
  delete m_f_SysWMbb;

  delete m_f_SysTtbarMbb;
  delete m_f_SysTtbarPtbb;

  delete m_f_SysWWMbb;
  delete m_f_SysWZMbb;
  delete m_f_SysZZMbb;

  delete m_f_SysTChanPt;
  delete m_f_SysWtPt2jLowPt;
  delete m_f_SysWtPt2jHighPt;
  delete m_f_SysWtPt3jLowPt;
  delete m_f_SysWtPt3jHighPt;

  // This are new for
}

void CorrsAndSysts2::Initialize() {
  // This cannot be set in a setter as things are setup only once here

  // Set up PMG Tool
  m_PMGSherpa22VJetsWeightTool =
      new PMGTools::PMGSherpa22VJetsWeightTool("PMGSherpa22VJetsWeightTool");

  // initialize the mappings to event types
  m_typeNames["WHlvbb"] = CAS::WHlvbb;
  m_typeNames["qqZHllbb"] = CAS::qqZHllbb;
  m_typeNames["qqZHvvbb"] = CAS::qqZHvvbb;
  m_typeNames["WlvH"] = CAS::WHlvbb;
  m_typeNames["qqZllH"] = CAS::qqZHllbb;
  m_typeNames["qqZvvH"] = CAS::qqZHvvbb;
  //** added by Lei: 2014jan31
  m_typeNames["ggZllH"] = CAS::ggZHllbb;
  m_typeNames["ggZvvH"] = CAS::ggZHvvbb;
  m_typeNames["ggZHllbb"] = CAS::ggZHllbb;
  m_typeNames["ggZHvvbb"] = CAS::ggZHvvbb;

  m_typeNames["Wb"] = CAS::Wb;
  m_typeNames["Wbb"] = CAS::Wb;
  m_typeNames["Wbc"] = CAS::Wb;
  m_typeNames["Wbl"] = CAS::Wb;
  m_typeNames["Wc"] = CAS::Wc;
  m_typeNames["Wcl"] = CAS::Wc;
  m_typeNames["Wcc"] = CAS::Wcc;
  m_typeNames["Wl"] = CAS::Wl;
  m_typeNames["Wll"] = CAS::Wl;
  m_typeNames["Zb"] = CAS::Zb;
  m_typeNames["Zbb"] = CAS::Zb;
  m_typeNames["Zbc"] = CAS::Zb;
  m_typeNames["Zbl"] = CAS::Zb;
  m_typeNames["Zc"] = CAS::Zc;
  m_typeNames["Zcc"] = CAS::Zcc;
  m_typeNames["Zcl"] = CAS::Zc;
  m_typeNames["Zl"] = CAS::Zl;
  m_typeNames["Zll"] = CAS::Zl;
  m_typeNames["top"] = CAS::ttbar;
  m_typeNames["ttbar"] = CAS::ttbar;
  m_typeNames["stop_Wt"] = CAS::stop_Wt;
  m_typeNames["stop_s"] = CAS::stop_s;
  m_typeNames["stop_t"] = CAS::stop_t;
  m_typeNames["stopWt"] = CAS::stop_Wt;
  m_typeNames["stops"] = CAS::stop_s;
  m_typeNames["stopt"] = CAS::stop_t;
  m_typeNames["WW"] = CAS::WW;
  m_typeNames["WZ"] = CAS::WZ;
  m_typeNames["ZZ"] = CAS::ZZ;
  m_typeNames["multijet"] = CAS::multijet;

  m_detailtypeNames["Sherpa_CT10_WpqqWmlv"] = CAS::WWincl;
  m_detailtypeNames["Sherpa_CT10_WplvWmqq"] = CAS::WWincl;
  m_detailtypeNames["Sherpa_CT10_WlvZqq"] = CAS::WlnuZhad;
  m_detailtypeNames["Sherpa_CT10_WqqZll"] = CAS::WhadZll;
  m_detailtypeNames["Sherpa_CT10_WqqZvv"] = CAS::WhadZnunu;
  m_detailtypeNames["Sherpa_CT10_ZqqZvv"] = CAS::ZnunuZhad;
  m_detailtypeNames["Sherpa_CT10_ZqqZll"] = CAS::ZllZhad;
  m_detailtypeNames["NODETAILNAME"] = CAS::NODETAILNAME;

  m_systNames[CAS::Nominal] = "";
  m_systNames[CAS::SysTheoryHPt] = "SysTheoryHPt";
  m_systNames[CAS::SysTheoryVPtQCD] = "SysTheoryVPtQCD";

  m_systNames[CAS::SysWtChanAcerMC] = "SysWtChanAcerMC";
  m_systNames[CAS::SysWtChanPythiaHerwig] = "SysWtChanPythiaHerwig";
  m_systNames[CAS::SysTChanPtB2] = "SysTChanPtB2";
  m_systNames[CAS::SysHerwigPt] = "SysHerwigPt";

  m_systNames[CAS::SysVVJetScalePt] = "SysVVJetScalePt";
  m_systNames[CAS::SysVVJetScalePtST1] = "SysVVJetScalePtST1";
  m_systNames[CAS::SysVVJetScalePtST2] = "SysVVJetScalePtST2";
  m_systNames[CAS::SysVVJetPDFAlphaPt] = "SysVVJetPDFAlphaPt";

  m_systNames[CAS::SysLepVeto] = "SysLepVeto";
  m_systNames[CAS::SysVVMbb] = "SysVVMbb";

  m_systNames[CAS::SysWDPhi] = "SysWDPhi";
  m_systNames[CAS::SysWPtV] = "SysWPtV";
  m_systNames[CAS::SysWMbb] = "SysWMbb";

  m_systNames[CAS::SysZDPhi] = "SysZDPhi";
  m_systNames[CAS::SysZPtV] = "SysZPtV";
  m_systNames[CAS::SysZMbb] = "SysZMbb";

  m_systNames[CAS::SysTtbarMbb] = "SysTtbarMbb";
  m_systNames[CAS::SysTtbarPtbb] = "SysTtbarPtbb";
  m_systNames[CAS::SysTopPt] = "SysTopPt";

  m_systNames[CAS::SysMJElDR] = "SysMJElDR";
  m_systNames[CAS::SysMJElPtV] = "SysMJElPtV";

  m_varNames[CAS::Up] = "Up";
  m_varNames[CAS::Do] = "Do";

  m_binNames[CAS::None] = "";
  m_binNames[CAS::Any] = "";
  m_binNames[CAS::Bin0] = "Bin0";
  m_binNames[CAS::Bin1] = "Bin1";
  m_binNames[CAS::Bin2] = "Bin2";
  m_binNames[CAS::Bin3] = "Bin3";
  m_binNames[CAS::Bin4] = "Bin4";

  m_systFromNames =
      Utils2::reverseMap<CAS::Systematic, std::string>(m_systNames);
  m_varFromNames = Utils2::reverseMap<CAS::SysVar, std::string>(m_varNames);
  // resolve ambiguity
  m_binFromNames = Utils2::reverseMap<CAS::SysBin, std::string>(m_binNames);
  m_binFromNames[""] = CAS::Any;

  // V pT bins. For now 5 bins as in cut-based
  pTbins[0] = 0.;
  pTbins[1] = 90.e3;
  pTbins[2] = 120.e3;
  pTbins[3] = 160.e3;
  pTbins[4] = 200.e3;
  pTbins[5] = 500.e3;

  /*********************************************
   *
   *    CORRECTIONS
   *    These should be applied to the nominal
   *
   *    Below the histograms which contain the
   *    values of the correction are created
   *
   *********************************************/

  // signal NLO EW corrections
  m_h_WHlvbbNLOEWKCorrection =
      new TH1F("WHlvbbpTCorr", "WHlvbbpTCorr", 95, 25.e3, 500.e3);
  m_h_WHlvbbNLOEWKCorrection->SetDirectory(0);
  m_h_ZHllbbNLOEWKCorrection =
      new TH1F("ZHllbbpTCorr", "ZHllbbpTCorr", 95, 25.e3, 500.e3);
  m_h_ZHllbbNLOEWKCorrection->SetDirectory(0);
  m_h_ZHvvbbNLOEWKCorrection =
      new TH1F("ZHvvbbpTCorr", "ZHvvbbpTCorr", 95, 25.e3, 500.e3);
  m_h_ZHvvbbNLOEWKCorrection->SetDirectory(0);

  // numbers from Jason
  if (m_seven) {
    Float_t a_whlvbbcorr[95] = {
        0.00224198,  -0.000370526, -0.00169178, -0.00189401, -0.00200129,
        -0.00301404, -0.00325795,  -0.0041637,  -0.00587139, -0.00739039,
        -0.00917371, -0.0112332,   -0.0134147,  -0.0152704,  -0.0175308,
        -0.0194987,  -0.021615,    -0.0233439,  -0.0251813,  -0.0278489,
        -0.0296617,  -0.0324331,   -0.0347805,  -0.0362031,  -0.0376821,
        -0.0403375,  -0.042188,    -0.0439056,  -0.045399,   -0.0482114,
        -0.0505533,  -0.0524687,   -0.0556041,  -0.0565102,  -0.0585783,
        -0.0618502,  -0.0639255,   -0.0660007,  -0.068076,   -0.0701513,
        -0.0722265,  -0.0743018,   -0.0763771,  -0.0784523,  -0.0805276,
        -0.0826029,  -0.0846781,   -0.0867534,  -0.0888287,  -0.0909039,
        -0.0929792,  -0.0950545,   -0.0971297,  -0.099205,   -0.10128,
        -0.103356,   -0.105431,    -0.107506,   -0.109581,   -0.111657,
        -0.113732,   -0.115807,    -0.117882,   -0.119958,   -0.122033,
        -0.124108,   -0.126183,    -0.128259,   -0.130334,   -0.132409,
        -0.134485,   -0.13656,     -0.138635,   -0.14071,    -0.142786,
        -0.144861,   -0.146936,    -0.149011,   -0.151087,   -0.153162,
        -0.155237,   -0.157312,    -0.159388,   -0.161463,   -0.163538,
        -0.165614,   -0.167689,    -0.169764,   -0.171839,   -0.173915,
        -0.17599,    -0.178065,    -0.18014,    -0.182216,   -0.184291};
    Utils2::FillTH1F(95, a_whlvbbcorr, m_h_WHlvbbNLOEWKCorrection, m_allHists);
    Float_t a_zhllbbcorr[95] = {
        0.000369691, -0.00311954, -0.00814806, -0.0101356, -0.0133924,
        -0.015624,   -0.0188796,  -0.0212749,  -0.0233653, -0.0254506,
        -0.0270923,  -0.028222,   -0.0284564,  -0.0287463, -0.0294866,
        -0.031416,   -0.0304275,  -0.0317627,  -0.0329827, -0.0322219,
        -0.0339739,  -0.0323855,  -0.0329175,  -0.0336001, -0.033455,
        -0.0350176,  -0.0341059,  -0.0383008,  -0.0375261, -0.0404764,
        -0.0405512,  -0.0410093,  -0.0432318,  -0.0475565, -0.0498079,
        -0.0470666,  -0.0544329,  -0.0534824,  -0.055241,  -0.0582209,
        -0.0604063,  -0.0625916,  -0.064777,   -0.0669624, -0.0691477,
        -0.0713331,  -0.0735185,  -0.0757038,  -0.0778892, -0.0800746,
        -0.0822599,  -0.0844453,  -0.0866307,  -0.088816,  -0.0910014,
        -0.0931868,  -0.0953721,  -0.0975575,  -0.0997429, -0.101928,
        -0.104114,   -0.106299,   -0.108484,   -0.11067,   -0.112855,
        -0.11504,    -0.117226,   -0.119411,   -0.121597,  -0.123782,
        -0.125967,   -0.128153,   -0.130338,   -0.132523,  -0.134709,
        -0.136894,   -0.139079,   -0.141265,   -0.14345,   -0.145636,
        -0.147821,   -0.150006,   -0.152192,   -0.154377,  -0.156562,
        -0.158748,   -0.160933,   -0.163119,   -0.165304,  -0.167489,
        -0.169675,   -0.17186,    -0.174045,   -0.176231,  -0.178416};
    Utils2::FillTH1F(95, a_zhllbbcorr, m_h_ZHllbbNLOEWKCorrection, m_allHists);
    Float_t a_zhvvbbcorr[95] = {
        0.0148102,   0.0137398,  0.0127929,   0.0117756,   0.011091,
        0.0100978,   0.0094288,  0.0083615,   0.00791759,  0.00740715,
        0.00712286,  0.00672918, 0.00662655,  0.00650805,  0.00671416,
        0.00694015,  0.00738593, 0.00741676,  0.00850807,  0.00886493,
        0.010138,    0.011426,   0.0122657,   0.0125201,   0.01283,
        0.0127657,   0.0135671,  0.0126575,   0.0118711,   0.0115731,
        0.0104059,   0.0108386,  0.00905639,  0.00774211,  0.00647519,
        0.00545708,  0.00377601, 0.00251831,  0.00148805,  5.26677e-06,
        -0.00186611, -0.0037375, -0.00560888, -0.00748026, -0.00935164,
        -0.011223,   -0.0130944, -0.0149658,  -0.0168372,  -0.0187085,
        -0.0205799,  -0.0224513, -0.0243227,  -0.0261941,  -0.0280655,
        -0.0299368,  -0.0318082, -0.0336796,  -0.035551,   -0.0374224,
        -0.0392937,  -0.0411651, -0.0430365,  -0.0449079,  -0.0467793,
        -0.0486507,  -0.050522,  -0.0523934,  -0.0542648,  -0.0561362,
        -0.0580076,  -0.0598789, -0.0617503,  -0.0636217,  -0.0654931,
        -0.0673645,  -0.0692359, -0.0711072,  -0.0729786,  -0.07485,
        -0.0767214,  -0.0785928, -0.0804641,  -0.0823355,  -0.0842069,
        -0.0860783,  -0.0879497, -0.0898211,  -0.0916924,  -0.0935638,
        -0.0954352,  -0.0973066, -0.099178,   -0.101049,   -0.102921};
    Utils2::FillTH1F(95, a_zhvvbbcorr, m_h_ZHvvbbNLOEWKCorrection, m_allHists);
  } else if (m_eight) {
    Float_t a_whlvbbcorr[95] = {
        0.00200484,  -9.13928e-05, -0.00219974, -0.00202884, -0.00219318,
        -0.00299564, -0.00336771,  -0.00409143, -0.0059936,  -0.00750034,
        -0.00881597, -0.0111589,   -0.0135328,  -0.0151878,  -0.0178145,
        -0.0189559,  -0.0215716,   -0.0233429,  -0.0250507,  -0.0278028,
        -0.0291708,  -0.0319617,   -0.0341056,  -0.0365647,  -0.0381327,
        -0.0399857,  -0.0405974,   -0.0444237,  -0.0454833,  -0.0482596,
        -0.0486239,  -0.0534461,   -0.0539187,  -0.0559509,  -0.0596689,
        -0.0610085,  -0.0633554,   -0.0639406,  -0.0672306,  -0.0699078,
        -0.0719664,  -0.0740249,   -0.0760835,  -0.078142,   -0.0802005,
        -0.0822591,  -0.0843176,   -0.0863761,  -0.0884347,  -0.0904932,
        -0.0925517,  -0.0946103,   -0.0966688,  -0.0987273,  -0.100786,
        -0.102844,   -0.104903,    -0.106961,   -0.10902,    -0.111079,
        -0.113137,   -0.115196,    -0.117254,   -0.119313,   -0.121371,
        -0.12343,    -0.125488,    -0.127547,   -0.129605,   -0.131664,
        -0.133722,   -0.135781,    -0.13784,    -0.139898,   -0.141957,
        -0.144015,   -0.146074,    -0.148132,   -0.150191,   -0.152249,
        -0.154308,   -0.156366,    -0.158425,   -0.160483,   -0.162542,
        -0.1646,     -0.166659,    -0.168718,   -0.170776,   -0.172835,
        -0.174893,   -0.176952,    -0.17901,    -0.181069,   -0.183127};
    Utils2::FillTH1F(95, a_whlvbbcorr, m_h_WHlvbbNLOEWKCorrection, m_allHists);
    Float_t a_zhllbbcorr[95] = {
        0.000664024, -0.00357095, -0.00767076, -0.00967366, -0.0134844,
        -0.0157148,  -0.0181885,  -0.0209647,  -0.0232788,  -0.0252373,
        -0.0265634,  -0.0275069,  -0.0285776,  -0.0281683,  -0.0294206,
        -0.0299975,  -0.0308047,  -0.0311716,  -0.030913,   -0.0324821,
        -0.0323192,  -0.0324639,  -0.0319356,  -0.0322621,  -0.0331146,
        -0.0338905,  -0.0345189,  -0.0358591,  -0.0358407,  -0.040018,
        -0.0396389,  -0.0407177,  -0.0445103,  -0.0441406,  -0.0471215,
        -0.0463301,  -0.0513777,  -0.0536773,  -0.0546446,  -0.0568508,
        -0.0590333,  -0.0612157,  -0.0633981,  -0.0655805,  -0.067763,
        -0.0699454,  -0.0721278,  -0.0743103,  -0.0764927,  -0.0786751,
        -0.0808575,  -0.08304,    -0.0852224,  -0.0874048,  -0.0895872,
        -0.0917697,  -0.0939521,  -0.0961345,  -0.098317,   -0.100499,
        -0.102682,   -0.104864,   -0.107047,   -0.109229,   -0.111412,
        -0.113594,   -0.115776,   -0.117959,   -0.120141,   -0.122324,
        -0.124506,   -0.126689,   -0.128871,   -0.131053,   -0.133236,
        -0.135418,   -0.137601,   -0.139783,   -0.141965,   -0.144148,
        -0.14633,    -0.148513,   -0.150695,   -0.152878,   -0.15506,
        -0.157242,   -0.159425,   -0.161607,   -0.16379,    -0.165972,
        -0.168155,   -0.170337,   -0.172519,   -0.174702,   -0.176884};
    Utils2::FillTH1F(95, a_zhllbbcorr, m_h_ZHllbbNLOEWKCorrection, m_allHists);
    Float_t a_zhvvbbcorr[95] = {
        0.0146846,  0.0136521,   0.0125801,   0.0117771,   0.010976,
        0.00989665, 0.00929942,  0.00836484,  0.00781992,  0.00733247,
        0.00688885, 0.00666833,  0.0063354,   0.00637412,  0.00662595,
        0.0069015,  0.00716689,  0.00760953,  0.00823267,  0.00914484,
        0.00960494, 0.0110894,   0.0122241,   0.0127155,   0.0126892,
        0.0125873,  0.01278,     0.0128243,   0.0118519,   0.0116125,
        0.0102697,  0.00960959,  0.00929141,  0.00807739,  0.00588976,
        0.00522135, 0.00365527,  0.00214147,  0.000569382, 0.000322672,
        -0.0015679, -0.00345846, -0.00534903, -0.0072396,  -0.00913017,
        -0.0110207, -0.0129113,  -0.0148019,  -0.0166924,  -0.018583,
        -0.0204736, -0.0223641,  -0.0242547,  -0.0261453,  -0.0280358,
        -0.0299264, -0.031817,   -0.0337076,  -0.0355981,  -0.0374887,
        -0.0393793, -0.0412698,  -0.0431604,  -0.045051,   -0.0469415,
        -0.0488321, -0.0507227,  -0.0526132,  -0.0545038,  -0.0563944,
        -0.0582849, -0.0601755,  -0.0620661,  -0.0639566,  -0.0658472,
        -0.0677378, -0.0696283,  -0.0715189,  -0.0734095,  -0.0753001,
        -0.0771906, -0.0790812,  -0.0809718,  -0.0828623,  -0.0847529,
        -0.0866435, -0.088534,   -0.0904246,  -0.0923152,  -0.0942057,
        -0.0960963, -0.0979869,  -0.0998774,  -0.101768,   -0.103659};
    Utils2::FillTH1F(95, a_zhvvbbcorr, m_h_ZHvvbbNLOEWKCorrection, m_allHists);
  } else if (m_thirteen) {
    Float_t a_whlvbbcorr[95] = {
        0.0305203,   0.0275837,   0.0239527,   0.0215937,  0.0170362,
        0.015103,    0.011289,    0.00953566,  0.00753487, 0.00497013,
        0.00211544,  -0.00118702, -0.00458593, -0.0044137, -0.00812502,
        -0.00802924, -0.0111074,  -0.0111592,  -0.0156807, -0.0176686,
        -0.0203234,  -0.0207377,  -0.0198427,  -0.0272455, -0.0255043,
        -0.0314772,  -0.0332882,  -0.0279931,  -0.0353625, -0.0363241,
        -0.0380089,  -0.038475,   -0.0418936,  -0.0490911, -0.0491424,
        -0.047753,   -0.055085,   -0.0538532,  -0.0635846, -0.0489332,
        -0.0591391,  -0.0654692,  -0.0667212,  -0.0739219, -0.0660949,
        -0.068587,   -0.0652828,  -0.0752986,  -0.0768462, -0.0739386,
        -0.088597,   -0.0880649,  -0.0800617,  -0.0899523, -0.081103,
        -0.0913021,  -0.0898038,  -0.102608,   -0.0973754, -0.101796,
        -0.0866853,  -0.112784,   -0.0972057,  -0.12284,   -0.143297,
        -0.126077,   -0.0955431,  -0.131121,   -0.120541,  -0.116152,
        -0.125354,   -0.12976,    -0.135101,   -0.12671,   -0.1366,
        -0.132531,   -0.136987,   -0.159221,   -0.149076,  -0.162372,
        -0.134163,   -0.152301,   -0.154554,   -0.16334,   -0.174772,
        -0.140081,   -0.139006,   -0.186223,   -0.139392,  -0.164362,
        -0.150565,   -0.172819,   -0.163485,   -0.182222,  -0.161225};
    Utils2::FillTH1F(95, a_whlvbbcorr, m_h_WHlvbbNLOEWKCorrection, m_allHists);
    Float_t a_zhllbbcorr[95] = {
        0.00575184,  0.00520017,  0.000907714, -0.000683623, -6.06938e-05,
        -0.00603715, -0.00820789, -0.00715682, -0.0109913,   -0.0116415,
        -0.0151916,  -0.0147331,  -0.0173633,  -0.0193058,   -0.0147234,
        -0.0200263,  -0.0222203,  -0.0202423,  -0.0147083,   -0.0202295,
        -0.0263968,  -0.0207094,  -0.0214692,  0.0329771,    -0.0710156,
        -0.0192676,  -0.0170452,  -0.0237475,  -0.0237,      -0.0288374,
        -0.0136505,  -0.0315127,  -0.0226618,  -0.0321998,   -0.0351517,
        -0.0264691,  -0.0370783,  -0.0436323,  -0.0377595,   -0.0335254,
        -0.0474162,  -0.046111,   -0.0515759,  -0.0408445,   -0.0534198,
        -0.0497336,  -0.0723432,  -0.0510952,  -0.0578118,   -0.0804619,
        -0.0574166,  -0.0599096,  -0.0614737,  -0.0833518,   -0.0679638,
        -0.101124,   -0.0422995,  -0.0950677,  -0.0668532,   -0.0867806,
        -0.0934843,  -0.10063,    -0.077255,   -0.103045,    -0.0657193,
        -0.125334,   -0.0972838,  -0.126751,   -0.0814412,   -0.112221,
        -0.153133,   -0.0717616,  -0.124109,   -0.12552,     -0.104948,
        -0.134106,   -0.120987,   -0.107912,   -0.174016,    -0.0926666,
        -0.153212,   -0.102483,   -0.172823,   -0.0660544,   -0.22406,
        -0.154227,   -0.117794,   -0.124252,   -0.179716,    -0.156098,
        -0.151035,   -0.155933,   -0.162196,   -0.123895,    -0.175934};
    Utils2::FillTH1F(95, a_zhllbbcorr, m_h_ZHllbbNLOEWKCorrection, m_allHists);
    Float_t a_zhvvbbcorr[95] = {
        0.0459368,    0.0444353,  0.0428537,  0.0433563,   0.0409541,
        0.0407437,    0.0407105,  0.0399589,  0.0390888,   0.0395181,
        0.0381305,    0.0376067,  0.0375164,  0.0381493,   0.0381599,
        0.038808,     0.0383673,  0.0387298,  0.039357,    0.0405333,
        0.0413119,    0.0434166,  0.0440145,  0.0432051,   0.0432172,
        0.0438688,    0.0442526,  0.0430004,  0.0413185,   0.0438529,
        0.0405151,    0.0394676,  0.0376205,  0.0372373,   0.0353171,
        0.0364613,    0.0332694,  0.0311073,  0.0318161,   0.0263172,
        0.0258451,    0.0251923,  0.0236341,  0.0231261,   0.0215979,
        0.0196104,    0.0172485,  0.0158832,  0.0104093,   0.00659796,
        0.00926259,   0.00815121, 0.00591785, 0.00375429,  0.00287649,
        -0.000966954, -0.0119222, 0.00247599, -0.00919346, -0.0117039,
        -0.0107326,   -0.0140965, -0.0239023, -0.0143103,  -0.0216578,
        -0.023237,    -0.0224681, -0.031081,  -0.0324773,  -0.028629,
        -0.0306012,   -0.0337712, -0.0358671, -0.0509928,  -0.040386,
        -0.0484243,   -0.0469028, -0.0568839, -0.0484311,  -0.0603877,
        -0.0565608,   -0.0534944, -0.0514455, -0.0551197,  -0.0540438,
        -0.0557179,   -0.0696116, -0.066831,  -0.0611314,  -0.06796,
        -0.0899669,   -0.0763285, -0.0772027, -0.0747105,  -0.0911288};
    Utils2::FillTH1F(95, a_zhvvbbcorr, m_h_ZHvvbbNLOEWKCorrection, m_allHists);
  }

  //** added by Lei: 2014jan31
  Float_t VH_pTbins[14] = {0e3,   25e3,  50e3,  75e3,  100e3, 125e3, 150e3,
                           175e3, 200e3, 225e3, 250e3, 300e3, 400e3, 500e3};
  m_h_ggZllH110_2JetCorrection =
      new TH1F("m_h_ggZllH110_2JetCorrection", "m_h_ggZllH110_2JetCorrection",
               13, VH_pTbins);
  m_h_ggZllH110_3JetCorrection =
      new TH1F("m_h_ggZllH110_3JetCorrection", "m_h_ggZllH110_3JetCorrection",
               13, VH_pTbins);
  m_h_ggZllH125_2JetCorrection =
      new TH1F("m_h_ggZllH125_2JetCorrection", "m_h_ggZllH125_2JetCorrection",
               13, VH_pTbins);
  m_h_ggZllH125_3JetCorrection =
      new TH1F("m_h_ggZllH125_3JetCorrection", "m_h_ggZllH125_3JetCorrection",
               13, VH_pTbins);
  m_h_ggZllH140_2JetCorrection =
      new TH1F("m_h_ggZllH140_2JetCorrection", "m_h_ggZllH140_2JetCorrection",
               13, VH_pTbins);
  m_h_ggZllH140_3JetCorrection =
      new TH1F("m_h_ggZllH140_3JetCorrection", "m_h_ggZllH140_3JetCorrection",
               13, VH_pTbins);

  m_h_ggZvvH110_2JetCorrection =
      new TH1F("m_h_ggZvvH110_2JetCorrection", "m_h_ggZvvH110_2JetCorrection",
               13, VH_pTbins);
  m_h_ggZvvH110_3JetCorrection =
      new TH1F("m_h_ggZvvH110_3JetCorrection", "m_h_ggZvvH110_3JetCorrection",
               13, VH_pTbins);
  m_h_ggZvvH125_2JetCorrection =
      new TH1F("m_h_ggZvvH125_2JetCorrection", "m_h_ggZvvH125_2JetCorrection",
               13, VH_pTbins);
  m_h_ggZvvH125_3JetCorrection =
      new TH1F("m_h_ggZvvH125_3JetCorrection", "m_h_ggZvvH125_3JetCorrection",
               13, VH_pTbins);
  m_h_ggZvvH140_2JetCorrection =
      new TH1F("m_h_ggZvvH140_2JetCorrection", "m_h_ggZvvH140_2JetCorrection",
               13, VH_pTbins);
  m_h_ggZvvH140_3JetCorrection =
      new TH1F("m_h_ggZvvH140_3JetCorrection", "m_h_ggZvvH140_3JetCorrection",
               13, VH_pTbins);

  m_h_ggZllH110_2JetCorrection->SetDirectory(0);
  m_h_ggZllH110_3JetCorrection->SetDirectory(0);
  m_h_ggZllH125_2JetCorrection->SetDirectory(0);
  m_h_ggZllH125_3JetCorrection->SetDirectory(0);
  m_h_ggZllH140_2JetCorrection->SetDirectory(0);
  m_h_ggZllH140_3JetCorrection->SetDirectory(0);

  m_h_ggZvvH110_2JetCorrection->SetDirectory(0);
  m_h_ggZvvH110_3JetCorrection->SetDirectory(0);
  m_h_ggZvvH125_2JetCorrection->SetDirectory(0);
  m_h_ggZvvH125_3JetCorrection->SetDirectory(0);
  m_h_ggZvvH140_2JetCorrection->SetDirectory(0);
  m_h_ggZvvH140_3JetCorrection->SetDirectory(0);

  Float_t a_ggzllh125corr2jet[13] = {
      1.00937, 1.01239, 1.02167, 1.04174, 1.0822,  1.14372, 1.14082,
      1.12698, 1.10225, 1.09086, 1.07102, 1.04184, 1.01108};
  Float_t a_ggzllh110corr2jet[13] = {
      1.0062,  1.00901, 1.01672, 1.03341, 1.06887, 1.13024, 1.15496,
      1.13601, 1.11276, 1.09777, 1.07149, 1.04459, 1.01823};
  Float_t a_ggzllh140corr2jet[13] = {
      1.01264, 1.01694, 1.02708, 1.04985, 1.0955,  1.14008, 1.1323,
      1.11454, 1.09703, 1.08029, 1.0619,  1.03748, 1.01982};
  Float_t a_ggzllh125corr3jet[13] = {
      1.02148, 1.02691, 1.04087, 1.07739, 1.14178, 1.22826, 1.24053,
      1.21577, 1.1894,  1.15865, 1.12002, 1.08653, 1.04982};
  Float_t a_ggzllh110corr3jet[13] = {
      1.0117,  1.01887, 1.03411, 1.06549, 1.12942, 1.21591, 1.25322,
      1.23969, 1.20605, 1.17112, 1.14841, 1.10395, 1.06018};
  Float_t a_ggzllh140corr3jet[13] = {
      1.02273, 1.03184, 1.05061, 1.08861, 1.16851, 1.21998, 1.2234,
      1.19279, 1.15547, 1.1408,  1.11259, 1.08314, 1.04514};

  Float_t a_ggzvvh125corr2jet[13] = {
      1,       1,       1,       1.06354, 1.07639, 1.12891, 1.13274,
      1.11608, 1.09783, 1.08246, 1.06971, 1.04079, 1.01996};
  Float_t a_ggzvvh110corr2jet[13] = {1,       1,       1,       1.05515, 1.066,
                                     1.11858, 1.14303, 1.12453, 1.10761, 1.0873,
                                     1.07299, 1.04276, 1.02137};
  Float_t a_ggzvvh140corr2jet[13] = {1,       1,       1,      1.0812,  1.09146,
                                     1.12811, 1.12335, 1.1067, 1.09037, 1.07571,
                                     1.06103, 1.03559, 1.01908};
  Float_t a_ggzvvh125corr3jet[13] = {
      1,       1,       1,       1.1033,  1.0953,  1.17007, 1.20049,
      1.18641, 1.18625, 1.13567, 1.12212, 1.08515, 1.03995};
  Float_t a_ggzvvh110corr3jet[13] = {
      1,       1,       1,      1.06022, 1.08618, 1.15969, 1.21299,
      1.21593, 1.18675, 1.1729, 1.14232, 1.09702, 1.0631};
  Float_t a_ggzvvh140corr3jet[13] = {
      1,       1,       1,       1.09432, 1.11244, 1.17889, 1.18199,
      1.16915, 1.14596, 1.12407, 1.1181,  1.08153, 1.04593};

  Utils2::ArraySubstractOne(a_ggzllh125corr2jet, 13);
  Utils2::ArraySubstractOne(a_ggzllh110corr2jet, 13);
  Utils2::ArraySubstractOne(a_ggzllh140corr2jet, 13);
  Utils2::ArraySubstractOne(a_ggzllh125corr3jet, 13);
  Utils2::ArraySubstractOne(a_ggzllh110corr3jet, 13);
  Utils2::ArraySubstractOne(a_ggzllh140corr3jet, 13);

  Utils2::ArraySubstractOne(a_ggzvvh125corr2jet, 13);
  Utils2::ArraySubstractOne(a_ggzvvh110corr2jet, 13);
  Utils2::ArraySubstractOne(a_ggzvvh140corr2jet, 13);
  Utils2::ArraySubstractOne(a_ggzvvh125corr3jet, 13);
  Utils2::ArraySubstractOne(a_ggzvvh110corr3jet, 13);
  Utils2::ArraySubstractOne(a_ggzvvh140corr3jet, 13);

  Utils2::FillTH1F(13, a_ggzllh125corr2jet, m_h_ggZllH125_2JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzllh110corr2jet, m_h_ggZllH110_2JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzllh140corr2jet, m_h_ggZllH140_2JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzllh125corr3jet, m_h_ggZllH125_3JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzllh110corr3jet, m_h_ggZllH110_3JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzllh140corr3jet, m_h_ggZllH140_3JetCorrection,
                   m_allHists);

  Utils2::FillTH1F(13, a_ggzvvh125corr2jet, m_h_ggZvvH125_2JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzvvh110corr2jet, m_h_ggZvvH110_2JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzvvh140corr2jet, m_h_ggZvvH140_2JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzvvh125corr3jet, m_h_ggZvvH125_3JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzvvh110corr3jet, m_h_ggZvvH110_3JetCorrection,
                   m_allHists);
  Utils2::FillTH1F(13, a_ggzvvh140corr3jet, m_h_ggZvvH140_3JetCorrection,
                   m_allHists);

  // VpT bins
  m_h_pTbins = new TH1F("pTbins", "pTbins", 5, pTbins);
  m_h_pTbins->SetDirectory(0);

  // top pT correction. Yuji 18/05/13
  // From https://cds.cern.ch/record/1470588/ Figure 46
  // update on 27/05/13 for error to be 50% of the correction

  Float_t topPtCorr[7] = {0.05128,  0.0288,    -0.000898004, -0.024253,
                          -0.08489, -0.170377, -0.247117};
  Float_t topPtCorrBins[8] = {0,      50.e3,  100.e3, 150.e3,
                              200.e3, 250.e3, 350.e3, 800.e3};

  /*
     float normTopPt = 100000./99833.4;
     for( int i=0; i<7; i++) {
     topPtCorr[i] *= normTopPt;
     }
     */

  m_h_topPtCorrection =
      new TH1F("topPtCorrection", "topPtCorrection", 7, topPtCorrBins);
  m_h_topPtCorrection->SetDirectory(0);
  Utils2::FillTH1F(7, topPtCorr, m_h_topPtCorrection, m_allHists);

  // DeltaR correction for truth-tagging. Applies to Wcc/Zcc
  m_f_dRjj = new TF1("f_dRjj", "x<2.7 ? [0]+[1]*log(x)+[2]*log(x)*log(x) : 0",
                     0.4, 5.0);
  m_f_dRjj->SetParameter(0, 0.9509 - 1);
  m_f_dRjj->SetParameter(1, 0.418);
  m_f_dRjj->SetParameter(2, -0.3697);

  m_v_ZNJet0TagSherpaCorr[0] = 0;
  m_v_ZNJet0TagSherpaCorr[1] = 0;
  m_v_ZNJet0TagSherpaCorr[2] = 0.08;
  m_v_ZNJet0TagSherpaCorr[3] = -0.05;
  m_v_ZNJet0TagSherpaCorr[4] = -0.18;

  m_v_ZNJet1TagSherpaCorr[0] = 0;
  m_v_ZNJet1TagSherpaCorr[1] = 0;
  m_v_ZNJet1TagSherpaCorr[2] = 0.09;
  m_v_ZNJet1TagSherpaCorr[3] = -0.04;
  m_v_ZNJet1TagSherpaCorr[4] = -0.12;

  m_v_ZNJet2TagSherpaCorr[0] = 0;
  m_v_ZNJet2TagSherpaCorr[1] = 0;
  m_v_ZNJet2TagSherpaCorr[2] = 0.02;
  m_v_ZNJet2TagSherpaCorr[3] = -0.02;
  m_v_ZNJet2TagSherpaCorr[4] = 0;

  m_v_ZNJet0TagMadgraphCorr[0] = 0;
  m_v_ZNJet0TagMadgraphCorr[1] = 0;
  m_v_ZNJet0TagMadgraphCorr[2] = -0.02;
  m_v_ZNJet0TagMadgraphCorr[3] = 0.02;
  m_v_ZNJet0TagMadgraphCorr[4] = 0.03;

  m_v_ZNJet1TagMadgraphCorr[0] = 0;
  m_v_ZNJet1TagMadgraphCorr[1] = 0;
  m_v_ZNJet1TagMadgraphCorr[2] = -0.08;
  m_v_ZNJet1TagMadgraphCorr[3] = 0.04;
  m_v_ZNJet1TagMadgraphCorr[4] = 0.15;

  m_v_ZNJet2TagMadgraphCorr[0] = 0;
  m_v_ZNJet2TagMadgraphCorr[1] = 0;
  m_v_ZNJet2TagMadgraphCorr[2] = -0.18;
  m_v_ZNJet2TagMadgraphCorr[3] = 0.04;
  m_v_ZNJet2TagMadgraphCorr[4] = 0.25;

  /*********************************************
   *
   *    SYSTEMATICS
   *    These should be applied to the nominal
   *
   *    Below the histograms which contain the
   *    values of the correction are created
   *
   *********************************************/

  m_h_SysTheoryWHlvbbPt =
      new TH1F("SysTheoryWHlvbbPt", "SysTheoryWHlvbbPt", 95, 25.e3, 500.e3);
  m_h_SysTheoryWHlvbbPt->SetDirectory(0);
  m_h_SysTheoryZHllbbPt =
      new TH1F("SysTheoryZHllbbPt", "SysTheoryZHllbbPt", 95, 25.e3, 500.e3);
  m_h_SysTheoryZHllbbPt->SetDirectory(0);
  m_h_SysTheoryZHvvbbPt =
      new TH1F("SysTheoryZHvvbbPt", "SysTheoryZHvvbbPt", 95, 25.e3, 500.e3);
  m_h_SysTheoryZHvvbbPt->SetDirectory(0);
  if (m_seven) {
    Float_t hw7_errors[95] = {
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.0201801, 0.0207736, 0.0213758,
        0.0219865, 0.0226059, 0.0232338, 0.0238704, 0.0245155, 0.0251693,
        0.0258317, 0.0265027, 0.0271822, 0.0278704, 0.0285672, 0.0292726,
        0.0299866, 0.0307092, 0.0314403, 0.0321801, 0.0329285, 0.0336855,
        0.0344511, 0.0352254, 0.0360082, 0.0367996, 0.0375996, 0.0384082,
        0.0392254, 0.0400513, 0.0408857, 0.0417287, 0.0425803, 0.0434406,
        0.0443094, 0.0451869, 0.0460729, 0.0469675, 0.0478708, 0.0487827,
        0.0497031, 0.0506322, 0.0515698, 0.0525161, 0.053471,  0.0544344,
        0.0554065, 0.0563872, 0.0573764, 0.0583743, 0.0593808, 0.0603959,
        0.0614196, 0.0624519, 0.0634928, 0.0645423, 0.0656004};
    Utils2::FillTH1F(95, hw7_errors, m_h_SysTheoryWHlvbbPt, m_allHists);
    Float_t hll7_errors[95] = {
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.0203477,
        0.0209749, 0.0216116, 0.0222579, 0.0229137, 0.023579,  0.0242538,
        0.0249381, 0.025632,  0.0263354, 0.0270483, 0.0277707, 0.0285026,
        0.0292441, 0.0299951, 0.0307556, 0.0315256, 0.0323051, 0.0330942,
        0.0338928, 0.0347009, 0.0355185, 0.0363457, 0.0371823, 0.0380285,
        0.0388842, 0.0397495, 0.0406242, 0.0415085, 0.0424023, 0.0433056,
        0.0442184, 0.0451408, 0.0460727, 0.0470141, 0.047965,  0.0489254,
        0.0498954, 0.0508749, 0.0518639, 0.0528624, 0.0538704};
    Utils2::FillTH1F(95, hll7_errors, m_h_SysTheoryZHllbbPt, m_allHists);
    Float_t hnn7_errors[95] = {
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,     0.02,      0.0202857, 0.0208312, 0.0213839, 0.0219438,
        0.022511, 0.0230854, 0.023667,  0.0242559, 0.024852};
    Utils2::FillTH1F(95, hnn7_errors, m_h_SysTheoryZHvvbbPt, m_allHists);
  } else if (m_eight) {
    Float_t hw8_errors[95] = {
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.0201193, 0.0207066, 0.0213024,
        0.0219067, 0.0225194, 0.0231405, 0.0237701, 0.0244082, 0.0250547,
        0.0257096, 0.026373,  0.0270449, 0.0277252, 0.0284139, 0.0291111,
        0.0298168, 0.0305309, 0.0312534, 0.0319844, 0.0327239, 0.0334718,
        0.0342281, 0.0349929, 0.0357662, 0.0365479, 0.037338,  0.0381367,
        0.0389437, 0.0397592, 0.0405832, 0.0414156, 0.0422564, 0.0431057,
        0.0439635, 0.0448297, 0.0457044, 0.0465875, 0.047479,  0.048379,
        0.0492875, 0.0502044, 0.0511298, 0.0520636, 0.0530058, 0.0539565,
        0.0549157, 0.0558833, 0.0568594, 0.0578439, 0.0588369, 0.0598383,
        0.0608481, 0.0618664, 0.0628932, 0.0639284, 0.0649721};
    Utils2::FillTH1F(95, hw8_errors, m_h_SysTheoryWHlvbbPt, m_allHists);
    Float_t hll8_errors[95] = {
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.0205646, 0.0211945, 0.021834,  0.022483,  0.0231415, 0.0238095,
        0.024487,  0.025174,  0.0258705, 0.0265765, 0.027292,  0.028017,
        0.0287516, 0.0294956, 0.0302491, 0.0310122, 0.0317847, 0.0325667,
        0.0333583, 0.0341594, 0.0349699, 0.03579,   0.0366195, 0.0374586,
        0.0383072, 0.0391653, 0.0400329, 0.0409099, 0.0417965, 0.0426926,
        0.0435982, 0.0445134, 0.045438,  0.0463721, 0.0473157, 0.0482688,
        0.0492315, 0.0502036, 0.0511852, 0.0521764, 0.053177};
    Utils2::FillTH1F(95, hll8_errors, m_h_SysTheoryZHllbbPt, m_allHists);
    Float_t hnn8_errors[95] = {
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.02,      0.02,      0.02,      0.02,
        0.02,      0.02,      0.0202975, 0.0208415, 0.0213928, 0.0219513,
        0.0225169, 0.0230898, 0.0236698, 0.0242571, 0.0248515};
    Utils2::FillTH1F(95, hnn8_errors, m_h_SysTheoryZHvvbbPt, m_allHists);
  } else if (m_thirteen) {
    Float_t hw13_errors[95] = {
        0.0484571, 0.0482641, 0.047791,  0.0468311, 0.0465664, 0.046179,
        0.0454778, 0.0448147, 0.044418,  0.0440651, 0.0432921, 0.0431831,
        0.0421244, 0.0419228, 0.0413434, 0.0406149, 0.0407655, 0.0404188,
        0.039901,  0.039533,  0.0394072, 0.0386198, 0.0387668, 0.0390588,
        0.0390053, 0.0384565, 0.0389162, 0.0382972, 0.0379813, 0.0379627,
        0.0372697, 0.0378582, 0.0378323, 0.0380233, 0.0378552, 0.0376143,
        0.0374118, 0.0376463, 0.0375152, 0.0373611, 0.0377874, 0.0376548,
        0.0379455, 0.0378946, 0.0375971, 0.0380454, 0.037055,  0.0378499,
        0.0375839, 0.0377017, 0.0368133, 0.038143,  0.0382894, 0.0384925,
        0.0378546, 0.0382847, 0.0392715, 0.0373284, 0.0382505, 0.0376119,
        0.0385875, 0.0379694, 0.0388461, 0.0384448, 0.0448735, 0.0388174,
        0.0387952, 0.0402528, 0.0396275, 0.0397726, 0.0390142, 0.0397522,
        0.0417357, 0.0395624, 0.0423012, 0.0407749, 0.0424476, 0.0512948,
        0.0471539, 0.0526161, 0.0413839, 0.0484515, 0.0493681, 0.0530257,
        0.0579799, 0.0436284, 0.0438099, 0.0631644, 0.0442861, 0.0534594,
        0.0477507, 0.057118,  0.0530871, 0.0613276, 0.0521333};
    Utils2::FillTH1F(95, hw13_errors, m_h_SysTheoryWHlvbbPt, m_allHists);
    Float_t hll13_errors[95] = {
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.0100887, 0.01,
        0.01,      0.01,      0.0108754, 0.01,      0.01,      0.01,
        0.0211221, 0.01,      0.01,      0.010373,  0.010364,  0.0113487,
        0.01,      0.0118792, 0.0101705, 0.0120174, 0.0126202, 0.0108892,
        0.0130216, 0.014434,  0.013165,  0.0122863, 0.0152826, 0.0149872,
        0.0162435, 0.0138243, 0.0166787, 0.0158143, 0.0214786, 0.0161309,
        0.0177387, 0.0237237, 0.017642,  0.0182566, 0.0186475, 0.0245498,
        0.0203139, 0.0299408, 0.0141409, 0.0280436, 0.0200236, 0.0255482,
        0.0275578, 0.0297836, 0.0228235, 0.0305557, 0.0197295, 0.0381448,
        0.0287306, 0.0386558, 0.024002,  0.033578,  0.048789,  0.021322,
        0.0377059, 0.0382118, 0.0311705, 0.0413624, 0.0365988, 0.0321411,
        0.0576454, 0.0273086, 0.0488211, 0.0303752, 0.0571196, 0.0198162,
        0.0818734, 0.0492347, 0.0354835, 0.0377572, 0.0601913, 0.0500017,
        0.0479402, 0.0499337, 0.0525421, 0.0376296, 0.0584959};
    Utils2::FillTH1F(95, hll13_errors, m_h_SysTheoryZHllbbPt, m_allHists);
    Float_t hnn13_errors[95] = {
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.01,      0.01,      0.01,      0.01,
        0.01,      0.01,      0.010402,  0.01,      0.01,      0.0102775,
        0.0101346, 0.0117928, 0.0120734, 0.0113079, 0.0116971, 0.0123364,
        0.0127685, 0.016107,  0.0137253, 0.0155128, 0.0151661, 0.0175121,
        0.0155143, 0.0183756, 0.0174335, 0.0166965, 0.0162129, 0.0170851,
        0.0168274, 0.0172293, 0.0207483, 0.0200179, 0.0185616, 0.0203129,
        0.0264939, 0.0225667, 0.022809,  0.0221217, 0.0268431};
    Utils2::FillTH1F(95, hnn13_errors, m_h_SysTheoryZHvvbbPt, m_allHists);
  }

  m_h_SysHerwigPt = new TH1F("SysHerwigPt", "SysHerwigPt", 5, pTbins);

  m_h_SysWW2JetInclScalePt =
      new TH1F("SysWW2JetInclScalePt", "SysWW2JetInclScalePt", 5, pTbins);
  m_h_SysWW2JetScalePt =
      new TH1F("SysWW2JetScalePt", "SysWW2JetScalePt", 5, pTbins);
  m_h_SysWW3JetScalePt =
      new TH1F("SysWW3JetScalePt", "SysWW3JetScalePt", 5, pTbins);
  m_h_SysWW3JetFor2JetScalePt =
      new TH1F("SysWW3JetFor2JetScalePt", "SysWW3JetFor2JetScalePt", 5, pTbins);
  m_h_SysWlnuZhad2JetInclScalePt = new TH1F(
      "SysWlnuZhad2JetInclScalePt", "SysWlnuZhad2JetInclScalePt", 5, pTbins);
  m_h_SysWlnuZhad2JetScalePt =
      new TH1F("SysWlnuZhad2JetScalePt", "SysWlnuZhad2JetScalePt", 5, pTbins);
  m_h_SysWlnuZhad3JetScalePt =
      new TH1F("SysWlnuZhad3JetScalePt", "SysWlnuZhad3JetScalePt", 5, pTbins);
  m_h_SysWlnuZhad3JetFor2JetScalePt =
      new TH1F("SysWlnuZhad3JetFor2JetScalePt", "SysWlnuZhad3JetFor2JetScalePt",
               5, pTbins);
  m_h_SysWhadZnunu2JetInclScalePt = new TH1F(
      "SysWhadZnunu2JetInclScalePt", "SysWhadZnunu2JetInclScalePt", 5, pTbins);
  m_h_SysWhadZnunu2JetScalePt =
      new TH1F("SysWhadZnunu2JetScalePt", "SysWhadZnunu2JetScalePt", 5, pTbins);
  m_h_SysWhadZnunu3JetScalePt =
      new TH1F("SysWhadZnunu3JetScalePt", "SysWhadZnunu3JetScalePt", 5, pTbins);
  m_h_SysWhadZnunu3JetFor2JetScalePt =
      new TH1F("SysWhadZnunu3JetFor2JetScalePt",
               "SysWhadZnunu3JetFor2JetScalePt", 5, pTbins);
  m_h_SysWhadZll2JetInclScalePt = new TH1F(
      "SysWhadZll2JetInclScalePt", "SysWhadZll2JetInclScalePt", 5, pTbins);
  m_h_SysWhadZll2JetScalePt =
      new TH1F("SysWhadZll2JetScalePt", "SysWhadZll2JetScalePt", 5, pTbins);
  m_h_SysWhadZll3JetScalePt =
      new TH1F("SysWhadZll3JetScalePt", "SysWhadZll3JetScalePt", 5, pTbins);
  m_h_SysWhadZll3JetFor2JetScalePt =
      new TH1F("SysWhadZll3JetFor2JetScalePt", "SysWhadZll3JetFor2JetScalePt",
               5, pTbins);
  m_h_SysZllZhad2JetInclScalePt = new TH1F(
      "SysZllZhad2JetInclScalePt", "SysZllZhad2JetInclScalePt", 5, pTbins);
  m_h_SysZllZhad2JetScalePt =
      new TH1F("SysZllZhad2JetScalePt", "SysZllZhad2JetScalePt", 5, pTbins);
  m_h_SysZllZhad3JetScalePt =
      new TH1F("SysZllZhad3JetScalePt", "SysZllZhad3JetScalePt", 5, pTbins);
  m_h_SysZllZhad3JetFor2JetScalePt =
      new TH1F("SysZllZhad3JetFor2JetScalePt", "SysZllZhad3JetFor2JetScalePt",
               5, pTbins);
  m_h_SysZnunuZhad2JetInclScalePt = new TH1F(
      "SysZnunuZhad2JetInclScalePt", "SysZnunuZhad2JetInclScalePt", 5, pTbins);
  m_h_SysZnunuZhad2JetScalePt =
      new TH1F("SysZnunuZhad2JetScalePt", "SysZnunuZhad2JetScalePt", 5, pTbins);
  m_h_SysZnunuZhad3JetScalePt =
      new TH1F("SysZnunuZhad3JetScalePt", "SysZnunuZhad3JetScalePt", 5, pTbins);
  m_h_SysZnunuZhad3JetFor2JetScalePt =
      new TH1F("SysZnunuZhad3JetFor2JetScalePt",
               "SysZnunuZhad3JetFor2JetScalePt", 5, pTbins);

  m_h_SysWW2JetPDFAlphaPt =
      new TH1F("SysWW2JetPDFAlphaPt", "SysWW2JetPDFAlphaPt", 5, pTbins);
  m_h_SysWW3JetPDFAlphaPt =
      new TH1F("SysWW3JetPDFAlphaPt", "SysWW3JetPDFAlphaPt", 5, pTbins);
  m_h_SysWlnuZhad2JetPDFAlphaPt = new TH1F(
      "SysWlnuZhad2JetPDFAlphaPt", "SysWlnuZhad2JetPDFAlphaPt", 5, pTbins);
  m_h_SysWlnuZhad3JetPDFAlphaPt = new TH1F(
      "SysWlnuZhad3JetPDFAlphaPt", "SysWlnuZhad3JetPDFAlphaPt", 5, pTbins);
  m_h_SysWhadZnunu2JetPDFAlphaPt = new TH1F(
      "SysWhadZnunu2JetPDFAlphaPt", "SysWhadZnunu2JetPDFAlphaPt", 5, pTbins);
  m_h_SysWhadZnunu3JetPDFAlphaPt = new TH1F(
      "SysWhadZnunu3JetPDFAlphaPt", "SysWhadZnunu3JetPDFAlphaPt", 5, pTbins);
  m_h_SysWhadZll2JetPDFAlphaPt = new TH1F(
      "SysWhadZll2JetPDFAlphaPt", "SysWhadZll2JetPDFAlphaPt", 5, pTbins);
  m_h_SysWhadZll3JetPDFAlphaPt = new TH1F(
      "SysWhadZll3JetPDFAlphaPt", "SysWhadZll3JetPDFAlphaPt", 5, pTbins);
  m_h_SysZllZhad2JetPDFAlphaPt = new TH1F(
      "SysZllZhad2JetPDFAlphaPt", "SysZllZhad2JetPDFAlphaPt", 5, pTbins);
  m_h_SysZllZhad3JetPDFAlphaPt = new TH1F(
      "SysZllZhad3JetPDFAlphaPt", "SysZllZhad3JetPDFAlphaPt", 5, pTbins);
  m_h_SysZnunuZhad2JetPDFAlphaPt = new TH1F(
      "SysZnunuZhad2JetPDFAlphaPt", "SysZnunuZhad2JetPDFAlphaPt", 5, pTbins);
  m_h_SysZnunuZhad3JetPDFAlphaPt = new TH1F(
      "SysZnunuZhad3JetPDFAlphaPt", "SysZnunuZhad3JetPDFAlphaPt", 5, pTbins);

  m_h_SysLepVeto2JetTop =
      new TH1F("SysLepVeto2JetTop", "SysLepVeto2JetTop", 5, pTbins);
  m_h_SysLepVeto2JetStop =
      new TH1F("SysLepVeto2JetStop", "SysLepVeto2JetStop", 5, pTbins);
  m_h_SysLepVeto2JetWbb =
      new TH1F("SysLepVeto2JetWbb", "SysLepVeto2JetWbb", 5, pTbins);
  m_h_SysLepVeto2JetWW =
      new TH1F("SysLepVeto2JetWW", "SysLepVeto2JetWW", 5, pTbins);
  m_h_SysLepVeto2JetWZ =
      new TH1F("SysLepVeto2JetWZ", "SysLepVeto2JetWZ", 5, pTbins);
  m_h_SysLepVeto3JetTop =
      new TH1F("SysLepVeto3JetTop", "SysLepVeto3JetTop", 5, pTbins);
  m_h_SysLepVeto3JetStop =
      new TH1F("SysLepVeto3JetStop", "SysLepVeto3JetStop", 5, pTbins);
  m_h_SysLepVeto3JetWbb =
      new TH1F("SysLepVeto3JetWbb", "SysLepVeto3JetWbb", 5, pTbins);
  m_h_SysLepVeto3JetWW =
      new TH1F("SysLepVeto3JetWW", "SysLepVeto3JetWW", 5, pTbins);
  m_h_SysLepVeto3JetWZ =
      new TH1F("SysLepVeto3JetWZ", "SysLepVeto3JetWZ", 5, pTbins);

  m_h_SysHerwigPt->SetDirectory(0);

  m_h_SysWW2JetInclScalePt->SetDirectory(0);
  m_h_SysWW2JetScalePt->SetDirectory(0);
  m_h_SysWW3JetScalePt->SetDirectory(0);
  m_h_SysWW3JetFor2JetScalePt->SetDirectory(0);
  m_h_SysWlnuZhad2JetInclScalePt->SetDirectory(0);
  m_h_SysWlnuZhad2JetScalePt->SetDirectory(0);
  m_h_SysWlnuZhad3JetScalePt->SetDirectory(0);
  m_h_SysWlnuZhad3JetFor2JetScalePt->SetDirectory(0);
  m_h_SysWhadZnunu2JetInclScalePt->SetDirectory(0);
  m_h_SysWhadZnunu2JetScalePt->SetDirectory(0);
  m_h_SysWhadZnunu3JetScalePt->SetDirectory(0);
  m_h_SysWhadZnunu3JetFor2JetScalePt->SetDirectory(0);
  m_h_SysWhadZll2JetInclScalePt->SetDirectory(0);
  m_h_SysWhadZll2JetScalePt->SetDirectory(0);
  m_h_SysWhadZll3JetScalePt->SetDirectory(0);
  m_h_SysWhadZll3JetFor2JetScalePt->SetDirectory(0);
  m_h_SysZllZhad2JetInclScalePt->SetDirectory(0);
  m_h_SysZllZhad2JetScalePt->SetDirectory(0);
  m_h_SysZllZhad3JetScalePt->SetDirectory(0);
  m_h_SysZllZhad3JetFor2JetScalePt->SetDirectory(0);
  m_h_SysZnunuZhad2JetInclScalePt->SetDirectory(0);
  m_h_SysZnunuZhad2JetScalePt->SetDirectory(0);
  m_h_SysZnunuZhad3JetScalePt->SetDirectory(0);
  m_h_SysZnunuZhad3JetFor2JetScalePt->SetDirectory(0);

  m_h_SysWW2JetPDFAlphaPt->SetDirectory(0);
  m_h_SysWW3JetPDFAlphaPt->SetDirectory(0);
  m_h_SysWlnuZhad2JetPDFAlphaPt->SetDirectory(0);
  m_h_SysWlnuZhad3JetPDFAlphaPt->SetDirectory(0);
  m_h_SysWhadZnunu2JetPDFAlphaPt->SetDirectory(0);
  m_h_SysWhadZnunu3JetPDFAlphaPt->SetDirectory(0);
  m_h_SysWhadZll2JetPDFAlphaPt->SetDirectory(0);
  m_h_SysWhadZll3JetPDFAlphaPt->SetDirectory(0);
  m_h_SysZllZhad2JetPDFAlphaPt->SetDirectory(0);
  m_h_SysZllZhad3JetPDFAlphaPt->SetDirectory(0);
  m_h_SysZnunuZhad2JetPDFAlphaPt->SetDirectory(0);
  m_h_SysZnunuZhad3JetPDFAlphaPt->SetDirectory(0);

  m_h_SysLepVeto2JetTop->SetDirectory(0);
  m_h_SysLepVeto2JetStop->SetDirectory(0);
  m_h_SysLepVeto2JetWbb->SetDirectory(0);
  m_h_SysLepVeto2JetWW->SetDirectory(0);
  m_h_SysLepVeto2JetWZ->SetDirectory(0);
  m_h_SysLepVeto3JetTop->SetDirectory(0);
  m_h_SysLepVeto3JetStop->SetDirectory(0);
  m_h_SysLepVeto3JetWbb->SetDirectory(0);
  m_h_SysLepVeto3JetWW->SetDirectory(0);
  m_h_SysLepVeto3JetWZ->SetDirectory(0);

  Float_t a_SysHerwigPt[5] = {1.04, 1.00, 1.00, 1.00, 1.00};

  // NEW Powheg Systematics
  Float_t a_SysWW2JetInclScalePt[5] = {1.03, 1.06, 1.09, 1.13, 1.19};
  Float_t a_SysWW2JetScalePt[5] = {1.0, 0.99, 0.98, 0.97, 0.96};
  Float_t a_SysWW3JetScalePt[5] = {0.89, 0.88, 0.86, 0.85, 0.83};
  Float_t a_SysWW3JetFor2JetScalePt[5] = {0.98, 0.96, 0.94, 0.91, 0.87};

  Float_t a_SysWlnuZhad2JetInclScalePt[5] = {1.03, 1.08, 1.12, 1.19, 1.28};
  Float_t a_SysWlnuZhad2JetScalePt[5] = {1.02, 1.03, 1.04, 1.05, 1.08};
  Float_t a_SysWlnuZhad3JetScalePt[5] = {0.88, 0.87, 0.85, 0.84, 0.83};
  Float_t a_SysWlnuZhad3JetFor2JetScalePt[5] = {0.97, 0.94, 0.90, 0.86, 0.78};

  Float_t a_SysWhadZnunu2JetInclScalePt[5] = {1.03, 1.07, 1.12, 1.18, 1.28};
  Float_t a_SysWhadZnunu2JetScalePt[5] = {1.02, 1.03, 1.04, 1.06, 1.06};
  Float_t a_SysWhadZnunu3JetScalePt[5] = {0.89, 0.88, 0.86, 0.85, 0.83};
  Float_t a_SysWhadZnunu3JetFor2JetScalePt[5] = {0.98, 0.95, 0.92, 0.88, 0.80};

  Float_t a_SysWhadZll2JetInclScalePt[5] = {1.03, 1.08, 1.13, 1.19, 1.29};
  Float_t a_SysWhadZll2JetScalePt[5] = {1.02, 1.03, 1.03, 1.05, 1.09};
  Float_t a_SysWhadZll3JetScalePt[5] = {0.88, 0.87, 0.86, 0.84, 0.82};
  Float_t a_SysWhadZll3JetFor2JetScalePt[5] = {0.98, 0.94, 0.91, 0.87, 0.79};

  Float_t a_SysZllZhad2JetInclScalePt[5] = {1.03, 1.05, 1.07, 1.10, 1.13};
  Float_t a_SysZllZhad2JetScalePt[5] = {1.0, 1.02, 1.02, 1.02, 1.05};
  Float_t a_SysZllZhad3JetScalePt[5] = {0.89, 0.88, 0.86, 0.85, 0.84};
  Float_t a_SysZllZhad3JetFor2JetScalePt[5] = {0.98, 0.96, 0.94, 0.92, 0.89};

  Float_t a_SysZnunuZhad2JetInclScalePt[5] = {1.03, 1.05, 1.07, 1.10, 1.14};
  Float_t a_SysZnunuZhad2JetScalePt[5] = {1.00, 1.01, 1.02, 1.02, 1.03};
  Float_t a_SysZnunuZhad3JetScalePt[5] = {0.89, 0.88, 0.87, 0.85, 0.84};
  Float_t a_SysZnunuZhad3JetFor2JetScalePt[5] = {0.98, 0.96, 0.95, 0.93, 0.89};

  Float_t a_SysWW2JetPDFAlphaPt[5] = {1.03, 1.03, 1.03, 1.03, 1.03};
  Float_t a_SysWW3JetPDFAlphaPt[5] = {1.02, 1.02, 1.02, 1.02, 1.02};
  Float_t a_SysWlnuZhad2JetPDFAlphaPt[5] = {1.04, 1.04, 1.04, 1.04, 1.04};
  Float_t a_SysWlnuZhad3JetPDFAlphaPt[5] = {1.02, 1.02, 1.02, 1.02, 1.02};
  Float_t a_SysWhadZnunu2JetPDFAlphaPt[5] = {1.04, 1.04, 1.04, 1.04, 1.04};
  Float_t a_SysWhadZnunu3JetPDFAlphaPt[5] = {1.02, 1.02, 1.02, 1.02, 1.02};
  Float_t a_SysWhadZll2JetPDFAlphaPt[5] = {1.04, 1.04, 1.04, 1.04, 1.04};
  Float_t a_SysWhadZll3JetPDFAlphaPt[5] = {1.02, 1.02, 1.02, 1.02, 1.02};
  Float_t a_SysZllZhad2JetPDFAlphaPt[5] = {1.03, 1.03, 1.03, 1.03, 1.03};
  Float_t a_SysZllZhad3JetPDFAlphaPt[5] = {1.03, 1.03, 1.03, 1.03, 1.03};
  Float_t a_SysZnunuZhad2JetPDFAlphaPt[5] = {1.03, 1.03, 1.03, 1.03, 1.03};
  Float_t a_SysZnunuZhad3JetPDFAlphaPt[5] = {1.03, 1.03, 1.03, 1.03, 1.03};

  Float_t a_SysLepVeto2JetTop[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto2JetStop[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto2JetWbb[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto2JetWW[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto2JetWZ[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto3JetTop[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto3JetStop[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto3JetWbb[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto3JetWW[5] = {0, 0, 0, 0, 0};
  Float_t a_SysLepVeto3JetWZ[5] = {0, 0, 0, 0, 0};

  if (m_zero && m_seven) {
    a_SysLepVeto2JetTop[2] = 0.0060;
    a_SysLepVeto2JetTop[3] = 0.0071;
    a_SysLepVeto2JetTop[4] = 0.0094;
    a_SysLepVeto2JetStop[2] = 0.0045;
    a_SysLepVeto2JetStop[3] = 0.0063;
    a_SysLepVeto2JetStop[4] = 0.0084;
    a_SysLepVeto2JetWbb[2] = 0.0024;
    a_SysLepVeto2JetWbb[3] = 0.0045;
    a_SysLepVeto2JetWbb[4] = 0.0061;
    a_SysLepVeto2JetWW[2] = 0.015;
    a_SysLepVeto2JetWW[3] = 0.0292;
    a_SysLepVeto2JetWW[4] = 0.0376;
    a_SysLepVeto2JetWZ[2] = 0.0108;
    a_SysLepVeto2JetWZ[3] = 0.0157;
    a_SysLepVeto2JetWZ[4] = 0.0130;

    a_SysLepVeto3JetTop[2] = 0.0050;
    a_SysLepVeto3JetTop[3] = 0.0062;
    a_SysLepVeto3JetTop[4] = 0.0072;
    a_SysLepVeto3JetStop[2] = 0.0036;
    a_SysLepVeto3JetStop[3] = 0.0049;
    a_SysLepVeto3JetStop[4] = 0.0051;
    a_SysLepVeto3JetWbb[2] = 0.0020;
    a_SysLepVeto3JetWbb[3] = 0.0028;
    a_SysLepVeto3JetWbb[4] = 0.0035;
    a_SysLepVeto3JetWW[2] = 0.0065;
    a_SysLepVeto3JetWW[3] = 0.0078;
    a_SysLepVeto3JetWW[4] = 0.0104;
    a_SysLepVeto3JetWZ[2] = 0.0063;
    a_SysLepVeto3JetWZ[3] = 0.0103;
    a_SysLepVeto3JetWZ[4] = 0.0077;
  } else if (m_zero && m_eight) {
    a_SysLepVeto2JetTop[2] = 0.0083;
    a_SysLepVeto2JetTop[3] = 0.0115;
    a_SysLepVeto2JetTop[4] = 0.0185;
    a_SysLepVeto2JetStop[2] = 0.0056;
    a_SysLepVeto2JetStop[3] = 0.0105;
    a_SysLepVeto2JetStop[4] = 0.0197;
    a_SysLepVeto2JetWbb[2] = 0.0059;
    a_SysLepVeto2JetWbb[3] = 0.0103;
    a_SysLepVeto2JetWbb[4] = 0.0200;
    a_SysLepVeto2JetWW[2] = 0.0044;
    a_SysLepVeto2JetWW[3] = 0.0081;
    a_SysLepVeto2JetWW[4] = 0.0128;
    a_SysLepVeto2JetWZ[2] = 0.0011;
    a_SysLepVeto2JetWZ[3] = 0.0017;
    a_SysLepVeto2JetWZ[4] = 0.0021;

    a_SysLepVeto3JetTop[2] = 0.0072;
    a_SysLepVeto3JetTop[3] = 0.0101;
    a_SysLepVeto3JetTop[4] = 0.0144;
    a_SysLepVeto3JetStop[2] = 0.0049;
    a_SysLepVeto3JetStop[3] = 0.0085;
    a_SysLepVeto3JetStop[4] = 0.0149;
    a_SysLepVeto3JetWbb[2] = 0.0047;
    a_SysLepVeto3JetWbb[3] = 0.0086;
    a_SysLepVeto3JetWbb[4] = 0.0136;
    a_SysLepVeto3JetWW[2] = 0.0025;
    a_SysLepVeto3JetWW[3] = 0.0046;
    a_SysLepVeto3JetWW[4] = 0.0069;
    a_SysLepVeto3JetWZ[2] = 0.0011;
    a_SysLepVeto3JetWZ[3] = 0.0016;
    a_SysLepVeto3JetWZ[4] = 0.0020;
  }

  Float_t a_SysMJ_dRBB_2lltag[] = {
      0,           -0.0135998, 0.00965142, 0.0800072, 0.174032,  0.250771,
      0.277158,    0.244559,   0.169417,   0.082818,  0.015993,  -0.0132827,
      -0.00720483, 0.0176533,  0.0431195,  0.0562062, 0.0454293, -0.000349641,
      -0.0786846,  -0.158951,  -0.193501,  -0.155585, -0.066539, 0.0199364,
      0.0581907,   0.0284573,  -0.0809196, -0.289582, -0.606984, -1};
  Float_t a_SysMJ_dRBB_2mmtag[] = {
      0,         0.236707,  0.450186,  0.575379,  0.564647,  0.46791,
      0.396546,  0.401034,  0.420926,  0.364748,  0.210681,  0.0170943,
      -0.141291, -0.228558, -0.254147, -0.248267, -0.234832, -0.219036,
      -0.195207, -0.166218, -0.156258, -0.204152, -0.337909, -0.546595,
      -0.775281, -0.949109, -0.996441, -0.855982, -0.502994, 0};
  Float_t a_SysMJ_dRBB_2tttag[] = {
      0,         0.128323,  0.468524,  0.924573,  1.23108,   1.27238,
      1.14299,   0.979895,  0.82623,   0.638216,  0.378942,  0.0753169,
      -0.208044, -0.408704, -0.482946, -0.431005, -0.318846, -0.236876,
      -0.212911, -0.20158,  -0.188239, -0.244364, -0.425223, -0.671554,
      -0.869165, -0.945,    -0.868227, -0.642703, -0.329841, 0};

  m_h_SysMJ_dRBB_2lltag = new TH1F(
      "SysMJ_dRBB_2lltag", "SysMJ_dRBB_2lltag",
      sizeof(a_SysMJ_dRBB_2lltag) / sizeof(a_SysMJ_dRBB_2lltag[0]), 0., 6.);
  m_h_SysMJ_dRBB_2mmtag = new TH1F(
      "SysMJ_dRBB_2mmtag", "SysMJ_dRBB_2mmtag",
      sizeof(a_SysMJ_dRBB_2mmtag) / sizeof(a_SysMJ_dRBB_2mmtag[0]), 0., 6.);
  m_h_SysMJ_dRBB_2tttag = new TH1F(
      "SysMJ_dRBB_2tttag", "SysMJ_dRBB_2tttag",
      sizeof(a_SysMJ_dRBB_2tttag) / sizeof(a_SysMJ_dRBB_2tttag[0]), 0., 6.);
  m_h_SysMJ_dRBB_2lltag->SetDirectory(0);
  m_h_SysMJ_dRBB_2mmtag->SetDirectory(0);
  m_h_SysMJ_dRBB_2tttag->SetDirectory(0);

  Float_t a_SysMJ_pTV_2lltag[] = {
      -0.5,        -0.119915,   0.0503517,  0.0444534,  0.0142201,
      0.0134692,   0.0142689,   0.00794375, 0.00400865, -2.8193e-05,
      0.00264597,  0.0387721,   0.096218,   0.0924126,  -0.049207,
      -0.144737,   -0.0509027,  0.0147166,  0,          -0.000398219,
      0.000796437, 0.000398159, 0,          0,          0,
      0,           0,           0,          0,          0,
      0,           0,           0,          0,          0,
      0,           0,           0,          0,          0,
      0,           0,           0,          0,          0,
      0,           0,           0,          0,          0};
  Float_t a_SysMJ_pTV_2mmtag[] = {-0.3,
                                  -0.117726,
                                  0.0360656,
                                  0.0104694,
                                  -0.0494054,
                                  -0.0885004,
                                  -0.0823072,
                                  -0.0380854,
                                  -0.015073,
                                  0.000139952,
                                  0.11667,
                                  0.33258,
                                  0.41441,
                                  0.156573,
                                  -0.252123,
                                  -0.404016,
                                  -0.233243,
                                  -0.0342451,
                                  0.0214897,
                                  0.00895774,
                                  0.00526297,
                                  0.00632608,
                                  0.00237072,
                                  -0.000594318,
                                  -0.000433743,
                                  1.64509e-05,
                                  -8.07047e-05,
                                  -0.000120699,
                                  -2.85506e-05,
                                  1.28746e-05,
                                  3.45707e-06,
                                  -1.13249e-06,
                                  2.02656e-06,
                                  2.02656e-06,
                                  2.38419e-07,
                                  -2.38419e-07,
                                  0,
                                  0,
                                  -5.96046e-08,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0};
  Float_t a_SysMJ_pTV_2tttag[] = {-0.4,
                                  -0.353325,
                                  -0.30889,
                                  -0.252719,
                                  -0.174699,
                                  -0.0889747,
                                  -0.0143766,
                                  0.0504112,
                                  0.116766,
                                  0.183599,
                                  0.233839,
                                  0.248563,
                                  0.222526,
                                  0.167124,
                                  0.100282,
                                  0.0364851,
                                  -0.0142018,
                                  -0.0437624,
                                  -0.0486286,
                                  -0.0342458,
                                  -0.0135418,
                                  0.00138092,
                                  0.0062809,
                                  0.00443137,
                                  0.00116265,
                                  -0.000563979,
                                  -0.000679493,
                                  -0.000238597,
                                  4.85182e-05,
                                  8.59499e-05,
                                  3.0756e-05,
                                  -5.90086e-06,
                                  -9.89437e-06,
                                  -3.27826e-06,
                                  4.76837e-07,
                                  7.15256e-07,
                                  1.19209e-07,
                                  -1.19209e-07,
                                  -5.96046e-08,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0};

  m_h_SysMJ_pTV_2lltag = new TH1F(
      "SysMJ_pTV_2lltag", "SysMJ_pTV_2lltag",
      sizeof(a_SysMJ_pTV_2lltag) / sizeof(a_SysMJ_pTV_2lltag[0]), 0., 500.e3);
  m_h_SysMJ_pTV_2mmtag = new TH1F(
      "SysMJ_pTV_2mmtag", "SysMJ_pTV_2mmtag",
      sizeof(a_SysMJ_pTV_2mmtag) / sizeof(a_SysMJ_pTV_2mmtag[0]), 0., 500.e3);
  m_h_SysMJ_pTV_2tttag = new TH1F(
      "SysMJ_pTV_2tttag", "SysMJ_pTV_2tttag",
      sizeof(a_SysMJ_pTV_2tttag) / sizeof(a_SysMJ_pTV_2tttag[0]), 0., 500.e3);
  m_h_SysMJ_pTV_2lltag->SetDirectory(0);
  m_h_SysMJ_pTV_2mmtag->SetDirectory(0);
  m_h_SysMJ_pTV_2tttag->SetDirectory(0);

  Utils2::ArraySubstractOne(a_SysHerwigPt, 5);

  Utils2::ArraySubstractOne(a_SysWW2JetInclScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWW2JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWW3JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWW3JetFor2JetScalePt, 5);

  Utils2::ArraySubstractOne(a_SysWlnuZhad2JetInclScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWlnuZhad2JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWlnuZhad3JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWlnuZhad3JetFor2JetScalePt, 5);

  Utils2::ArraySubstractOne(a_SysWhadZnunu2JetInclScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZnunu2JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZnunu3JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZnunu3JetFor2JetScalePt, 5);

  Utils2::ArraySubstractOne(a_SysWhadZll2JetInclScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZll2JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZll3JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZll3JetFor2JetScalePt, 5);

  Utils2::ArraySubstractOne(a_SysZllZhad2JetInclScalePt, 5);
  Utils2::ArraySubstractOne(a_SysZllZhad2JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysZllZhad3JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysZllZhad3JetFor2JetScalePt, 5);

  Utils2::ArraySubstractOne(a_SysZnunuZhad2JetInclScalePt, 5);
  Utils2::ArraySubstractOne(a_SysZnunuZhad2JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysZnunuZhad3JetScalePt, 5);
  Utils2::ArraySubstractOne(a_SysZnunuZhad3JetFor2JetScalePt, 5);

  Utils2::ArraySubstractOne(a_SysWW2JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysWW3JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysWlnuZhad2JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysWlnuZhad3JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZnunu2JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZnunu3JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZll2JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysWhadZll3JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysZllZhad2JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysZllZhad3JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysZnunuZhad2JetPDFAlphaPt, 5);
  Utils2::ArraySubstractOne(a_SysZnunuZhad3JetPDFAlphaPt, 5);

  Utils2::FillTH1F(5, a_SysHerwigPt, m_h_SysHerwigPt, m_allHists);

  Utils2::FillTH1F(5, a_SysWW2JetInclScalePt, m_h_SysWW2JetInclScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWW2JetScalePt, m_h_SysWW2JetScalePt, m_allHists);
  Utils2::FillTH1F(5, a_SysWW3JetScalePt, m_h_SysWW3JetScalePt, m_allHists);
  Utils2::FillTH1F(5, a_SysWW3JetFor2JetScalePt, m_h_SysWW3JetFor2JetScalePt,
                   m_allHists);

  Utils2::FillTH1F(5, a_SysWlnuZhad2JetInclScalePt,
                   m_h_SysWlnuZhad2JetInclScalePt, m_allHists);
  Utils2::FillTH1F(5, a_SysWlnuZhad2JetScalePt, m_h_SysWlnuZhad2JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWlnuZhad3JetScalePt, m_h_SysWlnuZhad3JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWlnuZhad3JetFor2JetScalePt,
                   m_h_SysWlnuZhad3JetFor2JetScalePt, m_allHists);

  Utils2::FillTH1F(5, a_SysWhadZnunu2JetInclScalePt,
                   m_h_SysWhadZnunu2JetInclScalePt, m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZnunu2JetScalePt, m_h_SysWhadZnunu2JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZnunu3JetScalePt, m_h_SysWhadZnunu3JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZnunu3JetFor2JetScalePt,
                   m_h_SysWhadZnunu3JetFor2JetScalePt, m_allHists);

  Utils2::FillTH1F(5, a_SysWhadZll2JetInclScalePt,
                   m_h_SysWhadZll2JetInclScalePt, m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZll2JetScalePt, m_h_SysWhadZll2JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZll3JetScalePt, m_h_SysWhadZll3JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZll3JetFor2JetScalePt,
                   m_h_SysWhadZll3JetFor2JetScalePt, m_allHists);

  Utils2::FillTH1F(5, a_SysZllZhad2JetInclScalePt,
                   m_h_SysZllZhad2JetInclScalePt, m_allHists);
  Utils2::FillTH1F(5, a_SysZllZhad2JetScalePt, m_h_SysZllZhad2JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysZllZhad3JetScalePt, m_h_SysZllZhad3JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysZllZhad3JetFor2JetScalePt,
                   m_h_SysZllZhad3JetFor2JetScalePt, m_allHists);

  Utils2::FillTH1F(5, a_SysZnunuZhad2JetInclScalePt,
                   m_h_SysZnunuZhad2JetInclScalePt, m_allHists);
  Utils2::FillTH1F(5, a_SysZnunuZhad2JetScalePt, m_h_SysZnunuZhad2JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysZnunuZhad3JetScalePt, m_h_SysZnunuZhad3JetScalePt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysZnunuZhad3JetFor2JetScalePt,
                   m_h_SysZnunuZhad3JetFor2JetScalePt, m_allHists);

  Utils2::FillTH1F(5, a_SysWW2JetPDFAlphaPt, m_h_SysWW2JetPDFAlphaPt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWW3JetPDFAlphaPt, m_h_SysWW3JetPDFAlphaPt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWlnuZhad2JetPDFAlphaPt,
                   m_h_SysWlnuZhad2JetPDFAlphaPt, m_allHists);
  Utils2::FillTH1F(5, a_SysWlnuZhad3JetPDFAlphaPt,
                   m_h_SysWlnuZhad3JetPDFAlphaPt, m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZnunu2JetPDFAlphaPt,
                   m_h_SysWhadZnunu2JetPDFAlphaPt, m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZnunu3JetPDFAlphaPt,
                   m_h_SysWhadZnunu3JetPDFAlphaPt, m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZll2JetPDFAlphaPt, m_h_SysWhadZll2JetPDFAlphaPt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysWhadZll3JetPDFAlphaPt, m_h_SysWhadZll3JetPDFAlphaPt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysZllZhad2JetPDFAlphaPt, m_h_SysZllZhad2JetPDFAlphaPt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysZllZhad3JetPDFAlphaPt, m_h_SysZllZhad3JetPDFAlphaPt,
                   m_allHists);
  Utils2::FillTH1F(5, a_SysZnunuZhad2JetPDFAlphaPt,
                   m_h_SysZnunuZhad2JetPDFAlphaPt, m_allHists);
  Utils2::FillTH1F(5, a_SysZnunuZhad3JetPDFAlphaPt,
                   m_h_SysZnunuZhad3JetPDFAlphaPt, m_allHists);

  Utils2::FillTH1F(5, a_SysLepVeto2JetTop, m_h_SysLepVeto2JetTop, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto2JetStop, m_h_SysLepVeto2JetStop, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto2JetWbb, m_h_SysLepVeto2JetWbb, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto2JetWW, m_h_SysLepVeto2JetWW, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto2JetWZ, m_h_SysLepVeto2JetWZ, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto3JetTop, m_h_SysLepVeto3JetTop, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto3JetStop, m_h_SysLepVeto3JetStop, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto3JetWbb, m_h_SysLepVeto3JetWbb, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto3JetWW, m_h_SysLepVeto3JetWW, m_allHists);
  Utils2::FillTH1F(5, a_SysLepVeto3JetWZ, m_h_SysLepVeto3JetWZ, m_allHists);

  Utils2::FillTH1F(m_h_SysMJ_dRBB_2lltag->GetNbinsX(), a_SysMJ_dRBB_2lltag,
                   m_h_SysMJ_dRBB_2lltag, m_allHists);
  Utils2::FillTH1F(m_h_SysMJ_dRBB_2mmtag->GetNbinsX(), a_SysMJ_dRBB_2mmtag,
                   m_h_SysMJ_dRBB_2mmtag, m_allHists);
  Utils2::FillTH1F(m_h_SysMJ_dRBB_2tttag->GetNbinsX(), a_SysMJ_dRBB_2tttag,
                   m_h_SysMJ_dRBB_2tttag, m_allHists);

  Utils2::FillTH1F(m_h_SysMJ_pTV_2lltag->GetNbinsX(), a_SysMJ_pTV_2lltag,
                   m_h_SysMJ_pTV_2lltag, m_allHists);
  Utils2::FillTH1F(m_h_SysMJ_pTV_2mmtag->GetNbinsX(), a_SysMJ_pTV_2mmtag,
                   m_h_SysMJ_pTV_2mmtag, m_allHists);
  Utils2::FillTH1F(m_h_SysMJ_pTV_2tttag->GetNbinsX(), a_SysMJ_pTV_2tttag,
                   m_h_SysMJ_pTV_2tttag, m_allHists);
  // Continuous systematics

  //** added by Lei: 2014Feb01
  m_f_SysTheoryqqVH2JetPtQCD = new TF1("m_f_SysTheoryqqVH2JetPtQCD",
                                       "[0] + [1] * x/400.e3", 0, pTbins[5]);
  m_f_SysTheoryqqVH2JetPtQCD->SetParameter(0, -0.02);
  m_f_SysTheoryqqVH2JetPtQCD->SetParameter(1, 0.10);  //** 8% at 400GeV

  m_f_SysTheoryqqVH3JetPtQCD = new TF1("m_f_SysTheoryqqVH3JetPtQCD",
                                       "[0] + [1] * x/400.e3", 0, pTbins[5]);
  m_f_SysTheoryqqVH3JetPtQCD->SetParameter(0, -0.03);
  m_f_SysTheoryqqVH3JetPtQCD->SetParameter(1, 0.13);  //** 10% at 400GeV

  m_f_SysTheoryggZH2JetPtQCD = new TF1("m_f_SysTheoryggZH2JetPtQCD",
                                       "[0] + [1] * x/400.e3", 0, pTbins[5]);
  m_f_SysTheoryggZH2JetPtQCD->SetParameter(0, -0.05);
  m_f_SysTheoryggZH2JetPtQCD->SetParameter(1, 0.25);  //** 20% at 400GeV

  m_f_SysTheoryggZH3JetPtQCD = new TF1("m_f_SysTheoryggZH3JetPtQCD",
                                       "[0] + [1] * x/400.e3", 0, pTbins[5]);
  m_f_SysTheoryggZH3JetPtQCD->SetParameter(0, -0.05);
  m_f_SysTheoryggZH3JetPtQCD->SetParameter(1, 0.25);  //** 20% at 400GeV

  m_f_SysTChanPt = new TF1("f_SysTChanPtB2",
                           "x>[2] ? [0] + [1] * [2]/1.e3 : [0] + [1] * x/1.e3",
                           0, pTbins[5]);
  m_f_SysTChanPt->SetParameter(0, 0.97 - 1);
  m_f_SysTChanPt->SetParameter(1, 0.001);
  m_f_SysTChanPt->SetParameter(2, 100e3);  // constant above

  m_f_SysWtPt2jLowPt = new TF1(
      "f_SysWtPt2jLowPt", "x>[2] ? [0] + [1] * [2]/1.e3 : [0] + [1] * x/1.e3",
      0, pTbins[5]);
  m_f_SysWtPt2jLowPt->SetParameter(0, 1.3 - 1);
  m_f_SysWtPt2jLowPt->SetParameter(1, -0.004);
  m_f_SysWtPt2jLowPt->SetParameter(2, 135e3);  // constant above

  m_f_SysWtPt2jHighPt = new TF1(
      "f_SysWtPt2jHighPt", "x>[2] ? [0] + [1] * [2]/1.e3 : [0] + [1] * x/1.e3",
      0, pTbins[5]);
  m_f_SysWtPt2jHighPt->SetParameter(0, 0.6 - 1);
  m_f_SysWtPt2jHighPt->SetParameter(1, 0.004);
  m_f_SysWtPt2jHighPt->SetParameter(2, 175e3);  // constant above

  m_f_SysWtPt3jLowPt = new TF1(
      "f_SysWtPt3jLowPt", "x>[2] ? [0] + [1] * [2]/1.e3 : [0] + [1] * x/1.e3",
      0, pTbins[5]);
  m_f_SysWtPt3jLowPt->SetParameter(0, 1.2 - 1);
  m_f_SysWtPt3jLowPt->SetParameter(1, -0.003);
  m_f_SysWtPt3jLowPt->SetParameter(2, 135e3);  // constant above

  m_f_SysWtPt3jHighPt = new TF1(
      "f_SysWtPt3jHighPt", "x>[2] ? [0] + [1] * [2]/1.e3 : [0] + [1] * x/1.e3",
      0, pTbins[5]);
  m_f_SysWtPt3jHighPt->SetParameter(0, 1.4 - 1);
  m_f_SysWtPt3jHighPt->SetParameter(1, -0.003);
  m_f_SysWtPt3jHighPt->SetParameter(2, 350e3);  // constant above

  // Song-Ming March 29, 2014
  m_f_SysWWMbb =
      new TF1("f_SysWWMbb", "x>[2] ? [0] + [1] * [2]/1.e3 : [0] + [1] * x/1.e3",
              0, pTbins[5]);
  m_f_SysWWMbb->SetParameter(0, 0.8694 - 1);
  m_f_SysWWMbb->SetParameter(1, 0.0008239);
  m_f_SysWWMbb->SetParameter(2, 300e3);  // constant above

  m_f_SysWZMbb =
      new TF1("f_SysWZMbb", "[0] + [1] / (1 + exp(-[2] * (x/1.e3 - [3])))", 0,
              pTbins[5]);
  m_f_SysWZMbb->SetParameter(0, 0.8248 - 1);
  m_f_SysWZMbb->SetParameter(1, 0.6231);
  m_f_SysWZMbb->SetParameter(2, 0.06612);
  m_f_SysWZMbb->SetParameter(3, 121.4);

  m_f_SysZZMbb =
      new TF1("f_SysZZMbb", "[0] + [1] / (1 + exp(-[2] * (x/1.e3 - [3])))", 0,
              pTbins[5]);
  m_f_SysZZMbb->SetParameter(0, 0.9477 - 1);
  m_f_SysZZMbb->SetParameter(1, 0.4185);
  m_f_SysZZMbb->SetParameter(2, 0.1069);
  m_f_SysZZMbb->SetParameter(3, 123.1);

  Utils2::SaveHist(m_f_SysTheoryqqVH2JetPtQCD, m_allHists);
  Utils2::SaveHist(m_f_SysTheoryqqVH3JetPtQCD, m_allHists);
  Utils2::SaveHist(m_f_SysTheoryggZH2JetPtQCD, m_allHists);
  Utils2::SaveHist(m_f_SysTheoryggZH3JetPtQCD, m_allHists);

  Utils2::SaveHist(m_f_SysWWMbb, m_allHists);
  Utils2::SaveHist(m_f_SysWZMbb, m_allHists);
  Utils2::SaveHist(m_f_SysZZMbb, m_allHists);
  Utils2::SaveHist(m_f_SysTChanPt, m_allHists);
  Utils2::SaveHist(m_f_SysWtPt2jLowPt, m_allHists);
  Utils2::SaveHist(m_f_SysWtPt2jHighPt, m_allHists);
  Utils2::SaveHist(m_f_SysWtPt3jLowPt, m_allHists);
  Utils2::SaveHist(m_f_SysWtPt3jHighPt, m_allHists);

  Utils2::SaveHist(m_f_dRjj, m_allHists);

  // Try new values for run II

  m_f_SysZDPhi = new TF1("f_SysZDPhi", " [0]*(x-3.141/2)/(2/3.141)", 0, 3.2);
  m_f_SysZDPhi->SetParameter(0, 0.05);
  Utils2::SaveHist(m_f_SysZDPhi, m_allHists);

  m_f_SysZPtV =
      new TF1("f_SysZPtV",
              "x>10000 ? [0]*log10(x/50000.0) :  [0]*log10(10000/50000.0) ",
              0.0, 400000.0);
  m_f_SysZPtV->SetParameter(0, 0.1);
  Utils2::SaveHist(m_f_SysZPtV, m_allHists);

  m_f_SysZMbb = new TF1("f_SysZMbb",
                        "x>[2] ? [0] * ([2]/1.e3 - [1]) : [0] * (x/1.e3 - [1])",
                        0, 400E3);
  m_f_SysZMbb->SetParameter(0, 0.0005);
  m_f_SysZMbb->SetParameter(1, 100.);
  m_f_SysZMbb->SetParameter(2, 300e3);  // constant above
  Utils2::SaveHist(m_f_SysZMbb, m_allHists);

  m_f_SysWDPhi = new TF1("f_SysWDPhi", " [0]*(x-3.141/2)/(2/3.141)", 0, 3.2);
  m_f_SysWDPhi->SetParameter(0, 0.05);
  Utils2::SaveHist(m_f_SysWDPhi, m_allHists);

  m_f_SysWPtV =
      new TF1("f_SysWPtV",
              "x>10000 ? [0]*log10(x/50000.0) :  [0]*log10(10000/50000.0) ",
              0.0, 400000.0);
  m_f_SysWPtV->SetParameter(0, 0.1);
  Utils2::SaveHist(m_f_SysWPtV, m_allHists);

  m_f_SysWMbb = new TF1("f_SysWMbb",
                        "x>[2] ? [0] * ([2]/1.e3 - [1]) : [0] * (x/1.e3 - [1])",
                        0, 400E3);
  m_f_SysWMbb->SetParameter(0, 0.002);
  m_f_SysWMbb->SetParameter(1, 100.);
  m_f_SysWMbb->SetParameter(2, 300e3);  // constant above
  Utils2::SaveHist(m_f_SysWMbb, m_allHists);

  m_f_SysTtbarPtbb =
      new TF1("f_SysTtbarPtbb",
              "x>100000 ? [1]*log10(x/100000.0) : x>10000 ? "
              "[0]*log10(x/50000.0) :  [0]*log10(10000/50000.0)  ",
              150.0, 1000000.0);
  m_f_SysTtbarPtbb->SetParameter(0, 0.1);
  m_f_SysTtbarPtbb->SetParameter(1, 0.4);
  Utils2::SaveHist(m_f_SysTtbarPtbb, m_allHists);

  m_f_SysTtbarMbb = new TF1(
      "f_SysTtbarMbb", "x>[2] ? [0] * ([2]/1.e3 - [1]) : [0] * (x/1.e3 - [1])",
      0, 400E3);
  m_f_SysTtbarMbb->SetParameter(0, 0.001);
  m_f_SysTtbarMbb->SetParameter(1, 100.);
  m_f_SysTtbarMbb->SetParameter(2, 300e3);  // constant above
  Utils2::SaveHist(m_f_SysTtbarMbb, m_allHists);

  if (m_draw) {
    TString fname("CorrsAndSysts2_");
    if (m_zero) {
      fname.Append("Zero");
    }
    if (m_one) {
      fname.Append("One");
    }
    if (m_two) {
      fname.Append("Two");
    }
    fname.Append("Lep_");
    if (m_seven) {
      fname.Append("7");
    }
    if (m_eight) {
      fname.Append("8");
    }
    fname.Append("TeV.root");
    WriteHistsToFile(fname);
  }

}  // Initialize

CAS::EventType CorrsAndSysts2::GetEventType(TString name) {
  if (m_typeNames.find(name.Data()) == m_typeNames.end()) {
    return CAS::NONAME;
    //     std::cout << "CorrsAndSysts2::ERROR - unknown event type " << name <<
    //     std::endl; exit(-1);
  }
  return m_typeNames[name.Data()];
}  // GetEventType

CAS::DetailEventType CorrsAndSysts2::GetDetailEventType(TString name) {
  if (m_detailtypeNames.find(name.Data()) == m_detailtypeNames.end()) {
    return CAS::NODETAILNAME;
    //     std::cout << "CorrsAndSysts2::ERROR - unknown detailed event type "
    //     << name << std::endl; exit(-1);
  }
  return m_detailtypeNames[name.Data()];
}  // GetEventType

CAS::SysBin CorrsAndSysts2::GetSysBin(float vpt) {
  if (vpt < pTbins[0]) {
    std::cout << "CorrsAndSysts2::ERROR - V pT " << vpt
              << " is smaller than lowest bin boundary " << pTbins[0]
              << std::endl;
  }
  if (vpt < pTbins[1]) {
    return CAS::Bin0;
  }
  if (vpt < pTbins[2]) {
    return CAS::Bin1;
  }
  if (vpt < pTbins[3]) {
    return CAS::Bin2;
  }
  if (vpt < pTbins[4]) {
    return CAS::Bin3;
  }
  return CAS::Bin4;

}  // GetSysBin

/*********************************************
 *
 *  CORRECTIONS continued - functions below
 *
 *********************************************/

float CorrsAndSysts2::Get_HiggsNLOEWKCorrection(CAS::EventType type,
                                                float VpT) {
  float scale = 0;
  switch (type) {
    case CAS::WHlvbb:
      scale = Utils2::GetScale(VpT, m_h_WHlvbbNLOEWKCorrection);
      break;
    case CAS::qqZHllbb:
      scale = Utils2::GetScale(VpT, m_h_ZHllbbNLOEWKCorrection);
      break;
    case CAS::qqZHvvbb:
      scale = Utils2::GetScale(VpT, m_h_ZHvvbbNLOEWKCorrection);
      break;
    default:
      scale = 0;
      break;
  }
  return 1 + scale;
}

float CorrsAndSysts2::Get_ggZHCorrection(CAS::EventType type, float VpT,
                                         int njet) {
  if (type != CAS::qqZHvvbb && type != CAS::qqZHllbb) {
    return 1;
  }

  TH1F* tmp_ggZH_hist = 0;
  if (type == CAS::qqZHllbb) {
    if (njet == 2)
      tmp_ggZH_hist = m_h_ggZllH125_2JetCorrection;
    else
      tmp_ggZH_hist = m_h_ggZllH125_3JetCorrection;
  } else if (type == CAS::qqZHvvbb) {
    if (njet == 2)
      tmp_ggZH_hist = m_h_ggZvvH125_2JetCorrection;
    else
      tmp_ggZH_hist = m_h_ggZvvH125_3JetCorrection;
  }

  int ptbin = tmp_ggZH_hist->FindBin(VpT) - 1;
  float scale = tmp_ggZH_hist->GetBinContent(ptbin + 1);
  return 1 + scale;
}

float CorrsAndSysts2::Get_ToppTCorrection(CAS::EventType type, float avgTopPt) {
  if (type != CAS::ttbar) {
    return 1;
  }
  float scale = Utils2::GetScale(avgTopPt, m_h_topPtCorrection);
  return 1 + scale;
}

float CorrsAndSysts2::Get_DibosonCorrection(CAS::EventType type, float VpT,
                                            CAS::DetailEventType detailtype) {
  type = type;  // not used
  if (detailtype != CAS::WWHerwig && detailtype != CAS::WZHerwig &&
      detailtype != CAS::ZZHerwig) {
    return 1;
  }
  int ptbin = m_h_pTbins->FindBin(VpT) - 1;
  float scale = m_h_SysHerwigPt->GetBinContent(ptbin + 1);
  scale *= 2;  // correction = 2 * SysUp
  // if (type == CAS::ZZ && m_eight && !m_two)
  //  scale = 1.04 * (scale + 1) - 1;
  return 1 + scale;
}

float CorrsAndSysts2::Get_DibosonCorrection(CAS::EventType type, float VpT,
                                            int mc_channel_number) {
  return Get_DibosonCorrection(type, VpT,
                               Utils2::GetDibosonType(mc_channel_number, type));
}

float CorrsAndSysts2::Get_DibosonCorrection(TString evtType, float VpT,
                                            int mc_channel_number) {
  return Get_DibosonCorrection(
      evtType, VpT,
      Utils2::GetDibosonType(mc_channel_number,
                             CorrsAndSysts2::GetEventType(evtType)));
}

float CorrsAndSysts2::Get_BkgDeltaPhiCorrection(CAS::EventType /*type*/,
                                                float DeltaPhi, int /*njet*/,
                                                float /*ptv*/) {
  DeltaPhi = fabs(TVector2::Phi_mpi_pi(DeltaPhi));
  float scale = 0;
  return 1 + scale;
}

float CorrsAndSysts2::Get_BkgNJetCorrection(CAS::EventType type, int njet,
                                            int ntag, bool sherpa) {
  float scale = 0;
  if ((type == CAS::Zl || type == CAS::Zcc || type == CAS::Zc ||
       type == CAS::Zb) ||
      (type == CAS::Wl || type == CAS::Wcc || type == CAS::Wc ||
       type == CAS::Wb)) {
    // Assume sherpa v2.2 is the default
    if (sherpa) {
      // if(ntag==0) scale=m_v_ZNJet0TagSherpaCorr[njetuse];
      // if(ntag==1) scale=m_v_ZNJet1TagSherpaCorr[njetuse];
      // if(ntag==2) scale=m_v_ZNJet2TagSherpaCorr[njetuse];
      return m_PMGSherpa22VJetsWeightTool->getSherpa22VJets_NJetCorrection(
          njet);
    } else {
      int njetuse = (njet > 3) ? 4 : njet;
      if (ntag == 0) scale = m_v_ZNJet0TagMadgraphCorr[njetuse];
      if (ntag == 1) scale = m_v_ZNJet1TagMadgraphCorr[njetuse];
      if (ntag == 2) scale = m_v_ZNJet2TagMadgraphCorr[njetuse];
    }
  }
  return 1 + scale;
}

float CorrsAndSysts2::Get_BkgPtVCorrection(CAS::EventType type, float /*ptv*/,
                                           int njet) {
  njet = njet;  // Not used
  float scale = 0;
  switch (type) {
    case CAS::Wl:
      break;
    case CAS::Wc:
      break;
    case CAS::Wcc:
      break;
    case CAS::Wb:
      break;
    case CAS::Zl:
      break;
    case CAS::Zc:
      break;
    case CAS::Zcc:
      break;
    case CAS::Zb:
      break;
    default:
      scale = 0;
      break;
  }
  return 1 + scale;
}

float CorrsAndSysts2::Get_DeltaRTruthTagCorrection(CAS::EventType type,
                                                   float /*deltaR*/, int ntag) {
  if (ntag != 2) return 1;

  float scale = 0;
  switch (type) {
    case CAS::Wcc:
    case CAS::Zcc:
      break;
    default:
      break;
  }
  return 1 + scale;
}

/*********************************************
 *
 *  SYSTEMATICS continued - functions below
 *
 *********************************************/

float CorrsAndSysts2::Get_SystematicWeight(TString evtType, float VpT,
                                           float Mbb, float ptbb, float truthPt,
                                           float DeltaPhi, float pTB1, int njet,
                                           int ntag, int mc_channel_number,
                                           TString sysName) {
  CAS::EventType type = GetEventType(evtType);
  CAS::Systematic sys;
  CAS::SysBin bin;
  CAS::SysVar var;
  GetSystFromName(sysName, sys, bin, var);

  CAS::DetailEventType detailtype =
      Utils2::GetDibosonType(mc_channel_number, type);

  DeltaPhi = fabs(TVector2::Phi_mpi_pi(DeltaPhi));
  // no systematics
  if (sys == CAS::Nominal) return 1;

  if (sys == CAS::PTBINNED || sys == CAS::CONTINUOUS) {
    std::cout << "CorrsAndSysts2::ERROR Using a systematic enum which should "
                 "not be used"
              << std::endl;
    exit(-1);
  }

  if (bin == CAS::None) {
    std::cout << "CorrsAndSysts2: no indication of type of binning given for "
                 "systematics ! Exiting..."
              << std::endl;
    exit(-1);
  }

  // sanity check: binned / continuous systematics
  if (sys < CAS::CONTINUOUS && bin == CAS::Any) {
    std::cout << "CorrsAndSysts2: asked no binning for a systematic which is "
                 "binned ! Exiting..."
              << std::endl;
    exit(-1);
  }
  if (sys >= CAS::CONTINUOUS && sys < CAS::LAST && bin != CAS::Any) {
    std::cout << "CorrsAndSysts2: asked specific bin for a systematic which is "
                 "not binned ! Exiting..."
              << std::endl;
    exit(-1);
  }

  // for binned systematics, check if the pT bin matches. else return 1
  int ptbin = m_h_pTbins->FindBin(VpT) - 1;
  int truthptbin = m_h_pTbins->FindBin(truthPt) - 1;
  if (sys > CAS::PTBINNED && sys < CAS::CONTINUOUS) {
    if (ptbin != bin) return 1;
  }

  float scale = 0;

  int sgn = 2 * var - 1;

  switch (sys) {
    case CAS::SysTheoryHPt:
      switch (type) {
        case CAS::WHlvbb:
          scale = Utils2::GetScale(truthPt, m_h_SysTheoryWHlvbbPt);
          break;
        case CAS::qqZHllbb:
          scale = Utils2::GetScale(truthPt, m_h_SysTheoryZHllbbPt);
          break;
        case CAS::qqZHvvbb:
          scale = Utils2::GetScale(truthPt, m_h_SysTheoryZHvvbbPt);
          break;
        default:
          return 1;
      }
      break;

    case CAS::SysTheoryVPtQCD:
      if (type == CAS::WHlvbb || type == CAS::qqZHvvbb ||
          type == CAS::qqZHllbb) {
        if (njet == 2)
          scale = m_f_SysTheoryqqVH2JetPtQCD->Eval(truthPt);
        else
          scale = m_f_SysTheoryqqVH3JetPtQCD->Eval(truthPt);
      } else if (type == CAS::ggZHvvbb || type == CAS::ggZHllbb) {
        if (njet == 2)
          scale = m_f_SysTheoryggZH2JetPtQCD->Eval(truthPt);
        else
          scale = m_f_SysTheoryggZH3JetPtQCD->Eval(truthPt);
      }
      break;

    case CAS::SysHerwigPt:
      if (detailtype != CAS::WWHerwig && detailtype != CAS::WZHerwig &&
          detailtype != CAS::ZZHerwig)
        return 1;
      scale = m_h_SysHerwigPt->GetBinContent(ptbin + 1);
      break;

    case CAS::SysVVJetScalePt:
      if (detailtype == CAS::WWincl && njet == 2)
        scale = m_h_SysWW2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WWincl && njet == 3)
        scale = m_h_SysWW3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZll && njet == 2)
        scale = m_h_SysWhadZll2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WlnuZhad && njet == 2)
        scale = m_h_SysWlnuZhad2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZnunu && njet == 2)
        scale = m_h_SysWhadZnunu2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WlnuZhad && njet == 3)
        scale = m_h_SysWlnuZhad3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZll && njet == 3)
        scale = m_h_SysWhadZll3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZnunu && njet == 3)
        scale = m_h_SysWhadZnunu3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZnunuZhad && njet == 2)
        scale = m_h_SysZnunuZhad2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZllZhad && njet == 2)
        scale = m_h_SysZllZhad2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZnunuZhad && njet == 3)
        scale = m_h_SysZnunuZhad3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZllZhad && njet == 3)
        scale = m_h_SysZllZhad3JetScalePt->GetBinContent(truthptbin + 1);
      else
        return 1;
      break;

    case CAS::SysVVJetScalePtST1:
      if (detailtype == CAS::WWincl && njet == 2)
        scale = m_h_SysWW3JetFor2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZll && njet == 2)
        scale = m_h_SysWhadZll3JetFor2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WlnuZhad && njet == 2)
        scale =
            m_h_SysWlnuZhad3JetFor2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZnunu && njet == 2)
        scale =
            m_h_SysWhadZnunu3JetFor2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZnunuZhad && njet == 2)
        scale =
            m_h_SysZnunuZhad3JetFor2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZllZhad && njet == 2)
        scale = m_h_SysZllZhad3JetFor2JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WlnuZhad && njet == 3)
        scale = -m_h_SysWlnuZhad3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZll && njet == 3)
        scale = -m_h_SysWhadZll3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZnunu && njet == 3)
        scale = -m_h_SysWhadZnunu3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZnunuZhad && njet == 3)
        scale = -m_h_SysZnunuZhad3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZllZhad && njet == 3)
        scale = -m_h_SysZllZhad3JetScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WWincl && njet == 3)
        scale = -m_h_SysWW3JetScalePt->GetBinContent(truthptbin + 1);
      else
        return 1;
      break;

    case CAS::SysVVJetScalePtST2:
      if (detailtype == CAS::WWincl && njet == 2)
        scale = m_h_SysWW2JetInclScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZll && njet == 2)
        scale = m_h_SysWhadZll2JetInclScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WlnuZhad && njet == 2)
        scale = m_h_SysWlnuZhad2JetInclScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZnunu && njet == 2)
        scale = m_h_SysWhadZnunu2JetInclScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZnunuZhad && njet == 2)
        scale = m_h_SysZnunuZhad2JetInclScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZllZhad && njet == 2)
        scale = m_h_SysZllZhad2JetInclScalePt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WWincl && njet == 3)
        scale = 0;
      else if (detailtype == CAS::WlnuZhad && njet == 3)
        scale = 0;
      else if (detailtype == CAS::WhadZll && njet == 3)
        scale = 0;
      else if (detailtype == CAS::WhadZnunu && njet == 3)
        scale = 0;
      else if (detailtype == CAS::ZnunuZhad && njet == 3)
        scale = 0;
      else if (detailtype == CAS::ZllZhad && njet == 3)
        scale = 0;
      else
        return 1;
      break;

    case CAS::SysVVJetPDFAlphaPt:
      if (detailtype == CAS::WWincl && njet == 2)
        scale = m_h_SysWW2JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WWincl && njet == 3)
        scale = m_h_SysWW3JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WlnuZhad && njet == 2)
        scale = m_h_SysWlnuZhad2JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZll && njet == 2)
        scale = m_h_SysWhadZll2JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZnunu && njet == 2)
        scale = m_h_SysWhadZnunu2JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WlnuZhad && njet == 3)
        scale = m_h_SysWlnuZhad3JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZll && njet == 3)
        scale = m_h_SysWhadZll3JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::WhadZnunu && njet == 3)
        scale = m_h_SysWhadZnunu3JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZnunuZhad && njet == 2)
        scale = m_h_SysZnunuZhad2JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZllZhad && njet == 2)
        scale = m_h_SysZllZhad2JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZnunuZhad && njet == 3)
        scale = m_h_SysZnunuZhad3JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else if (detailtype == CAS::ZllZhad && njet == 3)
        scale = m_h_SysZllZhad3JetPDFAlphaPt->GetBinContent(truthptbin + 1);
      else
        return 1;
      break;

    case CAS::SysLepVeto:
      if (njet == 2) {
        if (type == CAS::ttbar)
          scale = m_h_SysLepVeto2JetTop->GetBinContent(ptbin + 1);
        else if (type == CAS::stop_Wt || type == CAS::stop_s ||
                 type == CAS::stop_t)
          scale = m_h_SysLepVeto2JetStop->GetBinContent(ptbin + 1);
        else if (type == CAS::Wb || type == CAS::Wc || type == CAS::Wcc ||
                 type == CAS::Wl)
          scale = m_h_SysLepVeto2JetWbb->GetBinContent(ptbin + 1);
        else if (type == CAS::WW)  // Herwig, but use also for Powheg for now
          scale = m_h_SysLepVeto2JetWW->GetBinContent(ptbin + 1);
        else if (type == CAS::WZ)
          scale = m_h_SysLepVeto2JetWZ->GetBinContent(ptbin + 1);
        else
          return 1;
      } else if (njet == 3) {
        if (type == CAS::ttbar)
          scale = m_h_SysLepVeto3JetTop->GetBinContent(ptbin + 1);
        else if (type == CAS::stop_Wt || type == CAS::stop_s ||
                 type == CAS::stop_t)
          scale = m_h_SysLepVeto3JetStop->GetBinContent(ptbin + 1);
        else if (type == CAS::Wb || type == CAS::Wc || type == CAS::Wcc ||
                 type == CAS::Wl)
          scale = m_h_SysLepVeto3JetWbb->GetBinContent(ptbin + 1);
        else if (type == CAS::WW)
          scale = m_h_SysLepVeto3JetWW->GetBinContent(ptbin + 1);
        else if (type == CAS::WZ)
          scale = m_h_SysLepVeto3JetWZ->GetBinContent(ptbin + 1);
        else
          return 1;
      } else {
        return 1;
      }
      break;

      // now continuous systematics

      // inesochoa - added

    case CAS::SysVVMbb:
      if (type != CAS::WW && type != CAS::WZ && type != CAS::ZZ) return 1;
      if (type == CAS::WW) scale = m_f_SysWWMbb->Eval(Mbb);
      if (type == CAS::WZ) scale = m_f_SysWZMbb->Eval(Mbb);
      if (type == CAS::ZZ) scale = m_f_SysZZMbb->Eval(Mbb);
      break;

    case CAS::SysTopPt:
      if (type != CAS::ttbar) {
        return 1;
      }
      scale = Utils2::GetScale(truthPt, m_h_topPtCorrection);
      scale = .5 * scale / (1 + scale);  // DO is 50% of correction, UP is 150%
      break;

    case CAS::SysTtbarMbb:
      if (type != CAS::ttbar) return 1;
      scale = m_f_SysTtbarMbb->Eval(Mbb);
      break;

    case CAS::SysTtbarPtbb:
      if (type != CAS::ttbar) return 1;
      scale = m_f_SysTtbarPtbb->Eval(ptbb);
      break;

    case CAS::SysWDPhi:
      if (type != CAS::Wl && type != CAS::Wc && type != CAS::Wcc &&
          type != CAS::Wb)
        return 1;
      scale = m_f_SysWDPhi->Eval(DeltaPhi);
      break;
    case CAS::SysWPtV:
      if (type != CAS::Wl && type != CAS::Wc && type != CAS::Wcc &&
          type != CAS::Wb)
        return 1;
      scale = m_f_SysWPtV->Eval(truthPt);
      break;

    case CAS::SysWMbb:
      if (type != CAS::Wl && type != CAS::Wc && type != CAS::Wcc &&
          type != CAS::Wb)
        return 1;
      scale = m_f_SysWMbb->Eval(Mbb);
      break;

    case CAS::SysZMbb:
      if (type != CAS::Zl && type != CAS::Zc && type != CAS::Zcc &&
          type != CAS::Zb)
        return 1;
      scale = m_f_SysZMbb->Eval(Mbb);

      break;

    case CAS::SysZDPhi:
      if (type != CAS::Zl && type != CAS::Zc && type != CAS::Zcc &&
          type != CAS::Zb)
        return 1;
      scale = m_f_SysZDPhi->Eval(DeltaPhi);
      break;

    case CAS::SysZPtV:
      if (type != CAS::Zl && type != CAS::Zc && type != CAS::Zcc &&
          type != CAS::Zb)
        return 1;

      scale = m_f_SysZPtV->Eval(truthPt);

      break;
    case CAS::SysZNJet:
      // We use the same error for all flavours taken as half the correction
      // from the 2 tag fit
      if (type != CAS::Zl && type != CAS::Zc && type != CAS::Zcc &&
          type != CAS::Zb)
        return 1;
      if (ntag == 0) scale = m_v_ZNJet0TagSherpaCorr[njet];
      if (ntag == 1) scale = m_v_ZNJet1TagSherpaCorr[njet];
      if (ntag == 2) scale = m_v_ZNJet2TagSherpaCorr[njet];
      scale = .5 * scale / (1 + scale);  //  50% of correction, UP is 150%

      break;

    case CAS::SysWtChanAcerMC:
      if (type != CAS::stop_Wt) return 1;
      if (njet == 2 && VpT <= 120000) scale = m_f_SysWtPt2jLowPt->Eval(pTB1);
      if (njet == 3 && VpT <= 120000) scale = m_f_SysWtPt3jLowPt->Eval(pTB1);
      if (njet == 3 && VpT > 120000) scale = m_f_SysWtPt3jHighPt->Eval(Mbb);
      break;

    case CAS::SysWtChanPythiaHerwig:
      if (type != CAS::stop_Wt) return 1;
      if (njet == 2 && VpT > 120000) scale = m_f_SysWtPt2jHighPt->Eval(Mbb);
      break;

    default:
      return 1;
      break;
  }

  // generic case: symmetric uncertainties
  scale *= sgn;

  return 1 + scale;

}  // Get_SystematicWeight

float CorrsAndSysts2::Get_dRBB_pTV_MJCorrection(CAS::EventType type, bool isEl,
                                                float dRBB, float pTV, int nTag,
                                                int nJet) const {
  if (type != CAS::multijet) {
    return 1;
  }
  if (!isEl) {
    return 1;
  }
  if (nJet != 2) {
    return 1;
  }
  if (nTag < 2) {
    return 1;
  }  // 2 = 2L, 3 = 2M, 4 = 2T

  double scale = 1.;

  switch (nTag) {
    case 2:  // 2L
      scale *= (1. + m_h_SysMJ_dRBB_2lltag->GetBinContent(
                         m_h_SysMJ_dRBB_2lltag->GetXaxis()->FindBin(dRBB)));
      scale *= (1. + m_h_SysMJ_pTV_2lltag->GetBinContent(
                         m_h_SysMJ_pTV_2lltag->GetXaxis()->FindBin(pTV)));
      break;
    case 3:  // 2M
      scale *= (1. + m_h_SysMJ_dRBB_2mmtag->GetBinContent(
                         m_h_SysMJ_dRBB_2mmtag->GetXaxis()->FindBin(dRBB)));
      scale *= (1. + m_h_SysMJ_pTV_2mmtag->GetBinContent(
                         m_h_SysMJ_pTV_2mmtag->GetXaxis()->FindBin(pTV)));
      break;
    case 4:  // 2T
      scale *= (1. + m_h_SysMJ_dRBB_2tttag->GetBinContent(
                         m_h_SysMJ_dRBB_2tttag->GetXaxis()->FindBin(dRBB)));
      scale *= (1. + m_h_SysMJ_pTV_2tttag->GetBinContent(
                         m_h_SysMJ_pTV_2tttag->GetXaxis()->FindBin(pTV)));
      break;
    default:
      break;
  }

  return scale;
}

float CorrsAndSysts2::Get_MultijetSystematic(CAS::EventType type, bool isEl,
                                             float dRBB, float pTV, int nTag,
                                             int nJet, CAS::Systematic sys,
                                             CAS::SysVar var) const {
  if (type != CAS::multijet) {
    return 1;
  }
  if (!isEl) {
    return 1;
  }
  if (nJet != 2) {
    return 1;
  }
  if (nTag < 2) {
    return 1;
  }  // 2 = 2L, 3 = 2M, 4 = 2T

  double scale = 0.;
  int sgn = 2 * var - 1;

  switch (sys) {
    case CAS::SysMJElDR:

      switch (nTag) {
        case 2:  // 2L
          scale = m_h_SysMJ_dRBB_2lltag->GetBinContent(
              m_h_SysMJ_dRBB_2lltag->GetXaxis()->FindBin(dRBB));
          break;
        case 3:  // 2M
          scale = m_h_SysMJ_dRBB_2mmtag->GetBinContent(
              m_h_SysMJ_dRBB_2mmtag->GetXaxis()->FindBin(dRBB));
          break;
        case 4:  // 2T
          scale = m_h_SysMJ_dRBB_2tttag->GetBinContent(
              m_h_SysMJ_dRBB_2tttag->GetXaxis()->FindBin(dRBB));
          break;
        default:
          break;
      }  // nTag
      break;

    case CAS::SysMJElPtV:
      switch (nTag) {
        case 2:  // 2L
          scale = m_h_SysMJ_pTV_2lltag->GetBinContent(
              m_h_SysMJ_pTV_2lltag->GetXaxis()->FindBin(pTV < 150e3 ? pTV
                                                                    : 150e3));
          break;
        case 3:  // 2M
          scale = m_h_SysMJ_pTV_2mmtag->GetBinContent(
              m_h_SysMJ_pTV_2mmtag->GetXaxis()->FindBin(pTV < 150e3 ? pTV
                                                                    : 150e3));
          break;
        case 4:  // 2T
          scale = m_h_SysMJ_pTV_2tttag->GetBinContent(
              m_h_SysMJ_pTV_2tttag->GetXaxis()->FindBin(pTV < 150e3 ? pTV
                                                                    : 150e3));
          break;
        default:
          break;
      }
      break;

    default:
      return 1;
  }

  // DO is 50% of correction, UP is 150% for the corrected backgrounds
  scale = .5 * scale / (1 + scale);
  // generic case: symmetric uncertainties
  scale *= sgn;
  return 1 + scale;
}

// if using this function
//  need to have Up or Do as the last two characters of the syst name
//  the string Bin in the name if necessary
void CorrsAndSysts2::GetSystFromName(TString name, CAS::Systematic& sys,
                                     CAS::SysBin& bin, CAS::SysVar& var) {
  if (name.EndsWith("Up") || name.EndsWith("Do")) {
    var = m_varFromNames[name(name.Length() - 2, 2).Data()];
    name.Remove(name.Length() - 2);
  }
  if (name.Contains("Bin")) {
    bin = m_binFromNames[name(name.Length() - 4, 4).Data()];
    name.Remove(name.Length() - 4);
  } else {
    bin = CAS::Any;
  }
  sys = m_systFromNames[name.Data()];
}

// faster method, which does not allow to determine bin or direction
void CorrsAndSysts2::GetSystFromName(const TString& name,
                                     CAS::Systematic& sys) {
  sys = m_systFromNames[name.Data()];
}

void CorrsAndSysts2::WriteHistsToFile(TString fname) {
  TFile* file = new TFile(fname, "RECREATE");
  for (std::map<TString, TObject*>::iterator hists = m_allHists.begin();
       hists != m_allHists.end(); hists++) {
    hists->second->Write();
  }
  file->Close();
  delete file;
}  // WriteHistsToFile

/*
 *
 *  UTILITY FUNCTIONS
 *
 */

namespace Utils2 {
TH1F* BuildTH1F(std::vector<Double_t> contents, TString hname, float min,
                float max, std::map<TString, TObject*>& hists) {
  TH1F* tmp = new TH1F(hname, hname, contents.size(), min, max);
  for (unsigned int i = 1; i < contents.size() + 1; i++) {
    tmp->SetBinContent(i, contents[i - 1]);
  }
  if (tmp->GetBinContent(tmp->GetNbinsX() + 1) == 0) {
    tmp->SetBinContent(tmp->GetNbinsX() + 1,
                       tmp->GetBinContent(tmp->GetNbinsX()));
  }
  SaveHist(tmp, hists);
  return tmp;
}

void FillTH1F(std::vector<Float_t> contents, TH1F* h,
              std::map<TString, TObject*>& hists) {
  if (contents.size() != static_cast<unsigned int>(h->GetNbinsX())) {
    std::cout << "CorrsAndSysts2: filling the histogram " << h->GetName()
              << " with a wrong number of bins" << std::endl;
    exit(-1);
  }
  for (int i = 0; i < h->GetNbinsX(); i++) {
    h->SetBinContent(i + 1, contents[i]);
  }
  if (h->GetBinContent(h->GetNbinsX() + 1) == 0) {
    h->SetBinContent(h->GetNbinsX() + 1, h->GetBinContent(h->GetNbinsX()));
  }
  SaveHist(h, hists);
}

void FillTH1F(int len, Float_t* contents, TH1F* h,
              std::map<TString, TObject*>& hists) {
  if (len != h->GetNbinsX()) {
    std::cout << "CorrsAndSysts2: filling the histogram " << h->GetName()
              << " with a wrong number of bins" << std::endl;
    exit(-1);
  }
  for (int i = 0; i < h->GetNbinsX(); i++) {
    h->SetBinContent(i + 1, contents[i]);
  }
  if (h->GetBinContent(h->GetNbinsX() + 1) == 0) {
    h->SetBinContent(h->GetNbinsX() + 1, h->GetBinContent(h->GetNbinsX()));
  }
  SaveHist(h, hists);
}

void SaveHist(TObject* h, std::map<TString, TObject*>& hists) {
  if (hists.find(h->GetName()) != hists.end()) {
    std::cout << "CorrsAndSysts2::ERROR - non-unique name of "
                 "histogram/function is being used - please correct"
              << std::endl;
    exit(-1);
  }
  hists[h->GetName()] = h;
}  // SaveHist

inline float GetScale(float value, TH1F* h) {
  return h->GetBinContent(h->FindBin(value));
}

void ArraySubstractOne(float* array, unsigned int length) {
  for (unsigned int i = 0; i < length; i++) {
    array[i] -= 1;
  }
}

CAS::DetailEventType GetDibosonType(int mc_channel_number,
                                    CAS::EventType type) {
  if (type == CAS::WW && mc_channel_number == 105985)
    return CAS::WWHerwig;  // 7TeV (with lepton filter)
  else if (type == CAS::WZ && mc_channel_number == 105987)
    return CAS::WZHerwig;
  else if (type == CAS::ZZ && mc_channel_number == 105986)
    return CAS::ZZHerwig;
  else if (type == CAS::WW && mc_channel_number == 161995)
    return CAS::WWHerwig;  // 7/8TeV (without lepton filter)
  else if (type == CAS::WZ && mc_channel_number == 161996)
    return CAS::WZHerwig;
  else if (type == CAS::ZZ &&
           (mc_channel_number == 169492 || mc_channel_number == 169493))
    return CAS::ZZHerwig;
  else if (type == CAS::WW && mc_channel_number == 181971)
    return CAS::WWincl;
  else if (type == CAS::WZ && mc_channel_number == 181970)
    return CAS::WlnuZhad;
  else if (type == CAS::WZ && mc_channel_number == 181968)
    return CAS::WhadZll;
  else if (type == CAS::WZ && mc_channel_number == 181969)
    return CAS::WhadZnunu;
  else if (type == CAS::ZZ && mc_channel_number == 181967)
    return CAS::ZnunuZhad;
  else if (type == CAS::ZZ && mc_channel_number == 181966)
    return CAS::ZllZhad;
  else {
    if (type == CAS::WW || type == CAS::WZ || type == CAS::ZZ ||
        mc_channel_number == 105985 || mc_channel_number == 105987 ||
        mc_channel_number == 105986 || mc_channel_number == 161995 ||
        mc_channel_number == 161996 || mc_channel_number == 169492 ||
        mc_channel_number == 169493 || mc_channel_number == 181971 ||
        mc_channel_number == 181970 || mc_channel_number == 181968 ||
        mc_channel_number == 181969 || mc_channel_number == 181967 ||
        mc_channel_number == 181966)
      std::cout << "CorrsAndSysts2::ERROR - EventType not matching "
                   "mc_channel_number - please correct"
                << std::endl;
    return CAS::NODETAILNAME;
  }
}
}  // namespace Utils2
