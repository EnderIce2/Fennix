cmake_minimum_required(VERSION 3.10)
project(FennixRuntime)

if(NOT DEFINED TARGET_OS)
  message(FATAL_ERROR "TARGET_OS is not set")
endif()

if(NOT DEFINED TARGET_ARCH)
  message(FATAL_ERROR "TARGET_ARCH is not set")
endif()

set(RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_OS}/${TARGET_ARCH}")

if(NOT EXISTS "${RUNTIME_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "No runtime support for ${TARGET_OS}/${TARGET_ARCH}")
endif()

add_subdirectory(${RUNTIME_DIR})
