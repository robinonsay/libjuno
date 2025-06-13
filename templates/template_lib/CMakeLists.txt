cmake_minimum_required(VERSION 3.10)

project(template VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("Compiler: ${CMAKE_C_COMPILER}")
message("CPP Compiler: ${CMAKE_CXX_COMPILER}")

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
    -fPIC                           # Platform Independent
    -O1                             # Turn on some optimization
)

set(JUNO_COMPILE_CXX_OPTIONS
    -fno-rtti             # Warn if a function is declared without specifying argument types
    -fno-exceptions       # Warn if a global function is defined without a previous prototype declaration
)

set(JUNO_COMPILE_C_OPTIONS
    -Wstrict-prototypes             # Warn if a function is declared without specifying argument types
    -Wmissing-prototypes            # Warn if a global function is defined without a previous prototype declaration
)

find_library(
    JUNO_LIBRARY
    NAME juno
    PATHS ${template_SOURCE_DIR}/.local/ ENV JUNO_PATH
    PATH_SUFFIXES lib lib64
)

find_library(
    JUNO_SHARED_LIBRARY
    NAME juno_shared
    PATHS ${template_SOURCE_DIR}/.local/ ENV JUNO_PATH
    PATH_SUFFIXES lib lib64
)

find_path(
    JUNO_INCLUDE
    NAME juno
    PATHS ${template_SOURCE_DIR}/.local/ ENV JUNO_PATH
    PATH_SUFFIXES include inc
)


aux_source_directory(${template_SOURCE_DIR}/src TEMPLATE_SRCS)

set(TEMPLATE_SHARED_TARGET ${PROJECT_NAME}_shared)
add_library(${TEMPLATE_SHARED_TARGET} SHARED ${TEMPLATE_SRCS})
add_library(${PROJECT_NAME} STATIC ${TEMPLATE_SRCS})


target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${template_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
    PRIVATE
    ${JUNO_INCLUDE}
    ${template_SOURCE_DIR}/src
)

target_include_directories(${TEMPLATE_SHARED_TARGET} PUBLIC
    $<BUILD_INTERFACE:${template_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
    PRIVATE
    ${JUNO_INCLUDE}
    ${template_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${JUNO_LIBRARY})
target_link_libraries(${TEMPLATE_SHARED_TARGET} PUBLIC ${JUNO_SHARED_LIBRARY})

target_compile_options(${TEMPLATE_SHARED_TARGET} PRIVATE
    ${JUNO_COMPILE_OPTIONS}
    $<$<COMPILE_LANGUAGE:C>:${JUNO_COMPILE_C_OPTIONS}>
    $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${JUNO_COMPILE_OPTIONS}
    $<$<COMPILE_LANGUAGE:C>:${JUNO_COMPILE_C_OPTIONS}>
    $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
)

install(
  TARGETS ${PROJECT_NAME} ${JUNO_SHARED_TARGET}
  EXPORT junoTargets                          # for later use in find_package()
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # usually “lib”
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY ${template_SOURCE_DIR}/include/     # source directory
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include   # usually “include/juno”
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"  # only headers
)
