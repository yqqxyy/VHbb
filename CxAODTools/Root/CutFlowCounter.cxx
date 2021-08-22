#include "CxAODTools/CutFlowCounter.h"
#include <iostream>

CutFlowCounter::CutFlowCounter( TString name ) :
  m_name(name),
  m_previousPriority(-1)
{
  //std::cout<<" cutFlow name: "<<m_name<<std::endl;
}


// Set the name of cut one by one to setup the cutflowInfor map 
void CutFlowCounter::setCutFlow( TString cutName, int priority ) {
  
  if (priority < 0) {
    priority = m_previousPriority + 1;
  }

  //std::cout<<" priority "<<priority<<" cut name: "<<cutName<<std::endl;

  Counter cutInfor;
  cutInfor.priority = priority;
  cutInfor.count    = 0;
  cutInfor.sumOfWeight   = 0;

  m_cutFlowInfor[cutName.Data()] = cutInfor;
  //m_cutFlowInfor.insert ( std::pair<TString, Counter>( cutName, cutInfor ) );

  //std::cout<<" 0 ncutFlow: "<<m_cutFlowInfor.size()<<std::endl;
  
  m_previousPriority = priority;
}

// Count the number of entries and weights passing the cut
void CutFlowCounter::count( TString cutName, int priority, double weight ) {
  
  // TODO check on unique priority?
  // or just use map <priority, counter> and check names?

  if( m_cutFlowInfor.find( cutName.Data() )==m_cutFlowInfor.end() ) {
    setCutFlow( cutName, priority ) ;
  }

  m_cutFlowInfor[cutName.Data() ].count +=1;
  m_cutFlowInfor[cutName.Data() ].sumOfWeight +=weight;

}

// Don't count in some case, e.g. systematics variations and etc
TH1D * CutFlowCounter::getCutFlow( TString prefix ) {
  int  ncutFlow = m_cutFlowInfor.size();
  if (ncutFlow == 0) {
    ncutFlow = 1;
  }

  //std::cout<<" ncutFlow: "<<ncutFlow<<std::endl;
  
  TString fullName = prefix + m_name;

  TH1D * m_h_cutFlow = new TH1D( fullName, fullName, ncutFlow, 0, ncutFlow );
  //m_h_cutFlow ->Print();

  std::vector< std::pair< std::string, Counter > >  vectorToOrder;

  std::map<std::string, Counter>::iterator it=m_cutFlowInfor.begin();
  for( ; it!=m_cutFlowInfor.end(); it++ ) {
    vectorToOrder.push_back( std::make_pair(it->first, it->second) );
  }

  //std::sort(vectorToOrder.begin(), vectorToOrder.end(), ComparePriority );

  for( unsigned int idx=0; idx<vectorToOrder.size(); idx++ ) {
    std::pair< std::string, Counter > element = vectorToOrder[idx];
    //std::cout<<" order 0: "<< element.second.priority<<" idx: "<< idx <<std::endl;
  }

  for( unsigned int idx0=0; idx0<vectorToOrder.size(); idx0++ ) {
    
    unsigned int tmpMin_idx = idx0;

    for( unsigned int idx1=idx0+1; idx1<vectorToOrder.size(); idx1++ ) {

      //std::cout<<" 1: "<< vectorToOrder[tmpMin_idx].second.priority<<" 2: "<< vectorToOrder[idx1].second.priority <<std::endl;

      if( vectorToOrder[idx1].second.priority < vectorToOrder[tmpMin_idx].second.priority ) {
        tmpMin_idx = idx1;
      }

    }

    if(tmpMin_idx!=idx0) {
      std::pair< std::string, Counter > tmpelement = vectorToOrder[idx0];
      vectorToOrder[idx0] = vectorToOrder[tmpMin_idx];
      vectorToOrder[tmpMin_idx] = tmpelement;
    }

  }

  for( unsigned int idx=0; idx<vectorToOrder.size(); idx++ ) {
    std::pair< std::string, Counter > element = vectorToOrder[idx];
    //std::cout<<" order 2: "<< element.second.priority<<" idx: "<< idx <<std::endl;
  }

  for( unsigned int idx=0; idx<vectorToOrder.size(); idx++ ) {

    std::pair< std::string, Counter > element = vectorToOrder[idx];

    //std::cout<<" order: "<< element.second.priority<<" idx: "<< idx <<std::endl;

    m_h_cutFlow->GetXaxis()->SetBinLabel( idx+1 , element.first.c_str() );
    m_h_cutFlow->SetBinContent( idx+1 , element.second.sumOfWeight );
  }

  return m_h_cutFlow;
}
