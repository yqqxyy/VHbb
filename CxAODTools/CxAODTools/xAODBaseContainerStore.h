// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools__xAODBaseContainerStore_H
#define CxAODTools__xAODBaseContainerStore_H

//C++ 11 standard library includes
#include <iostream>

//Base Storage clas for erasure of typename requirement during map loading
class xAODBaseContainerStore
{

public:
  
  //Class constructore
  xAODBaseContainerStore();  //Default constructor
  
  //Class deconstructor
  ~xAODBaseContainerStore() = default;  //Default deconstructor

  //Virtual function to enable xAODBaseContainerStore pointers to access dervied class m_container object type
  virtual std::string GetContainerType() = 0;

  
  
};

#endif
