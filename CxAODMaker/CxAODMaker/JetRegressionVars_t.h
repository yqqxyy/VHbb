/* Dear emacs, this is -*-c++-*- */
#ifndef _JetRegressionVars_t_H_
#define _JetRegressionVars_t_H_

class JetRegressionVars_t
{
public:
	JetRegressionVars_t(const xAOD::Jet* a_jet, const xAOD::EventInfo* a_info, const xAOD::MuonContainer* a_muContainer, const xAOD::ElectronContainer* a_elContainer, const std::vector<const xAOD::Jet*> a_closeByJetVector, const xAOD::Jet* a_trackJet) : m_jet(a_jet), m_eventinfo(a_info), m_muons(a_muContainer), m_electrons(a_elContainer), m_closebyjets(a_closeByJetVector), m_trackJet(a_trackJet) {}

//	JetRegressionVars_t(const xAOD::Jet* a_jet, const xAOD::EventInfo* a_info, const xAOD::TruthParticle* a_truth, const xAOD::MuonContainer* a_muContainer, const xAOD::ElectronContainer* a_elContainer) :
//		m_jet(a_jet), m_eventinfo(a_info), m_truth(a_truth), m_muons(a_muContainer), m_electrons(a_elContainer) {}

	JetRegressionVars_t(const xAOD::Jet* a_jet, const xAOD::EventInfo* a_info, const xAOD::TruthParticle* a_truth, const xAOD::Jet* a_truthJet, const xAOD::MuonContainer* a_muContainer, const xAOD::ElectronContainer* a_elContainer, const std::vector<const xAOD::Jet*> a_closeByJetVector, const xAOD::Jet* a_trackJet) : m_jet(a_jet), m_eventinfo(a_info), m_truth(a_truth), m_truthJet(a_truthJet), m_muons(a_muContainer), m_electrons(a_elContainer), m_closebyjets(a_closeByJetVector), m_trackJet(a_trackJet){}

	double eventNumber() const {
		return m_eventinfo->eventNumber();
	}

	double truePt() const {
		return m_truth->pt();
	}

	double trueE() const {
		return m_truth->e();
	}

	double trueLabel() const {
		return m_truth->pdgId();
	}

	double trueJetPt() const {
		if(m_truthJet != NULL) return m_truthJet->pt();
		else return -1;
	}

	double pt() const {
		return m_jet->pt();
	}

	double eta() const {
		return m_jet->eta();
	}

	double phi() const {
		return m_jet->phi();
	}
	double theta() const{
		return TMath::ACos(TMath::TanH(m_jet->eta()));
	}

	double mass() const {
		return m_jet->m();
	}

	double energy() const {
		return m_jet->e();
	}

	double et() const {
		return m_jet->e()*TMath::Sin(JetRegressionVars_t::theta());
	}

	double mt() const{
		return TMath::Sqrt(JetRegressionVars_t::et()*JetRegressionVars_t::et() - m_jet->pt()*m_jet->pt());
	}

	double rawPt() const{
		const xAOD::JetFourMom_t rawFourMom = m_jet->jetP4(xAOD::JetScale::JetEMScaleMomentum);
		Props::RegInputEmscalePt.set(m_jet,rawFourMom.Pt());
		return rawFourMom.Pt();
	}

	double MV2c10() const{
		return Props::MV2c10.get(m_jet);
	}

	double width() const{
		double width = 0;
		m_jet->getAttribute(xAOD::JetAttribute::Width, width);
		return width;
	}

	double muPt() const{
		std::vector<int> indices = findLeptonsInJet(m_jet,m_muons);
		double sumPt = 0;

		if(indices.size() != 0){
			for(std::vector<int>::const_iterator muon_iter = indices.begin(); muon_iter != indices.end(); ++muon_iter){
				if(Props::acceptedMuonTool.get(m_muons->at(*muon_iter)) && m_muons->at(*muon_iter)->pt() > 4000){
				  sumPt += std::min(1e6,m_muons->at(*muon_iter)->pt());
				  //if(m_muons->at(*muon_iter)->pt() > 1e6) Info("JetRegressionVars_t", Form("Very large muon pT: %f",m_muons->at(*muon_iter)->pt()));
				}
			}
			Props::RegInputMuInJetPt.set(m_jet,sumPt);
			return sumPt;
		}
		else{
			Props::RegInputMuInJetPt.set(m_jet,-1);
			return -1;
		}
	}

