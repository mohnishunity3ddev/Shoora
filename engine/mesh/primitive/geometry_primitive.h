#if !defined(GEOMETRY_PRIMITIVE_H)

#include <defines.h>
#include <mesh/mesh_filter.h>


enum shoora_primitive_type
{
    PRIMITIVE_TYPE_NONE,
    PRIMITIVE_TYPE_SQUARE,
    PRIMITIVE_TYPE_CIRCLE,
    PRIMITIVE_TYPE_CUBE,
    PRIMITIVE_TYPE_SPHERE,
    PRIMITIVE_MAX_COUNT
};

struct shoora_primitive
{
    shoora_mesh_filter MeshFilter;
    shoora_primitive_type PrimitiveType;
};

struct shoora_primitive_collection
{
    shoora_primitive Circle;
};

void InitializePrimitives();
shoora_mesh_filter *GetPrimitive2d(shoora_primitive_type Type);

#define GEOMETRY_PRIMITIVE_H
#endif // GEOMETRY_PRIMITIVE_H