// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools_VBFHbb_CommonProperties_H
#define CxAODTools_VBFHbb_CommonProperties_H

#ifndef __MAKECINT__

#include "CxAODTools/CommonProperties.h"


// Preprocessor macro to define property classes
//
// Usage example:
// (in ~ CxAODTools_MyAnalysis/CxAODTools_MyAnalysis/Properties_MyAnalysis.h)
//
// #include "CxAODTools/CommonProperties.h"
// PROPERTY( Props , int   , myInt   );
// PROPERTY( Props , float , myFloat );
// PROPERTY( MyProps , int   , myInt   );
// ...

// Note, it is _never_ needed to specify your own namespace "MyProps" if the
// property already exists in another namespace, e.g. in "Props". In general,
// it's better always use namespace "Props" in your own analysis, since you
// then get an extra compile time check on whether you declared a property
// with the same name as another previously declared property but with a
// different type (if using a different namespace, this would compile, but
// fail at runtime).

// IMPORTANT :
// Don't attempt use bool properties !!!
// A bool properties gets saved as char when written to disk.
// Consequently, you cannot use the same code to access the property, when
// reading the file back in. For this reason, the bool template is disabled.


//PROPERTY( Props , int , PartonTruthLabelID )
PROPERTY( Props , int , npv)

// If Triggers Fired
PROPERTY( Props , int , pass_L1_2J25)
PROPERTY( Props , int , pass_L1_J25_0ETA23)
PROPERTY( Props , int , pass_L1_HT150_JJ15_ETA49)
PROPERTY( Props , int , pass_L1_2J15_31ETA49)
PROPERTY( Props , int , pass_L1_HT150_J20s5_ETA31)
PROPERTY( Props , int , pass_L1_MJJ_400)
PROPERTY( Props , int , pass_L1_J40_0ETA25)
PROPERTY( Props , int , pass_L1_MJJ_400_CF)
PROPERTY( Props , int , pass_L1_J20_31ETA49)
PROPERTY( Props , int , pass_L1_J40_0ETA25_2J25_J20_31ETA49)
PROPERTY( Props , int , pass_L1_J25_0ETA23_2J15_31ETA49)
PROPERTY( Props , int , pass_L1_HT150_J20s5_ETA31_MJJ_400_CF)
PROPERTY( Props , int , pass_L1_J40_0ETA25_2J15_31ETA49)
PROPERTY( Props , int , pass_L1_HT150_JJ15_ETA49_MJJ_400)
PROPERTY( Props , int , pass_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490)
PROPERTY( Props , int , pass_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49)
PROPERTY( Props , int , pass_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split)
PROPERTY( Props , int , pass_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split)
PROPERTY( Props , int , pass_HLT_j80_0eta240_j60_j45_320eta490)
PROPERTY( Props , int , pass_HLT_j80_0eta240_2j60_320eta490)
PROPERTY( Props , int , pass_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49)

// If Triggers were active
PROPERTY( Props , int , isActive_L1_2J25)
PROPERTY( Props , int , isActive_L1_J25_0ETA23)
PROPERTY( Props , int , isActive_L1_HT150_JJ15_ETA49)
PROPERTY( Props , int , isActive_L1_2J15_31ETA49)
PROPERTY( Props , int , isActive_L1_HT150_J20s5_ETA31)
PROPERTY( Props , int , isActive_L1_MJJ_400)
PROPERTY( Props , int , isActive_L1_J40_0ETA25)
PROPERTY( Props , int , isActive_L1_MJJ_400_CF)
PROPERTY( Props , int , isActive_L1_J20_31ETA49)
PROPERTY( Props , int , isActive_L1_J40_0ETA25_2J25_J20_31ETA49)
PROPERTY( Props , int , isActive_L1_J25_0ETA23_2J15_31ETA49)
PROPERTY( Props , int , isActive_L1_HT150_J20s5_ETA31_MJJ_400_CF)
PROPERTY( Props , int , isActive_L1_J40_0ETA25_2J15_31ETA49)
PROPERTY( Props , int , isActive_L1_HT150_JJ15_ETA49_MJJ_400)
PROPERTY( Props , int , isActive_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490)
PROPERTY( Props , int , isActive_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49)
PROPERTY( Props , int , isActive_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split)
PROPERTY( Props , int , isActive_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split)
PROPERTY( Props , int , isActive_HLT_j80_0eta240_j60_j45_320eta490)
PROPERTY( Props , int , isActive_HLT_j80_0eta240_2j60_320eta490)
PROPERTY( Props , int , isActive_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49)

