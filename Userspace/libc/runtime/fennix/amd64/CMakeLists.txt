cmake_minimum_required(VERSION 3.10)
project(FennixRuntime_${TARGET_OS}_${TARGET_ARCH})

find_program(COMPILER_PATH NAMES $ENV{CC} gcc REQUIRED)

set(LIB_OUTPUT_DIR "${CMAKE_INSTALL_PREFIX}/lib")
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})

add_custom_target(copy_crt_files ALL
  COMMAND ${COMPILER_PATH} -print-file-name=libgcc.a | xargs cp -t ${LIB_OUTPUT_DIR}
  COMMAND ${COMPILER_PATH} -print-file-name=crtbegin.o | xargs cp -t ${LIB_OUTPUT_DIR}
  COMMAND ${COMPILER_PATH} -print-file-name=crtend.o | xargs cp -t ${LIB_OUTPUT_DIR}
  COMMAND ${COMPILER_PATH} -print-file-name=crti.o | xargs cp -t ${LIB_OUTPUT_DIR}
  COMMAND ${COMPILER_PATH} -print-file-name=crtn.o | xargs cp -t ${LIB_OUTPUT_DIR}
  COMMENT "Copying CRT files"
)

file(GLOB CRT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set(OBJECT_FILES "")
foreach(source ${CRT_SOURCES})
  get_filename_component(name ${source} NAME_WE)
  set(obj "${CMAKE_BINARY_DIR}/${name}.o")
  add_custom_command(
    OUTPUT ${obj}
    COMMAND ${COMPILER_PATH} -c ${source} -o ${obj}
    DEPENDS ${source}
  )
  list(APPEND OBJECT_FILES ${obj})
endforeach()

if(OBJECT_FILES)
  add_custom_target(crt_objects ALL DEPENDS ${OBJECT_FILES})
  install(FILES ${OBJECT_FILES} DESTINATION lib)
endif()
