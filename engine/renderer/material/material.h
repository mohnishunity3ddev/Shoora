#if !defined(MATERIAL_H)

#include "defines.h"
#include <math/math.h>

struct shoora_material
{
    f32 Ambient;
    SHU_ALIGN_16 Shu::vec3f Diffuse;
    SHU_ALIGN_16 Shu::vec3f Specular;
    f32 Shininess;
};

enum MaterialType
{
    EMERALD,
    JADE,
    OBSIDIAN,
    PEARL,
    RUBY,
    TURQUOISE,
    BRASS,
    BRONZE,
    CHROME,
    COPPER,
    GOLD,
    SILVER,
    BLACK_PLASTIC,
    CYAN_PLASTIC,
    GREEN_PLASTIC,
    RED_PLASTIC,
    WHITE_PLASTIC,
    YELLOW_PLASTIC,
    BLACK_RUBBER,
    CYAN_RUBBER,
    GREEN_RUBBER,
    RED_RUBBER,
    WHITE_RUBBER,
    YELLOW_RUBBER,

    MAX_MATERIAL_COUNT
};

void GetMaterial(MaterialType Type, shoora_material *OutMaterial);

#define MATERIAL_H
#endif // MATERIAL_H