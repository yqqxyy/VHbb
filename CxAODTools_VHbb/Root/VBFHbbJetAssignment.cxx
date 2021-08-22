#include "CxAODTools_VHbb/VBFHbbJetAssignment.h"
#include "CxAODTools_VHbb/VBFHbb4centralEvtSelection.h"

#include "EventLoop/Worker.h"
#include "TLorentzVector.h"
#include "cmath"

// ==============================================================
jetAssignmentTool::jetAssignmentTool()
  : _type("DEFAULT"),
    _Selection("4central"),
    _debugMode(false), 
    m_isMC(false) {}
jetAssignmentTool::jetAssignmentTool(jetAssignmentTool const& other)
  : _type(other._type),
    _Selection(other._Selection),
    _signalJets( other._signalJets.begin(),other._signalJets.end() ),
    _vbfJets( other._vbfJets.begin(),other._vbfJets.end() ),
    _debugMode(other._debugMode),
    m_isMC(other.m_isMC) {} 
jetAssignmentTool::~jetAssignmentTool() {}
// ==============================================================
EL::StatusCode jetAssignmentTool::setAssignmentType(std::string type) { 
  _type = type;
  if (_debugMode) Info("jetAssignmentTool()","Setting Jet Assignment type to %15s",type.c_str());
  return EL::StatusCode::SUCCESS; 
}

EL::StatusCode jetAssignmentTool::setSelection(std::string selection) {
  _Selection = selection;
  if (_debugMode) Info("jetAssignmentTool()","Setting Jet Selection type to %15s",_Selection.c_str());
  return EL::StatusCode::SUCCESS; 
}

EL::StatusCode jetAssignmentTool::setIsMC(bool isMC) {
  m_isMC = isMC;
  if (_debugMode) Info("jetAssignmentTool()","Setting isMC to %15s",m_isMC ? "True" : "False");
  return EL::StatusCode::SUCCESS; 
}
// ==============================================================
const std::string jetAssignmentTool::assignmentType() const { return _type; }
std::vector<xAOD::Jet const*> jetAssignmentTool::signalJets() const { return _signalJets; }
std::vector<xAOD::Jet const*> jetAssignmentTool::vbfJets() const { return _vbfJets; }
// ==============================================================

void jetAssignmentTool::setDebugMode(bool debugMode) { _debugMode = debugMode; }
bool jetAssignmentTool::assignJets(std::vector<xAOD::Jet const*> const& inputJets) {
  // ** Common to all methods ** //
  std::vector<const xAOD::Jet*> shortListedInputJets;
  this->clear();

  if (_Selection.find("4central") != std::string::npos){
    for (unsigned int iJet(0); iJet < inputJets.size(); iJet++) 
      if (inputJets.at(iJet)->pt() * 0.001 > 55 && fabs(inputJets.at(iJet)->eta())<2.8) shortListedInputJets.push_back( inputJets.at(iJet) );
    if (shortListedInputJets.size() < 4) return false;
  }
  else if (_Selection.find("2central") != std::string::npos or _Selection.find("none") != std::string::npos){
    for (unsigned int iJet(0); iJet < inputJets.size(); iJet++) 
      shortListedInputJets.push_back( inputJets.at(iJet) );
  }
  // ** End common part ** //

  // Supported Types
  // BTAG_XX
  // 0tag

  if (_Selection.find("4central") != std::string::npos || 
      _Selection.find("none") != std::string::npos){
    if (_type == "0tag")
      return this->assign0tag_4cen(shortListedInputJets);
    else if (_type == "BTAG_85")
      return this->assignBtag_4cen(shortListedInputJets, "85");
    else if (_type == "BTAG_77")
      return this->assignBtag_4cen(shortListedInputJets, "77");
    else if (_type == "BTAG_70")
      return this->assignBtag_4cen(shortListedInputJets, "70");
  }
  else if (_Selection.find("2central") != std::string::npos){
    if (_type == "0tag")
      return this->assign0tag_2cen(shortListedInputJets);
    else if (_type == "BTAG_85")
      return this->assignBtag_2cen(shortListedInputJets, "85");
    else if (_type == "BTAG_77")
      return this->assignBtag_2cen(shortListedInputJets, "77");
    else if (_type == "BTAG_70")
      return this->assignBtag_2cen(shortListedInputJets, "70");
    else if (_type == "BTAG_asym")
      return this->assignBtag_2cen(shortListedInputJets, "asym");
    else if (_type == "BTAG_asym_reverse")
      return this->assignBtag_2cen_reverse(shortListedInputJets, "asym");
    else if (_type == "BTAG_asym_overlap")
      return this->assignBtag_2cen(shortListedInputJets, "overlap");
  }
  
  return false;
}

