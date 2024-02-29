#if !defined(VULKAN_IMGUI_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

void PrepareImGui(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImGuiContext, shu::vec2u ScreenDim,
                  VkRenderPass RenderPass);
void ImGuiUpdateBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImContext);
void ImGuiDrawFrame(VkCommandBuffer CmdBuffer, shoora_vulkan_imgui *ImContext);
void ImGuiUpdateInputState(f32 MouseX, f32 MouseY, b32 IsLeftMouseDown);
void ImGuiUpdateWindowSize(shu::vec2u WindowDim);

void ImGuiCleanup(shoora_vulkan_device *RenderDevice, shoora_vulkan_imgui *ImContext);

#define VULKAN_IMGUI_H
#endif // VULKAN_IMGUI_H