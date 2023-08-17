#if !defined(MATH_VECTOR_H)
#include "defines.h"

struct vec2
{
    union
    {
        struct
        {
            f32 x, y;
        };

        struct
        {
            f32 u, v;
        };
    };

    inline vec2 operator+=(vec2 A);
    inline vec2 operator-=(vec2 A);
    inline vec2 operator*=(f32 A);
    inline vec2 operator/=(vec2 A);
    inline vec2 operator/=(f32 A);
};

inline vec2 Vec2(float x, float y);
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

    };

    inline vec3 operator+=(vec3 A);
    inline vec3 operator-=(vec3 A);
    inline vec3 operator*=(f32 A);
    inline vec3 operator/=(vec3 A);
    inline vec3 operator/=(f32 A);
};

inline vec3 Vec3(int x, int y, int z);
inline vec3 Vec3(vec2 xy, int z);
inline vec3 ToVec3(vec2 A);
inline vec2 ToVec2(vec3 A);

inline f32 Dot(vec3 A, vec3 B);

inline vec3 operator+(vec3 A, vec3 B);
inline vec3 operator-(vec3 A, vec3 B);
inline vec3 operator*(vec3 A, f32 B);
inline vec3 operator/(vec3 A, vec3 B);
inline vec3 operator/(vec3 A, f32 B);

#define MATH_VECTOR_H
#endif
