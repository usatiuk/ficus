#include <gtest/gtest.h>

#include <SkipList.hpp>
#include <String.hpp>
#include <string>

using sl_test_list = testing::Types<String, std::string>;
template<class>
struct SkipListTestFixture : testing::Test {};
TYPED_TEST_SUITE(SkipListTestFixture, sl_test_list);

TYPED_TEST(SkipListTestFixture, PlaceAdd) {
    SkipList<int, TypeParam> test1;

    test1.add(5, "test5", false);
    test1.add(999, "test999", false);
    test1.add(5, "test5", false);
    test1.add(1, "test1", false);
    test1.add(999, "test999", false);

    ASSERT_EQ(test1.find(5)->data, "test5");
    ASSERT_EQ(test1.find(1)->data, "test1");
    ASSERT_EQ(test1.find(999)->data, "test999");

    test1.erase(1);
    ASSERT_NE(test1.find(1)->data, "test1");
    test1.add(87, "test87", false);
    ASSERT_EQ(test1.find(87)->data, "test87");

    auto p2 = test1.lower_bound_update(78);
    ASSERT_EQ(p2->data, "test87");
    test1.add(78, "test78", true);
    ASSERT_EQ(test1.find(78)->data, "test78");
}
