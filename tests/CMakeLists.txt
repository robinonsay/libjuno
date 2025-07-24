# Create the library (update the source files as needed)
set(JUNO_TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
aux_source_directory(${JUNO_TEST_DIR} JUNO_TEST_SRCS)


# Loop over each file and remove the extension
foreach(file ${JUNO_TEST_SRCS})
    # Get the file name without extension (NAME_WE stands for Name Without Extension)
    get_filename_component(test_name ${file} NAME_WE)
    add_executable(${test_name} ${file})
    target_link_libraries(${test_name} ${PROJECT_NAME} ${JUNO_POSIX_TARGET} unity)
    target_compile_options(${test_name} PRIVATE ${JUNO_COMPILE_OPTIONS})
    add_test(${test_name} ${test_name})
endforeach()



configure_file(
  "${CMAKE_SOURCE_DIR}/tests/test_zip_crc.bin"
  "${CMAKE_BINARY_DIR}/tests/test_zip_crc.bin"
  COPYONLY
)