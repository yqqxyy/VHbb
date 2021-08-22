################################################################################
# Build configuration for CxAODReader_VHbb
################################################################################

# Declare the name of the package:
atlas_subdir( CxAODReader_VHbb )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC 
   CxAODReader
   CxAODTools_VHbb
   CorrsAndSysts
   KinematicFit
   Reconstruction/Jet/JetUncertainties
   PhysicsAnalysis/D3PDTools/EventLoop
   PhysicsAnalysis/D3PDTools/EventLoopGrid
   Event/xAOD/xAODBase
   Event/xAOD/xAODEventInfo
   Control/xAODRootAccess
   PhysicsAnalysis/AnalysisCommon/PATInterfaces/PATInterfaces
   )

find_package( ROOT COMPONENTS Core Tree MathCore Hist Physics TMVA XMLParser XMLIO)

# Build a dictionary for the library:
atlas_add_root_dictionary( CxAODReader_VHbb _dictionarySource
   ROOT_HEADERS Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

# Build a shared library:
atlas_add_library( CxAODReader_VHbb
   CxAODReader_VHbb/*.h Root/*.h Root/*.cxx ${_dictionarySource}
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   PUBLIC_HEADERS CxAODReader_VHbb
   LINK_LIBRARIES ${ROOT_LIBRARIES} CxAODReader CxAODTools_VHbb CorrsAndSysts KinematicFit JetUncertaintiesLib)

atlas_add_executable( hsg5frameworkReadCxAOD
   util/hsg5frameworkReadCxAOD.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODBase xAODRootAccess xAODEventInfo PATInterfaces
   EventLoop EventLoopGrid CxAODReader CxAODReader_VHbb CxAODTools
   )

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )
