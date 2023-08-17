#include "vulkan_shaders.h"
#include <cstdio>
#include <memory>

u8 *
ReadShaderFile(const char *Filename, size_t *FileSize)
{
    u8 *Result = nullptr;

    FILE *File = fopen(Filename, "rb");

    if(File == nullptr)
    {
        LogOutput(LogType_Fatal, "Unable to open file: %s\n", Filename);
        Result = nullptr;
    }

    fseek(File, 0, SEEK_END);
    size_t DataSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    ASSERT(DataSize > 0);
    *FileSize = DataSize;

    u8 *Data = (u8 *)malloc(DataSize + 1);
    ASSERT(Data);
    memset(Data, 0, DataSize + 1);

    size_t ItemsRead = fread(Data, DataSize, 1, File);
    ASSERT(ItemsRead == 1);

    fclose(File);
    File = nullptr;

    Result = Data;
    return Result;
}

VkShaderModule
CreateShaderModule(shoora_vulkan_device *RenderDevice, const char *ShaderFile)
{
    size_t FileSize = 0;
    u8 *ShaderSrc = ReadShaderFile(ShaderFile, &FileSize);

    VkShaderModule Result = VK_NULL_HANDLE;

    VkShaderModuleCreateInfo CreateInfo;
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.codeSize = FileSize;
    CreateInfo.pCode = (u32 *)ShaderSrc;

    VK_CHECK(vkCreateShaderModule(RenderDevice->LogicalDevice, &CreateInfo, nullptr, &Result));

    free(ShaderSrc);
    ShaderSrc = nullptr;
    return Result;
}