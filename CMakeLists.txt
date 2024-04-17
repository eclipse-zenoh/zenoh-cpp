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

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	enable_testing()
	set_default_build_type(Release)
	configure_include_project(ZENOHCXX_ZENOHPICO zenohpico zenohpico "../zenoh-pico" zenohpico "https://github.com/eclipse-zenoh/zenoh-pico" ${zenoh_pico_branch})
	configure_include_project(ZENOHCXX_ZENOHC zenohc zenohc::lib "../zenoh-c" zenohc "https://github.com/eclipse-zenoh/zenoh-c" ${zenoh_c_branch})
endif()

#
# When changing code below change also corresponding code in install/PackageConfig.cmake.in
# 

# zenohcxx without dependencies
add_library(zenohcxx INTERFACE)
target_include_directories(zenohcxx INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# zenohcxx for zenohpico
if(TARGET zenohpico)
	message(STATUS "defined lib target zenohcxx::zenohpico for zenohpico")
	add_library(zenohcxx_zenohpico INTERFACE)
	target_compile_definitions(zenohcxx_zenohpico INTERFACE ZENOHCXX_ZENOHPICO)
	target_include_directories(zenohcxx_zenohpico INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
	add_dependencies(zenohcxx_zenohpico zenohpico)
	target_link_libraries(zenohcxx_zenohpico INTERFACE zenohpico)
	add_library(zenohcxx::zenohpico ALIAS zenohcxx_zenohpico)
endif()

# zenohcxx for zenohc static/dynamic depending on ZENOHC_LIB_STATIC
if(TARGET zenohc::lib)
	message(STATUS "defined lib target zenohcxx::zenohc::lib for zenohc::lib")
	add_library(zenohcxx_zenohc_lib INTERFACE)
	target_compile_definitions(zenohcxx_zenohc_lib INTERFACE ZENOHCXX_ZENOHC)
	target_include_directories(zenohcxx_zenohc_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
	add_dependencies(zenohcxx_zenohc_lib zenohc::lib)
	target_link_libraries(zenohcxx_zenohc_lib INTERFACE zenohc::lib)
	add_library(zenohcxx::zenohc::lib ALIAS zenohcxx_zenohc_lib)
endif()

# zenohcxx for zenohc dynamic
if(TARGET zenohc::shared)
	message(STATUS "defined lib target zenohcxx::zenohc::shared for zenohc::shared")
	add_library(zenohcxx_zenohc_shared INTERFACE)
	target_compile_definitions(zenohcxx_zenohc_shared INTERFACE ZENOHCXX_ZENOHC)
	target_include_directories(zenohcxx_zenohc_shared INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
	add_dependencies(zenohcxx_zenohc_lib zenohc::shared)
	target_link_libraries(zenohcxx_zenohc_shared INTERFACE zenohc::shared)
	add_library(zenohcxx::zenohc::shared ALIAS zenohcxx_zenohc_shared)
endif()

# zenohcxx for zenohc static
if(TARGET zenohc::static)
	message(STATUS "defined lib target zenohcxx::zenohc::static for zenohc::static")
	add_library(zenohcxx_zenohc_static INTERFACE)
	target_compile_definitions(zenohcxx_zenohc_static INTERFACE ZENOHCXX_ZENOHC)
	target_include_directories(zenohcxx_zenohc_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
	add_dependencies(zenohcxx_zenohc_static zenohc::static)
	target_link_libraries(zenohcxx_zenohc_static INTERFACE zenohc::static)
	add_library(zenohcxx::zenohc::static ALIAS zenohcxx_zenohc_static)
endif()

#
# Components included only if project is the root project
#
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	add_subdirectory(install)
	add_subdirectory(tests)
	add_subdirectory(examples)
	add_subdirectory(docs)
endif()
