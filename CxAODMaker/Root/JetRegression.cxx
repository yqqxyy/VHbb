/*
 * JetRegression.cxx
 *
 *  Created on: Apr 20, 2015
 *      Author: eschopf
 */

#include "CxAODMaker/ElectronHandler.h"
#include "CxAODMaker/MuonHandler.h"
#include "CxAODMaker/JetHandler.h"
#include "CxAODMaker/EventInfoHandler.h"
#include "CxAODMaker/TruthProcessor.h"
#include "CxAODMaker/JetRegressionVars_t.h"
#include "CxAODMaker/JetRegressionVarHelper.h"
#include "CxAODMaker/JetRegression.h"
#include "CxAODMaker/ObjectHandlerBase.h"
#include "CxAODMaker/TrackJetHandler.h"
#include "CxAODMaker/TruthJetHandler.h"

#include "CxAODTools/ITMVATrainingTool.h" 
#include "CxAODTools/ITMVAApplicationTool.h"
#include "CxAODTools/OvCompat.h"


JetRegression::JetRegression(ConfigStore& config):
  m_config(config),
  m_debug(false),
  m_jets(nullptr),
  m_trackjets(nullptr),
  m_muons(nullptr),
  m_electrons(nullptr),
  m_truthprocessor(nullptr),
  m_truthjets(nullptr),
  m_eventinfo(nullptr),
  m_analysisContext(nullptr),
  m_training(nullptr),
   m_mva(nullptr),
   m_trainingToolName("TMVATrainingTool"),
   m_mvaToolName("TMVAApplicationTool"),
   m_doTraining(false),
   m_doEvenOdd(false)
{
    m_config.getif<bool>("JetRegression.doTraining",m_doTraining);
    m_config.getif<bool>("JetRegression.doEvenOdd",m_doEvenOdd);
    m_config.getif<bool>("JetRegression.doPtSplit",m_doPtSplit);
    m_config.getif<std::string>("JetRegression.TMVATrainingToolName",m_trainingToolName);
    m_config.getif<std::string>("JetRegression.TMVAApplicationToolName",m_mvaToolName);

}

JetRegression::~JetRegression() {

}

void JetRegression::setJets(JetHandler* jets) {
	  m_jets = jets;
}

void JetRegression::setTrackJets(TrackJetHandler* trackjets) {
	  m_trackjets = trackjets;
}

void JetRegression::setMuons(MuonHandler* muons) {
	  m_muons = muons;
}

void JetRegression::setElectrons(ElectronHandler* electrons) {
	  m_electrons = electrons;
}

void JetRegression::setTruthProcessor(TruthProcessor* truthProcessor) {
	m_truthprocessor = truthProcessor;
}

void JetRegression::setTruthJets(TruthJetHandler* truthJets) {
	m_truthjets = truthJets;
}

void JetRegression::setEventInfo(EventInfoHandler* eventinfo) {
	m_eventinfo = eventinfo;
}

//this initializes the tmva tools
void JetRegression::initialize(Ov::IAnalysisMain &context) {

	m_truthlabeledB = 0;
	m_bJets = 0;
	m_muInJet = 0;
	m_elInJet =0;
	m_assocTrack = 0;
	
	m_2jetevts = 0;

	//old monitoring histograms
//	m_mBB_truthparton_histo = nullptr;
//	m_mBB_truthparton_histo = new TH1D("mbb_parton", "mbb_parton", 100, 0., 250000);
//	m_mBB_truthjet_histo = nullptr;
//	m_mBB_truthjet_histo = new TH1D("mbb_jet", "mbb_jet", 100, 0., 250000);
//	m_muInJet_ptHisto = nullptr;
//	m_muInJet_ptHisto = new TH1D("muInJet_pt","muInJet_pt", 50, 0, 100000);
//	m_muInJet_etaHisto = nullptr;
//	m_muInJet_etaHisto = new TH1D("muInJet_eta","muInJet_eta", 50, -2.5, 2.5);
//	m_muInJet_authorHisto = nullptr;
//	m_muInJet_authorHisto = new TH1D("muInJet_author","muInJet_author", 20, 0, 20);

	m_analysisContext=&context;

	m_training.reset();
	  m_mva.reset();
	  if (m_doTraining) {
	    ToolKitManager::createAndSet( m_trainingToolName, &context, m_training );
	    // initialize / finalize of Overkill tools needs to be called manually
	    m_varHelper.setup( m_training->getVariableList(), m_inputVars );
	    m_training->initialize(&context);

	  }
	  else {
	    ToolKitManager::createAndSet( m_mvaToolName, &context, m_mva );
	    // initialize / finalize of Overkill tools needs to be called manually
	    m_mva->initialize(&context);
	    m_varHelper.setup( m_mva->getVariableList(), m_inputVars );

	  }

}

