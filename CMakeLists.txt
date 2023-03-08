cmake_minimum_required(VERSION 3.20)
project(
	zenohcpp
	VERSION 0.7.0.1
	DESCRIPTION "C++ bindings for Zenoh"
	HOMEPAGE_URL "https://github.com/ZettaScaleLabs/zenoh-cpp"
	LANGUAGES C CXX
)

# zenohcpp without dependencies
add_library(zenohcpp INTERFACE)
target_include_directories(zenohcpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# zenohcpp for zenohc dynamic
add_library(zenohcpp_zenohc_lib INTERFACE)
target_compile_definitions(zenohcpp_zenohc_lib INTERFACE ZENOHCPP_ZENOHC)
target_include_directories(zenohcpp_zenohc_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_dependencies(zenohcpp_zenohc_lib zenohc::lib)
target_link_libraries(zenohcpp_zenohc_lib INTERFACE zenohc::lib)
add_library(zenohcpp::zenohc::lib ALIAS zenohcpp_zenohc_lib)

# zenohcpp for zenohc static
add_library(zenohcpp_zenohc_static INTERFACE)
target_compile_definitions(zenohcpp_zenohc_static INTERFACE ZENOHCPP_ZENOHC)
target_include_directories(zenohcpp_zenohc_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_dependencies(zenohcpp_zenohc_static zenohc::static)
target_link_libraries(zenohcpp_zenohc_static INTERFACE zenohc::static)
add_library(zenohcpp::zenohc::static ALIAS zenohcpp_zenohc_static)

# zenohcpp for zenohpico
add_library(zenohcpp_zenohpico INTERFACE)
target_compile_definitions(zenohcpp_zenohpico INTERFACE ZENOHCPP_ZENOHPICO)
if(WIN32)
	target_compile_definitions(zenohcpp_zenohpico INTERFACE ZENOH_WINDOWS)
elseif(APPLE)
	target_compile_definitions(zenohcpp_zenohpico INTERFACE ZENOH_MACOS)
elseif(UNIX)
	target_compile_definitions(zenohcpp_zenohpico INTERFACE ZENOH_LINUX)
endif()
target_include_directories(zenohcpp_zenohpico INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_dependencies(zenohcpp_zenohpico zenohpico)
target_link_libraries(zenohcpp_zenohpico INTERFACE zenohpico)
add_library(zenohcpp::zenohpico ALIAS zenohcpp_zenohpico)

#
# Components included only if project is the root project
#
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	enable_testing()
	include(cmake/helpers.cmake)
	set_default_build_type(Release)
	configure_include_project(ZENOHCPP_ZENOHC zenohc zenohc::lib "../zenoh-c" zenohc "https://github.com/eclipse-zenoh/zenoh-c" "")
	configure_include_project(ZENOHCPP_ZENOHPICO zenohpico zenohpico "../zenoh-pico" zenohpico "https://github.com/eclipse-zenoh/zenoh-pico" "")
	# add_subdirectory(install)
	add_subdirectory(tests)
	add_subdirectory(examples)
endif()