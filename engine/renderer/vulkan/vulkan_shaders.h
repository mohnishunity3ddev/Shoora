#if !defined(VULKAN_SHADERS_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

VkShaderModule CreateShaderModule(shoora_vulkan_device *RenderDevice, const char *ShaderFile);

#define VULKAN_SHADERS_H
#endif // VULKAN_SHADERS_H