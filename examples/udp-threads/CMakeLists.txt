# MIT License
# Copyright (c) 2025 Robin A. Onsay

cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(udp_threads VERSION 1.0.0 LANGUAGES C CXX)

if(JUNO_EXAMPLES)

    # Include FetchContent for downloading Google Test
    include(FetchContent)

    # Declare Google Test dependency
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
    )

    # Make Google Test available
    FetchContent_MakeAvailable(googletest)

    # Define compile options for the project
    set(JUNO_COMPILE_OPTIONS
        -Wall
        -Wextra
        -Werror
        -pedantic
        -Wshadow
        -Wcast-align
        -fno-common
        -fno-strict-aliasing
    )

    set(JUNO_COMPILE_CXX_OPTIONS
        -fno-rtti
        -fno-exceptions
    )

    # Create the static library target
    add_library(udp_threads_lib STATIC
        src/juno_udp_init.cpp
        src/linux_udp_impl.cpp
        src/juno_thread_init.cpp
        src/udp_thread_msg.cpp
        src/linux_thread_impl.cpp
        src/sender_app.cpp
        src/monitor_app.cpp
        src/udp_bridge_app.cpp
        src/processor_app.cpp
    )

    # Include directories for the library
    target_include_directories(udp_threads_lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Link against juno library and pthread
    target_link_libraries(udp_threads_lib PUBLIC
        juno
        pthread
    )

    # Apply compile options to the library
    target_compile_options(udp_threads_lib PRIVATE
        ${JUNO_COMPILE_OPTIONS}
        $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
    )

    # Create the test executable
    add_executable(udp_threads_tests
        tests/test_udp_module.cpp
        tests/test_thread_module.cpp
        tests/test_apps.cpp
    )

    # Link test executable against library, Google Test, and pthread
    target_link_libraries(udp_threads_tests
        udp_threads_lib
        GTest::gtest_main
        pthread
    )

    # Apply compile options to test executable
    target_compile_options(udp_threads_tests PRIVATE
        ${JUNO_COMPILE_OPTIONS}
        $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
    )

    # Register tests with Google Test
    include(GoogleTest)
    gtest_discover_tests(udp_threads_tests)

    # Create the main executable
    add_executable(udp_threads_main
        src/main.cpp
    )

    # Link main executable against the library
    target_link_libraries(udp_threads_main
        udp_threads_lib
    )

    # Apply compile options to main executable
    target_compile_options(udp_threads_main PRIVATE
        ${JUNO_COMPILE_OPTIONS}
        $<$<COMPILE_LANGUAGE:CXX>:${JUNO_COMPILE_CXX_OPTIONS}>
    )

endif()
