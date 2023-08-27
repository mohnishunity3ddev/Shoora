#include "jpeg_loader.h"

Shu::vec3f
RGBToYCbCr(Shu::vec3f Rgb)
{
    Shu::vec3f Result = {};

    f32 Red = Rgb.r*255.0f;
    f32 Green = Rgb.g*255.0f;
    f32 Blue = Rgb.b*255.0f;

    // Y
    Result.x = 0.299f*Red + 0.587f*Green + 0.114f*Blue;
    // Cb
    Result.y = -0.1687f*Red - 0.3313f*Green + 0.5f*Blue + 128;
    // Cr
    Result.z = 0.5f*Red - 0.4187f*Green - 0.0813f*Blue + 128;

    return Result;
}