// Prescales
PROPERTY( Props , float , prescale_L1_2J25)
PROPERTY( Props , float , prescale_L1_J25_0ETA23)
PROPERTY( Props , float , prescale_L1_HT150_JJ15_ETA49)
PROPERTY( Props , float , prescale_L1_2J15_31ETA49)
PROPERTY( Props , float , prescale_L1_HT150_J20s5_ETA31)
PROPERTY( Props , float , prescale_L1_MJJ_400)
PROPERTY( Props , float , prescale_L1_J40_0ETA25)
PROPERTY( Props , float , prescale_L1_MJJ_400_CF)
PROPERTY( Props , float , prescale_L1_J20_31ETA49)
PROPERTY( Props , float , prescale_L1_J40_0ETA25_2J25_J20_31ETA49)
PROPERTY( Props , float , prescale_L1_J25_0ETA23_2J15_31ETA49)
PROPERTY( Props , float , prescale_L1_HT150_J20s5_ETA31_MJJ_400_CF)
PROPERTY( Props , float , prescale_L1_J40_0ETA25_2J15_31ETA49)
PROPERTY( Props , float , prescale_L1_HT150_JJ15_ETA49_MJJ_400)
PROPERTY( Props , float , prescale_HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490)
PROPERTY( Props , float , prescale_HLT_j35_gsc55_bmv2c1070_split_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49)
PROPERTY( Props , float , prescale_HLT_ht300_2j40_0eta490_invm700_L1HT150_J20s5_ETA31_MJJ_400_CF_AND_2j25_gsc45_bmv2c1070_split)
PROPERTY( Props , float , prescale_HLT_j80_0eta240_j60_j45_320eta490_AND_2j25_gsc45_bmv2c1070_split)
PROPERTY( Props , float , prescale_HLT_j80_0eta240_j60_j45_320eta490)
PROPERTY( Props , float , prescale_HLT_j80_0eta240_2j60_320eta490)
PROPERTY( Props , float , prescale_HLT_j55_0eta240_2j45_320eta490_L1J25_0ETA23_2J15_31ETA49)


PROPERTY( Props , std::vector<float> , weight_pdf4lhc)
PROPERTY( Props , std::vector<float> , weight_pdfnnpdf30)
PROPERTY( Props , std::vector<float> , weight_qcd_nnlops)
PROPERTY( Props , std::vector<float> , weight_qcd_nnlops_2np)
PROPERTY( Props , std::vector<float> , weight_qcd_WG1)
PROPERTY( Props , std::vector<float> , weight_alternative_pdf)
PROPERTY( Props , float, alpha_s_up)
PROPERTY( Props , float, alpha_s_dn)

/// trigger matching objects<

PROPERTY( Props , int , HLTBJetMatched_2J45_MV2c20_70)
PROPERTY( Props , int , HLTBJetMatched_2J35_MV2c20_60)
PROPERTY( Props , int , HLTBJetMatched_J80_MV2c20_70)
PROPERTY( Props , int , HLTBJetMatched_J60_MV2c20_85)


PROPERTY( Props , int , HLTBJetMatched_2j25_gsc45_bmv2c1070_split )
PROPERTY( Props , int , HLTBJetMatched_j55_gsc80_bmv2c1070_split )
PROPERTY( Props , int , HLTBJetMatched_j45_gsc60_bmv2c1085_split )
PROPERTY( Props , int , HLTBJetMatched_j35_gsc55_bmv2c1070_split )





// // OLD TRIGGERS -- TO BE REMOVED

// // 2 central + 1 forward trigger
// PROPERTY( Props , int , passL1_J40_0ETA25_2J25_J20_31ETA49 )
// PROPERTY( Props , int , passHLT_j80_bmv2c2070_split_j60_bmv2c2085_split_j45_320eta490 )
// PROPERTY( Props , int , passHLT_j80_0eta240_j60_j45_320eta490 )
// // 1 central + 2 forward trigger
// PROPERTY( Props , int , passL1_J40_0ETA25_2J15_31ETA49 )
// PROPERTY( Props , int , passHLT_j80_bmv2c2085_split_2j60_320eta490 )
// PROPERTY( Props , int , passHLT_j80_0eta240_2j60_320eta490 )
// // old triggers
// PROPERTY( Props , int , passL1_4J20 )
// PROPERTY( Props , int , passHLT_2j55_bmv2c2077_split_2j55 )
// PROPERTY( Props , int , passHLT_2j45_bmv2c2070_split_2j45 )
// PROPERTY( Props , int , passL1_4J15 )
// PROPERTY( Props , int , passHLT_2j35_bmv2c2070_split_2j35_L14J15 )
// PROPERTY( Props , int , passHLT_2j35_bmv2c2060_split_2j35_L14J15 )
// PROPERTY( Props , int , passHLT_2j35_bmv2c2050_split_2j35_L14J15 )
// PROPERTY( Props , int , passHLT_j75_bmv2c2060_split_3j75_L14J15 )
// PROPERTY( Props , int , passHLT_j75_bmv2c2070_split_3j75_L14J15 )
// // additional triggers for photon study
// PROPERTY( Props , int , passHLT_2j45_bmv2c2077_split_2j45_L14J15 )
// PROPERTY( Props , int , passHLT_2j45_bmv2c2070_split_2j45_L14J15 )
// PROPERTY( Props , int , passHLT_2j45_bmv2c2070_split_2j45_L14J20 )
// PROPERTY( Props , int , passHLT_4j45 )

// //################################################################
// // trigger emulation parts to make proper turn on curves with data
// //################################################################
// // 2 central + 1 forward trigger
// PROPERTY( Props , int , passL1_J40_0ETA25_2J25 )
// PROPERTY( Props , int , passL1_J20_31ETA49 )
// PROPERTY( Props , int , passHLT_j80_bmv2c2070_split_j60_bmv2c2085_split )
// PROPERTY( Props , int , passHLT_j45_320eta490 )
// PROPERTY( Props , int , passHLT_j80_0eta240_j60 )
// // 1 central + 2 forward trigger
// PROPERTY( Props , int , passL1_J40_0ETA25 )
// PROPERTY( Props , int , passL1_2J15_31ETA49 )
// PROPERTY( Props , int , passHLT_j80_bmv2c2085_split )
// PROPERTY( Props , int , passHLT_2j60_320eta490 )
// PROPERTY( Props , int , passHLT_j80_0eta240 )

// //=======
// //########################
// // trigger emulation tool
// //########################

// PROPERTY( Props , int , passEmulToolL1_4J15 )
// PROPERTY( Props , int , passEmulToolL1_2J15_31ETA49 )
// PROPERTY( Props , int , passEmulToolL1_J40_0ETA25 )
// PROPERTY( Props , int , passEmulToolL1_J20_31ETA49 )
// PROPERTY( Props , int , passEmulToolL1_2J15 )
// PROPERTY( Props , int , passEmulToolL1_2J25 )

// PROPERTY( Props , int , passEmulToolHLT_4j35 )
// PROPERTY( Props , int , passEmulToolHLT_4j45 )
// PROPERTY( Props , int , passEmulToolHLT_2j35_bmv2c2060_split )
// PROPERTY( Props , int , passEmulToolHLT_2j45_bmv2c2070_split )
// PROPERTY( Props , int , passEmulToolHLT_j80_bmv2c2070_split  )
// PROPERTY( Props , int , passEmulToolHLT_2j60_bmv2c2085_split )
// PROPERTY( Props , int , passEmulToolHLT_j80_bmv2c2085_split  )
// PROPERTY( Props , int , passEmulToolHLT_j45_320eta490 )
// PROPERTY( Props , int , passEmulToolHLT_2j60_320eta490 )
// // Supporting HLT triggers
// PROPERTY( Props , int , passEmulToolHLT_j80_0eta240)
// PROPERTY( Props , int , passEmulToolHLT_2j60)

// PROPERTY( Props , int , passEmulToolHLT_2j35_bmv2c2060_split_2j35_L14J15 )
// PROPERTY( Props , int , passEmulToolHLT_2j45_bmv2c2070_split_2j45_L14J15 )
// PROPERTY( Props , int , passEmulToolHLT_j80_bmv2c2070_split_j60_bmv2c2085_split_j45_320eta490_L1_J40_0ETA25_2J25_J20_31ETA49 )
// PROPERTY( Props , int , passEmulToolHLT_j80_bmv2c2085_split_2j60_320eta490_L1_J40_0ETA25_2J15_31ETA49 )
// //>>>>>>> 74df6ffc79544f3eaba934dbfee7cf7ba57cd8ca


// //#############################
// // were triggers active or not?
// //#############################
// // 2 central + 1 forward trigger
// PROPERTY( Props , int , isActiveL1_J40_0ETA25_2J25_J20_31ETA49 )
// PROPERTY( Props , int , isActiveHLT_j80_bmv2c2070_split_j60_bmv2c2085_split_j45_320eta490 )
// PROPERTY( Props , int , isActiveHLT_j80_0eta240_j60_j45_320eta490 )
// // 1 central + 2 forward trigger
// PROPERTY( Props , int , isActiveL1_J40_0ETA25_2J15_31ETA49 )
// PROPERTY( Props , int , isActiveHLT_j80_bmv2c2085_split_2j60_320eta490 )
// PROPERTY( Props , int , isActiveHLT_j80_0eta240_2j60_320eta490 )
// // old triggers
// PROPERTY( Props , int , isActiveL1_4J20 )
// PROPERTY( Props , int , isActiveHLT_2j55_bmv2c2077_split_2j55 )
// PROPERTY( Props , int , isActiveHLT_2j45_bmv2c2070_split_2j45 )
// PROPERTY( Props , int , isActiveL1_4J15 )
// PROPERTY( Props , int , isActiveHLT_2j35_bmv2c2070_split_2j35_L14J15 )
// PROPERTY( Props , int , isActiveHLT_2j35_bmv2c2060_split_2j35_L14J15 )
// PROPERTY( Props , int , isActiveHLT_2j35_bmv2c2050_split_2j35_L14J15 )
// PROPERTY( Props , int , isActiveHLT_j75_bmv2c2060_split_3j75_L14J15 )
// PROPERTY( Props , int , isActiveHLT_j75_bmv2c2070_split_3j75_L14J15 )
// // additional triggers for photon study
// PROPERTY( Props , int , isActiveHLT_g140_loose )
// PROPERTY( Props , int , isActiveHLT_g25_medium )
// PROPERTY( Props , int , isActiveHLT_2j45_bmv2c2077_split_2j45_L14J15 )
// PROPERTY( Props , int , isActiveHLT_2j45_bmv2c2070_split_2j45_L14J15 )
// PROPERTY( Props , int , isActiveHLT_2j45_bmv2c2070_split_2j45_L14J20 )
// PROPERTY( Props , int , isActiveHLT_4j45 )




// // 2 central + 1 forward trigger prescales
// PROPERTY( Props , float , prescaleL1_J40_0ETA25_2J25_J20_31ETA49 )
// PROPERTY( Props , float , prescaleHLT_j80_bmv2c2070_split_j60_bmv2c2085_split_j45_320eta490 )
// PROPERTY( Props , float , prescaleHLT_j80_0eta240_j60_j45_320eta490 )
// // 1 central + 2 forward trigger prescales
// PROPERTY( Props , float , prescaleL1_J40_0ETA25_2J15_31ETA49 )
// PROPERTY( Props , float , prescaleHLT_j80_bmv2c2085_split_2j60_320eta490 )
// PROPERTY( Props , float , prescaleHLT_j80_0eta240_2j60_320eta490 )
// // old trigger prescales
// PROPERTY( Props , float , prescaleL1_4J20 )
// PROPERTY( Props , float , prescaleHLT_2j55_bmv2c2077_split_2j55 )
// PROPERTY( Props , float , prescaleHLT_2j45_bmv2c2070_split_2j45 )
// PROPERTY( Props , float , prescaleL1_4J15 )
// PROPERTY( Props , float , prescaleHLT_2j35_bmv2c2070_split_2j35_L14J15 )
// PROPERTY( Props , float , prescaleHLT_2j35_bmv2c2060_split_2j35_L14J15 )
// PROPERTY( Props , float , prescaleHLT_2j35_bmv2c2050_split_2j35_L14J15 )
// PROPERTY( Props , float , prescaleHLT_j75_bmv2c2060_split_3j75_L14J15 )
// PROPERTY( Props , float , prescaleHLT_j75_bmv2c2070_split_3j75_L14J15 )
// // additional triggers for photon study prescales
// PROPERTY( Props , float , prescaleHLT_g140_loose )
// PROPERTY( Props , float , prescaleHLT_g25_medium )
// PROPERTY( Props , float , prescaleHLT_2j45_bmv2c2077_split_2j45_L14J15 )
// PROPERTY( Props , float , prescaleHLT_2j45_bmv2c2070_split_2j45_L14J15 )
// PROPERTY( Props , float , prescaleHLT_2j45_bmv2c2070_split_2j45_L14J20 )
// PROPERTY( Props , float , prescaleHLT_4j45 )


#endif // __MAKECINT__
#endif //CxAODMaker_VBFHbb_CommonProperties_H