//finalize tmva tools
void JetRegression::finalize() {

	if(m_training){
		Info("JetRegression::finalize()","Object counts for JetRegression:");
		std::cout << "\t signal jets:\t" << m_bJets << std::endl;
		std::cout << "\t jets with associated truth b-quark:\t" << m_truthlabeledB << std::endl;
		std::cout << "\t jets with tracks:\t" << m_assocTrack << std::endl;
		std::cout << "\t jets with muon inside:\t" << m_muInJet << std::endl;
		std::cout << "\t jets with electrons inside:\t" << m_elInJet << std::endl;
	}

	//old monitoring histograms
//	m_mBB_truthparton_histo->SaveAs("truthpartonMbb.root");
//	m_mBB_truthjet_histo->SaveAs("truthjetMbb.root");
//	m_muInJet_ptHisto->SaveAs("muInJet_pt.root");
//	m_muInJet_etaHisto->SaveAs("muInJet_eta.root");
//	m_muInJet_authorHisto->SaveAs("muInJet_author.root");


	if (m_analysisContext) {
	        Ov::IAnalysisMain &context(*m_analysisContext);
		if (m_training) {
			m_training->finalize( &context );
		}
		if (m_mva) {
			m_mva->finalize( &context );
		}
	}

}

//this calculates the new jet pt and stores it in  the jet decoration variable ptCorr

