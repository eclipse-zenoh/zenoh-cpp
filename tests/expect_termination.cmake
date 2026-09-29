# Copyright (c) 2026 ZettaScale Technology.
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
execute_process(COMMAND "${TEST_EXECUTABLE}" "${CASE}" RESULT_VARIABLE result)
if("${result}" STREQUAL "125")
    message("SKIP: required feature disabled")
elseif(NOT "${result}" STREQUAL "77")
    message(FATAL_ERROR "Expected std::terminate (exit 77), got: ${result}")
endif()