void jetAssignmentTool::clear() {
  _signalJets.clear();
  _vbfJets.clear();
}

void jetAssignmentTool::printResults() {
  Info( "jetAssignmentTool()", "Results of assignment [%15s]", this->_type.c_str());
  Info( "jetAssignmentTool()", "signal Jet n. 1 [pt,eta,mv2c20] = [%.4f,%.4f,%.4f]", _signalJets.at(0)->pt(), _signalJets.at(0)->eta(), Props::MV2c10.get(_signalJets.at(0)) );
  Info( "jetAssignmentTool()", "signal Jet n. 2 [pt,eta,mv2c20] = [%.4f,%.4f,%.4f]", _signalJets.at(1)->pt(), _signalJets.at(1)->eta(), Props::MV2c10.get(_signalJets.at(1)) );
  Info( "jetAssignmentTool()", "   vbf Jet n. 1 [pt,eta,mv2c20] = [%.4f,%.4f,%.4f]", _vbfJets.at(0)->pt(), _vbfJets.at(0)->eta(), Props::MV2c10.get(_vbfJets.at(0)) );
  Info( "jetAssignmentTool()", "   vbf Jet n. 2 [pt,eta,mv2c20] = [%.4f,%.4f,%.4f]", _vbfJets.at(1)->pt(), _vbfJets.at(1)->eta(), Props::MV2c10.get(_vbfJets.at(1)) );
}

bool jetAssignmentTool::assignBtag_4cen(std::vector<xAOD::Jet const*> &inputJets, std::string WP) {

  double btagweight = 0;
  if (WP == "85") btagweight = m_b_85_weight;
  else if (WP == "77") btagweight = m_b_77_weight;
  else if (WP == "70") btagweight = m_b_70_weight;
  else if (WP == "60") btagweight = m_b_60_weight;
  else {
    Error( "jetAssignmentTool()", "BTAG weight cannot be set");
    return false;
  }
  // Choose signal jets to be the pair with highest pT passing tight btagging WP  
  double MaxPt = -1;

  _signalJets.push_back(nullptr);
  _signalJets.push_back(nullptr);

  for (unsigned int i(0); i < inputJets.size(); i++)
    {
      if ( Props::MV2c10.get( inputJets.at(i) ) < btagweight or fabs(inputJets.at(i)->eta())>2.5 )
	continue;

      for (unsigned int j(i+1); j < inputJets.size(); j++)
	{
	  if ( Props::MV2c10.get( inputJets.at(j) ) < btagweight or fabs(inputJets.at(j)->eta())>2.5 )
	    continue;

	  /* // TMP
	  if (m_isMC and  _Period == "PreICHEP" and Props::HLTBJetMatched_2J45_MV2c20_70.get(inputJets.at(i)) == Props::HLTBJetMatched_2J45_MV2c20_70.get(inputJets.at(j)))
	    continue;

	  if (m_isMC and _Period == "PostICHEP" and Props::HLTBJetMatched_2J35_MV2c20_60.get(inputJets.at(i)) == Props::HLTBJetMatched_2J35_MV2c20_60.get(inputJets.at(j)))
	    continue;
	  */  

	  TLorentzVector HiggsVect = inputJets.at(i)->p4() + inputJets.at(j)->p4();
	  if ( HiggsVect.Pt() > MaxPt ) {
	    MaxPt = HiggsVect.Pt();
	    _signalJets.at(0) = inputJets.at(i);
	    _signalJets.at(1) = inputJets.at(j);
	  }
	}
    }

  if ( _signalJets.at(0) == nullptr or _signalJets.at(1) == nullptr ) {
    return false;
  }

  // find the pair of jets that yields the highest mjj                                                                                                                     
  std::vector<xAOD::Jet const*> candidates;

  for (unsigned int i(0); i < inputJets.size(); i++) {
    if( Props::MV2c10.get( inputJets.at(i) ) < btagweight || fabs(inputJets.at(i)->eta())>2.5) {
      candidates.push_back( inputJets.at(i) );
    }
  }


  if (candidates.size() < 2) return false;

  double maxMjj(-1.);
  xAOD::Jet const* vbfjet1;
  xAOD::Jet const* vbfjet2;

  for (unsigned int i(0); i < candidates.size(); i++)
    {
      for (unsigned int j(i+1); j < candidates.size(); j++)
	{
	  TLorentzVector jet1(candidates.at(i)->p4());
	  TLorentzVector jet2(candidates.at(j)->p4());
	  if((jet1+jet2).M() > maxMjj){
	    maxMjj = (jet1+jet2).M();
	    vbfjet1 = candidates.at(i);
	    vbfjet2 = candidates.at(j);
	  }
	}
    }

  if (vbfjet1 == nullptr or vbfjet2 == nullptr) {
    return false;
  }

  _vbfJets.push_back( vbfjet1 );
  _vbfJets.push_back( vbfjet2 );

  // sort results in pt
  std::sort(_signalJets.begin(),_signalJets.end(),JetSort::sortJetsByPt);
  std::sort(_vbfJets.begin(),_vbfJets.end(),JetSort::sortJetsByPt);

  if (_debugMode) this->printResults();
  return true;
}

