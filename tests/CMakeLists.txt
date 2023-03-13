message(STATUS "zenoh-cxx tests")

add_custom_target(tests)
add_custom_target(tests_zenohc)
add_custom_target(tests_zenohpico)
add_dependencies(tests tests_zenohc)
add_dependencies(tests tests_zenohpico)

function(add_test_instance file mode lib)
	get_filename_component(filename ${file} NAME_WE)
	set(target ${mode}_${filename})
	add_executable(${target} EXCLUDE_FROM_ALL ${file})
	add_dependencies(tests_${mode} ${target})
	add_dependencies(${target} ${lib})
	target_link_libraries(${target} PUBLIC ${lib})
	set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
	set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
	add_platform_libraries(${target})
	add_test(NAME "test_${target}" COMMAND ${target})
endfunction()

# file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
set(files 
	structs.cpp
	# keyexpr.cpp
)
list(TRANSFORM files PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

foreach(file ${files})
    add_test_instance(${file} zenohc zenohcxx::zenohc::lib)
    add_test_instance(${file} zenohpico zenohcxx::zenohpico)
endforeach()
