#ifndef POINTERS_H
#define POINTERS_H

#include <atomic>
#include <optional>
#include <utility>

#include "assert.h"

class SharedPtrTester;

template<typename T>
class UniquePtr {
public:
    UniquePtr() = default;

    template<class... Args>
    explicit UniquePtr(Args &&...args) : ptr(new T(std::forward<Args>(args)...)) {}

    explicit UniquePtr(T *data) : ptr(data) {}

    ~UniquePtr() {
        if (ptr == nullptr) return;
        delete ptr;
    }

    UniquePtr(UniquePtr const &other)            = delete;
    UniquePtr &operator=(UniquePtr const &other) = delete;

    UniquePtr(UniquePtr &&other) {
        delete ptr;
        ptr       = other.ptr;
        other.ptr = nullptr;
    }

    UniquePtr &operator=(UniquePtr &&other) {
        delete ptr;
        ptr       = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    T *operator->() const { return ptr; }

    T &operator*() const { return *ptr; }

    T *get() const noexcept { return ptr; }

    T *release() noexcept {
        auto b = ptr;
        ptr    = nullptr;
        return b;
    }

private:
    T *ptr = nullptr;
};

struct SharedPtr_Base {
    struct UsesBlock {
        int32_t _uses_ctl;
        int32_t _uses_obj;
    } __attribute__((packed));

    std::atomic<UsesBlock> _uses;

    static_assert(decltype(_uses)::is_always_lock_free);

    // Increments control block use counter
    void weak_lock() {
        UsesBlock old_uses = _uses.load();
        UsesBlock new_uses;
        do {
            assert(old_uses._uses_ctl >= 1);
            new_uses = old_uses;
            new_uses._uses_ctl++;
        } while (!_uses.compare_exchange_weak(old_uses, new_uses));
    }

    // Decrements control block use counter
    // Returns true if it was deleted
    bool weak_release() {
        UsesBlock old_uses = _uses.load();
        UsesBlock new_uses;
        do {
            new_uses = old_uses;
            new_uses._uses_ctl--;
        } while (!_uses.compare_exchange_weak(old_uses, new_uses));

        if (new_uses._uses_ctl == 0)
            delete this;
        return new_uses._uses_ctl == 0;
    }

    // Increments control and object use counter
    // Returns false if the object was already deleted
    bool strong_lock() {
        UsesBlock old_uses = _uses.load();
        UsesBlock new_uses;
        do {
            if (old_uses._uses_obj <= 0)
                return false;
            new_uses = old_uses;
            new_uses._uses_ctl++;
            new_uses._uses_obj++;
        } while (!_uses.compare_exchange_weak(old_uses, new_uses));

        assert(new_uses._uses_obj > 0);
        assert(new_uses._uses_ctl >= new_uses._uses_obj);
        return true;
    }

    // Decrements control and object use counter
    // Returns true if the object is to be deleted (it was the last reference)
    bool strong_release() {
        UsesBlock old_uses = _uses.load();
        UsesBlock new_uses;
        do {
            new_uses = old_uses;
            new_uses._uses_obj--;
            new_uses._uses_ctl--;
        } while (!_uses.compare_exchange_weak(old_uses, new_uses));

        if (new_uses._uses_ctl == 0)
            delete this;

        return new_uses._uses_obj == 0;
    }
};

template<typename T>
class SharedPtr {
    friend SharedPtrTester;

public:
    SharedPtr() = default;

    explicit SharedPtr(T *data) : _ptr(data), _base(new SharedPtr_Base{SharedPtr_Base::UsesBlock{1, 1}}) {}
    SharedPtr(std::nullptr_t a_nullptr) : _ptr(nullptr), _base(nullptr) {}

    ~SharedPtr() {
        unref();
    }

    SharedPtr(SharedPtr const &other) : _base(other._base), _ptr(other._ptr) {
        if (!_base) return;
        _base->strong_lock();
    }

    SharedPtr(SharedPtr &&other) {
        unref();

        _base       = other._base;
        _ptr        = other._ptr;
        other._base = nullptr;
        other._ptr  = nullptr;
    }

    SharedPtr &operator=(SharedPtr other) {
        std::swap(_base, other._base);
        std::swap(_ptr, other._ptr);
        return *this;
    }

