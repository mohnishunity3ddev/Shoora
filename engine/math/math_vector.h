#if !defined(MATH_VECTOR_H)
#include "defines.h"
#include <platform/platform.h>
#include "math_trig.h"
#include <cmath>

namespace shu
{
    template <typename T> struct vec3;

    template <typename T>
    struct vec2
    {
        union
        {
            struct { T x, y; };
            struct { T u, v; };
            struct { T w, h; };
            f32 E[2];
        };

        inline vec2<T> operator+=(const vec2<T>& A);
        inline vec2<T> operator-=(const vec2<T>& A);
        inline vec2<T> operator*=(T A);
        inline vec2<T> operator/=(const vec2<T>& A);
        inline vec2<T> operator/=(T A);
        inline vec2<T> Normal() const;
        inline T& operator[](size_t Index);
        inline T Dot(const vec2<T>& A);
        inline T Cross(const vec2<T>& A) const;
        inline T SqMagnitude();
        inline T Magnitude() const;
        inline vec2<T> Rotate(f32 AngleRadians) const;
        inline f32 GetSlopeAngleInDegrees();
        inline b32 IsValid() const;
        static vec2<T> Zero()
        {
            vec2<T> Result = {(T)0, (T)0};
            return Result;
        }
    };

    typedef vec2<i32> vec2i;
    typedef vec2<f32> vec2f;
    typedef vec2<u32> vec2u;
    #define Vec2f Vec2<f32>
    #define Vec2u Vec2<u32>
    #define Vec2i Vec2<i32>

    template <typename T> SHU_EXPORT vec2<T> Vec2();
    template <typename T> SHU_EXPORT vec2<T> Vec2(T A);
    template <typename T> SHU_EXPORT vec2<T> Vec2(T x, T y);
    template <typename T> SHU_EXPORT vec2<T> Vec2(vec3<T> Vec3);
    template <typename T> SHU_EXPORT vec2<T> MakeVec2(const T *const Ptr);
    template <typename T> SHU_EXPORT T Dot(const vec2<T>& A, const vec2<T>& B);
    template <typename T> SHU_EXPORT vec2<T> operator+(const vec2<T>& A, const vec2<T>& B);
    template <typename T> SHU_EXPORT vec2<T> operator-(const vec2<T>& A, const vec2<T>& B);
    template <typename T> SHU_EXPORT vec2<T> operator-(const vec2<T>& A);
    template <typename T> SHU_EXPORT vec2<T> operator*(const vec2<T>& A, T B);
    template <typename T> SHU_EXPORT vec2<T> operator*(T A, const vec2<T> &B);
    template <typename T> SHU_EXPORT vec2<T> operator/(const vec2<T>& A, const vec2<T>& B);
    template <typename T> SHU_EXPORT vec2<T> operator/(const vec2<T>& A, T B);
    template <typename T> SHU_EXPORT T SqMagnitude(const vec2<T> &A);
    template <typename T> SHU_EXPORT T Magnitude(const vec2<T> &A);
    template <typename T> SHU_EXPORT vec2f Normalize(const vec2<T> &A);

    // ----------------------------------------------------------------------------------------------------------------

    template<typename T>
    struct vec3
    {
        union
        {
            struct { T x, y, z; };
            struct { T u, v, w_; };
            struct { T r, g, b; };
            struct { T w, h, d; };
            struct { vec2<T> xy; T Unused_0; };
            struct { T Unused_1; vec2<T> yz; };
            struct { vec2<T> uv; T Unused_2; };
            struct { T Unused_3; vec2<T> vw; };
            struct { vec2<T> rg; T Unused_4; };
            struct { T Unused_5; vec2<T> gb; };
            T E[3];
        };

        inline vec3<T> operator+=(const vec3<T>& A);
        inline vec3<T> operator-=(const vec3<T>& A);
        inline vec3<T> operator*=(T A);
        inline vec3<T> operator*=(const vec3<T> &A);
        inline vec3<T> operator/=(const vec3<T>& A);
        inline vec3<T> operator/=(T A);
        inline b32 operator==(const vec3<T> &Rhs) const;
        inline b32 operator!=(const vec3<T> &Rhs) const;
        inline T& operator[](size_t Index);
        inline T operator[](size_t Index) const;
        inline T SqMagnitude() const;
        inline T Magnitude() const;
        inline void Normalize();
        inline T Dot(const vec3<T> &A) const;
        inline vec3<T> Cross(const vec3<T> &A) const;
        inline vec3<T> Reciprocal() const;
        inline void GetOrtho(vec3<T> &U, vec3<T> &V, vec3<T> up = {0, 1, 0}) const;

        inline b32 IsValid() const;
        inline b32 IsNormalized() const;
        inline b32 IsZero() const;

        inline void ZeroOut();
        static vec3<T>
        Zero()
        {
            vec3<T> Result = {(T)0, (T)0, (T)0};
            return Result;
        }

        static vec3<T>
        Project(const vec3<T> &ProjV, const vec3<T> &Axis)
        {
            T dot = ProjV.Dot(Axis);
            vec3<T> n = shu::Normalize(Axis);
            return dot * n;
        }

        static f32
        Angle(const shu::vec3<T> &v1, const shu::vec3<T> &v2)
        {
            f32 cosTheta = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
            f32 magnitudeMult = sqrtf(v1.SqMagnitude() * v2.SqMagnitude());

            if (NearlyEqual(magnitudeMult, 0.0f, 1e-6))
            {
                return 0.0f;
            }

            cosTheta = ClampToRange(cosTheta / magnitudeMult, -1.0f, 1.0f);

            f32 Result = CosInverse(cosTheta);
            return Result;
        }

        static f32
        SignedAngle(const vec3<T> &from, const vec3<T> &to, const vec3<T> axis)
        {
            f32 uAngle = Angle(from, to);
            f32 crossX = from.y * to.z - from.z * to.y;
            f32 crossY = from.z * to.x - from.x * to.z;
            f32 crossZ = from.x * to.y - from.y * to.x;
            f32 sign = SIGN(axis.x*crossX + axis.y*crossY + axis.z*crossZ);
            return uAngle * sign;
        }
    };
    typedef vec3<i32> vec3i;
    typedef vec3<f32> vec3f;
    typedef vec3<u32> vec3u;
    #define Vec3f Vec3<f32>
    #define Vec3u Vec3<u32>
    #define Vec3i Vec3<i32>

    template <typename T> SHU_EXPORT vec3<T> Vec3();
    template <typename T> SHU_EXPORT vec3<T> Vec3(T A);
    template <typename T> SHU_EXPORT vec3<T> Vec3(T x, T y, T z);
    template <typename T> SHU_EXPORT vec3<T> Vec3(const vec2<T> &V);
    template <typename T> SHU_EXPORT vec3<T> MakeVec3(const T *const Ptr);
    template <typename T> SHU_EXPORT vec3<T> Vec3(const vec2<T>& xy, T z);
    template <typename T> SHU_EXPORT vec3<T> ToVec3(const vec2<T>& A);
    template <typename T> SHU_EXPORT vec2<T> ToVec2(const vec3<T>& A);
    template <typename T> SHU_EXPORT T Dot(const vec3<T>& A, const vec3<T>& B);
    template <typename T> SHU_EXPORT vec3<T> Cross(const vec3<T>& A, const vec3<T>& B);
    template <typename T> SHU_EXPORT vec3<T> operator+(const vec3<T>& A, const vec3<T>& B);
    template <typename T> SHU_EXPORT vec3<T> operator-(const vec3<T>& A, const vec3<T>& B);
    template <typename T> SHU_EXPORT vec3<T> operator-(const vec3<T>& A);
    template <typename T> SHU_EXPORT vec3<T> operator*(const vec3<T>& A, T B);
    template <typename T> SHU_EXPORT vec3<T> operator*(T A, const vec3<T> &B);
    template <typename T> SHU_EXPORT vec3<T> operator*(const vec3<T> &A, const vec3<T> &B);
    template <typename T> SHU_EXPORT vec3<T> operator/(const vec3<T>& A, const vec3<T>& B);
    template <typename T> SHU_EXPORT vec3<T> operator/(const vec3<T>& A, T B);
    template <typename T> SHU_EXPORT T SqMagnitude(const vec3<T> &A);
    template <typename T> SHU_EXPORT T Magnitude(const vec3<T> &A);
    // NOTE: Exaluates A.(B X C)
    template <typename T> SHU_EXPORT T ScalarTripleProduct(const vec3<T> &A, const vec3<T> &B, const vec3<T> &C);
    template <typename T> SHU_EXPORT vec3f Normalize(const vec3<T> &A);

    // ----------------------------------------------------------------------------------------------------------------

    template <typename T>
    struct vec4
    {
        union
        {
            struct { T x, y, z, w; };
            struct { T r, g, b, a; };
            struct { T u, v, s, t; };
            struct { vec2<T> xy; vec2<T> zw; };
            struct { vec2<T> rg; vec2<T> ba; };
            struct { vec2<T> uv; vec2<T> st; };
            struct { vec3<T> xyz; T Reserved_0; };
            struct { vec3<T> rgb; T Reserved_1; };
            T E[4];
        };

        inline vec4<T> operator+=(const vec4<T>& A);
        inline vec4<T> operator-=(const vec4<T>& A);
        inline vec4<T> operator*=(T A);
        inline vec4<T> operator/=(const vec4<T>& A);
        inline vec4<T> operator/=(T A);
        inline b32 operator==(const vec4<T>& rhs) const;

        inline T &operator[](size_t Index);
        inline T operator[](size_t Index) const;
        inline void ZeroOut();
        inline b32 IsValid() const;
    };

    template <typename T> SHU_EXPORT vec4<T> Vec4();
    template <typename T> SHU_EXPORT vec4<T> Vec4(T A);
    template <typename T> SHU_EXPORT vec4<T> Vec4(T x, T y, T z, T w);
    template <typename T> SHU_EXPORT vec4<T> Vec4(const vec3<T> &v, f32 w);
    template <typename T> SHU_EXPORT vec4<T> MakeVec4(const T *const Ptr);
    template <typename T> SHU_EXPORT T Dot(const vec4<T> &A, const vec4<T> &B);
    template <typename T> SHU_EXPORT vec4<T> operator+(const vec4<T>& A, const vec4<T>& B);
    template <typename T> SHU_EXPORT vec4<T> operator-(const vec4<T>& A, const vec4<T>& B);
    template <typename T> SHU_EXPORT vec4<T> operator*(const vec4<T>& A, const vec4<T>& B);
    template <typename T> SHU_EXPORT vec4<T> operator*(const vec4<T>& A, T B);
    template <typename T> SHU_EXPORT vec4<T> operator/(const vec4<T>& A, const vec4<T>& B);
    template <typename T> SHU_EXPORT vec4<T> operator/(const vec4<T>& A, T B);

