#pragma once

#include <cstdint>
#include <unordered_map>

namespace DStream
{
    template<typename T>
    struct _vec3
    {
        T x;
        T y;
        T z;

        _vec3(uint8_t X, uint8_t Y, uint8_t Z) : x(X), y(Y), z(Z) {}
        _vec3() = default;

        inline T& operator [](int idx)
        {
            switch (idx)
            {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            }
        }

        inline const T operator [](int idx) const
        {
            switch (idx)
            {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            }
        }

        inline bool operator==(const _vec3& other) const
        {
            return other.x == x && other.y == y && other.z == z;
        }

        inline bool operator!=(const _vec3& other) const
        {
            return other.x != x || other.y != y || other.z != z;
        }
    };

    typedef struct _vec3<uint8_t>   Color;
    typedef struct _vec3<float>     Vec3;
}

namespace std
{
    template<>
    struct hash<DStream::Vec3>
    {
        std::size_t operator()(const DStream::Vec3& k) const
        {
            using std::size_t;
            using std::hash;

            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            return ((hash<float>()(k.x)
                ^ (hash<float>()(k.y) << 1)) >> 1)
                ^ (hash<float>()(k.z) << 1);
        }
    };

    template<>
    struct hash<DStream::Color>
    {
        std::size_t operator()(const DStream::Color& k) const
        {
            using std::size_t;
            using std::hash;

            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            return (((size_t)k.x) << 16) | (((size_t)k.y)) | ((size_t)k.x);
        }
    };
}