bool jetAssignmentTool::assignBtag_2cen(std::vector<xAOD::Jet const*> &inputJets, std::string WP) {

  double btagweight_lead =0;
  double btagweight_sub =0;

  if (WP == "85"){
    btagweight_lead = m_b_85_weight;
    btagweight_sub = m_b_85_weight;
  }
  if (WP == "77"){
    btagweight_lead = m_b_77_weight;
    btagweight_sub = m_b_77_weight;
  }
  if (WP == "70"){
    btagweight_lead = m_b_70_weight;
    btagweight_sub = m_b_70_weight;
  }
  if (WP == "asym" or WP == "overlap"){
    btagweight_lead = m_b_70_weight;
    btagweight_sub = m_b_85_weight;
  }


  std::vector<xAOD::Jet const*> candidates_leading_b;
  std::vector<xAOD::Jet const*> candidates_sub_b;

  for (unsigned int i(0); i < inputJets.size(); i++)
    {
      if(Props::MV2c10.get( inputJets.at(i) ) >btagweight_lead){
	if (fabs(inputJets.at(i)->eta())<2.4 and inputJets.at(i)->pt()>95e3){
	  candidates_leading_b.push_back( inputJets.at(i) );
	}
      }
      if(Props::MV2c10.get( inputJets.at(i) ) >btagweight_sub){
	if(fabs(inputJets.at(i)->eta())<2.5 and inputJets.at(i)->pt()>70e3){
	  candidates_sub_b.push_back( inputJets.at(i) );
	}
      }
    }

  
  // Choose signal jets to be the pair with highest pT passing tight btagging WP  
  double MaxPt = -1;

  _signalJets.push_back(nullptr);
  _signalJets.push_back(nullptr);

  for (unsigned int i(0); i < candidates_leading_b.size(); i++)
    {
      for (unsigned int j(0); j < candidates_sub_b.size(); j++)
	{
	  if (candidates_leading_b.at(i) == candidates_sub_b.at(j)) continue; // no same jet
	    
	  // TMP
	  //	  if (Props::HLTBJetMatched_J80_MV2c20_70.get(candidates_leading_b.at(i)) == Props::HLTBJetMatched_J60_MV2c20_85.get(candidates_sub_b.at(j)))// not matched to same HLT jet
	  //	    continue; 
	  
	  TLorentzVector HiggsVect = candidates_leading_b.at(i)->p4() + candidates_sub_b.at(j)->p4();
	  if ( HiggsVect.Pt() > MaxPt ) {
	    MaxPt = HiggsVect.Pt();
	    _signalJets.at(0) = candidates_leading_b.at(i);
	    _signalJets.at(1) = candidates_sub_b.at(j);
	  }
	}
    }

  if ( _signalJets.at(0) == nullptr ) return false;
  if ( _signalJets.at(1) == nullptr ) return false;

  std::vector<xAOD::Jet const*> candidates_forward;
  std::vector<xAOD::Jet const*> candidates_all;

  double maxMjj(-1.);
  xAOD::Jet const* vbfjet1;
  xAOD::Jet const* vbfjet2;

  // forward jets
  for (unsigned int i(0); i < inputJets.size(); i++) {
    if( fabs(inputJets.at(i)->eta())>3.2 and inputJets.at(i)->pt()>60e3){
      candidates_forward.push_back(inputJets.at(i));
    }
  }

  // not signal candidates
  for (unsigned int i(0); i < inputJets.size(); i++) {
    if ( WP == "overlap"){
      candidates_all.push_back( inputJets.at(i) );
    }
    else if ( inputJets.at(i) != _signalJets.at(0) and inputJets.at(i) != _signalJets.at(1)){
      candidates_all.push_back( inputJets.at(i) );
    }
  }
  
  if (candidates_forward.size()<1 or candidates_all.size() < 2) return false;

  for (unsigned int i(0); i < candidates_forward.size(); i++)
    {
      for (unsigned int j(0); j < candidates_all.size(); j++)
	{
	  TLorentzVector jet1(candidates_forward.at(i)->p4());
	  TLorentzVector jet2(candidates_all.at(j)->p4());
	  if (candidates_forward.at(i) == candidates_all.at(j)) continue;
	  
	  if((jet1+jet2).M() > maxMjj){
	    maxMjj = (jet1+jet2).M();
	    vbfjet1 = candidates_forward.at(i);
	    vbfjet2 = candidates_all.at(j);
	  }
	}
    }

  if (vbfjet1 == nullptr) return false;
  if (vbfjet2 == nullptr) return false;

  _vbfJets.push_back( vbfjet1 );
  _vbfJets.push_back( vbfjet2 );
  
  if (_debugMode) this->printResults();
  return true;
}