void JetRegression::applyRegression(){

	if(!m_jets) throw std::runtime_error("JetRegression::applyRegression() No jet handler." );
	if(!m_trackjets) throw std::runtime_error("JetRegression::applyRegression() No track jet handler." );
	if(!m_muons) throw std::runtime_error("JetRegression::applyRegression() No muon handler." );
	if(!m_electrons) throw std::runtime_error("JetRegression::applyRegression() No electron handler." );
	if(!m_eventinfo) throw std::runtime_error("JetRegression::applyRegression() No event info handler." );
	if(m_doTraining && !m_truthprocessor) throw std::runtime_error("JetRegression::applyRegression() No truth particle handler." );
	if(m_doTraining && !m_truthjets) throw std::runtime_error("JetRegression::applyRegression() No truth jet handler." );

	const xAOD::EventInfo* eventInfo = m_eventinfo->getEventInfo();//  get_eventNumber();
	int eventNumber = eventInfo->eventNumber();
	//std::cout << "event: " << eventInfo->eventNumber() << std::endl;

	//////// TRAINING ////////

	//  fill training tree
	if (m_training) {

		std::map<TString,xAOD::JetContainer*>* jetMap = new std::map<TString,xAOD::JetContainer*>();
		jetMap->clear();
		jetMap = m_jets->getInParticles();
		if(!jetMap) throw std::runtime_error("No jet collections." );

		//fill ptCorr with a dummy value of 0 for training
		for(std::map<TString,xAOD::JetContainer*>::const_iterator jet_iter = jetMap->begin(); jet_iter != jetMap->end(); ++jet_iter){ 
    			const TString& variation=jet_iter->first;
			if(variation != "Nominal") continue;
			xAOD::JetContainer* jets_tmp = jet_iter->second;
			for (const xAOD::Jet *jet : *jets_tmp) {
				Props::ptCorr.set(jet,0);
			}
		}

		xAOD::JetContainer* jets = nullptr;
		jets = m_jets->getInParticleVariation("Nominal");
		if(!jets) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve jet collection." );

		xAOD::MuonContainer* muons = nullptr;
		muons = m_muons->getInParticleVariation("Nominal");
		if(!muons) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve muon collection." );

		xAOD::ElectronContainer* electrons = nullptr;
		electrons = m_electrons->getInParticleVariation("Nominal");
		if(!electrons) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve electron collection." );

		const xAOD::TruthParticleContainer* truthParticles = nullptr;
		truthParticles = m_truthprocessor->getInParticle("TruthBosonWithDecayParticles");
		if(!truthParticles) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve truth particle collection." );

		xAOD::JetContainer* truthJets = nullptr;
		truthJets = m_truthjets->getInParticleVariation("Nominal");
		if(!truthJets) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve truth jet collection." );

		xAOD::JetContainer* trackJets = nullptr;
		trackJets = m_trackjets->getInParticleVariation("Nominal");
		if(!trackJets) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve track jet collection." );

		std::vector<int> truthIndices;
		truthIndices.clear();
		std::vector<int> truthJetIndices;
		truthJetIndices.clear();

		m_partonIndices.clear();
		m_partonIndices = findPartonsFromHiggs(truthParticles);
		//if(m_partonIndices.size() != 2) return;
		
		// @TODO get proper event weight

		double event_weight = 1.;
		std::vector<double> values;
		for (const xAOD::Jet *jet : *jets) {

			if(Props::isSignalJet.get(jet)){

				int trueB_index = matchTrueBQuark(jet,truthParticles);
				int trueJet_index = associateTruthJet(jet, truthJets);
				int trackJet_index = associateTruthJet(jet, trackJets);
				int nBQuarks = nTrueBInJet(jet,truthParticles);
				if(nBQuarks > 1) return;

				m_bJets++;

				if(trueB_index != -1){

					m_truthlabeledB++;

					const xAOD::Jet* assocTruthJet = NULL;
					if(trueJet_index != -1) assocTruthJet = truthJets->at(trueJet_index);
					const xAOD::Jet* assocTrackJet = NULL;
					if(trackJet_index != -1) assocTrackJet = trackJets->at(trackJet_index);

					std::vector<const xAOD::Jet*> closeByJets = findCloseByJets(jet, jets);

					//additional studies stuff
					//std::vector<int> indices_tmp;
					//indices_tmp = JetRegressionVars_t::findLeptonsInJet(jet, truth_muon);
					//if(indices_tmp.size() > 1){
					//	for(std::vector<int>::const_iterator muon_iter = indices_tmp.begin(); muon_iter != indices_tmp.end(); ++muon_iter){
					//		std::cout << "Found truth muon in jet" << std::endl;
					//		std::cout << "pt = " << truth_muon->at(*muon_iter)->pt() << std::endl;
					//		if(truth_muon->at(*muon_iter)->pt() > 4000 && fabs(truth_muon->at(*muon_iter)->eta()) < 2.5){ m_muInJet++;
					//			std::cout << "origin: " <<  truth_muon->at(*muon_iter)->auxdata<int>("truthOrigin") << std::endl;}
					//	}
					//}

					std::vector<int> indices;
					indices = JetRegressionVars_t::findLeptonsInJet(jet, muons);
					for(std::vector<int>::const_iterator muon_iter = indices.begin(); muon_iter != indices.end(); ++muon_iter){
						if(Props::acceptedMuonTool.get(muons->at(*muon_iter)) && muons->at(*muon_iter)->pt() > 4000){
							//m_muInJet_ptHisto->Fill(muons->at(*muon_iter)->pt());
							//m_muInJet_etaHisto->Fill(muons->at(*muon_iter)->eta());
							//m_muInJet_authorHisto->Fill(muons->at(*muon_iter)->author());
							m_muInJet++;
						}
					}
					std::vector<int> indices2;
					indices2 = JetRegressionVars_t::findLeptonsInJet(jet, electrons);
					for(std::vector<int>::const_iterator el_iter = indices2.begin(); el_iter != indices2.end(); ++el_iter){
						if((Props::isLooseLH.get(electrons->at(*el_iter)) || Props::isMediumLH.get(electrons->at(*el_iter))) && electrons->at(*el_iter)->pt() > 2400) m_elInJet++;
					}
					std::vector<const xAOD::IParticle*> tracks = JetRegressionVars_t::getTracks(jet);
					if(tracks.size() != 0) m_assocTrack++;


					truthIndices.push_back(trueB_index);
					if(trueJet_index != -1) truthJetIndices.push_back(trueJet_index);
					JetRegressionVars_t vars(jet, eventInfo, truthParticles->at(trueB_index), assocTruthJet, muons, electrons, closeByJets, assocTrackJet);
					m_inputVars.getValues(vars, values);
					// @TODO default values for all the other arguments are good ?
					if(m_doEvenOdd) m_training->addEvent( eventNumber, values, event_weight );
					else m_training->addEvent( values, event_weight );
				}
			}


			//filling of the truth mBB histo
			//if(truthIndices.size() == 2){
			//	TLorentzVector truth1;
			//	truth1.SetPtEtaPhiM(truthParticles->at(truthIndices[0])->pt(), truthParticles->at(truthIndices[0])->eta(), truthParticles->at(truthIndices[0])->phi(), truthParticles->at(truthIndices[0])->m());
			//	TLorentzVector truth2;
			//	truth2.SetPtEtaPhiM(truthParticles->at(truthIndices[1])->pt(), truthParticles->at(truthIndices[1])->eta(), truthParticles->at(truthIndices[1])->phi(), truthParticles->at(truthIndices[1])->m());
			//	m_mBB_truthparton_histo->Fill((truth1+truth2).M());
			//}
			//if(truthJetIndices.size() == 2 && truthJets->at(truthJetIndices[0])->pt() > 0 && truthJets->at(truthJetIndices[1])->pt() > 0){
			//	TLorentzVector truthjet1;
			//	truthjet1.SetPtEtaPhiM(truthJets->at(truthJetIndices[0])->pt(), truthJets->at(truthJetIndices[0])->eta(), truthJets->at(truthJetIndices[0])->phi(), truthJets->at(truthJetIndices[0])->m());
			//	TLorentzVector truthjet2;
			//	truthjet2.SetPtEtaPhiM(truthJets->at(truthJetIndices[1])->pt(), truthJets->at(truthJetIndices[1])->eta(), truthJets->at(truthJetIndices[1])->phi(), truthJets->at(truthJetIndices[1])->m());
			//	m_mBB_truthjet_histo->Fill((truthjet1+truthjet2).M());
			//}

		}
		return;
	}

	//////// APPLICATION ////////

	if (!m_mva) throw std::runtime_error( "JetRegression::applyRegression() No MVA application tool." );

	std::map<TString,xAOD::JetContainer*>* jetMap = new std::map<TString,xAOD::JetContainer*>();
	jetMap->clear();
	jetMap = m_jets->getInParticles();
	if(!jetMap) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve jet collections." );

	//do this for all jet systematics
	for(std::map<TString,xAOD::JetContainer*>::const_iterator jet_iter = jetMap->begin(); jet_iter != jetMap->end(); ++jet_iter) {

		//05th June 2018: decided to run only on nominal
    		const TString& variation=jet_iter->first;
		if(variation != "Nominal") continue;

		xAOD::JetContainer* jets = jet_iter->second;

		xAOD::MuonContainer* muons = nullptr;
		muons = m_muons->getInParticleVariation("Nominal");
		if(!muons) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve muon collection." );

		xAOD::ElectronContainer* electrons = nullptr;
		electrons = m_electrons->getInParticleVariation("Nominal");
		if(!electrons) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve electron collection." );

		xAOD::JetContainer* trackJets = nullptr;
		trackJets = m_trackjets->getInParticleVariation("Nominal");
		if(!trackJets) throw std::runtime_error("JetRegression::applyRegression() Not able to retrieve track jet collection." );

		std::vector<double> values;

		for (const xAOD::Jet *jet : *jets) {

			if(Props::isSignalJet.get(jet)){

				//std::cout << "event number: " << eventNumber << std::endl;

				int trackJet_index = associateTruthJet(jet, trackJets);
				const xAOD::Jet* assocTrackJet = NULL;
				if(trackJet_index != -1) assocTrackJet = trackJets->at(trackJet_index);

				std::vector<const xAOD::Jet*> closeByJets = findCloseByJets(jet, jets);

				JetRegressionVars_t vars(jet, eventInfo, muons, electrons, closeByJets, assocTrackJet);
				m_inputVars.getValues(vars, values);

				// should provide the regression value for all methods which are registered in the application tool
				if(m_doEvenOdd) std::vector< std::vector<float> > results = m_mva->evaluateRegression(eventNumber, values);
				std::vector< std::vector<float> > results = m_mva->evaluateRegression(values);

				if(m_doPtSplit && m_mva->getNMethods() != 2) throw std::runtime_error("Not right amount of methods given for low/high pt application." );

				if(m_doPtSplit){
					if (results.empty())  throw std::runtime_error( "Regression did not return new jet pt");
					else {
					  double new_jetPt = jet->pt();
					  if(jet->pt() < 100000) new_jetPt = *((results[0]).begin());
					  if(jet->pt() >= 100000) new_jetPt = *((results[1]).begin());
					  //std::cout << "jet pt: " << jet->pt() << "\t reg pt: " << new_jetPt << std::endl; 
					  Props::ptCorr.set(jet,new_jetPt);
					}
				  }
		      
				else{
				  for (const std::vector<float> &new_jetPT_targets : results ) {
					if (new_jetPT_targets.empty())  throw std::runtime_error( "Regression did not return new jet pt");
					else {
						const double new_jetPt = *(new_jetPT_targets.begin()) * jet->pt();
						//double new_jetPt = TMath::Sqrt(new_jetEt*new_jetEt - JetRegression::mt(jet)*JetRegression::mt(jet));
						Props::ptCorr.set(jet,new_jetPt);
					}
				  }
				}
			}
			//in case the jet is no signal jet  ptCorr is set to the standard jet pt
			else Props::ptCorr.set(jet,jet->pt());
		}
	}
}

