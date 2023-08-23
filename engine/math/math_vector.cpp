#include "math_vector.h"

#if 0
// ----------------------------------------------------------------------------------------------------------------
// Vec2
// ----------------------------------------------------------------------------------------------------------------
template<typename T>
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
    T Result = A.x*B.x + A.y*B.y;
    return Result;
}

template<typename T>
vec2<T>
operator+(vec2<T> A, vec2<T> B)
{
    vec2<T> Result = { A.x + B.x, A.y + B.y };
    return Result;
}

template<typename T>
vec2<T>
operator-(vec2<T> A, vec2<T> B)
{
    vec2<T> Result = {A.x - B.x, A.y - B.y};
    return Result;
}

template<typename T>
vec2<T>
operator*(vec2<T> A, T B)
{
    vec2<T> Result = {B*A.x, B*A.y};
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
#endif

