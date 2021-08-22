// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools__EvtSelTruthAllocator_H
#define CxAODTools__EvtSelTruthAllocator_H

#include "CxAODTools/EventSelection.h"
#include "CxAODTools/xAODBaseContainerStore.h"
//#include "CxAODTools/xAODContainerStore.h"

//xAOD includes
#include "xAODTruth/TruthParticleContainer.h"

class EvtSelTruthAllocator
{
  
public:
  
  //Class constructor
  EvtSelTruthAllocator();

  //Class constructor (debug option)
  EvtSelTruthAllocator(bool debug){ SetDebug(debug); };
  
  //Class deconstructor
  ~EvtSelTruthAllocator() = default;
  
  //Diagnostic print of all reco. objects and associated truth objects if present
  template <typename T, typename U> 
  void SummariseAllocation( T* RecoObjContainer, std::string LinkName = "TruthPart" );

  //Reco. <-> truth object association method - dR
  template <typename T, typename U> 
  void RecoTruthAssociation_dR( T* RecoObjContainer, U* TruthObjContainer, std::string LinkName = "TruthPart");

  //Template Method (Master) : Clear all initialise all Props:: decorations
  template<typename T, typename U>
  void InitMatchDecorators( T* ObjContainer,  PROP<U> *property );
  
  //Extract approved Reco. object type and truth particle PDG ID
  template <typename U> int AllowedTruthPartID( U* RecoObj );

  //================ Data Members ====================
  //See : https://stackoverflow.com/questions/24702235/c-stdmap-holding-any-type-of-value
  std::map< int, xAODBaseContainerStore* > m_RecoTruthIDMapping;  //Constructed at run time. Requires that all classes are derived from each other   (1)
  std::map< xAODBaseContainerStore*, float > m_dRRecoTruthConeWidth;  //Constructed at run time. Requires that all classes are derived from each other   (1)
  //==================================================

private:

  //Private methods for setting internal data members
  void SetDebug(bool flag){ m_debug = flag; }

  //Overloaded member functions for accessing various container type 4-vectors
  TLorentzVector GetTLV(const xAOD::Jet *jet);
  template<typename T> TLorentzVector GetTLV( T *Part);

  //Bool for debug statements
  bool m_debug;
  
protected:

};

#include "CxAODTools/EvtSelTruthAllocator.icc"

#endif
