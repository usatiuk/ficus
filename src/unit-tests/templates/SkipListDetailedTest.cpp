#include <gtest/gtest.h>

#include <SkipList.hpp>
#include <string>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

using IntType = long long;

class CRange {
public:
    IntType l        = 0;
    IntType r        = 0;

            CRange() = default;

            CRange(IntType l, IntType r) : l(l), r(r) {
        if (l > r)
            throw std::logic_error("CRange bad initialisation");
    }

    friend std::ostream &operator<<(std::ostream &out, const CRange &p) {
        std::ios::fmtflags f(out.flags());
        out << std::dec;
        if (p.l == p.r) out << p.l;
        else
            out << "<" << p.l << ".." << p.r << ">";
        out.flags(f);
        return out;
    }

    bool includes(const IntType &p) const {
        return p >= l && p <= r;
    }

    bool includes(const CRange &p) const {
        return includes(p.l) && includes(p.r);
    }

    bool operator==(const CRange &p) const {
        return p.l == l && p.r == r;
    }
};


class CRangeList {
private:
    SkipList<IntType, CRange> data;

public:
    // constructor
    CRangeList() = default;

    CRangeList(std::initializer_list<CRange> l) {
        for (auto const &r: l)
            *this += r;
    }

    CRangeList(CRangeList &&l) {
        data = std::move(l.data);
    }

    CRangeList(CRangeList const &l) {
        data = l.data;
    }

    CRangeList &operator=(CRangeList l) {
        std::swap(l.data, data);
        return *this;
    }

    // includes long long / range
    bool includes(const IntType l) const {
        auto f = findInData(l);
        if (!f->end && f->data.includes(l)) return true;
        return false;
    }

    bool includes(const CRange &r) const {
        auto f = findInData(r.l);
        if (!f->end && f->data.includes(r)) return true;
        return false;
    }

    CRangeList operator+(const CRange &rhs) const {
        return CRangeList(*this) += rhs;
    }

    CRangeList operator-(const CRange &rhs) const {
        return CRangeList(*this) -= rhs;
    }


    decltype(data)::Node *findInData(IntType const &l) const {
        return data.find(l);
    }


    void set(CRange r, bool visible) {
        auto lb         = findInData(r.l);
        auto ub         = findInData(r.r);

        bool lbHasPoint = !lb->end && lb->data.includes(r.l);
        if (visible && !lbHasPoint && !lb->end && r.l != LLONG_MIN && lb->data.includes(r.l - 1)) {
            r.l--;
            lbHasPoint = true;
        }

        bool ubHasPoint = !ub->end && ub->data.includes(r.r);
        if (visible && !ubHasPoint && r.r != LLONG_MAX && ub->next[0]->data.includes(r.r + 1)) {
            ub = ub->next[0];
            r.r++;
            ubHasPoint = true;
        }

        if (!lbHasPoint) lb = lb->next[0];

        CRange toInsert[3];

        if (visible) {
            if (lbHasPoint)
                r.l = std::min(lb->data.l, r.l);
            if (ubHasPoint)
                r.r = std::max(ub->data.r, r.r);
            toInsert[0] = {r};
            data.add(toInsert, 1, data.erase(lb, ub->next[0], false));
        } else {
            int inserted = 0;
            if (lbHasPoint && lb->data.l != r.l) {
                toInsert[0] = {lb->data.l, r.l - 1};
                inserted++;
            }
            if (ubHasPoint && ub->data.r != r.r) {
                toInsert[inserted] = {r.r + 1, ub->data.r};
                inserted++;
            }
            data.add(toInsert, inserted, data.erase(lb, ub->next[0], false));
        }
    }

    // += range / range list
    CRangeList &operator+=(const CRange &r) {
        set(r, true);
        return *this;
    }

    CRangeList &operator+=(const CRangeList &r) {
        for (auto const &l: r)
            *this += l;
        return *this;
    }

    // -= range / range list
    CRangeList &operator-=(const CRange &r) {
        set(r, false);
        return *this;
    }

    CRangeList &operator-=(const CRangeList &r) {
        for (auto const &l: r)
            *this -= l;
        return *this;
    }

    // = range / range list
    CRangeList &operator=(const CRange &r) {
        data = decltype(data)();
        *this += (r);
        return *this;
    }

    // operator ==
    bool operator==(const CRangeList &r) const {
        return this->data == r.data;
    }

    // operator !=
    bool operator!=(const CRangeList &r) const {
        return !(*this == r);
    }

    // operator <<
    friend std::ostream &operator<<(std::ostream &out, const CRangeList &rl) {
        out << "{";
        bool first = true;
        for (const auto &i: rl) {
            if (!first) out << ",";
            out << i;
            first = false;
        }
        out << "}";
        return out;
    }

