# Create the library (update the source files as needed)
set(JUNO_EXAMPLE_DIR ${PROJECT_SOURCE_DIR}/examples)
aux_source_directory(${JUNO_EXAMPLE_DIR} JUNO_EXAMPLE_SRCS)


# Loop over each file and remove the extension
foreach(file ${JUNO_EXAMPLE_SRCS})
    # Get the file name without extension (NAME_WE stands for Name Without Extension)
    get_filename_component(example_name ${file} NAME_WE)
    add_executable(${example_name} ${file})
    target_link_libraries(${example_name} ${PROJECT_NAME} m)
    target_compile_options(${example_name} PRIVATE 
      ${JUNO_COMPILE_OPTIONS}
      $<$<COMPILE_LANGUAGE:C>:${JUNO_COMPILE_C_OPTIONS}>
      $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
    )
endforeach()
