#include "CxAODTools/TriggerTool.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/ConfigStore.h"

#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

#include "CxAODTools/TriggerScaleFactorsMET.h"

// TODO some Warnings might be better Errors and exit the job

TriggerTool::TriggerTool(ConfigStore& config) :
      m_config(config),
      m_debug(false),
      m_recomputePUWeight(false),
      m_applyMatching(true),
      m_thresholdFactor(1.05),
      m_eventInfoContNameMuonSF("EventInfo___Nominal"),
      m_muonSFTool(nullptr),
      m_electronEffProp(nullptr),
      m_electronSFProp(nullptr),
      m_met(nullptr),
      m_eventInfo(nullptr),
      m_lastNonZeroRun(-1),
      m_nRuns(0),
      m_nZeroRuns(0),
      m_nResetZeroRuns(0),
      m_nHardResetZeroRuns(0)
{
  m_config.getif<bool>("debug", m_debug);
  m_config.getif<bool>("recomputePUWeight", m_recomputePUWeight);
  m_config.getif<bool>("applyMatching", m_applyMatching);
  m_config.getif<std::string>("eventInfoContNameMuonSF", m_eventInfoContNameMuonSF);

  m_METscaleFactorTool = new TriggerScaleFactorsMET();
}

TriggerTool::~TriggerTool() {

  // Print out the numbers/fractions of events with RunNUmber=0      
  Info("TriggerTool::~TriggerTool()","NRuns processed               %i ", m_nRuns);
  Info("TriggerTool::~TriggerTool()","NRuns with runNumber=0        %i ", m_nZeroRuns);
  Info("TriggerTool::~TriggerTool()","NRuns set to previous run     %i ", m_nResetZeroRuns);
  Info("TriggerTool::~TriggerTool()","NRuns set to a 2017 run       %i ", m_nHardResetZeroRuns);
  Info("TriggerTool::~TriggerTool()","Fraction with runNumber=0     %f ", float(m_nZeroRuns)/float(m_nRuns));
  Info("TriggerTool::~TriggerTool()","Fraction set to previous run  %f ", float(m_nResetZeroRuns)/float(m_nRuns));
  Info("TriggerTool::~TriggerTool()","Fraction set to a 2017 run    %f ", float(m_nHardResetZeroRuns)/float(m_nRuns));

  delete m_METscaleFactorTool;
  if (m_muonSFTool) {
    delete m_muonSFTool;
  }
  for (TriggerInfo* trigger : m_triggers) {
    delete trigger;
  }
  m_triggers.clear();
}

