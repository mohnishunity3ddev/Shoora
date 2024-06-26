#if !defined(LINEAR_EQUATION_SOLVER_H)

#include <defines.h>
#include "math_vector.h"
#include "math_matrix.h"

namespace shu
{
    template <typename T>
    vec3<T>
    LE_CramersRule(const mat3<T> &CoeffMatrix, const vec3<T> &ConstantVec)
    {
        vec3<T> Result;

        T CoeffDet = CoeffMatrix.Determinant();

        mat3<T> XMat = CoeffMatrix;
        XMat.SetColumn(0, ConstantVec);
        T XDeter = XMat.Determinant();
        ASSERT(!NearlyEqual(XDeter, 0.0f));

        mat3<T> YMat = CoeffMatrix;
        YMat.SetColumn(1, ConstantVec);
        T YDeter = YMat.Determinant();
        ASSERT(!NearlyEqual(YDeter, 0.0f));

        mat3<T> ZMat = CoeffMatrix;
        ZMat.SetColumn(2, ConstantVec);
        T ZDeter = ZMat.Determinant();
        ASSERT(!NearlyEqual(ZDeter, 0.0f));

        Result.x = XDeter / CoeffDet;
        Result.y = YDeter / CoeffDet;
        Result.z = ZDeter / CoeffDet;

        return Result;
    }

    template <typename T>
    vec2<T>
    LE_CramersRule(const T CoeffMatrix[2][2], const vec2<T> &ConstantVec)
    {
        vec2<T> Result;

        T CoeffDet = CoeffMatrix[0][0]*CoeffMatrix[1][1] - CoeffMatrix[0][1]*CoeffMatrix[1][0];

        T XMat[2][2] = {ConstantVec.x, CoeffMatrix[0][1], ConstantVec.y, CoeffMatrix[1][1]};
        T XDeter = XMat[0][0]*XMat[1][1] - XMat[0][1]*XMat[1][0];
        ASSERT(!NearlyEqual(XDeter, 0.0f));

        T YMat[2][2] = {CoeffMatrix[0][0], ConstantVec.x, CoeffMatrix[1][0], ConstantVec.y};
        T YDeter = YMat[0][0]*YMat[1][1] - YMat[0][1]*YMat[1][0];
        ASSERT(!NearlyEqual(YDeter, 0.0f));

        Result.x = XDeter / CoeffDet;
        Result.y = YDeter / CoeffDet;

        return Result;
    }

    template<typename T>
    vec3<T>
    LCP_Mat3(const mat3<T> &M, const vec3<T> &V)
    {
        auto invM = M.InverseTranspose();
        auto v = V * invM;

    #if _SHU_DEBUG
        auto s1 = M.m00*v.x + M.m01*v.y + M.m02*v.z;
        auto s2 = M.m10*v.x + M.m11*v.y + M.m12*v.z;
        auto s3 = M.m20*v.x + M.m21*v.y + M.m22*v.z;

        ASSERT(NearlyEqualUlps(s1, V.x));
        ASSERT(NearlyEqualUlps(s2, V.y));
        ASSERT(NearlyEqualUlps(s3, V.z));
    #endif

        return v;
    }

    template<typename T>
    vec4<T>
    LCP_Mat4(const mat4<T> &M, const vec4<T> &V)
    {
        auto invM = M.InverseTranspose();
        auto v = V * invM;

    #if _SHU_DEBUG
        auto s1 = M.m00*v.x + M.m01*v.y + M.m02*v.z + M.m03*v.w;
        auto s2 = M.m10*v.x + M.m11*v.y + M.m12*v.z + M.m13*v.w;
        auto s3 = M.m20*v.x + M.m21*v.y + M.m22*v.z + M.m23*v.w;
        auto s4 = M.m30*v.x + M.m31*v.y + M.m32*v.z + M.m33*v.w;

        ASSERT(NearlyEqualUlps(s1, V.x));
        ASSERT(NearlyEqualUlps(s2, V.y));
        ASSERT(NearlyEqualUlps(s3, V.z));
        ASSERT(NearlyEqualUlps(s4, V.w));
    #endif

        return v;
    }