    struct CRangeListIterator {
        using iterator_category                                              = std::forward_iterator_tag;
        using difference_type                                                = std::ptrdiff_t;
        using value_type                                                     = const CRange;
        using pointer                                                        = value_type *;
        using reference                                                      = value_type &;

                            CRangeListIterator(CRangeListIterator const &it) = default;

                            CRangeListIterator(decltype(data)::Node *n) : n(n){};

        reference           operator*() const { return n->data; }

        pointer             operator->() const { return &(n->data); }


        CRangeListIterator &operator++() {
            if (n != nullptr && !n->end)
                n = n->next[0];
            return *this;
        }

        CRangeListIterator operator++(int) {
            CRangeListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const CRangeListIterator &a, const CRangeListIterator &b) { return a.n == b.n; };

        friend bool operator!=(const CRangeListIterator &a, const CRangeListIterator &b) { return !(a == b); };


    private:
        decltype(data)::Node *n;
    };

    using const_iterator = CRangeListIterator;

    const_iterator begin() const {
        return {data.begin()->before->next[0]};
    }

    const_iterator end() const {
        return {data.end()->before->next[0]};
    }
};


CRangeList operator+(const CRange &lhs, const CRange &rhs) {
    return CRangeList{lhs, rhs};
}

CRangeList operator-(const CRange &lhs, const CRange &rhs) {
    return CRangeList{lhs} - rhs;
}


