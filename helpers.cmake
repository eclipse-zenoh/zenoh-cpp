
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
function(declare_cash_var_for_subproject_path var default_path docstring)
    file(GLOB cmakelists ${default_path}/CMakeLists.txt)
    if(cmakelists STREQUAL "")
        set(default_path "")
    endif()
    declare_cache_var(${var} ${subproject_path} STRING ${docstring})
endfunction()

#
# Create target 'debug' and add function 'debug_print' which prints VARIABLE = value
# when target 'debug' is built. Useful to debug generated expressions.
#
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
# Try to include CMake project for specified target
# with add_subdirectory(project_path) and then find_package(package)
# Skips the step if corresponding parameter is empty
#
# Usage:
# include_target(target project_path package git_url)
#
# Example:
# include_target(zenohc::lib ..\zenoh_c zenohc)
#
# function(include_target target project_path package_name git_url)
#     if(NOT(${project_path} STREQUAL ""))
#         add_subdirectory(${project_path})
#         if(NOT TARGET ${target})
#             message(FATAL_ERROR "Project at '${project_path}' should define target ${target}")
#         endif()
#     endif()

#     # Give priority to install directory when looking for zenoh-c
#     # Useful for development when older zenohc version may be installed in system
#     #
#     # TODO: "if( NOT TARGET" below should be not necessary 
#     # (see https://cmake.org/cmake/help/latest/command/find_package.html, search for "override the order")
#     # but in fact cmake fails without it when zenohc is present both in CMAKE_INSTALL_PREFIX and in /usr/local.
#     # Consider is it still necessary after next bumping up cmake version
# 	find_package(${package_name} ${CMAKE_PROJECT_VERSION} PATHS ${CMAKE_INSTALL_PREFIX} NO_DEFAULT_PATH QUIET)
# 	if(NOT TARGET ${target})
# 		find_package(${package_name} ${CMAKE_PROJECT_VERSION} QUIET)
# 	endif()
# 	if(NOT TARGET ${target})
#         if(NOT (${git_url} STREQUAL ""))
#             message(FATAL_ERROR "Package '${package_name}' not found")
#         endif()
# 	endif()

#     if(NOT (${git_url} STREQUAL ""))
#         FetchContent
#     endif()
# endfunction()