bool jetAssignmentTool::assignBtag_2cen_reverse(std::vector<xAOD::Jet const*> &inputJets, std::string WP) {

  double btagweight_lead =0;
  double btagweight_sub =0;

  if (WP == "85"){
    btagweight_lead = m_b_85_weight;
    btagweight_sub = m_b_85_weight;
  }
  if (WP == "77"){
    btagweight_lead = m_b_77_weight;
    btagweight_sub = m_b_77_weight;
  }
  if (WP == "70"){
    btagweight_lead = m_b_70_weight;
    btagweight_sub = m_b_70_weight;
  }
  if (WP == "asym"){
    btagweight_lead = m_b_70_weight;
    btagweight_sub = m_b_85_weight;
  }

  std::vector<xAOD::Jet const*> candidates_forward;
  std::vector<xAOD::Jet const*> candidates_all;

  double maxMjj(-1.);
  xAOD::Jet const* vbfjet1;
  xAOD::Jet const* vbfjet2;

  // forward jets
  for (unsigned int i(0); i < inputJets.size(); i++) {
    if( fabs(inputJets.at(i)->eta())>3.2 and inputJets.at(i)->pt()>60e3){
      candidates_forward.push_back(inputJets.at(i));
    }
  }

  for (unsigned int i(0); i < inputJets.size(); i++) {
    candidates_all.push_back( inputJets.at(i) );
  }
  
  if (candidates_forward.size()<1 or candidates_all.size() < 2) return false;

  for (unsigned int i(0); i < candidates_forward.size(); i++)
    {
      for (unsigned int j(0); j < candidates_all.size(); j++)
	{
	  TLorentzVector jet1(candidates_forward.at(i)->p4());
	  TLorentzVector jet2(candidates_all.at(j)->p4());
	  if (candidates_forward.at(i) == candidates_all.at(j)) continue;
	  
	  if((jet1+jet2).M() > maxMjj){
	    maxMjj = (jet1+jet2).M();
	    vbfjet1 = candidates_forward.at(i);
	    vbfjet2 = candidates_all.at(j);
	  }
	}
    }
  
  if (vbfjet1 == nullptr) return false;
  if (vbfjet2 == nullptr) return false;
  
  _vbfJets.push_back( vbfjet1 );
  _vbfJets.push_back( vbfjet2 );
  

  std::vector<xAOD::Jet const*> candidates_leading_b;
  std::vector<xAOD::Jet const*> candidates_sub_b;

  for (unsigned int i(0); i < inputJets.size(); i++)
    {
      
      if(inputJets.at(i) == _vbfJets.at(0) or inputJets.at(i) == _vbfJets.at(1)   ){
	continue;
      }

      if(Props::MV2c10.get( inputJets.at(i) ) >btagweight_lead ){
	if (fabs(inputJets.at(i)->eta())<2.4 and inputJets.at(i)->pt()>95e3 ){
	  candidates_leading_b.push_back( inputJets.at(i) );
	}
      }
      if(Props::MV2c10.get( inputJets.at(i) ) >btagweight_sub){
	if(fabs(inputJets.at(i)->eta())<2.5 and inputJets.at(i)->pt()>70e3){
	  candidates_sub_b.push_back( inputJets.at(i) );
	}
      }
    }

  
  // Choose signal jets to be the pair with highest pT passing tight btagging WP  
  double MaxPt = -1;

  _signalJets.push_back(nullptr);
  _signalJets.push_back(nullptr);

  for (unsigned int i(0); i < candidates_leading_b.size(); i++)
    {
      for (unsigned int j(0); j < candidates_sub_b.size(); j++)
	{
	  if (candidates_leading_b.at(i) == candidates_sub_b.at(j)) continue; // no same jet

	  // TMP	    
	  //	  if (Props::HLTBJetMatched_J80_MV2c20_70.get(candidates_leading_b.at(i)) == Props::HLTBJetMatched_J60_MV2c20_85.get(candidates_sub_b.at(j)))// not matched to same HLT jet
	  //	    continue;
	  
	  TLorentzVector HiggsVect = candidates_leading_b.at(i)->p4() + candidates_sub_b.at(j)->p4();
	  if ( HiggsVect.Pt() > MaxPt ) {
	    MaxPt = HiggsVect.Pt();
	    _signalJets.at(0) = candidates_leading_b.at(i);
	    _signalJets.at(1) = candidates_sub_b.at(j);
	  }
	}
    }

  if ( _signalJets.at(0) == nullptr ) return false;
  if ( _signalJets.at(1) == nullptr ) return false;

  if (_debugMode) this->printResults();
  return true;
}


