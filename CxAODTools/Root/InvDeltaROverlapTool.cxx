// Framework includes
#include "CxxUtils/make_unique.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"

// Local includes
#include "CxAODTools/InvDeltaROverlapTool.h"

static const float invGeV = 0.001;

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  InvDeltaROverlapTool::InvDeltaROverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("DR", m_dR = 0.4, "Maximum dR for overlap match");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode InvDeltaROverlapTool::initializeDerived()
  {
    using CxxUtils::make_unique;

    // Initialize the dR matcher
    m_dRMatcher = make_unique<DeltaRMatcher>(m_dR, m_useRapidity);

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode InvDeltaROverlapTool::
  findOverlaps(const xAOD::IParticleContainer& cont1,
               const xAOD::IParticleContainer& cont2) const
  {
    ATH_MSG_DEBUG("Removing overlaps");

    // Initialize output decoration if necessary
    m_decHelper->initializeDecorations(cont1);
    m_decHelper->initializeDecorations(cont2);

    // Loop over surviving input objects in cont1
    for(const auto p2 : cont2){
      if(m_decHelper->isSurvivingObject(*p2)){
        // Loop over surviving input objects in cont2
        for(const auto p1 : cont1){
          if(m_decHelper->isSurvivingObject(*p1)){
            // Check for duplicates and overlap
            if(p1 != p2 && m_dRMatcher->objectsMatch(*p2, *p1)){
              ATH_MSG_DEBUG("  Found overlap " << p2->type() <<
                            " pt " << p2->pt()*invGeV);
	      //std::printf("Found an object of type %s overlapping with object of type %s.  Removing the second!\n",
	      //typeid(cont1) == typeid(xAOD::PhotonContainer) ? "photon" : "electron", typeid(cont2) == typeid(xAOD::ElectronContainer) ? "electron" : "photon");
              m_decHelper->setObjectFail(*p2);
              if(m_objLinkHelper)
                ATH_CHECK( m_objLinkHelper->addObjectLink(*p2, *p1) );
            }
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }

} // namespace ORUtils
