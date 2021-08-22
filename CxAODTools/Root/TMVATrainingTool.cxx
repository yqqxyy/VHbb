#include "CxAODTools/TMVATrainingTool.h"

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
#include <TFile.h>
#include <TRandom3.h>
#include <TMVA/ResultsRegression.h>
#include <TMVA/ResultsClassification.h>
#include <TMVA/MethodCategory.h>

KitRegistrar< GlobalToolKit<TMVATrainingTool> > s_TMVATrainingTool("TMVATrainingTool");

std::map<std::string, TMVA::Types::EMVA> TMVATrainingTool::m_methodTypeMap;

TMVATrainingTool::TMVATrainingTool(const std::string &type_name, const std::string &name, IAnalysisMain *analysis_main)
  : VerboseTool(type_name, name, analysis_main),
     m_isFirstEvent(true),
     m_hasTrainingEvents(false),
     m_hasTestEvents(false),
     m_analysisType( TMVA::Types::kClassification)
{
   m_config.registerConfigValue("FactoryConfiguration",
                                "The configuration string for the TMVA factory.",
                                m_factoryOptions="!V:Silent:Transformations=I:!Color:!DrawProgressBar");
   m_config.registerConfigValue("TrainingDataConfiguration",
                                "The configuration string for the training data processing.",
                                m_treeOptions="SplitMode=Block:NormMode=None:!V");
   m_config.registerConfigValue("OutputFileName",
                                "The TMVA output file name (and path).",
                                m_outputFileName="TMVATrainingOutput.root");
   m_config.registerConfigValue("InputVarNames",
                                "List with names of the TMVA input variables.",
                                m_varList);
   m_config.registerConfigValue("TargetNames",
                                "List with names of the regression targets.",
                                m_targetList);
   m_config.registerConfigValue("CategoryMarkerNames",
                                "List with names of the category markers.",
                                m_categoryMarkerList);
   m_config.registerConfigValue("Categories",
                                "List with formulas which define the categories.",
                                m_categories);
   m_config.registerConfigValue("CategoryVarNames",
                                "List which defines which variables should be used for a certain category. Format must be var1:var2:var3 etc.",
                                m_categoryVarList);
   m_config.registerConfigValue("MethodTypes",
                                "Types of methods to be trained. Choices are: Likelihood, PDERS, PDEFoam, KNN, LD, MLP, SVM,  BDT, RuleFit.",
                                m_methodTypeList);
   m_config.registerConfigValue("MethodNames",
                                "User defined names under which the methods will be know to TMVA in later applications. Names must be unique.",
                                m_methodNameList);
   m_config.registerConfigValue("MethodOptions",
                                "List of options for the methods to be trained. Order must be equal to the method order",
                                m_methodOptionList);
   m_config.registerConfigValue("DoTraining",
                                "Do Training within Overkill",
                                m_doTraining=false);
   m_config.registerConfigValue("IsDisabled",
                                "Disable tool (useful if training/application are part of the same analysis chain)",
                                m_isDisabled=false);
   m_config.registerConfigValue("WriteAllEventsToTrainingTree",
                                "Do not split events in training/testing but write everything to one tree",
                                m_writeAllEventsToTrainingTree=false);
   m_config.registerConfigValue("TestFraction",
                                "Fraction of events that will be used for testing",
                                m_testFraction=0.3);
   m_config.registerConfigValue("HistogramFolder",
                                "The name of the histogram folder.",
                                m_histogramFolderName="/TMVATraining");
   m_config.registerConfigValue("MinEventWeight","Only write events if the event weight is larger than this value.", m_minEventWeight=-std::numeric_limits<float>::max());
   if(m_methodTypeMap.empty()){
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("Likelihood", TMVA::Types::kLikelihood));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("PDERS", TMVA::Types::kPDERS));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("PDEFoam", TMVA::Types::kPDEFoam));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("KNN", TMVA::Types::kKNN));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("LD", TMVA::Types::kLD));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("MLP", TMVA::Types::kMLP));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("SVM", TMVA::Types::kSVM));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("BDT", TMVA::Types::kBDT));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("RuleFit", TMVA::Types::kRuleFit));
	   TMVATrainingTool::m_methodTypeMap.insert(std::make_pair("Category", TMVA::Types::kCategory));
   }

}

//_______________________________________________________________________

