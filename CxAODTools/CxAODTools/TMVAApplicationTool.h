/* Dear emacs, this is -*-c++-*- */
#ifndef _TMVAApplicationTool_H_
#define _TMVAApplicationTool_H_

#include "OvCompat.h"


#ifndef _OvCompat_H_
#include <VerboseTool.h>
#endif
#include "ITMVAApplicationTool.h"
#include <TMVA/Reader.h>
#ifndef _OvCompat_H_
#include "EventInfo_t.h"

#include <cppstdcompat.h>
#endif

class TH1;
class TRandom3;


class TMVAApplicationTool : virtual public ReflectionLayer<ITMVAApplicationTool>, public VerboseTool
{

public:

   TMVAApplicationTool(const std::string &type_name, const std::string &name, IAnalysisMain */*analysis_main*/);
  ~TMVAApplicationTool();

   void initialize(IAnalysisMain *analysis_main);
   void finalize(IAnalysisMain *analysis_main);

   std::vector<std::vector<float> > evaluateRegression(std::vector<double>& vars) const;
   std::vector<std::vector<float> > evaluateRegression(int eventnumber, std::vector<double>& vars) const;
   std::vector<float> evaluateClassification(std::vector<double>& vars) const;
   float evaluateClassification(std::vector<double>& vars, const std::string& methodName) const;
   std::vector<float> evaluateMulticlass(std::vector<double>& vars, unsigned int cls) const;
   std::vector<std::vector<float> > evaluateMulticlass(std::vector<double>& vars) const;
   std::vector<float> getSignalProbability(std::vector<double>& vars, double expectedSignalFraction = 0.5) const;
   std::vector<float> getBackgroundRarity(std::vector<double>& vars) const;
   
   unsigned int getNVars() const;
   unsigned int getNCategoryMarkers() const;
   unsigned int getNMethods() const;
   const std::vector<std::string>& getMethodList() const{
      return m_methodList;
   }
   const std::vector<std::string>& getVariableList() const{
      return m_varList;
   } 
   
private:

   TString StringFromInt( Long_t i ) const
   {
      std::stringstream s;
      s << i;
      return TString(s.str().c_str());
   }

   void BookMethods();

   EventInfo_t m_eventInfo;
   std::unique_ptr<TMVA::Reader> m_reader;
   std::string m_readerOptions;
   std::vector<std::string> m_varListDummy;
   mutable std::vector<std::string> m_varList;
   std::vector<std::string> m_categoryMarkerList;
   std::vector<std::string> m_weightfileList;
   std::vector<std::string> m_methodList;
   
   std::map<std::string, std::string> m_methodMap;
   bool m_isDisabled;
   mutable std::vector<float> m_tempEvent;

   bool m_doEvenOdd;
   unsigned int m_numPartitions;
   std::string m_lastReaderOptions;
   std::vector<std::string> m_lastVarList;
   std::vector<std::string> m_lastCategoryMarkerList;
   std::vector<std::string> m_lastMethodList;
   std::vector<std::string> m_lastWeightfileList;
   std::string m_commonWeightfilePath;
};

#endif
