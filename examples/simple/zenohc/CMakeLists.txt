cmake_minimum_required(VERSION 3.16)
project(zenohcxx_examples LANGUAGES C CXX)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(zenohc REQUIRED)
find_package(zenohcxx REQUIRED)

add_executable(zc_simple zc_simple.cxx)
target_link_libraries(zc_simple PRIVATE zenohcxx::zenohc)
set_property(TARGET zc_simple PROPERTY LANGUAGE CXX)
set_property(TARGET zc_simple PROPERTY CXX_STANDARD 17)