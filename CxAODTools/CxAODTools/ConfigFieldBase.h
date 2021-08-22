#ifndef CxAODTools_ConfigFieldBase_H
#define CxAODTools_ConfigFieldBase_H

// RootCint include
#include "Rtypes.h"


class ConfigFieldBase {

 public:

  // constructor
  ConfigFieldBase() {};

  // destructor
  virtual ~ConfigFieldBase() {};

  // clone
  virtual ConfigFieldBase* clone() const = 0;

  // print content
  // fieldName is not known to ConfigFieldBase and must be
  // provided by the calling instance as function argument
  virtual void print(const std::string &) const = 0;

  ClassDef(ConfigFieldBase, 1);

};

#endif
