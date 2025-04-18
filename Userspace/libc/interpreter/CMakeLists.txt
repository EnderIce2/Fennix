file(GLOB_RECURSE SYSDEPS_SOURCES ${SYSDEPS_PATH}/*.c ${SYSDEPS_GENERIC}/*.c)
file(GLOB_RECURSE INTERPRETER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(APPEND INTERPRETER_FILES ${SYSDEPS_SOURCES})

add_executable(ld.so ${INTERPRETER_FILES})

execute_process(COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if(GIT_COMMIT)
  add_compile_definitions(LIBC_GIT_COMMIT="${GIT_COMMIT}")
endif()

add_compile_definitions(FENNIX_DYNAMIC_LOADER="1")
add_compile_definitions(PROGRAM_VERSION="${PROJECT_VERSION}")

install(TARGETS ld.so DESTINATION lib)
target_compile_options(ld.so PRIVATE -fvisibility=hidden -fPIC)
target_link_options(ld.so PRIVATE -nostdlib -shared -fPIC -fPIE -fno-plt -Wl,-e,_dl_start)
