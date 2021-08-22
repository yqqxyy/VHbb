#ifndef CxAODMaker_EventSelector_H
#define CxAODMaker_EventSelector_H

// infrastructure includes
#include "EventLoop/StatusCode.h"
// Tool handles
#include "AsgTools/AnaToolHandle.h"

class OverlapRemover;
class OverlapRemoval;

class ConfigStore;
class EventSelection;
class CutFlowCounter;

class ElectronHandler;
class ForwardElectronHandler;
class PhotonHandler;
class MuonHandler;
class TauHandler;
class DiTauJetHandler;
class JetHandler;
class FatJetHandler;
class TrackJetHandler;
class EventInfoHandler;
class TruthEventHandler;
class TruthProcessor;
class METHandler;

#include <set>
#include <vector>
#include <map>
#include "TString.h"


class ObjectHandlerBase;
class OverlapRegisterAccessor;
namespace CP {
  class MuonTriggerScaleFactors;
  class IIsolationSelectionTool;
  class IIsolationCloseByCorrectionTool;
}

class EventSelector {

protected:

  ConfigStore & m_config;
  bool m_debug;
  bool m_doCloseByIsoCorr;

  JetHandler* m_jets;
  FatJetHandler* m_fatjets;
  FatJetHandler* m_fatjetsAlt;
  TrackJetHandler* m_trackjets;
  TrackJetHandler* m_subjets;
  MuonHandler* m_muons;
  TauHandler* m_taus;
  DiTauJetHandler* m_ditaus;
  ElectronHandler* m_electrons;
  ForwardElectronHandler* m_forwardElectrons;
  PhotonHandler* m_photons;
  METHandler* m_met;
  METHandler* m_metTrack;
  METHandler* m_metMJTight;
  METHandler* m_metFJVT;
  METHandler* m_metMJMUTight;
  METHandler* m_metMJMiddle;
  METHandler* m_metMJLoose;
  EventInfoHandler* m_info;
  TruthEventHandler* m_truthevent;
  TruthProcessor* m_truthElectrons;
  TruthProcessor* m_truthMuons;
  TruthProcessor* m_truthNeutrinos;

  std::vector<std::string> m_weightVariations;
  bool m_doOR;
  OverlapRemover* m_OR;
  EventSelection* m_selection;
  OverlapRegisterAccessor * m_overlapRegAcc;
  std::map<TString, bool> m_selectionResult;
  asg::AnaToolHandle<CP::IIsolationSelectionTool> m_isoTool;
  asg::AnaToolHandle<CP::IIsolationCloseByCorrectionTool> m_isoCloseByTool;

  // can use TString? == operator not defined?
  std::set<std::string> m_systematics;

  bool m_isFirstCall;

  virtual EL::StatusCode fillSystematics();

  virtual EL::StatusCode fillSystematics(ObjectHandlerBase* obj);

  virtual bool performSelection(const TString& sysName);

  virtual bool passORandPreSel(const TString& sysName);

public:

  EventSelector() = delete;

  EventSelector(ConfigStore & config);

  virtual ~EventSelector();

  virtual EL::StatusCode initialize();

  void setOverlapRegisterAccessor(OverlapRegisterAccessor * overlapRegAcc);
  
  virtual void setOverlapRemoval(OverlapRemoval* OR);

  virtual void setJets(JetHandler* jets);

  virtual void setFatJets(FatJetHandler* jets);

  virtual void setFatJetsAlt(FatJetHandler* jets);

  virtual void setTrackJets(TrackJetHandler* jets);

  virtual void setCoMSubJets(TrackJetHandler* jets);

  virtual void setMuons(MuonHandler* muons);

  virtual void setTaus(TauHandler* taus);

  virtual void setDiTaus(DiTauJetHandler* ditaus);

  virtual void setElectrons(ElectronHandler* electrons);

  virtual void setForwardElectrons(ForwardElectronHandler* electrons);

  virtual void setPhotons(PhotonHandler* photons);

  virtual void setMET(METHandler* met);
  virtual void setMETTrack(METHandler* met);
  virtual void setMETMJTight(METHandler* met);
  virtual void setMETFJVT(METHandler* met);
  virtual void setMETMJMUTight(METHandler* met);
  virtual void setMETMJMiddle(METHandler* met);
  virtual void setMETMJLoose(METHandler* met);

  virtual void setEventInfo(EventInfoHandler* info);

  virtual void setTruthEvent(TruthEventHandler* truthevent);
  virtual void setTruthElectronHandler(TruthProcessor* truthElectrons);
  virtual void setTruthMuonHandler(TruthProcessor* truthMuons);
  virtual void setTruthNeutrinoHandler(TruthProcessor* truthNeutrinos);

  /**
   * @brief Set the event selection to apply
   *
   * @c this becomes the owner of the EventSelection object !
   *
   * @param[in] selection	The event selection to apply
   */
  virtual void setSelection(EventSelection* selection);
  virtual EventSelection* getSelection() const;


  virtual EL::StatusCode fillEventVariables();
  virtual EL::StatusCode performSelection(bool& pass);

  const CutFlowCounter& getCutFlowCounter() const;

  bool getSelectionResult(const TString & sysName) const;

protected :

  CP::MuonTriggerScaleFactors* m_trig_sfmuon;  // !

};

#endif
