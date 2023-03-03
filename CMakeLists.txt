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

macro(do_add_library name alias compile_def)
	add_library(${name} INTERFACE)
	target_compile_definitions(${name} INTERFACE ${compile_def})
	target_include_directories(${name} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
	add_library(${alias} ALIAS ${name})
endmacro()

do_add_library(zenohcpp_zenohc zenohcpp::zenohc ZENOHCPP_ZENOHC)
do_add_library(zenohcpp_zenohpico zenohcpp::zenohpico ZENOHCPP_ZENOHPICO)

#
# Components included only if project is the root project
#
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	enable_testing()
	include(cmake/helpers.cmake)
	set_default_build_type(Release)
	configure_include_project(ZENOHCPP_ZENOHC zenohc zenohc::lib "../zenoh-c" zenohc "https://github.com/eclipse-zenoh/zenoh-c" "")
	configure_include_project(ZENOHCPP_ZENOHPICO zenohpico zenohpico "../zenoh-pico" zenohpico "https://github.com/eclipse-zenoh/zenoh-pico" "")
	add_subdirectory(install)
	add_subdirectory(tests)
	add_subdirectory(examples)
endif()