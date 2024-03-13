#include "shape.h"

// NOTE: Shape stuff
shoora_shape::shoora_shape(shoora_mesh_type Type)
{
    this->Type = Type;
    this->isPrimitive = true;
    this->MeshFilter = shoora_mesh_database::GetMeshFilter(Type);
}

shoora_shape::shoora_shape(shoora_mesh_type Type, shoora_mesh_filter *MeshFilter)
{
    this->Type = Type;
    this->isPrimitive = false;
    this->MeshFilter = MeshFilter;
}

shoora_mesh_filter *
shoora_shape::GetMeshFilter()
{
    shoora_mesh_filter *Result = this->MeshFilter;
    return Result;
}