	double elPt() const{
		std::vector<int> indices = findLeptonsInJet(m_jet,m_electrons);
		double sumPt = 0;
		if(indices.size() != 0){
			for(std::vector<int>::const_iterator el_iter = indices.begin(); el_iter != indices.end(); ++el_iter){
				if((Props::isLooseLH.get(m_electrons->at(*el_iter)) || Props::isMediumLH.get(m_electrons->at(*el_iter))) && m_electrons->at(*el_iter)->pt() > 2400){
				  sumPt += std::min(1e6,m_electrons->at(*el_iter)->pt());
				  //if(m_electrons->at(*el_iter)->pt() > 1e6) Info("JetRegressionVars_t", Form("Very large electron pT: %f", m_electrons->at(*el_iter)->pt()));
				}
			}
			Props::RegInputElInJetPt.set(m_jet,sumPt);
			return sumPt;
		}
		else{
			Props::RegInputElInJetPt.set(m_jet,-1);
			return -1;
		}
	}

	double sumPtLeps() const{
		double sumPt;
		if(elPt() == -1 && muPt() == -1) sumPt = -1;
		else if(muPt() == -1) sumPt = elPt();
		else if(elPt() == -1) sumPt = muPt();
		else sumPt = elPt()+muPt();
		Props::RegInputSumPtLeps.set(m_jet,sumPt);
		return sumPt;
	}

	double dRLepJet() const{
		double dR = -1;
		std::pair<int,int> nearestLep = findNearestLepToJetAxis(m_jet, m_muons, m_electrons);
		if(nearestLep.first != -1 && nearestLep.second == xAOD::Type::Muon) dR = deltaR(m_jet,m_muons->at(nearestLep.first));
		else if(nearestLep.first != -1 && nearestLep.second == xAOD::Type::Electron) dR = deltaR(m_jet,m_electrons->at(nearestLep.first));
		Props::RegInputDRJetLepInJet.set(m_jet,dR);
		return dR;
	}

	double jvt() const{
		double jvt = -1;
		jvt = Props::Jvt.get(m_jet);
		return jvt;
	}

	double leadTrackPt() const{
		std::vector<const xAOD::IParticle*> tracks = getTracks(m_jet);
		double leadPt = 0;
		if(tracks.size() != 0){
			for(std::vector<const xAOD::IParticle*>::const_iterator track_iter = tracks.begin(); track_iter != tracks.end(); ++track_iter){
				if((*track_iter)->pt() > leadPt && (*track_iter)->pt() < 1e6) leadPt = (*track_iter)->pt();
				if((*track_iter)->pt() > 1e6){
				  //Info("JetRegressionVars_t", Form("Very large track pT: %f",(*track_iter)->pt()));
				  leadPt = 1e6;
				  break;
				}
			}
			Props::RegInputLeadTrkPt.set(m_jet,leadPt);
			return leadPt;
		}
		else{
			Props::RegInputLeadTrkPt.set(m_jet,-1);
			return -1;
		}
	}

	double sumPtTracks() const{
		std::vector<const xAOD::IParticle*> tracks = getTracks(m_jet);
		double sumPt = 0;
		if(tracks.size() != 0){
			for(std::vector<const xAOD::IParticle*>::const_iterator track_iter = tracks.begin(); track_iter != tracks.end(); ++track_iter){
			  sumPt += std::min(1e6,(*track_iter)->pt());
			}
			Props::RegInputSumTrkPt.set(m_jet,sumPt);
			return sumPt;
		}
		else{
			Props::RegInputSumTrkPt.set(m_jet,-1);
			return -1;
		}
	}

	double nTracks() const{
		std::vector<const xAOD::IParticle*> tracks = getTracks(m_jet);
		if(tracks.size() != 0){
			Props::RegInputNTrks.set(m_jet,tracks.size());
			return tracks.size();
		}
		else{
			Props::RegInputNTrks.set(m_jet,-1);
			return -1;
		}
	}