//VH training: this finds the true b-quark directly originating from the Higgs
//ttbar training: this finds any true quark directly originating from a top, W, Z or Higgs
std::vector<int> JetRegression::findPartonsFromHiggs(const xAOD::TruthParticleContainer* truthParticles){
  
  std::vector<int> indices;
  indices.clear();

	for(const xAOD::TruthParticle *truth : *truthParticles) {

		//ttbar training
		if (fabs(truth->pdgId()) < 6 && truth->pt() > 4000){
		//VH training
		//if (fabs(truth->pdgId()) == 5 && truth->pt() > 4000){

			for(unsigned int iParents=0; iParents< truth->nParents(); ++iParents){
				//ttbar training
				if(truth->parent(iParents)->pdgId() == 25 || truth->parent(iParents)->pdgId() == 6 || truth->parent(iParents)->pdgId() == 23 || truth->parent(iParents)->pdgId() == 24) indices.push_back(truth->index());
				//VH training
				//if(truth->parent(iParents)->pdgId() == 25) indices.push_back(truth->index());
			}
		}
	}
	return indices;
}

//the true b-quark has to be within a cone of dR<0.4 around the jet
int JetRegression::matchTrueBQuark(const xAOD::Jet* jet, const xAOD::TruthParticleContainer* truthParticles){

	int truthIndex = -1;
	double dR = 1000;

	for(unsigned int part_i=0; part_i<m_partonIndices.size(); part_i++) {
	  
	  const xAOD::TruthParticle* truth = truthParticles->at(m_partonIndices[part_i]);
	  
	  if(JetRegressionVars_t::deltaR(jet,truth) < 0.4 && JetRegressionVars_t::deltaR(jet,truth) < dR){
	    dR = JetRegressionVars_t::deltaR(jet,truth);
	    truthIndex = truth->index();
	  }

	}
	//if(truthIndex >= 0)std::cout << "truth label: " << truthParticles->at(truthIndex)->pdgId() << "\t parent: " << truthParticles->at(truthIndex)->parent(0)->pdgId() << std::endl;
	return truthIndex;
}