    template <typename T, size_t N>
    vecN<T, N>
    LCP_GaussElimination(const matN<T, N> &A, const vecN<T, N> &b)
    {
        // Add the vecN V to the far right of matN M to convert this into a matMN matrix with M = N + 1.
        matMN<T,N,N+1> MatMN{A};
        MatMN.AddColumn(N, b);

        i32 nRows = (i32)N;
        i32 nCols = (i32)(N + 1);

    #if 0
        auto z = (re.Data[nRows-1][nCols-1]) / re.Data[nRows - 1][nCols - 2];
        auto y = (re.Data[nRows-2][nCols-1] - re.Data[nRows-2][nCols-2]*z) / (re.Data[nRows-2][nCols-3]);
        auto x = (re.Data[nRows-3][nCols-1] - re.Data[nRows-3][nCols-2]*z - re.Data[nRows-3][nCols-3]*y) / re.Data[nRows-3][nCols-4];
    #endif

        vecN<T,N> Result{};
        auto re = MatMN.RowEchelon();

        // TODO(mani): This part could be a little short maybe? a little easier?
        i32 vecIndex = N - 1;
        i32 colIndex = nCols - 2;
        for(i32 rowIndex = nRows-1; rowIndex >= 0; --rowIndex, --vecIndex)
        {
            auto numer = re.Data[rowIndex][nCols - 1];
            auto denom = re.Data[rowIndex][colIndex--];
            i32 nIterations = nRows - rowIndex - 1;

            i32 innerColIndex = nCols - 2;
            for(i32 iterIndex = 0; iterIndex < nIterations; ++iterIndex, --innerColIndex)
            {
                auto vData = Result.Data[N - iterIndex - 1];
                auto mData = re.Data[rowIndex][innerColIndex];
                numer -= mData * vData;
            }

            Result.Data[vecIndex] = (T)numer / (T)denom;
        }

    #if 0
        vecN<T, N> cmp = A.Transposed() * Result;
        for (i32 i = 0; i < N; ++i)
        {
            ASSERT(NearlyEqualUlps(cmp.Data[i], b.Data[i], 128));
        }
    #endif

        return Result;
    }

    template <typename T, size_t N>
    b32
    IsSymmetricPositiveDefinite(const matN<T, N> &A, const vecN<T, N> &b)
    {
        auto t1 = A * b;
        auto d = t1.Dot(b);

        b32 Result = (d > 0);
        return Result;
    }

    template<typename T, size_t N>
    vecN<T,N>
    LCP_GaussSeidel(const matN<T,N> &A, const vecN<T, N> &b)
    {
        // IMPORTANT: NOTE: Condition for the matrix for Gauss Siedel to work. Matrix should diagonallyDominant and
        // Symetric Positive Definite.

        // TODO): Check why this is failing for Penetration Constraints when adding friction into the mix.
        // ASSERT(A.IsDiagonallyDominant());

        // ASSERT(IsSymmetricPositiveDefinite(A, b));

        vecN<T,N> Result{};
        for(i32 iter = 0; iter < N; iter++)
        {
            for(i32 i = 0; i < N; i++)
            {
                T dx = (b.Data[i] - A.Rows[i].Dot(Result)) / A.Data[i][i];
                if((dx*0.0f) == (dx*0.0f))
                {
                    Result[i] += dx;
                }
            }
        }

        return Result;
    }

#if 0
    void test_LinearEqSolver()
    {
    #if 0
    shu::matN<f32, 3> Mat33{1, 3, -1,
                            4, -1, 1,
                            2, 4, 3};
    shu::vecN<f32, 3> Vector3 = {13, 9, 6};
    auto Soln = shu::LCP_GaussElimination(Mat33, Vector3);
    #else
        shu::matN<f32, 5> Mat55{3, 2, -1, 4, -2, 1, -3, 2, -3, 2, 2, 1, -4, 2, 1, 1, 4, 1, 1, -1, -2, 3, -2, 3, 1};
        shu::vecN<f32, 5> Vector4 = {12, -5, 8, 3, 6};
        auto Soln = shu::LCP_GaussElimination(Mat55, Vector4);

        shu::matN<f32, 3> Mat33{ 2,  -1,   0,
                                -1,   2,  -1,
                                 0,  -1,   2};
        shu::vecN<f32, 3> Vector3 = {7, 1, 1};

        auto Soln2 = shu::LCP_GaussElimination(Mat33, Vector3);
        auto Soln1 = shu::LCP_GaussSeidel(Mat33, Vector3);

    #endif
    }
#endif

}

#define LINEAR_EQUATION_SOLVER_H
#endif