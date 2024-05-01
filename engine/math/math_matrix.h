#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include "defines.h"
#include "math_vector.h"
#include "math_trig.h"
#include "platform/platform.h"

namespace shu
{
#if 0
    enum matrix_type
    {
        NONE,
        ZERO,
        IDENTITY,
        TRANSLATION,
        ORTHOGONAL,
        MODEL_MATRIX,
    };
#endif

    template <typename T>
    struct mat3
    {
        union
        {
            struct
            {
                T m00, m01, m02;
                T m10, m11, m12;
                T m20, m21, m22;
            };
            struct
            {
                vec3<T> r0;
                vec3<T> r1;
                vec3<T> r2;
            };
            T m[3][3];
            vec3<T> Rows[3];
            T E[9];
        };

        inline mat3<T> operator+=(const mat3<T> &M);
        inline mat3<T> operator-=(const mat3<T> &M);
        inline mat3<T> operator*=(const mat3<T> &M);
        inline mat3<T> operator*=(T A);
        inline mat3<T> operator/=(const mat3<T> &M);
        inline mat3<T> operator/=(T A);
        inline mat3<T>& operator=(const mat3<T> &other);
        inline mat3<T> Transposed() const;
        inline mat3<T> MakeTranspose();
        inline mat3<T> Mul(const mat3<T> &M);
        inline vec3<T> MulVec(const vec3<T> &V);
        inline mat3<T> Mul(T A);
        inline vec3<T> GetColumn(size_t Index);
        inline void SetColumn(u32 ColumnIndex, const vec3<T> &Column);
        inline vec3<T> GetRow(size_t Index);

        inline T Determinant() const;
        inline T CoFactor(i32 i, i32 j) const;
        inline mat3<T> Adjoint() const;
        inline mat3<T> Inverse() const;
        inline mat3<T> InverseTranspose() const;

        inline b32 IsIdentity();
        inline b32 IsZero();

        inline vec3<T>& operator[](size_t RowIndex);
        inline T& operator()(size_t RowIndex, size_t ColumnIndex);
    };

    template <typename T> SHU_EXPORT mat3<T> Mat3();
    template <typename T> SHU_EXPORT mat3<T> Mat3(T Val);
    template <typename T>
    SHU_EXPORT mat3<T> Mat3(T m00, T m01, T m02,
                            T m10, T m11, T m12,
                            T m20, T m21, T m22);
    template <typename T> SHU_EXPORT mat3<T> Mat3(const vec3<T> &r0, const vec3<T> &r1, const vec3<T> &r2);
    template <typename T> SHU_EXPORT mat3<T> operator+(const mat3<T> &M1, const mat3<T> &M2);
    template <typename T> SHU_EXPORT mat3<T> operator-(const mat3<T> &M1, const mat3<T> &M2);
    template <typename T> SHU_EXPORT mat3<T> operator*(const mat3<T> &M1, T B);
    template <typename T> SHU_EXPORT mat3<T> operator*(const mat3<T> &M1, const mat3<T> &M2);
    template <typename T> SHU_EXPORT vec3<T> operator*(const vec3<T> &V, const mat3<T> &M1);
    template <typename T> SHU_EXPORT mat3<T> operator/(const mat3<T> &M1, T B);
    template <typename T> SHU_EXPORT mat3<T> operator/(const mat3<T> &M1, const mat3<T> &M2);
    template <typename T> SHU_EXPORT mat3<T> Transpose(const mat3<T> &M1);

    typedef mat3<i32> mat3i;
    typedef mat3<u32> mat3u;
    typedef mat3<f32> mat3f;
#define Mat3f Mat3<f32>
#define Mat3u Mat3<u32>
#define Mat3i Mat3<i32>

    template <typename T>
    struct mat4
    {
        union
        {
            struct
            {
                T m00, m01, m02, m03;
                T m10, m11, m12, m13;
                T m20, m21, m22, m23;
                T m30, m31, m32, m33;
            };
            struct
            {
                vec4<T> Row0;
                vec4<T> Row1;
                vec4<T> Row2;
                vec4<T> Row3;
            };
            T m[4][4];
            vec4<T> Rows[4];
            T E[16];
        };

        inline mat3<T> ToMat3();
        inline mat4<T> operator+=(const mat4<T> &M);
        inline mat4<T> operator-=(const mat4<T> &M);
        inline mat4<T> operator*=(const mat4<T> &M);
        inline mat4<T> operator*=(T A);
        inline mat4<T> operator/=(const mat4<T> &M);
        inline mat4<T> operator/=(T A);
        inline vec4<T>& operator[](size_t RowIndex);
        inline T& operator()(size_t RowIndex, size_t ColumnIndex);
        inline mat4<T> Transposed();
        inline mat4<T> MakeTranspose();
        inline mat4<T> Mul(const mat4<T> &M);
        inline vec4<T> MulVec(const vec4<T> &V);
        inline vec4<T> MulVecDir(const vec3<T> &V);
        inline mat4<T> Mul(T A);
        inline vec4<T> GetColumn(size_t Index);
        inline vec4<T> GetRow(size_t Index);

        inline mat3<T> Minor(i32 i, i32 j) const;
        inline T Determinant() const;
        inline T CoFactor(i32 i, i32 j) const;
        inline mat4<T> Adjoint() const;
        inline mat4<T> Inverse() const;
        inline mat4<T> InverseTranspose() const;

    };

    typedef mat4<i32> mat4i;
    typedef mat4<u32> mat4u;
    typedef mat4<f32> mat4f;
#define Mat4f Mat4<f32>
#define Mat4u Mat4<u32>
#define Mat4i Mat4<i32>

    template <typename T> SHU_EXPORT mat4<T> Mat4();
    template <typename T> SHU_EXPORT mat4<T> Mat4(T Val);
    template <typename T>
    SHU_EXPORT mat4<T> Mat4(T m00, T m01, T m02, T m03,
                            T m10, T m11, T m12, T m13,
                            T m20, T m21, T m22, T m23,
                            T m30, T m31, T m32, T m33);
    template <typename T> SHU_EXPORT mat4<T> Mat4(const vec4<T> &r0, const vec4<T> &r1,
                                                  const vec4<T> &r2, const vec4<T> &r3);
    template <typename T> SHU_EXPORT mat4<T> operator+(const mat4<T> &M1, const mat4<T> &M2);
    template <typename T> SHU_EXPORT mat4<T> operator-(const mat4<T> &M1, const mat4<T> &M2);
    template <typename T> SHU_EXPORT mat4<T> operator*(const mat4<T> &M1, T B);
    template <typename T> SHU_EXPORT mat4<T> operator*(const mat4<T> &M1, const mat4<T> &M2);
    template <typename T> SHU_EXPORT vec4<T> operator*(const mat4<T> &M, const vec4<T> &V);
    template <typename T> SHU_EXPORT vec4<T> operator*(const vec4<T> &V, const mat4<T> &M);
    template <typename T> SHU_EXPORT vec4<T> operator*(const mat4<T> &M, const vec3<T> &V);
    template <typename T> SHU_EXPORT mat4<T> operator/(const mat4<T> &M, T B);
    template <typename T> SHU_EXPORT mat4<T> operator/(const mat4<T> &M1, const mat4<T> &M2);
    template <typename T> SHU_EXPORT mat4<T> Transpose(const mat4<T> &M);

    template <typename T>
    struct mat6
    {
        union
        {
            struct
            {
                T m00, m01, m02, m03, m04, m05;
                T m10, m11, m12, m13, m14, m15;
                T m20, m21, m22, m23, m24, m25;
                T m30, m31, m32, m33, m34, m35;
                T m40, m41, m42, m43, m44, m45;
                T m50, m51, m52, m53, m54, m55;
            };
            struct
            {
                vec6<T> Row0, Row1, Row2;
                vec6<T> Row3, Row4, Row5;
            };
            T m[6][6];
            vec6<T> Rows[6];
            T E[36];
        };

        inline mat6<T> ToMat6();
        inline mat6<T> operator+=(const mat6<T> &M);
        inline mat6<T> operator-=(const mat6<T> &M);
        inline mat6<T> operator*=(const mat6<T> &M);
        inline mat6<T> operator*=(T A);
        inline mat6<T> operator/=(const mat6<T> &M);
        inline mat6<T> operator/=(T A);
        inline vec6<T>& operator[](size_t RowIndex);
        inline T& operator()(size_t RowIndex, size_t ColumnIndex);
        inline mat6<T> Transposed();
        inline mat6<T> MakeTranspose();
        inline mat6<T> Mul(const mat6<T> &M);
        inline vec6<T> MulVec(const vec6<T> &V);
        inline mat6<T> Mul(T A);
        inline vec6<T> GetColumn(size_t Index);
        inline vec6<T> GetRow(size_t Index);
    };

