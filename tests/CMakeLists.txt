message(STATUS "zenoh-cxx tests")

add_custom_target(tests)
add_custom_target(tests_zenohc)
add_custom_target(tests_zenohpico)
add_dependencies(tests tests_zenohc)
add_dependencies(tests tests_zenohpico)


configure_file(${PROJECT_SOURCE_DIR}/tests/run_with_router.sh ${CMAKE_CURRENT_BINARY_DIR}/run_with_router.sh COPYONLY)

function(add_test_instance file mode lib option)
	set(zenoh_router_branch "dev/1.0.0")
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${mode})
	add_executable(${target} EXCLUDE_FROM_ALL ${file})
	add_dependencies(tests_${mode} ${target})
	add_dependencies(${target} ${lib})
	target_link_libraries(${target} PUBLIC ${lib})
	set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
	set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
	if (option STREQUAL "Router")
		add_test(NAME "test_${target}" COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/run_with_router.sh ${target} ${zenoh_router_branch})
	else()
		add_test(NAME "test_${target}" COMMAND ${target})
	endif()
endfunction()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/universal/*.cxx")
foreach(file ${files})
    add_test_instance(${file} zenohc zenohcxx::zenohc::static "")
    add_test_instance(${file} zenohpico zenohcxx::zenohpico "")
endforeach()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/universal/network/*.cxx")
foreach(file ${files})
    add_test_instance(${file} zenohc zenohcxx::zenohc::static "")
    add_test_instance(${file} zenohpico zenohcxx::zenohpico Router)
endforeach()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/zenohc/*.cxx")
foreach(file ${files})
    add_test_instance(${file} zenohc zenohcxx::zenohc::static "")
endforeach()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/zenohpico/*.cxx")
foreach(file ${files})
    add_test_instance(${file} zenohpico zenohcxx::zenohpico "")
endforeach()

function(set_strict_warnings file mode)
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${mode})
	# Enable all warnings and treat warnings as errors
	# to check that library can be built in maximally strict mode
	if(MSVC)
		target_compile_options(${target} PRIVATE /W4 /WX)
	else()
		target_compile_options(${target} PRIVATE 
			-Wall 
			-Wextra 
			-Wpedantic
			-Wold-style-cast 
			-Werror)
	endif()
endfunction()

add_test_instance(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.cxx zenohc zenohcxx::zenohc::static "")
add_test_instance(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.cxx zenohpico zenohcxx::zenohpico "")
set_strict_warnings(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.hxx zenohc)
set_strict_warnings(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.hxx zenohpico)