bool jetAssignmentTool::assign0tag_4cen(std::vector<xAOD::Jet const*> &inputJets) {
  int nb = 0;
  for (unsigned int i(0); i < inputJets.size(); i++){
    if (Props::MV2c10.get( inputJets.at(i) ) > m_b_70_weight) nb++;
  }
  if (nb<2) return false;

  // start considering all the cases
  double MaxPt = -1;
  unsigned int signaljet1 = -1;
  unsigned int signaljet2 = -1;
  _signalJets.push_back(nullptr);
  _signalJets.push_back(nullptr);

  for (unsigned int i(0); i < inputJets.size(); i++)
    {
      for (unsigned int j(i+1); j < inputJets.size(); j++)
	{
	  if ( fabs(inputJets.at(i)->eta()>2.5) || fabs(inputJets.at(j)->eta()>2.5) ) continue;
	  
	  TLorentzVector jet1(inputJets.at(i)->p4());
	  TLorentzVector jet2(inputJets.at(j)->p4());
	  if((jet1+jet2).Pt() > MaxPt){
	    MaxPt = (jet1+jet2).Pt();
	    _signalJets.at(0) = inputJets.at(i);
	    _signalJets.at(1) = inputJets.at(j);
	    signaljet1 = i;
	    signaljet2 = j;
	  }
	}
    }
  
  if ( _signalJets.at(0) == nullptr ) return false;
  if ( _signalJets.at(1) == nullptr ) return false;
  
  double maxMjj(-1.);
  xAOD::Jet const* vbfjet1;
  xAOD::Jet const* vbfjet2;
  
  for (unsigned int i(0); i < inputJets.size(); i++)
    {
      if (i==signaljet1 or i==signaljet2)
	continue;
      for (unsigned int j(i+1); j < inputJets.size(); j++)
	{
	  if (j==signaljet1 or j==signaljet2)
	    continue;

	  TLorentzVector jet1(inputJets.at(i)->p4());
	  TLorentzVector jet2(inputJets.at(j)->p4());
	  if((jet1+jet2).M() > maxMjj){
	    maxMjj = (jet1+jet2).M();
	    vbfjet1 = inputJets.at(i);
	    vbfjet2 = inputJets.at(j);
	  }
	}
    }

  if (vbfjet1 == nullptr) return false;
  if (vbfjet2 == nullptr) return false;
  
  _vbfJets.push_back( vbfjet1 );
  _vbfJets.push_back( vbfjet2 );
  
  // sort results in pt
  std::sort(_signalJets.begin(),_signalJets.end(),JetSort::sortJetsByPt); // in this case 
  std::sort(_vbfJets.begin(),_vbfJets.end(),JetSort::sortJetsByPt);
  
  if (_debugMode) this->printResults();
  return true;
}


