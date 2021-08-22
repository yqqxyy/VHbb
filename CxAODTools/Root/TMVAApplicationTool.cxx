#include "CxAODTools/TMVAApplicationTool.h"
#include "TSystem.h"

#ifndef _OvCompat_H_
#include <IAnalysisMain.h>
#include <TreeUtil.h>
#include <MessageUtil.h>
#endif

#include <algorithm>
#ifndef _OvCompat_H_
#include <cppstdcompat.h>
#endif
#include <limits>

#ifndef _OvCompat_H_
#include <ToolKit.h>
#include <KitRegistrar.h>
#endif

#ifndef _OvCompat_H_
#include <IHistogramService.h>
#endif
#include <TMVA/ResultsRegression.h>
#include <TMVA/ResultsClassification.h>

#include "CxAODTools/GetTMVAVars.h"
#ifndef _OvCompat_H_
#include <FileUtils.h>
#include <Issue.h>
#include <MessageUtil.h>
#endif

KitRegistrar< GlobalToolKit<TMVAApplicationTool> > s_TMVAApplicationTool("TMVAApplicationTool");

TMVAApplicationTool::TMVAApplicationTool(const std::string &type_name, const std::string &name, IAnalysisMain *analysis_main)
  : VerboseTool(type_name, name, analysis_main),
    m_eventInfo(*this)
{
   m_config.registerConfigValue("ReaderConfiguration",
                                "The configuration string for the TMVA reader.",
                                m_readerOptions="!Color:!Silent");
   m_config.registerConfigValue("InputVarNames",
                                "List with names of the TMVA input variables.",
                                m_varList);
   m_config.registerConfigValue("CategoryMarkerNames",
                                "List with names of the category markers.",
                                m_categoryMarkerList);
   m_config.registerConfigValue("Methods",
                                "List of methods to be evaluated.",
                                m_methodList);
   m_config.registerConfigValue("WeightFiles",
                                "List of weightfile paths. Order must be equal to the method order. Use 'default' to use TMVA default weightfile location/name.",
                                m_weightfileList);
   m_config.registerConfigValue("NumPartitions",
                                "Number of partitions in which the data will be divided for application. It is assumed that a corresponding number of trained classifiers exists.",
                                m_numPartitions = 1);
   m_config.registerConfigValue("IsDisabled",
                                "Disable tool (useful if training/application are part of the same analysis chain)",
                                m_isDisabled=false);
   m_config.registerConfigValue("CommonWeightfilePath", "If not empty, each weightfile name will be prefixed with this path.", m_commonWeightfilePath);
   m_config.registerConfigValue("doEvenOdd",
                                "Do the application of even training to odd events and vice versa",
                                m_doEvenOdd = false);
}

//_______________________________________________________________________

TMVAApplicationTool::~TMVAApplicationTool() {

}

unsigned int TMVAApplicationTool::getNVars() const{
   return m_varList.size();
}

unsigned int TMVAApplicationTool::getNCategoryMarkers() const{
   return m_categoryMarkerList.size();
}

unsigned int TMVAApplicationTool::getNMethods() const{
   return m_methodList.size();
}

//_______________________________________________________________________

