cmake_minimum_required(VERSION 3.17)
project(chess)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(chess
        chessSystem.c
        chessSystem.h
        util.h
        tournaments.c
        tournaments.h
        libmap.a
        map.h)
