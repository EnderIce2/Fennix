cmake_minimum_required(VERSION 3.10)
project(FennixRealTimeLibrary)

set(SOURCES librt.c)

add_library(rt STATIC ${SOURCES})
add_library(rt_shared SHARED ${SOURCES})

target_link_options(rt_shared PRIVATE -nostdlib)
set_target_properties(rt_shared PROPERTIES OUTPUT_NAME "rt")

install(TARGETS rt rt_shared
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include)