bool jetAssignmentTool::assign0tag_2cen(std::vector<xAOD::Jet const*> &inputJets) {

  std::vector<xAOD::Jet const*> candidates_J80;
  std::vector<xAOD::Jet const*> candidates_J60;

  for (unsigned int i(0); i < inputJets.size(); i++)
    {
      if(fabs(inputJets.at(i)->eta())<2.4 and inputJets.at(i)->pt()>95e3){
	candidates_J80.push_back( inputJets.at(i) );
      }
      if(fabs(inputJets.at(i)->eta())<2.5 and inputJets.at(i)->pt()>70e3){
	candidates_J60.push_back( inputJets.at(i) );
      }
    }
  

  // Choose signal jets to be the pair with highest pT passing tight btagging WP  
  double MaxPt = -1;

  _signalJets.push_back(nullptr);
  _signalJets.push_back(nullptr);

  for (unsigned int i(0); i < candidates_J80.size(); i++)
    {
      for (unsigned int j(0); j < candidates_J60.size(); j++)
	{
	  if (candidates_J80.at(i) == candidates_J60.at(j)) continue;
	      
	  TLorentzVector HiggsVect = inputJets.at(i)->p4() + inputJets.at(j)->p4();
	  if ( HiggsVect.Pt() > MaxPt ) {
	    MaxPt = HiggsVect.Pt();
	    _signalJets.at(0) = candidates_J80.at(i);
	    _signalJets.at(1) = candidates_J60.at(j);
	  }
	}
    }

  if ( _signalJets.at(0) == nullptr ) return false;
  if ( _signalJets.at(1) == nullptr ) return false;

  // find the pair of jets that yields the highest mjj for a foward jet and all other jets                                                                                                                
  std::vector<xAOD::Jet const*> candidates_forward;
  for (unsigned int i(0); i < inputJets.size(); i++) {
    if( fabs(inputJets.at(i)->eta())>3.2 and inputJets.at(i)->pt()>60e3) candidates_forward.push_back(inputJets.at(i));
  }

  if (candidates_forward.size()<1) return false;

  double maxMjj(-1.);
  xAOD::Jet const* vbfjet1;
  xAOD::Jet const* vbfjet2;

  for (unsigned int i(0); i < candidates_forward.size(); i++)
    {
      for (unsigned int j(0); j < inputJets.size(); j++)
	{
	  if (candidates_forward.at(i) == inputJets.at(j)) continue;
	  if (_signalJets.at(0)== inputJets.at(j)) continue;
	  if (_signalJets.at(1)== inputJets.at(j)) continue;

	  TLorentzVector jet1(candidates_forward.at(i)->p4());
	  TLorentzVector jet2(inputJets.at(j)->p4());
	  
	  if((jet1+jet2).M() > maxMjj){
	    maxMjj = (jet1+jet2).M();
	    vbfjet1 = candidates_forward.at(i);
	    vbfjet2 = inputJets.at(j);
	  }
	}
    }

  if (vbfjet1 == nullptr) return false;
  if (vbfjet2 == nullptr) return false;

  _vbfJets.push_back( vbfjet1 );
  _vbfJets.push_back( vbfjet2 );

  // sort results in pt
  std::sort(_signalJets.begin(),_signalJets.end(),JetSort::sortJetsByPt);
  std::sort(_vbfJets.begin(),_vbfJets.end(),JetSort::sortJetsByPt);

  if (_debugMode) this->printResults();
  return true;
}






/* Utilities */

bool JetSort::sortJetsByEta (xAOD::Jet const* jet1, xAOD::Jet const* jet2) { return (jet1->eta() > jet2->eta()); }
bool JetSort::sortJetsByBtag(xAOD::Jet const* jet1, xAOD::Jet const* jet2) { return (Props::MV2c10.get(jet1) > Props::MV2c10.get(jet2)); }
bool JetSort::sortJetsByPt  (xAOD::Jet const* jet1, xAOD::Jet const* jet2) { return (jet1->pt() > jet2->pt()); }
