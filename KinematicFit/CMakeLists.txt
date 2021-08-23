atlas_subdir( KinematicFit )

find_package( ROOT COMPONENTS Core Tree MathCore Hist Physics RIO Minuit )

atlas_add_library( KinematicFit
   KinematicFit/*.h Root/*.h Root/*.cxx ${_dictionarySource}
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   PUBLIC_HEADERS KinematicFit
   LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )
