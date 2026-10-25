cmake_minimum_required(VERSION 3.20)
project(ChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(project_warnings INTERFACE)

if(MSVC)
    target_compile_options(project_warnings INTERFACE /W4 /WX /O2)
else()
    target_compile_options(project_warnings INTERFACE
        -O2 -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wunused
    )
endif()

add_library(chess_engine STATIC
    src/core/piece.cpp
    src/core/pawn.cpp
    src/core/knight.cpp
    src/core/bishop.cpp
    src/core/rook.cpp
    src/core/queen.cpp
    src/core/king.cpp
    src/core/history.cpp
    src/core/board.cpp
    src/core/board_checks.cpp
    src/core/game.cpp
    src/ai/evaluation.cpp
    src/ai/search.cpp
)

target_compile_features(chess_engine PUBLIC cxx_std_17)

target_include_directories(chess_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ai
)

target_link_libraries(chess_engine PUBLIC project_warnings)

add_executable(chess_engine_search_benchmark EXCLUDE_FROM_ALL
    benchmarks/search_benchmark.cpp
)
target_link_libraries(chess_engine_search_benchmark PRIVATE chess_engine)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(chess_engine_tests
        tests/history_test.cpp
        tests/move_test.cpp
        tests/pawn_test.cpp
        tests/knight_test.cpp
        tests/bishop_test.cpp
        tests/rook_test.cpp
        tests/queen_test.cpp
        tests/king_test.cpp
        tests/game_test.cpp
        tests/evaluation_test.cpp
        tests/board_undo_test.cpp
    )
    target_link_libraries(chess_engine_tests PRIVATE chess_engine GTest::gtest_main)
    gtest_discover_tests(chess_engine_tests)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
endif()
