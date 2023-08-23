#if !defined(IMAGE_LOADER_H)

#include "defines.h"
#include "math/math.h"

struct shoora_image_mipmap
{
    // vec2f Dim;
    u64 Size;
    u32 Offset;
    u32 MipmapLevel;
    struct shoora_image_data *pImage;
};

struct shoora_image_data
{
    vec2i Dim;
    i32 NumChannels;

    shoora_image_mipmap Mipmaps[16];
    i32 MipmapCount;

    u32 TotalSize;
    u8 *Data;
};

shoora_image_data LoadImageFile(const char *Filename, u32 MipmapCount);
void FreeImageData(shoora_image_data *Data);

#define IMAGE_LOADER_H
#endif // IMAGE_LOADER_H