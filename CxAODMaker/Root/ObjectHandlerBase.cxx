#include "CxAODMaker/ObjectHandlerBase.h"

#include <iostream>
#include <iomanip>

#include "xAODBase/IParticle.h"

ObjectHandlerBase::ObjectHandlerBase(const std::string& name, ConfigStore & config, xAOD::TEvent * event) :
  m_handlerName(name),
  m_config(config),
  m_debug(false),
  m_msgLevel(MSG::WARNING),
  m_event(event),
  m_EventNumberToInvestigate(-1),
  m_containerName("none"),
  m_applyObjectSelection(true),
  m_storeOrig(false),
  m_linker(nullptr),
  m_usedForElementLinks(false),
  m_addPreselJets(false),
  m_createElementLinks(true),
  m_usedInOR(false),
  m_saveNonCrucialVariables(false)
{
  // set message level
  bool info = false;
  m_config.getif<bool>("debug", m_debug);
  m_config.getif<bool>("printCPToolsInfo", info);
  m_config.getif<int>("EventNumberToInvestigate", m_EventNumberToInvestigate);
  if (m_debug) {
    m_msgLevel = MSG::DEBUG;
  } else if (info) {
    m_msgLevel = MSG::INFO;
  }

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::ObjectHandler()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  // set containerName
  m_config.getif<std::string>(name+"Container",m_containerName);

  // master switch for object filtering
  m_config.getif<bool>("applyObjectSelection", m_applyObjectSelection);

  // set flag store original (non-calibrated) container
  m_config.getif<bool>("storeOriginal",m_storeOrig);

  // set flag to store variables non crucial for the final result
  m_config.getif<bool>("saveNonCrucialVariables",m_saveNonCrucialVariables);

  // add empty systematic (Nominal) by default
  m_sysList.push_back(CP::SystematicSet());

  // Recognize weight variations !
  std::vector<std::string> weightVariations;
  m_config.getif< std::vector<std::string> >("weightVariations", weightVariations);
  std::vector<std::string> pulls = {"1down", "1up"};
  for (const std::string& name : weightVariations) {
    for (const std::string& pull : pulls) {
      m_weightVariations.insert((name + "__" + pull).c_str());
    }
  }
  
  //initialize CutFlowCounter
  m_cutflow = new CutFlowCounter(name.c_str());

}

ObjectHandlerBase::~ObjectHandlerBase() 
{
  for (auto it : m_cutflowMap) {
    delete it.second;
  }
}

EL::StatusCode ObjectHandlerBase::addCPVariations(const std::vector<TString> &variations,
    const bool filterByTools, const bool skipWeightVar) {

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::addCPVariations()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  std::vector<std::string> jesPrefixes;
  m_config.getif<std::vector<std::string> >("jesPrefixes", jesPrefixes);
  std::vector<std::string> jesConfigs;
  m_config.getif<std::vector<std::string> >("jesConfigs", jesConfigs);
  if (jesConfigs.size() != jesPrefixes.size())
    jesPrefixes.clear();

  std::vector<std::string> jetUnc_FJ_prefixes;
  m_config.getif<std::vector<std::string> >("jetUnc_FJ_prefixes", jetUnc_FJ_prefixes);
  std::vector<std::string> jetUnc_FJ_configs;
  m_config.getif<std::vector<std::string> >("jetUnc_FJ_configs", jetUnc_FJ_configs);
  if (jetUnc_FJ_configs.size() != jetUnc_FJ_prefixes.size())
    jetUnc_FJ_prefixes.clear();

  std::vector<std::string> allPrefixes;
  allPrefixes.insert(allPrefixes.end(), jesPrefixes.begin(), jesPrefixes.end());
  allPrefixes.insert(allPrefixes.end(), jetUnc_FJ_prefixes.begin(), jetUnc_FJ_prefixes.end());

  //this is temp. to get the C2 unc. also in the new scheme
  if(std::find(allPrefixes.begin(), allPrefixes.end(), "FATJET_Medium_") == allPrefixes.end()){
    allPrefixes.push_back("FATJET_Medium_");
  }

  for (const TString &name : variations) {
    CP::SystematicVariation sysVar = CP::SystematicVariation(name.Data());
    bool useSys = !filterByTools;
    if (filterByTools) {
      std::string nameCopy (name.Data());
      for (auto &prefix: allPrefixes) {
        if (nameCopy.find(prefix) != std::string::npos)
          nameCopy = nameCopy.substr(nameCopy.find(prefix) + prefix.length());
      }
      for (CP::ISystematicsTool* tool : m_sysToolList) {
        useSys |= tool->isAffectedBySystematic(CP::SystematicVariation(nameCopy.c_str()));
      }
    }
    if(skipWeightVar && m_weightVariations.find(name) != m_weightVariations.end()) useSys = false;
    if (useSys) {
      // TODO check for duplicates?
      bool NoDuplicate = true;
      for(auto &var : m_sysList){
        if(var.name() == sysVar.name()){
          NoDuplicate = false;
          break;
        }
      }
      if(NoDuplicate){
        m_sysList.push_back(CP::SystematicSet());
        m_sysList.back().insert(sysVar);
      }

    }
  }
  return EL::StatusCode::SUCCESS;
}

std::vector<TString> ObjectHandlerBase::getAllVariations(bool includeNominal) {

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::getAllVariations()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  std::vector<TString> listOfContainers;
  listOfContainers.reserve(m_sysList.size());
  for (CP::SystematicSet &sysSet : m_sysList) {
    TString sysName = sysSet.name();
    if (sysName == "") {
      if (includeNominal) {
        sysName = "Nominal";
      } else {
        continue;
      }
    }
    listOfContainers.push_back(sysName);
  }
  if ( m_storeOrig ) listOfContainers.push_back("Original");

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::getAllVariations()", "Found %lu variations.",
        listOfContainers.size());
  }

  return listOfContainers;
}


void ObjectHandlerBase::printAllVariations() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandler::printAllVariations()", "Called for handler '%s'.", m_handlerName.c_str());
  }

  Info("ObjectHandler::printAllVariations()",
      ("Printing all variations to be run in handler '" + m_handlerName + "':").c_str());
  for (const TString& sysName : getAllVariations()) {
    std::cout << sysName.Data() << std::endl;
  }
  for (CP::ISystematicsTool* tool : m_sysToolList) {
    const std::string& toolName = tool -> name();
    Info("ObjectHandler::printAllVariations()",
        ("Printing variations available in CP tool '" + toolName + "':").c_str());
    CP::SystematicSet sysSet = tool->recommendedSystematics();
    const std::set<std::string> sysNames = sysSet.getBaseNames();
    for (const std::string& sysName: sysNames) {
      std::cout << sysName << std::endl;
    }
  }
}

void ObjectHandlerBase::setLinker(ParticleLinker* linker) {
  m_linker = linker;
}


EL::StatusCode ObjectHandlerBase::setPassSelFlags() {
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ObjectHandlerBase::setPassSelForLinked() {
  return EL::StatusCode::SUCCESS;
}


EL::StatusCode ObjectHandlerBase::fillOutputLinks() {
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode ObjectHandlerBase::flagOutputLinks() {
  return EL::StatusCode::SUCCESS;
}

void ObjectHandlerBase::printObjectCounts() {

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandlerBase::printObjectCounts()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  Info("ObjectHandlerBase::printObjectCounts()",
          ("Printing object counts for handler '" + m_handlerName +
          "' with container '" + m_containerName + "':").c_str());
  std::cout << std::setw(60) << "variation"
            << std::setw(10) << "input"
            << std::setw(11) << "passPreSel"
            << std::setw(10) << "passSel"
            << std::setw(10) << "output"
            << std::endl;
  for (std::pair<TString, long> const counts : m_inObjectsCount) {
    TString name = counts.first;
    std::cout << std::setw(60) << name
              << std::setw(10) << counts.second
              << std::setw(11) << m_inObjectsPreSel[name]
              << std::setw(10) << m_inObjectsSelected[name]
              << std::setw(10) << m_outObjectsCount[name]
              << std::endl;
  }
}

void ObjectHandlerBase::printParticle(xAOD::IParticle* particle) {

  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandlerBase::printParticle()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  std::cout << "particle->pt()  " << particle->pt() << std::endl;
  std::cout << "particle->eta() " << particle->eta() << std::endl;
  std::cout << "particle->phi() " << particle->phi() << std::endl;
  std::cout << "particle->e()   " << particle->e() << std::endl;
  std::cout << "particle->m()   " << particle->m() << std::endl;
}

EL::StatusCode ObjectHandlerBase::addCutflows() {
  
  if (m_msgLevel == MSG::DEBUG) {
    Info("ObjectHandlerBase::addCutflows()", "Called for handler '%s'.", m_handlerName.c_str());
  }
  
  std::vector<TString> listOfContainers = getAllVariations(true);
  for (const TString &contName : listOfContainers) {
    m_cutflowMap.insert(std::pair<TString, CutFlowCounter*>(contName, new CutFlowCounter(m_handlerName + "_" + contName)));
  }
  return EL::StatusCode::SUCCESS;
}
