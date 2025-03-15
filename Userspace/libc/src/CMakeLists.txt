file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE SYSDEPS_SOURCES ${SYSDEPS_PATH}/*.c ${SYSDEPS_GENERIC}/*.c ${SYSDEPS_PATH}/*.cpp ${SYSDEPS_GENERIC}/*.cpp)
list(APPEND SRC_FILES ${SYSDEPS_SOURCES})

add_library(libc_obj OBJECT ${SRC_FILES})
set_target_properties(libc_obj PROPERTIES POSITION_INDEPENDENT_CODE 1)

message(STATUS "Adding sysdeps sources: ${SYSDEPS_SOURCES}")

add_library(libc_shared SHARED $<TARGET_OBJECTS:libc_obj>)
set_target_properties(libc_shared PROPERTIES OUTPUT_NAME "c")
target_compile_definitions(libc_shared PRIVATE PROGRAM_VERSION="${PROJECT_VERSION}")

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_COMMIT)
  target_compile_definitions(libc_shared PRIVATE LIBC_GIT_COMMIT="${GIT_COMMIT}")
endif()

target_compile_options(libc_shared PRIVATE -fvisibility=hidden -fPIC)
target_link_options(libc_shared PRIVATE -nostdlib -shared -fPIC -fPIE -e _start -Wl,-soname,libc.so -lgcc)

add_library(libc_static STATIC $<TARGET_OBJECTS:libc_obj>)
set_target_properties(libc_static PROPERTIES OUTPUT_NAME "c")
target_compile_options(libc_static PRIVATE -fvisibility=hidden -fPIC)

install(TARGETS libc_shared libc_static DESTINATION lib)
