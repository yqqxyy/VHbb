/* Dear emacs, this is -*-c++-*- */
#ifndef _TMVATrainingTool_H_
#define _TMVATrainingTool_H_

#include "OvCompat.h"

#include <TFile.h>
#ifndef _OvCompat_H_
#include <VerboseTool.h>
#endif
#include "ITMVATrainingTool.h"
#include <TMVA/Factory.h>
#include <TMVA/DataLoader.h>
#include <TMVA/Types.h>

#ifndef _OvCompat_H_
#include <cppstdcompat.h>
#endif
//using namespace boost::assign;

class TH1;
class TRandom3;


class TMVATrainingTool : virtual public ReflectionLayer<ITMVATrainingTool>, public VerboseTool
{

public:

   TMVATrainingTool(const std::string &type_name, const std::string &name, IAnalysisMain */*analysis_main*/);
  ~TMVATrainingTool();

   void initialize(IAnalysisMain *analysis_main);
   void finalize(IAnalysisMain *analysis_main);

   void addEvent(std::vector<double>& vars,
                 double weight,
                 TMVA::Types::ESBType eventClass = TMVA::Types::kSignal, TMVA::Types::ETreeType eventType = TMVA::Types::kMaxTreeType) const;
   void addEvent(int eventNumber,
		   	   	   std::vector<double>& vars,
                   double weight,
                   TMVA::Types::ESBType eventClass = TMVA::Types::kSignal, TMVA::Types::ETreeType eventType = TMVA::Types::kMaxTreeType) const;


   
   unsigned int getNVars() const;
   unsigned int getNTargets() const;
   unsigned int getNCategoryMarkers() const;
   unsigned int getNCategories() const;
   const std::vector<std::string>& getVariableList() const{
      return m_varList;
   }

private:

   //helper class to handle ROOT directory changes
   class RestoreGFile
   {
   public:
      RestoreGFile()
         : m_file(gFile),
           m_dir(gDirectory)
      {}
      ~RestoreGFile() {
         gFile=m_file;
         gDirectory=m_dir;
      }
      
   private:
      TFile *m_file;
      TDirectory *m_dir;
   };

   void BookMethods();
   
   //temporary members for debugging
   std::string m_histogramFolderName;
   TH1 *eventCountAdded;
   TH1 *eventCountWritten;

   std::unique_ptr<TMVA::DataLoader> m_dataLoader;
   std::unique_ptr<TMVA::Factory> m_factory;
   std::string m_factoryOptions;
   std::string m_treeOptions;
   std::string m_outputFileName;
   std::vector<std::string> m_varList;
   std::vector<std::string> m_targetList;
   std::vector<std::string> m_categoryMarkerList;
   std::vector<std::string> m_categories;
   std::vector<std::string> m_categoryVarList;
   std::vector<std::string> m_methodOptionList;
   std::vector<std::string> m_methodNameList;
   std::vector<std::string> m_methodTypeList;
   bool m_doTraining;
   bool m_isDisabled;
   bool m_writeAllEventsToTrainingTree;
   float m_testFraction;
   mutable bool m_isFirstEvent;
   mutable bool m_hasTrainingEvents;
   mutable bool m_hasTestEvents;
   TMVA::Types::EAnalysisType m_analysisType;
   
   static std::map<std::string, TMVA::Types::EMVA> m_methodTypeMap;
   float m_minEventWeight;

   std::unique_ptr<TFile> m_outputFile;
   TFile * m_outputFileUse;
   std::unique_ptr<TRandom3> m_rand;
};

#endif