EL::StatusCode TriggerTool::initialize() {
  EL_CHECK("TriggerTool::initialize()", initTools());
  EL_CHECK("TriggerTool::initialize()", initProperties());
  EL_CHECK("TriggerTool::initialize()", initTriggers());
  Info("TriggerTool::initialize()",
          "Initialized with %lu triggers.", m_triggers.size());
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TriggerTool::initTools() {
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TriggerTool::initProperties() {
  return EL::StatusCode::SUCCESS;
}

// Lowest unprescaled triggers:
// https://twiki.cern.ch/twiki/bin/viewauth/Atlas/LowestUnprescaled

void TriggerTool::addLowestUnprescaledElectron() {
  ADD_TRIG_MATCH(HLT_e24_lhmedium_L1EM20VH, any,  data15, data15);
  ADD_TRIG_MATCH(HLT_e60_lhmedium,          any,  data15, data15);
  ADD_TRIG_MATCH(HLT_e120_lhloose,          any,  data15, data15);

  ADD_TRIG_MATCH(HLT_e26_lhtight_nod0_ivarloose, any,  data16A, data18);
  ADD_TRIG_MATCH(HLT_e60_lhmedium_nod0,          any,  data16A, data18);
  ADD_TRIG_MATCH(HLT_e140_lhloose_nod0,          any,  data16A, data18);

  ADD_TRIG(HLT_e300_etcut, 		 any,  data16A, data18);// Egamma experts agreed that no needs for trigger match for this trigger
  
}

void TriggerTool::addLowestUnprescaledMuon() {  
  // https://twiki.cern.ch/twiki/bin/viewauth/Atlas/MuonTriggerPhysicsRecommendationsRel212017#Recommended_triggers
  // 2015      HLT_mu20_iloose_L1MU15 OR HLT_mu50
  // 2016-2018 HLT_mu26_ivarmedium    OR HLT_mu50  
  ADD_TRIG_MATCH(HLT_mu20_iloose_L1MU15,    any,  data15, data15);
  ADD_TRIG_MATCH(HLT_mu50,                  any,  data15, data18);
  ADD_TRIG_MATCH(HLT_mu26_ivarmedium,       any,  data16A, data18);

}

void TriggerTool::addLowestUnprescaledMET() {
  ADD_TRIG(HLT_xe70,             any, data15, data15);
  ADD_TRIG(HLT_xe90_mht_L1XE50,  any, data16A, data16BD3);
  ADD_TRIG(HLT_xe110_mht_L1XE50, any, data16D4E3, data16F2L11);
  ADD_TRIG(HLT_xe110_pufit_L1XE55, any, data17, data17);
  ADD_TRIG(HLT_xe110_pufit_xe70_L1XE50, any, data18, data18);
}

EL::StatusCode TriggerTool::initMuonSFTool(TString muonQuality) {
  CP::MuonTriggerScaleFactors* tool = new CP::MuonTriggerScaleFactors("MuonTrigSFTool");
  TOOL_CHECK("TriggerTool::initMuonSFTool()", tool->setProperty("MuonQuality", muonQuality.Data()));
  // From 21.2.20 the muon triggertool gets the run number from a steerable EventInfo 
  TOOL_CHECK("TriggerTool::initMuonSFTool()", tool->setProperty("EventInfoContName",m_eventInfoContNameMuonSF));
  TOOL_CHECK("TriggerTool::initMuonSFTool()", tool->initialize());
  tool->print();
  m_muonSFTool = tool;
  return EL::StatusCode::SUCCESS;
}


void TriggerTool::setElectrons(std::vector<const xAOD::Electron*> electrons) {
  m_electrons = getValidParticles<xAOD::Electron>(electrons);
}

void TriggerTool::setPhotons(std::vector<const xAOD::Photon*> photons) {
  m_photons = getValidParticles<xAOD::Photon>(photons);
}

void TriggerTool::setMuons(std::vector<const xAOD::Muon*> muons) {
  m_muons = getValidParticles<xAOD::Muon>(muons);
}

void TriggerTool::setJets(std::vector<const xAOD::Jet*> jets) {
  m_jets = getValidParticles<xAOD::Jet>(jets);
}

void TriggerTool::setMET(const xAOD::MissingET* met) {
  m_met = met;
}
void TriggerTool::setSumpt(double sumpt) {
  m_sumpt = sumpt;
}

void TriggerTool::clearTriggerObjects() {
  m_electrons.clear();
  m_photons.clear();
  m_muons.clear();
  m_jets.clear();
  m_met = nullptr;
}

void TriggerTool::setEventInfo(const xAOD::EventInfo* eventInfo, int randomRunNumber) {
    m_eventInfo = eventInfo;
    m_randomRunNumber = randomRunNumber;
}

EL::StatusCode TriggerTool::applySystematicVariation(CP::SystematicSet set) {
  m_currentVariation = set;
  if (m_muonSFTool) {
    CP_CHECK("TriggerTool::applySystematicVariation()",
	     m_muonSFTool->applySystematicVariation(set), m_debug);
  }
  return EL::StatusCode::SUCCESS;
}

bool TriggerTool::getDecisionAndScaleFactor(double& triggerSF) {

  std::set<TriggerInfo*> passedTriggers = getPassedTriggers();
  bool decision = (passedTriggers.size() > 0);
  
  triggerSF = 1;
  if (decision) {
    triggerSF = getScaleFactor(passedTriggers);
  }

  if (m_debug) {
    Info("TriggerTool::getDecisionAndScaleFactor()",
         "Trigger decision = %i and scale factor = %f.",
	 (int) decision, triggerSF);
  }

  return decision;
}

bool TriggerTool::getDecision() {
  return (getPassedTriggers().size() > 0);
}

double TriggerTool::getScaleFactor() {
  return getScaleFactor(getPassedTriggers());
}

std::set<TriggerInfo*> TriggerTool::getPassedTriggers() {
  std::set<TriggerInfo*> passedTriggers;
  if(!m_eventInfo) {
    Warning("TriggerTool::getPassedTriggers()", "No event info provided! Returning zero triggers.");
    return passedTriggers;
  }
  int runNumber = getRunNumber();
  for (TriggerInfo* trigger : m_triggers) {
    if (m_debug) {
      Info("TriggerTool::getPassedTriggers()",
              "Checking trigger '%s'.", trigger->getName().Data());
    }
    if (!checkRequirements(trigger)) {
      continue;
    }
    if (!trigger -> appliesToRun(runNumber)) {
      continue;
    }
    bool triggerFlag = trigger -> getPassProperty() -> get(m_eventInfo);
    if (!triggerFlag) {
      continue;
    }
    bool triggerMatch = true;
    if ( m_applyMatching && trigger -> requiresMatching()) {
      triggerMatch = getMatch(trigger);
    }
    if (!triggerMatch) {
      continue;
    }
    if (m_debug) {
      Info("TriggerTool::getPassedTriggers()",
              "Trigger '%s' passed.", trigger->getName().Data());
    }
    passedTriggers.insert(trigger);
  }

  if (m_debug) {
    Info("TriggerTool::getPassedTriggers()",
         "Found %lu passed triggers for %lu total triggers.",
	 passedTriggers.size(), m_triggers.size());
  }

  return passedTriggers;
}

double TriggerTool::getScaleFactor(std::set<TriggerInfo*> passedTriggers) {
  if(!m_eventInfo) {
    Warning("TriggerTool::getScaleFactor()", "No event info provided! Returning 1.");
    return 1;
  }
  if (!Props::isMC.get(m_eventInfo)) {
    return 1;
  }

  std::set<TriggerInfo*> triggersElectron = getTriggersOfObject(passedTriggers, TriggerInfo::TriggerObject::Electron);
  std::set<TriggerInfo*> triggersPhoton   = getTriggersOfObject(passedTriggers, TriggerInfo::TriggerObject::Photon);
  std::set<TriggerInfo*> triggersMuon     = getTriggersOfObject(passedTriggers, TriggerInfo::TriggerObject::Muon);
  std::set<TriggerInfo*> triggersJet      = getTriggersOfObject(passedTriggers, TriggerInfo::TriggerObject::Jet);
  std::set<TriggerInfo*> triggersMET      = getTriggersOfObject(passedTriggers, TriggerInfo::TriggerObject::MET);
  bool isElectron = (triggersElectron.size());
  bool isPhoton   = (triggersPhoton.size());
  bool isMuon     = (triggersMuon.size());
  bool isJet      = (triggersJet.size());
  bool isMET      = (triggersMET.size());
  
  int nTypes = isElectron + isPhoton + isMuon + isJet + isMET;
  
  bool isElectronOnly = (nTypes == 1 && isElectron);
  //bool isPhotonOnly   = (nTypes == 1 && isPhoton);
  bool isMuonOnly     = (nTypes == 1 && isMuon);
  //bool isJetOnly      = (nTypes == 1 && isJet);
  bool isMETOnly      = (nTypes == 1 && isMET);
  
  bool isElectronAndMuon = (nTypes == 2 && isElectron && isMuon);
  bool isMETAndMuon      = (nTypes == 2 && isMET  && isMuon);
  
  // TODO implement more options
  
  if (isElectronAndMuon) {
    if(m_debug) Info("TriggerTool:getScaleFactor", "e-mu event ntypes %i",nTypes);
    //return getScaleFactorElectronMuon(triggersElectron,triggersMuon);//triggersElectron not used by function so commenting it out
    return getScaleFactorElectronMuon(triggersMuon);
  } else if (isElectronOnly) {
    if(m_debug) Info("TriggerTool:getScaleFactor", "e event ntypes %i",nTypes);
    return getScaleFactorElectron(triggersElectron);
  } else if (isMuonOnly) {
    if(m_debug) Info("TriggerTool:getScaleFactor mu", "mu event ntypes %i",nTypes);
    return getScaleFactorMuon(triggersMuon);
  } else if (isMETOnly) {
    if(m_debug) Info("TriggerTool:getScaleFactor", "met event ntypes %i",nTypes);
    return getScaleFactorMET(triggersMET);
  } else if (isMETAndMuon) {
    if(m_debug) Info("TriggerTool:getScaleFactor", "met-muon event ntypes %i",nTypes);
    return getScaleFactorMETMuon(triggersMET,triggersMuon);
  } else {
    Warning("TriggerTool::getScaleFactor()",
            "Don't know how to handle trigger configuration: "
            "isElectron = %i, isPhoton = %i, isMuon = %i, "
            "isJet = %i, isMET = %i. Returning 1.",
            isElectron, isPhoton, isMuon, isJet, isMET);
  }

  return 1;
}

std::set<TriggerInfo*> TriggerTool::getTriggersOfObject(
        std::set<TriggerInfo*> triggers,
        TriggerInfo::TriggerObject object) {
  std::set<TriggerInfo*> filteredTrigs;
  for (TriggerInfo* trigger : triggers) {
    if (trigger -> getTriggerObject() == object) {
      filteredTrigs.insert(trigger);
    }
  }
  return filteredTrigs;
}

bool TriggerTool::checkRequirements(TriggerInfo* trigger) {
  
  TriggerInfo::DataType type = TriggerInfo::DataType::data;
  if (Props::isMC.get(m_eventInfo)) {
    type = TriggerInfo::DataType::MC;
  }
  if (!trigger -> appliesToType(type)) {
    return false;
  }
  
  TriggerInfo::TriggerObject object = trigger -> getTriggerObject();
  if (object == TriggerInfo::TriggerObject::Electron && !m_electrons.size()) return false;
  if (object == TriggerInfo::TriggerObject::Photon   && !m_photons.size()) return false;
  if (object == TriggerInfo::TriggerObject::Muon     && !m_muons.size()) return false;
  if (object == TriggerInfo::TriggerObject::Jet      && !m_jets.size()) return false;
  if (object == TriggerInfo::TriggerObject::MET      && !m_met) return false;

  return true;
}

bool TriggerTool::getMatch(TriggerInfo* trigger) {
  TriggerInfo::TriggerObject object = trigger -> getTriggerObject();

  if (m_debug) {
    Info("TriggerTool::getMatch()",
         "Trying to get trigger match for trigger '%s' and object type '%i'",
	 trigger -> getName().Data(),(int)object);
  }
  
  if (object == TriggerInfo::TriggerObject::Electron) {
    return getMatch<xAOD::Electron>(trigger, m_electrons);
  } else if (object == TriggerInfo::TriggerObject::Photon) {
    return getMatch<xAOD::Photon>(trigger, m_photons);
  } else if (object == TriggerInfo::TriggerObject::Muon) {
    return getMatch<xAOD::Muon>(trigger, m_muons);
  }
  
  Warning("TriggerTool::getMatch()",
          "Cannot handle trigger object of type %i for trigger '%s'! Returning false.",
          (int)object, trigger -> getName().Data());
  
  return false;
}

template <class partType>
bool TriggerTool::getMatch(TriggerInfo* trigger, std::vector<const partType*> particles) {
  int matches = 0;
  for (const partType* particle : particles) {
    if (!trigger -> getMatchProperty() -> get(particle)) {
      continue;
    }
    float threshold = trigger -> getThreshold();
    if (particle -> pt() < threshold * m_thresholdFactor) {
      continue;
    }
    matches++;
  }
  if (m_debug) {
    Info("TriggerTool::getMatch()",
         "Found %i matches for %lu valid particles.",
	 matches, particles.size());
  }
  return (bool) matches;
}

template <class partType>
std::vector<const partType*> TriggerTool::getValidParticles(std::vector<const partType*> particles) {
  std::vector<const partType*> validParts;
  for (const partType* particle : particles) {
    if (particle == nullptr) {
      continue;
    }
    validParts.push_back(particle);
  }
  return validParts;
}

int TriggerTool::getRunNumber() {

  int runNumber = -1;

  // Get the run number from the data/MC EventInfo
  int runNumberFile = -1;
  int eventNumberFile = -1;
  if(!Props::isMC.get(m_eventInfo)){ //data
    runNumberFile = m_eventInfo->runNumber();
    eventNumberFile = m_eventInfo->eventNumber();
  } else { //MC
    runNumberFile = Props::RandomRunNumber.get(m_eventInfo);
    eventNumberFile = m_eventInfo->eventNumber();
  } 

  static int lastEventNumber(-1);
  static int lastRunNumber(-1);

  bool newevt = (eventNumberFile != lastEventNumber);
  bool newrun = ((runNumberFile != lastRunNumber)&&(newevt));

  if (newevt || newrun) {
    m_nRuns++;
    // deal with zero run numbers
    if (runNumberFile==0) { 
      m_nZeroRuns++;
      if (m_lastNonZeroRun == 0) {
	Warning("TriggerTool::",
		"MC Run Number is zero and no previous run, set to data17");
	runNumber = 340453; 
	m_nHardResetZeroRuns++;
      } else {
	Warning("TriggerInfo::getRunNumber()",
		"MC Run Number is zero setting to previous run '%i'!",  m_lastNonZeroRun);
	runNumber = m_lastNonZeroRun;
	m_nResetZeroRuns++;
      }
    } else {
      runNumber = runNumberFile;
      m_lastNonZeroRun = runNumber;  // set for non-zero run numbers only
    }
    lastEventNumber=eventNumberFile;
    lastRunNumber=runNumber;
  } else {
    runNumber=lastRunNumber;
  }

  if (m_debug) 
    Info ("TriggerTool::getRunNumber", "Run number = %i,%i,%i,%i,%i", runNumber, eventNumberFile, runNumberFile, lastEventNumber, lastRunNumber );
  return runNumber;
}

double TriggerTool::getScaleFactorElectron(std::set<TriggerInfo*> triggers) {

  if (!m_electronEffProp || !m_electronSFProp) {
    Warning("TriggerTool::getScaleFactorElectron()",
            "Electron efficiency / scale-factor properties not set! Returning 1.");
    return 1;
  }

  // TODO how to check validity for passed triggers?
  double totalIneffScaled=1;
  double totalIneff=getInefficiencyElectron(totalIneffScaled);

  double scaleFactor = (1 - totalIneffScaled) / (1 - totalIneff);
  if (m_debug) {
    Info("TriggerTool::getScaleFactorElectron()",
            "Scalefactor is %f for %lu valid electrons and %lu passed triggers.",
            scaleFactor, m_electrons.size(), triggers.size());
  }
  return scaleFactor;
}
double TriggerTool::getScaleFactorMuon(std::set<TriggerInfo*> triggers) {
  
  int runNumber = getRunNumber();

  TString triggerCombo=getMuonTriggerName(triggers, runNumber);
 
  ConstDataVector<xAOD::MuonContainer> selectedMuons(SG::VIEW_ELEMENTS);
  for (const xAOD::Muon* muon: m_muons) {
    selectedMuons.push_back(muon);
  }
  double scaleFactor = 1;
  if (m_debug) {
    Info("TriggerTool::getScaleFactorMuon()",
            "Before Scale factor is %f for %lu valid muons and %lu passed triggers.",
            scaleFactor, m_muons.size(), triggers.size());
  }

  CP_CHECK("TriggerTool::getScaleFactorMuon()",
          m_muonSFTool->getTriggerScaleFactor(*selectedMuons.asDataVector(),
          scaleFactor, triggerCombo.Data()), m_debug);
  if (m_debug) {
    Info("TriggerTool::getScaleFactorMuon()",
            "Scale factor is %f for %lu valid muons and %lu passed triggers.",
            scaleFactor, m_muons.size(), triggers.size());
  }

  return scaleFactor;
}


//triggersel not used, so commenting it out
//double TriggerTool::getScaleFactorElectronMuon(std::set<TriggerInfo*> triggersel, std::set<TriggerInfo*> triggersmu) {
double TriggerTool::getScaleFactorElectronMuon(std::set<TriggerInfo*> triggersmu) {

  //Scale factor for an OR of the electron and muon triggers

  if (!m_electronEffProp || !m_electronSFProp) {
    Warning("TriggerTool::getScaleFactorElectronMuon()",
            "Electron efficiency / scale-factor properties not set! Returning 1.");
    return 1;
  }

  double totalIneffScaled=1;
  double totalIneff=getInefficiencyElectron(totalIneffScaled);

  if (m_debug) {
    Info("TriggerTool::getScaleFactorElectronMuon()",
	 "After electrons totalIneff=%f, totalIneffScaled=%f ",totalIneff,totalIneffScaled );
  }

  double totalIneffScaledMu=1;

  totalIneff *= getInefficiencyMuon(totalIneffScaledMu, triggersmu);
  totalIneffScaled *=totalIneffScaledMu;

  if (m_debug) {
    Info("TriggerTool::getScaleFactorElectronMuon()",
	 "After muons totalIneff=%f, totalIneffScaled=%f ",totalIneff,totalIneffScaled );
  }

  double scaleFactor = (1 - totalIneffScaled) / (1 - totalIneff);
  if (m_debug) {
    Info("TriggerTool::getScaleFactorElectronMuon()",
	 "Scalefactor is %f for %lu valid electrons and %lu passed muons.",
	 scaleFactor, m_electrons.size(), m_muons.size());
  }

  return scaleFactor;
}


double TriggerTool::getScaleFactorMET(std::set<TriggerInfo*> triggers) {
  
  if (!m_METscaleFactorTool) {
    Warning("TriggerTool::getScaleFactorMET", "MET scale factor tool not properly initialized!");
  }
  if (triggers.size() != 1) {
    Warning("TriggerTool::getScaleFactorMET",
            "Can only handle 1 trigger, not %lu. Returning 1.", triggers.size());
    return 1;
  }
  
  TString triggerName = (*triggers.begin())->getName();
  TString variation = m_currentVariation.name();
  double MET_value = m_met -> met();
  double scaleFactor = m_METscaleFactorTool -> get_metTrigSF(MET_value, m_sumpt, variation.Data(), triggerName.Data());
  if (m_debug) {
    Info("TriggerTool::getScaleFactorMET()",
            "Scale factor is %f for trigger '%s' and variation '%s'.",
            scaleFactor, triggerName.Data(), variation.Data());
  }

  return scaleFactor;
}

double TriggerTool::getScaleFactorMETMuon(std::set<TriggerInfo*> triggersMET, std::set<TriggerInfo*> triggersmu) {
  
  //Trigger efficiency for an OR of the MET trigger and the muon trigger (for 1 and 2 lepton analyses)

  if (!m_METscaleFactorTool) {
    Warning("TriggerTool::getScaleFactorMETMuon", "MET scale factor tool not properly initialized!");
  }
  if (triggersMET.size() != 1) {
    Warning("TriggerTool::getScaleFactorMETMuon",
            "Can only handle 1 trigger, not %lu. Returning 1.", triggersMET.size());
    return 1;
  }

  double totalIneffScaled=1;
  double totalIneff=getInefficiencyMET(totalIneffScaled, triggersMET);

  if (m_debug) {
    Info("TriggerTool::getScaleFactorMETMuon()",
	 "After MET totalIneff=%f, totalIneffScaled=%f ",totalIneff,totalIneffScaled );
  }

  double totalIneffScaledMu=1;

  totalIneff *= getInefficiencyMuon(totalIneffScaledMu, triggersmu);
  totalIneffScaled *=totalIneffScaledMu;

  if (m_debug) {
    Info("TriggerTool::getScaleFactorElectronMuon()",
	 "After muons totalIneff=%f, totalIneffScaled=%f ",totalIneff,totalIneffScaled );
  }

  double scaleFactor = (1 - totalIneffScaled) / (1 - totalIneff);
  if (m_debug) {
    Info("TriggerTool::getScaleFactorMETMuon()",
	 "Scalefactor is %f for MET=%f and %lu passed muons.",
	 scaleFactor, m_met -> met(), m_muons.size());
  }

  return scaleFactor;
}

void TriggerTool::validateIneff(double& ineff, double& ineffScaled) {
  bool valid = true;
  if(ineff < 0 || ineffScaled < 0) valid = false;
  if(ineff >= 1 || ineffScaled >= 1) valid = false;
  if (!valid) {
    Warning("TriggerTool::validateIneff()",
            "Invalid trigger inefficiency (scaled) of %f (%f). Setting both to 0.",
            ineff, ineffScaled);
    ineff = 0;
    ineffScaled = 0;
  }
}

double TriggerTool::getInefficiencyMET(double& totalIneffScaled, std::set<TriggerInfo*> triggersMET) {
  double totalIneff=1;
  totalIneffScaled=1;

  TString triggerName = (*triggersMET.begin())->getName();
  TString variation = m_currentVariation.name();
  double MET_value  = m_met -> met();

  double sf = m_METscaleFactorTool -> get_metTrigSF(MET_value, m_sumpt, variation.Data(), triggerName.Data());
  double eff = m_METscaleFactorTool -> get_metTrigEff(MET_value, triggerName.Data());

  totalIneff = (1 - eff);
  totalIneffScaled = (1 - eff * sf);

  validateIneff(totalIneff, totalIneffScaled);
  return totalIneff;
}


double TriggerTool::getInefficiencyElectron(double& totalIneffScaled) {
  double totalIneff=1;
  totalIneffScaled=1;

  for (const xAOD::Electron* electron: m_electrons) {
    double eff = m_electronEffProp->get(electron);
    double sf = m_electronSFProp->get(electron);
    totalIneff *= (1 - eff);
    totalIneffScaled *= (1 - eff * sf);
  }

  validateIneff(totalIneff, totalIneffScaled);
  return totalIneff;
}


double TriggerTool::getInefficiencyMuon(double& totalIneffScaled, std::set<TriggerInfo*> triggersmu) {
  double totalIneff=1;
  totalIneffScaled=1;

  int runNumber = getRunNumber();
  TString triggerCombo=getMuonTriggerName(triggersmu, runNumber);

  //This code assumes each muon has an independent efficiency
  ConstDataVector<xAOD::MuonContainer> selectedMuons(SG::VIEW_ELEMENTS);
  for (const xAOD::Muon* muon: m_muons) {
    selectedMuons.clear();
    selectedMuons.push_back(muon);
    double eff=0;
    double sf=0;

    CP_CHECK("TriggerTool::getInefficiencyMuon() muon trigger efficiency",
	     m_muonSFTool->getTriggerEfficiency(*muon,eff,triggerCombo.Data(),false), m_debug);
    if (m_debug) {
      Info("TriggerTool::getInefficiencyMuon()",
            "Before per muon Scale factor and trig eff for muon is %f:", eff); 
    }


    CP_CHECK("TriggerTool::getInefficiencyMuon() muon scale factor",
	     m_muonSFTool->getTriggerScaleFactor(*selectedMuons.asDataVector(),
			       sf, triggerCombo.Data()), m_debug);
    totalIneff *= (1 - eff);
    totalIneffScaled *= (1 - eff * sf);
  }

  validateIneff(totalIneff, totalIneffScaled);
  return totalIneff;
}


TString TriggerTool::getMuonTriggerName(std::set<TriggerInfo*> triggersmu, int runNumber){

  TriggerInfo::DataPeriod period = TriggerInfo::getDataPeriodFromRun(runNumber);

  // TODO is there a less hard-coded way?
  TString triggerCombo = "";
  if      (period == TriggerInfo::DataPeriod::data15)      triggerCombo = "HLT_mu20_iloose_L1MU15_OR_HLT_mu50";
  else if (period == TriggerInfo::DataPeriod::data16A)     triggerCombo = "HLT_mu26_ivarmedium_OR_HLT_mu50";
  else if (period == TriggerInfo::DataPeriod::data16BD3)   triggerCombo = "HLT_mu26_ivarmedium_OR_HLT_mu50";
  else if (period == TriggerInfo::DataPeriod::data16D4E3)  triggerCombo = "HLT_mu26_ivarmedium_OR_HLT_mu50";
  else if (period == TriggerInfo::DataPeriod::data16F1)    triggerCombo = "HLT_mu26_ivarmedium_OR_HLT_mu50";
  else if (period == TriggerInfo::DataPeriod::data16F2L11) triggerCombo = "HLT_mu26_ivarmedium_OR_HLT_mu50";
  else if (period == TriggerInfo::DataPeriod::data17)      triggerCombo = "HLT_mu26_ivarmedium_OR_HLT_mu50";
  else if (period == TriggerInfo::DataPeriod::data18)      triggerCombo = "HLT_mu26_ivarmedium_OR_HLT_mu50"; 
  else {
    Warning("TriggerTool::getScaleFactorElectronMuon",
            "Muon trigger combination not defined for data period %i!", (int)period);
  }
  
  for (TriggerInfo* trigger: triggersmu) {
    if (!triggerCombo.Contains(trigger->getName())) {
      Warning("TriggerTool::getScaleFactorElectronMuon()",
              "Trigger '%s' does not appear in combination '%s'!",
              trigger->getName().Data(), triggerCombo.Data());
    }
  }
  return triggerCombo;
}
  
