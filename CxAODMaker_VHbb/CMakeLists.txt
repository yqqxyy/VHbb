################################################################################
# Build configuration for CxAODMaker_VHbb
################################################################################

# Declare the name of the package:
atlas_subdir( CxAODMaker_VHbb )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC 
   CxAODMaker
   CxAODTools_VHbb
   PhysicsAnalysis/D3PDTools/EventLoop
   PhysicsAnalysis/D3PDTools/EventLoopGrid
   Event/xAOD/xAODBase
   Event/xAOD/xAODEventInfo
   Control/xAODRootAccess
   PhysicsAnalysis/AnalysisCommon/PATInterfaces/PATInterfaces
   )

find_package( ROOT COMPONENTS Core RIO Hist Tree )

# Build a dictionary for the library:
atlas_add_root_dictionary( CxAODMaker_VHbb _dictionarySource
   ROOT_HEADERS Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

# Build a shared library:
atlas_add_library( CxAODMaker_VHbb
   CxAODMaker_VHbb/*.h Root/*.h Root/*.cxx ${_dictionarySource}
   PUBLIC_HEADERS CxAODMaker_VHbb
   LINK_LIBRARIES EventLoop CxAODMaker CxAODTools_VHbb )

# Install files from the package:
atlas_add_executable( hsg5framework
   util/hsg5framework.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODBase xAODRootAccess xAODEventInfo PATInterfaces
   EventLoop EventLoopGrid CxAODMaker CxAODMaker_VHbb CxAODTools
   )

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )
