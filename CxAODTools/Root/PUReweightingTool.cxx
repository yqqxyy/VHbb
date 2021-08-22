#ifndef __MAKECINT__
#include "CxAODTools/PUReweightingTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "PATInterfaces/SystematicSet.h"
#endif // __MAKECINT__


PUReweightingTool::PUReweightingTool(ConfigStore & config):
  m_config(config),
  m_debug(false),
  m_computePileupReweight(true),
  m_recomputePileupReweight(false),
  m_msgLevel(MSG::WARNING),
  m_initialized(false)
{
  m_config.getif<bool>("debug", m_debug);
  bool info = false;
  m_config.getif<bool>("printCPToolsInfo", info);
  if (m_debug) {
    m_msgLevel = MSG::DEBUG;
  } else if (info) {
    m_msgLevel = MSG::INFO;
  }
  m_ilumicalcFiles.clear();
  m_configFiles.clear();
}

PUReweightingTool::~PUReweightingTool()
{
}

EL::StatusCode PUReweightingTool::initialize()
{
  m_pileupReweighting.setTypeAndName("CP::PileupReweightingTool/PileupReweightingTool");
  m_config.getif<bool>("computePileupReweight", m_computePileupReweight);
  m_config.getif<bool>("recomputePileupReweight", m_recomputePileupReweight);
  if (m_debug) Info("PUReweightingTool::decorate()","computePileupReweight=%i",int(m_computePileupReweight));
  if (m_debug) Info("PUReweightingTool::decorate()","recomputePileupReweight=%i",int(m_recomputePileupReweight));
  //tool can be in the following status:
  //undefined: no ilumicalcFile
  //defined to get random run number for CP tools, but no weight and mu: ilumicalcFile(s), but no configFile(s) - to be confirmed
  //defined to get random run number for CP tools, but also weight and mu: both ilumicalcFiles(s) and configFile(s) 
  m_config.getif<std::vector<std::string>>("ilumicalcFiles", m_ilumicalcFiles);
  Info("PUReweightingTool::initialize()", "%i ilumicCalcFiles given for PileupReweighting tool.",int(m_ilumicalcFiles.size()));
  if (m_ilumicalcFiles.size()==0) {
    Info("PUReweightingTool::initialize()", "No ilumicCalcFiles given, so not initializing the PileupReweighting tool.");
    m_initialized=false;
    return EL::StatusCode::SUCCESS;
  }
  //we can proceed to initialize the tool
  for (size_t i=0; i<m_ilumicalcFiles.size(); i++){
    Info("PUReweightingTool::initialize()", "ilumicalcFile installed: %s", m_ilumicalcFiles[i].c_str());
  }
  std::vector<std::string> m_configFiles;
  m_configFiles=m_config.get< std::vector<std::string> >("configFiles");
  if (m_configFiles.size()>0) {
    Info("PUReweightingTool::initialize()", "%i configFiles given",int(m_configFiles.size()));
    for (size_t i=0; i<m_configFiles.size(); i++){
      Info("PUReweightingTool::initialize()", "configFile installed: %s.", m_configFiles[i].c_str());
    }
  }
  //set tool properties
  TOOL_CHECK("PUReweightingTool::initialize()",m_pileupReweighting.setProperty("LumiCalcFiles",m_ilumicalcFiles));
  TOOL_CHECK("PUReweightingTool::initialize()",m_pileupReweighting.setProperty("ConfigFiles",m_configFiles));
  TOOL_CHECK("PUReweightingTool::initialize()",m_pileupReweighting.setProperty("OutputLevel",m_msgLevel));
  // initialize tool
  TOOL_CHECK("PUReweightingTool::initialize()",m_pileupReweighting.retrieve());
  m_initialized=true;
 return EL::StatusCode::SUCCESS;
}

EL::StatusCode PUReweightingTool::decorate(const xAOD::EventInfo* eventInfo)
{
  if (!(m_computePileupReweight || m_recomputePileupReweight)){
    if (m_debug){
      Info("PUReweightingTool::decorate()","You did not ask to computePileupReweight or recomputePileupReweight, so do not decorate anything. Exiting method.");
    }
    return EL::StatusCode::SUCCESS;
  }
  // you are here, so you want to decorate the pileup reweighting info onto the eventInfo handler
  if(!m_initialized){
    Error("PUReweightingTool::decorate()","PileupReweighting tool has not been initialized properly, will ABORT!!!");
    return EL::StatusCode::FAILURE;
  }
  if(!eventInfo){
    Error("PUReweightingTool::decorate()","No eventInfo handler, will ABORT!!!");
    return EL::StatusCode::FAILURE;
  }
  // you ar here, so tool has been initialized and eventInfo handler, we proceed to compute all quantities
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ExtendedPileupReweighting#Pileup_Reweighting
  float pileupReweight = 1.0;
  if(Props::isMC.get(eventInfo)) pileupReweight=m_pileupReweighting->getCombinedWeight(*eventInfo);
  if (m_debug) Info("PUReweightingTool::decorate()","pileupReweight=%f",pileupReweight);
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ExtendedPileupReweighting#RandomRunNumbers_in_MC
  unsigned int randomRunNumber = -1;
  if(Props::isMC.get(eventInfo)) randomRunNumber=m_pileupReweighting->getRandomRunNumber(*eventInfo);
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ExtendedPileupReweighting#Correcting_mu_values
  if (m_debug) Info("PUReweightingTool::decorate()","randomRunNumber=%i",int(randomRunNumber));
  float correctedAvgMu = m_pileupReweighting->getCorrectedAverageInteractionsPerCrossing(*eventInfo);
  if (m_debug) Info("PUReweightingTool::decorate()","correctedAvgMu=%f",correctedAvgMu);
  float correctedAndScaledAvgMu = m_pileupReweighting->getCorrectedAverageInteractionsPerCrossing(*eventInfo,true);
  if (m_debug) Info("PUReweightingTool::decorate()","correctedAndScaledAvgMu=%f",correctedAndScaledAvgMu);
  float correctedMu = m_pileupReweighting->getCorrectedActualInteractionsPerCrossing(*eventInfo);
  if (m_debug) Info("PUReweightingTool::decorate()","correctedMu=%f",correctedMu);
  float correctedAndScaledMu = m_pileupReweighting->getCorrectedActualInteractionsPerCrossing(*eventInfo,true); 
  if (m_debug) Info("PUReweightingTool::decorate()","correctedAndScaledMu=%f",correctedAndScaledMu);
  // quantities computed, then store in Props of eventInfo handler
  if(m_computePileupReweight){
    Props::PileupReweight.set(eventInfo,pileupReweight);
    Props::RandomRunNumber.set(eventInfo,randomRunNumber);
    Props::CorrectedAvgMu.set(eventInfo,correctedAvgMu);
    Props::CorrectedAndScaledAvgMu.set(eventInfo,correctedAndScaledAvgMu);
    Props::CorrectedMu.set(eventInfo,correctedMu);
    Props::CorrectedAndScaledMu.set(eventInfo,correctedAndScaledMu);
  } else if(m_recomputePileupReweight){
    Props::RecomputePileupReweight.set(eventInfo,pileupReweight);
    Props::RecomputeRandomRunNumber.set(eventInfo,randomRunNumber);
    Props::RecomputeCorrectedAvgMu.set(eventInfo,correctedAvgMu);
    Props::RecomputeCorrectedAndScaledAvgMu.set(eventInfo,correctedAndScaledAvgMu);
    Props::RecomputeCorrectedMu.set(eventInfo,correctedMu);
    Props::RecomputeCorrectedAndScaledMu.set(eventInfo,correctedAndScaledMu);
  }

  return EL::StatusCode::SUCCESS;
}

CP::SystematicSet PUReweightingTool::affectingSystematics() 
{
  if ( m_pileupReweighting.isInitialized() ) {
    return m_pileupReweighting->affectingSystematics();
  }
  CP::SystematicSet defaultSet;
  return defaultSet;
}

CP::SystematicCode PUReweightingTool::applySystematicVariation(CP::SystematicSet set) {
  if (m_pileupReweighting.isInitialized()) {
    return m_pileupReweighting->applySystematicVariation(set);
  }
  return CP::SystematicCode::Ok;
}
