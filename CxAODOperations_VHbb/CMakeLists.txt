################################################################################
# Build configuration for CxAODOperations_VHbb
################################################################################

# Declare the name of the package:
atlas_subdir( CxAODOperations_VHbb )

# Declare the package's dependencies:
atlas_depends_on_subdirs()

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )
