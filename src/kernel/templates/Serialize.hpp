//
// Created by Stepan Usatiuk on 15.04.2023.
//

#ifndef SEMBACKUP_SERIALIZE_H
#define SEMBACKUP_SERIALIZE_H

#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "Vector.hpp"

//#ifdef __APPLE__
//#include <machine/endian.h>
//#define htobe64(x) htonll(x)
//#define be64toh(x) ntohll(x)
//#else
//#include <endian.h>
//#endif


/// Serialization library
/**
 *  To serialize the objects in Repository, we have to handle a couple of cases:
 *  1. Serializing integers (object ids, etc...)
 *  2. Serializing enums (object types)
 *  3. Serializing char vectors and strings
 *  4. Serializing other STL containers (which also requires serializing pairs)
 *  5. Serializing custom structs (including the objects themselves)
 *
 *  With this library it is possible to do all of that.
 *  One problem is that it isn't really portable, but it can be fixed by changing the std::is_integral<T>::value case to use something like be64toh/htobe64
 *
 */
namespace Serialize {
    template<typename, typename = void, typename = void>
    struct is_pair : std::false_type {};

    template<typename P>
    struct is_pair<P, std::void_t<decltype(std::declval<P>().first)>, std::void_t<decltype(std::declval<P>().second)>>
        : std::true_type {};

    template<typename, typename, typename = void>
    struct has_emplace_back : std::false_type {};

    template<typename T, typename V>
    struct has_emplace_back<T, V, std::void_t<decltype(T().emplace_back(std::declval<V>()))>> : std::true_type {};

    template<typename, typename = void, typename = void>
    struct serializable : std::false_type {};

    /// Checks if the object has the `serializable` type
    /// In that case, its serialization will be delegated to its .serialize() parameter,
    /// and deserialization to its T(char vector iterator in, const char vector iterator end) constructor,
    /// similar to Serialize::deserialize
    template<typename T>
    struct serializable<T, std::void_t<decltype(T::serializable::value)>> : std::true_type {};

    /// Deserializes object of type \p T starting from fist byte \p in, advances the iterator past the end of object
    /// \tparam T   Type to deserialize
    /// \param in   Iterator to the first byte of the object
    /// \param end  End iterator of source container
    /// \return     Deserialized value
    template<typename T, typename C = Vector<char>>
    static std::optional<T> deserialize(typename C::const_iterator &in, const typename C::const_iterator &end);

    /// Serializes object of type \p T into vector \p out
    /// \tparam T   Type to serialize
    /// \param what Constant reference to the serialized object
    /// \param out  Reference to output vector
    template<typename T, typename C = Vector<char>>
    static void serialize(const T &what, C &out);

    /// Serializes the object of type \p T and returns the resulting vector
    /// \tparam T   Type to serialize
    /// \param o    Constant reference to the serialized object
    /// \return     Serialized data
    template<typename T, typename C = Vector<char>>
    static C serialize(const T &o);

    /// Deserializes object of type \p T from input vector \p from
    /// \tparam T   Type to deserialize
    /// \param from Constant reference to the serialized object
    /// \return     Deserialized value
    template<typename T, typename C = Vector<char>>
    static std::optional<T> deserialize(const C &from);

    template<typename T, typename C>
    std::optional<T> deserialize(typename C::const_iterator &in, const typename C::const_iterator &end) {
        if (in >= end) return std::nullopt;

        if constexpr (serializable<T>::value) {
            // If the object declares itself as serializable, call its constructor with in and end
            return T(in, end);
        } else if constexpr (is_pair<T>::value) {
            // If the object is pair, deserialize the first and second element and return the pair
            using KT = typename std::remove_const<decltype(T::first)>::type;
            using VT = typename std::remove_const<decltype(T::second)>::type;
            auto K   = deserialize<KT>(in, end);
            auto V   = deserialize<VT>(in, end);
            return T(std::move(K), std::move(V));
        } else if constexpr (std::is_enum<T>::value) {
            // If the object is an enum, deserialize an int and cast it to the enum
            auto tmp = deserialize<uint32_t>(in, end);
            if (tmp >= 0 && tmp < static_cast<uint32_t>(T::END)) return static_cast<T>(tmp);
            else
                return std::nullopt;
        } else if constexpr (sizeof(T) == 1) {
            // If it's a single byte, just copy it
            if (std::distance(in, end) < sizeof(T)) return std::nullopt;
            return *(in++);
        } else if constexpr (std::is_integral<T>::value) {
            T tmp;
            // If the object is a number, copy it byte-by-byte
            if (std::distance(in, end) < sizeof(tmp)) return std::nullopt;

            std::copy(in, in + sizeof(tmp), reinterpret_cast<char *>(&tmp));
            in += sizeof(tmp);
            return static_cast<T>(tmp);
        } else {
            // Otherwise we treat it as a container, in format of <number of elements>b<elements>e
            auto size = deserialize<size_t>(in, end);
            if (!size) return std::nullopt;

            auto b = deserialize<char>(in, end);
            if (!b || *b != 'b') return std::nullopt;

            T out;
            if constexpr (sizeof(typename T::value_type) == 1) {
                // Optimization for char vectors
                if (std::distance(in, end) < *size) return std::nullopt;
                out.insert(out.end(), in, in + *size);
                in += *size;
            } else
                for (size_t i = 0; i < *size; i++) {
                    using V = typename T::value_type;
                    V v     = deserialize<V>(in, end);
                    // Try either emplace_back or emplace if it doesn't exist
                    if constexpr (has_emplace_back<T, V>::value) out.emplace_back(std::move(v));
                    else
                        out.emplace(std::move(v));
                }

            b = deserialize<char>(in, end);
            if (!b || *b != 'e') return std::nullopt;

            return out;
        }
    }

    template<typename T, typename C>
    void serialize(const T &what, C &out) {
        if constexpr (serializable<T>::value) {
            // If the object declares itself as serializable, call its serialize method
            what.serialize(out);
        } else if constexpr (is_pair<T>::value) {
            // If the object is pair, serialize the first and second element
            serialize(what.first, out);
            serialize(what.second, out);
        } else if constexpr (std::is_enum<T>::value) {
            // If the object is an enum, cast it to an int and serialize that
            serialize(static_cast<uint32_t>(what), out);
        } else if constexpr (sizeof(T) == 1) {
            // If it's a single byte, just copy it
            out.push_back(what);
        } else if constexpr (std::is_integral<T>::value) {
            // If the object is a number, copy it byte-by-byte
            T tmp = static_cast<T>(what);
            out.insert(out.end(), (reinterpret_cast<const char *>(&tmp)),
                       (reinterpret_cast<const char *>(&tmp) + sizeof(tmp)));
        } else {
            // Otherwise we treat it as a container, in format of <number of elements>b<elements>e
            serialize(what.size(), out);
            serialize('b', out);
            if constexpr (sizeof(typename T::value_type) == 1) {
                // Optimization for char vectors
                out.insert(out.end(), what.begin(), what.end());
            } else
                for (auto const &i: what) { serialize(i, out); }
            serialize('e', out);
        }
    }

    template<typename T, typename C>
    C serialize(const T &o) {
        C out;
        serialize(o, out);
        return out;
    }

    template<typename T, typename C>
    std::optional<T> deserialize(const C &from) {
        auto bgwr = from.begin();
        return deserialize<T>(bgwr, from.end());
    }
} // namespace Serialize

#endif //SEMBACKUP_SERIALIZE_H
