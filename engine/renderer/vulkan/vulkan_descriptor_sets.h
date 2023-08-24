#if !defined(VULKAN_DESCRIPTOR_SETS_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

// Initializers!!!!
VkDescriptorImageInfo GetImageDescriptorInfo(VkSampler Sampler, VkImageView ImageView, VkImageLayout ImageLayout);

VkDescriptorPoolSize GetDescriptorPoolSize(VkDescriptorType DescriptorType, u32 DescriptorCount);
void CreateDescriptorPool(shoora_vulkan_device *RenderDevice, VkDescriptorPoolSize *PoolSizes, u32 PoolSizeCount,
                          u32 MaxSets, VkDescriptorPool *pDescriptorPool);
void CreateDescriptorPool(shoora_vulkan_device *RenderDevice, u32 PoolSizeCount, VkDescriptorPoolSize *pSizes,
                          u32 DescriptorSetCount, VkDescriptorPool *pDescriptorPool);
VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding(u32 BindingIndex, VkDescriptorType DescriptorType,
                                                           u32 DescriptorCount,
                                                           VkShaderStageFlags ConsumingShaderStage);
void CreatePipelineLayout(shoora_vulkan_device *RenderDevice, u32 SetLayoutCount,
                          VkDescriptorSetLayout *pSetLayouts, u32 PushConstantCount,
                          VkPushConstantRange *pPushConstants, VkPipelineLayout *pPipelineLayout);

void CreateDescriptorSetLayout(shoora_vulkan_device *RenderDevice, VkDescriptorSetLayoutBinding *Bindings,
                               u32 BindingCount, VkDescriptorSetLayout *pSetLayout);
void AllocateDescriptorSets(shoora_vulkan_device *RenderDevice, VkDescriptorPool Pool, u32 Count,
                            VkDescriptorSetLayout *pSetLayouts, VkDescriptorSet *pSets);
void UpdateBufferDescriptorSet(shoora_vulkan_device *RenderDevice, VkDescriptorSet DescriptorSet, u32 BindingIndex,
                               VkDescriptorType DescriptorType, VkBuffer BufferHandle, u64 BufferSize);
void UpdateImageDescriptorSet(shoora_vulkan_device *RenderDevice, VkDescriptorSet DescriptorSet, u32 BindingIndex,
                              VkDescriptorType DescriptorType, VkDescriptorImageInfo *pImageInfo);

void CreateUniformDescriptors(shoora_vulkan_device *RenderDevice, VkShaderStageFlags ConsumingShaderStage,
                              VkDescriptorSetLayout *pSetLayout, u32 PushConstantCount,
                              VkPushConstantRange *pPushConstants, u32 UniformBufferCount,
                              shoora_vulkan_buffer *pUniformBuffers, VkDescriptorSet *pUniformDescriptors,
                              VkPipelineLayout *pPipelineLayout, VkDescriptorPool *pDescriptorPool);

#if 0
struct shoora_sampler_mipmap_info
{
    VkSamplerMipmapMode MipmapMode;
    f32 mipLodBias;
    f32 MinLod;
    f32 MaxLod;
};

struct shoora_sampler_create_info
{
    // For Linear Filtering
    VkFilter SamplerFilter;
    // Behavior for sampler when we sample outside the bounds of the image.
    VkSamplerAddressMode AddressMode;
    // If the AddressingMode was set to ClampToBorder, then this color is returned by the sampler when the image is
    // sampled beyond its borders.
    VkBorderColor BorderColor;
    // For Anisotropic filtering.
    b32 HasAnisotropy;
    // Should check against the max limit of the physical device selected.
    shoora_quality AnisotropyQuality;
    // Sampled values are checked against some reference value.
    b32 ShouldCompare;
    // The Operation involved. Like Less, Greater etc.
    VkCompareOp ComparisonOp;
    // If we create Mipmaps for the image.
    b32 HasMipMaps;
    shoora_sampler_mipmap_info MipMapInfo;

    // The Coordinates for all images sampled through this sampler is done by converting the UV in the range 0-1
    b32 HasNormalizedCoordinates;
};

// IMPORTANT: NOTE:
//

// NOTE: For Samplers and all kinds of image descriptors.
struct shoora_image_description_info
{
    VkDescriptorSet TargetDescriptorSet;
    u32 TargetDescriptorBinding;
    u32 TargetArrayElement;
    VkDescriptorType TargetDescriptorType;
    VkDescriptorImageInfo ImageInfos[8];
    u32 ImageInfoCount;
};

// NOTE: For Uniform and Storage Buffers and their dynamic variations
struct shoora_buffer_description_info
{
    VkDescriptorSet TargetDescriptorSet;
    u32 TargetDescriptorBinding;
    u32 TargetArrayElement;
    VkDescriptorType TargetDescriptorType;
    VkDescriptorBufferInfo BufferInfos[8];
};

// NOTE: For uniform and storage texel buffers
struct shoora_texel_buffer_description_info
{
    VkDescriptorSet TargetDescriptorSet;
    u32 TargetDescriptorBinding;
    u32 TargetArrayElement;
    VkDescriptorType TargetDescriptorType;
    VkBufferView TexelBufferViews[8];
};

// NOTE: This is different than the descriptor infos defined above. This is used when we want to copy the data of
// an already updated desciptor set into a new one.
struct shoora_copy_description_info
{
    VkDescriptorSet TargetDescriptorSet;
    u32 TargetDescriptorBinding;
    u32 TargetArrayElement;
    VkDescriptorSet SourceDescriptorSet;
    u32 SourceDescriptorBinding;
    u32 SourceArrayElement;
    u32 DescriptorCount;
};

b32 CreateTextureAndUniformBufferDescriptor(shoora_vulkan_device *RenderDevice);
#endif

#define VULKAN_DESCRIPTOR_SETS_H
#endif // VULKAN_DESCRIPTOR_SETS_H