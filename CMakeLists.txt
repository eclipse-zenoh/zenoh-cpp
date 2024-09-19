cmake_minimum_required(VERSION 3.16)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.txt version)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/zenoh-c-branch.txt zenoh_c_branch)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/zenoh-pico-branch.txt zenoh_pico_branch)

project(
	zenohcxx
	VERSION ${version}
	DESCRIPTION "C++ bindings for Zenoh"
	HOMEPAGE_URL "https://github.com/eclipse-zenoh/zenoh-cpp"
	LANGUAGES C CXX
)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
include(helpers)

set(project_version "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
if(NOT DEFINED PROJECT_VERSION_TWEAK)
	set(project_version "${project_version}")
elseif(PROJECT_VERSION_TWEAK EQUAL 0)
	set(project_version "${project_version}-dev")
elseif(PROJECT_VERSION_TWEAK GREATER 1)
	set(project_version "${project_version}-pre.${PROJECT_VERSION_TWEAK}")
endif()
status_print(project_version)

declare_cache_var(ZENOHCXX_ZENOHC ON BOOL "Build for Zenoh-c target")
declare_cache_var(ZENOHCXX_ZENOHPICO OFF BOOL "Build for Zenoh-pico target")
declare_cache_var(ZENOHCXX_EXAMPLES_PROTOBUF ON BOOL "Build Protobuf example (turn off if you have problems with installed Protobuf version)")

set_default_build_type(Release)

# zenohcxx without dependencies
add_library(zenohcxx INTERFACE)
target_include_directories(zenohcxx INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(ZENOHCXX_ZENOHPICO) 
	find_package(zenohpico REQUIRED)
	if(TARGET zenohpico::lib)
		message(STATUS "defined lib target zenohcxx::zenohpico for zenohpico::lib")
		add_library(zenohcxx_zenohpico INTERFACE)
		target_compile_definitions(zenohcxx_zenohpico INTERFACE ZENOHCXX_ZENOHPICO)
		target_include_directories(zenohcxx_zenohpico INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
		add_dependencies(zenohcxx_zenohpico zenohpico::lib)
		target_link_libraries(zenohcxx_zenohpico INTERFACE zenohpico::lib)
		add_library(zenohcxx::zenohpico ALIAS zenohcxx_zenohpico)
	else()
		message(FATAL_ERROR "zenohpico::lib target not found")
	endif()
endif()

if(ZENOHCXX_ZENOHC)
	find_package(zenohc REQUIRED)
	if(TARGET zenohc::lib)
		message(STATUS "defined lib target zenohcxx::zenohc::lib for zenohc::lib")
		add_library(zenohcxx_zenohc INTERFACE)
		target_compile_definitions(zenohcxx_zenohc INTERFACE ZENOHCXX_ZENOHC)
		target_include_directories(zenohcxx_zenohc INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
		add_dependencies(zenohcxx_zenohc zenohc::lib)
		target_link_libraries(zenohcxx_zenohc INTERFACE zenohc::lib)
		add_library(zenohcxx::zenohc ALIAS zenohcxx_zenohc)
	else()
		message(FATAL_ERROR "zenohc::lib target not found")
	endif()
endif()


add_subdirectory(install)
enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(docs)
