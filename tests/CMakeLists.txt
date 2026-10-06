# Tests

add_executable(floe-tests
    lexer_test.cpp
)

target_link_libraries(floe-tests PRIVATE
    floe-syntax
    floe-diagnostics
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(floe-tests)
