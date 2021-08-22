// Dear enacs, this is -*-cx++-*-

// Class header includes
#include "CxAODReader_VHbb/MVAApplication_TMVA.h"

#include <dirent.h>
#include "TXMLEngine.h"

MVAApplication_TMVA ::MVAApplication_TMVA(std::string anaType, MVAType mvatype,
                                          std::string AppName) {
  Info("MVAApplication_TMVA ::MVAApplication_TMVA()",
       "Creating MVA evaluation object with name: %s", AppName.c_str());
  m_anaType = anaType;
  m_AppName = AppName;
  m_mvatype = mvatype;
  m_variables.clear();
  m_xmlFiles.clear();
  m_readers.clear();
}

void MVAApplication_TMVA::Initialise(std::string xmlPath, MVATree_VHbb *tree,
                                     std::string treeName) {
  // Expand WorkDir_DIR
  std::string workdir = getenv("WorkDir_DIR");
  if (xmlPath.find("$WorkDir_DIR") != std::string::npos) {
    xmlPath = xmlPath.replace(xmlPath.find_first_of("$WorkDir_DIR"),
                              strlen("$WorkDir_DIR"), workdir);
  }

  Info("MVAApplication_TMVA::Initialise()",
       "setting the path of BDT xml files to: %s", xmlPath.c_str());

  initialiseXmlFiles(xmlPath);
  initialiseReaders();
  addVariablesAndBook(tree, treeName);
}

float MVAApplication_TMVA::Evaluate(unsigned long long EventNumber, int nTags,
                                    int nJ, float pTV) {
  float score = -99;
  MVACategory eventCategory = getMVACategory(EventNumber, nTags, nJ, pTV);
  std::map<MVACategory, TMVA::Reader *>::iterator reader_it;
  reader_it = m_readers.find(eventCategory);

  if (reader_it != m_readers.end()) {
    score = reader_it->second->EvaluateMVA("BDT");
  }
  return score;
}

void MVAApplication_TMVA::addVariablesAndBook(MVATree_VHbb *tree,
                                              std::string treeName) {
  for (std::map<MVACategory, TMVA::Reader *>::iterator reader_it =
           m_readers.begin();
       reader_it != m_readers.end(); reader_it++) {
    MVACategory categ = reader_it->first;

    std::string xml = m_xmlFiles[categ];
    std::vector<std::string> expMVAinput = getExpressions(xml, "Variable");
    std::vector<std::string> expMVAspectator = getExpressions(xml, "Spectator");

    Info("MVAApplication_TMVA::addVariablesAndBook()",
         "booking MVA reader for file: %s", xml.c_str());

    TTree *dummyTree = tree->getTree(treeName);
    TObjArray *listOfLeaves = dummyTree->GetListOfLeaves();

    for (unsigned int invar_i = 0; invar_i < expMVAinput.size(); invar_i++) {
      std::string expVar = expMVAinput[invar_i];
      TString varName = TString(shorten(expVar));
      if (listOfLeaves->Contains(varName)) {
        if (m_variables.find(varName) == m_variables.end())
          m_variables[varName] =
              (float *)dummyTree->GetLeaf(varName)->GetValuePointer();
      } else {
        Error("MVAApplication_TMVA::addVariablesAndBook()",
              "No leaf with name '%s' in tree", varName.Data());
        exit(EXIT_FAILURE);
      }
      Info("MVAApplication_TMVA::addVariablesAndBook()", "setting variable: %s",
           varName.Data());
      m_readers[categ]->AddVariable(expVar, m_variables[varName]);
    }
    for (unsigned int spec_i = 0; spec_i < expMVAspectator.size(); spec_i++) {
      std::string expSpec = expMVAspectator[spec_i];
      TString specName = TString(shorten(expSpec));
      if (listOfLeaves->Contains(specName)) {
        if (m_variables.find(specName) == m_variables.end())
          m_variables[specName] =
              (float *)dummyTree->GetLeaf(specName)->GetValuePointer();
      } else {
        Error("MVAApplication_TMVA::addVariablesAndBook()",
              "No leaf with name '%s' in tree", specName.Data());
        exit(EXIT_FAILURE);
      }
      Info("MVAApplication_TMVA::addVariablesAndBook()",
           "setting spectator: %s", specName.Data());
      m_readers[categ]->AddSpectator(expSpec, m_variables[specName]);
    }
    m_readers[categ]->BookMVA("BDT", xml);
  }
}

void MVAApplication_TMVA ::initialiseReaders() {
  for (std::map<MVACategory, std::string>::iterator xml_it = m_xmlFiles.begin();
       xml_it != m_xmlFiles.end(); xml_it++) {
    m_readers[xml_it->first] = new TMVA::Reader("V:Color:!Silent");
  }
}

// weightfile "dump" (probably not the nicest solution -> ToDo: polish/clean-up)
void MVAApplication_TMVA::initialiseXmlFiles(std::string xmlPath) {
  if (m_mvatype == MVAType::ichepStyle ||
      m_mvatype == MVAType::ichepStyleIncl1LepMedium) {
    if (m_anaType == "0lep") {
      m_xmlFiles[nTags2_nJ2_ptvHigh_even] =
          xmlPath + "/TMVAClassification_BDT_0L_2J_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_0L_2J_150ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_even] =
          xmlPath + "/TMVAClassification_BDT_0L_3J_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_0L_3J_150ptv_2of2.weights.xml";
    }
    if (m_anaType == "1lep") {
      m_xmlFiles[nTags2_nJ2_ptvHigh_even] =
          xmlPath + "/TMVAClassification_BDT_1L_2J_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_1L_2J_150ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_even] =
          xmlPath + "/TMVAClassification_BDT_1L_3J_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_1L_3J_150ptv_2of2.weights.xml";
      if (m_mvatype == MVAType::ichepStyleIncl1LepMedium) {
        m_xmlFiles[nTags2_nJ2_ptvMedium_even] =
            xmlPath +
            "/TMVAClassification_BDT_1L_2J_75ptv_150ptv_1of2.weights.xml";
        m_xmlFiles[nTags2_nJ2_ptvMedium_odd] =
            xmlPath +
            "/TMVAClassification_BDT_1L_2J_75ptv_150ptv_2of2.weights.xml";
        m_xmlFiles[nTags2_nJ3_ptvMedium_even] =
            xmlPath +
            "/TMVAClassification_BDT_1L_3J_75ptv_150ptv_1of2.weights.xml";
        m_xmlFiles[nTags2_nJ3_ptvMedium_odd] =
            xmlPath +
            "/TMVAClassification_BDT_1L_3J_75ptv_150ptv_2of2.weights.xml";
      }
    }
    if (m_anaType == "2lep") {
      m_xmlFiles[nTags2_nJ2_ptvMedium_even] =
          xmlPath +
          "/TMVAClassification_BDT_2L_2J_75ptv_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvMedium_odd] =
          xmlPath +
          "/TMVAClassification_BDT_2L_2J_75ptv_150ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvMedium_even] =
          xmlPath +
          "/TMVAClassification_BDT_2L_3pJ_75ptv_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvMedium_odd] =
          xmlPath +
          "/TMVAClassification_BDT_2L_3pJ_75ptv_150ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_even] =
          xmlPath + "/TMVAClassification_BDT_2L_2J_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_2L_2J_150ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_even] =
          xmlPath + "/TMVAClassification_BDT_2L_3pJ_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_2L_3pJ_150ptv_2of2.weights.xml";
    }
  } else if (m_mvatype == MVAType::split250 ||
             m_mvatype == MVAType::split250Incl1LepMedium) {
    if (m_anaType == "0lep") {
      m_xmlFiles[nTags2_nJ2_ptvHigh_even] =
          xmlPath +
          "/TMVAClassification_BDT_0L_2J_150ptv_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_odd] =
          xmlPath +
          "/TMVAClassification_BDT_0L_2J_150ptv_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_even] =
          xmlPath +
          "/TMVAClassification_BDT_0L_3J_150ptv_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_odd] =
          xmlPath +
          "/TMVAClassification_BDT_0L_3J_150ptv_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvVeryHigh_even] =
          xmlPath + "/TMVAClassification_BDT_0L_2J_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvVeryHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_0L_2J_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvVeryHigh_even] =
          xmlPath + "/TMVAClassification_BDT_0L_3J_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvVeryHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_0L_3J_250ptv_2of2.weights.xml";
    }
    if (m_anaType == "1lep") {
      m_xmlFiles[nTags2_nJ2_ptvHigh_even] =
          xmlPath +
          "/TMVAClassification_BDT_1L_2J_150ptv_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_odd] =
          xmlPath +
          "/TMVAClassification_BDT_1L_2J_150ptv_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_even] =
          xmlPath +
          "/TMVAClassification_BDT_1L_3J_150ptv_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_odd] =
          xmlPath +
          "/TMVAClassification_BDT_1L_3J_150ptv_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvVeryHigh_even] =
          xmlPath + "/TMVAClassification_BDT_1L_2J_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvVeryHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_1L_2J_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvVeryHigh_even] =
          xmlPath + "/TMVAClassification_BDT_1L_3J_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvVeryHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_1L_3J_250ptv_2of2.weights.xml";
      if (m_mvatype == MVAType::split250Incl1LepMedium) {
        m_xmlFiles[nTags2_nJ2_ptvMedium_even] =
            xmlPath +
            "/TMVAClassification_BDT_1L_2J_75ptv_150ptv_1of2.weights.xml";
        m_xmlFiles[nTags2_nJ2_ptvMedium_odd] =
            xmlPath +
            "/TMVAClassification_BDT_1L_2J_75ptv_150ptv_2of2.weights.xml";
        m_xmlFiles[nTags2_nJ3_ptvMedium_even] =
            xmlPath +
            "/TMVAClassification_BDT_1L_3J_75ptv_150ptv_1of2.weights.xml";
        m_xmlFiles[nTags2_nJ3_ptvMedium_odd] =
            xmlPath +
            "/TMVAClassification_BDT_1L_3J_75ptv_150ptv_2of2.weights.xml";
      }
    }
    if (m_anaType == "2lep") {
      m_xmlFiles[nTags2_nJ2_ptvMedium_even] =
          xmlPath +
          "/TMVAClassification_BDT_2L_2J_75ptv_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvMedium_odd] =
          xmlPath +
          "/TMVAClassification_BDT_2L_2J_75ptv_150ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvMedium_even] =
          xmlPath +
          "/TMVAClassification_BDT_2L_3pJ_75ptv_150ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvMedium_odd] =
          xmlPath +
          "/TMVAClassification_BDT_2L_3pJ_75ptv_150ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_even] =
          xmlPath +
          "/TMVAClassification_BDT_2L_2J_150ptv_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvHigh_odd] =
          xmlPath +
          "/TMVAClassification_BDT_2L_2J_150ptv_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_even] =
          xmlPath +
          "/TMVAClassification_BDT_2L_3pJ_150ptv_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvHigh_odd] =
          xmlPath +
          "/TMVAClassification_BDT_2L_3pJ_150ptv_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvVeryHigh_even] =
          xmlPath + "/TMVAClassification_BDT_2L_2J_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ2_ptvVeryHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_2L_2J_250ptv_2of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvVeryHigh_even] =
          xmlPath + "/TMVAClassification_BDT_2L_3pJ_250ptv_1of2.weights.xml";
      m_xmlFiles[nTags2_nJ3_ptvVeryHigh_odd] =
          xmlPath + "/TMVAClassification_BDT_2L_3pJ_250ptv_2of2.weights.xml";
    }
  }
}

MVACategory MVAApplication_TMVA::getMVACategory(unsigned long long EventNumber,
                                                int nTags, int nJ, float pTV) {
  MVACategory category = MVACategory::Unknown;
  if (nTags == 2) {
    if (nJ == 2) {
      if (pTV >= 150) {
        if (m_mvatype == MVAType::split250 ||
            m_mvatype == MVAType::split250Incl1LepMedium) {
          if (pTV >= 150 && pTV < 250) {
            if (EventNumber % 2 < 1)
              category = MVACategory::nTags2_nJ2_ptvHigh_even;
            if (EventNumber % 2 > 0)
              category = MVACategory::nTags2_nJ2_ptvHigh_odd;
          } else if (pTV >= 250) {
            if (EventNumber % 2 < 1)
              category = MVACategory::nTags2_nJ2_ptvVeryHigh_even;
            if (EventNumber % 2 > 0)
              category = MVACategory::nTags2_nJ2_ptvVeryHigh_odd;
          }
        } else {
          if (EventNumber % 2 < 1)
            category = MVACategory::nTags2_nJ2_ptvHigh_even;
          if (EventNumber % 2 > 0)
            category = MVACategory::nTags2_nJ2_ptvHigh_odd;
        }
      } else if (pTV > 75 && pTV < 150) {
        if (m_anaType == "0lep")
          return category;
        else if (m_anaType == "2lep" ||
                 (m_anaType == "1lep" &&
                  (m_mvatype == MVAType::ichepStyleIncl1LepMedium ||
                   m_mvatype == MVAType::split250Incl1LepMedium))) {
          if (EventNumber % 2 < 1)
            category = MVACategory::nTags2_nJ2_ptvMedium_even;
          if (EventNumber % 2 > 0)
            category = MVACategory::nTags2_nJ2_ptvMedium_odd;
        }
      } else
        return category;
    } else if (nJ == 3 || (m_anaType == "2lep" && nJ >= 3)) {
      if (pTV >= 150) {
        if (m_mvatype == MVAType::split250 ||
            m_mvatype == MVAType::split250Incl1LepMedium) {
          if (pTV >= 150 && pTV < 250) {
            if (EventNumber % 2 < 1)
              category = MVACategory::nTags2_nJ3_ptvHigh_even;
            if (EventNumber % 2 > 0)
              category = MVACategory::nTags2_nJ3_ptvHigh_odd;
          } else if (pTV >= 250) {
            if (EventNumber % 2 < 1)
              category = MVACategory::nTags2_nJ3_ptvVeryHigh_even;
            if (EventNumber % 2 > 0)
              category = MVACategory::nTags2_nJ3_ptvVeryHigh_odd;
          }
        } else {
          if (EventNumber % 2 < 1)
            category = MVACategory::nTags2_nJ3_ptvHigh_even;
          if (EventNumber % 2 > 0)
            category = MVACategory::nTags2_nJ3_ptvHigh_odd;
        }
      } else if (pTV > 75 && pTV < 150) {
        if (m_anaType == "0lep")
          return category;
        else if (m_anaType == "2lep" ||
                 (m_anaType == "1lep" &&
                  (m_mvatype == MVAType::ichepStyleIncl1LepMedium ||
                   m_mvatype == MVAType::split250Incl1LepMedium))) {
          if (EventNumber % 2 < 1)
            category = MVACategory::nTags2_nJ3_ptvMedium_even;
          if (EventNumber % 2 > 0)
            category = MVACategory::nTags2_nJ3_ptvMedium_odd;
        }
      } else
        return category;
    }
  }
  return category;
}