    typedef vec4<i32> vec4i;
    typedef vec4<f32> vec4f;
    typedef vec4<u32> vec4u;
    #define Vec4f Vec4<f32>
    #define Vec4u Vec4<u32>
    #define Vec4i Vec4<i32>

    template <typename T>
    struct vec6
    {
        union
        {
            struct { T x1, y1, z1, x2, y2, z2; };
            struct { vec3<T> xyz1; vec3<T> xyz2; };
            T E[6];
        };

        inline T Dot(const vec6<T>& A);

        inline vec6<T> operator+=(const vec6<T>& A);
        inline vec6<T> operator+=(T A);
        inline vec6<T> operator-=(const vec6<T>& A);
        inline vec6<T> operator-=(T A);

        inline vec6<T> operator*=(T A);
        inline vec6<T> operator/=(const vec6<T>& A);
        inline vec6<T> operator/=(T A);
        inline T &operator[](size_t Index);
        inline const T &operator[](size_t Index) const;
        inline b32 IsValid() const;
    };

    template <typename T> SHU_EXPORT vec6<T> Vec6();
    template <typename T> SHU_EXPORT vec6<T> Vec6(T A);
    template <typename T> SHU_EXPORT vec6<T> Vec6(T x1, T y1, T z1, T x2, T y2, T z2);
    template <typename T> SHU_EXPORT vec6<T> MakeVec6(const T *const Ptr);
    template <typename T> SHU_EXPORT T Dot(const vec6<T> &A, const vec6<T> &B);
    template <typename T> SHU_EXPORT vec6<T> operator+(const vec6<T>& A, const vec6<T>& B);
    template <typename T> SHU_EXPORT vec6<T> operator+(const vec6<T>& A, T B);
    template <typename T> SHU_EXPORT vec6<T> operator-(const vec6<T>& A, const vec6<T>& B);
    template <typename T> SHU_EXPORT vec6<T> operator-(const vec6<T>& A, T B);
    template <typename T> SHU_EXPORT vec6<T> operator*(const vec6<T>& A, const vec6<T>& B);
    template <typename T> SHU_EXPORT vec6<T> operator*(const vec6<T>& A, T B);
    template <typename T> SHU_EXPORT vec6<T> operator/(const vec6<T>& A, const vec6<T>& B);
    template <typename T> SHU_EXPORT vec6<T> operator/(const vec6<T>& A, T B);

    typedef vec6<i32> vec6i;
    typedef vec6<f32> vec6f;
    typedef vec6<u32> vec6u;
    #define Vec6f Vec6<f32>
    #define Vec6u Vec6<u32>
    #define Vec6i Vec6<i32>

    template <typename T, size_t N>
    struct vecN
    {
        T Data[N];

        vecN();
        template <typename... Args>
        vecN(Args... args);
        vecN(const vecN &rhs);
        vecN &operator=(const vecN &rhs);
        ~vecN();
        i32 getSize() { return N; }

        T operator[](const i32 idx) const;
        T &operator[](const i32 idx);
        const vecN &operator*=(T rhs);
        vecN operator*(T rhs) const;
        vecN operator+(const vecN &rhs) const;
        void operator+=(const vecN &rhs);
        vecN operator-(const vecN &rhs) const;
        const vecN &operator-=(const vecN &rhs);

        T Dot(const vecN &rhs) const;
        void Zero();
        inline b32 IsValid() const;
    };

#if 1
    // ----------------------------------------------------------------------------------------------------------------
    // Vec2
    // ----------------------------------------------------------------------------------------------------------------
    template <typename T>
    vec2<T>
    Vec2()
    {
        vec2<T> Result = {(T)0, (T)0};
        return Result;
    }

    template <typename T>
    vec2<T>
    Vec2(T A)
    {
        vec2<T> Result = {A, A};
        return Result;
    }

    template <typename T>
    vec2<T>
    MakeVec2(const T * const Ptr)
    {
        vec2<T> Result = {};
        memcpy(Result.E, Ptr, sizeof(T)*2);
        return Result;
    }


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
    vec2<T>
    Vec2(vec3<T> Vec3)
    {
        vec2<T> Result;
        Result.x = Vec3.x;
        Result.y = Vec3.y;
        return Result;

    }

    template <typename T>
    vec2<T>
    operator+(const vec2<T>& A, const vec2<T>& B)
    {
        vec2<T> Result = {A.x + B.x, A.y + B.y};
        return Result;
    }

    template <typename T>
    vec2<T>
    operator-(const vec2<T>& A, const vec2<T>& B)
    {
        vec2<T> Result = {A.x - B.x, A.y - B.y};
        return Result;
    }

    template <typename T>
    vec2<T>
    operator-(const vec2<T> &A)
    {
        vec2<T> Result = {-A.x, -A.y};
        return Result;
    }

    template <typename T>
    vec2<T>
    operator*(const vec2<T>& A, T B)
    {
        vec2<T> Result = {B * A.x, B * A.y};
        return Result;
    }

