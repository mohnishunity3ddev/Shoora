#include "material.h"

struct shoora_material_internal
{
    const char *Name;
    shu::vec3f Ambient;
    shu::vec3f Diffuse;
    shu::vec3f Specular;
    f32 Shininess;
};

static const shoora_material_internal Materials[] =
{
    {
        .Name = "Emerald",
        .Ambient   = shu::vec3f{ 0.0215f, 0.1745f, 0.0215f },
        .Diffuse   = shu::vec3f{ 0.07568f, 0.61424f, 0.07568f },
        .Specular  = shu::vec3f{ 0.633f, 0.727811f, 0.633f },
        .Shininess =  0.6f
    },
    {
        .Name = "Jade",
        .Ambient   = { 0.135f, 0.2225f, 0.1575f },
        .Diffuse   = { 0.54f, 0.89f, 0.63f },
        .Specular  = { 0.316228f, 0.316228f, 0.316228f },
        .Shininess =  0.1f
    },
    {
        .Name = "Obsidian",
        .Ambient =	{0.05375f,	0.05f,	0.06625f},
        .Diffuse = {0.18275f, 0.17f, 0.22525f},
        .Specular = {0.332741f,	0.328634f, 0.346435f},
        .Shininess = 0.3f
    },
    {
        .Name = "Pearl",
        .Ambient   = { 0.25f, 0.20725f, 0.20725f },
        .Diffuse   = { 1.0f, .829f, .829f },
        .Specular  = { 0.296648f, 0.296648f, 0.296648f },
        .Shininess =  0.088f
    },
    {
        .Name = "Ruby",
        .Ambient =	{0.1745f,	0.01175f,	0.01175f},
        .Diffuse = {0.61424f, 0.04136f, 0.04136f},
        .Specular = {0.727811f,	0.626959f, 0.626959f},
        .Shininess = 0.6f
    },
    {
        .Name = "Turquoise",
        .Ambient =	{0.1f,	0.18725f,	0.1745f},
        .Diffuse = {0.396f, 0.74151f, 0.69102f},
        .Specular = {0.297254f,	0.30829f, 0.306678f},
        .Shininess = 0.1f
    },
    {
        .Name = "Brass",
        .Ambient =	{0.329412f,	0.223529f,	0.027451f},
        .Diffuse = {0.780392f, 0.568627f, 0.113725f},
        .Specular = {0.992157f,	0.941176f, 0.807843f},
        .Shininess = 0.21794872f
    },
    {
        .Name = "Bronze",
        .Ambient =	{0.2125f,	0.1275f,	0.054f},
        .Diffuse = {0.714f, 0.4284f, 0.18144f},
        .Specular = {0.393548f,	0.271906f, 0.166721f},
        .Shininess = 0.2f
    },
    {
        .Name = "Chrome",
        .Ambient =	{0.25f,	0.25f,	0.25f},
        .Diffuse = {0.4f, 0.4f, 0.4f},
        .Specular = {0.774597f,	0.774597f, 0.774597f},
        .Shininess = 0.6f
    },
    {
        .Name = "Copper",
        .Ambient =	{0.19125f,	0.0735f,	0.0225f},
        .Diffuse = {0.7038f, 0.27048f, 0.0828f},
        .Specular = {0.256777f,	0.137622f, 0.086014f},
        .Shininess = 0.1f
    },
    {
        .Name = "Gold",
        .Ambient =	{0.24725f,	0.1995f,	0.0745f},
        .Diffuse = {0.75164f, 0.60648f, 0.22648f},
        .Specular = {0.628281f,	0.555802f, 0.366065f},
        .Shininess = 0.4f
    },
    {
        .Name = "Silver",
        .Ambient =	{0.19225f,	0.19225f,	0.19225f},
        .Diffuse = {0.50754f, 0.50754f, 0.50754f},
        .Specular = {0.508273f,	0.508273f, 0.508273f},
        .Shininess = 0.4f
    },
    {
        .Name = "Black Plastic",
        .Ambient =	{0.0f,	0.0f,	0.0f},
        .Diffuse = {0.01f,	0.01f,	0.01f},
        .Specular = {0.50f,	0.50f,	0.50f},
        .Shininess = .25f
    },
    {
        .Name = "Cyan Plastic",
        .Ambient   = {0.0f,	0.1f, 0.06f},
        .Diffuse   = {0.0f,	0.50980392f, 0.50980392f},
        .Specular  = {0.50196078f, 0.50196078f,	0.50196078f},
        .Shininess = .25f
    },
    {
        .Name = "Green Plastic",
        .Ambient   = {0.0f,	0.0f, 0.0f},
        .Diffuse   = {0.1f,	0.35f, 0.1f},
        .Specular  = {0.45f, 0.55f,	0.45f},
        .Shininess = .25f
    },
    {
        .Name = "Red Plastic",
        .Ambient   = {0.0f,	0.0f, 0.0f},
        .Diffuse   = {0.5f,	0.0f, 0.0f},
        .Specular  = {0.7f,	0.6f, 0.6f},
        .Shininess = .25f
    },
    {
        .Name = "White Plastic",
        .Ambient   = {0.0f, 0.0f, 0.0f},
        .Diffuse   = {0.55f, 0.55f, 0.55f},
        .Specular  = {0.70f, 0.70f, 0.70f},
        .Shininess = .25f
    },
    {
        .Name = "Yellow Plastic",
        .Ambient   = {0.0f,	0.0f, 0.0f},
        .Diffuse   = {0.5f,	0.5f, 0.0f},
        .Specular  = {0.60f, 0.60f,	0.50f},
        .Shininess = .25f
    },
    {
        .Name = "Black Rubber",
        .Ambient   = {0.02f, 0.02f, 0.02f},
        .Diffuse   = {0.01f, 0.01f,	0.01f},
        .Specular  = {0.4f,	0.4f, 0.4f},
        .Shininess = .078125f
    },
    {
        .Name = "Cyan Rubber",
        .Ambient   = {0.0f,	0.05f,	0.05f},
        .Diffuse   = {0.4f,	0.5f,	0.5f},
        .Specular  = {0.04f, 0.7f,	0.7f},
        .Shininess = .07125f
    },
    {
        .Name = "Green Rubber",
        .Ambient =	{0.0f,	0.05f,	0.0f},
        .Diffuse = {0.4f,	0.5f,	0.4f},
        .Specular = {0.04f,	0.7f,	0.04f},
        .Shininess = .07125f
    },
    {
        .Name = "Red Rubber",
        .Ambient =	{0.05f,	0.0f,	0.0f},
        .Diffuse = {0.5f,	0.4f,	0.4f},
        .Specular = {0.7f,	0.04f,	0.04f},
        .Shininess = .07125f
    },
    {
        .Name = "White Rubber",
        .Ambient =	{0.05f,	0.05f,	0.05f},
        .Diffuse = {0.5f,	0.5f,	0.5f},
        .Specular = {0.7f,	0.7f,	0.7f},
        .Shininess = .07125f
    },
    {
        .Name = "Yellow Rubber",
        .Ambient =	{0.05f,	0.05f,	0.0f},
        .Diffuse = {0.5f,	0.5f,	0.4f},
        .Specular = {0.7f,	0.7f,	0.04f},
        .Shininess = .07125f
    },
};

#if 0
void
GetMaterial(MaterialType Type, shoora_model_material *OutMaterial)
{
    ASSERT(ARRAY_SIZE(Materials) == MaterialType::MAX_MATERIAL_COUNT);
    ASSERT(Type < MaterialType::MAX_MATERIAL_COUNT);

    shoora_material_internal Mat = Materials[(u32)Type];
    Shu::vec3f Coeff = {0.212671f, 0.715160f, 0.072169f};
    f32 AmbientStrength = Shu::Dot(Coeff, Mat.Ambient) / Shu::Dot(Coeff, Mat.Diffuse);
    f32 Shininess = Mat.Shininess*128.0f;

    OutMaterial->Ambient = AmbientStrength;
    OutMaterial->Diffuse = Mat.Diffuse;
    OutMaterial->Specular = Mat.Specular;
    OutMaterial->Shininess = Shininess;
}
#endif
// content of material.cpp goes here
