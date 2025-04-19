cmake_minimum_required(VERSION 3.30)
project(asciiplayer)

set(CMAKE_CXX_STANDARD 17)

add_executable(asciiplayer main.cpp
        utils/console_info.cpp
        utils/console_info.h)
