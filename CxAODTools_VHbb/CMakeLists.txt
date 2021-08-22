################################################################################
# Build configuration for CxAODTools_VHbb
################################################################################

# Declare the name of the package:
atlas_subdir( CxAODTools_VHbb )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC 
   CxAODTools
   PhysicsAnalysis/D3PDTools/EventLoop
   Reconstruction/Jet/BoostedJetTaggers
   )

# External dependencies:
find_package( ROOT COMPONENTS TMVA XMLParser)



# Build a shared library:
atlas_add_library( CxAODTools_VHbb
   CxAODTools_VHbb/*.h Root/*.h Root/*.cxx ${_dictionarySource}
   PUBLIC_HEADERS CxAODTools_VHbb
   LINK_LIBRARIES ${ROOT_LIBRARIES} EventLoop CxAODTools BoostedJetTaggersLib )

# Install files from the package:
atlas_install_scripts( scripts/*.sh )

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )
