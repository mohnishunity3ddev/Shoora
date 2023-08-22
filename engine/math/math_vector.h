#if !defined(MATH_VECTOR_H)
#include "defines.h"

struct vec2
{
    union
    {
        struct { f32 x, y; };
        struct { f32 u, v; };
        f32 E[2];
    };

    inline vec2 operator+=(vec2 A);
    inline vec2 operator-=(vec2 A);
    inline vec2 operator*=(f32 A);
    inline vec2 operator/=(vec2 A);
    inline vec2 operator/=(f32 A);
};

inline vec2 Vec2(f32 x, f32 y);
inline f32 Dot(vec2 A, vec2 B);

inline vec2 operator+(vec2 A, vec2 B);
inline vec2 operator-(vec2 A, vec2 B);
inline vec2 operator*(vec2 A, f32 B);
inline vec2 operator/(vec2 A, vec2 B);
inline vec2 operator/(vec2 A, f32 B);

struct vec3
{
    union
    {
        struct { f32 x, y, z; };
        struct { f32 u, v, w; };
        struct { f32 r, g, b; };
        struct { vec2 xy; f32 Reserved_0; };
        struct { f32 Reserved_1; vec2 yz; };
        struct { vec2 uv; f32 Reserved_2; };
        struct { f32 Reserved_3; vec2 vw; };
        struct { vec2 rg; f32 Reserved_4; };
        struct { f32 Reserved_5; vec2 gb; };
        f32 E[3];
    };

    inline vec3 operator+=(vec3 A);
    inline vec3 operator-=(vec3 A);
    inline vec3 operator*=(f32 A);
    inline vec3 operator/=(vec3 A);
    inline vec3 operator/=(f32 A);
};

inline vec3 Vec3(f32 x, f32 y, f32 z);
inline vec3 Vec3(vec2 xy, f32 z);
inline vec3 ToVec3(vec2 A);
inline vec2 ToVec2(vec3 A);

inline f32 Dot(vec3 A, vec3 B);

inline vec3 operator+(vec3 A, vec3 B);
inline vec3 operator-(vec3 A, vec3 B);
inline vec3 operator*(vec3 A, f32 B);
inline vec3 operator/(vec3 A, vec3 B);
inline vec3 operator/(vec3 A, f32 B);

// ----------------------------------------------------------------------------------------------------------------
// Vec2
// ----------------------------------------------------------------------------------------------------------------
inline vec2
Vec2(f32 x, f32 y)
{
    vec2 Result;
    Result.x = x;
    Result.y = y;
    return Result;
}

inline f32
Dot(vec2 A, vec2 B)
{
    f32 Result = A.x * B.x + A.y * B.y;
    return Result;
}

inline vec2
operator+(vec2 A, vec2 B)
{
    vec2 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return Result;
}

inline vec2
operator-(vec2 A, vec2 B)
{
    vec2 Result;
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return Result;
}

inline vec2
operator*(vec2 A, f32 B)
{
    vec2 Result;
    Result.x = B * A.x;
    Result.y = B * A.y;
    return Result;
}

inline vec2
operator/(vec2 A, vec2 B)
{
    vec2 Result;

    ASSERT(B.x != 0.0f && B.y != 0.0f);
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;

    return Result;
}

inline vec2
operator/(vec2 A, f32 B)
{
    vec2 Result;
    ASSERT(B != 0.0f);

    Result.x = A.x / B;
    Result.y = A.y / B;

    return Result;
}

inline vec2
vec2::operator+=(vec2 A)
{
    *this = *this + A;
    return *this;
}

inline vec2
vec2::operator-=(vec2 A)
{
    *this = *this - A;
    return *this;
}

inline vec2
vec2::operator*=(f32 A)
{
    *this = *this * A;
    return *this;
}

inline vec2
vec2::operator/=(vec2 A)
{
    *this = *this / A;
    return *this;
}

inline vec2
vec2::operator/=(f32 A)
{
    *this = *this / A;
    return *this;
}

// ----------------------------------------------------------------------------------------------------------------
// Vec3
// ----------------------------------------------------------------------------------------------------------------
inline vec3
Vec3(f32 x, f32 y, f32 z)
{
    vec3 Result;

    Result.x = x;
    Result.y = y;
    Result.z = z;

    return Result;
}

inline vec3
Vec3(vec2 xy, f32 z)
{
    vec3 Result = Vec3(xy.x, xy.y, z);

    return Result;
}

inline vec3
ToVec3(vec2 A)
{
    vec3 Result = Vec3(A.x, A.y, 0.0f);
    return Result;
}

inline vec2
ToVec2(vec3 A)
{
    vec2 Result = Vec2(A.x, A.y);
    return Result;
}

inline f32
Dot(vec3 A, vec3 B)
{
    f32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
    return Result;
}

inline vec3
operator+(vec3 A, vec3 B)
{
    vec3 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return Result;
}

inline vec3
operator-(vec3 A, vec3 B)
{
    vec3 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return Result;
}

inline vec3
operator*(vec3 A, f32 B)
{
    vec3 Result;

    Result.x = B * A.x;
    Result.y = B * A.y;
    Result.z = B * A.z;

    return Result;
}

inline vec3
operator/(vec3 A, vec3 B)
{
    vec3 Result;

    ASSERT(B.x != 0.0f && B.y != 0.0f && B.z != 0.0f);

    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;

    return Result;
}

inline vec3
operator/(vec3 A, f32 B)
{
    vec3 Result;

    ASSERT(B != 0.0f);

    Result.x = A.x / B;
    Result.y = A.y / B;
    Result.z = A.z / B;

    return Result;
}

inline vec3
vec3::operator+=(vec3 A)
{
    *this = *this + A;
    return *this;
}

inline vec3
vec3::operator-=(vec3 A)
{
    *this = *this - A;
    return *this;
}

inline vec3
vec3::operator*=(f32 A)
{
    *this = *this * A;
    return *this;
}

inline vec3
vec3::operator/=(vec3 A)
{
    *this = *this / A;
    return *this;
}

inline vec3
vec3::operator/=(f32 A)
{
    *this = *this / A;
    return *this;
}

#define MATH_VECTOR_H
#endif
