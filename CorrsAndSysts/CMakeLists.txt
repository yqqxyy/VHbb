################################################################################
# Build configuration for CorrsAndSysts
################################################################################

# Declare the name of the package:
atlas_subdir( CorrsAndSysts )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC 
   PMGTools
   )

find_package( ROOT COMPONENTS Core Tree MathCore TMVA Hist Physics XMLParser XMLIO)

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )


# Build a shared library:
atlas_add_library( CorrsAndSysts
   CorrsAndSysts/*.h Root/*.h Root/*.cxx ${_dictionarySource}
   PUBLIC_HEADERS CorrsAndSysts
   LINK_LIBRARIES ${ROOT_LIBRARIES} PMGToolsLib)
