// Dear emacs, this is -*-c++-*-
//
/*
 * \class TruthProcessor
 *
 * \ingroup CxAODMaker
 *
 * \brief This implements a base class for reading and decorating truth-related objects.
 *
 */

#ifndef CxAODMaker_TruthProcessor_H
#define CxAODMaker_TruthProcessor_H

#include <vector>
#include <set>
#include <TString.h>
#include <string>

// Analysis includes
#include "CxAODTools/ConfigStore.h"

// infrastructure includes
#include "EventLoop/StatusCode.h"

#include "TError.h"

#include "xAODRootAccess/TEvent.h"

#include "CxAODTools/CutFlowCounter.h"

// Analysis includes
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

#include "xAODTruth/TruthParticleContainer.h"

class TruthProcessor {
 
public:

  /// \brief Constructor
  /// \param name Name of the class
  /// \param config The object used to access the configuration database
  /// \param event The event store
  /// \param eventInfoHandler The handler used to access the EventInfo object per event.
  TruthProcessor(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                       EventInfoHandler & eventInfoHandler);
  
  /// \brief Destructor
  virtual ~TruthProcessor();

  /// \brief Get input container.
  /// \param containerName Name of the container.
  /// \return Input container
  const xAOD::TruthParticleContainer *getInParticle(const std::string &containerName);

  /// \brief Main member function: to be called once per event. It reads the input containers
  ///        and makes shallow copies.
  /// \return Status code.
  virtual EL::StatusCode setObjects();

  /// \brief Second main member function: to be called once per event. Select relevant particles to be saved.
  /// \return Status code.
  virtual EL::StatusCode select();

  /// \brief Copy four-momentum from input to output particle.
  /// \param in Input particle.
  /// \param out Output particle.
  virtual void setP4(const xAOD::TruthParticle * in, xAOD::TruthParticle * out);

  /// \brief Add decorations in the particles.
  virtual void decorate();

  /// \brief Fill output container from input shallow copies.
  /// \param outPartCont Container to be filled.
  /// \param inContainerName Name of the input container.
  virtual void fillOutputContainer(xAOD::TruthParticleContainer * outPartCont, const std::string &inContainerName);

  /// \brief Fill output container from input shallow copies.
  /// \return Status code.
  virtual EL::StatusCode fillOutputContainer();

  /// \brief Copy information from the input particle to the output one.
  /// \param inPart Input particle from the derivation.
  /// \param outPart Output particle to be written in the CxAOD.
  /// \return Status code.
  virtual EL::StatusCode writeOutputVariables(xAOD::TruthParticle * inPart, xAOD::TruthParticle * outPart);

  /// \brief Copy extra information from the input particle to the output one. Called by writeOutputVariables.
  /// \param inPart Input particle from the derivation.
  /// \param outPart Output particle to be written in the CxAOD.
  /// \return Status code.
  virtual EL::StatusCode writeCustomOutputVariables(xAOD::TruthParticle * inPart, xAOD::TruthParticle * outPart);

  /// \brief Check whether to write the particle in the CxAOD.
  /// \param part Particle to consider.
  /// \param containerName Container where the particle was.
  /// \return Whether to write it in the CxAOD.
  virtual bool passTruthParticle(xAOD::TruthParticle * part, const std::string &containerName);

  /// \brief Called by passTruthParticle. Should be overwritten.
  /// \param part Particle to consider.
  /// \param containerName Container where the particle was.
  /// \return Whether to write it in the CxAOD.
  virtual bool passCustomTruthParticle(xAOD::TruthParticle * part, const std::string &containerName);

  /// \brief Delete any owned objects created during the event.
  /// \return Status code.
  virtual EL::StatusCode clearEvent();

  /// \brief Print the particle information
  /// \param part Particle to be printed out.
  void print(const xAOD::TruthParticle * part);

  /// \brief Print the decay tree of a particle.
  /// \param part The particle.
  /// \param depth How deep to look in the decay chain.
  /// \param statusOnly Whether to print only the status code.
  void printDecayTree(const xAOD::TruthParticle * part, int depth = 0, bool statusOnly = false);

  /// \brief Print the full decay tree.
  /// \param part The particle.
  void printFullTree(const xAOD::TruthParticle * part);

  /// \brief Compute code identifying ttbar decay type and return it by reference using the TruthTopQuarkWithDecayParticles object.
  ///        This should be faster than scanning the TruthParticles tree, but the old getCodeTTBarDecay function is left for reference.
  ///        The return codes are as follows:
  ///          0 -> jet jet
  ///          1 -> jet e
  ///          2 -> jet mu
  ///          3 -> jet tau
  ///          4 -> e   e
  ///          5 -> mu  mu
  ///          6 -> tau tau
  ///          7 -> e   mu
  ///          8 -> e   tau
  ///          9 -> mu  tau
  ///          The return code is negative in case of errors:
  ///          -1 -> The code was not able to reconstruct all the decay products of the ttbar event.
  ///          -2 -> The W doesn't have any parents.
  ///          -3 -> The W doesn't have a decay vertex.
  ///          -4 -> could not find W boson in the boson container
  ///          -5 -> could not find tau in the TruthTaus
  /// \param codeTTBarDecay Integer identifying ttbar decay channel to be returned by reference.
  /// \param final_pt Final pT to be returned by reference.
  /// \param final_ptl Final pT of the lepton to be returned by reference.
  /// \param final_etal Final pT of the lepton to be returned by reference.
  /// \return Success or failure.
  EL::StatusCode getTTBarDecayFromTruthTop(int & codeTTBarDecay, float & final_pt, float & final_ptl, float & final_etal);

  /// \brief Compute code identifying ttbar decay type and return it by reference.
  ///        The return codes are as follows:
  ///          0 -> jet jet
  ///          1 -> jet e
  ///          2 -> jet mu
  ///          3 -> jet tau
  ///          4 -> e   e
  ///          5 -> mu  mu
  ///          6 -> tau tau
  ///          7 -> e   mu
  ///          8 -> e   tau
  ///          9 -> mu  tau
  ///          The return code is negative in case of errors:
  ///          -1 -> The code was not able to reconstruct all the decay products of the ttbar event.
  ///          -2 -> The W doesn't have any parents.
  ///          -3 -> The W doesn't have a decay vertex.
  /// \param codeTTBarDecay Integer identifying ttbar decay channel to be returned by reference.
  /// \param final_pt Final pT to be returned by reference.
  /// \param final_ptl Final pT of the lepton to be returned by reference.
  /// \param final_etal Final eta of the lepton to be returned by reference.
  /// \return Success or failure.
  EL::StatusCode getCodeTTBarDecay(int & codeTTBarDecay, float & final_pt, float & final_ptl, float & final_etal);

  /// \brief Look for particle decays on which a particle decay on itself.
  /// \param part Particle to consider.
  /// \return Last particle with the same decay as this.
  const xAOD::TruthParticle * findLastParticleDecayingInItself(const xAOD::TruthParticle* part);

  /// \brief Store barcode of muons that came from a hadron decay to save them later.
  /// \return Success or failure.
  EL::StatusCode findMuonsFromHadronDecay();

  /// \brief Compute pT of the vector boson in Sherpa V+jets samples.
  /// \return pT of the vector boson in GeV.
  float ComputeSherpapTV();
  bool checkSherpaVZqqZbb();

  /// \brief Does the input file has the new truth information containers (TruthElectrons, TruthMuons, etc.)?
  /// \return True or false.
  bool isNewStyle();

  /// \brief Returns true if the particle is the first or last top from a decay chain. Only meaningful after a call to getCodeTTBarDecay.
  /// \return Whether this is a first or last top in the decay chain.
  bool isTopFromTtbar(const xAOD::TruthParticle *part);

  /// \brief Returns true if the particle is the first or last W from a decay chain. Only meaningful after a call to getCodeTTBarDecay.
  /// \return Whether this is a first or last W in the decay chain.
  bool isWFromTtbar(const xAOD::TruthParticle *part);

  /// \brief Returns true if the particle is a W decay product quark or lepton. Only meaningful after a call to getCodeTTBarDecay.
  /// \return Whether this is a W decay product quark or lepton.
  bool isWdecayFromTtbar(const xAOD::TruthParticle *part);

  /// \brief Returns true if the particle is a muon from a hadron decay. Only meaningful after a call to findMuonsFromHadronDecay.
  /// \return Whether this is a W decay product quark or lepton.
  bool isMuonFromHadronDecay(const xAOD::TruthParticle *part);