void TMVAApplicationTool::initialize(IAnalysisMain */*analysis_main*/)
{
    if(m_isDisabled)
      return;

    if (!m_reader.get() 
        || m_lastReaderOptions != m_readerOptions 
        || m_lastVarList != m_varList 
        || m_lastCategoryMarkerList != m_categoryMarkerList
        || m_lastMethodList != m_methodList            
        || m_lastWeightfileList !=m_weightfileList) {

    std::unique_ptr<TMVA::Reader> temp_ptr( new TMVA::Reader(m_readerOptions) );
    m_reader = std::move(temp_ptr);

    m_tempEvent.clear();
//    m_tempEvent.resize(m_varList.size() + m_categoryMarkerList.size());

    if  (!m_weightfileList.empty()) {
       std::cout << "INFO [TMVAApplicationTool::initialize] " << name() << " " << (m_weightfileList.begin())->c_str() << std::endl;

       if(m_weightfileList.size() == 0){
           m_weightfileList.reserve(m_methodList.size());
           for(unsigned int i = 0; i< m_methodList.size(); ++i){
                 m_weightfileList.push_back("default");
           }
        }
        else
           checkDimensions(m_methodList, m_weightfileList);

       if(m_doEvenOdd) m_numPartitions = 2;

        std::string weightfile;
        for(unsigned int i = 0; i< m_methodList.size(); ++i){
           for(unsigned int ipart = 0; ipart<m_numPartitions; ++ipart){
              std::string methodName = m_methodList[i] + "_" + StringFromInt(ipart).Data();
              std::string default_weightfile =  "weights/TMVAOverkill_" + m_methodList[i] + ".weights.xml";
              weightfile = (m_weightfileList[i]=="default" ? default_weightfile : m_weightfileList[i]);
		weightfile = gSystem->ExpandPathName( weightfile.c_str() );

              if (!m_commonWeightfilePath.empty())
                 weightfile = m_commonWeightfilePath + weightfile;

              unsigned int pos = weightfile.find(".weights");
              if(m_doEvenOdd) weightfile.insert(pos,TString("_") + StringFromInt(ipart).Data());

              //weightfile.insert(pos,TString("_") + StringFromInt(ipart).Data());
              break;
           }
           break;
        }

       GetTMVAVars get_vars( weightfile.c_str() );
       if (!get_vars) {
          throw RuntimeIssue(NTA_MAKE_MESSAGE_STR(NTA::Message::kFATAL, name(),395, "Inconsistent variable definitions in weight file : " << *(m_weightfileList.begin()) <<" ."));
       }
       get_vars.dump();
       m_varList = get_vars.variables(); 
       m_tempEvent.resize(m_varList.size() + m_categoryMarkerList.size());

       for (std::vector<std::string>::const_iterator iter = m_varList.begin();
            iter != m_varList.end();
            ++iter) {
         
         unsigned int index = (iter - m_varList.begin());
         m_reader->AddVariable(*iter,  &(m_tempEvent[index]));
       }
    }
  //  for(unsigned int i = 0; i< m_varList.size(); ++i){
  //     m_reader->AddVariable(m_varList[i], &(m_tempEvent[i]));
  //   }
   for(unsigned int i = 0; i< m_categoryMarkerList.size(); ++i){
      m_reader->AddSpectator(m_categoryMarkerList[i], &(m_tempEvent[m_varList.size()+i]));
   }
   
   BookMethods();

   m_lastReaderOptions = m_readerOptions;
   m_lastVarList = m_varList;
   m_lastCategoryMarkerList = m_categoryMarkerList;
   m_lastMethodList = m_methodList;
   m_lastWeightfileList=m_weightfileList;
   }
}

//_______________________________________________________________________

void TMVAApplicationTool::finalize(IAnalysisMain */*analysis_main*/) 
{   
   if(m_isDisabled)
      return;
   
   //   m_reader.reset();
}

//_______________________________________________________________________

std::vector<std::vector<float> > TMVAApplicationTool::evaluateRegression(std::vector<double>& vars) const{
  
   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }
   
   std::vector<std::vector<float> > result;
   result.reserve(m_methodList.size());

   std::vector<std::string>::const_iterator methodIter = m_methodList.begin();
   for(; methodIter != m_methodList.end(); ++methodIter){
      unsigned int part = m_eventInfo.eventNumber()%m_numPartitions;
      std::string methodName = (*methodIter) + "_" + StringFromInt(part).Data();
      result.push_back(m_reader->EvaluateRegression(methodName));
   }

   return result;
}

std::vector<std::vector<float> > TMVAApplicationTool::evaluateRegression(int eventnumber, std::vector<double>& vars) const{

   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }

   std::vector<std::vector<float> > result;
   result.reserve(m_methodList.size());

   std::vector<std::string>::const_iterator methodIter = m_methodList.begin();
   for(; methodIter != m_methodList.end(); ++methodIter){
      unsigned int part = eventnumber%m_numPartitions;

      std::string methodName = (*methodIter) + "_" + StringFromInt(part).Data();
      //std::cout << "method name: " << methodName << std::endl;
      result.push_back(m_reader->EvaluateRegression(methodName));
   }

   return result;
}
//_______________________________________________________________________

std::vector<float> TMVAApplicationTool::evaluateClassification(std::vector<double>& vars) const{
  
   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }
   
   std::vector<float> result;
   result.reserve(m_methodList.size());

   std::vector<std::string>::const_iterator methodIter = m_methodList.begin();
   for(; methodIter != m_methodList.end(); ++methodIter){
      unsigned int part = m_eventInfo.eventNumber()%m_numPartitions;
      std::string methodName = (*methodIter) + "_" + StringFromInt(part).Data();
      result.push_back(m_reader->EvaluateMVA(methodName));    
   }

   return result;
}

//_______________________________________________________________________

float TMVAApplicationTool::evaluateClassification(std::vector<double>& vars, const std::string& methodName) const{
  
   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }
   
   //check if method with name "methodName" is recognized...
   if(find (m_methodList.begin(), m_methodList.end(), methodName) != m_methodList.end()){
      unsigned int part = m_eventInfo.eventNumber()%m_numPartitions;
      std::string name = methodName + "_" + StringFromInt(part).Data();
      return m_reader->EvaluateMVA(name);
   }
   else{
      throw RuntimeIssue(NTA_MAKE_MESSAGE_STR(NTA::Message::kFATAL, "TMVAApplicationTool",400, "Method \"" << methodName << " \" is not known to the TMVAApplicationTool."));
   }
}


