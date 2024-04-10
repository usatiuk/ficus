target_link_libraries(templates INTERFACE KApi)

add_executable(
        SkipListTest
        SkipListTest.cpp
)

target_link_libraries(
        SkipListTest
        templates
        GTest::gtest_main
)
add_executable(
        SkipListDetailedTest
        SkipListDetailedTest.cpp
)

target_link_libraries(
        SkipListDetailedTest
        templates
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(SkipListTest DISCOVERY_TIMEOUT 600)
gtest_discover_tests(SkipListDetailedTest DISCOVERY_TIMEOUT 600)
