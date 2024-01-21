#include "mesh_utils.h"
#include <utils/utils.h>

// Sorted in ascending order of angle the vectors make with the reference point for convex hull.
b32
AngleComparator(const Shu::vec2f &a, const Shu::vec2f &b)
{
    f32 angleA = Shu::TanInverse(a.y, a.x);
    f32 angleB = Shu::TanInverse(b.y, b.x);
    b32 Result = angleA <= angleB;
    return Result;
}

convex_hull_2d::convex_hull_2d(Shu::vec3f *VertexPositions, i32 VertexCount)
{
    f32 min = SHU_FLOAT_MAX;

    // IMPORTANT: NOTE:
    // Getting that one vertex which has the lowest y value. Getting the point which is at the bottom.
    i32 ReferenceIndex = -1;
    for (int i = 0; i < VertexCount; ++i)
    {
        auto *VertexPos = VertexPositions + i;
        if (min > VertexPos->y)
        {
            ReferenceIndex = i;
            min = VertexPos->y;
        }
    }

    if (ReferenceIndex != 0)
    {
        SWAP(VertexPositions[0], VertexPositions[ReferenceIndex]);
        ReferenceIndex = 0;
    }

    Shu::vec2f *SortedPoints = new Shu::vec2f[VertexCount];
    SortedPoints[0] = VertexPositions[ReferenceIndex].xy;
    for (i32 i = 1; i < VertexCount; ++i)
    {
        SortedPoints[i] = VertexPositions[i].xy - VertexPositions[0].xy;
    }

    // IMPORTANT: NOTE:
    // sort the vertices starting from index 1 based on the angle they make with the x-axis going through the
    // reference point(which is at index 0).
    QuicksortRecursive(SortedPoints + 1, 0, VertexCount - 1, AngleComparator);
    for (int i = 1; i < VertexCount; ++i)
    {
        SortedPoints[i] += SortedPoints[0];
    }

    // IMPORTANT: NOTE:
    // Graham's Algorithm/Scan to get the Convex Hull of the provided vertices. This will return a shape than
    // encloses all the points.
    stack<Shu::vec2f> Stack{VertexCount, Shu::Vec2f(SHU_FLOAT_MIN, SHU_FLOAT_MIN)};
    Stack.push(SortedPoints[0]);
    Stack.push(SortedPoints[1]);
    Stack.push(SortedPoints[2]);

    for (int i = 3; i < VertexCount; ++i)
    {
        auto currentVertex = SortedPoints[i];

        auto top = Stack.peek();
        auto topMinus1 = Stack.peek(-1);
        auto v0 = top - topMinus1;
        auto v1 = currentVertex - top;
        // v0 X v1
        f32 Cross = v0.x * v1.y - v0.y * v1.x;

        while (Cross < 0.0f)
        {
            Stack.pop();

            v0 = Stack.peek() - Stack.peek(-1);
            v1 = currentVertex - Stack.peek();
            Cross = v0.x * v1.y - v0.y * v1.x;
        }
        Stack.push(currentVertex);
    }

    this->VertexCount = Stack.getSize();
    this->Vertices = new Shu::vec2f[this->VertexCount];
    for (int i = 0; i < this->VertexCount; ++i)
    {
        this->Vertices[i] = Stack.pop();
    }

    delete[] SortedPoints;
}