//_______________________________________________________________________

std::vector<std::vector<float> > TMVAApplicationTool::evaluateMulticlass(std::vector<double>& vars) const{
  
   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }
   
   std::vector<std::vector<float> > result;
   result.reserve(m_methodList.size());

   std::vector<std::string>::const_iterator methodIter = m_methodList.begin();
   for(; methodIter != m_methodList.end(); ++methodIter){
      unsigned int part = m_eventInfo.eventNumber()%m_numPartitions;
      std::string methodName = (*methodIter) + "_" + StringFromInt(part).Data();  
      result.push_back(m_reader->EvaluateMulticlass(methodName));
   }

   return result;
}

//_______________________________________________________________________

std::vector<float> TMVAApplicationTool::evaluateMulticlass(std::vector<double>& vars, unsigned int cls) const{
  
   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }
   
   std::vector<float> result;
   result.reserve(m_methodList.size());

   std::vector<std::string>::const_iterator methodIter = m_methodList.begin();
   for(; methodIter != m_methodList.end(); ++methodIter){
      unsigned int part = m_eventInfo.eventNumber()%m_numPartitions;
      std::string methodName = (*methodIter) + "_" + StringFromInt(part).Data();
      result.push_back(m_reader->EvaluateMulticlass(cls, methodName));
   }

   return result;
}

//_______________________________________________________________________

std::vector<float> TMVAApplicationTool::getSignalProbability(std::vector<double>& vars, double expectedSignalFraction) const{
  
   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }
   
   std::vector<float> result;
   result.reserve(m_methodList.size());

   std::vector<std::string>::const_iterator methodIter = m_methodList.begin();
   for(; methodIter != m_methodList.end(); ++methodIter){
      unsigned int part = m_eventInfo.eventNumber()%m_numPartitions;
      std::string methodName = (*methodIter) + "_" + StringFromInt(part).Data();
      result.push_back(m_reader->GetProba(methodName, expectedSignalFraction));    
   }

   return result;
}

//_______________________________________________________________________

std::vector<float> TMVAApplicationTool::getBackgroundRarity(std::vector<double>& vars) const{
  
   assert(m_tempEvent.size() == vars.size());
   for(unsigned int i = 0; i< vars.size(); ++i){
      m_tempEvent[i]=vars[i];
   }
   
   std::vector<float> result;
   result.reserve(m_methodList.size());

   std::vector<std::string>::const_iterator methodIter = m_methodList.begin();
   for(; methodIter != m_methodList.end(); ++methodIter){
      unsigned int part = m_eventInfo.eventNumber()%m_numPartitions;
      std::string methodName = (*methodIter) + "_" + StringFromInt(part).Data();
      result.push_back(m_reader->GetRarity(methodName));    
   }

   return result;
}

//_______________________________________________________________________

void TMVAApplicationTool::BookMethods()
{
   if(m_weightfileList.size() == 0){
      m_weightfileList.reserve(m_methodList.size());
      for(unsigned int i = 0; i< m_methodList.size(); ++i){
         m_weightfileList.push_back("default");
      }
   }    
   else
     checkDimensions(m_methodList, m_weightfileList);
   
   for(unsigned int i = 0; i< m_methodList.size(); ++i){
      for(unsigned int ipart = 0; ipart<m_numPartitions; ++ipart){
         std::string methodName = m_methodList[i] + "_" + StringFromInt(ipart).Data();
         std::string default_weightfile =  "weights/TMVAOverkill_" + m_methodList[i] + ".weights.xml";
         std::string weightfile = (m_weightfileList[i]=="default" ? default_weightfile : m_weightfileList[i]);
	weightfile = gSystem->ExpandPathName( weightfile.c_str() );

         if (!m_commonWeightfilePath.empty())
            weightfile = m_commonWeightfilePath + weightfile;

         unsigned int pos = weightfile.find(".weights");
         if(m_doEvenOdd) weightfile.insert(pos,TString("_") + StringFromInt(ipart).Data());
         { 
            std::ifstream in(weightfile.c_str());
            if (!in) {
               throw RuntimeIssue(NTA_MAKE_MESSAGE_STR(NTA::Message::kFATAL, "TMVAApplicationTool",400, "Weight file  \"" << weightfile << "\" for method \"" << methodName << "\" does not exist."));
            }
         }
         m_reader->BookMVA(methodName, weightfile);
      }
   }
}