    template <typename T>
    vec2<T>
    operator*(T A, const vec2<T> &B)
    {
        vec2<T> Result = {A * B.x, A * B.y};
        return Result;
    }

    template <typename T>
    vec2<T>
    operator/(const vec2<T>& A, const vec2<T>& B)
    {
        vec2<T> Result;

        ASSERT(B.x != 0.0f && B.y != 0.0f);
        Result.x = A.x / B.x;
        Result.y = A.y / B.y;

        return Result;
    }

    template <typename T>
    vec2<T>
    operator/(const vec2<T>& A, T B)
    {
        vec2<T> Result;
        ASSERT(B != 0.0f);

        Result.x = A.x / B;
        Result.y = A.y / B;

        return Result;
    }

    template <typename T>
    vec2<T>
    vec2<T>::operator+=(const vec2<T>& A)
    {
        *this = *this + A;
        return *this;
    }

    template <typename T>
    vec2<T>
    vec2<T>::operator-=(const vec2<T>& A)
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
    vec2<T>::operator/=(const vec2<T>& A)
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

    template <typename T>
    vec2<T>
    vec2<T>::Normal() const
    {
        vec2<T> Result = shu::Normalize(shu::Vec2<T>(-this->y, this->x));
        return Result;
    }

    template <typename T>
    T&
    vec2<T>::operator[](size_t Index)
    {
        if(Index >= 2)
        {
            ASSERT(!"Index Out of Bounds");
        }

        return this->E[Index];
    }

    template <typename T>
    T
    vec2<T>::SqMagnitude()
    {
        T Result = this->x * this->x + this->y * this->y;
        return Result;
    }

    template <typename T>
    T
    vec2<T>::Magnitude() const
    {
        T Result = sqrtf(this->x*this->x + this->y*this->y);
        return Result;
    }

    template <typename T>
    vec2<T>
    vec2<T>::Rotate(f32 AngleRadians) const
    {
        vec2<T> Result;

        f32 CosAlpha = CosRad(AngleRadians);
        f32 SinAlpha = SinRad(AngleRadians);

        Result.x = this->x*CosAlpha -this->y*SinAlpha;
        Result.y = this->x*SinAlpha + this->y*CosAlpha;

        return Result;
    }

    template <typename T>
    f32
    vec2<T>::GetSlopeAngleInDegrees()
    {
        f32 Result = atan2f(this->y, this->x);
        Result *= RAD_TO_DEG;
        return Result;
    }

    template <typename T>
    b32
    vec2<T>::IsValid() const
    {
        b32 Result = true;
        if(this->x * 0.0f != this->x * 0.0f) {
            Result = false;
        } else if(this->y * 0.0f != this->y * 0.0f) {
            Result = false;
        }
        return Result;
    }

    template <typename T>
    T
    SqMagnitude(const vec2<T> &A)
    {
        T Result = A.x*A.x + A.y*A.y;
        return Result;
    }

    template <typename T>
    T
    Magnitude(const vec2<T> &A)
    {
        T Result = sqrtf(A.x*A.x + A.y*A.y);
        return Result;
    }

    template <typename T>
    vec2f
    Normalize(const vec2<T> &A)
    {
        f32 OneByMagnitude = 1.0f / (f32)A.Magnitude();
        vec2f Result = Vec2f((f32)(A.x*OneByMagnitude),
                            (f32)(A.y*OneByMagnitude));
        return Result;
    }

    template <typename T>
    T
    Dot(const vec2<T>& A, const vec2<T>& B)
    {
        T Result = A.x*B.x + A.y*B.y;
        return Result;
    }


    template <typename T>
    T
    vec2<T>::Dot(const vec2<T>& A)
    {
        T Result = A.x*this->x + A.y*this->y;
        return Result;
    }

