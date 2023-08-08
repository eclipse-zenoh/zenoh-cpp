if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    cmake_minimum_required (VERSION 3.16)
    project(zenohcxx_docs)
else()
    message(STATUS "zenoh-cxx docs")
    include(../cmake/helpers.cmake)
endif()

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
    message(STATUS "Doxygen not found, skipping docs")
    return()
endif()

find_package(Sphinx)
if(NOT SPHINX_FOUND)
    message(STATUS "Sphinx not found, skipping docs")
    return()
endif()

#
# Doxygen target
#

set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)

message(STATUS "Doxygen output dir: ${DOXYGEN_OUTPUT_DIR}")

# Input files are configured in "Doxyfile" with relative paths:
# INPUT = ../include/zenohcxx/base.hxx ../include/zenohcxx/api.hxx
# So on adding/changing header files, they should be added both here and in "Doxyfile"
#
# CMake script will copy the include files and Doxyfile to the build directory
#
# We do not use here "configure_file" from "Doxygen.in" to "Doxygen" scheme,
# because doxygen should work being executed from the source directory.
# This is the case for readthedocs.io where doxygen is executed by "conf.py",
# without CMakeLists.txt.
set(DOXYGEN_INPUT_FILES 
${CMAKE_CURRENT_SOURCE_DIR}/../include/zenohcxx/base.hxx 
${CMAKE_CURRENT_SOURCE_DIR}/../include/zenohcxx/api.hxx
${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
)
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}/../include)

add_custom_target(Doxygen)

add_custom_command(TARGET Doxygen PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_SOURCE_DIR}/../include
                   ${DOXYGEN_OUTPUT_DIR}/../include)

add_custom_command(TARGET Doxygen PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
                   ${DOXYGEN_OUTPUT_DIR}/Doxyfile)

add_custom_command(TARGET Doxygen POST_BUILD
                   BYPRODUCTS ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${DOXYGEN_INPUT_FILES}
                   WORKING_DIRECTORY ${DOXYGEN_OUTPUT_DIR}
                   COMMAND ${DOXYGEN_EXECUTABLE}
                   COMMENT "Generating docs")

#
# Sphinx target
#

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)

file (GLOB_RECURSE RST_FILES *.rst)
add_custom_target(Sphinx
                  DEPENDS conf.py ${RST_FILES} ${DOXYGEN_INDEX_FILE}
                  COMMAND ${SPHINX_EXECUTABLE} -b html
                  # Tell Breathe where to find the Doxygen output
                  -Dbreathe_projects.zenohcpp=${DOXYGEN_OUTPUT_DIR}/xml
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  COMMENT "Generating documentation with Sphinx")

add_custom_target(docs DEPENDS Sphinx)

add_custom_command(TARGET docs POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
                   "See read-the-docs html in ${SPHINX_BUILD}/index.html")