// type: Variable, Spectator
std::vector<std::string> MVAApplication_TMVA ::getExpressions(
    std::string xmlFile, std::string type) {
  TXMLEngine *xmlEngine = new TXMLEngine();
  XMLDocPointer_t doc = xmlEngine->ParseFile(xmlFile.c_str());
  if (!doc) {
    Error("MVAApplication_TMVA ::getExpressions()",
          "Couldn't parse xml file '%s' ", xmlFile.c_str());
    exit(EXIT_FAILURE);
  }

  std::vector<std::string> expressions;
  XMLNodePointer_t node = xmlEngine->DocGetRootElement(doc);
  if (!node) {
    Error("MVAApplication_TMVA ::getExpressions()",
          "Couldn't get root node '%s' from file ", xmlFile.c_str());
    exit(EXIT_FAILURE);
  }

  // Go to level of Variables:
  node = xmlEngine->GetChild(node);
  // Loop over nodes:
  while (node) {
    std::string nodeName = xmlEngine->GetNodeName(node);
    // Read Expression from Variable:
    if (nodeName == type) {
      const char *expr = xmlEngine->GetAttr(node, "Expression");
      if (expr) {
        expressions.push_back(expr);
      }
    }
    if (nodeName == type + "s") {
      // Go to level of Variable:
      node = xmlEngine->GetChild(node);
    } else {
      // Next node:
      node = xmlEngine->GetNext(node);
    }
  }
  xmlEngine->CleanNode(xmlEngine->DocGetRootElement(doc));
  delete xmlEngine;
  return expressions;
}

std::vector<std::string> MVAApplication_TMVA::split(std::string str,
                                                    std::string del) {
  std::string::size_type delim_length = del.length();
  std::vector<std::string> result;
  if (delim_length == 0) {
    result.push_back(str);
  } else {
    std::string::size_type offset = 0;
    while (true) {
      std::string::size_type pos = str.find(del, offset);
      if (pos == std::string::npos) {
        result.push_back(str.substr(offset));
        break;
      }
      result.push_back(str.substr(offset, pos - offset));
      offset = pos + delim_length;
    }
  }
  return result;
}

std::string MVAApplication_TMVA ::shorten(std::string exp) {
  std::string tmp = exp;
  if (tmp.find(">") != std::string::npos) tmp = split(tmp, ">")[0];
  if (tmp.find("<") != std::string::npos) tmp = split(tmp, "<")[0];
  return tmp;
}