    template <typename T> SHU_EXPORT mat6<T> Mat6();
    template <typename T> SHU_EXPORT mat6<T> Mat6(T Val);
    template <typename T>
    SHU_EXPORT mat6<T> Mat6(T m00, T m01, T m02, T m03, T m04, T m05,
                            T m10, T m11, T m12, T m13, T m14, T m15,
                            T m20, T m21, T m22, T m23, T m24, T m25,
                            T m30, T m31, T m32, T m33, T m34, T m35,
                            T m40, T m41, T m42, T m43, T m44, T m45,
                            T m50, T m51, T m52, T m53, T m54, T m55);

    template <typename T>
    SHU_EXPORT mat6<T> Mat6(const vec6<T> &r0, const vec6<T> &r1, const vec6<T> &r2,
                            const vec6<T> &r3, const vec6<T> &r4, const vec6<T> &r5);

    template <typename T> SHU_EXPORT mat6<T> operator+(const mat6<T> &M1, const mat6<T> &M2);
    template <typename T> SHU_EXPORT mat6<T> operator-(const mat6<T> &M1, const mat6<T> &M2);
    template <typename T> SHU_EXPORT mat6<T> operator*(const mat6<T> &M1, T B);
    template <typename T> SHU_EXPORT mat6<T> operator*(const mat6<T> &M1, const mat6<T> &M2);
    template <typename T> SHU_EXPORT vec6<T> operator*(const mat6<T> &M, const vec6<T> &V);
    template <typename T> SHU_EXPORT vec6<T> operator*(const vec6<T> &V, const mat6<T> &M);
    template <typename T> SHU_EXPORT mat6<T> operator/(const mat6<T> &M, T B);
    template <typename T> SHU_EXPORT mat6<T> operator/(const mat6<T> &M1, const mat6<T> &M2);
    template <typename T> SHU_EXPORT mat6<T> Transpose(const mat6<T> &M);

    typedef mat6<i32> mat6i;
    typedef mat6<u32> mat6u;
    typedef mat6<f32> mat6f;
#define Mat6f Mat6<f32>
#define Mat6u Mat6<u32>
#define Mat6i Mat6<i32>

    // N X N Matrix
#include "math_vector.h"

    template<typename T, size_t N>
    struct matN
    {
        union
        {
            vecN<T, N> Rows[N];
            T Data[N][N];
        };

        matN();
        matN(const matN &rhs);

        template <typename... Args>
        matN(Args... args);

        ~matN();

        void Identity();
        void Zero();

        void Transpose();
        matN<T, N> Transposed() const;
        b32 IsDiagonallyDominant() const;

        vecN<T, N> GetColumn(i32 Index) const;

        const matN &operator=(const matN &rhs);
        void operator*=(T rhs);
        vecN<T, N> operator*(const vecN<T, N> &rhs) const;
        matN<T,N> operator*(const matN<T,N> &rhs) const;
    };

    template<typename T, size_t M, size_t N>
    struct matMN
    {
        union
        {
            vecN<T, N> Rows[M];
            T Data[M][N];
        };

        matMN();
        matMN(const matMN &rhs);
        matMN(const matN<T, M> &rhs);

        template <typename... Args>
        matMN(Args... args);
        ~matMN();

        void Zero();

        void AddColumn(i32 columnIndex, const vecN<T, M> &v);
        matMN<T, N, M> Transposed() const;

        matMN<T, M, N> RowEchelon() const;
        b32 IsRowEchelon() const;

        const matMN &operator=(const matMN &rhs);
        void operator*=(T rhs);
        vecN<T, M> operator*(const vecN<T, N> &rhs) const;

        matN<T, M> operator*(const matMN<T, N, M> &rhs) const;
        matMN<T, M, N> operator*(const matN<T, N> &rhs) const;
    };


    template<typename T, size_t M, size_t N>
    vecN<T, N> operator*(const vecN<T, M> &Vector, const matMN<T, M, N> &Matrix);

#if 1
    // -------------------------------------------------------------------------------------------------
    // Matrix3x3
    // -------------------------------------------------------------------------------------------------

    template <typename T>
    mat3<T>
    Mat3()
    {
        mat3<T> Result = {};
        return Result;
    }

    template <typename T>
    mat3<T>
    Mat3(T Val)
    {
        mat3<T> Result = {};

        Result.m00 = Val;
        Result.m11 = Val;
        Result.m22 = Val;

        return Result;
    }

    template <typename T>
    mat3<T>
    Mat3(T m00, T m01, T m02,
         T m10, T m11, T m12,
         T m20, T m21, T m22)
    {
        mat3<T> Result;

        Result.m00 = m00;
        Result.m01 = m01;
        Result.m02 = m02;

        Result.m10 = m10;
        Result.m11 = m11;
        Result.m12 = m12;

        Result.m20 = m20;
        Result.m21 = m21;
        Result.m22 = m22;

        return Result;
    }

    template <typename T>
    mat3<T>
    Mat3(const vec3<T> &r0, const vec3<T> &r1, const vec3<T> &r2)
    {
        mat3<T> Result;

        Result.Rows[0] = r0;
        Result.Rows[1] = r1;
        Result.Rows[2] = r2;

        return Result;
    }

