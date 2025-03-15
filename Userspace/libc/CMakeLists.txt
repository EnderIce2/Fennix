cmake_minimum_required(VERSION 3.10)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(FennixCLibrary VERSION 1.0.0)

if(NOT DEFINED ENV{WORKSPACE_DIR})
  set(STANDALONE_BUILD ON)
  message(STATUS "Compiling standalone")
  set(CMAKE_INSTALL_PREFIX "/usr")
else()
  set(STANDALONE_BUILD OFF)
  set(CMAKE_INSTALL_PREFIX "$ENV{WORKSPACE_DIR}/out")
  message(STATUS "Compiling within workspace")
  try_compile(
    WORKSPACE_TEST
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/workspace_test.c
    CMAKE_FLAGS "-DCMAKE_C_COMPILER=$ENV{CC} -DCMAKE_CXX_COMPILER=$ENV{CXX} -DCMAKE_ASM_COMPILER=$ENV{AS} -DCMAKE_AR=$ENV{AR} -DCMAKE_LINKER=$ENV{LD}"
    OUTPUT_VARIABLE OUTPUT)
  if(NOT WORKSPACE_TEST)
    message(FATAL_ERROR "Workspace test failed: ${OUTPUT}")
  else()
    message(STATUS "Workspace test passed")
  endif()
endif()

if(NOT DEFINED TARGET_OS)
  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(TARGET_OS "linux")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Fennix")
    set(TARGET_OS "fennix")
  else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
  endif()
endif()

if(NOT DEFINED TARGET_ARCH)
  set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

message(STATUS "Building for ${TARGET_OS}/${TARGET_ARCH}")

if(DEFINED ENV{CC})
  set(CMAKE_C_COMPILER "$ENV{CC}")
endif()
if(DEFINED ENV{CXX})
  set(CMAKE_CXX_COMPILER "$ENV{CXX}")
endif()
if(DEFINED ENV{AS})
  set(CMAKE_ASM_COMPILER "$ENV{AS}")
endif()
if(DEFINED ENV{AR})
  set(CMAKE_AR "$ENV{AR}")
endif()
if(DEFINED ENV{LD})
  set(CMAKE_LINKER "$ENV{LD}")
endif()

if(DEFINED ENV{DEBUG} AND "$ENV{DEBUG}" STREQUAL "1")
  set(CMAKE_C_FLAGS "-ggdb3 -O0 -DDEBUG")
else()
  set(CMAKE_C_FLAGS "-O2")
endif()

set(SYSDEPS_DIR ${CMAKE_SOURCE_DIR}/sysdeps)
set(ABIS_DIR ${CMAKE_SOURCE_DIR}/abis)

set(SYSDEPS_GENERIC ${SYSDEPS_DIR}/${TARGET_OS}/generic)
set(SYSDEPS_PATH ${SYSDEPS_DIR}/${TARGET_OS}/${TARGET_ARCH})
set(ABIS_GENERIC ${ABIS_DIR}/${TARGET_OS}/generic)
set(ABIS_PATH ${ABIS_DIR}/${TARGET_OS}/${TARGET_ARCH})

if(NOT EXISTS ${SYSDEPS_PATH} AND NOT EXISTS ${SYSDEPS_GENERIC})
  message(FATAL_ERROR "Missing sysdeps for ${TARGET_OS}: ${SYSDEPS_PATH} or ${SYSDEPS_GENERIC}")
endif()

if(NOT EXISTS ${ABIS_PATH} AND NOT EXISTS ${ABIS_GENERIC})
  message(FATAL_ERROR "Missing abis for ${TARGET_OS}: ${ABIS_PATH} or ${ABIS_GENERIC}")
endif()

message(STATUS "Using sysdeps from: ${SYSDEPS_GENERIC} and ${SYSDEPS_PATH}")
message(STATUS "Using abis from: ${ABIS_GENERIC} and ${ABIS_PATH}")

include_directories(${ABIS_GENERIC} ${ABIS_PATH})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(runtime)
add_subdirectory(interpreter)
add_subdirectory(src)
add_subdirectory(libs)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*")

install(DIRECTORY ${ABIS_GENERIC}/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/
  FILES_MATCHING
  PATTERN "*")

install(DIRECTORY ${ABIS_PATH}/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/
  FILES_MATCHING
  PATTERN "*")
