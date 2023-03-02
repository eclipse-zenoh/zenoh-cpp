cmake_minimum_required(VERSION 3.20)
project(
	zenohcpp
	VERSION 0.7.0.1
	DESCRIPTION "C++ bindings for Zenoh"
	HOMEPAGE_URL "https://github.com/ZettaScaleLabs/zenoh-cpp"
	LANGUAGES C CXX
)
add_library(zenohcpp INTERFACE)
target_include_directories(zenohcpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

#
# Components included only if project is the root project
#
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	enable_testing()
	include(cmake/helpers.cmake)
	set_default_build_type(Release)
	configure_include_project(ZENOHCPP_ZENOHC zenohc zenohc::lib "../zenoh-c" zenohc "https://github.com/eclipse-zenoh/zenoh-c" "")
	add_subdirectory(install)
	add_subdirectory(tests)
	add_subdirectory(examples)
endif()

# #
# # Configure installation
# #
# message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
# include(GNUInstallDirs)
# set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/zenohcpp")
# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# configure_file("${CMAKE_CURRENT_SOURCE_DIR}/zenohcpp.pc.in" "${CMAKE_CURRENT_SOURCE_DIR}/zenohcpp.pc" @ONLY)
# if(APPLE OR UNIX)
# 	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/zenohc.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig" OPTIONAL)
# endif()
# set(CMAKE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# # Generate <Package>Config.cmake
# include(CMakePackageConfigHelpers)
# configure_package_config_file(
#   "PackageConfig.cmake.in"
#   "${PROJECT_NAME}Config.cmake"
#   INSTALL_DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")

# # Generate <Package>Version.cmake
# write_basic_package_version_file(
#   "${PROJECT_NAME}ConfigVersion.cmake"
#   VERSION ${PROJECT_VERSION}
#   COMPATIBILITY SameMajorVersion)

# install(
#   FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
#         "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#   DESTINATION "${CMAKE_INSTALL_CMAKEDIR}"
#   COMPONENT dev)
