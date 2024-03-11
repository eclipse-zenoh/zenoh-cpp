cmake_minimum_required(VERSION 3.16)
project(zenohcxx_examples LANGUAGES C CXX)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

FetchContent_declare(c_backend GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-pico" GIT_TAG main)
FetchContent_MakeAvailable(c_backend)
FetchContent_declare(cpp_wrapper GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-cpp" GIT_TAG main)
FetchContent_MakeAvailable(cpp_wrapper)

add_executable(zp_simple zp_simple.cxx)
target_link_libraries(zp_simple PRIVATE zenohcxx::zenohpico)
set_property(TARGET zp_simple PROPERTY LANGUAGE CXX)
set_property(TARGET zp_simple PROPERTY CXX_STANDARD 17)
