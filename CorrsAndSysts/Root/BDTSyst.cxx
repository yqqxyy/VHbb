#include "CorrsAndSysts/BDTSyst.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include "TError.h"
#include "TFile.h"
#include "TKey.h"
#include "TPRegexp.h"

BDTSyst::BDTSyst() : m_debug(0), m_useTF1(false) {}

BDTSyst::~BDTSyst() {}

void BDTSyst::ShapeProvider(int channel) {
  std::ostringstream channel_str;
  channel_str << channel;

  std::string fileName = m_WorkDir_DIR + "/data/CorrsAndSysts/Shapes_" +
                         channel_str.str() + "Lepton.root";

  TFile* file = TFile::Open(fileName.c_str());

  if (m_debug)
    Info("BDTSyst::ShapeProvider()", "Reading file '%s'.", fileName.c_str());

  file->cd();

  TKey* key = 0;
  TIter next = static_cast<TList*>(file->GetListOfKeys());

  int is_first = 1;

  while ((key = static_cast<TKey*>(next()))) {
    TString histName = key->GetName();
    TClass* cl = gROOT->GetClass(key->GetClassName());
    if (cl->InheritsFrom("TH1")) {
      TH1* temp_th1 = static_cast<TH1*>((file->Get(histName))->Clone());
      m_ShapeForm_th1[histName] = temp_th1;
      if (is_first) {
        m_useTF1 = false;
        is_first = 0;
      }
    } else if (cl->InheritsFrom("TF1")) {
      TF1* temp_tf1 = static_cast<TF1*>((file->Get(histName))->Clone());
      m_ShapeForm_tf1[histName] = temp_tf1;
      if (is_first) {
        m_useTF1 = true;
        is_first = 0;
      }
    } else
      continue;  // only use TF1 or TH1 as shapes
  }
  file->Close();
}

void BDTSyst::Initialise(int channel) { ShapeProvider(channel); }

TString BDTSyst::DetermineXMLName(TString variation, int channel) {
  TString XMLname = "TMVAClassification_BDTCategories_";
  XMLname += channel;
  XMLname += "lep_";
  XMLname += m_Variables["nJ"];
  XMLname += "jet_";
  XMLname += variation;
  XMLname += "_";
  if (m_sample == "ttbar") {
    if (m_Variables["FlavourLabel"] == 0)
      XMLname += "bb";
    else if (m_Variables["FlavourLabel"] == 1)
      XMLname += "bc";
    else
      XMLname += "oth";
  } else {
    XMLname += "HF";
  }

  XMLname += ".weights.xml";

  if (m_debug)
    Info("BDTSyst::DetermineXMLName()", "Reading histo '%s'.", XMLname.Data());

  return XMLname;
}

TString BDTSyst::DetermineTF1Name(TString variation, int channel) {
  TString TF1name = "";
  TF1name += variation + "_";
  TF1name += m_Variables["nTag"];
  TF1name += "tag";
  TF1name += m_Variables["nJ"];

  if (channel == 0)
    TF1name += "jet_150ptv_";
  else if (channel == 1)
    TF1name += "jet_75ptv_";

  if (m_Variables["FlavourLabel"] == 0)
    TF1name += "bb";
  else if (m_Variables["FlavourLabel"] == 1)
    TF1name += "bc";
  else if (m_Variables["FlavourLabel"] == 2)
    if (m_sample == "ttbar")
      TF1name += "oth";
    else
      TF1name += "bl";
  else if (m_Variables["FlavourLabel"] == 3)
    if (m_sample == "ttbar")
      TF1name += "oth";
    else
      TF1name += "cc";
  else
    TF1name += "oth";

  if (m_debug)
    Info("BDTSyst::DetermineTF1Name()", "Reading histo '%s'.", TF1name.Data());

  return TF1name;
}

TString BDTSyst::DetermineTH1Name(TString variation, int channel) {
  TString TH1name = "";
  TH1name += variation + "_";
  TH1name += m_Variables["nTag"];
  TH1name += "tag";
  TH1name += m_Variables["nJ"];

  if (channel == 0)
    TH1name += "jet_150ptv_";
  else if (channel == 1)
    TH1name += "jet_75ptv_";

  if (m_Variables["FlavourLabel"] == 0)
    TH1name += "bb";
  else if (m_Variables["FlavourLabel"] == 1)
    TH1name += "bc";
  else if (m_Variables["FlavourLabel"] == 2)
    if (m_sample == "ttbar")
      TH1name += "oth";
    else
      TH1name += "bl";
  else if (m_Variables["FlavourLabel"] == 3)
    if (m_sample == "ttbar")
      TH1name += "oth";
    else
      TH1name += "cc";
  else
    TH1name += "oth";

  if (m_debug)
    Info("BDTSyst::DetermineTH1Name()", "Reading histo '%s'.", TH1name.Data());

  return TH1name;
}

TString BDTSyst::DetermineMVAToEval(int channel) {
  TString MVAToEval = "BDT::MVACat_";
  MVAToEval += m_Variables["FlavourLabel"];
  MVAToEval += "flav_";
  MVAToEval += m_Variables["nJ"];
  MVAToEval += "j_";
  // MVAToEval += m_Variables["EventMod2"] + 1;
  if (m_sample == "ttbar" && channel == 0)
    MVAToEval += 1;
  else
    MVAToEval += m_Variables["EventNumberModNfold"] + 1;

  if (m_sample == "ttbar" && channel == 0)
    MVAToEval += "o1";
  else
    MVAToEval += "o2";

  if (m_debug)
    Info("BDTSyst::DetermineMVAToEval()", "MVAToEval is '%s'.",
         MVAToEval.Data());
  return MVAToEval;
}

std::vector<TString> BDTSyst::getExpressions(TString xmlFile,
                                             std::vector<TString>& spectators,
                                             std::vector<TString>& BDTs) {
  xmlFile = m_WorkDir_DIR + "/data/CorrsAndSysts/" + xmlFile;

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
      std::string expr_string(expr);
      expressions.push_back(TString(expr));
    }
    if (nodeName == "Variables") {
      // Go to level of Variable:
      node = xmlEngine->GetChild(node);
    } else {
      // Next node:
      node = xmlEngine->GetNext(node);
    }
  }
  // Have to to this loop twice to get spectators, but could be written better!
  node = xmlEngine->DocGetRootElement(doc);
  node = xmlEngine->GetChild(node);
  while (node) {
    TString nodeName = xmlEngine->GetNodeName(node);
    // Read Expression from Spectator:
    if (nodeName == "Spectator") {
      const char* expr = xmlEngine->GetAttr(node, "Expression");
      if (expr) {
        spectators.push_back(TString(expr));
      }
    }
    if (nodeName == "Spectators") {
      // Go to level of Spectator:
      node = xmlEngine->GetChild(node);
    } else {
      // Next node:
      node = xmlEngine->GetNext(node);
    }
  }

  // Now loop to get the available BDTs
  node = xmlEngine->DocGetRootElement(doc);
  node = xmlEngine->GetChild(node);
  while (node) {
    TString nodeName = xmlEngine->GetNodeName(node);
    // Read Expression from Spectator:
    if (nodeName == "SubMethod") {
      const char* expr = xmlEngine->GetAttr(node, "Method");
      BDTs.push_back(TString(expr));
    }
    if (nodeName == "Weights") {
      // Go to level of Spectator:
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

void BDTSyst::AddFactory(TString XMLName) {
  // This loading of variables needs to be automated, as done in MVATree.cxx
  // Need to automate booking of MVA name too

  std::vector<TString> expressions, spectators, BDTs;

  expressions = getExpressions(XMLName, spectators,
                               BDTs);  // Get the variables from the xml file

  TMVA::Reader* reader = new TMVA::Reader("!Color:!Silent");
  std::cout << "Going to iterate" << std::endl;

  std::map<TString, float>::iterator it;

  for (unsigned int i = 0; i < expressions.size(); ++i) {
    reader->AddVariable(
        expressions.at(i),
        &m_Variables[expressions.at(i)]);  // Adding the variables
  }
  for (unsigned int i = 0; i < spectators.size(); ++i) {
    reader->AddSpectator(
        spectators.at(i),
        &m_Variables[spectators.at(i)]);  // Adding the spectators
  }

  for (unsigned int i = 0; i < BDTs.size(); i++) {
    reader->BookMVA(BDTs.at(i),
                    m_WorkDir_DIR + "/data/CorrsAndSysts/" + XMLName);
  }
  // reader->BookMVA("MVACategories_1o2",
  // m_WorkDir_DIR + "/data/CorrsAndSysts/" + XMLName);

  m_BDTFactories[XMLName] = reader;
}

float BDTSyst::DetermineWeight(TString variation, TString sample, int channel,
                               float& BDTScore) {
  m_sample = sample;

  if (m_debug)
    Info("BDTSyst::DetermineWeight()",
         "Looking at sample '%s' with variation %s .", m_sample.Data(),
         variation.Data());

  TString histName;

  if (sample == "ttbar") {
    m_useTF1 = true;
    histName = DetermineTF1Name(variation, channel);
  } else if (sample == "W")
    histName = DetermineTH1Name(variation, channel);

  TString XMLName = DetermineXMLName(variation, channel);

  if (m_BDTFactories[XMLName] == 0) {
    AddFactory(XMLName);
  }

  if (m_debug) {
    std::vector<TString> expressions, spectators, BDTs;
    expressions = getExpressions(XMLName, spectators, BDTs);
    for (unsigned int i = 0; i < expressions.size(); ++i) {
      Info("BDTSyst::DetermineWeight()", " Found value %f for variable '%s'.",
           m_Variables[expressions.at(i)], expressions.at(i).Data());
    }
    for (unsigned int i = 0; i < spectators.size(); ++i) {
      Info("BDTSyst::DetermineWeight()", " Found value %f for spectator '%s'.",
           m_Variables[spectators.at(i)], spectators.at(i).Data());
    }
  }

  if (((m_useTF1 && m_ShapeForm_tf1[histName]) ||
       (!m_useTF1 && m_ShapeForm_th1[histName])) &&
      m_BDTFactories[XMLName]->FindMVA(DetermineMVAToEval(channel))) {
    // Need to correctly identify variation, and correct BDT to evaluate

    BDTScore =
        m_BDTFactories[XMLName]->EvaluateMVA(DetermineMVAToEval(channel));
    if (m_debug) Info("BDTSyst::DetermineWeight()", "BDT value: %f", BDTScore);
  } else {
    if (m_debug)
      Info("BDTSyst::DetermineWeight()",
           " No ratio files or BDT score has been found for this event");
    return 1.;
  }

  float reweight = 1.;
  if (m_useTF1) {
    reweight = m_ShapeForm_tf1[histName]->Eval(BDTScore);
  } else {
    reweight = m_ShapeForm_th1[histName]->GetBinContent(
        m_ShapeForm_th1[histName]->FindBin(BDTScore));
  }

  return reweight;
}

// Reset input values to BDT stored in m_Variables member variable
void BDTSyst::ResetInputs() {
  std::map<TString, Float_t>::iterator Iter = m_Variables.begin();
  std::map<TString, Float_t>::iterator EndIter = m_Variables.end();
  for (; Iter != EndIter; Iter++) {
    Iter->second = 0.0;
  }
}  // ResetInputs() End
