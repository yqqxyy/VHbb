################################################################################
# Build configuration for CxAODOperations
################################################################################

# Declare the name of the package:
atlas_subdir( CxAODOperations )

# Declare the package's dependencies:
atlas_depends_on_subdirs()

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )
