#include <CxAODTools/ConfigFieldBase.h>
#include <CxAODTools/ConfigField.h>
#include <CxAODTools/ConfigStore.h>
#include <CxAODTools/OverlapRegister.h>

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass;

#endif

#ifdef __CINT__

#pragma link C++ class ConfigFieldBase+;
#pragma link C++ class ConfigField<bool>+;
#pragma link C++ class ConfigField<int>+;
#pragma link C++ class ConfigField<float>+;
#pragma link C++ class ConfigField<double>+;
#pragma link C++ class ConfigField<std::string>+;
#pragma link C++ class ConfigField<std::vector<bool> >+;
#pragma link C++ class ConfigField<std::vector<int> >+;
#pragma link C++ class ConfigField<std::vector<float> >+;
#pragma link C++ class ConfigField<std::vector<double> >+;
#pragma link C++ class ConfigField<std::vector<std::string> >+;
#pragma link C++ class ConfigStore+;

#endif
