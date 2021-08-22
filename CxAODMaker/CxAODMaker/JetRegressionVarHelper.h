/* Dear emacs, this is -*-c++-*- */
#ifndef _JetRegressionVarHelper_H_
#define _JetRegressionVarHelper_H_

class JetRegressionVars_t;
class JetRegressionVarList_t;

class JetRegressionVarHelper
{
public:
  JetRegressionVarHelper() {
    m_methodList.insert( std::make_pair("eventNumber",&JetRegressionVars_t::eventNumber));
    m_methodList.insert( std::make_pair("truePt",&JetRegressionVars_t::truePt));
    m_methodList.insert( std::make_pair("trueLabel",&JetRegressionVars_t::trueLabel));
    m_methodList.insert( std::make_pair("trueJetPt",&JetRegressionVars_t::trueJetPt));
    m_methodList.insert( std::make_pair("jetPt",&JetRegressionVars_t::pt));
    m_methodList.insert( std::make_pair("jetEta",&JetRegressionVars_t::eta));
    m_methodList.insert( std::make_pair("jetPhi",&JetRegressionVars_t::phi));
    m_methodList.insert( std::make_pair("jetEt",&JetRegressionVars_t::et));
    m_methodList.insert( std::make_pair("jetEnergy",&JetRegressionVars_t::energy));
    m_methodList.insert( std::make_pair("jetTheta",&JetRegressionVars_t::theta));
    m_methodList.insert( std::make_pair("jetMass",&JetRegressionVars_t::mass));
    m_methodList.insert( std::make_pair("jetMt",&JetRegressionVars_t::mt));
    m_methodList.insert( std::make_pair("jetRawPt",&JetRegressionVars_t::rawPt));
    m_methodList.insert( std::make_pair("jetMV2c10",&JetRegressionVars_t::MV2c10));
    m_methodList.insert( std::make_pair("jetWidth",&JetRegressionVars_t::width));
    m_methodList.insert( std::make_pair("muonPt",&JetRegressionVars_t::muPt));
    m_methodList.insert( std::make_pair("electronPt",&JetRegressionVars_t::elPt));
    m_methodList.insert( std::make_pair("sumPtLeps",&JetRegressionVars_t::sumPtLeps));
    m_methodList.insert( std::make_pair("dRLepJet",&JetRegressionVars_t::dRLepJet));
    m_methodList.insert( std::make_pair("jvt",&JetRegressionVars_t::jvt));
    m_methodList.insert( std::make_pair("leadTrackPt",&JetRegressionVars_t::leadTrackPt));
    m_methodList.insert( std::make_pair("sumPtTracks",&JetRegressionVars_t::sumPtTracks));
    m_methodList.insert( std::make_pair("nTracks",&JetRegressionVars_t::nTracks));
    m_methodList.insert( std::make_pair("efrac",&JetRegressionVars_t::efrac));
    m_methodList.insert( std::make_pair("EtaWidthTracks",&JetRegressionVars_t::EtaWidthTracks));
    m_methodList.insert( std::make_pair("PhiWidthTracks",&JetRegressionVars_t::PhiWidthTracks));
    m_methodList.insert( std::make_pair("sumPtCloseByJets",&JetRegressionVars_t::sumPtCloseByJets));
    m_methodList.insert( std::make_pair("dRJetNearestJet",&JetRegressionVars_t::dRJetNearestJet));
    m_methodList.insert( std::make_pair("nPileUpVertices",&JetRegressionVars_t::nPileUpVertices));
    m_methodList.insert( std::make_pair("secVtxMass",&JetRegressionVars_t::secVtxMass));
    m_methodList.insert( std::make_pair("secVtxNormDist",&JetRegressionVars_t::secVtxNormDist));
  }

  ~JetRegressionVarHelper() {
  }

  void setup( const std::vector<std::string> &var_names_in, JetRegressionVarList_t &vars_out) {
    for (const std::string &var_name : var_names_in) {
      
      std::map<std::string, JetRegressionVars_t::Func_t>::const_iterator iter = m_methodList.find(var_name );
      if (iter == m_methodList.end()) {
        std::stringstream message;
        message << "Unknown variable " << var_name << " for jet energy regression.";
        throw std::runtime_error( message.str() );
      }
      vars_out.push_back( iter->second );
    }
  }


protected:
  
private:
  std::map<std::string, JetRegressionVars_t::Func_t> m_methodList;
};
#endif
