cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("Compiler: ${CMAKE_C_COMPILER}")
message("CPP Compiler: ${CMAKE_CXX_COMPILER}")

project(template VERSION 0.1.0 LANGUAGES C CXX)

set(JUNO_COMPILE_OPTIONS
    -Wall                           # Enable most warning messages
    -Wextra                         # Enable additional warning messages
    -Werror                         # Treat all warnings as errors
    -pedantic                       # Enforce ISO C standards
    -Wshadow                        # Warn when variable declarations shadow outer scope variables
    -Wcast-align                    # Warn for potential issues with pointer casts
    -Wswitch                        # Warn about missing cases in switch statements
    -Wswitch-default                # Warn whenever a switch statement does not have a default case
    -Wmissing-field-initializers    # Warn about missing fields in struct init
    -fno-common                     # Disallow multiple definitions of common symbols
    -fno-strict-aliasing            # Disable strict aliasing
    -fPIC                           # Platform Independent
    -O1                             # Turn on some optimization
)

set(JUNO_COMPILE_CXX_OPTIONS
    -fno-rtti             # Warn if a function is declared without specifying argument types
    -fno-exceptions       # Warn if a global function is defined without a previous prototype declaration
)

set(JUNO_COMPILE_C_OPTIONS

)

find_library(
    JUNO_LIBRARY
    NAME juno
    PATHS ${PROJECT_SOURCE_DIR}/.local/ ENV JUNO_PATH
    PATH_SUFFIXES lib lib64
)

find_library(
    JUNO_SHARED_LIBRARY
    NAME juno_shared
    PATHS ${PROJECT_SOURCE_DIR}/.local/ ENV JUNO_PATH
    PATH_SUFFIXES lib lib64
)

find_path(
    JUNO_INCLUDE
    NAME juno
    PATHS ${PROJECT_SOURCE_DIR}/.local/ ENV JUNO_PATH
    PATH_SUFFIXES include inc
)


aux_source_directory(${PROJECT_SOURCE_DIR}/src ${PROJECT_NAME}_SRCS)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRCS})


target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
    PRIVATE
    ${JUNO_INCLUDE}
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${JUNO_LIBRARY})

target_compile_options(${PROJECT_NAME} PRIVATE
    ${JUNO_COMPILE_OPTIONS}
    $<$<COMPILE_LANGUAGE:C>:${JUNO_COMPILE_C_OPTIONS}>
    $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT junoTargets                          # for later use in find_package()
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # usually “lib”
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/include/     # source directory
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include   # usually “include/juno”
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"  # only headers
)
