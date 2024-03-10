#if !defined(SHAPE_CONVEX_H)
#define SHAPE_CONVEX_H

#include <defines.h>
#include <math/math.h>
#include <memory/memory.h>

typedef void OnWorkComplete(struct shoora_shape_convex *ConvexShape);

struct shape_convex_build_work_data
{
    struct shoora_shape_convex *ConvexShapeMemory;
    shu::vec3f *Points;
    i32 NumPoints;
    OnWorkComplete *CompleteCallback;
    task_with_memory *TaskMem;
    memory_arena *Arena;
};

void BuildConvexThreaded(platform_work_queue *Queue, shoora_shape_convex *ConvexMem, shu::vec3f *Points,
                         i32 NumPoints, OnWorkComplete *OnComplete, memory_arena *Arena);

#endif