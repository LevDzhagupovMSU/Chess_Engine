find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)

add_library(chess_server STATIC
    src/chess-server.cpp
    src/session.cpp
    src/handle_errors.cpp
    moves/serializer.cpp
)

target_compile_features(chess_server PUBLIC cxx_std_17)

target_include_directories(chess_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/moves
)

target_link_libraries(chess_server PUBLIC
    Boost::system
    nlohmann_json::nlohmann_json
    chess_engine
)

add_executable(chess_server_app
    src/main.cpp
)

target_link_libraries(chess_server_app
    PRIVATE chess_server
)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(chess_server_serializer_test tests/serializer_test.cpp)
    target_link_libraries(chess_server_serializer_test PRIVATE chess_server GTest::gtest_main)
    gtest_discover_tests(chess_server_serializer_test)
endif()
