
#include "CxAODTools/sumOfWeightsProvider.h"

#include <fstream>
#include <sstream>

#include "TError.h"

sumOfWeightsProvider::sumOfWeightsProvider(std::string fileName) {
  
  std::ifstream file;
  file.open(fileName.c_str());
  if (!file.good()) {
    Error("sumOfWeightsProvider()", "Can't open file '%s'.", fileName.c_str());
    exit(EXIT_FAILURE);
  }

  Info("sumOfWeightsProvider()", "Reading file '%s'.", fileName.c_str());

  // process file
  while (!file.eof()) {
    // read line
    std::string lineString;
    getline(file, lineString);
    //std::cout << lineString << std::endl;
    
    // skip empty lines                                                                                                                                                                                                                      
    if (lineString.length() == 0) {
      continue;
    }

    // skip lines starting with #
    if (lineString.find("#") == 0) {
      continue;
    }

    // store in map
    std::stringstream line(lineString);
    int sid;
    std::string name;
    float nentries;
    float nentriesSelectedOut;
    float SumOfWeights;
    line >> sid >> nentries >> nentriesSelectedOut >> SumOfWeights >> name;
    m_sumOfWeights[sid] = {name, nentries, nentriesSelectedOut, SumOfWeights};
  }
  file.close();
}

float sumOfWeightsProvider::getsumOfWeights(int mc_channel_number) {
  
  if (m_sumOfWeights.count(mc_channel_number) == 0) {
    Error("sumOfWeightsProvider::getsumOfWeights()", "Unknown mc_channel_number %i", mc_channel_number);
    exit(EXIT_FAILURE);
  }
  
  sumOfWeights& sum = m_sumOfWeights[mc_channel_number];
  if ( (361020 <= mc_channel_number) && (mc_channel_number <= 361032) ) {// 361020-361032 is dijet_JZxW
     return sum.nentries;
  } else {
     return sum.SumOfWeights;
  }
}

float sumOfWeightsProvider::getNEntries(int mc_channel_number) {
  
  if (m_sumOfWeights.count(mc_channel_number) == 0) {
    Error("sumOfWeightsProvider::getNEntries()", "Unknown mc_channel_number %i", mc_channel_number);
    exit(EXIT_FAILURE);
  }
  
  sumOfWeights& sum = m_sumOfWeights[mc_channel_number];
  return sum.nentries;
}

float sumOfWeightsProvider::getNEntriesSelectedOut(int mc_channel_number) {

  if (m_sumOfWeights.count(mc_channel_number) == 0) {
    Error("sumOfWeightsProvider::getNEntriesSelectedOut()", "Unknown mc_channel_number %i", mc_channel_number);
    exit(EXIT_FAILURE);
  }

  sumOfWeights& sum = m_sumOfWeights[mc_channel_number];
  return sum.nentriesSelectedOut;
}
void sumOfWeightsProvider::setDSIDsumOfWeights(int mc_channel_number, float sumOfWeights, float nentries) {
  
  if (m_sumOfWeights.count(mc_channel_number) == 0) {
    Error("sumOfWeightsProvider::setDSIDsumOfWeights()", "Unknown mc_channel_number %i", mc_channel_number);
    exit(EXIT_FAILURE);
  }
  m_sumOfWeights[mc_channel_number].nentries = nentries;
  m_sumOfWeights[mc_channel_number].SumOfWeights = sumOfWeights;
  return;
}
