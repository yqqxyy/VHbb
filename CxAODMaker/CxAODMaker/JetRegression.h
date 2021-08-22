/*
 * JetRegression.h
 *
 *  Created on: Apr 20, 2015
 *      Author: eschopf
 */

#ifndef JETREGRESSION_H_
#define JETREGRESSION_H_

// infrastructure includes
class ElectronHandler;
class MuonHandler;
class JetHandler;
class EventInfoHandler;
class TruthProcessor;
class TrackJetHandler;
class TruthJetHandler;

class ITMVATrainingTool;
class ITMVAApplicationTool;

class JetRegressionVars_t;
class JetRegressionVarList_t;
class JetRegressionVarHelper;

#include "CxAODMaker/JetRegressionVars_t.h"
#include "CxAODMaker/JetRegressionVarHelper.h"


class ObjectHandlerBase;
namespace Ov {
  class IAnalysisMain;
}

class JetRegression {

protected:

	ConfigStore & m_config;
	bool m_debug;

	JetHandler* m_jets;
	TrackJetHandler* m_trackjets;
	MuonHandler* m_muons;
	ElectronHandler* m_electrons;
	TruthProcessor* m_truthprocessor;
	TruthJetHandler* m_truthjets;
	EventInfoHandler* m_eventinfo;

	std::vector<int> m_partonIndices;

	Ov::IAnalysisMain *m_analysisContext;
	std::unique_ptr<ITMVATrainingTool> m_training;
	std::unique_ptr<ITMVAApplicationTool> m_mva;
	std::string m_trainingToolName;
	std::string m_mvaToolName;

	JetRegressionVarHelper m_varHelper;
	JetRegressionVarList_t m_inputVars;

	bool m_doTraining;
	bool m_doEvenOdd;
	bool m_doPtSplit;

	int m_bJets;
	int m_truthlabeledB;
	int m_muInJet;
	int m_elInJet;
	int m_assocTrack;

	int m_2jetevts;

	TH1* m_mBB_truthparton_histo;
	TH1* m_mBB_truthjet_histo;
	TH1D* m_muInJet_ptHisto;
	TH1D* m_muInJet_etaHisto;
	TH1D* m_muInJet_authorHisto;

	float et(const xAOD::IParticle* part){
		return part->e()*TMath::Sin(JetRegression::theta(part));
	}
	float theta(const xAOD::IParticle* part){
		return TMath::ACos(TMath::TanH(part->eta()));
	}
	float mt(const xAOD::IParticle* part){
		return TMath::Sqrt(JetRegression::et(part)*JetRegression::et(part) - part->pt()*part->pt());
	}
	std::vector<int> findPartonsFromHiggs(const xAOD::TruthParticleContainer* truthParticles);
	int matchTrueBQuark(const xAOD::Jet* jet, const xAOD::TruthParticleContainer* truthParticle);
	int nTrueBInJet(const xAOD::Jet* jet, const xAOD::TruthParticleContainer* truthParticles);

	int associateTruthJet(const xAOD::Jet* jet, const xAOD::JetContainer* truthJets);

	std::vector<const xAOD::Jet*> findCloseByJets(const xAOD::Jet* jet, const xAOD::JetContainer* truthJets);

public:

	JetRegression() = delete;

	JetRegression(ConfigStore & config);

	virtual ~JetRegression();

	virtual void setJets(JetHandler* jets);

	virtual void setTrackJets(TrackJetHandler* trackjets);

	virtual void setTruthProcessor(TruthProcessor* truthProcessor);

	virtual void setEventInfo(EventInfoHandler* eventinfo);

	virtual void setTruthJets(TruthJetHandler* truthJets);

	virtual void initialize(Ov::IAnalysisMain &context);

	virtual void finalize();

	virtual void setMuons(MuonHandler* muons);

	virtual void setElectrons(ElectronHandler* electrons);

	virtual void applyRegression();

};

#endif /* JETREGRESSION_H_ */
