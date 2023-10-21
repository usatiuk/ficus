//
// Created by Stepan Usatiuk on 21.10.2023.
//

#include "TestTemplates.hpp"


#include "SkipList.hpp"
#include "SkipListSet.hpp"
#include "String.hpp"
#include "Vector.hpp"
#include "serial.hpp"

#include "tty.hpp"

//#include "String.hpp"
//#include "String.hpp"

//template<typename T>
//std::string toString(const T &x) {
//    std::ostringstream oss;
//    oss << x;
//    return oss.str();
//}
//
//class SharedPtrTester {
//private:
//    auto getThingy() {
//        return SharedPtr<Vector<String>>(
//                new Vector<String>{"Thingy1", "Thingy2", "Thingy3"});
//    }
//
//public:
//    bool test() {
//        SharedPtr<Vector<String>> test1(
//                new Vector<String>{"hello1", "hello2", "hello3"});
//        SharedPtr<Vector<String>> test2(getThingy());
//        assert((*test2 == Vector<String>{"Thingy1", "Thingy2", "Thingy3"}));
//        auto test22 = test2;
//        auto test12 = test1;
//        test12->emplace_back("hello4");
//        assert((*test1)[3] == "hello4");
//        test22->erase(2);
//        assert(test2->size() == 2);
//        assert((*test2)[1] == "Thingy2");
//        return true;
//    }
//};
//
//class COWTester {
//private:
//    auto getThingy() const {
//        return COWPointer<Vector<String>>(
//                new Vector<String>{"Thingy1", "Thingy2", "Thingy3"});
//    }
//
//public:
//    bool test() const {
//        COWPointer<Vector<String>> test1(
//                new Vector<String>{"hello1", "hello2", "hello3"});
//        COWPointer<Vector<String>> test2(getThingy());
//        assert((*test2.get() == Vector<String>{"Thingy1", "Thingy2", "Thingy3"}));
//
//        auto test22 = test2;
//        auto test12 = test1;
//
//        assert(test12.ptr.get() == test1.ptr.get());
//        assert(test22.ptr.get() == test2.ptr.get());
//        assert((*test2.get() == Vector<String>{"Thingy1", "Thingy2", "Thingy3"}));
//        assert((*test22.get() == Vector<String>{"Thingy1", "Thingy2", "Thingy3"}));
//        assert((*test1.get() == Vector<String>{"hello1", "hello2", "hello3"}));
//        assert((*test12.get() == Vector<String>{"hello1", "hello2", "hello3"}));
//        assert(test12.ptr.get() == test1.ptr.get());
//        assert(test22.ptr.get() == test2.ptr.get());
//
//        test12.getRW()->emplace_back("hello4");
//        assert(test1.get()->size() == 3);
//        assert(test12.get()->size() == 4);
//        assert((*test12.get())[3] == "hello4");
//        test22.getRW()->erase(2);
//        assert(test2.get()->size() == 3);
//        assert(test22.get()->size() == 2);
//        assert((*test22.get())[1] == "Thingy2");
//        assert((*test2.get())[1] == "Thingy2");
//
//        assert(test12.ptr.get() != test1.ptr.get());
//        assert(test22.ptr.get() != test2.ptr.get());
//
//        return true;
//    }
//};

class VectorTester {
public:
    bool test() {
        Vector<String> testv1;
        testv1.emplace_back("H1");
        testv1.emplace_back("H2");
        testv1.emplace_back("H3");
        testv1.emplace_back("H4");
        assert(testv1.size() == 4);
        assert(testv1.capacity >= 4);
        assert(testv1[0] == "H1");
        assert(testv1[1] == "H2");
        assert(testv1[2] == "H3");
        assert(testv1[3] == "H4");
        testv1.erase(1);
        assert(testv1.size() == 3);
        assert(testv1[0] == "H1");
        assert(testv1[1] == "H3");
        assert(testv1[3] == "H4");
        testv1.erase(1);
        assert(testv1.size() == 2);
        assert(testv1[0] == "H1");
        assert(testv1[1] == "H4");

        testv1.emplace_back("H5");

        auto testv2 = testv1;
        assert(testv1 == testv2);
        testv1.erase(2);
        assert(testv1 != testv2);
        testv1.emplace_back("H6");
        assert(testv1 != testv2);
        assert(testv1[2] == "H6");
        assert(testv2[2] == "H5");

        all_tty_putstr("Vector tests ok!\n");
        return true;
    }
};

class StringTester {
public:
    bool test() {
        String str1("hello");
        assert(str1 == "hello");
        str1 += "Hello!";
        assert(str1 == "helloHello!");
        str1 = String("abcd");
        String str2("dcba");
        assert(str2 > str1);
        assert(str1 < str2);
        assert(str1 <= str2);
        str2 = "abcd";
        assert(str1 <= str2);

        all_tty_putstr("String tests ok!\n");
        return true;
    }
};

class SkipListTester {
public:
    bool test() {
        SkipList<int, String> test1;

        test1.add(5, "test5", false);
        test1.add(999, "test999", false);
        test1.add(5, "test5", false);
        test1.add(1, "test1", false);
        test1.add(999, "test999", false);

        assert(test1.find(5)->data == "test5");
        assert(test1.find(1)->data == "test1");
        assert(test1.find(999)->data == "test999");

        test1.erase(1);
        assert(test1.find(1)->data != "test1");
        test1.add(87, "test87", false);
        assert(test1.find(87)->data == "test87");

        auto p2 = test1.lower_bound_update(78);
        assert(p2->data == "test87");
        test1.add(78, "test78", true);
        assert(test1.find(78)->data == "test78");

        all_tty_putstr("SkipList tests ok!\n");
        return true;
    }
};

class SkipListSetTester {
public:
    bool test() {
        SkipListSet<int> test1;

        test1.add(5, false);
        test1.add(999, false);
        test1.add(5, false);
        test1.add(1, false);
        test1.add(999, false);

        assert(test1.find(5)->key == 5);
        assert(test1.find(1)->key == 1);
        assert(test1.find(999)->key == 999);

        test1.erase(1);
        assert(test1.find(1)->key != 1);
        test1.add(87, false);
        assert(test1.find(87)->key == 87);

        auto p2 = test1.lower_bound_update(78);
        assert(p2->key == 87);
        test1.add(78, true);
        assert(test1.find(78)->key == 78);

        all_tty_putstr("SkipListSet tests ok!\n");
        return true;
    }
};

int test_templates() {

    SkipListTester SLTester;
    SLTester.test();
    SkipListSetTester SLSTester;
    SLSTester.test();
    StringTester stringTester;
    stringTester.test();
    VectorTester vectorTester;
    vectorTester.test();
    //    SharedPtrTester sharedPtrTester;
    //    sharedPtrTester.test();
    //    COWTester cowTester;
    //    cowTester.test();
    return 0;
}
