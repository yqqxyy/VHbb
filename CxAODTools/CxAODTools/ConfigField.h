#ifndef CxAODTools_ConfigField_H
#define CxAODTools_ConfigField_H

// Analysis includes
#include "CxAODTools/ConfigFieldBase.h"

// STL include
#include <iostream>
#include <cxxabi.h>

template <class T>
class ConfigField : public ConfigFieldBase {

public:

  // default constructor for RootCint
  ConfigField() {}

  // constructor
  ConfigField(const T& value) : m_value(value) {}

  // copy constructor
  template <class U>
  ConfigField(const ConfigField<U>& other) : m_value( U(other.value) ) { }

  // assignment operator
  template <class U>
  ConfigField& operator=(const ConfigField<U>& other)
  {
    m_value = U(other.value) ;
    return *this;
  }

  // destructor
  virtual ~ConfigField() {}

  // method to retrieve value of field
  const T& get() const { return m_value ; }

  // clone
  ConfigFieldBase* clone() const { return new ConfigField<T>( m_value ) ; }

  // print
  void print(const std::string &fieldName) const;

private:

  T m_value;

  ClassDef(ConfigField, 1);

};

#include "CxAODTools/ConfigField.icc"

#endif
