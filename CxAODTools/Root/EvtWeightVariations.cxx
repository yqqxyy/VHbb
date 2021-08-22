#include "CxAODTools/EvtWeightVariations.h"
#include "CxAODTools/CommonProperties.h"

void EvtWeightVariations::initSignalMiNLO( const int mode ){
  // Generating by the python script makeWeightMetaData.py
  // To make a map, see details in https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/LHE3EventWeights

  m_variations_SignalMiNLO.insert(std::make_pair("MUR__1down", 1));    // "muR = 0.5, muF = 1.0"
  m_variations_SignalMiNLO.insert(std::make_pair("MUR__1up", 2));      // "muR = 2.0, muF = 1.0"
  m_variations_SignalMiNLO.insert(std::make_pair("MUF__1down", 3));    // "muR = 1.0, muF = 0.5"
  m_variations_SignalMiNLO.insert(std::make_pair("MURMUF__1down", 4)); // "muR = 0.5, muF = 0.5"
  m_variations_SignalMiNLO.insert(std::make_pair("MUF__1up", 6));      // "muR = 1.0, muF = 2.0"
  m_variations_SignalMiNLO.insert(std::make_pair("MURMUF__1up", 8));   // "muR = 2.0, muF = 2.0"
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90400", 109));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90401", 110));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90402", 111));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90403", 112));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90404", 113));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90405", 114));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90406", 115));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90407", 116));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90408", 117));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90409", 118));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90410", 119));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90411", 120));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90412", 121));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90413", 122));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90414", 123));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90415", 124));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90416", 125));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90417", 126));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90418", 127));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90419", 128));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90420", 129));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90421", 130));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90422", 131));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90423", 132));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90424", 133));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90425", 134));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90426", 135));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90427", 136));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90428", 137));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90429", 138));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90430", 139));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90431", 140));
  m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 90432", 141));

  if (mode==1 || mode==2) {
    m_variations_SignalMiNLO.insert(std::make_pair("MURMUFAnti__1down", 7)); // "muR = 0.5, muF = 2.0"
    m_variations_SignalMiNLO.insert(std::make_pair("MURMUFAnti__1up", 5));   // "muR = 2.0, muF = 0.5"
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260001", 9));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260002", 10));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260003", 11));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260004", 12));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260005", 13));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260006", 14));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260007", 15));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260008", 16));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260009", 17));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260010", 18));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260011", 19));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260012", 20));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260013", 21));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260014", 22));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260015", 23));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260016", 24));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260017", 25));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260018", 26));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260019", 27));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260020", 28));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260021", 29));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260022", 30));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260023", 31));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260024", 32));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260025", 33));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260026", 34));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260027", 35));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260028", 36));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260029", 37));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260030", 38));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260031", 39));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260032", 40));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260033", 41));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260034", 42));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260035", 43));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260036", 44));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260037", 45));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260038", 46));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260039", 47));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260040", 48));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260041", 49));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260042", 50));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260043", 51));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260044", 52));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260045", 53));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260046", 54));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260047", 55));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260048", 56));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260049", 57));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260050", 58));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260051", 59));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260052", 60));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260053", 61));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260054", 62));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260055", 63));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260056", 64));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260057", 65));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260058", 66));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260059", 67));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260060", 68));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260061", 69));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260062", 70));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260063", 71));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260064", 72));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260065", 73));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260066", 74));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260067", 75));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260068", 76));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260069", 77));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260070", 78));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260071", 79));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260072", 80));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260073", 81));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260074", 82));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260075", 83));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260076", 84));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260077", 85));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260078", 86));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260079", 87));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260080", 88));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260081", 89));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260082", 90));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260083", 91));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260084", 92));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260085", 93));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260086", 94));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260087", 95));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260088", 96));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260089", 97));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260090", 98));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260091", 99));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260092", 100));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260093", 101));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260094", 102));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260095", 103));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260096", 104));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260097", 105));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260098", 106));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260099", 107));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 260100", 108));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 11068", 142));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 25200", 143));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 13165", 144));
    m_variations_SignalMiNLO.insert(std::make_pair("PDF set = 25300", 145));
    m_variations_SignalMiNLO.insert(std::make_pair("khw.0.5, kht.1, khb.1", 146));
    m_variations_SignalMiNLO.insert(std::make_pair("khw.1, kht.0, khb.1", 147));
    m_variations_SignalMiNLO.insert(std::make_pair("khw.1, kht.1, khb.0", 148));
    m_variations_SignalMiNLO.insert(std::make_pair("khw.1, kht.0, khb.0", 149));
    m_variations_SignalMiNLO.insert(std::make_pair("khw.0.5, kht.1, khb.0", 150));
    m_variations_SignalMiNLO.insert(std::make_pair("khw.0.5, kht.0, khb.1", 151));
  }
 
  //Insert the variations into a simple unordered_set of strings
  for( auto& var : m_variations_SignalMiNLO ){
    m_listOfVariations_SignalMiNLO.insert( var.first );
  }

}



