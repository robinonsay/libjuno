cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(JUNO_TESTS "Enable unit testing" OFF)
option(JUNO_COVERAGE "Compile Juno with code coverage" OFF)
option(JUNO_DOCS "Enable doxygen docs" OFF)
option(JUNO_PIC "Compile Juno with Position Independent Code" ON)
option(JUNO_SHARED "Compile the juno shared library" OFF)
message("Compiler: ${CMAKE_C_COMPILER}")
message("Testing: ${JUNO_TESTS}")

project(juno VERSION 0.42.0 LANGUAGES C CXX)

if(JUNO_API)
    add_definitions(-DJUNO_API)
endif(JUNO_API)



# Create the library (update the source files as needed)
aux_source_directory(${PROJECT_SOURCE_DIR}/src JUNO_SRCS)
add_library(${PROJECT_NAME} STATIC
    ${JUNO_SRCS}
)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

set(JUNO_COMPILE_OPTIONS
    -Wall                           # Enable most warning messages
    -Wextra                         # Enable additional warning messages
    -Werror                         # Treat all warnings as errors
    -pedantic                       # Enforce ISO C standards
    -Wshadow                        # Warn when variable declarations shadow outer scope variables
    -Wcast-align                    # Warn for potential issues with pointer casts
    -Wundef                         # Warn if an undefined identifier is evaluated in an #if directive
    -Wswitch                        # Warn about missing cases in switch statements
    -Wswitch-default                # Warn whenever a switch statement does not have a default case
    -Wmissing-field-initializers    # Warn about missing fields in struct init
    -fno-common                     # Disallow multiple definitions of common symbols
    -fno-strict-aliasing            # Disable strict aliasing
)

set(JUNO_COMPILE_CXX_OPTIONS
    -fno-rtti             # Warn if a function is declared without specifying argument types
    -fno-exceptions       # Warn if a global function is defined without a previous prototype declaration
)

set(JUNO_COMPILE_C_OPTIONS
    -Wstrict-prototypes             # Warn if a function is declared without specifying argument types
    -Wmissing-prototypes            # Warn if a global function is defined without a previous prototype declaration
)

if(JUNO_PIC)
    list(APPEND JUNO_COMPILE_OPTIONS -fPIC)
endif()
if(JUNO_COVERAGE)
    list(APPEND JUNO_COMPILE_OPTIONS
        --coverage
        -g
    )
    link_libraries(gcov)
endif()

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
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )
    # Enable a comprehensive set of warnings
    target_compile_options(${lib} PRIVATE
        ${JUNO_COMPILE_OPTIONS}
        $<$<COMPILE_LANGUAGE:C>:${JUNO_COMPILE_C_OPTIONS}>
        $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
        -nostdlib
        -ffreestanding                  # Disable the hosted environment
    )
    target_link_options(${lib} PRIVATE
        -nostdlib
    )
endforeach()

if(JUNO_SHARED)
    set(JUNO_SHARED_TARGET ${PROJECT_NAME}_shared)
    add_library(${JUNO_SHARED_TARGET} SHARED ${JUNO_SRCS})
    target_include_directories(${JUNO_SHARED_TARGET} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
    )
endif()

install(
  TARGETS ${JUNO_LIBS} ${JUNO_SHARED_TARGET} ${JUNO_POSIX_TARGET}
  EXPORT junoTargets                          # for later use in find_package()
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # usually “lib”
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/include/     # source directory
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include  # usually “include/juno”
  FILES_MATCHING PATTERN "*.h"               # only headers
)

if(JUNO_TESTS)
    add_definitions(-DUNITY_INCLUDE_DOUBLE)
    set(CMAKE_BUILD_TYPE Debug)
    enable_testing()
    add_library(unity ${PROJECT_SOURCE_DIR}/deps/unity/src/unity.c)
    target_include_directories(unity PUBLIC ${PROJECT_SOURCE_DIR}/deps/unity/src)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()


if(JUNO_DOCS)
# 1) Find Doxygen
find_package(Doxygen REQUIRED)

if (DOXYGEN_FOUND)
  # 2a) Point to your template Doxyfile.in (in source tree)
  set(DOXYFILE_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  # 2b) Tell CMake where to generate the real Doxyfile
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # 2c) Substitute @PROJECT_NAME@, @PROJECT_VERSION@, etc.
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  # 3) Create a custom target "docs"
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
else()
  message(WARNING "Doxygen not found—'make docs' target will be unavailable.")
endif()
endif()