	double efrac() const{
	  double SumPtTrk=Props::SumPtTrkPt500PV.get(m_jet);
	  if(SumPtTrk != -1) return SumPtTrk/m_jet->pt();
	  else return -1;
	}

	double EtaWidthTracks() const{
		std::vector<const xAOD::IParticle*> tracks = getTracks(m_jet);
		if(tracks.size() != 0){
			std::vector <double> v_eta;
			v_eta.clear();
			for(std::vector<const xAOD::IParticle*>::const_iterator track_iter = tracks.begin(); track_iter != tracks.end(); ++track_iter){
				v_eta.push_back((*track_iter)->eta());
			}
			Props::RegInputEtaWidthTrks.set(m_jet,TMath::RMS(v_eta.begin(), v_eta.end()));
			return TMath::RMS(v_eta.begin(), v_eta.end());
		}
		else{
			Props::RegInputEtaWidthTrks.set(m_jet,-1);
			return -1;
		}
	}

	double PhiWidthTracks() const{
		std::vector<const xAOD::IParticle*> tracks = getTracks(m_jet);
		if(tracks.size() != 0){
			std::vector <double> v_phi;
			v_phi.clear();
			for(std::vector<const xAOD::IParticle*>::const_iterator track_iter = tracks.begin(); track_iter != tracks.end(); ++track_iter){
				v_phi.push_back((*track_iter)->phi());
			}
			Props::RegInputPhiWidthTrks.set(m_jet,TMath::RMS(v_phi.begin(), v_phi.end()));
			return TMath::RMS(v_phi.begin(), v_phi.end());
		}
		else{
			Props::RegInputPhiWidthTrks.set(m_jet,-1);
			return -1;
		}
	}

	double sumPtCloseByJets() const{
		if(m_closebyjets.size() != 0){
			double sumPt = 0;
			for(std::vector<const xAOD::Jet*>::const_iterator jet_iter = m_closebyjets.begin(); jet_iter != m_closebyjets.end(); ++jet_iter){
				sumPt += (*jet_iter)->pt();
			}
			Props::RegInputSumPtCloseByJets.set(m_jet,sumPt);
			return sumPt;
		}
		else{
	                Props::RegInputSumPtCloseByJets.set(m_jet,-1);
			return -1;
		}
	}

	double dRJetNearestJet() const{
		if(m_closebyjets.size() != 0){
			double dR = 99;
			for(std::vector<const xAOD::Jet*>::const_iterator jet_iter = m_closebyjets.begin(); jet_iter != m_closebyjets.end(); ++jet_iter){
				if(deltaR(m_jet, *jet_iter) < dR) dR = deltaR(m_jet, *jet_iter);
			}
			return dR;
		}
		else{	
		 	return -1;
		}
	}

	double nPileUpVertices() const{
		return m_eventinfo->averageInteractionsPerCrossing();
	}

	double secVtxMass() const{
		float Vtxmass = -1;
		if(m_trackJet != NULL){
    			const xAOD::BTagging * tagInfo = m_trackJet->btagging();
			if(tagInfo) tagInfo->taggerInfo(Vtxmass,xAOD::SV1_masssvx);
			if(Vtxmass <= 0) Vtxmass = -1;
		}
		Props::RegInputSecVtxMass.set(m_jet,Vtxmass);
		return Vtxmass;
	}

	double secVtxNormDist() const{
		float normdist = -1;
		if(m_trackJet != NULL){
    			const xAOD::BTagging * tagInfo = m_trackJet->btagging();
			if(tagInfo) tagInfo->taggerInfo(normdist,xAOD::SV1_normdist);
			if(normdist <= 0) normdist = -1;
		}
                Props::RegInputSecVtxNormDist.set(m_jet,normdist);
		return normdist;
	}


typedef double (JetRegressionVars_t:: *Func_t)() const;
typedef std::vector<JetRegressionVars_t> FuncList_t;

//these are helper functions to retrieve objects associated to the jet (leps, tracks...)
	static double deltaR(const xAOD::IParticle* part1, const xAOD::IParticle* part2){
		return TMath::Sqrt( (part1->eta()-part2->eta())*(part1->eta()-part2->eta()) + (deltaPhi(part1->phi(),part2->phi()))*(deltaPhi(part1->phi(),part2->phi())) );
	}