TMVATrainingTool::~TMVATrainingTool() {

}
//_______________________________________________________________________


unsigned int TMVATrainingTool::getNVars() const{
   return m_varList.size();
}

unsigned int TMVATrainingTool::getNTargets() const{
   return m_targetList.size();
}

unsigned int TMVATrainingTool::getNCategoryMarkers() const{
   return m_categoryMarkerList.size();
}

unsigned int TMVATrainingTool::getNCategories() const{
   return m_categories.size();
}

//_______________________________________________________________________

void TMVATrainingTool::initialize(IAnalysisMain *analysis_main)
{
#ifdef _OvCompat_H_
  UNUSED(analysis_main);
#endif
   m_outputFile.reset();
   m_dataLoader.reset();
   m_factory.reset();

   if(m_isDisabled)
      return;
   
   RestoreGFile restore;
#ifdef _OvCompat_H_
   if (!analysis_main) {
      throw RuntimeIssue(NTA_MAKE_MESSAGE_STR(NTA::Message::kFATAL, "TMVATrainingTool",101, "No analysis context"));
   }
   std::pair<TFile *,bool> ret( getFile( *analysis_main, m_outputFileName.c_str()) );
   m_outputFileUse = ret.first;
   if (ret.second) {
     m_outputFile.reset( m_outputFileUse );
   }
   NTA_MSG_DEBUG_STR(msgSvc(), msgLvl(), name(), 100, "Tried to create output file  " << m_outputFileName
                       << " got " << (m_outputFileUse ? m_outputFileUse->GetName() : "[nothing]"));
#else
   {
     std::unique_ptr<TFile> temp( new TFile( m_outputFileName.c_str(), "RECREATE" ) );
     m_outputFile = std::move(temp);
     m_outputFileUse = m_outputFile;
     NTA_MSG_DEBUG_STR(msgSvc(), msgLvl(), name(), 100, "Created training tree file " << m_outputFileName
                       << " ptr=" << static_cast<void *>(m_outputFile.get()));
   }
#endif

   {
     std::unique_ptr<TMVA::DataLoader> temp_dataLoader(new TMVA::DataLoader("OverkillDataLoadeOverkillDataLoader"));
     m_dataLoader=std::move(temp_dataLoader);
     std::unique_ptr<TMVA::Factory> temp( new TMVA::Factory("TMVAOverkill", m_outputFileUse, m_factoryOptions) );
     m_factory = std::move(temp);
   }

   if(m_varList.size() == 0)
      throw RuntimeIssue(NTA_MAKE_MESSAGE_STR(NTA::Message::kFATAL, "TMVATrainingTool",110, "No variables booked for TMVA training!"));
      
   std::set<std::string> tempset(m_varList.begin(), m_varList.end());
   if (m_varList.size() > tempset.size()) {
      std::sort(m_varList.begin(), m_varList.end());
      std::string last;
      std::stringstream msg;
      for (std::vector<std::string>::const_iterator it = m_varList.begin(); it != m_varList.end(); ++it) {
         if (last == *it) {
            msg << "\t" << *it;
         }
         last = *it;
      }
      NTA_MSG_WARNING_STR(msgSvc(), msgLvl(), name(), 120, "There are duplicate variables in the list of variables. They are being removed for training now. List of duplicates:\n" << msg.str());

      m_varList.assign(tempset.begin(), tempset.end());
   }
   for(std::vector<std::string>::const_iterator varIter = m_varList.begin(); varIter != m_varList.end(); ++varIter){
      m_dataLoader->AddVariable((*varIter), 'F');
   }

   std::vector<std::string>::const_iterator targetIter = m_targetList.begin();
   for(; targetIter != m_targetList.end(); ++targetIter){
      m_dataLoader->AddTarget((*targetIter));
      m_analysisType =  TMVA::Types::kRegression;
   }
   std::vector<std::string>::const_iterator markerIter = m_categoryMarkerList.begin();
   for(; markerIter != m_categoryMarkerList.end(); ++markerIter){
      m_dataLoader->AddSpectator((*markerIter), 'F');
   }
    
   {
     // @TODO use pseudo random number tool, so that one can customize the seeding ?
     std::unique_ptr<TRandom3> temp(new TRandom3(0));
     m_rand = std::move(temp); 
   }

   //for debugging
   
#ifndef _OvCompat_H_
   eventCountAdded = analysis_main->histoSvc()->createHistogram(m_histogramFolderName.c_str(),"EventCountAdded","EventCountAdded",Binning_t(2,-0.5,1.5));
   eventCountWritten = analysis_main->histoSvc()->createHistogram(m_histogramFolderName.c_str(),"EventCountWritten","EventCountWritten",Binning_t(2,-0.5,1.5));
#endif
   
}

