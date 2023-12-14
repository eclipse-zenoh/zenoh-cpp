cmake_minimum_required(VERSION 3.16)
project(zenoh_foo)

find_package(zenohc REQUIRED)
find_package(zenohcxx REQUIRED)

add_executable(foo main.cpp foo.cpp)
target_link_libraries(foo zenohcxx::zenohc::lib)
set_property(TARGET foo PROPERTY CXX_STANDARD 17)
