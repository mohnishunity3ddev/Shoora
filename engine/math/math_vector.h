#if !defined(MATH_VECTOR_H)
#include "defines.h"

template <typename T>
struct vec2
{
    union
    {
        struct { T x, y; };
        struct { T u, v; };
        struct { T Width, Height; };
        f32 E[2];
    };

    inline vec2<T> operator+=(vec2<T> A);
    inline vec2<T> operator-=(vec2<T> A);
    inline vec2<T> operator*=(T A);
    inline vec2<T> operator/=(vec2<T> A);
    inline vec2<T> operator/=(T A);
};

// ----------------------------------------------------------------------------------------------------------------
// Vec2
// ----------------------------------------------------------------------------------------------------------------
template <typename T>
vec2<T>
Vec2(T x, T y)
{
    vec2<T> Result;
    Result.x = x;
    Result.y = y;
    return Result;
}

template <typename T>
T
Dot(vec2<T> A, vec2<T> B)
{
    T Result = A.x * B.x + A.y * B.y;
    return Result;
}

template <typename T>
vec2<T>
operator+(vec2<T> A, vec2<T> B)
{
    vec2<T> Result = {A.x + B.x, A.y + B.y};
    return Result;
}

template <typename T>
vec2<T>
operator-(vec2<T> A, vec2<T> B)
{
    vec2<T> Result = {A.x - B.x, A.y - B.y};
    return Result;
}

template <typename T>
vec2<T>
operator*(vec2<T> A, T B)
{
    vec2<T> Result = {B * A.x, B * A.y};
    return Result;
}

template <typename T>
vec2<T>
operator/(vec2<T> A, vec2<T> B)
{
    vec2<T> Result;

    ASSERT(B.x != 0.0f && B.y != 0.0f);
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;

    return Result;
}

template <typename T>
vec2<T>
operator/(vec2<T> A, T B)
{
    vec2<T> Result;
    ASSERT(B != 0.0f);

    Result.x = A.x / B;
    Result.y = A.y / B;

    return Result;
}

template <typename T>
vec2<T>
vec2<T>::operator+=(vec2<T> A)
{
    *this = *this + A;
    return *this;
}

template <typename T>
vec2<T>
vec2<T>::operator-=(vec2<T> A)
{
    *this = *this - A;
    return *this;
}

template <typename T>
vec2<T>
vec2<T>::operator*=(T A)
{
    *this = *this * A;
    return *this;
}

template <typename T>
vec2<T>
vec2<T>::operator/=(vec2<T> A)
{
    *this = *this / A;
    return *this;
}

template <typename T>
vec2<T>
vec2<T>::operator/=(T A)
{
    *this = *this / A;
    return *this;
}
typedef vec2<i32> vec2i;
typedef vec2<f32> vec2f;
typedef vec2<u32> vec2u;

template<typename T>
struct vec3
{
    union
    {
        struct { T x, y, z; };
        struct { T u, v, w; };
        struct { T r, g, b; };
        struct { T Width, Height, Depth; };
        struct { vec2<T> xy; T Reserved_0; };
        struct { T Reserved_1; vec2<T> yz; };
        struct { vec2<T> uv; T Reserved_2; };
        struct { T Reserved_3; vec2<T> vw; };
        struct { vec2<T> rg; T Reserved_4; };
        struct { T Reserved_5; vec2<T> gb; };
        T E[3];
    };

    inline vec3<T> operator+=(vec3<T> A);
    inline vec3<T> operator-=(vec3<T> A);
    inline vec3<T> operator*=(T A);
    inline vec3<T> operator/=(vec3<T> A);
    inline vec3<T> operator/=(T A);
};

// ----------------------------------------------------------------------------------------------------------------
// Vec3
// ----------------------------------------------------------------------------------------------------------------
template <typename T>
vec3<T>
Vec3(T x, T y, T z)
{
    vec3<T> Result = {x, y, z};
    return Result;
}

template <typename S, typename T>
vec3<T>
Vec3(S x, S y, S z)
{
    vec3<T> Result = {(T)x, (T)y, (T)z};
    return Result;
}

template <typename T>
vec3<T>
Vec3(vec2<T> xy, T z)
{
    vec3<T> Result = Vec3(xy.x, xy.y, z);

    return Result;
}

template <typename T>
vec3<T>
ToVec3(vec2<T> A)
{
    vec3<T> Result = Vec3(A.x, A.y, 0.0f);
    return Result;
}

template <typename T>
vec2<T>
ToVec2(vec3<T> A)
{
    vec2<T> Result = Vec2(A.x, A.y);
    return Result;
}

template <typename T>
T
Dot(vec3<T> A, vec3<T> B)
{
    T Result = A.x * B.x + A.y * B.y + A.z * B.z;
    return Result;
}

template <typename T>
vec3<T>
operator+(vec3<T> A, vec3<T> B)
{
    vec3<T> Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return Result;
}

template <typename T>
vec3<T>
operator-(vec3<T> A, vec3<T> B)
{
    vec3<T> Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return Result;
}

template <typename T>
vec3<T>
operator*(vec3<T> A, T B)
{
    vec3<T> Result;

    Result.x = B * A.x;
    Result.y = B * A.y;
    Result.z = B * A.z;

    return Result;
}

template <typename T>
vec3<T>
operator/(vec3<T> A, vec3<T> B)
{
    vec3<T> Result;

    ASSERT(B.x != 0.0f && B.y != 0.0f && B.z != 0.0f);

    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;

    return Result;
}

template <typename T>
vec3<T>
operator/(vec3<T> A, T B)
{
    vec3<T> Result;

    ASSERT(B != 0.0f);

    Result.x = A.x / B;
    Result.y = A.y / B;
    Result.z = A.z / B;

    return Result;
}

template <typename T>
vec3<T>
vec3<T>::operator+=(vec3<T> A)
{
    *this = *this + A;
    return *this;
}

template <typename T>
vec3<T>
vec3<T>::operator-=(vec3<T> A)
{
    *this = *this - A;
    return *this;
}

template <typename T>
vec3<T>
vec3<T>::operator*=(T A)
{
    *this = *this * A;
    return *this;
}

template <typename T>
vec3<T>
vec3<T>::operator/=(vec3<T> A)
{
    *this = *this / A;
    return *this;
}

template <typename T>
vec3<T>
vec3<T>::operator/=(T A)
{
    *this = *this / A;
    return *this;
}

typedef vec3<i32> vec3i;
typedef vec3<f32> vec3f;
typedef vec3<u32> vec3u;

#define MATH_VECTOR_H
#endif
