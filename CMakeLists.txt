cmake_minimum_required(VERSION 3.10)
include(ExternalProject)
project(
	zenohcpp
	VERSION 0.6.0.1
	DESCRIPTION "C++ bindings for Zenoh"
	HOMEPAGE_URL "https://github.com/ZettaScaleLabs/zenoh-cpp"
	LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
	set(default_build_type "RelWithDebInfo")
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)

	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/target/debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/target/release")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/target/release")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/target/release")
else()
	message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

set(zenohc_prefix "zenohc")
set(zenohc_download_path "${CMAKE_CURRENT_BINARY_DIR}/${zenohc_prefix}/download")
set(zenohc_install_path "${CMAKE_CURRENT_BINARY_DIR}/${zenohc_prefix}/install")

message(STATUS "zenoh-c build-time download path: ${zenohc_download_path}")
message(STATUS "zenoh-c build-time install path: ${zenohc_install_path}")

ExternalProject_add(
    zenohc_project
	CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${zenohc_install_path}
	PREFIX ${zenohc_prefix}
    SOURCE_DIR "../../zenoh-c"
    # SOURCE_DIR ${zenohc_download_path}
    # GIT_REPOSITORY https://github.com/milyin/zenoh-c
)

set(libzenohc_include "${zenohc_install_path}/include/zenohc")
file(MAKE_DIRECTORY ${libzenohc_include}) # INTERFACE_INCLUDE_DIRECTORIES requires that path should exist
message(STATUS "zenoh-c headers: ${libzenohc_include}")

if(APPLE)
	set(libzenohc "${zenohc_install_path}/lib/libzenohc.dylib")
	set(libzenohc_static "${zenohc_install_path}/lib/libzenohc.a")
elseif(UNIX)
	set(libzenohc "${zenohc_install_path}/lib/libzenohc.so")
	set(libzenohc_static "${zenohc_install_path}/lib/libzenohc.a")
elseif(WIN32)
	set(libzenohc "${zenohc_install_path}/lib/zenohc.dll")
	set(libzenohc_static "${zenohc_install_path}/lib/zenohc.lib")
endif()
message(STATUS "zenoh-c static library path : ${libzenohc_static}")
message(STATUS "zenoh-c dynamic library path : ${libzenohc}")

add_library(zenohc SHARED IMPORTED)
add_dependencies(zenohc zenohc_project)
set_property(TARGET zenohc PROPERTY IMPORTED_NO_SONAME TRUE) # prevent linker from inserting into executable full path to the library
set_property(TARGET zenohc PROPERTY IMPORTED_LOCATION "${libzenohc}")
set_property(TARGET zenohc PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${libzenohc_include}")

add_library(zenohc_static STATIC IMPORTED)
add_dependencies(zenohc_static zenohc_project)
set_property(TARGET zenohc_static PROPERTY IMPORTED_LOCATION "${libzenohc_static}")
set_property(TARGET zenohc_static PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${libzenohc_include}")

add_library(zenohcpp INTERFACE)
add_dependencies(zenohcpp zenohc_project)
target_include_directories(zenohcpp INTERFACE "${CMAKE_SOURCE_DIR}/include")

function(add_example file suffix is_cpp)
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${suffix})
	add_executable(${target} EXCLUDE_FROM_ALL ${file})
	add_dependencies(examples ${target})
	if (${is_cpp})
		target_link_libraries(${target} PUBLIC zenohcpp)
		set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
		set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
	else()
		set_property(TARGET ${target} PROPERTY LANGUAGE C)
		set_property(TARGET ${target} PROPERTY C_STANDARD 11)
	endif()
	set_property(TARGET ${target} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples")
	# target_link_libraries(${target} PUBLIC zenohc_static)
	target_link_libraries(${target} PUBLIC zenohc)
	if(APPLE)
		find_library(FFoundation Foundation)
		find_library(FSecurity Security)
		target_link_libraries(${target} PUBLIC ${FFoundation} ${FSecurity})
	elseif(UNIX)
		target_link_libraries(${target} PUBLIC rt pthread m dl)
	elseif(WIN32)
		target_link_libraries(${target} PUBLIC ws2_32 crypt32 secur32 bcrypt ncrypt userenv ntdll iphlpapi runtimeobject)
	endif()
endfunction()

function(add_examples glob suffix is_cpp)
	file(GLOB files ${glob})
	foreach(file ${files})
		add_example(${file} ${suffix} ${is_cpp})
	endforeach()
endfunction()

if(APPLE OR UNIX OR WIN32)
	add_custom_target(examples)
	# C examples compiled with C compiler
	add_examples("${CMAKE_SOURCE_DIR}/examples/c/*.c" "c" FALSE)
	# C examples compiled with C++ compiler
	add_examples("${CMAKE_SOURCE_DIR}/examples/c/*.c" "c_cpp" TRUE)
	# C++ examples compiled with C++ compiler
	add_examples("${CMAKE_SOURCE_DIR}/examples/cpp/*.cpp" "cpp" TRUE)
else()
	message(WARNING "You platform doesn't seem to support building the examples or tests.")
endif()
