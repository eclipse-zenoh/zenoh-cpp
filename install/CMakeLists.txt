file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../version.txt version)

status_print(CMAKE_INSTALL_PREFIX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

#
# Install headers
#
get_target_property(include_dirs zenohcxx INTERFACE_INCLUDE_DIRECTORIES)
foreach(dir ${include_dirs})
    install(DIRECTORY "${dir}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT dev)
endforeach()

#
# Install package config files
#

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/zenohcxx.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/zenohcxx.pc" @ONLY)
if(APPLE OR UNIX)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/zenohcxx.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig" OPTIONAL COMPONENT dev)
endif()

set(CMAKE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/zenohcxx")

# Generate <Package>Config.cmake
include(CMakePackageConfigHelpers)
configure_package_config_file(
  "PackageConfig.cmake.in"
  "zenohcxxConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")

# Generate <Package>Version.cmake
write_basic_package_version_file(
  "zenohcxxConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/zenohcxxConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/zenohcxxConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_CMAKEDIR}"
  COMPONENT dev)


# Configure CPack
#
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")
set(CPACK_PACKAGE_CHECKSUM MD5)
set(CPACK_PACKAGE_VENDOR "The Eclipse Foundation")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_COMPONENT_DEV_DESCRIPTION "The C++ header wrapper library for Eclipse zenoh-c and zenoh-pico libraries")
set(CPACK_COMPONENT_DEV_GROUP "dev")
set(CPACK_GENERATOR ZIP)
# set package name to format <name>-<version>.<extension>
# platform name doesn't matter for header-only library
set(CPACK_PACKAGE_NAME zenohcpp)

if(NOT CPACK_PACKAGE_VERSION)
    set(SEM_VER "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    if(PROJECT_VERSION_TWEAK STREQUAL "")
        set(CPACK_PACKAGE_VERSION ${SEM_VER})
    elseif(PROJECT_VERSION_TWEAK EQUAL 0)
        set(CPACK_PACKAGE_VERSION "${SEM_VER}~dev-1")
    elseif(PROJECT_VERSION_TWEAK GREATER 0)
        set(CPACK_PACKAGE_VERSION "${SEM_VER}~pre.${PROJECT_VERSION_TWEAK}-1")
    endif()
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cpack_project_config.cmake")

include(CPack)
