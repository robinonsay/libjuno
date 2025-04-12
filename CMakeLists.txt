cmake_minimum_required(VERSION 3.10)

project(juno VERSION 0.0.1 LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(JUNO_TESTS "Enable unit testing" OFF)
message("Compiler: ${CMAKE_C_COMPILER}")
message("Testing: ${JUNO_TESTS}")

if(JUNO_API)
    add_definitions(-DJUNO_API)
endif(JUNO_API)



# Create the library (update the source files as needed)
aux_source_directory(${CMAKE_SOURCE_DIR}/src JUNO_SRCS)
add_library(${PROJECT_NAME} STATIC
    ${JUNO_SRCS}
)

set(JUNO_LIBS ${PROJECT_NAME})
foreach(file ${JUNO_SRCS})
    # Get the file name without extension (NAME_WE stands for Name Without Extension)
    get_filename_component(lib_name ${file} NAME_WE)
    add_library(${lib_name} STATIC
        ${file}
    )
    list(APPEND JUNO_LIBS ${lib_name})
endforeach()

# Specify the include directories for consumers of this library
foreach(lib ${JUNO_LIBS})
    target_include_directories(${lib} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    # Enable a comprehensive set of warnings
    target_compile_options(${lib} PRIVATE
    -Wall                           # Enable most warning messages
    -Wextra                         # Enable additional warning messages
    -Werror                         # Treat all warnings as errors
    -pedantic                       # Enforce ISO C standards
    -Wshadow                        # Warn when variable declarations shadow outer scope variables
    -Wcast-align                    # Warn for potential issues with pointer casts
    -Wstrict-prototypes             # Warn if a function is declared without specifying argument types
    -Wmissing-prototypes            # Warn if a global function is defined without a previous prototype declaration
    -Wundef                         # Warn if an undefined identifier is evaluated in an #if directive
    -Wswitch                        # Warn about missing cases in switch statements
    -Wswitch-default                # Warn whenever a switch statement does not have a default case
    -Wmissing-field-initializers    # Warn about missing fields in struct init
    -fno-common                     # Disallow multiple definitions of common symbols
    -fno-strict-aliasing            # Disable strict aliasing
    -ffreestanding                  # Disable the hosted environment
    -fPIC
    )

    target_link_options(${lib} PRIVATE
        -nostdlib
    )

endforeach()

if(JUNO_TESTS)
    set(CMAKE_BUILD_TYPE Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE
        --coverage
        -g
        -O0
    )
    link_libraries(
        gcov
    )
    enable_testing()
    add_library(unity ${CMAKE_SOURCE_DIR}/deps/unity/src/unity.c)
    target_include_directories(unity PUBLIC ${CMAKE_SOURCE_DIR}/deps/unity/src)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()

add_library(${PROJECT_NAME}_Shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}>)
set_target_properties(${PROJECT_NAME}_Shared PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

# Install the library target
install(TARGETS ${JUNO_LIBS} ${PROJECT_NAME}_Shared
    EXPORT junoTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/juno DESTINATION include)

# Export the targets to a script
install(EXPORT junoTargets
    FILE junoTargets.cmake
    NAMESPACE juno::
    DESTINATION lib/cmake/juno
)

include(CMakePackageConfigHelpers)

# Configure the package configuration file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/junoConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/junoConfig.cmake"
    INSTALL_DESTINATION lib/cmake/juno
    NO_SET_AND_CHECK_MACRO
)

# Write a version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/junoConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Install the package configuration files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/junoConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/junoConfigVersion.cmake"
    DESTINATION lib/cmake/juno
)
