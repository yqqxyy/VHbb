// $Id$

// Local include(s):
#include "CxAODTools/LumiMetaDataTool.h"

#include "xAODLuminosity/LumiBlockRange.h"

   LumiMetaDataTool::LumiMetaDataTool( const std::string& name )
      : asg::AsgMetadataTool( name ),
        m_md(), m_mdAux(), m_beginFileIncidentSeen( false ) {

      declareProperty( "InputKey", m_inputKey = "LumiBlocks" );
      declareProperty( "OutputKey", m_outputKey = "LumiBlocks" );

#ifdef ASGTOOL_ATHENA
      declareInterface< ::IMetaDataTool >( this );
#endif // ASGTOOL_ATHENA
   }

   StatusCode LumiMetaDataTool::initialize() {

      // Greet the user:
      ATH_MSG_DEBUG( "Initialising LumiMetaDataTool" );
      ATH_MSG_DEBUG( "  InputKey  = " << m_inputKey );
      ATH_MSG_DEBUG( "  OutputKey = " << m_outputKey );

      // Reset the member variable(s):
      m_md.reset();
      m_mdAux.reset();
      m_beginFileIncidentSeen = false;

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode LumiMetaDataTool::beginInputFile() {

      // Whatever happens, we've seen the incident:
      m_beginFileIncidentSeen = true;

      // If the input file doesn't have any file-level metadata, then
      // finish right away:
      if( ! inputMetaStore()->contains< xAOD::LumiBlockRangeContainer >( m_inputKey ) ) {
         return StatusCode::SUCCESS;
      }

      // Retrieve the input object:
      const xAOD::LumiBlockRangeContainer* input = 0;
      ATH_CHECK( inputMetaStore()->retrieve( input, m_inputKey ) );

      // Create the output objects if they don't exist yet:
      if( ( ! m_md.get() ) && ( ! m_mdAux.get() ) ) {
         ATH_MSG_DEBUG( "Creating output objects" );
         m_md.reset( new xAOD::LumiBlockRangeContainer() );
         m_mdAux.reset( new xAOD::LumiBlockRangeAuxContainer() );
         m_md->setStore( m_mdAux.get() );

         //// Copy the payload of the input object:
         //*( m_md.get() ) = *input;
      }

      // Make sure that the objects are compatible:
      //if( *( m_md.get() ) != *input ) {
      //}

      ATH_MSG_INFO( "LumiMetaDataTool: beginInputFile" );
      for(const xAOD::LumiBlockRange* lb : *input) {
        xAOD::LumiBlockRange* newlb = new xAOD::LumiBlockRange(*lb);
        m_md.get()->push_back(newlb);
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode LumiMetaDataTool::beginEvent() {

      // In case we missed the BeginInputFile incident for the first input file,
      // make sure that we still run the appropriate function.
      if( ! m_beginFileIncidentSeen ) {
        ATH_MSG_INFO( "LumiMetaDataTool: doing stuff in beginEvent" );
         ATH_CHECK( beginInputFile() );
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   StatusCode LumiMetaDataTool::metaDataStop() {

      // Don't be offended if the metadata already exists in the output:
      if( outputMetaStore()->contains< xAOD::LumiBlockRangeContainer >( m_outputKey ) ) {
         ATH_MSG_DEBUG( "xAOD::LumiMetaData already in the output" );
         return StatusCode::SUCCESS;
      }

      // Record the metadata, if any was found on the input:
      if( m_md.get() && m_mdAux.get() ) {
         ATH_MSG_DEBUG( "Recoding file level metadata" );
         ATH_CHECK( outputMetaStore()->record( m_md.release(), m_outputKey ) );
         ATH_CHECK( outputMetaStore()->record( m_mdAux.release(),
                                               m_outputKey + "Aux." ) );
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

