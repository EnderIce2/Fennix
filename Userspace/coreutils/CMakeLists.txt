cmake_minimum_required(VERSION 3.10)
project(FennixCoreUtilities VERSION 1.0.0)

if(NOT DEFINED ENV{WORKSPACE_DIR})
  set(STANDALONE_BUILD ON)
  message(STATUS "Compiling standalone")
else()
  set(STANDALONE_BUILD OFF)
  message(STATUS "Compiling within workspace")
endif()

set(CMAKE_C_COMPILER "$ENV{CC}")
set(CMAKE_CXX_COMPILER "$ENV{CXX}")
set(CMAKE_ASM_COMPILER "$ENV{AS}")
set(CMAKE_AR "$ENV{AR}")
set(CMAKE_LINKER "$ENV{LD}")

if(DEFINED ENV{DEBUG} AND "$ENV{DEBUG}" STREQUAL "1")
  set(CMAKE_C_FLAGS "-ggdb3 -O0 -DDEBUG")
else()
  set(CMAKE_C_FLAGS "-O2")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SINGLE_SOURCE "src/*.c")
foreach(file ${SINGLE_SOURCE})
  get_filename_component(name ${file} NAME_WE)
  add_executable(${name} ${file})
  target_compile_definitions(${name} PRIVATE
    PROGRAM_NAME="${name}"
    PROGRAM_VERSION="${PROJECT_VERSION}"
  )
  install(TARGETS ${name} DESTINATION bin)
endforeach()

file(GLOB_RECURSE GROUP_SOURCES "src/*/*.c")
foreach(file ${GROUP_SOURCES})
  get_filename_component(name ${file} DIRECTORY)
  get_filename_component(name ${name} NAME)
  list(APPEND GROUP_MAP_${name} ${file})
endforeach()

foreach(name IN LISTS GROUP_MAP_)
  add_executable(${name} ${GROUP_MAP_${name}})
  target_compile_definitions(${name} PRIVATE
    PROGRAM_NAME="${name}"
    PROGRAM_VERSION="${PROJECT_VERSION}"
  )
  install(TARGETS ${name} DESTINATION bin)
endforeach()

if(UNIX AND TARGET test)
  install(CODE "execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink 
                test 
                \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/[\"
    )")
endif()
