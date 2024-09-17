add_custom_target(examples)

if(ZENOHCXX_ZENOHC)
	add_custom_target(examples_zenohc)
	add_dependencies(examples examples_zenohc)
endif()

if(ZENOHCXX_ZENOHPICO)
	add_custom_target(examples_zenohpico)
	add_dependencies(examples examples_zenohpico)
endif()

function(add_example file mode lib)
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${mode})
	add_executable(${target} EXCLUDE_FROM_ALL ${file})
	set_target_properties(${target} PROPERTIES 
		OUTPUT_NAME ${filename}
		RUNTIME_OUTPUT_DIRECTORY ${mode})
	add_dependencies(examples_${mode} ${target})
	add_dependencies(${target} ${lib})
    target_link_libraries(${target} PUBLIC ${lib})
	set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
	set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
	copy_dlls(${target})
endfunction()

function(add_examples glob mode lib)
	file(GLOB files ${glob})
	foreach(file ${files})
		# Exclude SHM examples if SHM feature is disabled
		if(NOT(ZENOHC_BUILD_WITH_SHARED_MEMORY AND (ZENOHC_BUILD_WITH_UNSTABLE_API)))
			if(${file} MATCHES "^.*_shm.*$")
				continue()
			endif()
		endif()

		if(("${mode}" STREQUAL "zenohc") AND (NOT(ZENOHC_BUILD_WITH_UNSTABLE_API)))
			if(${file} MATCHES ".*liveliness.*$")
				continue()
			endif()
		endif()
		add_example(${file} ${mode} ${lib})
	endforeach()
endfunction()

if(ZENOHCXX_ZENOHC)
	# zenohcxx examples compiled with C++ compiler with zenohc
	add_examples("${CMAKE_CURRENT_SOURCE_DIR}/zenohc/*.cxx" zenohc zenohcxx::zenohc)
	add_examples("${CMAKE_CURRENT_SOURCE_DIR}/universal/*.cxx" zenohc zenohcxx::zenohc)
endif()
if(ZENOHCXX_ZENOHPICO)
	# zenohcxx examples compiled with C++ compiler with zenohpico
	add_examples("${CMAKE_CURRENT_SOURCE_DIR}/universal/*.cxx" zenohpico zenohcxx::zenohpico)
endif()

# check compilation of simple examples
#add_examples("${CMAKE_CURRENT_SOURCE_DIR}/simple/universal/z_simple.cxx" zenohpico zenohcxx::zenohpico)
#add_examples("${CMAKE_CURRENT_SOURCE_DIR}/simple/universal/z_simple.cxx" zenohc zenohcxx::zenohc::lib)
#add_examples("${CMAKE_CURRENT_SOURCE_DIR}/simple/zenohpico/zp_simple.cxx" zenohpico zenohcxx::zenohpico)
#add_examples("${CMAKE_CURRENT_SOURCE_DIR}/simple/zenohc/zc_simple.cxx" zenohc zenohcxx::zenohc::lib)