//_______________________________________________________________________

void TMVATrainingTool::finalize(IAnalysisMain */*analysis_main*/) 
{
   if(m_isDisabled)
      return;

   RestoreGFile restore;
   m_outputFileUse->cd();

   if( m_hasTrainingEvents){
      m_dataLoader->PrepareTrainingAndTestTree("", m_treeOptions );
      
      if(m_doTraining){
         BookMethods();
         m_factory->TrainAllMethods();
         m_factory->TestAllMethods();
         m_factory->EvaluateAllMethods();
      }
      else{
         int dummy_regval = 0;
         
         //some clumsy workarounds to build dummy results that make TMVA think it has
         //been trained. Writing the trees will not work otherwise...
         
         //get number of events in training tree
         int nevents_train = m_dataLoader->AddDataSet("Default").GetDataSet()->GetNEvents(TMVA::Types::kTraining);
         //create dummy results object for training tree
         TMVA::Results* results_train =
            m_dataLoader->AddDataSet("Default").GetDataSet()->GetResults("Dummy",TMVA::Types::kTraining, m_analysisType);
         
         //fill dummy results for training
         if(m_analysisType == TMVA::Types::kRegression){
            //fill dummy results for regression
            for(int i=0; i<nevents_train; ++i){
               std::vector< Float_t > dummyResults(getNTargets(), dummy_regval);
               dynamic_cast<TMVA::ResultsRegression*>(results_train)->SetValue(dummyResults, i );
            }
         }
         else{
            dynamic_cast<TMVA::ResultsClassification*>(results_train)->Resize(nevents_train);
         }
         
         if(nevents_train != 0){
            //write the prepared training tree to output file
            //START DEBUG
            //float entries = m_dataLoader->AddDataSet("Default").GetDataSet()->GetTree(TMVA::Types::kTraining)->GetEntries();
#ifndef _OvCompat_H_
            eventCountWritten->Fill(0.,nevents_train);
#endif
            //std::cout << entries << std::endl;
            //END DEBUG
            m_dataLoader->AddDataSet("Default").GetDataSet()->GetTree(TMVA::Types::kTraining)->Write( "", TObject::kOverwrite );
         }
         
         if(!m_writeAllEventsToTrainingTree){
            //the test tree is only written if it was filled
            if( m_hasTestEvents){
               //get number of events in testing tree
               int nevents_test = m_dataLoader->AddDataSet("Default").GetDataSet()->GetNEvents(TMVA::Types::kTesting);
               //create dummy results object for testing tree
               TMVA::Results* results_test =
                  m_dataLoader->AddDataSet("Default").GetDataSet()->GetResults("Dummy",TMVA::Types::kTesting, m_analysisType);
               
               //fill dummy results for testing
               if(m_analysisType == TMVA::Types::kRegression){
                  //fill dummy results for regression
                  for(int i=0; i<nevents_test; ++i){
                     std::vector< Float_t > dummyResults(getNTargets(), dummy_regval);
                     dynamic_cast<TMVA::ResultsRegression*>(results_test)->SetValue(dummyResults, i );
                  }
               }
               else{
                  dynamic_cast<TMVA::ResultsClassification*>(results_test)->Resize(nevents_test);
               }
               
               if(nevents_test != 0){
                  //write the prepared test tree to output file
                  //START DEBUG
                  //float entries = m_dataLoader->AddDataSet("Default").GetDataSet()->GetTree(TMVA::Types::kTesting)->GetEntries();
                  //eventCountWritten->Fill(1.,entries);
                  //END DEBUG
                  m_dataLoader->AddDataSet("Default").GetDataSet()->GetTree(TMVA::Types::kTesting)->Write( "", TObject::kOverwrite );
               }
            }
         }
      }
   }
   m_factory.reset();
   m_dataLoader.reset();
   if (m_outputFile.get()) {
     m_outputFile->Close();
     m_outputFile.reset();
   }
}

