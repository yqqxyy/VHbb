// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools_ObjectDecorator_H
#define CxAODTools_ObjectDecorator_H

// Standard Template Library includes
#include <string>
#include <type_traits>

#ifndef __MAKECINT__
// xAOD EDM includes
#include "AthContainers/AuxElement.h"


// Preprocessor macro to define property classes
// 
// Usage example:
// (in ~ CxAODTools_MyAnalysis/CxAODTools_MyAnalysis/ObjectDecorator_MyAnalysis.h)
// 
// #include "CxAODTools/ObjectDecorator.h"
// PROPERTY( Props , int   , passPreSel );
// PROPERTY( Props , float , charge     );
// ...

// Script for declaring and instantiating a derived PROP class
#define PROPERTY(scope,type,name) \
  namespace scope { \
    class name##_class : public PROP<type> { public : name##_class() : PROP<type>(#scope,#type,#name) {} }; \
    static name##_class name; \
  }

// Script for declaring a derived PROP class
#define PROPERTY_DECL(scope,type,name) \
  namespace scope { \
    class name##_class : public PROP<type> { public : name##_class() : PROP<type>(#scope,#type,#name) {} }; \
    extern name##_class name; \
  }

// Script for instantiating a derived PROP class
#define PROPERTY_INST(scope,type,name) \
  namespace scope { \
    name##_class name; \
  }

// Templated base class to define type of acc/dec                                                                  
template <typename T>
class PROP {

public:
  
  // constructor
  PROP(const std::string & _scope, const std::string & _type, const std::string & _name) 
    : scope(_scope), type(_type), name(_name), acc(nullptr), dec(nullptr)
  {
    
    // disable bool (gives compile time error)
    static_assert( ! std::is_same<T, bool>::value , "Don't use bool decorations!!!" );

    // initialise pointers
    try {
      dec = new SG::AuxElement::Decorator<T>( name );
      acc = new SG::AuxElement::Accessor <T>( name );
    } catch (...) {
      delete dec;
      throw;
    }

  }

  // destructor
  virtual ~PROP()
  {
    delete dec;
    delete acc;
    acc = nullptr;
    dec = nullptr;
  }
  
  // utility methods
  void set    (const SG::AuxElement * object, const T & value);
  T    get    (const SG::AuxElement * object);
  bool get    (const SG::AuxElement * object, T & value);
  void copy   (const SG::AuxElement * objectIn, SG::AuxElement * objectOut);
  bool exists (const SG::AuxElement * object);
  bool copyIfExists (const SG::AuxElement * objectIn, SG::AuxElement * objectOut);

  // meta data getters
  const std::string & Scope () const { return scope ; } 
  const std::string & Type  () const { return type  ; } 
  const std::string & Name  () const { return name  ; } 
  

private:
  
  // namespace, type and name of property
  const std::string scope;
  const std::string type;
  const std::string name;

  // accessor and decorator pointers
  SG::AuxElement::Accessor<T>  * acc;
  SG::AuxElement::Decorator<T> * dec;

};

#include "CxAODTools/ObjectDecorator.icc"


#endif // __MAKECINT__

#endif //CxAODMaker_ObjectDecorator_H
