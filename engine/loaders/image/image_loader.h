#if !defined(IMAGE_LOADER_H)

#include "defines.h"
#include "math/math.h"
#include "platform/platform.h"

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
    Shu::vec2i Dim;
    i32 NumChannels;

    shoora_image_mipmap Mipmaps[16];
    i32 MipmapCount;

    u32 TotalSize;
    u8 *Data;
};

shoora_image_data LoadImageFile(const char *Filename, b32 FlipImageVertically = true, u32 MipmapCount = 0,
                                u32 DesiredChannelCount = 0);
void FreeImageData(shoora_image_data *Data);

#define IMAGE_LOADER_H
#endif // IMAGE_LOADER_H