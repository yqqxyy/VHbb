#include "CxAODMaker/TruthJetHandler.h"
#include "CxAODMaker/EventInfoHandler.h"

TruthJetHandler::TruthJetHandler(const std::string& name, ConfigStore & config, xAOD::TEvent * event,
                             EventInfoHandler & eventInfoHandler) : 
  ObjectHandler(name, config, event, eventInfoHandler),
  m_isMC(0),
  m_storeTruthJet4Vector(false)
{
  using std::placeholders::_1;
  m_selectFcns.clear();
  m_selectFcns.push_back(std::bind( &TruthJetHandler::passTruthJet, this, _1));
  m_config.getif<bool>("storeTruthJet4Vector",m_storeTruthJet4Vector);

}


TruthJetHandler::~TruthJetHandler() 
{

}

EL::StatusCode TruthJetHandler::initializeTools()
{

  m_isMC = m_eventInfoHandler.get_isMC();
  
  // register ISystematicsTools
  //---------------------------
  m_sysToolList.clear();

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode TruthJetHandler::decorate(xAOD::Jet *) 
{  
  return EL::StatusCode::SUCCESS;
} 

EL::StatusCode TruthJetHandler::calibrateCopies(xAOD::JetContainer * particles, const CP::SystematicSet & /*sysSet*/) 
{

  // loop over the particles in the container
  for (xAOD::Jet * jet : *particles) {
    //this ensures that the DynAux container has the same number of entries as the original one
    setP4( jet , jet );

    // print particle (pt,eta,phi,e,m)
    //std::cout << "===> Fat jet <===" << std::endl;
    //printParticle(jet);
    
    // apply calibrations

    // decorate jet
    if ( decorate( jet ) != EL::StatusCode::SUCCESS ) return EL::StatusCode::FAILURE;
    
  }

  return EL::StatusCode::SUCCESS;

}

bool TruthJetHandler::passTruthJet(xAOD::Jet* jet)
{
  bool passTruthCut=true;
  Props::passPreSel.set(jet, passTruthCut);
  return passTruthCut;
}

EL::StatusCode TruthJetHandler::writeOutputVariables(xAOD::Jet* inJet, xAOD::Jet* outJet, bool isKinVar, bool isWeightVar, const TString& sysName)
{
   if(!isWeightVar){
    // set four momentum
     //we do not set it to save space, as we only care about the truth jet matched in DR to the reco jet
     //which we add as a signal state already
    //setP4( inJet , outJet );
   }
   if(m_isMC) {
     Props::HadronConeExclTruthLabelID.copyIfExists(inJet, outJet);
     if(m_storeTruthJet4Vector) {
       Props::pt.copy(inJet, outJet);
       Props::m.copy(inJet, outJet);
       Props::eta.copy(inJet, outJet);
       Props::phi.copy(inJet, outJet);
       static SG::AuxElement::ConstAccessor<float>    acc_ECF1 ("ECF1"); //accessors for the energy correlation functions
       static SG::AuxElement::ConstAccessor<float>    acc_ECF2 ("ECF2");
       static SG::AuxElement::ConstAccessor<float>    acc_ECF3 ("ECF3");
       //check if the ECFs are present since they only exist for fat jets
       if (acc_ECF1.isAvailable(*inJet) && acc_ECF2.isAvailable(*inJet) && acc_ECF3.isAvailable(*inJet)){ 
         const float& ecf1 = inJet->getAttribute<float>("ECF1"); //access the ECFs
         const float& ecf2 = inJet->getAttribute<float>("ECF2");
         const float& ecf3 = inJet->getAttribute<float>("ECF3");
         float d2 = 0;
         if (fabs(ecf2) > 0) {
           d2 = ecf3 * pow(ecf1, 3) / pow(ecf2, 3); //calculate D2
         }
         Props::D2.set(outJet, d2); //fill D2
       }// end check on ECF accessors
     }
   }
   return writeCustomVariables(inJet, outJet, isKinVar, isWeightVar, sysName);
}

EL::StatusCode TruthJetHandler::writeCustomVariables(xAOD::Jet*, xAOD::Jet*, bool, bool, const TString&)
{
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TruthJetHandler::clearEvent() 
{
  EL_CHECK("TruthJetHandler::clearEvent()",ObjectHandler::clearEvent());
  return EL::StatusCode::SUCCESS;

}
