cmake_minimum_required(VERSION 3.10)

project(libjuno VERSION 1.0.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Create the library (update the source files as needed)
aux_source_directory(${CMAKE_SOURCE_DIR}/src LIBJUNO_SRCS)
add_library(libjuno SHARED
    ${LIBJUNO_SRCS}
)

# Specify the include directories for consumers of this library
target_include_directories(libjuno PUBLIC
    ${CMAKE_SOURCE_DIR}/inc
PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Enable a comprehensive set of warnings
target_compile_options(libjuno PUBLIC
-Wall                     # Enable most warning messages
-Wextra                   # Enable additional warning messages
-Werror                   # Treat all warnings as errors
-pedantic                 # Enforce ISO C standards
-Wshadow                  # Warn when variable declarations shadow outer scope variables
-Wcast-align             # Warn for potential issues with pointer casts
-Wconversion             # Warn about implicit conversions that may alter a value
-Wsign-conversion        # Warn for implicit sign conversions
-Wstrict-prototypes      # Warn if a function is declared without specifying argument types
-Wmissing-prototypes      # Warn if a global function is defined without a previous prototype declaration
-Wundef                  # Warn if an undefined identifier is evaluated in an #if directive
-Wswitch                 # Warn about missing cases in switch statements
-Wswitch-default         # Warn whenever a switch statement does not have a default case
-Wstrict-aliasing=2       # Enforce strict aliasing rules to avoid undefined behavior
-fno-common                 # Disallow multiple definitions of common symbols
-O2 # Optimize for performance while maintaining predictability
)
