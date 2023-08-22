#if !defined(VULKAN_IMGUI_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

void PrepareImGui(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImGuiContext, vec2 ScreenDim,
                  VkRenderPass RenderPass);
void ImGuiNewFrame();
void ImGuiUpdateBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImContext);
void ImGuiDrawFrame(VkCommandBuffer CmdBuffer, shoora_vulkan_imgui *ImContext);
void ImGuiUpdateInput(b32 LMouseClicked, vec2 MousePos);
void ImGuiCleanup(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImContext);

#define VULKAN_IMGUI_H
#endif // VULKAN_IMGUI_H