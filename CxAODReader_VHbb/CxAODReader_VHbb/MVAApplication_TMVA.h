// Dear emacs, this is -*-c++-*-
#ifndef CxAODReader_VHbb__MVAApplication_TMVA_H
#define CxAODReader_VHbb__MVAApplication_TMVA_H

#include "CxAODReader_VHbb/MVATree_VHbb.h"
#include "TLeaf.h"
#include "TMVA/Reader.h"

enum MVACategory {
  nTags2_nJ2_ptvMedium_even,
  nTags2_nJ2_ptvMedium_odd,
  nTags2_nJ2_ptvHigh_even,
  nTags2_nJ2_ptvHigh_odd,
  nTags2_nJ2_ptvVeryHigh_even,
  nTags2_nJ2_ptvVeryHigh_odd,
  nTags2_nJ3_ptvMedium_even,
  nTags2_nJ3_ptvMedium_odd,
  nTags2_nJ3_ptvHigh_even,
  nTags2_nJ3_ptvHigh_odd,
  nTags2_nJ3_ptvVeryHigh_even,
  nTags2_nJ3_ptvVeryHigh_odd,
  Unknown
};

// This can be extended to define multiple MVA models, e.g. new variables etc.
enum MVAType {
  ichepStyle,
  split250,
  ichepStyleIncl1LepMedium,
  split250Incl1LepMedium
};

class MVAApplication_TMVA {
 public:
  // Class Constructor
  MVAApplication_TMVA(std::string anaType, MVAType mvatype = ichepStyle,
                      std::string AppName = "App_TMVA");

  // Class Desconstructor
  ~MVAApplication_TMVA() = default;

  // Initialisation method
  //      -> Used to initialise the TMVA::Reader objects with weight files (xml)
  // 	  -> Also used to assign variables to the Reader objects using
  // MVATree_VHbb
  void Initialise(std::string xmlPath, MVATree_VHbb *tree,
                  std::string treeName = "Nominal");

  // Evaluation method
  //   	-> Used to evaluate per event the MVA algorithm
  float Evaluate(unsigned long long EventNumber, int nTags, int nJ, float pTV);

 protected:
  // assigns xml files to a category
  void initialiseXmlFiles(std::string xmlPath);
  // books a reader for each xml file
  void initialiseReaders();
  // adds variables to readers and books readers
  void addVariablesAndBook(MVATree_VHbb *tree,
                           std::string treeName = "Nominal");

  // get the MVA category from the event info
  MVACategory getMVACategory(unsigned long long EventNumber, int nTags, int nJ,
                             float pTV);

  // get input variables or spectators from xml file
  std::vector<std::string> getExpressions(std::string xmlFile,
                                          std::string type);
  // helper function to get the right variable expression from xml file
  std::string shorten(std::string exp);
  // helper function to split an expression
  std::vector<std::string> split(std::string str, std::string del);

  // member variables:
  std::string m_AppName;
  std::string m_anaType;
  MVAType m_mvatype;
  std::map<MVACategory, std::string> m_xmlFiles;
  std::map<MVACategory, TMVA::Reader *> m_readers;
  std::map<TString, float *> m_variables;
};

#endif
