include(FindPackageHandleStandardArgs)

find_path(Enet_INCLUDE_DIRS enet/enet.h)
find_library(Enet_LIBRARIES NAMES enet)

find_package_handle_standard_args(Enet DEFAULT_MSG Enet_INCLUDE_DIRS Enet_LIBRARIES )
mark_as_advanced(Enet_INCLUDE_DIRS Enet_LIBRARIES )

if( ENET_FOUND )
  if( NOT TARGET Enet::enet )
    add_library(Enet::enet UNKNOWN IMPORTED)
    set_target_properties(Enet::enet PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION ${Enet_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES ${Enet_INCLUDE_DIRS}
    )
  endif()
endif()