	static double deltaPhi(float phi1, float phi2){
		if (phi1<phi2) {
			float delta_phi=phi2-phi1;
			float delta_phi_alt=2*M_PI+phi1-phi2;
			return (delta_phi < delta_phi_alt ? delta_phi : delta_phi_alt);
		}
		else {
			float delta_phi=phi1-phi2;
			float delta_phi_alt=2*M_PI+phi2-phi1;
			return (delta_phi < delta_phi_alt ? delta_phi : delta_phi_alt);
		}
	}

	static std::vector<int> findLeptonsInJet(const xAOD::Jet* jet, const xAOD::IParticleContainer* leptons){
		std::vector<int> lep_indices;
		for (const xAOD::IParticle *lep : *leptons) {
			if(deltaR(jet,lep) < 0.4) lep_indices.push_back(lep->index());
		}
		return lep_indices;
	}

	static std::pair<int,int> findNearestLepToJetAxis(const xAOD::Jet* jet, const xAOD::MuonContainer* muons, const xAOD::ElectronContainer* electrons){
		std::vector<int> mu_indices = findLeptonsInJet(jet, muons);
		std::vector<int> el_indices = findLeptonsInJet(jet, electrons);

		float dR = 1000;
		int nearestLep_index = -1;
		int type = -1;

		if(mu_indices.size() != 0){
			for(std::vector<int>::const_iterator muon_iter = mu_indices.begin(); muon_iter != mu_indices.end(); ++muon_iter){
				float temp = deltaR(jet,muons->at(*muon_iter));
				if(temp<dR){
					dR = temp;
					nearestLep_index = muons->at(*muon_iter)->index();
					type = muons->at(*muon_iter)->type();
				}
				else continue;
			}
		}
		if(el_indices.size() != 0){
			for(std::vector<int>::const_iterator el_iter = el_indices.begin(); el_iter != el_indices.end(); ++el_iter){
				float temp = deltaR(jet,electrons->at(*el_iter));
				if(temp<dR){
					dR = temp;
					nearestLep_index = electrons->at(*el_iter)->index();
					type = electrons->at(*el_iter)->type();
				}
				else continue;
			}
		}
		return std::make_pair(nearestLep_index,type);
	}

	static std::vector<const xAOD::IParticle*> getTracks(const xAOD::Jet* jet){
		std::vector<const xAOD::IParticle*> tracks = jet->getAssociatedObjects<const xAOD::IParticle>(xAOD::JetAttribute::GhostTrack);
		return tracks;
	}

//	static std::vector<const xAOD::Vertex*> getSecVtx(const xAOD::Jet* jet){
//		std::vector<const xAOD::Vertex*> secVtx;
//		const xAOD::BTagging* bjet = jet->btagging();
//
//		const std::vector<ElementLink<xAOD::VertexContainer > >  SV1vertices = bjet->auxdata<std::vector<ElementLink<xAOD::VertexContainer > > >("SV1_vertices");
//		for (unsigned int sv1V=0; sv1V< SV1vertices.size(); sv1V++) {
//			const xAOD::Vertex*  tmpVertex=*(SV1vertices.at(sv1V));
//			secVtx.push_back(tmpVertex);
//		}
//		return secVtx;
//	}


private:
	const xAOD::Jet *m_jet;
	const xAOD::EventInfo *m_eventinfo;
	const xAOD::TruthParticle *m_truth;
	const xAOD::Jet *m_truthJet;
	const xAOD::MuonContainer *m_muons;
	const xAOD::ElectronContainer *m_electrons;
	const std::vector<const xAOD::Jet*> m_closebyjets;
	const xAOD::Jet *m_trackJet;
};

class JetRegressionVarList_t
{
public:


  void push_back(JetRegressionVars_t::Func_t a_func)  { m_list.push_back( a_func ); }

  void getValues( JetRegressionVars_t &vars, std::vector<double> &out) const {
    out.clear();
    out.reserve( m_list.size() );
    for (JetRegressionVars_t::Func_t a_func : m_list ) {
      out.push_back ( (vars.*a_func)() );
    }
  }


private:
  std::vector<JetRegressionVars_t::Func_t> m_list;
};

#endif
