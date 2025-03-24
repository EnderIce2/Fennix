cmake_minimum_required(VERSION 3.10)
project(FennixRealTimeLibrary)

set(SOURCES libpthread.c)

add_library(pthread STATIC ${SOURCES})
add_library(pthread_shared SHARED ${SOURCES})

target_link_options(pthread_shared PRIVATE -nostdlib)
set_target_properties(pthread_shared PROPERTIES OUTPUT_NAME "pthread")

install(TARGETS pthread pthread_shared
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include)
