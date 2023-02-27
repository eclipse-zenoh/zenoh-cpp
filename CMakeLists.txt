cmake_minimum_required(VERSION 3.16)
project(
	zenohcpp
	VERSION 0.7.0.1
	DESCRIPTION "C++ bindings for Zenoh"
	HOMEPAGE_URL "https://github.com/ZettaScaleLabs/zenoh-cpp"
	LANGUAGES C CXX
)
include(cmake/helpers.cmake)
set_default_build_type(Release)

add_library(zenohcpp INTERFACE)
add_dependencies(zenohcpp zenohc::lib)
target_include_directories(zenohcpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

#
# Components included only if project is the root project
#
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	configure_include_project(ZENOHCPP_ZENOHC zenohc zenohc::lib "../zenoh-c" zenohc "https://github.com/eclipse-zenoh/zenoh-c")
	# add_subdirectory(install)
	# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${cargo_binary_dir}/tests)
	# add_subdirectory(tests)
	# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${cargo_binary_dir}/examples)
	add_subdirectory(examples)
endif()

# function(add_libraries target)
# 	target_link_libraries(${target} PUBLIC zenohc::lib)
# 	if(APPLE)
# 		find_library(FFoundation Foundation)
# 		find_library(FSecurity Security)
# 		target_link_libraries(${target} PUBLIC ${FFoundation} ${FSecurity})
# 	elseif(UNIX)
# 		target_link_libraries(${target} PUBLIC rt pthread m dl)
# 	elseif(WIN32)
# 		target_link_libraries(${target} PUBLIC ws2_32 crypt32 secur32 bcrypt ncrypt userenv ntdll iphlpapi runtimeobject)
# 	endif()
# endfunction()

# function(add_example file suffix is_cpp)
# 	get_filename_component(filename ${file} NAME_WE)
# 	set(target ${filename}_${suffix})
# 	add_executable(${target} EXCLUDE_FROM_ALL ${file})
# 	add_dependencies(examples ${target})
# 	if (${is_cpp})
# 		target_link_libraries(${target} PUBLIC zenohcpp)
# 		set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
# 		set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
# 	else()
# 		set_property(TARGET ${target} PROPERTY LANGUAGE C)
# 		set_property(TARGET ${target} PROPERTY C_STANDARD 11)
# 	endif()
# 	set_property(TARGET ${target} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/examples")
# 	add_libraries(${target})
# endfunction()

# function(add_examples glob suffix is_cpp)
# 	file(GLOB files ${glob})
# 	foreach(file ${files})
# 		add_example(${file} ${suffix} ${is_cpp})
# 	endforeach()
# endfunction()

# function(add_test_instance file)
# 	get_filename_component(filename ${file} NAME_WE)
# 	set(target ${filename})
# 	add_executable(${target} EXCLUDE_FROM_ALL ${file})
# 	add_dependencies(tests ${target})
# 	target_link_libraries(${target} PUBLIC zenohcpp)
# 	set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
# 	set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
# 	set_property(TARGET ${target} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
# 	add_libraries(${target})
# 	add_test(NAME "test_${filename}" COMMAND ${target})
# endfunction()

# function(add_tests glob)
# 	file(GLOB files ${glob})
# 	foreach(file ${files})
# 		add_test_instance(${file})
# 	endforeach()
# endfunction()

# if(APPLE OR UNIX OR WIN32)
# 	enable_testing()
# 	add_custom_target(tests)
# 	add_tests("${CMAKE_SOURCE_DIR}/tests/*.cpp")

# 	add_custom_target(examples)
# 	# C examples compiled with C compiler
# 	add_examples("${CMAKE_SOURCE_DIR}/examples/c/*.c" "c" FALSE)
# 	# C examples compiled with C++ compiler
# 	add_examples("${CMAKE_SOURCE_DIR}/examples/c/*.c" "c_cpp" TRUE)
# 	# C++ examples compiled with C++ compiler
# 	add_examples("${CMAKE_SOURCE_DIR}/examples/cpp/*.cpp" "cpp" TRUE)
# else()
# 	message(WARNING "You platform doesn't seem to support building the examples or tests.")
# endif()

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
