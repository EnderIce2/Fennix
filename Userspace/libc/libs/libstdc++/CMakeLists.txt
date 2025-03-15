cmake_minimum_required(VERSION 3.10)
project(FennixStandardC++Library)

set(SOURCES libstdc++.cpp)

add_library(stdc++ STATIC ${SOURCES})
add_library(stdc++_shared SHARED ${SOURCES})

target_link_options(stdc++_shared PRIVATE -nostdlib)
set_target_properties(stdc++_shared PROPERTIES OUTPUT_NAME "stdc++")

install(TARGETS stdc++ stdc++_shared
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include)