void EvtWeightVariations::initSignalGG( const int mode ){
  // Generating by the python script makeWeightMetaData.py
  // To make a map, see details in https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/LHE3EventWeights

  m_variations_SignalGG.insert(std::make_pair("MUR__1down", 1));    // "muR = 0.5, muF = 1.0"
  m_variations_SignalGG.insert(std::make_pair("MUR__1up", 2));      // "muR = 2.0, muF = 1.0"
  m_variations_SignalGG.insert(std::make_pair("MUF__1down", 3));    // "muR = 1.0, muF = 0.5"
  m_variations_SignalGG.insert(std::make_pair("MURMUF__1down", 4)); // "muR = 0.5, muF = 0.5"
  m_variations_SignalGG.insert(std::make_pair("MUF__1up", 6));      // "muR = 1.0, muF = 2.0"
  m_variations_SignalGG.insert(std::make_pair("MURMUF__1up", 8));   // "muR = 2.0, muF = 2.0"
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90400", 109));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90401", 110));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90402", 111));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90403", 112));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90404", 113));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90405", 114));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90406", 115));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90407", 116));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90408", 117));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90409", 118));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90410", 119));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90411", 120));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90412", 121));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90413", 122));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90414", 123));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90415", 124));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90416", 125));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90417", 126));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90418", 127));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90419", 128));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90420", 129));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90421", 130));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90422", 131));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90423", 132));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90424", 133));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90425", 134));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90426", 135));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90427", 136));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90428", 137));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90429", 138));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90430", 139));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90431", 140));
  m_variations_SignalGG.insert(std::make_pair("PDF set = 90432", 141));

  if (mode==1 || mode==2) {
    m_variations_SignalGG.insert(std::make_pair("MURMUFAnti__1up", 5));   // "muR = 2.0, muF = 0.5"
    m_variations_SignalGG.insert(std::make_pair("MURMUFAnti__1down", 7)); // "muR = 0.5, muF = 2.0"
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260001", 9));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260002", 10));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260003", 11));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260004", 12));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260005", 13));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260006", 14));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260007", 15));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260008", 16));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260009", 17));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260010", 18));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260011", 19));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260012", 20));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260013", 21));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260014", 22));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260015", 23));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260016", 24));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260017", 25));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260018", 26));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260019", 27));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260020", 28));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260021", 29));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260022", 30));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260023", 31));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260024", 32));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260025", 33));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260026", 34));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260027", 35));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260028", 36));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260029", 37));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260030", 38));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260031", 39));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260032", 40));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260033", 41));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260034", 42));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260035", 43));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260036", 44));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260037", 45));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260038", 46));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260039", 47));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260040", 48));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260041", 49));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260042", 50));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260043", 51));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260044", 52));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260045", 53));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260046", 54));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260047", 55));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260048", 56));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260049", 57));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260050", 58));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260051", 59));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260052", 60));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260053", 61));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260054", 62));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260055", 63));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260056", 64));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260057", 65));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260058", 66));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260059", 67));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260060", 68));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260061", 69));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260062", 70));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260063", 71));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260064", 72));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260065", 73));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260066", 74));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260067", 75));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260068", 76));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260069", 77));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260070", 78));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260071", 79));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260072", 80));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260073", 81));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260074", 82));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260075", 83));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260076", 84));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260077", 85));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260078", 86));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260079", 87));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260080", 88));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260081", 89));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260082", 90));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260083", 91));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260084", 92));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260085", 93));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260086", 94));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260087", 95));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260088", 96));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260089", 97));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260090", 98));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260091", 99));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260092", 100));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260093", 101));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260094", 102));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260095", 103));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260096", 104));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260097", 105));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260098", 106));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260099", 107));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 260100", 108));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90400", 109));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90401", 110));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90402", 111));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90403", 112));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90404", 113));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90405", 114));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90406", 115));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90407", 116));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90408", 117));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90409", 118));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90410", 119));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90411", 120));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90412", 121));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90413", 122));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90414", 123));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90415", 124));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90416", 125));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90417", 126));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90418", 127));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90419", 128));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90420", 129));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90421", 130));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90422", 131));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90423", 132));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90424", 133));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90425", 134));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90426", 135));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90427", 136));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90428", 137));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90429", 138));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90430", 139));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90431", 140));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 90432", 141));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 11068", 142));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 25200", 143));
    m_variations_SignalGG.insert(std::make_pair("PDF set = 13165", 144));
    m_variations_SignalGG.insert(std::make_pair("khz.0, kht.1, khb.1", 145));
    m_variations_SignalGG.insert(std::make_pair("khz.1, kht.0, khb.1", 146));
    m_variations_SignalGG.insert(std::make_pair("khz.1, kht.1, khb.0", 147));
    m_variations_SignalGG.insert(std::make_pair("khz.1, kht.0, khb.0", 148));
    m_variations_SignalGG.insert(std::make_pair("khz.0, kht.1, khb.0", 149));
    m_variations_SignalGG.insert(std::make_pair("khz.-1, kht.0, khb.1", 150));
  }

  //Insert the variations into a simple unordered_set of strings
  for( auto& var : m_variations_SignalGG ){
    m_listOfVariations_SignalGG.insert( var.first );
  }
  
}