//_______________________________________________________________________

void TMVATrainingTool::addEvent(std::vector<double>& vars,
                                double weight,
                                TMVA::Types::ESBType eventClass,
                                TMVA::Types::ETreeType eventType) const{
  if (m_isDisabled || weight<=m_minEventWeight) return;
   
   RestoreGFile restore;
   m_outputFileUse->cd();

   if(m_writeAllEventsToTrainingTree){
      m_dataLoader->AddEvent("Signal", TMVA::Types::kTraining, vars, weight);
#ifndef _OvCompat_H_
      eventCountAdded->Fill(0); //debugging
#endif
      if(m_isFirstEvent){
         //duplicate first event in test tree to avoid "empty tree" exception
         m_dataLoader->AddEvent("Signal", TMVA::Types::kTesting, vars, weight);
         //eventCountAdded->Fill(1); //debugging
         m_hasTrainingEvents = true;
         m_hasTestEvents = true;
         m_isFirstEvent = false;
      }
      return;
   }
   else{
	   if(eventType == TMVA::Types::kMaxTreeType){
		   if(m_rand->Rndm()>m_testFraction){
			   eventType =  TMVA::Types::kTraining;
			   m_hasTrainingEvents = true;
		   }
		   else{
			   eventType =  TMVA::Types::kTesting;
			   m_hasTestEvents = true;
		   }
	   }
	   else if(eventType == TMVA::Types::kTraining){
		   m_hasTrainingEvents = true;
	   }
	   else if(eventType == TMVA::Types::kTesting){
		   m_hasTestEvents = true;
	   }
   }
   
   if(m_analysisType == TMVA::Types::kRegression){
      m_dataLoader->AddEvent("Regression", eventType, vars, weight);
   }
   else{
      if(eventClass == TMVA::Types::kSignal){
         m_dataLoader->AddEvent("Signal", eventType, vars, weight);
      }
      else{
         m_dataLoader->AddEvent("Background", eventType, vars, weight);
      }
   }
}

void TMVATrainingTool::addEvent(int eventNumber,
		std::vector<double>& vars,
		double weight,
		TMVA::Types::ESBType eventClass,
        TMVA::Types::ETreeType eventType) const{
	if (m_isDisabled || weight<=m_minEventWeight) return;

	RestoreGFile restore;
	m_outputFileUse->cd();

	if(eventType == TMVA::Types::kMaxTreeType){

		if(eventNumber%2 == 0){
			eventType =  TMVA::Types::kTraining;
			m_hasTrainingEvents = true;
		}
		else{
			eventType =  TMVA::Types::kTesting;
			m_hasTestEvents = true;
		}
	}

	 if(m_analysisType == TMVA::Types::kRegression){
	      m_dataLoader->AddEvent("Regression", eventType, vars, weight);
	   }
	   else{
	      if(eventClass == TMVA::Types::kSignal){
	         m_dataLoader->AddEvent("Signal", eventType, vars, weight);
	      }
	      else{
	         m_dataLoader->AddEvent("Background", eventType, vars, weight);
	      }
	   }
}

//_______________________________________________________________________

void TMVATrainingTool::BookMethods ()
{
   assert(m_methodNameList.size()==m_methodTypeList.size());
   assert(m_methodNameList.size()==m_methodOptionList.size());
   for(unsigned int i = 0; i< m_methodNameList.size(); ++i){
      if(m_methodTypeMap.find(m_methodTypeList[i])->second == TMVA::Types::kCategory){
         TMVA::MethodBase* mbase = m_factory->BookMethod( m_dataLoader.get(), TMVA::Types::kCategory, m_methodNameList[i],"");
         TMVA::MethodCategory* mcat = dynamic_cast<TMVA::MethodCategory*>(mbase);
         for(unsigned int j = 0; j<m_categories.size(); ++j){
            ++i;
            mcat->AddMethod(m_categories[j].c_str(), m_categoryVarList[j], m_methodTypeMap.find(m_methodTypeList[i])->second, m_methodNameList[i], m_methodOptionList[i]);
         }
      }
      else{
         m_factory->BookMethod( m_dataLoader.get(),m_methodTypeMap.find(m_methodTypeList[i])->second, m_methodNameList[i],
                                m_methodOptionList[i]);
      }
   }
}
