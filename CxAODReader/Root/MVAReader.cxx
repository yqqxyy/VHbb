#include "CxAODReader/MVAReader.h"

#include "TError.h"
#include "TPRegexp.h"
#include "TXMLEngine.h"

MVAReader::MVAReader()
    : m_splitVar(nullptr),
      m_reader(),
      m_varsInt(),
      m_varsFloat(),
      m_varsULong64() {}

MVAReader::~MVAReader() {
  for (std::pair<TString, ReaderFold*> fold : m_reader) {
    delete fold.second;
  }
}

void MVAReader::AddReader(TString name, int kFold) {
  if (m_reader.count(name)) {
    Error("MVAReader::AddReader()", "Reader '%s' already registered!",
          name.Data());
    exit(EXIT_FAILURE);
  }

  if (kFold < 1) {
    kFold = 1;
  }
  m_reader[name] = new ReaderFold(kFold);
}

void MVAReader::AddVariable(TString name, Int_t* adress) {
  if (m_varsInt.count(name)) {
    Error("MVAReader::AddVariable()", "Variable '%s' (int) already registered!",
          name.Data());
    exit(EXIT_FAILURE);
  }
  m_varsInt[name] = adress;
}

void MVAReader::AddVariable(TString name, Float_t* adress) {
  if (m_varsFloat.count(name)) {
    Error("MVAReader::AddVariable()",
          "Variable '%s' (float) already registered!", name.Data());
    exit(EXIT_FAILURE);
  }
  m_varsFloat[name] = adress;
}

void MVAReader::AddVariable(TString name, ULong64_t* adress) {
  if (m_varsULong64.count(name)) {
    Error("MVAReader::AddVariable()",
          "Variable '%s' (float) already registered!", name.Data());
    exit(EXIT_FAILURE);
  }
  m_varsULong64[name] = adress;
}

std::vector<TString> MVAReader::getExpressions(TString xmlFile) {
  TXMLEngine* xmlEngine = new TXMLEngine();
  XMLDocPointer_t doc = xmlEngine->ParseFile(xmlFile);
  if (!doc) {
    Error("MVAReader::getExpressions()", "Could not parse XML file '%s'!",
          xmlFile.Data());
    exit(EXIT_FAILURE);
  }

  XMLNodePointer_t node = xmlEngine->DocGetRootElement(doc);
  if (!node) {
    Error("MVAReader::getExpressions()",
          "Could get root node from XML file '%s'!", xmlFile.Data());
    exit(EXIT_FAILURE);
  }

  std::vector<TString> expressions;
  // Go to level of Variables:
  node = xmlEngine->GetChild(node);
  // Loop over nodes:
  while (node) {
    TString nodeName = xmlEngine->GetNodeName(node);
    // Read Expression from Variable:
    if (nodeName == "Variable") {
      const char* expr = xmlEngine->GetAttr(node, "Expression");
      if (expr) {
        expressions.push_back(TString(expr));
      }
    }
    if (nodeName == "Variables") {
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

bool MVAReader::matchExpr(TString expression, TString varName) {
  TPRegexp regex("(\\w+)");
  TObjArray* arr = regex.MatchS(expression);
  for (TObject* obj : *arr) {
    if (((TObjString*)obj)->GetString() == varName) {
      return true;
    }
  }
  return false;
}

void MVAReader::addVariables(TMVA::Reader* reader,
                             std::vector<TString> expressions) {
  // Loop over expressions (same order is expected by reader)
  for (TString expression : expressions) {
    // Add integer if varName matches the expression
    int matches = 0;
    for (std::pair<TString, Int_t*> var : m_varsInt) {
      TString varName = var.first;
      if (matchExpr(expression, varName) && !matches) {
        reader->AddVariable(expression, var.second);
        matches++;
      }
    }
    // Add float if varName matches the expression
    for (std::pair<TString, Float_t*> var : m_varsFloat) {
      TString varName = var.first;
      if (matchExpr(expression, varName) && !matches) {
        reader->AddVariable(expression, var.second);
        matches++;
      }
    }
    // Check for multiple matches
    if (matches > 1) {
      Error("MVAReader::addVariables()",
            "Found multiple matches for expression '%s'!", expression.Data());
      exit(EXIT_FAILURE);
    }
  }
}

void MVAReader::BookReader(TString name, TString xmlFile) {
  if (!m_reader.count(name)) {
    Error("MVAReader::BookReader()", "Reader '%s' not initialized!",
          name.Data());
    exit(EXIT_FAILURE);
  }

  Info("MVAReader::BookReader()", "Booking reader '%s' with file '%s'",
       name.Data(), xmlFile.Data());

  ReaderFold* fold = m_reader[name];

  if (xmlFile.Contains("Regression")) {
    fold->isRegression = true;
  }

  std::vector<TString> expressions = getExpressions(xmlFile);
  if (!expressions.size()) {
    Error("MVAReader::BookReader()",
          "Did not find any Expression in XML file!");
    exit(EXIT_FAILURE);
  }
  addVariables(fold->reader, expressions);

  if (fold->k == 1) {
    fold->reader->BookMVA(name, xmlFile);
    return;
  }

  TString kFoldTag = TString::Format("0of%i", fold->k);
  if (!xmlFile.Contains(kFoldTag)) {
    Error("MVAReader::BookReader()",
          "Could not find kFold tag '%s' in file name!", kFoldTag.Data());
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < fold->k; i++) {
    TString iFoldTag = TString::Format("%iof%i", i, fold->k);
    TString fullName = name + "_" + iFoldTag;
    TString xmlFileI = xmlFile;
    xmlFileI.ReplaceAll(kFoldTag, iFoldTag);
    fold->reader->BookMVA(fullName, xmlFileI);
    Info("MVAReader::BookReader()", "Added kFold file '%s'.", xmlFileI.Data());
  }
}

float MVAReader::EvaluateMVA(TString name) {
  if (!m_reader.count(name)) {
    Error("MVAReader::EvaluateMVA()", "Reader '%s' not initialized!",
          name.Data());
    exit(EXIT_FAILURE);
  }

  ReaderFold* fold = m_reader[name];
  TMVA::Reader* reader = fold->reader;
  TString fullName = name;
  if (fold->k > 1) {
    if (!m_splitVar) {
      Error("MVAReader::EvaluateMVA()",
            "Split variable for kFold not initialized!");
      exit(EXIT_FAILURE);
    }
    int iFold = (*m_splitVar) % (fold->k);
    TString iFoldTag = TString::Format("%iof%i", iFold, fold->k);
    fullName = name + "_" + iFoldTag;
  }

  if (!reader->FindMVA(fullName)) {
    Error("MVAReader::EvaluateMVA()", "MVA method '%s' not booked!",
          fullName.Data());
    exit(EXIT_FAILURE);
  }

  if (fold->isRegression) {
    return reader->EvaluateRegression(0, fullName);
  }

  return reader->EvaluateMVA(fullName);
}
