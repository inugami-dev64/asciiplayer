cmake_minimum_required(VERSION 3.30)
project(asciiplayer)

set(CMAKE_CXX_STANDARD 17)

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