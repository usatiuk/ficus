#include <gtest/gtest.h>

#include <SkipList.hpp>

// FIXME
char *itoa(int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;
    // Check for supported base.
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if (value < 0 && base == 10) {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while (low < ptr) {
        char tmp = *low;
        *low++   = *ptr;
        *ptr--   = tmp;
    }
    return rc;
}

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
