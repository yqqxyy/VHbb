// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_ObjectHandlerBase_H
#define CxAODMaker_ObjectHandlerBase_H

#include <vector>
#include <set>
#include <TString.h>

// Analysis includes
#include "CxAODTools/ConfigStore.h"

// infrastructure includes
#include "EventLoop/StatusCode.h"
#include "PATInterfaces/ISystematicsTool.h"

#include "TError.h"

#include "xAODRootAccess/TEvent.h"

#include "CxAODTools/CutFlowCounter.h"

namespace xAOD {
  class IParticle;
}

class ParticleLinker;

class ObjectHandlerBase {

protected:
  
  // Name of the handler
  std::string m_handlerName;

  // steering file
  ConfigStore & m_config;
  bool m_debug;
  
  // message level for CP tools
  MSG::Level m_msgLevel;

  // xAOD event
  xAOD::TEvent * m_event;

  // event number to investigate
  int m_EventNumberToInvestigate;

  // name of container in xAOD
  std::string m_containerName;

  // list of weight variations
  std::set<TString> m_weightVariations;
  
  // master switch for object filtering
  bool m_applyObjectSelection;

  // flag to store original (non-calibrated) objects
  bool m_storeOrig;

  // list of CP tool, which are affected by systematics
  std::vector<CP::ISystematicsTool *> m_sysToolList;

  // list of CP systematics
  std::vector<CP::SystematicSet> m_sysList;

  // counters
  std::map<TString, long> m_inObjectsCount;
  std::map<TString, long> m_inObjectsPreSel;
  std::map<TString, long> m_inObjectsSelected;
  std::map<TString, long> m_outObjectsCount;

  // Cutflow 
  CutFlowCounter *m_cutflow;
  std::map<TString, CutFlowCounter*> m_cutflowMap;

  // Particle linking
  ParticleLinker* m_linker; //!
  std::vector<std::string> m_linkNames;
  bool m_usedForElementLinks;
  bool m_addPreselJets; // TODO this should really be somewhere else
  bool m_createElementLinks;

  // flag to store whether this particular Handler is used for OR
  bool m_usedInOR;

  // flag to store variables that are not crucial for the analysis result
  bool m_saveNonCrucialVariables;

public:

  /**
   * @brief Constructor..
   */
  ObjectHandlerBase(const std::string& name, ConfigStore & config, xAOD::TEvent * event);

  /**
   * @brief Destructor.
   */
  virtual ~ObjectHandlerBase();

  /**
   * @brief Retrieve physics objects from event.
   * @return Status code.
   */
  virtual EL::StatusCode setObjects() = 0;

  /**
   * @brief Calibration/selection/... tools are initialized.
   * @return Status code.
   */
  virtual EL::StatusCode initializeTools() = 0;
  
  /**
   * @brief Get the name of this handler.
   * @return The name.
   */
  const std::string& name() const { return m_handlerName; }

  /**
   * @brief Add systematic variations.
   * @return Status code.
   *
   * Add systematic variations to the internal list.
   * If filterByTools=true only those that affect the registered CP tools
   * are added.
   *
   * A preceding call @c initializeTools() is required.
   */
  virtual EL::StatusCode addCPVariations(
        const std::vector<TString> &variations,
        const bool filterByTools = true, const bool skipWeightVar = false);

  /**
   * @brief Get all variations that affect this handler.
   * @return The list of all systematics
   * 
   * This includes "Nominal" if requested and "Original" if available .
   */
  virtual std::vector<TString> getAllVariations(bool includeNominal = true);

  /**
   * @brief Print all CP variations.
   * 
   * Print all variations to be run on in this handler and those available
   * in the registered ISystematicTools.
   */
  virtual void printAllVariations();
  
  /**
   * @brief Get the cut flow counter of this handler.
   * @return The cut flow counter.
   */
  virtual const std::map<TString, CutFlowCounter*> getCutFlowCounter() const = 0;

  /**
   * @brief Calibrate physics objects.
   * @return Status code.
   */
  virtual EL::StatusCode calibrate() = 0;

  /**
   * @brief Run selection on physics objects.
   * @return Status code.
   */
  virtual EL::StatusCode select() = 0;

  /**
   * @brief Set passSel flags on input objects based on passPreSel, passORGlob
   * @return Status code.
   */
  virtual EL::StatusCode setPassSelFlags();

  /**
   * @brief Set passSel flags on input objects that are linked to
   * @return Status code.
   */
  virtual EL::StatusCode setPassSelForLinked();

  /**
   * @brief Copy selected physics objects to output container.
   * @return Status code.
   */
  virtual EL::StatusCode fillOutputContainer() = 0;
  
  /**
   * @brief Set the particle linker.
   */
  void setLinker(ParticleLinker* linker);

  /**
   * @brief Use the references stored in m_linker to create new
   * @return Status code.
   * ElementLinks in the output object. These reference the newly
   * created equivalent of the original link. 
   */
  virtual EL::StatusCode fillOutputLinks();
  
  /**
   * @brief Use the references stored in m_linker to create new
   * @return Status code.
   * ElementLinks in the output object. These reference the newly
   * created equivalent of the original link. 
   */
  virtual EL::StatusCode flagOutputLinks();

  /**
   * @brief Delete any owned objects created during the event.
   * @return Status code.
   */
  virtual EL::StatusCode clearEvent() = 0;

  /**
   * @brief count size of physics objects.
   * @return Status code.
   *
   * count the size of the input/output containers
   */
  virtual void countObjects() = 0;

  /**
   * @brief print the counted physics objects.
   * @return Status code.
   *
   * print the total size of the input/output containers
   */
  virtual void printObjectCounts();
  
  /**
   * @brief Add cutflow counters.
   * @return Status code.
   */
  virtual EL::StatusCode addCutflows();

  /**
   * @brief Print 4-vector information of object particle to screen.
   * @param particle.
   */
  void printParticle(xAOD::IParticle* particle);

};

#endif
