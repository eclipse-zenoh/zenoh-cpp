if(CPACK_GENERATOR MATCHES "DEB")
    # DEB package
    if(NOT DEBARCH)
        execute_process(
        COMMAND dpkg --print-architecture
        OUTPUT_VARIABLE DEBARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
    message(STATUS "Configure DEB packaging for Linux ${DEBARCH}")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ZettaScale Zenoh Team, <zenoh@zettascale.tech>")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${DEBARCH})
    set(CPACK_DEB_COMPONENT_INSTALL OFF)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_NAME lib${CPACK_PACKAGE_NAME}-dev)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
endif()

if(CPACK_GENERATOR MATCHES "RPM")
  # RPM package
  # rpmbuild should be installed
  # apt install rpm
  if(NOT RPMARCH)
    set(RPMARCH ${CMAKE_SYSTEM_PROCESSOR})
  endif()
  message(STATUS "Configure RPM packaging for Linux ${RPMARCH}")
  set(CPACK_RPM_PACKAGE_ARCHITECTURE ${RPMARCH})
  set(CPACK_RPM_COMPONENT_INSTALL OFF)
  set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
  set(CPACK_RPM_PACKAGE_NAME ${CPACK_PACKAGE_NAME}-dev)
endif()
