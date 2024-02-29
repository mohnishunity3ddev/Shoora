#if !defined(VULKAN_GEOMETRY_H)

#include "defines.h"
#include "volk/volk.h"

#include "vulkan_renderer.h"

void UpdateGeometryUniformBuffers(shoora_vulkan_geometry *Geometry, shoora_camera *Camera,
                                  const shu::mat4f &Projection);
void DrawGeometry(VkCommandBuffer DrawCmdBuffer, shoora_vulkan_geometry *Geometry);

void SetupGeometry(shoora_vulkan_device *RenderDevice, shoora_vulkan_geometry *Geometry, shoora_camera *Camera,
                   const shu::mat4f &Projection, VkRenderPass RenderPass, const char *MeshFile,
                   const char *VertexShaderFile, const char *FragmentShaderFile);

void CleanupGeometry(shoora_vulkan_device *RenderDevice, shoora_vulkan_geometry *Geometry);

#define VULKAN_GEOMETRY_H
#endif // VULKAN_GEOMETRY_H