// Dear emacs, this is -*- c++ -*-
// $Id$
#ifndef CXAODTOOLS_LUMIMETADATATOOL_H
#define CXAODTOOLS_LUMIMETADATATOOL_H

// System include(s):
#include <string>
#include <memory>

// Infrastructure include(s):
#include "AsgTools/AsgMetadataTool.h"
#ifdef ASGTOOL_ATHENA
#   include "AthenaPoolKernel/IMetaDataTool.h"
#endif // ASGTOOL_ATHENA

// EDM include(s):
#include "xAODLuminosity/LumiBlockRangeContainer.h"
#include "xAODLuminosity/LumiBlockRangeAuxContainer.h"

   /// Tool taking care of propagating xAOD::LumiMetaData information
   ///
   class LumiMetaDataTool : public asg::AsgMetadataTool
#ifdef ASGTOOL_ATHENA
                          , public virtual ::IMetaDataTool
#endif // ASGTOOL_ATHENA
   {

      /// Declare the correct constructor for Athena
      ASG_TOOL_CLASS0( LumiMetaDataTool )

   public:
      /// Regular AsgTool constructor
      LumiMetaDataTool( const std::string& name = "LumiMetaDataTool" );

      /// Function initialising the tool
      virtual StatusCode initialize();

   protected:
      /// @name Functions called by the AsgMetadataTool base class
      /// @{

      /// Function collecting the metadata from a new input file
      virtual StatusCode beginInputFile();

      /// Function making sure that BeginInputFile incidents are not missed
      virtual StatusCode beginEvent();

      /// Function writing the collected metadata to the output
      virtual StatusCode metaDataStop();

      /// @}

   private:
      /// Key of the metadata object in the input file
      std::string m_inputKey;
      /// Key of the metadata object for the output file
      std::string m_outputKey;

      /// The output interface object
      std::unique_ptr< xAOD::LumiBlockRangeContainer > m_md;
      /// The output auxiliary object
      std::unique_ptr< xAOD::LumiBlockRangeAuxContainer > m_mdAux;

      /// Internal flag for keeping track of whether a BeginInputFile incident
      /// was seen already
      bool m_beginFileIncidentSeen;

   }; // class LumiMetaDataTool

#endif // CXAODTOOLS_LUMIMETADATATOOL_H