    template <typename T>
    T
    vec2<T>::Cross(const vec2<T>& Other) const
    {
        T Result = this->x * Other.y - this->y * Other.x;
        return Result;
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Vec3
    // ----------------------------------------------------------------------------------------------------------------
    template <typename T>
    vec3<T>
    Vec3()
    {
        vec3<T> Result = {(T)0, (T)0, (T)0};
        return Result;
    }

    template <typename T>
    vec3<T>
    Vec3(T A)
    {
        vec3<T> Result = {A, A, A};
        return Result;
    }

    template <typename T>
    vec3<T>
    Vec3(T x, T y, T z)
    {
        vec3<T> Result = {x, y, z};
        return Result;
    }

    template <typename T>
    vec3<T>
    Vec3(const vec2<T> &V)
    {
        vec3<T> Result = {V.x, V.y, (T)0};
        return Result;
    }

    template <typename T>
    vec3<T>
    MakeVec3(const T * const Ptr)
    {
        vec3<T> Result = {};

        memcpy(Result.E, Ptr, sizeof(T) * 3);

        return Result;
    }

    template <typename T>
    vec3<T>
    Vec3(const vec2<T>& xy, T z)
    {
        vec3<T> Result = Vec3(xy.x, xy.y, z);

        return Result;
    }

    template <typename T>
    vec3<T>
    ToVec3(const vec2<T>& A)
    {
        vec3<T> Result = Vec3(A.x, A.y, 0.0f);
        return Result;
    }

    template <typename T>
    vec2<T>
    ToVec2(const vec3<T>& A)
    {
        vec2<T> Result = Vec2(A.x, A.y);
        return Result;
    }

    template <typename T>
    T
    Dot(const vec3<T>& A, const vec3<T>& B)
    {
        T Result = A.x * B.x + A.y * B.y + A.z * B.z;
        return Result;
    }

    template <typename T>
    T
    vec3<T>::Dot(const vec3<T>& A) const
    {
        T Result = A.x*this->x + A.y*this->y + A.z*this->z;
        return Result;
    }

    template <typename T>
    vec3<T>
    Cross(const vec3<T> &A, const vec3<T> &B)
    {
        vec3<T> Result;
        Result.x = A.y*B.z - A.z*B.y;
        Result.y = A.z*B.x - A.x*B.z;
        Result.z = A.x*B.y - A.y*B.x;
        return Result;
    }

    template <typename T>
    vec3<T>
    vec3<T>::Cross(const vec3<T> &A) const
    {
        vec3<T> Result = shu::Cross(*this, A);
        return Result;
    }

    template <typename T>
    inline vec3<T>
    vec3<T>::Reciprocal() const
    {
        vec3<T> Result;

        Result.x = NearlyEqualUlps(this->x, 0.0f) ? 0.0f : (1.0f/this->x);
        Result.y = NearlyEqualUlps(this->y, 0.0f) ? 0.0f : (1.0f/this->y);
        Result.z = NearlyEqualUlps(this->z, 0.0f) ? 0.0f : (1.0f/this->z);

        return Result;
    }

    template <typename T>
    inline void
    vec3<T>::GetOrtho(vec3<T> &U, vec3<T> &V, vec3<T> up) const
    {
        vec3<T> n = *this;
        n.Normalize();

        if (up == Vec3<T>(0, 1, 0))
        {
            const shu::vec3<T> w = (n.z*n.z > .81f) ? up : Vec3<T>(0, 0, 1);
            U = w.Cross(n);

            V = n.Cross(U);
            U = V.Cross(n);
        }
        else
        {
            up.Normalize();
            U = up.Cross(n);
            V = up;
        }
    }

    template <typename T>
    inline b32
    vec3<T>::IsValid() const
    {
        b32 Result = true;

        // Checking For Nans
        if(this->x * 0.0f != this->x * 0.0f) {
            Result = false;
        } else if(this->y * 0.0f != this->y * 0.0f) {
            Result = false;
        } else if(this->z * 0.0f != this->z * 0.0f) {
            Result = false;
        }

        return Result;
    }

    template <typename T>
    inline b32
    vec3<T>::IsNormalized() const
    {
        f32 SqMagnitude = this->x*this->x * this->y*this->y * this->z*this->z;

        b32 Result = NearlyEqual(SqMagnitude, 1.0f, 0.0001f);

        return Result;
    }

    template <typename T>
    inline b32
    vec3<T>::IsZero() const
    {
        b32 xIsZero = (NearlyEqual(this->x, 0.0f, 0.00001f));
        if(!xIsZero) return false;
        b32 yIsZero = (NearlyEqual(this->y, 0.0f, 0.00001f));
        if(!yIsZero) return false;
        b32 zIsZero = (NearlyEqual(this->z, 0.0f, 0.00001f));
        if(!zIsZero) return false;

        return true;
    }

    template <typename T>
    inline void
    vec3<T>::ZeroOut()
    {
        this->x = (T)0;
        this->y = (T)0;
        this->z = (T)0;
    }

    template <typename T>
    vec3<T>
    operator+(const vec3<T>& A, const vec3<T>& B)
    {
        vec3<T> Result;

        Result.x = A.x + B.x;
        Result.y = A.y + B.y;
        Result.z = A.z + B.z;

        return Result;
    }

    template <typename T>
    vec3<T>
    operator-(const vec3<T>& A, const vec3<T>& B)
    {
        vec3<T> Result;

        Result.x = A.x - B.x;
        Result.y = A.y - B.y;
        Result.z = A.z - B.z;

        return Result;
    }

    template <typename T>
    vec3<T>
    operator-(const vec3<T>& A)
    {
        vec3<T> Result;

        Result.x = -A.x;
        Result.y = -A.y;
        Result.z = -A.z;

        return Result;
    }

    template <typename T>
    vec3<T>
    operator*(const vec3<T>& A, T B)
    {
        vec3<T> Result;

        Result.x = B * A.x;
        Result.y = B * A.y;
        Result.z = B * A.z;

        return Result;
    }

    template <typename T>
    vec3<T>
    operator*(T A, const vec3<T> &B)
    {
        vec3<T> Result;

        Result.x = A * B.x;
        Result.y = A * B.y;
        Result.z = A * B.z;

        return Result;
    }

    template <typename T>
    vec3<T>
    operator*(const vec3<T> &A, const vec3<T> &B)
    {
        vec3<T> Result;

        Result.x = A.x * B.x;
        Result.y = A.y * B.y;
        Result.z = A.z * B.z;

        return Result;
    }

    template <typename T>
    vec3<T>
    operator/(const vec3<T>& A, const vec3<T>& B)
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
    operator/(const vec3<T>& A, T B)
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
    vec3<T>::operator+=(const vec3<T>& A)
    {
        *this = *this + A;
        return *this;
    }

    template <typename T>
    vec3<T>
    vec3<T>::operator-=(const vec3<T>& A)
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
    vec3<T>::operator*=(const vec3<T> &A)
    {
        this->x *= A.x;
        this->y *= A.y;
        this->z *= A.z;
        return *this;
    }

    template <typename T>
    vec3<T>
    vec3<T>::operator/=(const vec3<T>& A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    b32
    vec3<T>::operator==(const vec3<T> &Rhs) const
    {
        b32 Result = NearlyEqual(this->x, Rhs.x, 0.0001f) &&
                     NearlyEqual(this->y, Rhs.y, 0.0001f) &&
                     NearlyEqual(this->z, Rhs.z, 0.0001f);
        return Result;
    }

    template <typename T>
    b32
    vec3<T>::operator!=(const vec3<T> &Rhs) const
    {
        b32 Result = !NearlyEqual(this->x, Rhs.x, 0.0001f) ||
                     !NearlyEqual(this->y, Rhs.y, 0.0001f) ||
                     !NearlyEqual(this->z, Rhs.z, 0.0001f);
        return Result;
    }

    template <typename T>
    vec3<T>
    vec3<T>::operator/=(T A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    T&
    vec3<T>::operator[](size_t Index)
    {
        if (Index >= 3)
        {
            ASSERT(!"Index Out of Bounds");
        }

        return this->E[Index];
    }

    template <typename T>
    T
    vec3<T>::operator[](size_t Index) const
    {
        if (Index >= 3)
        {
            ASSERT(!"Index Out of Bounds");
        }

        return this->E[Index];
    }

    template <typename T>
    T
    vec3<T>::SqMagnitude() const
    {
        T Result = this->x*this->x + this->y*this->y + this->z*this->z;
        return Result;
    }

    template <typename T>
    T
    vec3<T>::Magnitude() const
    {
        // NOTE: Change this to use ouw own Square Root function
        T Result = sqrtf(this->x*this->x + this->y*this->y + this->z*this->z);
        return Result;
    }

    template <typename T>
    void
    vec3<T>::Normalize()
    {
        // NOTE: Change this to use ouw own Square Root function
        *this = shu::Normalize(*this);
    }

    template <typename T>
    T
    SqMagnitude(const vec3<T> &A)
    {
        T Result = A.x*A.x + A.y*A.y + A.z*A.z;
        return Result;
    }

    template <typename T>
    T
    Magnitude(const vec3<T> &A)
    {
        T Result = sqrtf(A.x*A.x + A.y*A.y + A.z*A.z);
        return Result;
    }

    template <typename T>
    T
    ScalarTripleProduct(const vec3<T> &A, const vec3<T> &B, const vec3<T> &C)
    {
        T Result = Dot(A, Cross(B, C));
        return Result;
    }

    template <typename T>
    vec3f
    Normalize(const vec3<T> &A)
    {
        vec3f Result = A;
        f32 SqMagnitude = (f32)A.SqMagnitude();
        if(SqMagnitude > 0.0f)
        {
            f32 OneByMagnitude = 1.0f / sqrtf(SqMagnitude);
            Result *= OneByMagnitude;
        }
        return Result;
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Vec4
    // ----------------------------------------------------------------------------------------------------------------
    template <typename T>
    vec4<T>
    Vec4()
    {
        vec4<T> Result = {(T)0, (T)0, (T)0, (T)0};
        return Result;
    }

    template <typename T>
    vec4<T>
    Vec4(T A)
    {
        vec4<T> Result = {A, A, A, A};
        return Result;
    }

    template <typename T>
    vec4<T>
    Vec4(T x, T y, T z, T w)
    {
        vec4<T> Result;
        Result.x = x;
        Result.y = y;
        Result.z = z;
        Result.w = w;
        return Result;
    }

    template <typename T>
    vec4<T>
    Vec4(const vec3<T> &v, f32 w)
    {
        vec4<T> Result;
        Result.x = v.x;
        Result.y = v.y;
        Result.z = v.z;
        Result.w = w;
        return Result;
    }

    template <typename T>
    vec4<T>
    MakeVec4(const T *const Ptr)
    {
        vec4<T> Result = {};
        memcpy(Result.E, Ptr, sizeof(T)*4);
        return Result;
    }

    template <typename T>
    T
    Dot(const vec4<T>& A, const vec4<T>& B)
    {
        T Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
        return Result;
    }

    template <typename T>
    vec4<T>
    operator+(const vec4<T>& A, const vec4<T>& B)
    {
        vec4<T> Result = {A.x+B.x, A.y+B.y, A.z+B.z, A.w+B.w};
        return Result;
    }

    template <typename T>
    vec4<T>
    operator-(const vec4<T>& A, const vec4<T>& B)
    {
        vec4<T> Result = {A.x-B.x, A.y-B.y, A.z-B.z, A.w-B.w};
        return Result;
    }

    template <typename T>
    vec4<T>
    operator*(const vec4<T>& A, const vec4<T>& B)
    {
        vec4<T> Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};
        return Result;
    }

    template <typename T>
    vec4<T>
    operator*(const vec4<T>& A, T B)
    {
        vec4<T> Result = {A.x*B, A.y*B, A.z*B, A.w*B};
        return Result;
    }

    template <typename T>
    vec4<T>
    operator/(const vec4<T>& A, const vec4<T>& B)
    {
        ASSERT((B.x != 0.0f) && (B.y != 0.0f) && (B.z != 0.0f) && (B.w != 0.0f));

        vec4<T> Result = {A.x/B.x, A.y/B.y, A.z/B.z, A.w/B.w};
        return Result;
    }

    template <typename T>
    vec4<T>
    operator/(const vec4<T>& A, T B)
    {
        ASSERT(B != 0.0f);

        vec4<T> Result = {A.x/B, A.y/B, A.z/B, A.w/B};
        return Result;
    }

    template <typename T>
    vec4<T>
    vec4<T>::operator+=(const vec4<T>& A)
    {
        *this = *this + A;
        return *this;
    }

    template <typename T>
    vec4<T>
    vec4<T>::operator-=(const vec4<T>& A)
    {
        *this = *this - A;
        return *this;
    }

    template <typename T>
    vec4<T>
    vec4<T>::operator*=(T A)
    {
        *this = *this * A;
        return *this;
    }

    template <typename T>
    vec4<T>
    vec4<T>::operator/=(const vec4<T>& A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    b32
    vec4<T>::operator==(const vec4<T> &rhs) const
    {
        b32 Result = false;
        Result = NearlyEqual(x, rhs.x) && NearlyEqual(y, rhs.y) &&
                 NearlyEqual(z, rhs.z) && NearlyEqual(w, rhs.w);

        return Result;
    }

    template <typename T>
    vec4<T>
    vec4<T>::operator/=(T A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    T &
    vec4<T>::operator[](size_t Index)
    {
        if (Index >= 4)
        {
            ASSERT(!"Index Out of Bounds");
        }

        return this->E[Index];
    }

    template <typename T>
    T
    vec4<T>::operator[](size_t Index) const
    {
        if(Index >= 4)
        {
            ASSERT(!"Index Out of Bounds");
        }

        return this->E[Index];
    }

    template <typename T>
    void
    vec4<T>::ZeroOut()
    {
        this->x = (T)0;
        this->y = (T)0;
        this->z = (T)0;
        this->w = (T)0;
    }

    template <typename T>
    b32
    vec4<T>::IsValid() const
    {
        b32 Result = true;

        // Checking For Nans
        if(this->x * 0.0f != this->x * 0.0f) {
            Result = false;
        } else if(this->y * 0.0f != this->y * 0.0f) {
            Result = false;
        } else if(this->z * 0.0f != this->z * 0.0f) {
            Result = false;
        } else if(this->w * 0.0f != this->w * 0.0f) {
            Result = false;
        }

        return Result;
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Vec6
    // ----------------------------------------------------------------------------------------------------------------
    template <typename T>
    vec6<T>
    Vec6()
    {
        vec6<T> Result;
        for(i32 i = 0; i < 6; ++i)
        {
            Result.E[i] = 0;
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    Vec6(T A)
    {
        vec6<T> Result;
        for (i32 i = 0; i < 6; ++i)
        {
            Result.E[i] = A;
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    Vec6(T x1, T y1, T z1, T x2, T y2, T z2)
    {
        vec6<T> Result;
        Result.x1 = x1;
        Result.y1 = y1;
        Result.z1 = z1;

        Result.x2 = x2;
        Result.y2 = y2;
        Result.z2 = z2;
        return Result;
    }

    template <typename T>
    vec6<T>
    MakeVec6(const T *const Ptr)
    {
        vec6<T> Result = {};
        memcpy(Result.E, Ptr, sizeof(T)*6);
        return Result;
    }

    template <typename T>
    T
    Dot(const vec6<T>& A, const vec6<T>& B)
    {
        T Result = (T)0;
        for (i32 i = 0; i < 6; ++i)
        {
            Result += A[i]*B[i];
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator+(const vec6<T>& A, const vec6<T>& B)
    {
        vec6<T> Result;

        for (i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i]+B[i];
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator+(const vec6<T>& A, T B)
    {
        vec6<T> Result;

        for (i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i] + B;
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator-(const vec6<T>& A, const vec6<T>& B)
    {
        vec6<T> Result;

        for (i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i]-B[i];
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator-(const vec6<T>& A, T B)
    {
        vec6<T> Result;

        for (i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i] - B;
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator*(const vec6<T>& A, const vec6<T>& B)
    {
        vec6<T> Result;

        for (i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i]*B[i];
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator*(const vec6<T>& A, T B)
    {
        vec6<T> Result;

        for (i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i]*B;
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator/(const vec6<T>& A, const vec6<T>& B)
    {
        for (i32 i = 0; i < 6; ++i)
        {
            ASSERT(B[i] != 0.0f);
        }

        vec6<T> Result;

        for(i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i] / B[i];
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator/(const vec6<T>& A, T B)
    {
        ASSERT(B != 0.0f);

        vec6<T> Result;
        for(i32 i = 0; i < 6; ++i)
        {
            Result[i] = A[i] / B;
        }
        return Result;
    }

    template <typename T>
    inline T
    vec6<T>::Dot(const vec6<T> &A)
    {
        T Result = (T)0;

        for(i32 i = 0; i < 6; ++i)
        {
            Result += this->E[i] * A[i];
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    vec6<T>::operator+=(const vec6<T>& A)
    {
        *this = *this + A;
        return *this;
    }

    template <typename T>
    vec6<T>
    vec6<T>::operator+=(T A)
    {
        *this = *this + A;
        return *this;
    }

    template <typename T>
    vec6<T>
    vec6<T>::operator-=(const vec6<T>& A)
    {
        *this = *this - A;
        return *this;
    }

    template <typename T>
    vec6<T>
    vec6<T>::operator-=(T A)
    {
        *this = *this - A;
        return *this;
    }

    template <typename T>
    vec6<T>
    vec6<T>::operator*=(T A)
    {
        *this = *this * A;
        return *this;
    }

    template <typename T>
    vec6<T>
    vec6<T>::operator/=(const vec6<T>& A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    vec6<T>
    vec6<T>::operator/=(T A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    T &
    vec6<T>::operator[](size_t Index)
    {
        if (Index < 0 && Index > 6)
        {
            ASSERT(!"Index Out of Bounds");
        }

        return this->E[Index];
    }

    template <typename T>
    const T &
    vec6<T>::operator[](size_t Index) const
    {
        if (Index < 0 && Index > 6)
        {
            ASSERT(!"Index Out of Bounds");
        }

        return this->E[Index];
    }

    template <typename T>
    b32
    vec6<T>::IsValid() const
    {
        b32 Result = true;

        for (i32 i = 0; i < 6; ++i)
        {
            if(this->E[i] * 0.0f != this->E[i] * 0.0f) {
                Result = false;
                break;
            }
        }

        return Result;
    }

    // -----------------------------------------------------------------------
    // VecN
    // -----------------------------------------------------------------------
    template <typename T, size_t N>
    inline vecN<T, N>::vecN()
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Data[i] = (T)0;
        }
    }

    template <typename T, size_t N>
    inline
    vecN<T, N>::vecN(const vecN<T, N> &rhs)
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Data[i] = rhs.Data[i];
        }
    }

    template <typename T, size_t N>
    inline vecN<T, N> &
    vecN<T, N>::operator=(const vecN<T, N> &rhs)
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Data[i] = rhs.Data[i];
        }

        return *this;
    }

    template <typename T, size_t N>
    inline vecN<T, N>::~vecN()
    {
        // LogInfoUnformatted("vecN Destructor called!");
    }

    template <typename T, size_t N>
    T
    vecN<T, N>::operator[](const i32 idx) const
    {
        ASSERT(idx < N);
        return this->Data[idx];
    }

    template <typename T, size_t N>
    T &
    vecN<T, N>::operator[](const i32 idx)
    {
        ASSERT(idx < N);
        return this->Data[idx];
    }


    template <typename T, size_t N>
    const vecN<T, N> &
    vecN<T, N>::operator*=(T rhs)
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Data[i] *= rhs;
        }
        return *this;
    }

    template <typename T, size_t N>
    vecN<T, N>
    vecN<T, N>::operator*(T rhs) const
    {
        vecN<T, N> Result = *this;
        for (i32 i = 0; i < N; ++i)
        {
            Result.Data[i] *= rhs;
        }
        return Result;
    }

    template <typename T, size_t N>
    vecN<T, N>
    vecN<T, N>::operator+(const vecN<T, N> &rhs) const
    {
        vecN<T, N> Result = *this;
        for (i32 i = 0; i < N; ++i)
        {
            Result.Data[i] += rhs.Data[i];
        }
        return Result;
    }

    template <typename T, size_t N>
    void
    vecN<T, N>::operator+=(const vecN<T, N> &rhs)
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Data[i] += rhs.Data[i];
        }
    }

    template <typename T, size_t N>
    vecN<T, N>
    vecN<T, N>::operator-(const vecN<T, N> &rhs) const
    {
        vecN<T, N> Result = *this;
        for (i32 i = 0; i < N; ++i)
        {
            Result.Data[i] -= rhs.Data[i];
        }
        return Result;
    }

    template <typename T, size_t N>
    const vecN<T, N> &
    vecN<T, N>::operator-=(const vecN<T, N> &rhs)
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Data[i] -= rhs.Data[i];
        }
        return *this;
    }

    template <typename T, size_t N>
    T
    vecN<T, N>::Dot(const vecN<T, N> &rhs) const
    {
        T Result = (T)0;
        for (i32 i = 0; i < N; ++i)
        {
            Result += this->Data[i] * rhs.Data[i];
        }

        return Result;
    }

    template <typename T, size_t N>
    void
    vecN<T, N>::Zero()
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Data[i] = (T)0;
        }
    }

    template <typename T, size_t N>
    b32
    vecN<T, N>::IsValid() const
    {
        b32 Result = true;
        for (i32 i = 0; i < N; ++i)
        {
            if (this->Data[i] * 0.0f != this->Data[i] * 0.0f)
            {
                Result = false;
                break;
            }
        }
        return Result;
    }

    template <typename T, size_t N>
    template <typename... Args>
    vecN<T, N>::vecN(Args... args)
    {
        size_t numArgs = sizeof...(args);
        T tempArray[N] = {static_cast<T>(args)...};
        for (size_t i = 0; i < N; ++i)
        {
            if (i < numArgs)
            {
                this->Data[i] = tempArray[i];
            }
            else
            {
                this->Data[i] = (T)0;
            }
        }
    }
#endif // #if 1
}

#define MATH_VECTOR_H
#endif
