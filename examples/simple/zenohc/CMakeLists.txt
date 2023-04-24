cmake_minimum_required(VERSION 3.20)
project(zenohcxx_examples LANGUAGES C CXX)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

FetchContent_declare(c_backend GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-c")
FetchContent_MakeAvailable(c_backend)
FetchContent_declare(cpp_wrapper GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-cpp" GIT_TAG main)
FetchContent_MakeAvailable(cpp_wrapper)

add_executable(zc_simple zc_simple.cxx)
target_link_libraries(zc_simple PRIVATE zenohcxx::zenohc::lib)
