cmake_minimum_required(VERSION 3.20)
project(asciiplayer)

set(CMAKE_CXX_STANDARD 17)

# Custom variables
set(ENABLE_TESTS OFF CACHE BOOL "Build and run tests if enabled")

# Works on Linux:tm: as long as you have those libraries installed
# I haven't really given much thought into how to deal with dependencies on Windows
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVUTIL_LIBRARY avutil)
find_library(SWSCALE_LIBRARY swscale)

add_executable(asciiplayer main.cpp
        src/Console.cpp
        src/Console.h
        src/Player.cpp
        src/Player.h
        src/Logger.cpp
        src/Logger.h
        src/WorkQueue.h)

target_include_directories(asciiplayer
    PRIVATE /usr/include/ffmpeg)

target_link_libraries(asciiplayer
    PRIVATE ${AVCODEC_LIBRARY}
    PRIVATE ${AVFORMAT_LIBRARY}
    PRIVATE ${AVUTIL_LIBRARY}
    PRIVATE ${SWSCALE_LIBRARY})

# Setup GoogleTest for testing purposes
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

if (ENABLE_TESTS)
    include(cmake/tests/WorkQueueTests.cmake)
endif()