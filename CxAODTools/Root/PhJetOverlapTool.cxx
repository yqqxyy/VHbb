// System includes
#include <typeinfo>
#include "AssociationUtils/DeltaRMatcher.h"

// Framework includes
#include "CxxUtils/make_unique.h"
#include "AthContainers/ConstDataVector.h"

// Local includes
#include "CxAODTools/PhJetOverlapTool.h"

static const double GeV = 1e3;
static const double invGeV = 1e-3;

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  PhJetOverlapTool::PhJetOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("BJetLabel", m_bJetLabel = "",
                    "Input b-jet flag. Disabled by default.");
    declareProperty("ApplyJVT", m_applyJVT = true,
                    "Activate JVT requirement");
    declareProperty("JVT", m_jvt = 0.59,
                    "JVT requirement for photon removal");
    declareProperty("JVTPt", m_jvtPt = 60.*GeV,
                    "Max PT for the JVT requirement");
    declareProperty("JVTEta", m_jvtEta = 2.4,
                    "Max abs(eta) for the JVT requirement");
    declareProperty("InnerDR", m_innerDR = 0.2,
                    "Inner cone for removing jets");
    declareProperty("OuterDR", m_outerDR = 0.4,
                    "Outer cone for removing photons");
    declareProperty("UseSlidingDR", m_useSlidingDR = false,
                    "Use sliding dR cone to reject photons");
    declareProperty("SlidingDRC1", m_slidingDRC1 = 0.04,
                    "The constant offset for sliding dR");
    declareProperty("SlidingDRC2", m_slidingDRC2 = 10.*GeV,
                    "The inverse muon pt factor for sliding dR");
    declareProperty("SlidingDRMaxCone", m_slidingDRMaxCone = 0.4,
                    "Maximum size of sliding dR cone");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode PhJetOverlapTool::initializeDerived()
  {
    using CxxUtils::make_unique;

    // Initialize the b-jet helper
    if(!m_bJetLabel.empty()) {
      ATH_MSG_DEBUG("Configuring btag-aware OR with btag label: " << m_bJetLabel);
      m_bJetHelper = make_unique<BJetHelper>(m_bJetLabel);
    }

    // Initialize the dR matchers
    m_dRMatchCone1 = make_unique<DeltaRMatcher>(m_innerDR, m_useRapidity);
    if(m_useSlidingDR) {
      ATH_MSG_DEBUG("Configuring sliding outer cone for ph-jet OR with " <<
                    "constants C1 = " << m_slidingDRC1 << ", C2 = " <<
                    m_slidingDRC2 << ", MaxCone = " << m_slidingDRMaxCone);
      m_dRMatchCone2 =
        make_unique<SlidingDeltaRMatcher>
          (m_slidingDRC1, m_slidingDRC2, m_slidingDRMaxCone, m_useRapidity);
    }
    else {
      m_dRMatchCone2 = make_unique<DeltaRMatcher>(m_outerDR, m_useRapidity);
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode PhJetOverlapTool::
  findOverlaps(const xAOD::IParticleContainer& cont1,
               const xAOD::IParticleContainer& cont2) const
  {
    // Check the container types
    if(typeid(cont1) != typeid(xAOD::JetContainer) &&
       typeid(cont1) != typeid(ConstDataVector<xAOD::JetContainer>)) {
      ATH_MSG_ERROR("Second container arg is not of type JetContainer!");
      return StatusCode::FAILURE;
    }
    if(typeid(cont2) != typeid(xAOD::PhotonContainer) &&
       typeid(cont2) != typeid(ConstDataVector<xAOD::PhotonContainer>)) {
      ATH_MSG_ERROR("First container arg is not a PhotonContainer!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK( findOverlaps(static_cast<const xAOD::JetContainer&>(cont1),
			    static_cast<const xAOD::PhotonContainer&>(cont2) ) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode PhJetOverlapTool::
  findOverlaps(const xAOD::JetContainer& jets,
	       const xAOD::PhotonContainer& photons) const
  {
    ATH_MSG_DEBUG("Removing overlapping photons and jets");

    // Initialize output decorations if necessary
    m_decHelper->initializeDecorations(photons);
    m_decHelper->initializeDecorations(jets);

    // First flag overlapping jets
    for(const auto photon : photons){
      if(!m_decHelper->isSurvivingObject(*photon)) continue;

      for(const auto jet : jets){
        if(!m_decHelper->isSurvivingObject(*jet)) continue;
        // Don't reject user-defined b-tagged jets
        if(m_bJetHelper && m_bJetHelper->isBJet(*jet)) continue;

        if(m_dRMatchCone1->objectsMatch(*jet, *photon)){
          ATH_MSG_DEBUG("  Found overlap jet: " << jet->pt()*invGeV);
          m_decHelper->setObjectFail(*jet);
          if(m_objLinkHelper)
            ATH_CHECK( m_objLinkHelper->addObjectLink(*jet, *photon) );
        }
      }
    }

    // Now flag overlapping photons
    for(const auto jet: jets){
      if(!m_decHelper->isSurvivingObject(*jet)) continue;
      // Don't reject photons from pileup jets
      if(isPileupJet(*jet)) continue;

      for(const auto photon : photons){
        if(!m_decHelper->isSurvivingObject(*photon)) continue;

        if(m_dRMatchCone2->objectsMatch(*photon, *jet)){
          ATH_MSG_DEBUG("  Found overlap ph: " << photon->pt()*invGeV);
          m_decHelper->setObjectFail(*photon);
          if(m_objLinkHelper)
            ATH_CHECK( m_objLinkHelper->addObjectLink(*photon, *jet) );
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify a pileup jet
  //---------------------------------------------------------------------------
  bool PhJetOverlapTool::
  isPileupJet(const xAOD::Jet& jet) const
  {
    return (m_applyJVT &&
	    jet.pt() < m_jvtPt &&
	    jet.getAttribute<xAOD::JetFourMom_t>("JetEMScaleMomentum").eta() < m_jvtEta &&
	    jet.getAttribute<float>("Jvt") < m_jvt);
  }
  
} // namespace ORUtils
