################################################################################
# Build configuration for CxAODReader
################################################################################

# Declare the name of the package:
atlas_subdir( CxAODReader )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC 
   PhysicsAnalysis/D3PDTools/EventLoop
   CxAODTools
   Control/xAODRootAccess
   )

find_package( ROOT COMPONENTS Core Tree MathCore Hist Physics TMVA XMLParser XMLIO)

# Build a dictionary for the library:
atlas_add_root_dictionary( CxAODReader _dictionarySource
   ROOT_HEADERS Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

# Build a shared library:
atlas_add_library( CxAODReader
   CxAODReader/*.h Root/*.h Root/*.cxx ${_dictionarySource}
   PUBLIC_HEADERS CxAODReader
   LINK_LIBRARIES ${ROOT_LIBRARIES} EventLoop xAODRootAccess CxAODTools)