  ///
  /// \brief Get the name of this handler.
  /// \return The name.
  const std::string& name() const { return m_name; }


  /// \brief Is this a W boson?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a W.
  static bool isW(const xAOD::TruthParticle * part);

  /// \brief Is this a W or Z boson?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a W or Z.
  static bool isWorZ(const xAOD::TruthParticle * part);

  /// \brief Is this a Higgs boson?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a Higgs.
  static bool isHiggs(const xAOD::TruthParticle * part);

  /// \brief Is this a top boson?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a top.
  static bool isTop(const xAOD::TruthParticle * part);

  /// \brief Is this a tau lepton?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a tau.
  static bool isTau(const xAOD::TruthParticle * part);

  /// \brief Is this a Z boson?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a Z boson.
  static bool isZ(const xAOD::TruthParticle * part);

  /// \brief Is this a H35 particle?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a H35.
  static bool isH35(const xAOD::TruthParticle * part);

  /// \brief Is this a W or Z particle following Pythia standards?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a W or Z.
  static bool checkWorZPythia(const xAOD::TruthParticle * part);

  /// \brief Is this a Higgs particle following Pythia standards?
  /// \param part Input TruthParticle object.
  /// \return Whether part is a Higgs.
  static bool checkHiggsPythia(const xAOD::TruthParticle * part);

  /// \brief Is this an electroweak lepton?
  /// \param part Input TruthParticle object.
  /// \return Whether part is an electroweak lepton.
  bool isEWLepton(const xAOD::TruthParticle * part);

  /// \brief Get truth electrons from either TruthParticles or the special container, depending on which is available.
  /// \return Electron list.
  const xAOD::TruthParticleContainer *getElectrons();

  /// \brief Get truth muons from either TruthParticles or the special container, depending on which is available.
  /// \return Muon list.
  const xAOD::TruthParticleContainer *getMuons();

  /// \brief Get truth taus from either TruthParticles or the special container, depending on which is available.
  /// \return Tau list.
  const xAOD::TruthParticleContainer *getTaus();

  /// \brief Get truth neutrinos from either TruthParticles or the special container, depending on which is available.
  /// \return Neutrino list.
  const xAOD::TruthParticleContainer *getNeutrinos();

protected:

  /// \brief Add decorations in particle part
  /// \param part Particle.
  /// \param outPartCont Container to be filled.
  virtual void decorate(xAOD::TruthParticle * part, const std::string &containerName);


  /// \brief Check if a particle satisfy a property within a decay chain.
  /// \param part The particle.
  /// \param checkParent Property asked from the particle.
  /// \param allowStateChanges Whether the state can change.
  /// \return Whether a particle was found satisfying the conditions.
  bool isFromDecay(const xAOD::TruthParticle * part,
        bool(*checkParent)(const xAOD::TruthParticle*),
        bool allowStateChanges = true);
  
  /// \brief Decorate particle with extra information.
  /// \param part Particle to consider.
  /// \return Success or failure.
  bool setPassPresel(xAOD::TruthParticle * part);

  /// \brief Check if truth particle is good to be saved to output container
  /// \param inPart xAOD::TruthParticle from DxAOD to be saved to CxAOD
  /// \return bool dtermining if particle is good for storage in CxAOD
  bool ValidParticle( xAOD::TruthParticle *inPart);

  /// Name
  std::string m_name;

  /// steering file
  ConfigStore & m_config;

  /// xAOD event
  xAOD::TEvent * m_event;

  // event info
  EventInfoHandler & m_eventInfoHandler; //!

  // maps of < type, xAOD container >
  // shallow copies of input xAOD objects
  std::map<std::string, xAOD::TruthParticleContainer *> m_in;
  // selected objects for output xAOD
  // maps of < type, xAOD container >
  std::map<std::string, xAOD::TruthParticleContainer *> m_out;

  std::set<std::string> m_listOfContainers;

  std::vector<int> m_muonsFromHadronDecays;
  std::vector<int> m_tBarcode;
  std::vector<int> m_WBarcode;
  std::vector<int> m_WDecaysBarcode;

  bool m_first;
  bool m_newStyle;

  std::string m_selectionName;
};

#endif