    template <typename T>
    mat3<T>
    operator+(const mat3<T> &M1, const mat3<T> &M2)
    {
        mat3<T> Result = {};

        for (u32 i = 0; i < 9; ++i)
        {
            Result.E[i] = M1.E[i] + M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat3<T>
    operator-(const mat3<T> &M1, const mat3<T> &M2)
    {
        mat3<T> Result = {};

        for (u32 i = 0; i < 9; ++i)
        {
            Result.E[i] = M1.E[i] - M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat3<T>
    operator*(const mat3<T> &M1, T B)
    {
        mat3<T> Result = {};

        for (u32 i = 0; i < 9; ++i)
        {
            Result.E[i] = M1.E[i] * B;
        }

        return Result;
    }

    // NOTE: Row of first matrix multiplied by columns of second matrix.
    template <typename T>
    mat3<T>
    operator*(const mat3<T> &M1, const mat3<T> &M2)
    {
        mat3<T> Result = {};

        for (u32 i = 0; i < 3; ++i)
        {
            for (u32 j = 0; j < 3; ++j)
            {
                for (u32 k = 0; k < 3; ++k)
                {
                    Result.m[i][j] += M1.m[i][k] * M2.m[k][j];
                }
            }
        }

        return Result;
    }

    template <typename T>
    vec3<T>
    operator*(const vec3<T> &V, const mat3<T> &M)
    {
        vec3<T> Result;

        Result.x = V.x*M.m00 + V.y*M.m10 + V.z*M.m20;
        Result.y = V.x*M.m01 + V.y*M.m11 + V.z*M.m21;
        Result.z = V.x*M.m02 + V.y*M.m12 + V.z*M.m22;

        return Result;
    }

    template <typename T>
    mat3<T>
    operator/(const mat3<T> &M1, T B)
    {
        ASSERT(B != (T)0);
        mat3<T> Result = {};

        for (u32 i = 0; i < 9; ++i)
        {
            Result.E[i] = M1.E[i] / B;
        }

        return Result;
    }

    template <typename T>
    mat3<T>
    operator/(const mat3<T> &M1, const mat3<T> &M2)
    {
        mat3<T> Result = {};

        for (u32 i = 0; i < 9; ++i)
        {
            ASSERT(M2.E[i] != (T)0);
            Result.E[i] = M1.E[i] / M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat3<T>
    mat3<T>::operator+=(const mat3<T> &M)
    {
        *this = *this + M;
        return *this;
    }

    template <typename T>
    mat3<T>
    mat3<T>::operator-=(const mat3<T> &M)
    {
        *this = *this - M;
        return *this;
    }

    template <typename T>
    mat3<T>
    mat3<T>::operator*=(const mat3<T> &M)
    {
        *this = *this * M;
        return *this;
    }

    template <typename T>
    mat3<T>&
    mat3<T>::operator=(const mat3<T> &M)
    {
        this->m00 = M.m00; this->m01 = M.m01; this->m02 = M.m02;
        this->m10 = M.m10; this->m11 = M.m11; this->m12 = M.m12;
        this->m20 = M.m20; this->m21 = M.m21; this->m22 = M.m22;
        return *this;
    }

    template <typename T>
    mat3<T>
    mat3<T>::operator*=(T A)
    {
        *this = *this * A;
        return *this;
    }

    template <typename T>
    mat3<T>
    mat3<T>::operator/=(const mat3<T> &M)
    {
        *this = *this / M;
        return *this;
    }

    template <typename T>
    mat3<T>
    mat3<T>::operator/=(T A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    mat3<T>
    mat3<T>::Mul(const mat3<T> &M)
    {
        mat3<T> Result = *this * M;
        return Result;
    }

    template <typename T>
    vec3<T>
    mat3<T>::MulVec(const vec3<T> &V)
    {
        vec3<T> Result = V * (*this);
        return Result;
    }

    template <typename T>
    mat3<T>
    mat3<T>::Mul(T A)
    {
        mat3<T> Result = *this * A;
        return Result;
    }

    template <typename T>
    vec3<T>
    mat3<T>::GetColumn(size_t Index)
    {
        if (Index < 0 || Index >= 3)
        {
            ASSERT(!"Index Out of bounds");
        }

        vec3<T> Result = vec3<T>{this->m[0][Index], this->m[1][Index], this->m[2][Index]};
        return Result;
    }

    template <typename T>
    void
    mat3<T>::SetColumn(u32 ColumnIndex, const vec3<T> &Column)
    {
        if (ColumnIndex < 0 || ColumnIndex >= 3)
        {
            ASSERT(!"Index Out of bounds");
        }

        this->m[0][ColumnIndex] = Column.x;
        this->m[1][ColumnIndex] = Column.y;
        this->m[2][ColumnIndex] = Column.z;
    }

    template <typename T>
    vec3<T>
    mat3<T>::GetRow(size_t Index)
    {
        if (Index < 0 || Index >= 3)
        {
            ASSERT(!"Index Out of bounds");
        }

        vec3<T> Result = this->Rows[Index];
        return Result;
    }

    template <typename T>
    T
    mat3<T>::Determinant() const
    {
        auto c00 =  m00 * (m11*m22 - m12*m21);
        auto c01 = -m01 * (m10*m22 - m12*m20);
        auto c02 =  m02 * (m10*m21 - m11*m20);

        auto Result = c00 + c01 + c02;
        return Result;
    }

    template<typename T>
    T
    mat3<T>::CoFactor(i32 row, i32 column) const
    {
        ASSERT(row >= 0 && row < 3);
        ASSERT(column >= 0 && column < 3);

        T Sign = (((row + column) & 0x1) == 0) ? (T)1 : (T)-1;

        i32 r1 = (row + 1) % 3;
        i32 r2 = (r1 + 1) % 3;
        if(r2 < r1) {
            SWAP(r1, r2);
        }
        i32 c1 = (column + 1) % 3;
        i32 c2 = (c1 + 1) % 3;
        if(c2 < c1) {
            SWAP(c1, c2);
        }

        T detMinor = (this->m[r1][c1]*this->m[r2][c2]) - (this->m[r1][c2]*this->m[r2][c1]);

        return detMinor * Sign;
    }

    template<typename T>
    mat3<T>
    mat3<T>::Adjoint() const
    {
        mat3<T> Result{};
        for(i32 rowIndex = 0; rowIndex < 3; ++rowIndex)
        {
            for(i32 colIndex = 0; colIndex < 3; ++colIndex)
            {
                Result.m[colIndex][rowIndex] = this->CoFactor(rowIndex, colIndex);
            }
        }
        return Result;
    }

    template <typename T>
    b32
    mat3<T>::IsIdentity()
    {
        b32 Result = true;

        for(i32 x = 0; x < 3; ++x)
        {
            for(i32 y = 0; y < 3; ++y)
            {
                if(x == y)
                {
                    b32 isOne = NearlyEqual(this->m[x][y], 1.0f);
                    if(!isOne) {
                        Result = false;
                        break;
                    }
                }
                else
                {
                    b32 isZero = NearlyEqual(this->m[x][y], 0.0f);
                    if(!isZero) {
                        Result = false;
                        break;
                    }
                }
            }
        }

        return Result;
    }

    template <typename T>
    b32
    mat3<T>::IsZero()
    {
        b32 Result = true;

        for(i32 x = 0; x < 3; ++x)
        {
            for(i32 y = 0; y < 3; ++y)
            {
                b32 isZero = NearlyEqual(this->m[x][y], 0.0f);
                if (!isZero)
                {
                    Result = false;
                    break;
                }
            }
        }

        return Result;
    }

    template<typename T>
    mat3<T>
    mat3<T>::Inverse() const
    {
        // NOTE: 3x3 matrices cannot be translational matrices.
        // ASSERT(this->Type != ZERO && this->Type != TRANSLATION);
        mat3<T> Result{};
        Result = this->Adjoint();
        auto det = this->Determinant();
        ASSERT(det != 0.0f);
        Result /= det;

#if 0
        switch(this->Type)
        {
            case matrix_type::IDENTITY:
            {
                Result.m00 = 1.0f / this->m00;
                Result.m11 = 1.0f / this->m11;
                Result.m22 = 1.0f / this->m22;
            } break;

            case matrix_type::ORTHOGONAL:
            {
                Result = Transpose(*this);
            } break;

            case matrix_type::NONE:
            {
                Result = this->Adjoint();
                auto det = this->Determinant();
                ASSERT(det != 0.0f);
                Result /= det;
            } break;

            SHU_INVALID_DEFAULT;
        }
        // TODO: Check if this is okay to do!
        Result.Type = this->Type;
#endif

        return Result;
    }

    template<typename T>
    mat3<T>
    mat3<T>::InverseTranspose() const
    {
        mat3<T> Result{};
        for (i32 rowIndex = 0; rowIndex < 3; ++rowIndex)
        {
            for (i32 colIndex = 0; colIndex < 3; ++colIndex)
            {
                Result.m[rowIndex][colIndex] = this->CoFactor(rowIndex, colIndex);
            }
        }

        auto det = this->Determinant();
        ASSERT(!NearlyEqualUlps(det, 0.0f, 1));
        Result /= det;

#if 0
        switch (this->Type)
        {
            case matrix_type::IDENTITY:
            {
                Result = this->Inverse();
            }
            break;

            case matrix_type::ORTHOGONAL:
            {
                Result = *this;
            }
            break;

            case matrix_type::NONE:
            {
                for(i32 rowIndex = 0; rowIndex < 3; ++rowIndex)
                {
                    for(i32 colIndex = 0; colIndex < 3; ++colIndex)
                    {
                        Result.m[rowIndex][colIndex] = this->CoFactor(rowIndex, colIndex);
                    }
                }

                auto det = this->Determinant();
                ASSERT(!NearlyEqualUlps(det, 0.0f, 1));
                Result /= det;
            } break;

            SHU_INVALID_DEFAULT;
        }
#endif

        return Result;
    }

    template <typename T>
    vec3<T> &
    mat3<T>::operator[](size_t RowIndex)
    {
        if (RowIndex < 0 || RowIndex >= 3)
        {
            ASSERT(!"Index Out of bounds");
        }

        return this->Rows[RowIndex * 3];
    }

    template <typename T>
    T &
    mat3<T>::operator()(size_t RowIndex, size_t ColumnIndex)
    {
        if (RowIndex < 0 || RowIndex >= 3)
        {
            ASSERT(!"Index Out of bounds");
        }
        if (ColumnIndex < 0 || ColumnIndex >= 3)
        {
            ASSERT(!"Index Out of bounds");
        }

        return this->E[RowIndex*3 + ColumnIndex];
    }

    template <typename T>
    mat3<T>
    Transpose(const mat3<T> &M)
    {
        mat3<T> Result = Mat3<T>(M.m00, M.m10, M.m20,
                                 M.m01, M.m11, M.m21,
                                 M.m02, M.m12, M.m22);

        return Result;
    }

    template <typename T>
    mat3<T>
    mat3<T>::Transposed() const
    {
        mat3<T> Result = Mat3<T>(this->m00, this->m10, this->m20,
                                 this->m01, this->m11, this->m21,
                                 this->m02, this->m12, this->m22);

        return Result;
    }

    template <typename T>
    mat3<T>
    mat3<T>::MakeTranspose()
    {
        *this = Mat3<T>(this->m00, this->m10, this->m20,
                        this->m01, this->m11, this->m21,
                        this->m02, this->m12, this->m22);

        return *this;
    }

    inline void
    DisplayMatrix(const mat3f &M)
    {
        LogInfoUnformatted("The Matrix is: \n");
        LogInfo("[%10.4f, %10.4f, %10.4f]\n", M.m00, M.m01, M.m02);
        LogInfo("|%10.4f, %10.4f, %10.4f|\n", M.m10, M.m11, M.m12);
        LogInfo("[%10.4f, %10.4f, %10.4f]\n", M.m20, M.m21, M.m22);
        LogInfoUnformatted("\n");
    }

    inline void
    DisplayMatrix(const mat3i &M)
    {
        LogInfoUnformatted("The Matrix is: \n");
        LogInfo("[%6d, %6d, %6d]\n", M.m00, M.m01, M.m02);
        LogInfo("|%6d, %6d, %6d|\n", M.m10, M.m11, M.m12);
        LogInfo("[%6d, %6d, %6d]\n", M.m20, M.m21, M.m22);
        LogInfoUnformatted("\n");
    }

    inline void
    DisplayMatrix(const mat3u &M)
    {
        LogInfoUnformatted("The Matrix is: \n");
        LogInfo("[%6u, %6u, %6u]\n", M.m00, M.m01, M.m02);
        LogInfo("|%6u, %6u, %6u|\n", M.m10, M.m11, M.m12);
        LogInfo("[%6u, %6u, %6u]\n", M.m20, M.m21, M.m22);
        LogInfoUnformatted("\n");
    }

    // -------------------------------------------------------------------------------------------------
    // Matrix4x4
    // -------------------------------------------------------------------------------------------------
    template <typename T>
    mat3<T>
    mat4<T>::ToMat3()
    {
        mat3<T> M = Mat3<T>(this->m00, this->m01, this->m02,
                            this->m10, this->m11, this->m12,
                            this->m20, this->m21, this->m22);

        return M;
    }

    template <typename T>
    mat4<T>
    Mat4()
    {
        mat4<T> Result = {};
        return Result;
    }

    template <typename T>
    mat4<T>
    Mat4(T Val)
    {
        mat4<T> Result = {};
        Result.m00 = Val;
        Result.m11 = Val;
        Result.m22 = Val;
        Result.m33 = Val;
        return Result;
    }

    template <typename T>
    mat4<T>
    Mat4(T m00, T m01, T m02, T m03,
         T m10, T m11, T m12, T m13,
         T m20, T m21, T m22, T m23,
         T m30, T m31, T m32, T m33)
    {
        mat4<T> Result;

        Result.m00 = m00;
        Result.m01 = m01;
        Result.m02 = m02;
        Result.m03 = m03;

        Result.m10 = m10;
        Result.m11 = m11;
        Result.m12 = m12;
        Result.m13 = m13;

        Result.m20 = m20;
        Result.m21 = m21;
        Result.m22 = m22;
        Result.m23 = m23;

        Result.m30 = m30;
        Result.m31 = m31;
        Result.m32 = m32;
        Result.m33 = m33;

        return Result;
    }

    template <typename T>
    mat4<T>
    Mat4(const vec4<T> &r0, const vec4<T> &r1, const vec4<T> &r2, const vec4<T> &r3)
    {
        mat4<T> Result;

        Result.Rows[0] = r0;
        Result.Rows[1] = r1;
        Result.Rows[2] = r2;
        Result.Rows[3] = r3;

        return Result;
    }

    template <typename T>
    mat4<T>
    operator+(const mat4<T> &M1, const mat4<T> &M2)
    {
        mat4<T> Result = {};

        for(u32 i = 0; i < 16; ++i)
        {
            Result.E[i] = M1.E[i] + M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat4<T>
    operator-(const mat4<T> &M1, const mat4<T> &M2)
    {
        mat4<T> Result = {};

        for(u32 i = 0; i < 16; ++i)
        {
            Result.E[i] = M1.E[i] - M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat4<T>
    operator*(const mat4<T> &M1, T B)
    {
        mat4<T> Result = {};

        for(u32 i = 0; i < 16; ++i)
        {
            Result.E[i] = M1.E[i]*B;
        }

        return Result;
    }

    template <typename T>
    vec4<T>
    operator*(const mat4<T> &M, const vec4<T> &V)
    {
        vec4<T> Result;

        Result.x = shu::Dot(M.Row0, V);
        Result.y = shu::Dot(M.Row1, V);
        Result.z = shu::Dot(M.Row2, V);
        Result.w = shu::Dot(M.Row3, V);

        return Result;
    }

    template <typename T>
    vec4<T>
    operator*(const vec4<T> &V, const mat4<T> &M)
    {
        vec4<T> Result;

        Result.x = M.m00*V.x + M.m10*V.y + M.m20*V.z + M.m30*V.w;
        Result.y = M.m01*V.x + M.m11*V.y + M.m21*V.z + M.m31*V.w;
        Result.z = M.m02*V.x + M.m12*V.y + M.m22*V.z + M.m32*V.w;
        Result.w = M.m03*V.x + M.m13*V.y + M.m23*V.z + M.m33*V.w;

        return Result;
    }

    template <typename T>
    vec4<T>
    operator*(const mat4<T> &M, const vec3<T> &V)
    {
        vec4<T> v4 = shu::Vec4f(V, 1.0f);

        vec4<T> Result;

        Result.x = shu::Dot(M.Row0, v4);
        Result.y = shu::Dot(M.Row1, v4);
        Result.z = shu::Dot(M.Row2, v4);
        Result.w = shu::Dot(M.Row3, v4);

        return Result;
    }


    template <typename T>
    mat4<T>
    operator*(const mat4<T> &M1, const mat4<T> &M2)
    {
        mat4<T> Result = {};

        for(u32 i = 0; i < 4; ++i)
        {
            for(u32 j = 0; j < 4; ++j)
            {
                for(u32 k = 0; k < 4; ++k)
                {
                    Result.m[i][j] += M1.m[i][k] * M2.m[k][j];
                }
            }
        }

        return Result;
    }

    template <typename T>
    mat4<T>
    operator/(const mat4<T> &M1, T B)
    {
        ASSERT(B != (T)0);
        mat4<T> Result = {};

        for(u32 i = 0; i < 16; ++i)
        {
            Result.E[i] = M1.E[i] / B;
        }

        return Result;
    }

    template <typename T>
    mat4<T>
    operator/(const mat4<T> &M1, const mat4<T> &M2)
    {
        mat4<T> Result = {};

        for(u32 i = 0; i < 16; ++i)
        {
            ASSERT(M2.E[i] != (T)0);
            Result.E[i] = M1.E[i] / M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat4<T>
    mat4<T>::operator+=(const mat4<T> &M)
    {
        *this = *this + M;
        return *this;
    }

    template <typename T>
    mat4<T>
    mat4<T>::operator-=(const mat4<T> &M)
    {
        *this = *this - M;
        return *this;
    }

    template <typename T>
    mat4<T>
    mat4<T>::operator*=(const mat4<T> &M)
    {
        *this = *this * M;
        return *this;
    }

    template <typename T>
    mat4<T>
    mat4<T>::operator*=(T A)
    {
        *this = *this * A;
        return *this;
    }

    template <typename T>
    mat4<T>
    mat4<T>::operator/=(const mat4<T> &M)
    {
        *this = *this / M;
        return *this;
    }

    template <typename T>
    mat4<T>
    mat4<T>::operator/=(T A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    mat4<T>
    mat4<T>::Mul(const mat4<T> &M)
    {
        mat4<T> Result = *this * M;
        return Result;
    }

    template <typename T>
    mat4<T>
    mat4<T>::Mul(T A)
    {
        mat4<T> Result = *this * A;
        return Result;
    }

    template <typename T>
    vec4<T>
    mat4<T>::MulVec(const vec4<T> &V)
    {
        vec4<T> Result = V * (*this);
        return Result;
    }

    template <typename T>
    vec4<T>
    mat4<T>::MulVecDir(const vec3<T> &V)
    {
        vec4<T> Result = V * (*this);
        return Result;
    }

    template <typename T>
    vec4<T>
    mat4<T>::GetColumn(size_t Index)
    {
        if(Index < 0 || Index >= 4)
        {
            ASSERT(!"Index Out of bounds");
        }

        vec4<T> Result = vec4<T>{this->m[0][Index], this->m[1][Index], this->m[2][Index], this->m[3][Index]};
        return Result;
    }

    template <typename T>
    vec4<T>
    mat4<T>::GetRow(size_t Index)
    {
        if(Index < 0 || Index >= 4)
        {
            ASSERT(!"Index Out of bounds");
        }

        vec4<T> Result = this->Rows[Index];
        return Result;
    }

    template <typename T>
    vec4<T>&
    mat4<T>::operator[](size_t RowIndex)
    {
        if(RowIndex < 0 || RowIndex >= 4)
        {
            ASSERT(!"Index Out of bounds");
        }

        return this->Rows[RowIndex*4];
    }

    template <typename T>
    T&
    mat4<T>::operator()(size_t RowIndex, size_t ColumnIndex)
    {
        if(RowIndex < 0 || RowIndex >= 4)
        {
            ASSERT(!"Index Out of bounds");
        }
        if(ColumnIndex < 0 || ColumnIndex >= 4)
        {
            ASSERT(!"Index Out of bounds");
        }

        return this->E[RowIndex*4 + ColumnIndex];
    }

    template <typename T>
    mat4<T>
    Transpose(const mat4<T> &M)
    {
        mat4<T> Result = Mat4<T>(M.m00, M.m10, M.m20, M.m30,
                                 M.m01, M.m11, M.m21, M.m31,
                                 M.m02, M.m12, M.m22, M.m32,
                                 M.m03, M.m13, M.m23, M.m33);
        return Result;
    }

    template <typename T>
    mat4<T>
    mat4<T>::Transposed()
    {
        mat4<T> Result = Mat4<T>(this->m00, this->m10, this->m20, this->m30,
                                 this->m01, this->m11, this->m21, this->m31,
                                 this->m02, this->m12, this->m22, this->m32,
                                 this->m03, this->m13, this->m23, this->m33);
        return Result;
    }

    template <typename T>
    mat4<T>
    mat4<T>::MakeTranspose()
    {
        *this = Mat4<T>(this->m00, this->m10, this->m20, this->m30,
                        this->m01, this->m11, this->m21, this->m31,
                        this->m02, this->m12, this->m22, this->m32,
                        this->m03, this->m13, this->m23, this->m33);
        return *this;
    }

    template <typename T>
    mat3<T>
    mat4<T>::Minor(i32 i, i32 j) const
    {
        ASSERT(i >= 0 && i < 4);
        ASSERT(j >= 0 && j < 4);

        mat3<T> Result{};
        i32 yy = 0;
        for (i32 y = 0; y < 4; ++y)
        {
            if(y == j) continue;
            i32 xx = 0;
            for (i32 x = 0; x < 4; ++x)
            {
                if(x == i) continue;
                Result.m[xx][yy] = this->m[x][y];
                ++xx;
            }
            ++yy;
        }

        return Result;
    }

    template <typename T>
    T
    mat4<T>::CoFactor(i32 i, i32 j) const
    {
        T Sign = ((i + j) & 0x1) ? (T)-1 : (T)1;

        T Result = Sign * Minor(i, j).Determinant();
        return Result;
    }

    template <typename T>
    T
    mat4<T>::Determinant() const
    {
        T Result = (T)0;
        for(i32 colIndex = 0; colIndex < 4; ++colIndex)
        {
            T Sign = colIndex & 0x1 ? (T)-1 : (T)1;

            T currElement = this->m[0][colIndex];
            if(!NearlyEqual(currElement, 0.0f))
            {
                auto Minor = this->Minor(0, colIndex);
                Result += (Sign * currElement * Minor.Determinant());
            }
        }

        return Result;
    }

    template <typename T>
    mat4<T>
    mat4<T>::Adjoint() const
    {
        mat4<T> Result;
        for(i32 x = 0; x < 4; ++x)
        {
            for(i32 y = 0; y < 4; ++y)
            {
                Result.m[y][x] = CoFactor(x, y);
            }
        }

        return Result;
    }

    template <typename T>
    mat4<T>
    mat4<T>::Inverse() const
    {
        mat4<T> Result;
        Result = this->Adjoint();
        auto det = this->Determinant();
        ASSERT(det != 0.0f);
        Result /= det;

#if 0
        switch(this->Type)
        {
            case matrix_type::IDENTITY:
            {
                Result.m00 = 1.0f / Result.m00;
                Result.m11 = 1.0f / Result.m11;
                Result.m22 = 1.0f / Result.m22;
                Result.m33 = 1.0f / Result.m33;
            } break;

            case matrix_type::ORTHOGONAL:
            {
                Result = Transpose(*this);
            } break;

            case matrix_type::TRANSLATION:
            {
                Result.m30 = -Result.m30;
                Result.m31 = -Result.m31;
                Result.m32 = -Result.m32;
            } break;

            case matrix_type::NONE:
            {
                Result = this->Adjoint();
                auto det = this->Determinant();
                ASSERT(det != 0.0f);
                Result /= det;
            } break;

            SHU_INVALID_DEFAULT;
        }
#endif

        return Result;
    }

    template <typename T>
    mat4<T>
    mat4<T>::InverseTranspose() const
    {
        mat4<T> Result;
        for (i32 x = 0; x < 4; ++x)
        {
            for (i32 y = 0; y < 4; ++y)
            {
                Result.m[x][y] = CoFactor(x, y);
            }
        }

        auto det = this->Determinant();
        Result /= det;
#if 0
        switch (this->Type)
        {
            case matrix_type::IDENTITY:
            {
                Result = this->Inverse();
            }
            break;

            case matrix_type::ORTHOGONAL:
            {
                Result = *this;
            }
            break;

            case matrix_type::TRANSLATION:
            {
                Result = Transpose(this->Inverse());
            } break;

            case matrix_type::NONE:
            {
                for(i32 x = 0; x < 4; ++x)
                {
                    for(i32 y = 0; y < 4; ++y)
                    {
                        Result.m[x][y] = CoFactor(x, y);
                    }
                }

                auto det = this->Determinant();
                Result /= det;
            } break;

            SHU_INVALID_DEFAULT;
        }
#endif
        return Result;
    }


    inline void
    DisplayMatrix(const mat4f &M)
    {
        LogInfoUnformatted("The Matrix is: \n");
        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f]\n", M.m00, M.m01, M.m02, M.m03);
        LogInfo("|%10.4f, %10.4f, %10.4f, %10.4f|\n", M.m10, M.m11, M.m12, M.m13);
        LogInfo("|%10.4f, %10.4f, %10.4f, %10.4f|\n", M.m20, M.m21, M.m22, M.m23);
        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f]\n", M.m30, M.m31, M.m32, M.m33);
        LogInfoUnformatted("\n");
    }

    inline void
    DisplayMatrix(const mat4i &M)
    {
        LogInfoUnformatted("The Matrix is: \n");
        LogInfo("[%6d, %6d, %6d, %6d]\n", M.m00, M.m01, M.m02, M.m03);
        LogInfo("|%6d, %6d, %6d, %6d|\n", M.m10, M.m11, M.m12, M.m13);
        LogInfo("|%6d, %6d, %6d, %6d|\n", M.m20, M.m21, M.m22, M.m23);
        LogInfo("[%6d, %6d, %6d, %6d]\n", M.m30, M.m31, M.m32, M.m33);
        LogInfoUnformatted("\n");
    }

    inline void
    DisplayMatrix(const mat4u &M)
    {
        LogInfoUnformatted("The Matrix is: \n");
        LogInfo("[%6u, %6u, %6u, %6u]\n", M.m00, M.m01, M.m02, M.m03);
        LogInfo("|%6u, %6u, %6u, %6u|\n", M.m10, M.m11, M.m12, M.m13);
        LogInfo("|%6u, %6u, %6u, %6u|\n", M.m20, M.m21, M.m22, M.m23);
        LogInfo("[%6u, %6u, %6u, %6u]\n", M.m30, M.m31, M.m32, M.m33);
        LogInfoUnformatted("\n");
    }

    // -------------------------------------------------------------------------------------------------
    // Matrix6x6
    // -------------------------------------------------------------------------------------------------
    template <typename T>
    mat6<T>
    Mat6()
    {
        mat6<T> Result = {};
        return Result;
    }

    template <typename T>
    mat6<T>
    Mat6(T Val)
    {
        mat6<T> Result = {};
        Result.m00 = Val;
        Result.m11 = Val;
        Result.m22 = Val;
        Result.m33 = Val;
        Result.m44 = Val;
        Result.m55 = Val;

        return Result;
    }

    template <typename T>
    mat6<T>
    Mat6(T m00, T m01, T m02, T m03, T m04, T m05,
         T m10, T m11, T m12, T m13, T m14, T m15,
         T m20, T m21, T m22, T m23, T m24, T m25,
         T m30, T m31, T m32, T m33, T m34, T m35,
         T m40, T m41, T m42, T m43, T m44, T m45,
         T m50, T m51, T m52, T m53, T m54, T m55)
    {
        mat6<T> Result;

        Result.m00 = m00;
        Result.m01 = m01;
        Result.m02 = m02;
        Result.m03 = m03;
        Result.m04 = m04;
        Result.m05 = m05;

        Result.m10 = m10;
        Result.m11 = m11;
        Result.m12 = m12;
        Result.m13 = m13;
        Result.m14 = m14;
        Result.m15 = m15;

        Result.m20 = m20;
        Result.m21 = m21;
        Result.m22 = m22;
        Result.m23 = m23;
        Result.m24 = m24;
        Result.m25 = m25;

        Result.m30 = m30;
        Result.m31 = m31;
        Result.m32 = m32;
        Result.m33 = m33;
        Result.m34 = m34;
        Result.m35 = m35;

        Result.m40 = m40;
        Result.m41 = m41;
        Result.m42 = m42;
        Result.m43 = m43;
        Result.m44 = m44;
        Result.m45 = m45;

        Result.m50 = m50;
        Result.m51 = m51;
        Result.m52 = m52;
        Result.m53 = m53;
        Result.m54 = m54;
        Result.m55 = m55;

        return Result;
    }

    template <typename T>
    mat6<T>
    Mat6(const vec6<T> &r0, const vec6<T> &r1, const vec6<T> &r2,
         const vec6<T> &r3, const vec6<T> &r4, const vec6<T> &r5)
    {
        mat6<T> Result;

        Result.Rows[0] = r0;
        Result.Rows[1] = r1;
        Result.Rows[2] = r2;

        Result.Rows[3] = r3;
        Result.Rows[4] = r4;
        Result.Rows[5] = r5;

        return Result;
    }

    template <typename T>
    mat6<T>
    operator+(const mat6<T> &M1, const mat6<T> &M2)
    {
        mat6<T> Result = {};

        for(u32 i = 0; i < 36; ++i)
        {
            Result.E[i] = M1.E[i] + M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat6<T>
    operator-(const mat6<T> &M1, const mat6<T> &M2)
    {
        mat6<T> Result = {};

        for(u32 i = 0; i < 36; ++i)
        {
            Result.E[i] = M1.E[i] - M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat6<T>
    operator*(const mat6<T> &M1, T B)
    {
        mat6<T> Result = {};

        for(u32 i = 0; i < 36; ++i)
        {
            Result.E[i] = M1.E[i]*B;
        }

        return Result;
    }

    template <typename T>
    vec6<T>
    operator*(const mat6<T> &M, const vec6<T> &V)
    {
        vec6<T> Result;

        Result.x1 = M.m00*V.x1 + M.m10*V.y1 + M.m20*V.z1 + M.m30*V.x2 + M.m40*V.y2 + M.m50*V.z2;
        Result.y1 = M.m01*V.x1 + M.m11*V.y1 + M.m21*V.z1 + M.m31*V.x2 + M.m41*V.y2 + M.m51*V.z2;
        Result.z1 = M.m02*V.x1 + M.m12*V.y1 + M.m22*V.z1 + M.m32*V.x2 + M.m42*V.y2 + M.m52*V.z2;
        Result.x2 = M.m03*V.x1 + M.m13*V.y1 + M.m23*V.z1 + M.m33*V.x2 + M.m43*V.y2 + M.m53*V.z2;
        Result.y2 = M.m04*V.x1 + M.m14*V.y1 + M.m24*V.z1 + M.m34*V.x2 + M.m44*V.y2 + M.m54*V.z2;
        Result.z2 = M.m05*V.x1 + M.m15*V.y1 + M.m25*V.z1 + M.m35*V.x2 + M.m45*V.y2 + M.m55*V.z2;

        return Result;
    }

    template <typename T>
    vec6<T>
    operator*(const vec6<T> &V, const mat6<T> &M)
    {
        vec6<T> Result;

        Result.x1 = M.m00*V.x1 + M.m10*V.y1 + M.m20*V.z1 + M.m30*V.x2 + M.m40*V.y2 + M.m50*V.z2;
        Result.y1 = M.m01*V.x1 + M.m11*V.y1 + M.m21*V.z1 + M.m31*V.x2 + M.m41*V.y2 + M.m51*V.z2;
        Result.z1 = M.m02*V.x1 + M.m12*V.y1 + M.m22*V.z1 + M.m32*V.x2 + M.m42*V.y2 + M.m52*V.z2;
        Result.x2 = M.m03*V.x1 + M.m13*V.y1 + M.m23*V.z1 + M.m33*V.x2 + M.m43*V.y2 + M.m53*V.z2;
        Result.y2 = M.m04*V.x1 + M.m14*V.y1 + M.m24*V.z1 + M.m34*V.x2 + M.m44*V.y2 + M.m54*V.z2;
        Result.z2 = M.m05*V.x1 + M.m15*V.y1 + M.m25*V.z1 + M.m35*V.x2 + M.m45*V.y2 + M.m55*V.z2;

        return Result;
    }

    template <typename T>
    mat6<T>
    operator*(const mat6<T> &M1, const mat6<T> &M2)
    {
        mat6<T> Result = {};

        for(u32 i = 0; i < 6; ++i)
        {
            for(u32 j = 0; j < 6; ++j)
            {
                for(u32 k = 0; k < 6; ++k)
                {
                    Result.m[i][j] += M1.m[i][k] * M2.m[k][j];
                }
            }
        }

        return Result;
    }

    template <typename T>
    mat6<T>
    operator/(const mat6<T> &M1, T B)
    {
        ASSERT(B != (T)0);
        mat6<T> Result = {};

        for(u32 i = 0; i < 36; ++i)
        {
            Result.E[i] = M1.E[i] / B;
        }

        return Result;
    }

    template <typename T>
    mat6<T>
    operator/(const mat6<T> &M1, const mat6<T> &M2)
    {
        mat6<T> Result = {};

        for(u32 i = 0; i < 36; ++i)
        {
            ASSERT(M2.E[i] != (T)0);
            Result.E[i] = M1.E[i] / M2.E[i];
        }

        return Result;
    }

    template <typename T>
    mat6<T>
    mat6<T>::operator+=(const mat6<T> &M)
    {
        *this = *this + M;
        return *this;
    }

    template <typename T>
    mat6<T>
    mat6<T>::operator-=(const mat6<T> &M)
    {
        *this = *this - M;
        return *this;
    }

    template <typename T>
    mat6<T>
    mat6<T>::operator*=(const mat6<T> &M)
    {
        *this = *this * M;
        return *this;
    }

    template <typename T>
    mat6<T>
    mat6<T>::operator*=(T A)
    {
        *this = *this * A;
        return *this;
    }

    template <typename T>
    mat6<T>
    mat6<T>::operator/=(const mat6<T> &M)
    {
        *this = *this / M;
        return *this;
    }

    template <typename T>
    mat6<T>
    mat6<T>::operator/=(T A)
    {
        *this = *this / A;
        return *this;
    }

    template <typename T>
    mat6<T>
    mat6<T>::Mul(const mat6<T> &M)
    {
        mat6<T> Result = *this * M;
        return Result;
    }

    template <typename T>
    mat6<T>
    mat6<T>::Mul(T A)
    {
        mat6<T> Result = *this * A;
        return Result;
    }

    template <typename T>
    vec6<T>
    mat6<T>::MulVec(const vec6<T> &V)
    {
        vec6<T> Result = V * (*this);
        return Result;
    }

    template <typename T>
    vec6<T>
    mat6<T>::GetColumn(size_t Index)
    {
        if(Index < 0 || Index > 5)
        {
            ASSERT(!"Index Out of bounds");
        }

        vec6<T> Result = vec6<T>{this->m[0][Index], this->m[1][Index], this->m[2][Index],
                                 this->m[3][Index], this->m[4][Index], this->m[5][Index]};
        return Result;
    }

    template <typename T>
    vec6<T>
    mat6<T>::GetRow(size_t Index)
    {
        if(Index < 0 || Index > 5)
        {
            ASSERT(!"Index Out of bounds");
        }

        vec6<T> Result = this->Rows[Index];
        return Result;
    }

    template <typename T>
    vec6<T>&
    mat6<T>::operator[](size_t RowIndex)
    {
        if(RowIndex < 0 || RowIndex > 5)
        {
            ASSERT(!"Index Out of bounds");
        }

        return this->Rows[RowIndex];
    }

    template <typename T>
    T&
    mat6<T>::operator()(size_t RowIndex, size_t ColumnIndex)
    {
        if(RowIndex < 0 || RowIndex > 5)
        {
            ASSERT(!"Index Out of bounds");
        }
        if(ColumnIndex < 0 || ColumnIndex > 5)
        {
            ASSERT(!"Index Out of bounds");
        }

        return this->E[RowIndex*6 + ColumnIndex];
    }

    template <typename T>
    mat6<T>
    Transpose(const mat6<T> &M)
    {
        mat6<T> Result = Mat6<T>(M.m00, M.m10, M.m20, M.m30, M.m40, M.m50,
                                 M.m01, M.m11, M.m21, M.m31, M.m41, M.m51,
                                 M.m02, M.m12, M.m22, M.m32, M.m42, M.m52,
                                 M.m03, M.m13, M.m23, M.m33, M.m43, M.m53,
                                 M.m04, M.m14, M.m24, M.m34, M.m44, M.m54,
                                 M.m05, M.m15, M.m25, M.m35, M.m45, M.m55);
        return Result;
    }

    template <typename T>
    mat6<T>
    mat6<T>::Transposed()
    {
        mat6<T> Result = Mat6<T>(this->m00, this->m10, this->m20, this->m30, this->m40, this->m50,
                                 this->m01, this->m11, this->m21, this->m31, this->m41, this->m51,
                                 this->m02, this->m12, this->m22, this->m32, this->m42, this->m52,
                                 this->m03, this->m13, this->m23, this->m33, this->m43, this->m53,
                                 this->m04, this->m14, this->m24, this->m34, this->m44, this->m54,
                                 this->m05, this->m15, this->m25, this->m35, this->m45, this->m55);
        return Result;
    }

    template <typename T>
    mat6<T>
    mat6<T>::MakeTranspose()
    {
        *this = Mat6<T>(this->m00, this->m10, this->m20, this->m30, this->m40, this->m50,
                        this->m01, this->m11, this->m21, this->m31, this->m41, this->m51,
                        this->m02, this->m12, this->m22, this->m32, this->m42, this->m52,
                        this->m03, this->m13, this->m23, this->m33, this->m43, this->m53,
                        this->m04, this->m14, this->m24, this->m34, this->m44, this->m54,
                        this->m05, this->m15, this->m25, this->m35, this->m45, this->m55);
        return *this;
    }

    inline void
    DisplayMatrix(const mat6f &M)
    {
        LogInfoUnformatted("The Matrix is: \n");

        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f, %10.4f, %10.4f]\n", M.m00, M.m10, M.m20, M.m30, M.m40, M.m50);
        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f, %10.4f, %10.4f]\n", M.m01, M.m11, M.m21, M.m31, M.m41, M.m51);
        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f, %10.4f, %10.4f]\n", M.m02, M.m12, M.m22, M.m32, M.m42, M.m52);
        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f, %10.4f, %10.4f]\n", M.m03, M.m13, M.m23, M.m33, M.m43, M.m53);
        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f, %10.4f, %10.4f]\n", M.m04, M.m14, M.m24, M.m34, M.m44, M.m54);
        LogInfo("[%10.4f, %10.4f, %10.4f, %10.4f, %10.4f, %10.4f]\n", M.m05, M.m15, M.m25, M.m35, M.m45, M.m55);

        LogInfoUnformatted("\n");
    }

    inline void
    DisplayMatrix(const mat6i &M)
    {
        LogInfoUnformatted("The Matrix is: \n");

        LogInfo("[%6d, %6d, %6d, %6d, %6d, %6d]\n", M.m00, M.m10, M.m20, M.m30, M.m40, M.m50);
        LogInfo("[%6d, %6d, %6d, %6d, %6d, %6d]\n", M.m01, M.m11, M.m21, M.m31, M.m41, M.m51);
        LogInfo("[%6d, %6d, %6d, %6d, %6d, %6d]\n", M.m02, M.m12, M.m22, M.m32, M.m42, M.m52);
        LogInfo("[%6d, %6d, %6d, %6d, %6d, %6d]\n", M.m03, M.m13, M.m23, M.m33, M.m43, M.m53);
        LogInfo("[%6d, %6d, %6d, %6d, %6d, %6d]\n", M.m04, M.m14, M.m24, M.m34, M.m44, M.m54);
        LogInfo("[%6d, %6d, %6d, %6d, %6d, %6d]\n", M.m05, M.m15, M.m25, M.m35, M.m45, M.m55);


        LogInfoUnformatted("\n");
    }

    inline void
    DisplayMatrix(const mat6u &M)
    {
        LogInfoUnformatted("The Matrix is: \n");

        LogInfo("[%6u, %6u, %6u, %6u, %6u, %6u]\n", M.m00, M.m10, M.m20, M.m30, M.m40, M.m50);
        LogInfo("[%6u, %6u, %6u, %6u, %6u, %6u]\n", M.m01, M.m11, M.m21, M.m31, M.m41, M.m51);
        LogInfo("[%6u, %6u, %6u, %6u, %6u, %6u]\n", M.m02, M.m12, M.m22, M.m32, M.m42, M.m52);
        LogInfo("[%6u, %6u, %6u, %6u, %6u, %6u]\n", M.m03, M.m13, M.m23, M.m33, M.m43, M.m53);
        LogInfo("[%6u, %6u, %6u, %6u, %6u, %6u]\n", M.m04, M.m14, M.m24, M.m34, M.m44, M.m54);
        LogInfo("[%6u, %6u, %6u, %6u, %6u, %6u]\n", M.m05, M.m15, M.m25, M.m35, M.m45, M.m55);


        LogInfoUnformatted("\n");
    }

    // --------------------------------------------------------------------------------------
    // MatN - N X N
    // --------------------------------------------------------------------------------------
    template <typename T, size_t N>
    matN<T, N>::matN()
    {
        this->Zero();
    }

    template <typename T, size_t N>
    matN<T, N>::matN(const matN &rhs)
    {
        *this = rhs;
    }

    template <typename T, size_t N>
    template <typename... Args>
    matN<T, N>::matN(Args... args)
    {
        size_t numArgs = sizeof...(args);
        T tempArray[N*N] = {static_cast<T>(args)...};
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                this->Data[i][j] = tempArray[i*N + j];
            }
        }
    }

    template <typename T, size_t N>
    matN<T, N>::~matN()
    {
        // LogInfoUnformatted("MatN Destructor called!\n");
    }

    template <typename T, size_t N>
    vecN<T, N>
    matN<T, N>::GetColumn(i32 Index) const
    {
        ASSERT(Index < N);
        vecN<T, N> Result;
        Result.Zero();

        for(i32 i = 0; i < N; ++i)
        {
            Result.Data[i] = this->Data[i][Index];
        }

        return Result;
    }

    template <typename T, size_t N>
    const matN<T, N> &
    matN<T, N>::operator=(const matN<T, N> &rhs)
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Rows[i] = rhs.Rows[i];
        }

        return *this;
    }

    template <typename T, size_t N>
    void
    matN<T, N>::Identity()
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Rows[i].Zero();
            this->Rows[i][i] = (T)1;
        }
    }

    template <typename T, size_t N>
    void
    matN<T, N>::Zero()
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Rows[i].Zero();
        }
    }

    template <typename T, size_t N>
    void
    matN<T, N>::Transpose()
    {
        matN<T, N> temp;
        for (i32 i = 0; i < N; ++i)
        {
            for (i32 j = 0; j < N; ++j)
            {
                temp.Data[i][j] = this->Data[j][i];
            }
        }

        *this = temp;
    }

    template <typename T, size_t N>
    matN<T, N>
    matN<T, N>::Transposed() const
    {
        matN<T, N> Result;
        for (i32 i = 0; i < N; ++i)
        {
            for (i32 j = 0; j < N; ++j)
            {
                Result.Data[i][j] = this->Data[j][i];
            }
        }

        return Result;
    }

    template <typename T, size_t N>
    b32
    matN<T,N>::IsDiagonallyDominant() const
    {
        b32 Result = true;

        for(i32 rowIndex = 0; rowIndex < N; ++rowIndex)
        {
            T diagVal = SHU_ABSOLUTE(this->Data[rowIndex][rowIndex]);

            T sum = (T)0;
            for (i32 colIndex = 0; colIndex < N; ++colIndex)
            {
                if(colIndex == rowIndex) continue;

                sum += SHU_ABSOLUTE(this->Data[rowIndex][colIndex]);
            }

            if(diagVal < sum) {
                Result = false;
                break;
            }
        }

        return Result;
    }


    template <typename T, size_t N>
    void
    matN<T, N>::operator*=(T rhs)
    {
        for (i32 i = 0; i < N; ++i)
        {
            this->Rows[i] *= rhs;
        }
    }

    template <typename T, size_t N>
    vecN<T, N>
    matN<T, N>::operator*(const vecN<T, N> &rhs) const
    {
        vecN<T, N> Result;
        for (i32 i = 0; i < N; ++i)
        {
            Result[i] = (T)0;
            for (i32 j = 0; j < N; ++j)
            {
                Result[i] += rhs[j] * this->Data[j][i];
            }
        }

        return Result;
    }

    template <typename T, size_t N>
    matN<T, N>
    matN<T, N>::operator*(const matN<T, N> &rhs) const
    {
        matN Result;
        for (i32 i = 0; i < N; ++i)
        {
            for (i32 j = 0; j < N; ++j)
            {
                for (i32 k = 0; k < N; ++k)
                {
                    Result.Data[i][j] += this->Data[i][k] * rhs.Data[k][j];
                }
            }
        }

        return Result;
    }

    // --------------------------------------------------------------------------------------
    // MatMN - M X N Matrix
    // --------------------------------------------------------------------------------------

    template<typename T, size_t M, size_t N>
    matMN<T, M, N>::matMN() { this->Zero(); }

    template<typename T, size_t M, size_t N>
    matMN<T, M, N>::matMN(const matMN &rhs) { *this = rhs; }

    template<typename T, size_t M, size_t N>
    matMN<T, M, N>::matMN(const matN<T, M> &rhs)
    {
        ASSERT(M < N);
        this->Zero();
        for (i32 i = 0; i < M; ++i)
        {
            for (i32 j = 0; j < M; ++j)
            {
                this->Data[i][j] = rhs.Data[i][j];
            }
        }
    }

    template<typename T, size_t M, size_t N>
    template <typename... Args>
    matMN<T, M, N>::matMN(Args... args)
    {
        size_t numArgs = sizeof...(args);
        T tempArray[M * N] = {static_cast<T>(args)...};
        for (size_t i = 0; i < M; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                size_t index = i * N + j;
                this->Data[i][j] = tempArray[index];
            }
        }
    }

    template<typename T, size_t M, size_t N>
    matMN<T, M, N>::~matMN()
    {
        // LogInfoUnformatted("MatMN destructor called!\n");
    }

    template <typename T, size_t M, size_t N>
    void
    matMN<T, M, N>::Zero()
    {
        for (i32 i = 0; i < M; ++i)
        {
            for (i32 j = 0; j < N; ++j)
            {
                this->Data[i][j] = (T)0;
            }
        }
    }

    template <typename T, size_t M, size_t N>
    void
    matMN<T, M, N>::AddColumn(i32 columnIndex, const vecN<T, M> &v)
    {
        ASSERT(columnIndex < N);
        for(i32 i = 0; i < M; ++i)
        {
            this->Data[i][columnIndex] = v.Data[i];
        }
    }

    template <typename T, size_t M, size_t N>
    matMN<T, N, M>
    matMN<T, M, N>::Transposed() const
    {
        matMN<T, N, M> Result;

        for (i32 i = 0; i < M; ++i)
        {
            for (i32 j = 0; j < N; ++j)
            {
                T value = this->Data[i][j];
                Result.Data[j][i] = value;
            }
        }

        return Result;
    }

    template <typename T, size_t M, size_t N>
    void RowMultAdd(matMN<T, M, N> &Mat, i32 ModRowIndex, i32 RefRowIndex, T factor)
    {
        ASSERT(ModRowIndex >= 0 && ModRowIndex < M);
        ASSERT(RefRowIndex >= 0 && RefRowIndex < M);

        for(i32 colIndex = 0; colIndex < N; ++colIndex)
        {
            T refVal = Mat.Data[RefRowIndex][colIndex];
            T currVal = Mat.Data[ModRowIndex][colIndex];

            Mat.Data[ModRowIndex][colIndex] = currVal + factor * refVal;
        }
    }

    template <typename T, size_t M, size_t N>
    b32
    matMN<T, M, N>::IsRowEchelon() const
    {
        b32 Result = false;

        // Proving that lower diagonal elements are all zero.
        T sum = (T)0;
        for(i32 rowIndex = 1; rowIndex < M; ++rowIndex)
        {
            for(i32 colIndex = 0; colIndex < rowIndex; ++colIndex)
            {
                sum += this->Data[rowIndex][colIndex];
            }
        }

        Result = NearlyEqual(sum, 0.0f);
        return Result;
    }


    template <typename T, size_t M, size_t N>
    matMN<T, M, N>
    matMN<T, M, N>::RowEchelon() const
    {
        matMN<T, M, N> Result = *this;
        if(N < M) {
            ASSERT(!"The matrix must have at least as many columns as rows");
            return Result;
        }

        i32 cRow, cCol;
        i32 maxCount = 1;
        i32 count = 0;
        b32 completeFlag = false;

        while(!completeFlag && count < maxCount)
        {
            for(i32 diagIndex = 0; diagIndex < M; ++diagIndex)
            {
                cRow = diagIndex;
                cCol = diagIndex;

                for(i32 rowIndex = cRow + 1; rowIndex < M; ++rowIndex)
                {
                    if(!NearlyEqualUlps<T>(Result.Data[rowIndex][cCol], 0.0f))
                    {
                        i32 rowOneIndex = cCol;

                        T currentElementValue = Result.Data[rowIndex][cCol];
                        T rowOneValue = Result.Data[rowOneIndex][cCol];

                        if(!NearlyEqualUlps<T>(rowOneValue, 0.0f))
                        {
                            T correctionFactor = -(currentElementValue / rowOneValue);
                            RowMultAdd(Result, rowIndex, rowOneIndex, correctionFactor);
                        }
                    }
                }
            }

            ASSERT(Result.IsRowEchelon());
            ++count;
        }

        return Result;
    }

    template <typename T, size_t M, size_t N>
    const matMN<T, M, N>&
    matMN<T, M, N>::operator=(const matMN &rhs)
    {
        for (i32 i = 0; i < M; ++i)
        {
            this->Rows[i] = rhs.Rows[i];
        }

        return *this;
    }

    template <typename T, size_t M, size_t N>
    void
    matMN<T, M, N>::operator*=(T rhs)
    {
        for (i32 i = 0; i < M; ++i)
        {
            for (i32 j = 0; j < N; ++j)
            {
                this->Data[i][j] *= rhs;
            }
        }
    }

    template <typename T, size_t M, size_t N>
    vecN<T, M>
    matMN<T, M, N>::operator*(const vecN<T, N> &rhs) const
    {
        vecN<T, M> Result;
        Result.Zero();

        for (i32 i = 0; i < M; ++i)
        {
            for (i32 j = 0; j < N; ++j)
            {
                Result.Data[i] += this->Data[i][j] * rhs.Data[j];
            }
        }

        return Result;
    }

    template<typename T, size_t M, size_t N>
    vecN<T, N> operator*(const vecN<T, M> &Vector, const matMN<T, M, N> &Matrix)
    {
        vecN<T, N> Result;
        Result.Zero();

        for (i32 j = 0; j < N; ++j)
        {
            for (i32 i = 0; i < M; ++i)
            {
                Result.Data[j] += Vector.Data[i] * Matrix.Data[i][j];
            }
        }

        return Result;
    }


    template <typename T, size_t M, size_t N>
    matN<T, M>
    matMN<T, M, N>::operator*(const matMN<T, N, M> &rhs) const
    {
        matN<T, M> Result;
        Result.Zero();

        for (i32 i = 0; i < M; ++i)
        {
            for (i32 j = 0; j < M; ++j)
            {
                for (i32 k = 0; k < N; ++k)
                {
                    Result.Data[i][j] += this->Data[i][k] * rhs.Data[k][j];
                }
            }
        }

        return Result;
    }

    template <typename T, size_t M, size_t N>
    matMN<T, M, N>
    matMN<T, M, N>::operator*(const matN<T, N> &rhs) const
    {
        matMN<T, M, N> Result;
        Result.Zero();

        for (i32 rowIndex = 0; rowIndex < M; ++rowIndex)
        {
            T sum = (T)0;
            for (i32 colIndex = 0; colIndex < N; ++colIndex)
            {
                vecN<T,N> column = rhs.GetColumn(colIndex);
                Result.Data[rowIndex][colIndex] = this->Rows[rowIndex].Dot(column);
            }
        }

        return Result;
    }

#endif
}

#endif // MATH_MATRIX_H