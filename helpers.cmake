include(FetchContent)
include(CMakeParseArguments)

#
# Show VARIABLE = value on configuration stage
#
function(status_print var)
	message(STATUS "${var} = ${${var}}")
endfunction()

#
# Declare cache variable and print VARIABLE = value on configuration stage
#
function(declare_cache_var var default_value type docstring)
	set(${var} ${default_value} CACHE ${type} ${docstring})
	status_print(${var})
endfunction()

#
# Declare cache variable with path to other CMake project to be included with
# add_subdirectory. Sets variable to emplty value if no cmake project in the
# specified directory
#
function(declare_subproject_path_cache_var var default_path docstring)
    file(GLOB cmakelists ${default_path}/CMakeLists.txt)
    if(cmakelists STREQUAL "")
        set(default_path "")
    endif()
    declare_cache_var(${var} ${default_path} STRING ${docstring})
endfunction()

#
# Create target 'debug' and add function 'debug_print' which prints VARIABLE = value
# when target 'debug' is built. Useful to debug generated expressions.
#`
macro(declare_target_debug debug)
    add_custom_target(${debug})
    function(debug_print var)
        add_custom_command(
            COMMAND ${CMAKE_COMMAND} -E echo ${var} = ${${var}}
            TARGET ${debug}
        )
    endfunction()
endmacro()

#
# Select default build config with support of multi config generators
#
macro(set_default_build_type config_type)
    get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(GENERATOR_IS_MULTI_CONFIG)
        if(NOT DEFINED CMAKE_BUILD_TYPE) # if user passed argument '-DCMAKE_BUILD_TYPE=value', use it
            set(CMAKE_BUILD_TYPE ${config_type})
        endif()
         list(FIND CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} n)
        if(n LESS 0)
            message(FATAL_ERROR "Configuration ${CMAKE_BUILD_TYPE} is not in CMAKE_CONFIGURATION_TYPES")
        else()
            if(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
                set(CMAKE_DEFAULT_BUILD_TYPE ${CMAKE_BUILD_TYPE})
                status_print(CMAKE_DEFAULT_BUILD_TYPE)
            else()
                message(STATUS "Default build type is not supported for generator '${CMAKE_GENERATOR}'")
                message(STATUS "use cmake --build . --config ${config_type}")
            endif()
        endif()
    else()
        if(CMAKE_BUILD_TYPE STREQUAL "")
            set(CMAKE_BUILD_TYPE ${config_type})
        endif()
         status_print(CMAKE_BUILD_TYPE)
    endif()
endmacro()

#
# Unset variable if it have empty string value
#
macro(unset_if_empty vars)
    foreach(var ${vars})
        if(${var} STREQUAL "")
            unset(${var})
        endif()
    endforeach()
endmacro()

# 
# Try to include CMake project for specified target
# with add_subdirectory(project_path), then with find_package(project_name) and then with FetchContent(git_url)
# Skips the step if corresponding parameter is not set or have empty value
# 
# include_project(<project_name> TARGET <target> 
#  [PATH project_path] 
#  [FIND_PACKAGE] 
#  [GIT_URL git_url [GIT_TAG git_tag]]
# )
# 
# Example:
# include_project(zenohc TARGET zenohc::lib PATH "${CMAKE_CURRENT_SOURCE_DIR}..\zenoh_c" FIND_PACKAGE)
#
# Functionality is similar to FetchContent with 'FIND_PACKAGE_ARGS' argument, but this argument was added
# in CMake 3.24 only
#
function(include_project project_name)
    cmake_parse_arguments(PARSE_ARGV 1 "ARG" "FIND_PACKAGE" "TARGET;PATH;GIT_URL;GIT_TAG" "")
    unset_if_empty(ARG_PATH ARG_TARGET ARG_GIT_URL)
    if(NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Non-empty TARGET parameter is required")
    endif()

    if(DEFINED ARG_PATH)
        add_subdirectory(${ARG_PATH} ${project_name})
        if(NOT TARGET ${ARG_TARGET})
            message(FATAL_ERROR "Project at '${ARG_PATH}' should define target ${ARG_TARGET}")
        endif()
    endif()

    if(DEFINED FIND_PACKAGE)
        # Give priority to install directory
        # Useful for development when older version of the project version may be installed in system
        #
        # TODO: "if( NOT TARGET" below should be not necessary 
        # (see https://cmake.org/cmake/help/latest/command/find_package.html, search for "override the order")
        # but in fact cmake fails without it when zenohc is present both in CMAKE_INSTALL_PREFIX and in /usr/local.
        # Consider is it still necessary after next bumping up cmake version
        find_package(${project_name} PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH QUIET)
        if(NOT TARGET ${ARG_TARGET})
            find_package(${project_name} QUIET)
        endif()
        if(NOT TARGET ${ARG_TARGET})
            if(NOT ARG_GIT_URL)
                message(FATAL_ERROR "Package '${ARG_PACKAGE}' not found")
            endif()
        endif()
    endif()

    if(DEFINED ARG_GIT_URL)
        if(DEFINED ARG_GIT_TAG)
            FetchContent_Declare(${project_name}
                GIT_REPOSITORY ${ARG_GIT_URL}
                GIT_TAG ${ARG_GIT_TAG}
            )
        else()
            FetchContent_Declare(${project_name}
                GIT_REPOSITORY ${ARG_GIT_URL}
            )
        endif()
        FetchContent_MakeAvailable(${project_name})
    endif()
endfunction()