std::string toString(const CRangeList &x) {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

TEST(SkipListDetailedTest, ItWorks) {

    assert(std::is_trivially_copyable<CRange>::value);

    CRangeList t;

    t = CRange(10, 20);
    //    t.data.print();
    auto ts = toString(t);
    //    std::cout << ts;
    assert(ts == "{<10..20>}");
    //    return 0;
    t = CRangeList{{10, 15}};
    assert(toString(t) == "{<10..15>}");

    t = CRange(10, 20);
    //    t.data.print();
    assert(toString(t) == "{<10..20>}");
    t += CRange(LLONG_MIN, LLONG_MAX);
    assert(t.includes(CRange(LLONG_MIN, LLONG_MAX)));
    assert(t.includes(LLONG_MIN));
    assert(t.includes(LLONG_MAX));
    t -= CRange(LLONG_MIN, LLONG_MAX);

    t += CRange(LLONG_MIN, LLONG_MAX - 1);
    t += CRange(LLONG_MAX, LLONG_MAX);
    assert(toString(t) == "{<-9223372036854775808..9223372036854775807>}");
    t -= CRange(LLONG_MIN, LLONG_MAX);
    assert(toString(t) == "{}");

    t += CRange(LLONG_MIN, LLONG_MAX);
    t += CRange(LLONG_MIN, LLONG_MAX - 1);
    assert(toString(t) == "{<-9223372036854775808..9223372036854775807>}");
    t -= CRange(LLONG_MIN, LLONG_MAX);
    assert(toString(t) == "{}");


    t += CRange(LLONG_MAX, LLONG_MAX);
    t += CRange(LLONG_MIN, LLONG_MAX - 1);
    assert(toString(t) == "{<-9223372036854775808..9223372036854775807>}");
    t -= CRange(LLONG_MIN, LLONG_MAX);
    assert(toString(t) == "{}");


    t += CRange(LLONG_MIN, LLONG_MAX);
    t -= CRange(LLONG_MAX, LLONG_MAX);
    assert(toString(t) == "{<-9223372036854775808..9223372036854775806>}");
    t -= CRange(LLONG_MIN, LLONG_MAX);
    assert(toString(t) == "{}");


    t += CRange(LLONG_MIN, LLONG_MAX - 1);
    t += CRange(LLONG_MIN, LLONG_MAX);
    assert(toString(t) == "{<-9223372036854775808..9223372036854775807>}");
    t -= CRange(LLONG_MIN, LLONG_MAX);


    t += CRange(10, 20);
    assert(toString(t) == "{<10..20>}");
    t += CRange(30, 40);
    assert(toString(t) == "{<10..20>,<30..40>}");
    t += CRange(2, 4);
    assert(toString(t) == "{<2..4>,<10..20>,<30..40>}");
    t += CRange(15, 35);
    auto s = toString(t);
    assert(toString(t) == "{<2..4>,<10..40>}");
    t += CRange(15, 50);
    s = toString(t);
    assert(toString(t) == "{<2..4>,<10..50>}");
    t -= CRange(5, 6);
    s = toString(t);
    assert(toString(t) == "{<2..4>,<10..50>}");
    t += CRange(100, 150);
    assert(toString(t) == "{<2..4>,<10..50>,<100..150>}");
    t -= CRange(70, 80);
    assert(toString(t) == "{<2..4>,<10..50>,<100..150>}");
    t += CRange(60, 70);
    assert(toString(t) == "{<2..4>,<10..50>,<60..70>,<100..150>}");
    t += CRange(70, 80);
    assert(toString(t) == "{<2..4>,<10..50>,<60..80>,<100..150>}");
    t += CRange(55, 60);
    assert(toString(t) == "{<2..4>,<10..50>,<55..80>,<100..150>}");

    t += CRange(50, 55);
    assert(toString(t) == "{<2..4>,<10..80>,<100..150>}");
    auto t2 = CRange(6, 9) + CRange(151, 160);
    assert(toString(t2) == "{<6..9>,<151..160>}");
    t += t2;
    s = toString(t);

    assert(toString(t) == "{<2..4>,<6..80>,<100..160>}");
    t += CRange(-100, 200);
    assert(toString(t) == "{<-100..200>}");

    t -= CRange(-1000, -900);
    t -= CRange(1000, 1100);
    assert(toString(t) == "{<-100..200>}");

    t = CRangeList{{10, 15},
                   {20, 25},
                   {30, 40}};
    //    t.data.print();
    assert(toString(t) == "{<10..15>,<20..25>,<30..40>}");
    t -= CRange(40, 9999);
    assert(toString(t) == "{<10..15>,<20..25>,<30..39>}");
    t -= CRange(35, 9999);
    assert(toString(t) == "{<10..15>,<20..25>,<30..34>}");
    t -= CRange(35, 9999);
    assert(toString(t) == "{<10..15>,<20..25>,<30..34>}");
    t -= CRange(-1000, 10);
    assert(toString(t) == "{<11..15>,<20..25>,<30..34>}");
    t -= CRange(-1000, 13);
    assert(toString(t) == "{<14..15>,<20..25>,<30..34>}");
    t -= CRange(22, 22);
    assert(toString(t) == "{<14..15>,<20..21>,<23..25>,<30..34>}");

    t = CRangeList();
    t += CRange(0, 100);
    assert(toString(t) == "{<0..100>}");
    t += CRange(200, 300);
    assert(toString(t) == "{<0..100>,<200..300>}");
    t -= CRange(150, 250);
    assert(toString(t) == "{<0..100>,<251..300>}");
    t += CRange(160, 180);
    assert(toString(t) == "{<0..100>,<160..180>,<251..300>}");
    t -= CRange(170, 170);
    assert(toString(t) == "{<0..100>,<160..169>,<171..180>,<251..300>}");

    t = CRangeList();
    t += CRange(10, 90);
    t -= CRange(20, 30);
    t -= CRange(40, 50);
    t -= CRange(60, 90);
    t += CRange(70, 80);

    t2       = CRange(10, 90) - CRange(20, 30) - CRange(40, 50) - CRange(60, 90) + CRange(70, 80);
    ts       = toString(t);
    auto ts2 = toString(t2);
    assert(ts == ts2);

    CRangeList a, b;

    assert(sizeof(CRange) <= 2 * sizeof(long long));
    a = CRange(5, 10);
    a += CRange(25, 100);
    assert(toString(a) == "{<5..10>,<25..100>}");
    a += CRange(-5, 0);
    a += CRange(8, 50);
    assert(toString(a) == "{<-5..0>,<5..100>}");
    a += CRange(101, 105) + CRange(120, 150) + CRange(160, 180) + CRange(190, 210);
    assert(toString(a) == "{<-5..0>,<5..105>,<120..150>,<160..180>,<190..210>}");
    a += CRange(106, 119) + CRange(152, 158);
    assert(toString(a) == "{<-5..0>,<5..150>,<152..158>,<160..180>,<190..210>}");
    a += CRange(-3, 170);
    a += CRange(-30, 1000);
    assert(toString(a) == "{<-30..1000>}");
    b = CRange(-500, -300) + CRange(2000, 3000) + CRange(700, 1001);
    a += b;
    assert(toString(a) == "{<-500..-300>,<-30..1001>,<2000..3000>}");
    a -= CRange(-400, -400);
    assert(toString(a) == "{<-500..-401>,<-399..-300>,<-30..1001>,<2000..3000>}");
    a -= CRange(10, 20) + CRange(900, 2500) + CRange(30, 40) + CRange(10000, 20000);
    assert(toString(a) == "{<-500..-401>,<-399..-300>,<-30..9>,<21..29>,<41..899>,<2501..3000>}");
    try {
        a += CRange(15, 18) + CRange(10, 0) + CRange(35, 38);
        assert("Exception not thrown" == nullptr);
    } catch (const std::logic_error &e) {
    } catch (...) {
        assert("Invalid exception thrown" == nullptr);
    }
    assert(toString(a) == "{<-500..-401>,<-399..-300>,<-30..9>,<21..29>,<41..899>,<2501..3000>}");
    b = a;
    assert(a == b);
    assert(!(a != b));
    b += CRange(2600, 2700);
    assert(toString(b) == "{<-500..-401>,<-399..-300>,<-30..9>,<21..29>,<41..899>,<2501..3000>}");
    assert(a == b);
    assert(!(a != b));
    b += CRange(15, 15);
    assert(toString(b) == "{<-500..-401>,<-399..-300>,<-30..9>,15,<21..29>,<41..899>,<2501..3000>}");
    assert(!(a == b));
    assert(a != b);
    assert(b.includes(15));
    assert(b.includes(2900));
    assert(b.includes(CRange(15, 15)));
    assert(b.includes(CRange(-350, -350)));
    assert(b.includes(CRange(100, 200)));
    assert(!b.includes(CRange(800, 900)));
    assert(!b.includes(CRange(-1000, -450)));
    assert(!b.includes(CRange(0, 500)));
    a += CRange(-10000, 10000) + CRange(10000000, 1000000000);
    assert(toString(a) == "{<-10000..10000>,<10000000..1000000000>}");
    b += a;
    assert(toString(b) == "{<-10000..10000>,<10000000..1000000000>}");
    b -= a;
    assert(toString(b) == "{}");
    b += CRange(0, 100) + CRange(200, 300) - CRange(150, 250) + CRange(160, 180) - CRange(170, 170);
    //    b.data.print();
    assert(toString(b) == "{<0..100>,<160..169>,<171..180>,<251..300>}");
    //    b.data.print();
    b -=
            CRange(10, 90) -
            CRange(20, 30) -
            CRange(40, 50) -
            CRange(60, 90) +
            CRange(70, 80);
    s = toString(b);
    assert(toString(b) == "{<0..9>,<20..30>,<40..50>,<60..69>,<81..100>,<160..169>,<171..180>,<251..300>}");
    CRangeList x{{5, 20},
                 {150, 200},
                 {-9, 12},
                 {48, 93}};
    assert(toString(x) == "{<-9..20>,<48..93>,<150..200>}");
    std::ostringstream oss;
    oss << std::setfill('=') << std::hex << std::left;
    for (const auto &v: x + CRange(-100, -100)) {
        //        std::cout << "-" << v << "-" << std::endl;
        oss << v << std::endl;
    }
    oss << std::setw(10) << 1024;
    //    std::cout << oss.str() << std::endl;
    assert(oss.str() == "-100\n<-9..20>\n<48..93>\n<150..200>\n400=======");

    // FIXME: I'll leave it a "stress test" for now
    for (long long x = 10; x <= 100; x *= 10) {
        CRangeList t;
        std::cout << x << "====" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        auto stop  = std::chrono::high_resolution_clock::now();

        for (long long i = -x; i < x; i += 2) {
            start = std::chrono::high_resolution_clock::now();
            t += CRange(i, i);
            stop = std::chrono::high_resolution_clock::now();
        }
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;


        for (long long i = -x; i < x; i += 2) {
            start = std::chrono::high_resolution_clock::now();
            t     = t + CRange(i, i);
            stop  = std::chrono::high_resolution_clock::now();
        }
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;

        // t.data.printS();

        for (long long i = -x; i < x; i += 2) {
            start = std::chrono::high_resolution_clock::now();
            assert(t.includes(i));
            stop = std::chrono::high_resolution_clock::now();
        }
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;


        t -= CRange(LLONG_MIN, LLONG_MAX);

        for (long long i = x; i > -x; i -= 2) {
            start = std::chrono::high_resolution_clock::now();
            t += CRange(i, i);
            stop = std::chrono::high_resolution_clock::now();
        }
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;


        for (long long i = x; i > -x; i -= 2) {
            start = std::chrono::high_resolution_clock::now();
            t     = t + CRange(i, i);
            stop  = std::chrono::high_resolution_clock::now();
        }
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;


        for (long long i = x; i > -x; i -= 2) {
            start = std::chrono::high_resolution_clock::now();
            assert(t.includes(i));
            stop = std::chrono::high_resolution_clock::now();
        }
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;

        for (long long i = x; i > -x; i -= 2) {
            start = std::chrono::high_resolution_clock::now();
            if (rand() % 100 > 50) {
                t += CRange(-rand(), rand());
            } else {
                t -= CRange(-rand(), rand());
            }
            stop = std::chrono::high_resolution_clock::now();
        }
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;
    }
}
