if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    # Settings when 'examples' is the root projet
    cmake_minimum_required(VERSION 3.20)
    project(zenohcpp_examples LANGUAGES C CXX)
    include(../cmake/helpers.cmake)
    set_default_build_type(Release)
    configure_include_project(ZENOHCPP_ZENOHC zenohc zenohc::lib "../../zenoh-c" zenohc "https://github.com/eclipse-zenoh/zenoh-c" "")
    configure_include_project(ZENOHCPP zenohcpp zenohcpp ".." zenohcpp "https://github.com/eclipse-zenoh/zenoh-cpp" "")
    add_custom_target(examples ALL)
else()
    message(STATUS "zenoh-cpp examples")
    add_custom_target(examples)
endif()

function(add_example file mode is_cpp zenohc_lib zenohcpp_lib)
	unset_if_empty(zenohc_lib zenohcpp_lib)
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${mode})
	add_executable(${target} EXCLUDE_FROM_ALL ${file})
	set_target_properties(${target} PROPERTIES 
		OUTPUT_NAME ${filename}
		RUNTIME_OUTPUT_DIRECTORY ${mode})
	add_dependencies(examples ${target})
	add_dependencies(${target} ${zenohc_lib})
    target_link_libraries(${target} PUBLIC ${zenohc_lib})
	if (${is_cpp})
		if(DEFINED zenohcpp_lib)
			add_dependencies(${target} ${zenohcpp_lib})
			target_link_libraries(${target} PUBLIC ${zenohcpp_lib})
		endif()
		set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
		set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
	else()
		set_property(TARGET ${target} PROPERTY LANGUAGE C)
		set_property(TARGET ${target} PROPERTY C_STANDARD 11)
	endif()
	add_platform_libraries(${target})
	add_platform_target_compile_definitions(${target} WIN32 PRIVATE ZENOH_WINDOWS)
	add_platform_target_compile_definitions(${target} APPLE PRIVATE ZENOH_MACOS)
	add_platform_target_compile_definitions(${target} UNIX PRIVATE ZENOH_LINUX)
endfunction()

function(add_examples glob mode is_cpp zenohc_lib zenohcpp_lib)
	file(GLOB files ${glob})
	foreach(file ${files})
		add_example(${file} ${mode} ${is_cpp} ${zenohc_lib} "${zenohcpp_lib}")
	endforeach()
endfunction()

# C examples compiled with C compiler and zenohc
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/c/*.c" "c" FALSE zenohc::lib "")
# C examples compiled with C++ compiler and zenohc
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/c/*.c" "cpp_c" TRUE zenohc::lib "")

# C++ examples compiled with C++ compiler with zenohc library
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.cpp" "cpp_zenohc" TRUE zenohc::lib zenohcpp::zenohc)
# C++ examples compiled with C++ compiler with zenohpico library
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.cpp" "cpp_zenohpico" TRUE zenohpico zenohcpp::zenohpico)