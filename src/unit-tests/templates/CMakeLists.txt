add_executable(
        SkipListTest
        SkipListTest.cpp
)
target_link_libraries(
        SkipListTest
        templates
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(SkipListTest DISCOVERY_TIMEOUT 600)
