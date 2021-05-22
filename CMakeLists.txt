cmake_minimum_required(VERSION 3.17)
project(chess)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
link_directories(.)

add_executable(chess
        tests/chessSystemTestsExample.c
        chessSystem.c
        chessSystem.h
        util.h
        util.c
        tournaments.c
        tournaments.h
        libmap.a
        map.h)

add_executable(maptests
        map_example_test_new.c
        chessSystem.c
        chessSystem.h
        util.h
        util.c
        tournaments.c
        tournaments.h
        libmap.a
        map.h)


target_link_libraries(chess libmap.a)
target_link_libraries(maptests libmap.a)
set_property(TARGET chess PROPERTY POSITION_INDEPENDENT_CODE FALSE)
set_property(TARGET maptests PROPERTY POSITION_INDEPENDENT_CODE FALSE)
