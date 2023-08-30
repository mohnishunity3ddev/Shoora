#include "image_loader.h"

#if SHU_USE_STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#else
#include "png_loader.h"
#include "jpeg_loader.h"
#endif

shoora_image_mipmap
GetMipmapLevelData()
{
    return {};
}

// TODO)): Check the filename for png/jpeg format and choose appropriately.
shoora_image_data
LoadImageFile(const char *Filename, u32 MipmapCount, u32 DesiredChannelCount)
{
    shoora_image_data ImageData = {};

#if SHU_USE_STB
#if 0
    stbi_info(Filename, &ImageData.Dim.Width, &ImageData.Dim.Height, &ChannelCount);
#endif
    stbi_set_flip_vertically_on_load(1);
    ImageData.Data = stbi_load(Filename, &ImageData.Dim.w, &ImageData.Dim.h, &ImageData.NumChannels,
                               STBI_rgb_alpha);


    // ASSERT(ImageData.NumChannels == 4);

    if((DesiredChannelCount != 0) &&
       (DesiredChannelCount != ImageData.NumChannels))
    {
        ASSERT(!"Your desired number of channels for this file is not supported!");
    }
    ImageData.TotalSize = (ImageData.Dim.w)*(ImageData.Dim.h)*(4);

    if(MipmapCount > 0)
    {
        ASSERT(!"Mipmaps are not supported right now!\n");
    }
#else
    LoadPNG(Filename);
#error "custom loaders for file not supported yet!"
#endif

    return ImageData;
}

void
FreeImageData(shoora_image_data *ImageData)
{
    ASSERT((ImageData != nullptr) && (ImageData->Data != nullptr));

#if SHU_USE_STB
    stbi_image_free(ImageData->Data);
#else
    FreePng(ImageData);
#error "custom image loaders are not implemented yet!"
#endif
    ImageData->Data = nullptr;
    ImageData->Dim.w = 0;
    ImageData->Dim.h = 0;
    ImageData->NumChannels = 0;
}


























#if 0
void
GenerateMipMaps(const char *InputFilename, const char *OutputFilename, i32 MipLevelCount, i32 Quality,
                u64 *MipOffsets, b32 CapImageToFullHD)
{
    i32 ImageWidth, ImageHeight, BytesPerPixel;
    unsigned char *InputData = stbi_load(InputFilename, &ImageWidth, &ImageHeight, &BytesPerPixel, 0);
    if (!InputData)
    {
        fprintf(stderr, "Failed to load image: %s\n", InputFilename);
        return;
    }
    if (CapImageToFullHD && ImageWidth > 1920)
    {
        u8 *Temp = (u8 *)malloc(1920 * 1080 * BytesPerPixel);
        stbir_resize_uint8(InputData, ImageWidth, ImageHeight, 0, Temp, 1920, 1080, 0, BytesPerPixel);
        ImageWidth = 1920;
        ImageHeight = 1080;
        free(InputData);
        InputData = Temp;
    }

    i32 MipLevels = MipLevelCount;            // Example number of mip levels
    i32 CombinedWidth = (ImageWidth * 3) / 2; // Width of the combined image
    i32 CombinedHeight = ImageHeight;         // Height of the combined image
    i32 CombinedChannels = BytesPerPixel;     // Number of channels (e.g., 3 for RGB)
    i32 CombinedSize = CombinedWidth * CombinedHeight * CombinedChannels;

    u8 *CombinedData = (u8 *)malloc(CombinedSize);
    memset(CombinedData, 0, CombinedSize);

    if (!CombinedData)
    {
        fprintf(stderr, "Memory allocation failed for combined image\n");
        return;
    }

    int XOffset = 0;
    int YOffset = 0;

    for (i32 MipLevel = 0; MipLevel < MipLevels; ++MipLevel)
    {
        MipOffsets[MipLevel] = (YOffset * CombinedWidth + XOffset) * CombinedChannels;

        int MipWidth = ImageWidth >> MipLevel;
        int MipHeight = ImageHeight >> MipLevel;

        if (MipWidth < 1 || MipHeight < 1)
        {
            break;
        }

        u8 *MipmapData = (u8 *)malloc(MipWidth * MipHeight * CombinedChannels);
        if (!MipmapData)
        {
            fprintf(stderr, "Memory allocation failed for mip level %d\n", MipLevel);
            return;
        }

        stbir_resize_uint8(InputData, ImageWidth, ImageHeight, 0, MipmapData, MipWidth, MipHeight, 0,
                           CombinedChannels);

        i32 Y = 0;
        for (; Y < MipHeight; ++Y)
        {
            for (i32 X = 0; X < MipWidth; ++X)
            {
                for (i32 C = 0; C < CombinedChannels; ++C)
                {
                    CombinedData[((Y + YOffset) * CombinedWidth + XOffset + X) * CombinedChannels +
                                 C] = MipmapData[(Y * MipWidth + X) * CombinedChannels + C];
                }
            }
        }

        ASSERT((MipHeight + YOffset) <= CombinedHeight);

        if ((MipHeight + YOffset) == CombinedHeight)
        {
            XOffset += MipWidth;
            YOffset = 0;
        }
        else
        {
            YOffset += Y;
        }
        free(MipmapData);
    }

    // Save the combined image
    if (!stbi_write_jpg(OutputFilename, CombinedWidth, CombinedHeight, CombinedChannels, CombinedData, Quality))
    {
        fprintf(stderr, "Failed to write image: %s\n", OutputFilename);
    }

    // Cleanup
    stbi_image_free(InputData);
    free(CombinedData);
}
#endif
