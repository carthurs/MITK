project(mbilog)

message("Start of Utilities/mbilog/CMakeLists.txt")

option(MBILOG_ENABLE_DEBUG_MESSAGES "Enable extra debug log output" OFF)
mark_as_advanced(MBILOG_ENABLE_DEBUG_MESSAGES)

if(NOT (${CMAKE_BUILD_TYPE} STREQUAL "Release"))
  message("Detected Debug build config for MITK, going to enable MBILOG_ENABLE_DEBUG_MESSAGES")
  set(MBILOG_ENABLE_DEBUG_MESSAGES TRUE)
endif()

if(MBILOG_ENABLE_DEBUG_MESSAGES)
  set(_define_enable_debug "#define MBILOG_ENABLE_DEBUG")
endif(MBILOG_ENABLE_DEBUG_MESSAGES)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mbilogConfig.cmake.in"
"${CMAKE_CURRENT_BINARY_DIR}/mbilogConfig.cmake" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mbilogConfig.h.in"
"${CMAKE_CURRENT_BINARY_DIR}/mbilogConfig.h" @ONLY)

mitk_create_module(
  NO_INIT
  WARNINGS_AS_ERRORS
)

message("End of Utilities/mbilog/CMakeLists.txt")