#include "png_loader.h"

#include <cstdio>
#include <memory>

struct shoora_file
{
    u32 ContentSize;
    void *Contents;
};

#pragma pack(push, 1)
// NOTE: From https://www.w3.org/TR/2003/REC-PNG-20031110/#5DataRep
struct png_header
{
    u8 Signature[8];
};
static u8 PNGSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

struct png_chunk_header
{
    u32 Length;
    union
    {
        u32 TypeU32;
        char Type[4];
    };
};

struct png_chunk_footer
{
    u32 CRC;
};

struct png_ihdr
{
    u32 Width;
    u32 Height;
    u8 BitDepth;
    u8 ColourType;
    u8 CompressionMethod;
    u8 FilterMethod;
    u8 InterlaceMethod;
};

struct png_idat_header
{
    u8 ZLibMethodFlags;
    u8 AdditionalFlags;
};

// NOTE: From https://www.ietf.org/rfc/rfc1950.txt and https://www.ietf.org/rfc/rfc1951.txt
struct png_idat_zlib_flags
{
    u8 CM;
    u8 CINFO;
    u8 FCHECK;
    u8 FDICT;
    u8 FLEVEL;
};

struct png_idat_footer
{
    u8 CheckValue;
};

#pragma pack(pop)

#define FOURCC(String) (((u32)(String[0]) << 0) | ((u32)(String[1]) << 8) | ((u32)(String[2]) << 16) | ((u32)(String[3]) << 24))

shoora_file
ReadFile(const char *Filename)
{
    shoora_file Result = {};

    FILE *In = fopen(Filename, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Result.ContentSize = ftell(In);
        fseek(In, 0, SEEK_SET);

        Result.Contents = malloc(Result.ContentSize);
        fread(Result.Contents, Result.ContentSize, 1, In);
        fclose(In);
    }
    else
    {
        LogFatal("File %s could not be opened!\n", Filename);
    }

    return Result;
}

#define Consume(File, Type) (Type *)ConsumeSize(File, sizeof(Type))
void *
ConsumeSize(shoora_file *File, u32 Size)
{
    void *Result = 0;

    if(File->ContentSize >= Size)
    {
        Result = File->Contents;
        File->Contents = (u8 *)File->Contents + Size;
        File->ContentSize -= Size;
    }
    else
    {
        // LogOutput(!"Invalid Path");
        File->ContentSize = 0;
    }

    return Result;
}

void
EndianSwap(u32 *Val)
{
    u32 V = *Val;

    u8 B0 = ((V >> 0) & 0xFF);
    u8 B1 = ((V >> 8) & 0xFF);
    u8 B2 = ((V >> 16) & 0xFF);
    u8 B3 = ((V >> 24) & 0xFF);

    V = ((B0 << 24) | (B1 << 16) | (B2 << 8) | B3);

    *Val = V;
}

void *
AllocatePixels(u32 Width, u32 Height, u32 BytesPerPixel)
{
    void *Result = malloc(Width * Height * BytesPerPixel);
    ASSERT(Result != nullptr);

    memset(Result, 0, Width * Height * BytesPerPixel);

    return Result;
}

void
FreePixels(void *Memory)
{
    ASSERT(Memory != nullptr);
    free(Memory);
    Memory = nullptr;
}

void
ParsePNG(shoora_file File)
{
    shoora_file *At = &File;
    b32 Supported = false;

    u8 *DecompressedPixels = nullptr;

    LogInfoUnformatted("PNG Headers...\n");
    png_header *PngHeader = Consume(At, png_header);
    if(PngHeader)
    {
        while(At->ContentSize > 0)
        {
            png_chunk_header *ChunkHeader = Consume(At, png_chunk_header);
            if(ChunkHeader)
            {
                LogInfo("%c%c%c%c\n", ChunkHeader->Type[0],
                                      ChunkHeader->Type[1],
                                      ChunkHeader->Type[2],
                                      ChunkHeader->Type[3]);
                EndianSwap(&ChunkHeader->Length);


                void *ChunkData = ConsumeSize(At, ChunkHeader->Length);
                png_chunk_footer *ChunkFooter = Consume(At, png_chunk_footer);
                EndianSwap(&ChunkFooter->CRC);

                switch(ChunkHeader->TypeU32)
                {
                    case FOURCC("IHDR"):
                    {
                        png_ihdr *IHDR = (png_ihdr *)ChunkData;

                        EndianSwap(&IHDR->Width);
                        EndianSwap(&IHDR->Height);

                        LogInfo("   Width: %u\n", IHDR->Width);
                        LogInfo("   Height: %u\n", IHDR->Height);
                        LogInfo("   BitDepth: %u\n", IHDR->BitDepth);
                        LogInfo("   ColourType: %u\n", IHDR->ColourType);
                        LogInfo("   CompressionMethod: %u\n", IHDR->CompressionMethod);
                        LogInfo("   FilterMethod: %u\n", IHDR->FilterMethod);
                        LogInfo("   InterlaceMethod: %u\n", IHDR->InterlaceMethod);

                        if (IHDR->BitDepth == 8 &&
                            IHDR->ColourType == 6 &&
                            IHDR->CompressionMethod == 0 &&
                            IHDR->FilterMethod == 0 &&
                            IHDR->InterlaceMethod == 0)
                        {
                            DecompressedPixels = (u8 *)AllocatePixels(IHDR->Width, IHDR->Height, 4);
                            Supported = true;
                        }
                    }
                    break;

                    case FOURCC("iCCP"):
                    {

                    } break;

                    case FOURCC("bKGD"):
                    {

                    } break;

                    case FOURCC("pHYs"):
                    {

                    } break;

                    case FOURCC("tIME"):
                    {

                    } break;

                    case FOURCC("tEXt"):
                    {
                        char *Keyword = (char *)ChunkData;
                        char *String = Keyword;
                        while(*String++) {}
                        LogInfo("   %s: %.*s\n", Keyword, (u32)((char *)ChunkFooter - String), String);
                    } break;

                    case FOURCC("IDAT"):
                    {
                        if(Supported)
                        {
                            LogInfoUnformatted("    Got an IDAT Chunk\n");
                            png_idat_header *IDATHeader = (png_idat_header *)ChunkData;

                            png_idat_zlib_flags ZlibFlags = {};
                            ZlibFlags.CM = (IDATHeader->ZLibMethodFlags & 0xF);
                            ZlibFlags.CINFO = (IDATHeader->ZLibMethodFlags >> 4);
                            ZlibFlags.FCHECK = (IDATHeader->AdditionalFlags & 0x1F);
                            ZlibFlags.FDICT = (IDATHeader->AdditionalFlags >> 5) & 0x1;
                            ZlibFlags.FLEVEL = (IDATHeader->AdditionalFlags >> 6);

                            LogInfo("   CM: %u\n", ZlibFlags.CM);
                            LogInfo("   CINFO: %u\n", ZlibFlags.CINFO);
                            LogInfo("   FCHECK: %u\n", ZlibFlags.FCHECK);
                            LogInfo("   FDICT: %u\n", ZlibFlags.FDICT);
                            LogInfo("   FLEVEL: %u\n", ZlibFlags.FLEVEL);

                            int x = 0;
                        }
                    }
                    break;

                    case FOURCC("IEND"):
                    {

                    } break;
                }
            }
        }
    }

    LogInfo("SUPPORTED: %s\n", Supported ? "TRUE" : "FALSE");

    free(DecompressedPixels);
}

void
LoadPNG(const char *Filename)
{
    LogInfo("Parsing %s...\n", Filename);
    ParsePNG(ReadFile(Filename));

    int x = 0;
}