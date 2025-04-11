message(STATUS "zenoh-cxx tests")

add_custom_target(tests)
if(ZENOHCXX_ZENOHC)
	add_custom_target(tests_zenohc)
	add_dependencies(tests tests_zenohc)
endif()

if(ZENOHCXX_ZENOHPICO)
	add_custom_target(tests_zenohpico)
	add_dependencies(tests tests_zenohpico)
endif()


configure_file(${PROJECT_SOURCE_DIR}/tests/run_with_router.sh ${CMAKE_CURRENT_BINARY_DIR}/run_with_router.sh COPYONLY)

function(add_test_instance file mode lib option)
	# Exclude SHM tests if SHM feature is disabled
	if(NOT(ZENOHC_BUILD_WITH_SHARED_MEMORY AND (ZENOHC_BUILD_WITH_UNSTABLE_API)))
		if(${file} MATCHES "^.*shm.*$")
			return()
		endif()
	endif()
	# Exclude advanced pub-sub test if unstable features are disabled
	if(NOT(ZENOHC_BUILD_WITH_UNSTABLE_API))
		if(${file} MATCHES "^.*advanced_pub_sub.*$")
			return()
		endif()
	endif()

	set(zenoh_router_branch "main")
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${mode})
	add_executable(${target} EXCLUDE_FROM_ALL ${file})
	add_dependencies(tests_${mode} ${target})
	add_dependencies(${target} ${lib})
	target_link_libraries(${target} PUBLIC ${lib})
	set_property(TARGET ${target} PROPERTY LANGUAGE CXX)
	set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
	if (option STREQUAL "Router")
		add_test(NAME "test_${target}" COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/run_with_router.sh $<TARGET_FILE:${target}> ${zenoh_router_branch})
	else()
		add_test(NAME "test_${target}" COMMAND ${target})
	endif()
	copy_dlls(${target})
endfunction()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/universal/*.cxx")
foreach(file ${files})
	if(ZENOHCXX_ZENOHC) 
    	add_test_instance(${file} zenohc zenohcxx::zenohc "")
	endif()
	if(ZENOHCXX_ZENOHPICO) 
    	add_test_instance(${file} zenohpico zenohcxx::zenohpico "")
	endif()
endforeach()

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/universal/network/*.cxx")
foreach(file ${files})
	if(ZENOHCXX_ZENOHC) 
    	add_test_instance(${file} zenohc zenohcxx::zenohc "")
	endif()
	if(ZENOHCXX_ZENOHPICO)
		if ((${file} MATCHES "^.*pub_sub.*$") AND NOT((ZENOHPICO_FEATURE_PUBLICATION) AND (ZENOHPICO_FEATURE_SUBSCRIPTION)))
			continue()
		endif()
		if ((${file} MATCHES "^.*queryable_get.*$") AND NOT((ZENOHPICO_FEATURE_QUERY) AND (ZENOHPICO_FEATURE_QUERYABLE)))
			continue()
		endif()
		if ((${file} MATCHES "^.*liveliness.*$") AND NOT(ZENOHPICO_FEATURE_LIVELINESS))
			continue()
		endif()
    	add_test_instance(${file} zenohpico zenohcxx::zenohpico Router)
	endif()
endforeach()

if(ZENOHCXX_ZENOHC) 
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/zenohc/*.cxx")
	foreach(file ${files})
		add_test_instance(${file} zenohc zenohcxx::zenohc "")
	endforeach()
endif()

if(ZENOHCXX_ZENOHPICO)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/zenohpico/*.cxx")
	foreach(file ${files})
		add_test_instance(${file} zenohpico zenohcxx::zenohpico "")
	endforeach()

	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/zenohpico/network/*.cxx")
	foreach(file ${files})
		if ((${file} MATCHES "^.*batching.*$") AND NOT(ZENOHPICO_FEATURE_BATCHING))
			continue()
		endif()
		add_test_instance(${file} zenohpico zenohcxx::zenohpico Router)
	endforeach()
endif()

function(set_strict_warnings file mode)
	get_filename_component(filename ${file} NAME_WE)
	set(target ${filename}_${mode})
	# Enable all warnings and treat warnings as errors
	# to check that library can be built in maximally strict mode
	if(MSVC)
		target_compile_options(${target} PRIVATE 
			/W4 # show all warning
			/WX # consider all warning as errors
			#/wd4996 # allow deprecated
		)
	else()
		target_compile_options(${target} PRIVATE 
			-Wall 
			-Wextra 
			-Wpedantic
			-Wold-style-cast
			#-Wno-deprecated-declarations
			-Werror)
	endif()
endfunction()

if(ZENOHCXX_ZENOHC)
	add_test_instance(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.cxx zenohc zenohcxx::zenohc "")
	set_strict_warnings(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.hxx zenohc)
endif()

if(ZENOHCXX_ZENOHPICO)
	add_test_instance(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.cxx zenohpico zenohcxx::zenohpico "")
	set_strict_warnings(${CMAKE_CURRENT_SOURCE_DIR}/build/warnings.hxx zenohpico)
endif()