int JetRegression::nTrueBInJet(const xAOD::Jet* jet, const xAOD::TruthParticleContainer* truthParticles){
	
	int nTruth = 0;

	for(unsigned int part_i=0; part_i<m_partonIndices.size(); part_i++) {
 		const xAOD::TruthParticle* truth = truthParticles->at(m_partonIndices[part_i]);
	  	if(JetRegressionVars_t::deltaR(jet,truth) < 0.4) nTruth++;
	}
        return nTruth;
}

//associate a truth jet to the jet
int JetRegression::associateTruthJet(const xAOD::Jet* jet, const xAOD::JetContainer* truthJets){

	int truthjetIndex = -1;
	int ntruthJets = 0;
	double dR = 1000;

	for(const xAOD::Jet *truthjet : *truthJets) {

		if(JetRegressionVars_t::deltaR(jet,truthjet) < 0.4){

			ntruthJets++;

			if(JetRegressionVars_t::deltaR(jet,truthjet) < dR){
				dR = JetRegressionVars_t::deltaR(jet,truthjet);
				truthjetIndex = truthjet->index();
			}
		}
	}
	//if more than one truth jet was associated do not use the jet
	if(ntruthJets > 1) truthjetIndex = -1;

	return truthjetIndex;
}

std::vector<const xAOD::Jet*> JetRegression::findCloseByJets(const xAOD::Jet* jet, const xAOD::JetContainer* jetContainer){

  std::vector<const xAOD::Jet*> closeByJets;
  closeByJets.clear();

  for(const xAOD::Jet *tmp_jet : *jetContainer) {

		if(JetRegressionVars_t::deltaR(jet,tmp_jet) < 1 && jet->index() != tmp_jet->index()){
				closeByJets.push_back(tmp_jet);
		}
	}
	return closeByJets;
}
