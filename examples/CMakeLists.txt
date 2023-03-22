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

function(add_example file mode lib is_cpp)
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${mode})
	add_executable(${target} EXCLUDE_FROM_ALL ${file})
	set_target_properties(${target} PROPERTIES 
		OUTPUT_NAME ${filename}
		RUNTIME_OUTPUT_DIRECTORY ${mode})
	add_dependencies(examples ${target})
	add_dependencies(${target} ${lib})
    target_link_libraries(${target} PUBLIC ${lib})
	if (${is_cpp})
		set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
		set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
	else()
		set_property(TARGET ${target} PROPERTY LANGUAGE C)
		set_property(TARGET ${target} PROPERTY C_STANDARD 11)
	endif()
	add_platform_libraries(${target})
endfunction()

function(add_examples glob mode lib is_cpp)
	file(GLOB files ${glob})
	foreach(file ${files})
		add_example(${file} ${mode} ${lib} ${is_cpp})
	endforeach()
endfunction()

# zenohc examples compiled with C compiler
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/c/zenohc/*.c" c_zenohc zenohc::lib FALSE)
# zenohc examples compiled with C++ compiler
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/c/zenohc/*.c" c_zenohc_cxx zenohc::lib FALSE)

# zenohpico examples compiled with C compiler
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/c/zenohpico/*.c" c_zenohpico zenohcxx::zenohpico FALSE)
# zenohpico examples compiled with C++ compiler
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/c/zenohpico/*.c" c_zenohpico_cxx zenohpico FALSE)

# zenohcpp examples compiled with C++ compiler with zenohc
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/cpp/zenohc/*.cxx" cxx_zenohc zenohcxx::zenohc::lib TRUE)
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/cpp/universal/*.cxx" cxx_zenohc zenohcxx::zenohc::lib TRUE)

# zenohcpp examples compiled with C++ compiler with zenohpico
add_examples("${CMAKE_CURRENT_SOURCE_DIR}/cpp/universal/*.cxx" cxx_zenohpico zenohcxx::zenohpico TRUE)
