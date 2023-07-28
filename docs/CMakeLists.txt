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

# Find all the public headers

set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# file(GLOB_RECURSE HEADERS ${DOXYGEN_INPUT_DIR}/*.hxx)
set(DOXYGEN_INPUT_FILES 
${DOXYGEN_INPUT_DIR}/zenohcxx/base.hxx 
${DOXYGEN_INPUT_DIR}/zenohcxx/api.hxx
)

list(JOIN DOXYGEN_INPUT_FILES " " DOXYGEN_INPUT_FILES_CFG)
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

#This will be the main output of our command
set(DOXYGEN_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/html/index.html)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${DOXYGEN_INPUT_FILES}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating docs")

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})


set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)


file (GLOB_RECURSE RST_FILES *.rst)
add_custom_target(Sphinx ALL
                  DEPENDS conf.py ${RST_FILES} ${DOXYGEN_INDEX_FILE}
                  COMMAND ${SPHINX_EXECUTABLE} -b html
                  # Tell Breathe where to find the Doxygen output
                  -Dbreathe_projects.zenohcpp=${DOXYGEN_OUTPUT_DIR}/xml
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  COMMENT "Generating documentation with Sphinx")

add_custom_target(docs ALL DEPENDS Sphinx)