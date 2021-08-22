// Dear emacs, this is -*-c++-*-
#ifndef CxAODMaker_ObjectHandler_H
#define CxAODMaker_ObjectHandler_H

// Analysis includes
#include "CxAODMaker/ObjectHandlerBase.h"
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODTools/ParticleLinker.h"

#include <utility>
#include <iterator>

class EventInfoHandler;

// EDM includes (which rootcint doesn't like)
#ifndef __MAKECINT__

#include "xAODEventInfo/EventInfo.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODBase/IParticleHelpers.h"

#endif // not __MAKECINT__


template <class partContainer>
class ObjectHandler : public ObjectHandlerBase {

  public:
    typedef typename partContainer::value_type partTypePtr;
    typedef typename std::iterator_traits<partTypePtr>::value_type partType;


  protected:

  bool m_doCopyBeforeSyst;//!

  // event info
  EventInfoHandler & m_eventInfoHandler;//!

  // maps of < variation name, xAOD container >
  // shallow copies of input xAOD objects for calibration
  std::map<TString, partContainer *> m_inContainer;
  // selected objects for output xAOD and event processing
  std::map<TString, partContainer *> m_outContainer;

  // Selection property used to determine which objects are written to the output.
  PROP<int>* m_passProperty;

  /// List of functions used to apply selection on the objects
  std::vector< std::function<bool(partType*)> > m_selectFcns;

  /**
   * @brief Set 4-vector
   * @param input particle.
   * @param output particle.
   *
   * For unkown reasons, this is not a member of xAOD::IParticle (?!),
   * so we make an interface for it!
   */
  virtual void setP4(const partType * inPart, partType * outPart) = 0;

  /**
   * @brief decorate original object.
   * Due to xAOD EDM limitation, cannot modify existing variables, but can add new ones.
   * THis can be used to perform operations that are independent of object calibration
   * or of any systematic, but that may be costly.
   * @param particles Physics object in Input container
   * @return Status code.
   */
  virtual EL::StatusCode decorateOriginParticle(const partType * particle);

  /**
   * @brief decorate original object.
   * This can be used to perform operations that are independent of object calibration
   * or of any systematic, but that may be costly.
   * It is similar conceptually to decorateOriginParticle, except nonconst and takes the 
   * full container as input.
   * @param particles Physics object in Input container
   * @return Status code.
   */
  virtual EL::StatusCode decorateOriginParticles(partContainer * particles);

  /**
   * @brief decorate object
   * @param particles Physics object
   * @return Status code.
   */
  virtual EL::StatusCode decorate(partType * particle) = 0;

  /**
   * @calibrate Calibrate physics objects.
   * @param particles Physics objects
   * @param sysSet Systematic variation(s).
   * @return Status code.
   */
  virtual EL::StatusCode calibrateCopies(partContainer * particles, const CP::SystematicSet & sysSet) = 0;

  /**
   * @brief Select one physics object for any systematic variation.
   * @param partContList All objects for all systematic variations.
   * @param index Index of object for selection.
   * @param passFunction Selection function.
   * @return Decision.
   *
   * An iteration on all object containers is performed. For each
   * container the passFunction is called for the object at index.
   */
  bool passAny(std::map<TString, partContainer *> partContList,
	       int index,
	       std::function<bool(partType*)> passFunction);

  /**
   * @brief Select one physics object for any systematic variation.
   * @param partContList All objects for all systematic variations.
   * @param index Index of object for selection.
   * @param passProp Selection property flag.
   * @return Decision.
   *
   * An iteration on all object containers is performed. For each
   * container the passFunction is called for the object at index.
   */
  bool passAny(std::map<TString, partContainer *> partContList,
	       int index,
	       PROP<int>& passProperty);

  
private:
  /**
   * @brief Copy input objects to output container.
   * @param outPartCont Output container for systematic variations sysName.
   * @param sysName Systematic variation.
   *
   * The physics object container for variation sysName
   * is retrieved from m_inContainer and copied to outPartCont.
   *
   * For the nominal objects a new container outPartCont is created.
   * All objects from partContList, that passed
   * for any of the variations the selection, are added.
   * The objects are decorated with non-standard variables.
   *
   * For systematic variations shallow copies are created
   * on the nominal outPartCont container.
   * Variation dependent object variables are overwritten.
   */
  void fillOutputContainer(
		     partContainer * outPartCont,
		     TString sysName);

protected:
  /**
   * @brief Sets the default variables that are written to the output container.
   * @param inPart Object from which the variables are read.
   * @param outPart Object that is written to output container, for which variables are set.
   * @param isSysVar Bool setting if the values are systematically varied.
   * @param sysName TString name of the current variation.
   * @return StatusCode.
   *
   * This function is called in copyContainer and needs to be implemented
   * in the derived classes.
   *
   * For a nominal object the variables that should be written to the output are defined
   * and set.
   * Variation dependent object variables are overwritten.
   */
  virtual EL::StatusCode writeOutputVariables(partType * inPart, partType * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) = 0;

  /**
   * @brief Sets additional variables that are written to the output container.
   * @param inPart Object from which the variables are read.
   * @param outPart Object that is written to output container, for which variables are set.
   * @param isSysVar Bool setting if the values are systematically varied.
   * @param sysName TString name of the current variation.
   * @return StatusCode.
   *
   * This function is called in writeOutputVariables. It is meant to write additional
   * (analysis specific) variables, not implemented by writeOutputVariables.
   */
  virtual EL::StatusCode writeCustomVariables(partType * inPart, partType * outPart, bool isKinVar, bool isWeightVar, const TString& sysName) = 0;

  /**
   * @bried Sets passPreSel variable to true so that the default object handlers
   * can have OR applied.
   */
  virtual bool passPreSel(partType* part);

private:
  /**
   * @brief Check selection for particle
   * @return bool passed selection.
   *
   * The particle is asked for the passPreSel or the passORGlob flag.
   */
  bool checkPassSel(const partType* particle);

  /**
   * @brief Check selection for linked particle
   * @return bool passed selection.
   *
   * The particle is asked for the passSel flag.
   * Decision is or'ed/replaced with the particle being linked to.
   */
  bool checkPassLinked(const partType* particle);

protected:
  /**
   * @brief Get physics objects from given container.
   * @return Map of all objects for the requested variation.
   *
   * If the variation is not found, "Nominal" is returned.
   * If "Nominal" doesn't exist either, nullptr is returned.
   *
   * The objects are calibrated if @c calibrate() was called.
   * They are decorated with selection flags if select() was called.
   *
   * A preceding call @c setObjects() is required.
   */
  partContainer* getParticleVariation(
          std::map<TString, partContainer *> &container,
          const TString &variation);

public:

  /**
   * @brief Constructor.
   * @param config Steering configuration.
   * @param event For retrieving xAOD objects.
   */
  ObjectHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                EventInfoHandler & eventInfoHandler);

  /**
   * @brief Destructor.
   */
  virtual ~ObjectHandler() = default;

  /**
   * @brief Retrieve physics objects from event.
   * @return Status code.
   *
   * m_inMouns and m_outContainer are cleared.
   *
   * A shallow copy for each variation (nominal + systematics)
   * is created and stored in m_inContainer.
   *
   * A call preceding @c addCPVariations() is required for
   * systematics, the "Nominal" collections exists always.
   */
    EL::StatusCode setObjects() override;

  /**
   * @brief Calibration/selection/... tools are initialized.
   * @return Status code.
   *
   * Calibration tools are initialized and their list
   * of systematics is retrieved.
   */
  virtual EL::StatusCode initializeTools() override = 0;

  /**
   * @brief Get the cut flow counter of this handler.
   * @return The cut flow counter.
   */
  const std::map<TString, CutFlowCounter*> getCutFlowCounter() const override { return m_cutflowMap; }

  /**
   * @brief Calibrate physics objects.
   * @return Status code.
   *
   * The shallow copies in m_inContainer are calibrated for
   * each variation.
   *
   * A preceding call @c setObjects() is required.
   */
  virtual EL::StatusCode calibrate() override;

  /**
   * @brief Run selection on physics objects.
   * @return Status code.
   *
   * A selection is performed on for each physics object and
   * all variations in m_inContainer.
   * The result is stored as decorators for each object.
   *
   * A preceding call @c setObjects() is required.
   */
  virtual EL::StatusCode select() override;

  /**
   * @brief Set passSel flags on input objects based on passPreSel, passORGlob
   * @return Status code.
   */
  virtual EL::StatusCode setPassSelFlags() override;

  /**
   * @brief Set passSel flags on input objects that are linked to
   * @return Status code.
   */
  virtual EL::StatusCode setPassSelForLinked() override;

  virtual unsigned int getNInputParticles();

  /**
   * @brief Get input physics objects.
   * @return Map of all objects for all variations.
   *
   * The objects are calibrated if @c calibrate() was called.
   * They are decorated with selection flags if select() was called.
   *
   * A preceding call @c setObjects() is required.
   */
  std::map<TString, partContainer*>* getInParticles() {
    return &m_inContainer;
  }

  /**
   * @brief Get output physics objects.
   * @return Map of selected objects for all variations.
   *
   * The objects are calibrated if @c calibrate() was called.
   * They are decorated with selection flags.
   *
   * A preceding call @c fillOutputContainer() is required.
   */
  std::map<TString, partContainer*>* getOutParticles() { return &m_outContainer; }

  /**
   * @brief Get input physics objects.
   * @return All objects for the requested variation.
   *
   * See getParticleVariation() for behaviour.
   */
  partContainer* getInParticleVariation(const TString &variation) {
    return getParticleVariation(m_inContainer, variation);
  }

  /**
   * @brief Get output physics objects.
   * @return All objects for the requested variation.
   *
   * See getParticleVariation() for behaviour.
   */
  partContainer* getOutParticleVariation(const TString &variation) {
    return getParticleVariation(m_outContainer, variation);
  }

  /**
   * @brief Copy selected physics objects to output container.
   * @return Status code.
   *
   * Selected objects from m_inContainer are copied to m_outContainer.
   * Please see @c fillOutputContainerCheck() for details.
   *
   * A preceding call @c select() is required.
   */
  virtual EL::StatusCode fillOutputContainer() override;
  
  /**
   * @brief Delete any owned objects created during the event.
   * @return Status code.
   */
  virtual EL::StatusCode clearEvent() override = 0;


  /**
   * @brief count size of physics objects.
   * @return Status code.
   *
   * count the size of the input/output containers
   */
  void countObjects() override;

  /**
   * @brief Declare that this handler is used for OR
   *
   * The flag can be used to make decision on what objects should be written
   * in the output
   *
   * @param use	Whether the handler is used or not
   * @return Status code
   */
  void useForOR(bool use) {
    m_usedInOR = use;
  }
  
};

#endif
