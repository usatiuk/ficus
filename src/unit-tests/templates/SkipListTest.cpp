#include <gtest/gtest.h>

#include <SkipList.hpp>
#include <String.hpp>
#include <string>

using sl_test_list = testing::Types<String, std::string>;
template<class>
struct SkipListTestFixture : testing::Test {};
TYPED_TEST_SUITE(SkipListTestFixture, sl_test_list);

TYPED_TEST(SkipListTestFixture, PlaceAdd) {
    SkipListMap<int, TypeParam>  test1;
    SkipListMap<int, TypeParam>  test2;
    SkipListMap<long, TypeParam> test3;
    SkipListMap<long, TypeParam> test4;
    SkipListMap<long, long>      test5;
    SkipListMap<long, long>      test6;
    SkipListMap<int, long>       test7;
    SkipListMap<int, long>       test8;

    test1.emplace(5, "test5");
    test1.emplace(999, "test999");
    test1.emplace(5, "test5");
    test1.emplace(1, "test1");
    test1.emplace(999, "test999");

    ASSERT_EQ(test1.find(5)->second, "test5");
    ASSERT_EQ(test1.find(1)->second, "test1");
    ASSERT_EQ(test1.find(999)->second, "test999");

    typename decltype(test1)::iterator       tit  = test1.begin();
    typename decltype(test1)::const_iterator tcit = tit;

    test1.erase(1);
    ASSERT_EQ(test1.find(1), test1.cend());
    test1.emplace(87, "test87");
    ASSERT_EQ(test1.find(87)->second, "test87");

    test1.emplace(78, "test78");
    ASSERT_EQ(test1.find(78)->second, "test78");
}
// TYPED_TEST(SkipListTestFixture, MultiMapTest) {
//     SkipListMultiMap<int, TypeParam> test1;
//
//     test1.emplace(5, "test5");
//     test1.emplace(999, "test999");
//     test1.emplace(5, "test5");
//     test1.emplace(1, "test1");
//     test1.emplace(999, "test999");
//
//     ASSERT_EQ(test1.find(5)->second, "test5");
//     ASSERT_EQ(test1.find(1)->second, "test1");
//     ASSERT_EQ(test1.find(999)->second, "test999");
//
//     test1.erase(1);
//     ASSERT_EQ(test1.find(1), test1.cend());
//     test1.emplace(87, "test87");
//     ASSERT_EQ(test1.find(87)->second, "test87");
//
//     test1.emplace(78, "test78");
//     ASSERT_EQ(test1.find(78)->second, "test78");
//
//     ASSERT_EQ(test1.find(5)->second, "test5");
//     ASSERT_EQ(test1.find(999)->second, "test999");
//     test1.erase(5);
//     test1.erase(999);
//     ASSERT_EQ(test1.find(5)->second, "test5");
//     ASSERT_EQ(test1.find(999)->second, "test999");
//     test1.erase(5);
//     test1.erase(999);
//     ASSERT_EQ(test1.find(5), test1.end());
//     ASSERT_EQ(test1.find(999), test1.end());
//
//     auto r1 = test1.emplace(999, "test9991");
//     ASSERT_TRUE(r1.second);
//     ASSERT_EQ(r1.first->second, "test9991");
//     auto r2 = test1.emplace(999, "test9992");
//     ASSERT_TRUE(r2.second);
//     ASSERT_EQ(r2.first->second, "test9992");
//     auto r3 = test1.emplace(999, "test9993");
//     ASSERT_TRUE(r3.second);
//     ASSERT_EQ(r3.first->second, "test9993");
//
//     test1.erase(r2.first);
//     ASSERT_TRUE(r1.second);
//     ASSERT_EQ(r1.first->second, "test9991");
//     ASSERT_TRUE(r3.second);
//     ASSERT_EQ(r3.first->second, "test9993");
// }
//
// TYPED_TEST(SkipListTestFixture, MultiMapEraseTest) {
//     SkipListMultiMap<int, TypeParam> test1;
//
//     test1.emplace(5, "test5");
//     test1.emplace(999, "test999");
//     test1.emplace(5, "test5");
//     test1.emplace(1, "test1");
//     test1.emplace(999, "test999");
//
//     ASSERT_EQ(test1.find(5)->second, "test5");
//     ASSERT_EQ(test1.find(1)->second, "test1");
//     ASSERT_EQ(test1.find(999)->second, "test999");
//
//     test1.erase(1);
//     ASSERT_EQ(test1.find(1), test1.cend());
//     test1.emplace(87, "test87");
//     ASSERT_EQ(test1.find(87)->second, "test87");
//
//     test1.emplace(78, "test78");
//     ASSERT_EQ(test1.find(78)->second, "test78");
//
//     ASSERT_EQ(test1.find(5)->second, "test5");
//     ASSERT_EQ(test1.find(999)->second, "test999");
//     test1.erase(5);
//     test1.erase(999);
//     ASSERT_EQ(test1.find(5)->second, "test5");
//     ASSERT_EQ(test1.find(999)->second, "test999");
//     test1.erase(5);
//     test1.erase(999);
//     ASSERT_EQ(test1.find(5), test1.end());
//     ASSERT_EQ(test1.find(999), test1.end());
//
//     {
//         auto r1 = test1.emplace(999, "test9991");
//         ASSERT_TRUE(r1.second);
//         ASSERT_EQ(r1.first->second, "test9991");
//         auto r2 = test1.emplace(999, "test9992");
//         ASSERT_TRUE(r2.second);
//         ASSERT_EQ(r2.first->second, "test9992");
//         auto r3 = test1.emplace(999, "test9993");
//         ASSERT_TRUE(r3.second);
//         ASSERT_EQ(r3.first->second, "test9993");
//
//         test1.erase(r2.first);
//         ASSERT_TRUE(r1.second);
//         ASSERT_EQ(r1.first->second, "test9991");
//         ASSERT_TRUE(r3.second);
//         ASSERT_EQ(r3.first->second, "test9993");
//     }
//     test1.emplace(5, "test5");
//     test1.emplace(999, "test999");
//     test1.emplace(5, "test5");
//     test1.emplace(1, "test1");
//     test1.emplace(999, "test999");
//     {
//         auto r1 = test1.emplace(500, "t");
//         ASSERT_TRUE(r1.second);
//         ASSERT_EQ(r1.first->second, "t");
//         auto r2 = test1.emplace(500, "t");
//         ASSERT_TRUE(r2.second);
//         ASSERT_EQ(r2.first->second, "t");
//         auto r3 = test1.emplace(500, "t");
//         ASSERT_TRUE(r3.second);
//         ASSERT_EQ(r3.first->second, "t");
//
//         test1.erase(r2.first);
//         ASSERT_EQ(r1.first->second, "t");
//         ASSERT_EQ(r3.first->second, "t");
//         test1.erase(r3.first);
//         ASSERT_EQ(r1.first->second, "t");
//         ASSERT_EQ(test1.find(500), r1.first);
//         test1.erase(r1.first);
//         ASSERT_EQ(test1.find(500), test1.end());
//     }
//     {
//         auto r1 = test1.emplace(500, "t");
//         ASSERT_TRUE(r1.second);
//         ASSERT_EQ(r1.first->second, "t");
//         auto r2 = test1.emplace(500, "t");
//         ASSERT_TRUE(r2.second);
//         ASSERT_EQ(r2.first->second, "t");
//         auto r3 = test1.emplace(500, "t");
//         ASSERT_TRUE(r3.second);
//         ASSERT_EQ(r3.first->second, "t");
//
//         test1.erase(r2.first);
//         ASSERT_EQ(r1.first->second, "t");
//         ASSERT_EQ(r3.first->second, "t");
//         test1.erase(r1.first);
//         ASSERT_EQ(r3.first->second, "t");
//         ASSERT_EQ(test1.find(500), r3.first);
//         test1.erase(r3.first);
//         ASSERT_EQ(test1.find(500), test1.end());
//     }
//
//     {
//         auto r1 = test1.emplace(500, "t");
//         ASSERT_TRUE(r1.second);
//         ASSERT_EQ(r1.first->second, "t");
//         auto r2 = test1.emplace(500, "t");
//         ASSERT_TRUE(r2.second);
//         ASSERT_EQ(r2.first->second, "t");
//         auto r3 = test1.emplace(500, "t");
//         ASSERT_TRUE(r3.second);
//         ASSERT_EQ(r3.first->second, "t");
//
//         test1.erase(r1.first);
//         ASSERT_EQ(r2.first->second, "t");
//         ASSERT_EQ(r3.first->second, "t");
//         test1.erase(r2.first);
//         ASSERT_EQ(r3.first->second, "t");
//         ASSERT_EQ(test1.find(500), r3.first);
//         test1.erase(r3.first);
//         ASSERT_EQ(test1.find(500), test1.end());
//     }
//     {
//         auto r1 = test1.emplace(500, "t");
//         ASSERT_TRUE(r1.second);
//         ASSERT_EQ(r1.first->second, "t");
//         auto r2 = test1.emplace(500, "t");
//         ASSERT_TRUE(r2.second);
//         ASSERT_EQ(r2.first->second, "t");
//         auto r3 = test1.emplace(500, "t");
//         ASSERT_TRUE(r3.second);
//         ASSERT_EQ(r3.first->second, "t");
//
//         test1.erase(r3.first);
//         ASSERT_EQ(r1.first->second, "t");
//         ASSERT_EQ(r2.first->second, "t");
//         test1.erase(r1.first);
//         ASSERT_EQ(r2.first->second, "t");
//         ASSERT_EQ(test1.find(500), r2.first);
//         test1.erase(r2.first);
//         ASSERT_EQ(test1.find(500), test1.end());
//     }
// }
