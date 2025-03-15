cmake_minimum_required(VERSION 3.10)
project(FennixMathLibrary)

set(SOURCES libm.c)

add_library(m STATIC ${SOURCES})
add_library(m_shared SHARED ${SOURCES})

target_link_options(m_shared PRIVATE -nostdlib)
set_target_properties(m_shared PROPERTIES OUTPUT_NAME "m")

install(TARGETS m m_shared
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include)
