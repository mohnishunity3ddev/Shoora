#if !defined(MESH_UTILS_H)

#include <defines.h>
#include <math/math.h>

struct convex_hull_2d
{
  private:
    Shu::vec2f *Vertices;
    i32 VertexCount;

  public:
    convex_hull_2d() = delete;
    convex_hull_2d(Shu::vec3f *VertexPositions, i32 VertexCount);

    Shu::vec2f *GetVertices() { return Vertices; }
    i32 GetVertexCount() { return VertexCount; }

    ~convex_hull_2d()
    {
        delete[] Vertices;
        VertexCount = 0;
    }

};

#define MESH_UTILS_H
#endif