void EvtWeightVariations::initSherpaZmumu221(){
  // Generating by the python script makeWeightMetaData.py
  // To make a map, see details in https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/LHE3EventWeights
  
  m_variations_SherpaZmumu221.insert(std::make_pair("MURMUF__1down", 4)); // "MUR0.5_MUF0.5_PDF261000"
  m_variations_SherpaZmumu221.insert(std::make_pair("MUR__1down", 5));    // "MUR0.5_MUF1_PDF261000"
  m_variations_SherpaZmumu221.insert(std::make_pair("MUF__1down", 6));    // "MUR1_MUF0.5_PDF261000"
  //  m_variations_SherpaZmumu221.insert(std::make_pair("MUR1_MUF1_PDF261000", 7));
  m_variations_SherpaZmumu221.insert(std::make_pair("MUF__1up", 8));      // "MUR1_MUF2_PDF261000"
  m_variations_SherpaZmumu221.insert(std::make_pair("MUR__1up", 9));      // "MUR2_MUF1_PDF261000"
  m_variations_SherpaZmumu221.insert(std::make_pair("MURMUF__1up", 10));  // "MUR2_MUF2_PDF261000"
  m_variations_SherpaZmumu221.insert(std::make_pair("MUR1_MUF1_PDF25300", 11));
  m_variations_SherpaZmumu221.insert(std::make_pair("MUR1_MUF1_PDF13000", 12));

  //Insert the variations into a simple unordered_set of strings
  for( auto& var : m_variations_SherpaZmumu221 ){
    m_listOfVariations_SherpaZmumu221.insert( var.first );
  }

}



