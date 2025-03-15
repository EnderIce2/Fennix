cmake_minimum_required(VERSION 3.10)
project(FennixRuntime_${TARGET_OS}_${TARGET_ARCH})

find_program(COMPILER_PATH NAMES $ENV{CC} gcc REQUIRED)

set(BUILD_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib")
set(INSTALL_OUTPUT_DIR "${CMAKE_INSTALL_PREFIX}/lib")

file(MAKE_DIRECTORY ${BUILD_OUTPUT_DIR})

execute_process(COMMAND ${COMPILER_PATH} -print-file-name=crtbegin.o OUTPUT_VARIABLE CRTBEGIN_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${COMPILER_PATH} -print-file-name=crtend.o OUTPUT_VARIABLE CRTEND_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${COMPILER_PATH} -print-file-name=crti.o OUTPUT_VARIABLE CRTI_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${COMPILER_PATH} -print-file-name=crtn.o OUTPUT_VARIABLE CRTN_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)

add_custom_target(copy_crt_files ALL
  COMMAND cp ${CRTBEGIN_PATH} ${CRTEND_PATH} ${CRTI_PATH} ${CRTN_PATH} ${BUILD_OUTPUT_DIR}
  COMMENT "Copying CRT files"
)

file(GLOB CRT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set(OBJECT_FILES "")

foreach(source ${CRT_SOURCES})
  get_filename_component(name ${source} NAME_WE)
  set(obj "${BUILD_OUTPUT_DIR}/${name}.o")
  add_custom_command(
    OUTPUT ${obj}
    COMMAND ${COMPILER_PATH} -c ${source} -o ${obj}
    DEPENDS ${source}
  )
  list(APPEND OBJECT_FILES ${obj})
endforeach()

if(OBJECT_FILES)
  add_custom_target(crt_objects ALL DEPENDS ${OBJECT_FILES})
endif()

install(FILES ${BUILD_OUTPUT_DIR}/crtbegin.o ${BUILD_OUTPUT_DIR}/crtend.o ${BUILD_OUTPUT_DIR}/crti.o ${BUILD_OUTPUT_DIR}/crtn.o
  DESTINATION lib
)

install(FILES ${OBJECT_FILES} DESTINATION lib)
