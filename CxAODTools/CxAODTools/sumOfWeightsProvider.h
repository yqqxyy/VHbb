#ifndef CxAODTools_sumOfWeightsProvider_H
#define CxAODTools_sumOfWeightsProvider_H

#include <string>
#include <map>

class sumOfWeightsProvider {
  
private:
  
  struct sumOfWeights {
    std::string name;
    float nentries;
    float nentriesSelectedOut;
    float SumOfWeights;
  };
  
  std::map<int, sumOfWeights> m_sumOfWeights;
  
public:
  
  sumOfWeightsProvider(std::string fileName);
  ~sumOfWeightsProvider() = default;
  
  bool hasMCchannel(int mc_channel_number) {return m_sumOfWeights.count(mc_channel_number);}
  float getsumOfWeights(int mc_channel_number);
  float getNEntries(int mc_channel_number);
  float getNEntriesSelectedOut(int mc_channel_number);
  void setDSIDsumOfWeights(int mc_channel_number, float sumOfWeights, float nentries = -1);
};


#endif