void EvtWeightVariations::initSherpaV( const int mode ){
  // Generating by the python script makeWeightMetaData.py
  // To make a map, see details in https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/LHE3EventWeights

  // All samples EXCEPT Zmumu include 100 NNPDF3.0nnlo replicas (c.f. https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/MC15SystematicUncertainties#V_jets_and_gamma_jets_Sherpa)
  // map for Zmumu is defined in initSherpaZmumu().
  m_variations_SherpaV.insert(std::make_pair("MURMUF__1down", 4)); // "MUR0.5_MUF0.5_PDF261000"
  m_variations_SherpaV.insert(std::make_pair("MUR__1down", 5));    // "MUR0.5_MUF1_PDF261000"
  m_variations_SherpaV.insert(std::make_pair("MUF__1down", 6));    // "MUR1_MUF0.5_PDF261000"
  // m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261000", 7));
  m_variations_SherpaV.insert(std::make_pair("MUF__1up", 8));      // "MUR1_MUF2_PDF261000"
  m_variations_SherpaV.insert(std::make_pair("MUR__1up", 9));      // "MUR2_MUF1_PDF261000"
  m_variations_SherpaV.insert(std::make_pair("MURMUF__1up", 10));  // "MUR2_MUF2_PDF261000"
  m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF269000", 111));
  m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF270000", 112));
  m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF25300", 113));
  m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF13000", 114));

  if (mode==1 || mode==2) {
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261001", 11));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261002", 12));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261003", 13));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261004", 14));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261005", 15));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261006", 16));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261007", 17));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261008", 18));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261009", 19));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261010", 20));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261011", 21));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261012", 22));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261013", 23));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261014", 24));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261015", 25));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261016", 26));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261017", 27));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261018", 28));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261019", 29));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261020", 30));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261021", 31));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261022", 32));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261023", 33));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261024", 34));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261025", 35));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261026", 36));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261027", 37));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261028", 38));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261029", 39));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261030", 40));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261031", 41));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261032", 42));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261033", 43));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261034", 44));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261035", 45));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261036", 46));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261037", 47));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261038", 48));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261039", 49));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261040", 50));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261041", 51));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261042", 52));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261043", 53));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261044", 54));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261045", 55));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261046", 56));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261047", 57));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261048", 58));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261049", 59));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261050", 60));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261051", 61));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261052", 62));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261053", 63));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261054", 64));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261055", 65));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261056", 66));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261057", 67));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261058", 68));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261059", 69));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261060", 70));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261061", 71));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261062", 72));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261063", 73));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261064", 74));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261065", 75));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261066", 76));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261067", 77));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261068", 78));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261069", 79));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261070", 80));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261071", 81));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261072", 82));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261073", 83));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261074", 84));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261075", 85));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261076", 86));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261077", 87));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261078", 88));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261079", 89));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261080", 90));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261081", 91));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261082", 92));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261083", 93));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261084", 94));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261085", 95));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261086", 96));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261087", 97));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261088", 98));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261089", 99));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261090", 100));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261091", 101));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261092", 102));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261093", 103));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261094", 104));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261095", 105));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261096", 106));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261097", 107));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261098", 108));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261099", 109));
    m_variations_SherpaV.insert(std::make_pair("MUR1_MUF1_PDF261100", 110));
  }

  //Insert the variations into a simple unordered_set of strings
  for( auto& var : m_variations_SherpaV ){
    m_listOfVariations_SherpaV.insert( var.first );
  }
}



void EvtWeightVariations::initPowhegPythia8Ttbar( const int mode ){
  // Generating by the python script makeWeightMetaData.py
  // To make a map, see details in https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/LHE3EventWeights
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MUR__1up", 198));   // "isr:muRfac=1.0_fsr:muRfac=2.0"
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MUR__1down", 199)); // "isr:muRfac=1.0_fsr:muRfac=0.5"

  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MURp25__1up", 211));   // "isr:muRfac=1.0_fsr:muRfac=1.25"
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MURp25__1down", 214)); // "isr:muRfac=1.0_fsr:muRfac=0.875"
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MURp50__1up", 210));   // "isr:muRfac=1.0_fsr:muRfac=1.5"
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MURp50__1down", 213)); // "isr:muRfac=1.0_fsr:muRfac=0.75"
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MURp75__1up", 209));   // "isr:muRfac=1.0_fsr:muRfac=1.75"
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("MURp75__1down", 212)); // "isr:muRfac=1.0_fsr:muRfac=0.625"

  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("RadLo", -666));      // hack for RadLow
  m_variations_PowhegPythia8Ttbar.insert(std::make_pair("RadHiPrime", -668)); // hack for RadHiPrime: like RadHi, without changing HDamp

  if ( mode==1 || mode==2 ){
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 25200", 9));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 13165", 10));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90900", 11));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 265000", 12));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 266000", 13));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 303400", 14));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90900", 11));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90901", 115));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90902", 116));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90903", 117));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90904", 118));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90905", 119));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90906", 120));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90907", 121));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90908", 122));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90909", 123));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90910", 124));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90911", 125));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90912", 126));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90913", 127));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90914", 128));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90915", 129));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90916", 130));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90917", 131));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90918", 132));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90919", 133));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90920", 134));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90921", 135));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90922", 136));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90923", 137));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90924", 138));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90925", 139));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90926", 140));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90927", 141));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90928", 142));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90929", 143));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("PDF set = 90930", 144));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("0p5muF_PDF4LHC15_NLO_30", 162));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("2muR_PDF4LHC15_NLO_30", 163));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("0p5muR_PDF4LHC15_NLO_30", 164));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("0p5muF_0p5muR_PDF4LHC15_NLO_30", 165));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("2muF_2muR_PDF4LHC15_NLO_30", 166));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("0p5muF_2muR_PDF4LHC15_NLO_30", 167));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("2muF_0p5muR_PDF4LHC15_NLO_30", 168));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("Var3cUp", 193));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("Var3cDown", 194));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=2.0_fsr:muRfac=2.0", 195));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=2.0_fsr:muRfac=1.0", 196));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=2.0_fsr:muRfac=0.5", 197));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=2.0", 198));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=0.5", 199));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=0.5_fsr:muRfac=2.0", 200));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=0.5_fsr:muRfac=1.0", 201));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=0.5_fsr:muRfac=0.5", 202));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=1.75_fsr:muRfac=1.0", 203));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=1.5_fsr:muRfac=1.0", 204));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=1.25_fsr:muRfac=1.0", 205));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=0.625_fsr:muRfac=1.0", 206));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=0.75_fsr:muRfac=1.0", 207));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("isr:muRfac=0.875_fsr:muRfac=1.0", 208));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("hardHi", 215));
    m_variations_PowhegPythia8Ttbar.insert(std::make_pair("hardLo", 216));
  }

  //Insert the variations into a simple unordered_set of strings
  for( auto& var : m_variations_PowhegPythia8Ttbar ){
    m_listOfVariations_PowhegPythia8Ttbar.insert( var.first );
  }
}

void EvtWeightVariations::initPowhegPythia8Stop( const int mode ){
  // Generating by the python script makeWeightMetaData.py
  // To make a map, see details in https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/LHE3EventWeights
  m_variations_PowhegPythia8Stop.insert(std::make_pair("MUR__1up", 147));   // "isr:muRfac=1.0_fsr:muRfac=2.0"
  m_variations_PowhegPythia8Stop.insert(std::make_pair("MUR__1down", 148)); // "isr:muRfac=1.0_fsr:muRfac=0.5"
  m_variations_PowhegPythia8Stop.insert(std::make_pair("RadLo", -665));  // hack for RadLow
  m_variations_PowhegPythia8Stop.insert(std::make_pair("RadHi", -667));  // hack for RadHi single top

  if ( mode==1 || mode==2 ){
    // these lines are commented out in case we want to re-enable these variations
    // at some point

    // m_variations_PowhegPythia8Stop.insert(std::make_pair("nominal", 0));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 1.00, muF = 2.00", 1));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 1.00, muF = 0.50", 2));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 2.00, muF = 1.00", 3));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 0.50, muF = 1.00", 4));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 0.50, muF = 0.50", 5));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 2.00, muF = 2.00", 6));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 2.00, muF = 0.50", 7));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("muR = 0.50, muF = 2.00", 8));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 25200", 9));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 13165", 10));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90900", 11));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260001", 12));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260002", 13));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260003", 14));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260004", 15));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260005", 16));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260006", 17));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260007", 18));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260008", 19));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260009", 20));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260010", 21));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260011", 22));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260012", 23));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260013", 24));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260014", 25));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260015", 26));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260016", 27));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260017", 28));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260018", 29));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260019", 30));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260020", 31));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260021", 32));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260022", 33));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260023", 34));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260024", 35));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260025", 36));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260026", 37));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260027", 38));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260028", 39));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260029", 40));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260030", 41));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260031", 42));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260032", 43));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260033", 44));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260034", 45));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260035", 46));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260036", 47));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260037", 48));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260038", 49));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260039", 50));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260040", 51));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260041", 52));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260042", 53));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260043", 54));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260044", 55));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260045", 56));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260046", 57));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260047", 58));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260048", 59));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260049", 60));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260050", 61));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260051", 62));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260052", 63));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260053", 64));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260054", 65));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260055", 66));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260056", 67));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260057", 68));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260058", 69));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260059", 70));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260060", 71));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260061", 72));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260062", 73));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260063", 74));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260064", 75));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260065", 76));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260066", 77));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260067", 78));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260068", 79));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260069", 80));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260070", 81));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260071", 82));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260072", 83));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260073", 84));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260074", 85));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260075", 86));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260076", 87));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260077", 88));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260078", 89));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260079", 90));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260080", 91));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260081", 92));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260082", 93));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260083", 94));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260084", 95));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260085", 96));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260086", 97));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260087", 98));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260088", 99));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260089", 100));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260090", 101));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260091", 102));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260092", 103));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260093", 104));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260094", 105));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260095", 106));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260096", 107));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260097", 108));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260098", 109));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260099", 110));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 260100", 111));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90901", 112));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90902", 113));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90903", 114));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90904", 115));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90905", 116));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90906", 117));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90907", 118));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90908", 119));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90909", 120));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90910", 121));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90911", 122));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90912", 123));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90913", 124));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90914", 125));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90915", 126));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90916", 127));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90917", 128));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90918", 129));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90919", 130));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90920", 131));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90921", 132));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90922", 133));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90923", 134));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90924", 135));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90925", 136));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90926", 137));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90927", 138));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90928", 139));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90929", 140));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("PDF set = 90930", 141));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("Var3cUp", 142));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("Var3cDown", 143));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=2.0_fsr:muRfac=2.0", 144));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=2.0_fsr:muRfac=1.0", 145));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=2.0_fsr:muRfac=0.5", 146));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=2.0", 147));
    // m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=0.5", 148));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=0.5_fsr:muRfac=2.0", 149));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=0.5_fsr:muRfac=1.0", 150));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=0.5_fsr:muRfac=0.5", 151));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.75_fsr:muRfac=1.0", 152));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.5_fsr:muRfac=1.0", 153));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.25_fsr:muRfac=1.0", 154));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=0.625_fsr:muRfac=1.0", 155));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=0.75_fsr:muRfac=1.0", 156));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=0.875_fsr:muRfac=1.0", 157));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=1.75", 158));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=1.5", 159));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=1.25", 160));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=0.625", 161));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=0.75", 162));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:muRfac=1.0_fsr:muRfac=0.875", 163));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("hardHi", 164));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("hardLo", 165));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:PDF:plus", 166));
    m_variations_PowhegPythia8Stop.insert(std::make_pair("isr:PDF:minus", 167));
  }

  //Insert the variations into a simple unordered_set of strings
  for( auto& var : m_variations_PowhegPythia8Stop ){
    m_listOfVariations_PowhegPythia8Stop.insert( var.first );
  }
}

bool EvtWeightVariations::isSignalMiNLO( int DSID ){
  return ( (DSID>=345053 && DSID<=345056) || (DSID>=345109 && DSID<=345112) );
}

bool EvtWeightVariations::isSignalGG( int DSID ){
  return ( (DSID>=345113 && DSID<=345114) || (DSID>=345057 && DSID<=345058) );
}

bool EvtWeightVariations::isSherpaZmumu221( int DSID ){
  return ( (364100 <= DSID && DSID <= 364113) || (364216 <= DSID && DSID <= 364217) );
}

bool EvtWeightVariations::isSherpaV( int DSID ){
  return ( (364114 <= DSID && DSID <= 364197) || // W+jets, Z+jets slices
	   (364218 <= DSID && DSID <= 364229) || // Znunu*_Sh221_PTV.
	   (366001 <= DSID && DSID <= 366035) || // Znunu*_Sh221_PTV. NB: DSID 366009, 366018 and 366027 do not exist
	   (                  DSID == 363489) || // qqWlvZqq
	   (363355 <= DSID && DSID <= 363360) || // other qqVV
	   (345043 <= DSID && DSID <= 345045) || // qqVZbb
	   (364302 <= DSID && DSID <= 364305) ); // ggVV
}

