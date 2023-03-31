if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    # Settings when 'examples' is the root projet
    cmake_minimum_required(VERSION 3.20)
    project(
        zenohcxx
        VERSION 0.7.0.1
        DESCRIPTION "C++ bindings for Zenoh"
        HOMEPAGE_URL "https://github.com/ZettaScaleLabs/zenoh-cpp"
        LANGUAGES C CXX
    )
    include(../cmake/helpers.cmake)
    include_project(zenohcxx TARGET zenohcxx PATH "..")
else()
    message(STATUS "zenoh-cpp install")
endif()

status_print(CMAKE_INSTALL_PREFIX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

#
# Install headers
#
get_target_property(include_dirs zenohcxx INTERFACE_INCLUDE_DIRECTORIES)
foreach(dir ${include_dirs})
    install(DIRECTORY "${dir}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endforeach()

#
# Install package config files
#

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/zenohcxx.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/zenohcxx.pc" @ONLY)
if(APPLE OR UNIX)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/zenohcxx.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig" OPTIONAL)
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
