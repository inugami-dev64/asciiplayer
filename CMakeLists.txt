cmake_minimum_required(VERSION 3.20)
project(asciiplayer)

set(CMAKE_CXX_STANDARD 11)

# Custom variables
set(ENABLE_TESTS OFF CACHE BOOL "Build and run tests if enabled")

add_executable(asciiplayer
    main.cpp
    src/Console.cpp
    src/Console.h
    src/Player.cpp
    src/Player.h
    src/Logger.cpp
    src/Logger.h
    src/WorkQueue.h
    src/Presenter.h
    src/AudioPresenter.h
    src/AudioPresenter.cpp
    src/VideoPresenter.h
    src/VideoPresenter.cpp)

# Works on Linux:tm: as long as you have those libraries installed
# I haven't really given much thought into how to deal with dependencies on Windows
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libswresample
    libavutil
    libavdevice
    libavformat
    libavcodec
    libswscale
)

find_library(LIBPA portaudio)

target_link_libraries(asciiplayer
    PRIVATE PkgConfig::LIBAV
    PRIVATE ${LIBPA})

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