bool EvtWeightVariations::isPowhegPythia8Ttbar( int DSID ){
  return ( (DSID==345935) || (DSID==345951) || (DSID==346031) || (DSID>=407345 && DSID<=407347) || (DSID>=410470 && DSID<=410472) );
}

bool EvtWeightVariations::isPowhegPythia8Stop( int DSID ){
  return ( (DSID>=410644 && DSID<=410649) || (DSID>=410658 && DSID<=410659) );
}

// ======================================= //
//             PUBLIC METHODS              //
// ======================================= //

//Check member variable m_listOfVariations for a valid systematic
bool EvtWeightVariations::hasVariation(const std::string &SYSNAME, const int DSID ){
  std::unordered_set<std::string> listOfVariations;
  // assigns correct list of variations depending on DSID
  // if DSIS not matched, then returns 0
  if ( isSignalMiNLO( DSID ) ) listOfVariations =  m_listOfVariations_SignalMiNLO;
  else if ( isSignalGG( DSID ) ) listOfVariations =  m_listOfVariations_SignalGG;
  else if ( isSherpaZmumu221( DSID ) ) listOfVariations =  m_listOfVariations_SherpaZmumu221;
  else if ( isSherpaV( DSID ) ) listOfVariations =  m_listOfVariations_SherpaV;
  else if ( isPowhegPythia8Ttbar( DSID ) ) listOfVariations =  m_listOfVariations_PowhegPythia8Ttbar;
  else if ( isPowhegPythia8Stop( DSID ) ) listOfVariations =  m_listOfVariations_PowhegPythia8Stop;
  else return false;

  // Search m_listOfVariations for presence of said systematic
  auto result = listOfVariations.find(SYSNAME);
  return (result != listOfVariations.end());

}


float EvtWeightVariations::getWeightVariation( const xAOD::EventInfo* eventInfo, const std::string& SYSNAME ){
  if( !Props::isMC.get(eventInfo) ) return 1.0; // always assign 1.0 to data
  int DSID=eventInfo->mcChannelNumber();

  int index = -1;
  if( isSignalMiNLO( DSID ) ){
    if( m_variations_SignalMiNLO.find( SYSNAME ) == m_variations_SignalMiNLO.end() ) return Props::MCEventWeight.get(eventInfo);
    index=m_variations_SignalMiNLO[SYSNAME];
  }
  else if( isSignalGG( DSID ) ){
    if( m_variations_SignalGG.find( SYSNAME ) == m_variations_SignalGG.end() ) return Props::MCEventWeight.get(eventInfo);
    index=m_variations_SignalGG[SYSNAME];
  }
  else if( isSherpaZmumu221( DSID ) ){
    if( m_variations_SherpaZmumu221.find( SYSNAME ) == m_variations_SherpaZmumu221.end() ) return Props::MCEventWeight.get(eventInfo);
    index=m_variations_SherpaZmumu221[SYSNAME];
  }
  else if( isSherpaV( DSID ) ){
    if( m_variations_SherpaV.find( SYSNAME ) == m_variations_SherpaV.end() ) return Props::MCEventWeight.get(eventInfo);
    index=m_variations_SherpaV[SYSNAME];
  }
  else if( isPowhegPythia8Ttbar( DSID ) ){
    if( m_variations_PowhegPythia8Ttbar.find( SYSNAME ) == m_variations_PowhegPythia8Ttbar.end() ) return Props::MCEventWeight.get(eventInfo);
    index=m_variations_PowhegPythia8Ttbar[SYSNAME];
  }
  else if( isPowhegPythia8Stop( DSID ) ){
    if( m_variations_PowhegPythia8Stop.find( SYSNAME ) == m_variations_PowhegPythia8Stop.end() ) return Props::MCEventWeight.get(eventInfo);
    index=m_variations_PowhegPythia8Stop[SYSNAME];
  }

  if ( index != -1){ 
    if ( index==-666) return ( Props::MCEventWeightSys.get(eventInfo).at(6)*Props::MCEventWeightSys.get(eventInfo).at(194)/Props::MCEventWeight.get(eventInfo) ); // hack for RadLo (ttbar): "muR = 2.0, muF = 2.0"*Var3cDown/Nominal
    else if ( index==-665) return ( Props::MCEventWeightSys.get(eventInfo).at(6)*Props::MCEventWeightSys.get(eventInfo).at(143)/Props::MCEventWeight.get(eventInfo) ); // hack for RadLo (stop): "muR = 2.0, muF = 2.0"*Var3cDown/Nominal
    else if ( index==-667) return ( Props::MCEventWeightSys.get(eventInfo).at(5)*Props::MCEventWeightSys.get(eventInfo).at(142)/Props::MCEventWeight.get(eventInfo) ); // hack for RadHi (stop): "muR=05,muF=05"*Var3cUp/Nominal
    else if ( index==-668) return ( Props::MCEventWeightSys.get(eventInfo).at(5)*Props::MCEventWeightSys.get(eventInfo).at(193)/Props::MCEventWeight.get(eventInfo) ); // hack for RadHi PRIME (ttbar): "muR = 0.5, muF = 0.5"*"Var3cUp"/Nom
    else if ((int)Props::MCEventWeightSys.get(eventInfo).size() <= index && DSID == 366005){
      // Temporary solution: An uncorrect weight vector is not expected !!!!!!!!!!
      // only add the protection on 366005(metadata) or 366014(filename) sherpa ptv sample, the two DSID correspond to the same sample
      Warning("EvtWeightVariations::getWeightVariation()",
              "Index of internal weight is %d but only %d exist, return nominal weight",
              index, (int)Props::MCEventWeightSys.get(eventInfo).size());
      return Props::MCEventWeight.get(eventInfo);
    }
    else return Props::MCEventWeightSys.get(eventInfo).at(index);
  }
  else return Props::MCEventWeight.get(eventInfo); // return nominal value if it is not Sherpa 2.2.1 V+jets / Sherpa 2.2.1 Zmumu / SignalMiNLO / SignalGG / PowhegPythia8Ttbar / PowhegPythia8Stop
}



