#ifndef CXAODTOOLS_OVERLAPTOOLDR_H
#define CXAODTOOLS_OVERLAPTOOLDR_H

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"


namespace ORUtils
{

  /// @class InvDeltaROverlapTool
  /// @brief A simple overlap finder that uses a dR match.
  ///
  /// This class will remove _all_ objects that fit the criteria.
  /// The standard tool removes object in cont1 that overlap with 
  /// those in cont2.  This does the opposite!
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  /// Update provided by Peyton Rose <prose@ucsc.edu>
  class InvDeltaROverlapTool : public virtual IOverlapTool,
                            public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(InvDeltaROverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      InvDeltaROverlapTool(const std::string& name);

      /// @brief Identify overlaps with simple dR check.
      /// Flags all objects in cont1 which are found to overlap
      /// with any object in cont2 within the configured dR window.
      virtual StatusCode
      findOverlaps(const xAOD::IParticleContainer& cont1,
                   const xAOD::IParticleContainer& cont2) const override;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    private:

      //
      // Configurable properties
      //

      /// Delta-R cone for flagging objects as overlap.
      float m_dR;
      /// Calculate delta-R using rapidity
      bool m_useRapidity;

      //
      // Utilities
      //

      /// Delta-R matcher
      std::unique_ptr<DeltaRMatcher> m_dRMatcher;

  }; // class DeltaROverlapTool

} // namespace ORUtils

#endif
