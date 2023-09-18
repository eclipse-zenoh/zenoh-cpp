cmake_minimum_required(VERSION 3.16)
project(zenohcxx_examples LANGUAGES C CXX)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

#
# Variants of using Zenoh libraries in CMake project:
#
# 1. Include from subdirectories (useful for development, clone corresponding repos manually):
#    add_subdirectory(../zenoh-pico)
#    add_subdirectory(../zenoh-c)
#    add_subdirectory(../zenoh-cpp)
#
# 2. Use find_package (if corresponding library installed):
#    find_package(zenohpico)
#    find_package(zenohc)
#    find_package(zenohcxx)
#
# 3. Use FetchContent:
include(FetchContent)
FetchContent_declare(zenohpico_backend GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-pico")
FetchContent_MakeAvailable(zenohpico_backend)
FetchContent_declare(zenohc_backend GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-c")
FetchContent_MakeAvailable(zenohc_backend)
FetchContent_declare(cpp_wrapper GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-cpp" GIT_TAG main)
FetchContent_MakeAvailable(cpp_wrapper)

#
# Same source can be used for both zenohpico and zenohc
#
# The correct preprocessor selector (ZENOHCXX_ZENOHPICO or ZENOHCXX_ZENOHC) and 
# the correct backend C library dependency (zenohpico or zenohc::lib or zenohc::static) 
# are selected automatically by corresponding zenohcxx::* target
#
add_executable(zp_simple z_simple.cxx)
target_link_libraries(zp_simple PRIVATE zenohcxx::zenohpico)
set_property(TARGET zp_simple PROPERTY LANGUAGE CXX)
set_property(TARGET zp_simple PROPERTY CXX_STANDARD 17)

add_executable(zc_dynlib_simple z_simple.cxx)
target_link_libraries(zc_dynlib_simple PRIVATE zenohcxx::zenohc::lib)
set_property(TARGET zc_dynlib_simple PROPERTY LANGUAGE CXX)
set_property(TARGET zc_dynlib_simple PROPERTY CXX_STANDARD 17)

add_executable(zc_static_simple z_simple.cxx)
target_link_libraries(zc_static_simple PRIVATE zenohcxx::zenohc::static)
set_property(TARGET zc_static_simple PROPERTY LANGUAGE CXX)
set_property(TARGET zc_static_simple PROPERTY CXX_STANDARD 17)