std::unordered_set<std::string> EvtWeightVariations::getListOfVariations( const int DSID ){
  std::unordered_set<std::string> listOfVariations;
  listOfVariations.clear();

  // assigns correct list of variations depending on DSID
  // if DSIS not matched, then returns
  if ( isSignalMiNLO( DSID ) ) listOfVariations =  m_listOfVariations_SignalMiNLO;
  else if ( isSignalGG( DSID ) ) listOfVariations =  m_listOfVariations_SignalGG;
  else if ( isSherpaZmumu221( DSID ) ) listOfVariations =  m_listOfVariations_SherpaZmumu221;
  else if ( isSherpaV( DSID ) ) listOfVariations =  m_listOfVariations_SherpaV;
  else if ( isPowhegPythia8Ttbar( DSID ) ) listOfVariations =  m_listOfVariations_PowhegPythia8Ttbar;
  else if ( isPowhegPythia8Stop( DSID ) ) listOfVariations =  m_listOfVariations_PowhegPythia8Stop;

  return listOfVariations;
}

//Check member variable m_listOfVariations for a valid systematic
// bool EvtWeightVariations::hasVariationSignal(const std::string &SYSNAME){
  
//   //Search m_listOfVariations for presence of said systematic
//   auto result = m_variations_SignalMiNLO.find(SYSNAME);
//   return (result != m_variations_SignalMiNLO.end());

// }

float EvtWeightVariations::GetTtbarRadHiNewNominal( const xAOD::EventInfo* eventInfo ){
  if (Props::MCEventWeightSys.get(eventInfo).size()>193){
    return Props::MCEventWeightSys.get(eventInfo).at(5)*Props::MCEventWeightSys.get(eventInfo).at(193)/Props::MCEventWeight.get(eventInfo); // "muR = 0.5, muF = 0.5"*"Var3cUp"/No
  }
  else 
    return Props::MCEventWeightSys.get(eventInfo).at(5)/Props::MCEventWeight.get(eventInfo);
}
