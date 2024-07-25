file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../version.txt version)

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
  DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")

#
# Configure CPack
#
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")
set(CPACK_PACKAGE_CHECKSUM MD5)
set(CPACK_PACKAGE_VENDOR "The Eclipse Foundation")
set(CPACK_COMPONENT_LIB_DESCRIPTION "The C++ header wrapper library for Eclipse zenoh-c and zenoh-pico libraries")
set(CPACK_COMPONENT_DEV_DESCRIPTION "${CPACK_COMPONENT_LIB_DESCRIPTION} - devel files")
set(CPACK_GENERATOR ZIP)
# set package name to format <name>-<version>.<extension>
# platform name doesn't matter for header-only library
set(CPACK_PACKAGE_NAME zenoh-cpp)
if(NOT CPACK_PACKAGE_VERSION)
  set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}) 
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

if(UNIX)
  # DEB package
  if(NOT DEBARCH)
    execute_process(
      COMMAND dpkg --print-architecture
      OUTPUT_VARIABLE DEBARCH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()
  message(STATUS "Configure DEB packaging for Linux ${DEBARCH}")
  list(APPEND CPACK_GENERATOR DEB)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ZettaScale Zenoh Team, <zenoh@zettascale.tech>")
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${DEBARCH})
  set(CPACK_DEB_COMPONENT_INSTALL ON)
  set(CPACK_DEBIAN_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
  set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "")

  # RPM package
  # rpmbuild should be installed
  # apt install rpm
  if(NOT RPMARCH)
    set(RPMARCH ${CMAKE_SYSTEM_PROCESSOR})
  endif()
  message(STATUS "Configure RPM packaging for Linux ${RPMARCH}")
  list(APPEND CPACK_GENERATOR RPM)
  set(CPACK_RPM_PACKAGE_ARCHITECTURE ${RPMARCH})
  set(CPACK_RPM_COMPONENT_INSTALL ON)
  set(CPACK_RPM_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
  set(CPACK_RPM_DEV_PACKAGE_REQUIRES "")
endif()
include(CPack)