    T *operator->() const {
        return _ptr;
    }

    T &operator*() const {
        return *_ptr;
    }

    T *get() const noexcept {
        return _ptr;
    }

    [[nodiscard]] int useCount() const {
        if (!_base) return 0;
        return _base->_uses.load()._uses_obj;
    }

    template<typename Tgt, template<class> class Ptr, typename Orig>
    friend Ptr<Tgt> static_ptr_cast(const Ptr<Orig> &ptr);

private:
    template<typename U>
    friend class WeakPtr;

    explicit SharedPtr(T *ptr, SharedPtr_Base *base) : _ptr(ptr), _base(base) {}

    void unref() {
        if (!_base) return;
        if (_base->strong_release())
            delete _ptr;
        _ptr  = nullptr;
        _base = nullptr;
    }

    T              *_ptr  = nullptr;
    SharedPtr_Base *_base = nullptr;
};


template<typename T>
class WeakPtr {
public:
    WeakPtr() = default;

    WeakPtr(const SharedPtr<T> &shared) : _base(shared._base), _ptr(shared._ptr) { _base->weak_lock(); }
    WeakPtr(std::nullptr_t a_nullptr) : _ptr(nullptr), _base(nullptr) {}

    ~WeakPtr() {
        unref();
    }

    WeakPtr(WeakPtr const &other) : _base(other._base), _ptr(other._ptr) {
        if (!_base) return;
        _base->weak_lock();
    }

    WeakPtr(WeakPtr &&other) {
        unref();

        _base       = other._base;
        _ptr        = other._ptr;
        other._base = nullptr;
        other._ptr  = nullptr;
    }

    WeakPtr &operator=(WeakPtr other) {
        std::swap(_ptr, other._ptr);
        std::swap(_base, other._base);
        return *this;
    }

    std::optional<SharedPtr<T>> lock() {
        if (!_base) return std::nullopt;
        if (_base->strong_lock())
            return SharedPtr(_ptr, _base);
        else
            return std::nullopt;
    }

    [[nodiscard]] int expired() const {
        if (!_base) return true;
        return _base->_uses.load()._uses_obj <= 0;
    }

    template<typename Tgt, template<class> class Ptr, typename Orig>
    friend Ptr<Tgt> static_ptr_cast(const Ptr<Orig> &ptr);

private:
    void unref() {
        if (!_base) return;
        _base->weak_release();
        _base = nullptr;
        _ptr  = nullptr;
    }

    T              *_ptr  = nullptr;
    SharedPtr_Base *_base = nullptr;
};

template<typename Tgt, template<class> class Ptr, typename Orig>
static Ptr<Tgt> static_ptr_cast(const Ptr<Orig> &ptr) {
    static_assert(std::is_convertible_v<Orig *, Tgt *> || std::is_base_of_v<Orig, Tgt>);
    if constexpr (std::is_same_v<Ptr<Tgt>, SharedPtr<Tgt>>) {
        ptr._base->strong_lock();
    } else if constexpr (std::is_same_v<Ptr<Tgt>, WeakPtr<Tgt>>) {
        ptr._base->weak_lock();
    } else {
        static_assert(false);
    }
    return Ptr<Tgt>(static_cast<Tgt *>(ptr._ptr), ptr._base);
}


class COWTester;

template<typename T>
class COWPointer {
private:
    friend COWTester;
    SharedPtr<T> ptr;

    void copy() {
        if (ptr.get() && ptr.useCount() > 1) {
            ptr = SharedPtr<T>(new T(*ptr));
        }
    }

public:
    COWPointer() = default;

    explicit COWPointer(T *data) : ptr(data) {}

    explicit COWPointer(SharedPtr<T> data) : ptr(std::move(data)) {}

    COWPointer(COWPointer &&other) = default;

    COWPointer(COWPointer const &data) = default;

    COWPointer &operator=(COWPointer other) {
        std::swap(ptr, other.ptr);
        return *this;
    }

    ~COWPointer() = default;

    T *get() const {
        return ptr.get();
    }

    T *getRW() {
        copy();
        return ptr.get();
    }

    int useCount() { return ptr.useCount(); };

    const T &operator*() const {
        return *ptr;
    }

    const T *operator->() const {
        return ptr.operator->();
    }
};


#endif