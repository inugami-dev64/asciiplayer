add_executable(
    WorkQueueTests
    src/tests/WorkQueueTests.cpp
)

target_link_libraries(
    WorkQueueTests
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(WorkQueueTests)