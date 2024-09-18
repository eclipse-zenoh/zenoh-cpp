cmake_minimum_required(VERSION 3.16)
project(zenohcxx_examples LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake" ${CMAKE_MODULE_PATH})
include(helpers)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(zenohpico REQUIRED)
find_package(zenohcxx REQUIRED)

add_executable(zp_simple zp_simple.cxx)
target_link_libraries(zp_simple PRIVATE zenohcxx::zenohpico)
set_property(TARGET zp_simple PROPERTY LANGUAGE CXX)
set_property(TARGET zp_simple PROPERTY CXX_STANDARD 17)
copy_dlls(zp_simple)
