#include "vulkan_descriptor_sets.h"
#include "vulkan_image.h"
// #include <stb_image.h>

VkDescriptorImageInfo
GetImageDescriptorInfo(VkSampler Sampler, VkImageView ImageView, VkImageLayout ImageLayout)
{
    VkDescriptorImageInfo ImageInfo = {};
    ImageInfo.sampler = Sampler;
    ImageInfo.imageView = ImageView;
    ImageInfo.imageLayout = ImageLayout;

    return ImageInfo;
}

VkDescriptorSetLayoutBinding
GetDescriptorSetLayoutBinding(u32 BindingIndex, VkDescriptorType DescriptorType, u32 DescriptorCount,
                              VkShaderStageFlags ConsumingShaderStage)
{
    VkDescriptorSetLayoutBinding LayoutBinding = {};
    LayoutBinding.binding = BindingIndex;
    LayoutBinding.descriptorType = DescriptorType;
    LayoutBinding.descriptorCount = DescriptorCount;
    LayoutBinding.stageFlags = ConsumingShaderStage;

    return LayoutBinding;
}

VkDescriptorSetLayoutBinding
GetDefaultFragSamplerLayoutBinding()
{
    VkDescriptorSetLayoutBinding LayoutBinding = {};
    LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    LayoutBinding.descriptorCount = 1;
    LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    LayoutBinding.pImmutableSamplers = nullptr;

    return LayoutBinding;
}

void
CreatePipelineLayout(shoora_vulkan_device *RenderDevice, u32 SetLayoutCount, VkDescriptorSetLayout *pSetLayouts,
                     u32 PushConstantCount, VkPushConstantRange *pPushConstants, VkPipelineLayout *pPipelineLayout)
{
    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.setLayoutCount = SetLayoutCount;
    PipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
    PipelineLayoutCreateInfo.pushConstantRangeCount = PushConstantCount;
    PipelineLayoutCreateInfo.pPushConstantRanges = pPushConstants;

    VK_CHECK(vkCreatePipelineLayout(RenderDevice->LogicalDevice, &PipelineLayoutCreateInfo, nullptr,
                                    pPipelineLayout));
}

void
CreateDescriptorSetLayout(shoora_vulkan_device *RenderDevice, VkDescriptorSetLayoutBinding *Bindings,
                          u32 BindingCount, VkDescriptorSetLayout *pSetLayout)
{
    VkDescriptorSetLayoutCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    CreateInfo.bindingCount = BindingCount;
    CreateInfo.pBindings = Bindings;
    VK_CHECK(vkCreateDescriptorSetLayout(RenderDevice->LogicalDevice, &CreateInfo, nullptr, pSetLayout));
}

VkDescriptorPoolSize
GetDescriptorPoolSize(VkDescriptorType DescriptorType, u32 DescriptorCount)
{
    VkDescriptorPoolSize Size;
    Size.type = DescriptorType;
    Size.descriptorCount = DescriptorCount;

    return Size;
}

void
CreateDescriptorPool(shoora_vulkan_device *RenderDevice, VkDescriptorType DescriptorType, u32 DescriptorSetCount,
                     VkDescriptorPool *pDescriptorPool)
{
    VkDescriptorPoolSize PoolSizes[1];
    PoolSizes[0] = GetDescriptorPoolSize(DescriptorType, DescriptorSetCount);

    VkDescriptorPoolCreateInfo PoolCreateInfo = {};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolCreateInfo.pNext = nullptr;
    PoolCreateInfo.flags = 0;
    PoolCreateInfo.maxSets = DescriptorSetCount;
    PoolCreateInfo.poolSizeCount = ARRAY_SIZE(PoolSizes);
    PoolCreateInfo.pPoolSizes = PoolSizes;

    VK_CHECK(vkCreateDescriptorPool(RenderDevice->LogicalDevice, &PoolCreateInfo, nullptr, pDescriptorPool));
}

void
CreateDescriptorPool(shoora_vulkan_device *RenderDevice, u32 PoolSizeCount, VkDescriptorPoolSize *pSizes,
                     u32 MaxSets, VkDescriptorPool *pDescriptorPool)
{
    VkDescriptorPoolCreateInfo PoolCreateInfo = {};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolCreateInfo.pNext = nullptr;
    PoolCreateInfo.flags = 0;
    PoolCreateInfo.maxSets = MaxSets;
    PoolCreateInfo.poolSizeCount = PoolSizeCount;
    PoolCreateInfo.pPoolSizes = pSizes;

    VK_CHECK(vkCreateDescriptorPool(RenderDevice->LogicalDevice, &PoolCreateInfo, nullptr, pDescriptorPool));
}

void
AllocateDescriptorSets(shoora_vulkan_device *RenderDevice, VkDescriptorPool Pool, u32 Count,
                       VkDescriptorSetLayout *pSetLayouts, VkDescriptorSet *pSets)
{
    VkDescriptorSetAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.pNext = nullptr;
    AllocInfo.descriptorPool = Pool;
    AllocInfo.descriptorSetCount = Count;
    AllocInfo.pSetLayouts = pSetLayouts;

    VK_CHECK(vkAllocateDescriptorSets(RenderDevice->LogicalDevice, &AllocInfo, pSets));
}

void
UpdateBufferDescriptorSet(shoora_vulkan_device *RenderDevice, VkDescriptorSet DescriptorSet, u32 BindingIndex,
                          VkDescriptorType DescriptorType, VkBuffer BufferHandle, u64 BufferSize)
{
    VkDescriptorBufferInfo BufferInfo = {};
    BufferInfo.buffer = BufferHandle;
    BufferInfo.offset = 0;
    BufferInfo.range = BufferSize;

    VkWriteDescriptorSet WriteSet = {};
    WriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteSet.dstSet = DescriptorSet;
    WriteSet.descriptorCount = 1;
    WriteSet.descriptorType = DescriptorType;
    WriteSet.pBufferInfo = &BufferInfo;
    WriteSet.dstBinding = BindingIndex;

    vkUpdateDescriptorSets(RenderDevice->LogicalDevice, 1, &WriteSet, 0, nullptr);
}

void
UpdateImageDescriptorSets(shoora_vulkan_device *RenderDevice, VkDescriptorSet DescriptorSet, u32 BindingIndex,
                          VkDescriptorType DescriptorType, u32 DescriptorCount, VkDescriptorImageInfo *pImageInfos)
{
    VkWriteDescriptorSet WriteSet = {};
    WriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteSet.dstSet = DescriptorSet;
    WriteSet.dstBinding = BindingIndex;
    WriteSet.descriptorCount = DescriptorCount;
    WriteSet.descriptorType = DescriptorType;
    WriteSet.pImageInfo = pImageInfos;

    vkUpdateDescriptorSets(RenderDevice->LogicalDevice, 1, &WriteSet, 0, nullptr);
}

VkWriteDescriptorSet
GetWriteDescriptorSet(shoora_vulkan_device *RenderDevice, u32 BindingIndex, VkDescriptorSet Set)
{
    VkWriteDescriptorSet WriteSet = {};
    WriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteSet.dstSet = Set;
    WriteSet.descriptorCount = 1;
    WriteSet.dstBinding = BindingIndex;

    return WriteSet;
}

#if 0
void
SetupDescriptors(shoora_vulkan_device *RenderDevice, shoora_vulkan_swapchain *Swapchain,
                 shoora_uniform_resource_info *DescriptorInfos, u32 DescriptorInfoCount, VkDescriptorPool *pPool,
                 VkDescriptorSetLayout *pSetLayout, VkPipelineLayout *pPipelineLayout, VkDescriptorSet *pSets,
                 u32 SetCount, shoora_vulkan_buffer *pBuffer, shoora_vulkan_image *pImage)
{
    ASSERT(DescriptorInfoCount <= 16);
    VkDescriptorPoolSize PoolSizeInfos[16];
    u32 PoolSizeInfoCount = DescriptorInfoCount;
    VkDescriptorSetLayoutBinding LayoutBindings[16];
    u32 LayoutBindingCount = DescriptorInfoCount;

    for(u32 DescriptorIndex = 0;
        DescriptorIndex < DescriptorInfoCount;
        ++DescriptorIndex)
    {
        shoora_uniform_resource_info *Info = DescriptorInfos + DescriptorIndex;
        PoolSizeInfos[DescriptorIndex] = GetDescriptorPoolSize(Info->DescriptorType, Info->DescriptorCount);

        LayoutBindings[DescriptorIndex] = GetDescriptorSetLayoutBinding(DescriptorIndex, Info->DescriptorType, 1,
                                                                        Info->ConsumingShaderStage);
    }

    CreateDescriptorPool(RenderDevice, PoolSizeInfos, PoolSizeInfoCount, 100, pPool);
    CreateDescriptorSetLayout(RenderDevice, LayoutBindings, LayoutBindingCount, pSetLayout);

    u32 PushConstantCount = 0;
    VkPushConstantRange *PushConstants = nullptr;
    CreatePipelineLayout(RenderDevice, 1, pSetLayout, PushConstantCount, PushConstants, pPipelineLayout);

    for(u32 SetIndex = 0;
        SetIndex = SetCount;
        ++SetIndex)
    {
        VkDescriptorSet *pSet = &pSets[SetIndex];
        AllocateDescriptorSets(RenderDevice, *pPool, 1, pSetLayout, pSet);

        VkWriteDescriptorSet WriteDescriptorSets[16];
        u32 WriteDescriptorCount = DescriptorInfoCount;
        for(u32 TypeIndex = 0;
            TypeIndex < DescriptorInfoCount;
            ++TypeIndex)
        {
            shoora_uniform_resource_info *SetInfo = DescriptorInfos + TypeIndex;
            VkWriteDescriptorSet WriteSet = GetWriteDescriptorSet(RenderDevice, SetInfo->BindingIndex, *pSet);

            if(SetInfo->DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                shoora_vulkan_buffer *Buffer =
                VkDescriptorBufferInfo BufferInfo = {};
                BufferInfo.buffer = BufferHandle;
                BufferInfo.offset = 0;
                BufferInfo.range = BufferSize;

            }
            else if(SetInfo->DescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {

            }
            else
            {
                LogError("[SHOORA]: DescriptorType(%u) not supported yet!", (u32)SetInfo->DescriptorType);
            }
        }

        vkUpdateDescriptorSets(RenderDevice->LogicalDevice, WriteDescriptorCount, WriteDescriptorSets, 0, nullptr);
    }

}
#endif
//? <---------------------------------------------------------------------------------------------------------------->
//  ------------------------------------------------------------------------------------------------------------------
//? <---------------------------------------------------HELPERS------------------------------------------------------>
// -------------------------------------------------------------------------------------------------------------------
//? <---------------------------------------------------------------------------------------------------------------->
#if 0
// NOTE: descriptors represent shader resources. THey are organized into sets.
// their contents are specified by descirptor set layout.
// We being desciprotr sets to the pipeline.
// and then from within shaders we specify from which set and from which location within a set we want to access a
// resource
//
// Defines a set of aprameters that control how image data is loaded inside shaders
// Samplers are for giving the shaders a way to have access to a texture's texel data so that they can use that in
// their logic.
b32
CreateSampler(shoora_vulkan_device *RenderDevice, shoora_sampler_create_info ShCreateInfo, VkSampler *Sampler)
{
    VkSamplerCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.magFilter = ShCreateInfo.SamplerFilter;
    CreateInfo.minFilter = ShCreateInfo.SamplerFilter;
    CreateInfo.addressModeU = ShCreateInfo.AddressMode;
    CreateInfo.addressModeV = ShCreateInfo.AddressMode;
    CreateInfo.addressModeW = ShCreateInfo.AddressMode;

    if(ShCreateInfo.AddressMode == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
    {
        CreateInfo.borderColor = ShCreateInfo.BorderColor;
    }
    else
    {
        CreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    }

    if(ShCreateInfo.HasAnisotropy && RenderDevice->Features.samplerAnisotropy)
    {
        CreateInfo.anisotropyEnable = true;

        u32 MaxAnisotropyAvailable = RenderDevice->DeviceProperties.limits.maxSamplerAnisotropy;
        f32 AnisotropyQuality = (f32)(ShCreateInfo.AnisotropyQuality) + 1.0f;
        f32 MaxQuality = (f32)(shoora_quality::Quality_Count);
        f32 Coeff = (AnisotropyQuality / MaxQuality);
        ASSERT(Coeff <= 1.0f);

        CreateInfo.maxAnisotropy = MaxAnisotropyAvailable * Coeff;
    }
    else
    {
        CreateInfo.anisotropyEnable = false;
        CreateInfo.maxAnisotropy = 1.0f;
    }

    CreateInfo.unnormalizedCoordinates = !ShCreateInfo.HasNormalizedCoordinates;

    if(ShCreateInfo.ShouldCompare)
    {
        CreateInfo.compareEnable = true;
        CreateInfo.compareOp = ShCreateInfo.ComparisonOp;
    }
    else
    {
        CreateInfo.compareEnable = false;
        CreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    }

    if (ShCreateInfo.HasMipMaps)
    {
        CreateInfo.mipmapMode = ShCreateInfo.MipMapInfo.MipmapMode;
        CreateInfo.mipLodBias = ShCreateInfo.MipMapInfo.mipLodBias;
        CreateInfo.minLod = ShCreateInfo.MipMapInfo.MinLod;
        CreateInfo.maxLod = ShCreateInfo.MipMapInfo.MaxLod;
    }
    else
    {
        CreateInfo.mipLodBias = 0.0f;
        CreateInfo.minLod = 0.0f;
        CreateInfo.maxLod = 0.0f;
    }

    VK_CHECK(vkCreateSampler(RenderDevice->LogicalDevice, &CreateInfo, 0, Sampler));
    ASSERT(*Sampler != VK_NULL_HANDLE);
    return true;
}

// NOTE: Sampeld Images are used as a source of image data inside shaders.
b32
CreateSampledImage(shoora_vulkan_device *RenderDevice, VkImage ImageHandle, VkFormat ImageFormat, b32 IsLinearFiltering)
{
    ASSERT(ImageHandle != VK_NULL_HANDLE);

    // NOTE: We have to do this everytime. It is advised to NOT cache format properties for a specific image format from
    // the physical device.
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, ImageFormat, &FormatProperties);

    if(!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
    {
        return false;
    }

    if(IsLinearFiltering &&
       (!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)))
    {
        return false;
    }

    VkImageView ImageView = VK_NULL_HANDLE;
    // TODO)): CreateImageView using the information we have gathered till now.
    ASSERT(ImageView != VK_NULL_HANDLE);

    // IMPORTANT: NOTE:
    // Before we can use the texture/image as a source of data inside of shaders, we need to set the image layout to
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // This can happen right now or later when we actually use this image inside the shader.
    return true;
}

b32
CreateCombinedImageSampler(shoora_vulkan_device *RenderDevice)
{
    // TODO)): Create a Sampler Object from above functions
    // NOTE: bool linear_filtering = (mag_filter == VK_FILTER_LINEAR) || (min_filter ==
    //          VK_FILTER_LINEAR) || (mipmap_mode == VK_SAMPLER_MIPMAP_MODE_LINEAR);
    //          This linearFiltering is info that we have to send to SampledImage function to tell it if the image
    //          has data linearly filled out or should be linearly filled out.
    // TODO)): Create Sampled Image and get an ImageView out of it
    return true;
}

// Shaders can store their data to images.
// Usage is VK_IMAGE_USAGE_STORAGE_BIT
b32
CreateStorageImageForShaders(shoora_vulkan_device *RenderDevice, b32 AtomicOp)
{
    VkFormat Format;
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, Format, &FormatProperties);

    // Check whether VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT is supported!
    if(!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
    {
        return false;
    }

    if(AtomicOp && !(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT))
    {
        return false;
    }

    // TODO)): Create Image with Usage VK_IMAGE_USAGE_STORAGE_BIT
    VkImage StorageImage;

    // TODO)): AllocateAndBindMemoryToImage()

    VkImageView StorageImageView = VK_NULL_HANDLE;
    // TODO)): Create imageView using the above functions
    // TODO)): To support images to laod and store data to from withinshaders, the image layout for these images should be
    //          VK_IMAGE_LAYOUT_GENERAL

    return true;
}

// Similar to reading data from simple images but instead of reading the data as aingular value, the data is read as formatted
// pixels(texels) with 1,2,3 or 4 components.
// Usage Type is VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
b32
CreateUniformTexelBuffer(shoora_vulkan_device *RenderDevice)
{
    VkFormat Format;
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, Format, &FormatProperties);

    if(!(FormatProperties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
    {
        return false;
    }

    VkBuffer UniformTexelBuffer;
    // TODO)): Create Buffer with Usage VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT

    // TODO)): Allocate and Bind Memory to the buffer.

    VkBufferView UniformTexelBufferView;
    // TODO)): Create the buffer view.
    return true;
}

// Like Uniform Texel Buffers they can be read from in shaders
// These also allow storage of data and perform atomic operations on them
// Usage Flag is VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
b32
CreateStorageTexelBuffer(shoora_vulkan_device *RenderDevice, b32 AtomicOp)
{
    VkFormat Format;
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, Format, &FormatProperties);
    if(!(FormatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
    {
        return false;
    }
    if(AtomicOp && !(FormatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT))
    {
        return false;
    }

    VkBuffer StorageTexelBuffer;
    // TODO)): CreateBuffer with usage VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
    // TODO)): AllocateAndBinfMemory

    // This buffer view defines the layout of how the image data is to be read. For ex: RGB or RGBA etc.
    VkBufferView StorageTexelBufferView;
    // TODO)): Create the buffer view!

    return true;
}

// NOTE: Uniform Buffers are used for matrics/v2/scalar values to be usded in the shader.
//          This data should be laid out inside the buffer in the order defined by the std140 layout of the GLSL
//          language.
b32
CreateUniformBuffer(shoora_vulkan_device *RenderDevice)
{
    VkBuffer UniformBuffer;

    // TODO)): Create Buffer using VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    // TODO)): AllocateMemory in Device Local Memory

    return true;
}

// NOTE: For both reading and storing data inside the vairable in the shader, we use a storage buffer.
//          Can also perform atomic operations on its members which have unsigned integer formats.
//          Placement of variables inside this buffer should adhere to the std430 layout of the GLSL language.
//          Used for vectors/matrices/scalar data.
b32
CreateStorageBuffer(shoora_vulkan_device *RenderDevice)
{
    VkBuffer StorageBuffer;

    // TODO)): Create Buffer using VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    // TODO)): AllocateMemory in Device Local Memory

    return true;
}

// NOTE: These are the images into which the result is stored let's say by the fragment shader. These are also
//          called render targets. These are the things into which we render during drawing commands, inside
//          render passes
// These Images should have Usage Flags set to VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
//
// NOTE: These are image resources from which we can read data from within shdaers. The only difference is that
//          we can only access one location corresponding to a processed fragment.0
//
// IMPORTANT: NOTE:
// One common scenario for input attachments is for post processing passes. A renderpass can have
// multiple subpasses. One subpass writes into a render target and that render target can be sent to subsequest
// subpass as an input attachment so that the rendererd image can be read to do post processing.
b32
CreateInputAttachment(shoora_vulkan_device *RenderDevice)
{
    VkFormat Format;
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, Format, &FormatProperties);

    VkImageAspectFlags ImageAspect;
    b32 WillWeReadColorDataFromThisImage = (ImageAspect & VK_IMAGE_ASPECT_COLOR_BIT);
    if (WillWeReadColorDataFromThisImage &&
        !(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
    {
        return false;
    }

    b32 WillWeReadDepthStencilDataFromThisImage = (ImageAspect &
                                                   (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
    if (WillWeReadDepthStencilDataFromThisImage &&
        !(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        return false;
    }

    VkImage InputAttachment;
    // TODO)): Create the Image. make sure that VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT is set.

    // TODO)): Allocate Memory from Device Local Memory and Bind the memory to the image.

    VkImageView InputAttachmentView;

    // TODO)): Create the Image View for this!

    return true;
}

// NOTE: Defines the internal structure of a descriptor set and strictly defines what resources can bind to it.
//          The index of a resource(descritor set) is given the name "binding"
b32
CreateDescriptorSetLayout(shoora_vulkan_device *RenderDevice)
{
    VkDescriptorSetLayoutBinding Bindings[16];

    // For each resource you want to make and later bind to the pipeline using a descriptor set
    for(u32 Index = 0; Index < 16; ++Index)
    {
        VkDescriptorSetLayoutBinding Binding = Bindings[Index];
        // The index of the resource inside the descriptor set layout
        Binding.binding = Index;
        // The type of resource bound to this index in the descriptor set layout.
        Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // The number of resources of this type accessed in the shader through arrays
        Binding.descriptorCount = 1;
        // The Logical OR of all the stages where this resource will be used
        Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        Binding.pImmutableSamplers = nullptr;

    }

    VkDescriptorSetLayoutCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.bindingCount = ARRAY_SIZE(Bindings);
    CreateInfo.pBindings = Bindings;

    VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
    VK_CHECK(vkCreateDescriptorSetLayout(RenderDevice->LogicalDevice, &CreateInfo, 0, &DescriptorSetLayout));
    ASSERT(DescriptorSetLayout != VK_NULL_HANDLE);

    return true;
}

// NOTE: Descriptors gathered into descriptor sets are allocated from descriptor pool
//          When we create a pool, we must define which descriptors, and how many of them can be allocated from the
//          created pool.
b32
CreateDescriptorPool(shoora_vulkan_device *RenderDevice)
{
    VkDescriptorPoolSize DescriptorTypes[16];

    // NOTE: For each type of descriptor that will be allocated from the pool, we add an entry here in this array.
    //          Basically how many descriptors of a given type are there.
    for(u32 Index = 0; Index < 16; ++Index)
    {
        VkDescriptorPoolSize DescriptorType = DescriptorTypes[Index];

        DescriptorType.descriptorCount = 2;
        DescriptorType.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }

    VkDescriptorPoolCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    CreateInfo.pNext = nullptr;
    // NOTE: Go with this flag if we want to be able to free individual descriptor sets, 0 if we want to free all
    //          descriptor sets allocated from this pool when we reset the descirptor pool
    CreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    // The maximum number of sets allocated from this pool
    CreateInfo.maxSets = 1;
    CreateInfo.poolSizeCount = ARRAY_SIZE(DescriptorTypes);
    CreateInfo.pPoolSizes = DescriptorTypes;

    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    vkCreateDescriptorPool(RenderDevice->LogicalDevice, &CreateInfo, 0, &DescriptorPool);
    ASSERT(DescriptorPool != VK_NULL_HANDLE);

    return true;
}

// NOTE: Descritptor sets gather shader resources in one container object. Its contents, resource types and the
//          number of resources are described by the descriptor set layout. Storage is taken from pools from
//          which we can allocate descriptor sets
//
// IMPORTANT: NOTE:
// Descriptor sets provide resources to the shaders. They form an interface between the application and the
// programmable pipeline stages like Vertex Shader and the fragment Shader. The structure of this interface is
// described by the descriptor set layouts.
// The actual data is provided via buffers, images when we upadte the descriptor set and later bind them to command
// buffers during a recording operation.
b32
AllocateDescriptorSet(shoora_vulkan_device *RenderDevice)
{
    // This is where we allocate the descriptor set from
    VkDescriptorPool Pool;
    // TODO)): Create the descriptor pool from the above methods.
    ASSERT(Pool != VK_NULL_HANDLE);

    // For each descriptor set to be allocated from this pool, we take the information from its corresponding
    // Layout here which describes the descriptor type, the number of resources and its count.
    VkDescriptorSetLayout Layouts[16];

    VkDescriptorSetAllocateInfo AllocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    AllocInfo.pNext = nullptr;
    AllocInfo.descriptorPool = Pool;
    AllocInfo.descriptorSetCount = ARRAY_SIZE(Layouts);
    AllocInfo.pSetLayouts = Layouts;

    VkDescriptorSet DescriptorSets[16];
    VK_CHECK(vkAllocateDescriptorSets(RenderDevice->LogicalDevice, &AllocInfo, DescriptorSets));

    return true;
}

// NOTE: This is where we store the actual data the descriptor will point to when references inside of shaders.
//          The Provided data will depend on the resource type the descriptor was created to be applcable to.
//          Updating Descriptor sets causes specified resources to populate entries in the indicated sets. When the
//          updated set is bound to a pipeline, the shaders can have access to these resources.
// IMPORTANT: NOTE:
// Custom structs are provided for different resource types
// shoora_image_description_info
// shoora_buffer_description_info
// shoora_texel_buffer_description_info
// shoora_copy_description_info
b32
WriteDescriptorSet(shoora_vulkan_device *RenderDevice, shoora_image_description_info ImageDescriptorInfo)
{
    VkDescriptorImageInfo ImageInfo = ImageDescriptorInfo.ImageInfos[0];
    // ImageInfo.sampler = ;
    // ImageInfo.imageView = ;
    // ImageInfo.imageLayout = ;

    VkWriteDescriptorSet WriteDescriptors[16];
    for (u32 Index = 0; Index < 16; ++Index)
    {
        VkWriteDescriptorSet WriteDescriptor = WriteDescriptors[Index];
        WriteDescriptor.sType = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        WriteDescriptor.pNext = nullptr;
        WriteDescriptor.dstSet = ImageDescriptorInfo.TargetDescriptorSet;
        WriteDescriptor.dstBinding = ImageDescriptorInfo.TargetDescriptorBinding;
        WriteDescriptor.dstArrayElement = 0;
        WriteDescriptor.descriptorCount = ImageDescriptorInfo.ImageInfoCount;
        WriteDescriptor.descriptorType = ImageDescriptorInfo.TargetDescriptorType;
        WriteDescriptor.pImageInfo = ImageDescriptorInfo.ImageInfos;
        WriteDescriptor.pBufferInfo = 0;
        WriteDescriptor.pTexelBufferView = 0;

    }

    vkUpdateDescriptorSets(RenderDevice->LogicalDevice, ARRAY_SIZE(WriteDescriptors), WriteDescriptors, 0,
                           nullptr);
    return true;
}

// NOTE: When a descriptor set is ready with all the resources the shaders are going to be accessing, we can bind
// them to command buffers during a recording operation.
// Before we can record drawing operations that references images and buffers, we need to bind those resources to
// the command buffer. This is what exactly this function does.
b32
BindDescriptorSet(shoora_vulkan_device *RenderDevice, VkCommandBuffer CmdBuffer)
{
    // TODO)): ASSERT(CmdBuffer is in recording state !);

    // Represents the type of pipeline(Graphics/Compute) where the descriptor sets will be used!
    VkPipelineBindPoint PipelineType;

    VkPipelineLayout PipelineLayout;

    // TODO)): Initialize the descriptor set which will be used in the pipeline.
    VkDescriptorSet DescriptorSets[16];

    u32 IndexForFirstSet = 0;

    vkCmdBindDescriptorSets(CmdBuffer, PipelineType, PipelineLayout, IndexForFirstSet, ARRAY_SIZE(DescriptorSets),
                            DescriptorSets, 0, nullptr);
    return true;
}





// IMPORTANT: Example
#include "utils/utils.h"
#include <memory.h>

u32
GetMaxImageMipLevels(u32 ImageWidth, u32 ImageHeight)
{
    u32 Max = ImageWidth;
    if (ImageHeight > Max)
    {
        Max = ImageHeight;
    }
    u32 Result = LogBase2(Max);
    return Result;
}

void
Create2DVulkanImageForSampling(shoora_vulkan_device *RenderDevice, u32 ImageWidth, u32 ImageHeight,
                               VkFormat ImageFormat, VkImageUsageFlags UsageFlags, u32 MipMapCount, b32 GenerateMipMaps,
                               VkImage *ImageHandle)
{
    VkImageCreateInfo ImageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ImageCreateInfo.pNext = nullptr;
    ImageCreateInfo.flags = 0;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format = ImageFormat;
    ImageCreateInfo.extent = {.width = ImageWidth, .height = ImageHeight, .depth = 1};
    ImageCreateInfo.mipLevels = MipMapCount;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    // TODO)): Ask for a tiling. If it is linear tiling then we have to make sure we dnt make a staging buffer to
    // upload to the GPU since we can use it directly. Linear tiling means the data is laid out linearly which is
    // easier to read for the CPU. But GPU will have a hard time.
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // NOTE: Since we transfer the image data from staging buffer to the image, we set its usage to be a
    // destination of image data.
    ImageCreateInfo.usage = UsageFlags;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.queueFamilyIndexCount = 0;
    ImageCreateInfo.pQueueFamilyIndices = nullptr;
    ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(RenderDevice->LogicalDevice, &ImageCreateInfo, 0, ImageHandle));
}

// NOTE: We create a combined image sampler and a uniform buffer
// 1. Prepare a descriptor set layout mentioning that the descriptors will have an image sampler and a uniform buffer.
// 2. Create a Descriptor Pool to allocate descriptor sets out of.
// 3. Allocate a Descriptor set out of it.
// 4. Update the descriptor set with these resources.
// 5. Bind the command buffer with the descriptor so that they can be accessed from within the shaders.
b32
CreateTextureAndUniformBufferDescriptor(shoora_vulkan_device *RenderDevice)
{
    // TODO)): Implement the rest of these.
    // NOTE: Create a combined Image Sampler(Image, ImageView, Sampler)

    // Image Data
    i32 ImageWidth;
    i32 ImageHeight;
    i32 BytesPerPixel;
    u8 *pImageData = stbi_load("images/cyberpunk_keanu_03.jpg", &ImageWidth, &ImageHeight, &BytesPerPixel, 0);
    // GenerateMipMaps("images/cyberpunk_person_mask.jpg");
    u64 MipOffsets[16] = {};
    GenerateMipMaps("images/cyberpunk_keanu_02.jpg", "images/generated_mips.jpg", 8, 100, MipOffsets);
    VkDeviceSize ImageSizeInBytes = ImageWidth*ImageHeight*BytesPerPixel;


    // Setup staging buffer to upload image data from CPU to the GPU
    VkBuffer StagingBuffer;
    VkBufferCreateInfo StagingBufferCreateInfo = {};
    StagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    StagingBufferCreateInfo.pNext = nullptr;
    StagingBufferCreateInfo.flags = 0;
    StagingBufferCreateInfo.size = ImageSizeInBytes;
    StagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    StagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    StagingBufferCreateInfo.queueFamilyIndexCount = 0;
    StagingBufferCreateInfo.pQueueFamilyIndices = nullptr;
    VK_CHECK(vkCreateBuffer(RenderDevice->LogicalDevice, &StagingBufferCreateInfo, nullptr, &StagingBuffer));

    VkDeviceMemory StagingMemory;
    VkMemoryRequirements StagingBufferMemoryRequirements;
    vkGetBufferMemoryRequirements(RenderDevice->LogicalDevice, StagingBuffer, &StagingBufferMemoryRequirements);
    VkMemoryAllocateInfo StagingMemoryAllocInfo;
    StagingMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    StagingMemoryAllocInfo.pNext = nullptr;
    StagingMemoryAllocInfo.allocationSize = StagingBufferMemoryRequirements.size;
    StagingMemoryAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice,
                                                                 StagingBufferMemoryRequirements.memoryTypeBits,
                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &StagingMemoryAllocInfo, nullptr, &StagingMemory));
    VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, StagingBuffer, StagingMemory, 0));

    // Store ImageData to the stageing buffer
    u8 *pImageDataMappedPtr;
    VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, StagingMemory, 0, StagingBufferMemoryRequirements.size, 0,
                         (void **)&pImageDataMappedPtr));
    memcpy(pImageDataMappedPtr, pImageData, ImageSizeInBytes);
    vkUnmapMemory(RenderDevice->LogicalDevice, StagingMemory);

    // TODO)): Remove this and implement mipmapping
    u32 MipMapCount = 1;
    // u32 MipMapCount = GetMaxImageMipLevels(ImageWidth, ImageHeight) + 1;

    // Now we need to make the bufferCopyRegions for each mip level, so that it can be copied from the buffer to the image
    ASSERT(MipMapCount <= 32);
    VkBufferImageCopy BufferCopyRegions[32];
    for(u32 MipLevel = 0;
        MipLevel < MipMapCount;
        ++MipLevel)
    {
        VkBufferImageCopy Region = BufferCopyRegions[MipLevel];
        Region = {};

        u32 Width = MAX(1U, ImageWidth >> MipLevel);
        u32 Height = MAX(1U, ImageHeight >> MipLevel);

        // TODO)): Figure out to these offsets for different mip levels when mipmapping was done!
        u32 MipOffset = 0;

        Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Region.imageSubresource.baseArrayLayer = 0;
        Region.imageSubresource.layerCount = 1;
        Region.imageSubresource.mipLevel = MipLevel;
        Region.imageExtent.width = Width;
        Region.imageExtent.height = Height;
        Region.imageExtent.depth = 1;

        Region.bufferOffset = MipOffset;
    }

    // Create a VkImage Handle
    VkImage ImageHandle = VK_NULL_HANDLE;
    VkFormat ImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    Create2DVulkanImageForSampling(RenderDevice, ImageWidth, ImageHeight, ImageFormat,
                                   VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, MipMapCount,
                                   false, &ImageHandle);
    ASSERT(ImageHandle != VK_NULL_HANDLE);

    // Getting Memory for the Image on the GPU memory
    VkMemoryRequirements ImageMemoryReqs = {};
    vkGetImageMemoryRequirements(RenderDevice->LogicalDevice, ImageHandle, &ImageMemoryReqs);
    VkMemoryAllocateInfo ImageMemoryAllocInfo = {};
    ImageMemoryAllocInfo.pNext = nullptr;
    ImageMemoryAllocInfo.allocationSize = ImageMemoryReqs.size;
    ImageMemoryAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, ImageMemoryReqs.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDeviceMemory ImageMemory;
    VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &ImageMemoryAllocInfo, nullptr, &ImageMemory));
    VK_CHECK(vkBindImageMemory(RenderDevice->LogicalDevice, ImageHandle, ImageMemory, 0));

    // NOTEL At this point, we have made the iamge and the staging buffer which is filled with image data we had from the
    // file.
    // Now, we want to upload the data to the GPU memory(Image)
    // But first we want to transition the layout of the image to be TRANSFER_DST_BIT so that it is optimal for us to
    // treat it as a destination in a transfer op.
    VkImageSubresourceRange SubresourceRange = {};
    SubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    SubresourceRange.baseMipLevel = 0; // Mipmap level to start at
    SubresourceRange.levelCount = 1;   // TODO)): Do Mipmapping later. leaving this as 1 for now
    SubresourceRange.layerCount = 1;
    VkImageMemoryBarrier ImageMemoryBarrier = {};
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImageMemoryBarrier.pNext = nullptr;
    ImageMemoryBarrier.srcAccessMask = 0;
    ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.image = ImageHandle;
    ImageMemoryBarrier.subresourceRange = SubresourceRange;
    // TODO)): Get this Command Buffer from the device.
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    // NOTE: VK_PIPELINE_STAGE_HOST_BIT pseudostage communicates there will be memory domain transfers between host
    // and device.
    vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &ImageMemoryBarrier);

    // NOTE: Now, After doing the synchronization to make sure that before the GPU starts the transfer, all CPU stuff is
    // completed!
    // Now, we start the Transfer from the Buffer which contains image data to the device local memory for the image.
    vkCmdCopyBufferToImage(CommandBuffer, StagingBuffer, ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           MipMapCount, BufferCopyRegions);

    // NOTE: Now we want to synchronize the transfer and the shader read, since this image will be sampled from in
    // the fragment shader. we want change the image layout and also make sure the transfer has been completed
    // before the fragment shader uses the image.
    ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // The synchronization starts frmo the transfer pipeline stage and ends at fragment shader stage.
    // The kind of accesses at these respective stages are "Transfer_Write" and "Shader_Read"
    // We also specify the image layout based on the kind of uage its going to go through.
    vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);

    // NOTE: Now that the transfer is complete, and the image is there in GPU memory with the correct usage in the pipeline
    // and the layout done, we can get rid of the staging buffer.
    vkFreeMemory(RenderDevice->LogicalDevice, StagingMemory, nullptr);
    vkDestroyBuffer(RenderDevice->LogicalDevice, StagingBuffer, nullptr);
    StagingBuffer = VK_NULL_HANDLE;

    // NOTE: Create the Sampler for the Image so that it can be sampled from in the fragment shader.
    VkSampler ImageSampler = VK_NULL_HANDLE;
    shoora_sampler_create_info SamplerCreateInfo = {};
    SamplerCreateInfo.SamplerFilter = VK_FILTER_LINEAR;
    SamplerCreateInfo.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCreateInfo.HasAnisotropy = true;
    SamplerCreateInfo.HasNormalizedCoordinates = true;
    CreateSampler(RenderDevice, SamplerCreateInfo, &ImageSampler);

    // NOTE: Create Image View
    VkImageView ImageView = VK_NULL_HANDLE;

    VkImageViewCreateInfo ImageViewCreateInfo = {};
    VkImageSubresourceRange ImageSubresourceRange = {};
    ImageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageSubresourceRange.baseMipLevel = 0;
    ImageSubresourceRange.levelCount = 1; // 1 mipmap
    ImageSubresourceRange.baseArrayLayer = 0;
    ImageSubresourceRange.layerCount = 1;

    ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewCreateInfo.pNext = nullptr;
    ImageViewCreateInfo.flags = 0;
    ImageViewCreateInfo.image = ImageHandle;
    ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ImageViewCreateInfo.format = ImageFormat;
    ImageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                      VK_COMPONENT_SWIZZLE_A};

    ImageViewCreateInfo.subresourceRange = ImageSubresourceRange;
    vkCreateImageView(RenderDevice->LogicalDevice, &ImageViewCreateInfo, nullptr, &ImageView);
    ASSERT(ImageView != VK_NULL_HANDLE);

    // -------------------------------------------------------------------------------------------------------------
    // UNIFORM BUFFER ----------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    VkBuffer UniformBuffer = VK_NULL_HANDLE;
    VkDeviceSize UniformBufferSize = 1024;
    void *UniformBufferData = nullptr;

    VkBufferCreateInfo BufferCreateInfo = {};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.pNext = nullptr;
    BufferCreateInfo.size = UniformBufferSize;
    BufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(RenderDevice->LogicalDevice, &BufferCreateInfo, nullptr, &UniformBuffer));

    VkMemoryRequirements BufferMemoryReqs = {};
    VkMemoryPropertyFlags BufferMemoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkGetBufferMemoryRequirements(RenderDevice->LogicalDevice, UniformBuffer, &BufferMemoryReqs);
    VkDeviceMemory UniformBufferMemory;
    VkMemoryAllocateInfo BufferAllocInfo = {};
    BufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    BufferAllocInfo.pNext = nullptr;
    BufferAllocInfo.allocationSize = BufferMemoryReqs.size;
    BufferAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, BufferMemoryReqs.memoryTypeBits,
                                                          BufferMemoryProps);

    VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &BufferAllocInfo, nullptr, &UniformBufferMemory));
    VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, UniformBuffer, UniformBufferMemory, 0));

    // Store the Buffer Data to its pointer.
    if(UniformBufferData != nullptr)
    {
        void *BufferMappedPtr;
        VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, UniformBufferMemory, 0, UniformBufferSize,
                             0, &BufferMappedPtr));
        ASSERT(BufferMappedPtr);

        memcpy(BufferMappedPtr, UniformBufferData, UniformBufferSize);

        // NOTE: If the memory type that we got DOES NOT have Host Coherency bit, then we have to flush the
        // buffer so that the updated data is visible to the GPU as well instantly. otherwise there is no guarantee
        // that the GPU will be able to view these changes.
        if((BufferMemoryProps & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            VkMappedMemoryRange MemRange = {};
            MemRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            MemRange.pNext = nullptr;
            MemRange.memory = UniformBufferMemory;
            MemRange.offset = 0;
            MemRange.size = UniformBufferSize;

            vkFlushMappedMemoryRanges(RenderDevice->LogicalDevice, 1, &MemRange);
        }

        vkUnmapMemory(RenderDevice->LogicalDevice, UniformBufferMemory);
        BufferMappedPtr = nullptr;
    }

    // ----------------------------------------------------------------------------------------------------------
    // NOTE: Bind Shader Resources to Descriptors and Descriptor sets and do all that jazz associated with it.
    // -------------------------------------------------------------------------------------------------------------
    VkDescriptorSetLayoutBinding Bindings[2];
    u32 BindingCount = 2;

    // binding for the image sampler to be used in the fragment shader.
    Bindings[0].binding = 0;
    Bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Bindings[0].descriptorCount = 1;
    Bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Uniform Buffer binding for Vertex shader uniforms
    Bindings[1].binding = 1;
    Bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Bindings[1].descriptorCount = 1;
    Bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Create the Descriptor set layout
    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {};
    DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DescriptorSetLayoutCreateInfo.pNext = nullptr;
    DescriptorSetLayoutCreateInfo.bindingCount = 2;
    DescriptorSetLayoutCreateInfo.pBindings = Bindings;

    VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
    VK_CHECK(vkCreateDescriptorSetLayout(RenderDevice->LogicalDevice, &DescriptorSetLayoutCreateInfo, nullptr,
                                         &DescriptorSetLayout));

    // Setup Descriptor Pool from where the descriptor sets will be allocated.
    VkDescriptorPoolSize DescriptorTypes[2];
    DescriptorTypes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DescriptorTypes[0].descriptorCount = 1;
    DescriptorTypes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DescriptorTypes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo PoolCreateInfo = {};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    PoolCreateInfo.pNext = nullptr;
    PoolCreateInfo.maxSets = 2;
    PoolCreateInfo.poolSizeCount = 2;
    PoolCreateInfo.pPoolSizes = DescriptorTypes;

    VkDescriptorPool DescriptorPool;
    vkCreateDescriptorPool(RenderDevice->LogicalDevice, &PoolCreateInfo, nullptr, &DescriptorPool);
    // ----------------------------------------------------------------------------------------------------------

    // -------------------------------------------------------------------------------------------------------------
    // NOTEL Make the Descriptor Set based on the Descriptor set layout we defined above.
    // -------------------------------------------------------------------------------------------------------------
    VkDescriptorSet DescriptorSets[1];
    VkDescriptorSetAllocateInfo DescriptorSetAllocInfo;
    DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    DescriptorSetAllocInfo.pNext = nullptr;
    DescriptorSetAllocInfo.descriptorPool = DescriptorPool;
    DescriptorSetAllocInfo.descriptorSetCount = 1;
    DescriptorSetAllocInfo.pSetLayouts = &DescriptorSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(RenderDevice->LogicalDevice, &DescriptorSetAllocInfo, DescriptorSets));

    // Update the Descriptor sets to bind them to actual data resources
    VkWriteDescriptorSet WriteDescriptorSets[2];
    // Sampled Image Descriptor
    VkDescriptorImageInfo ImageDescriptorInfo;
    ImageDescriptorInfo.sampler = ImageSampler;
    ImageDescriptorInfo.imageView = ImageView;
    ImageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    WriteDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteDescriptorSets[0].pNext = nullptr;
    WriteDescriptorSets[0].dstSet = DescriptorSets[0];
    WriteDescriptorSets[0].dstBinding = 0;
    WriteDescriptorSets[0].descriptorCount = 1;
    WriteDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    WriteDescriptorSets[0].pImageInfo = &ImageDescriptorInfo;
    WriteDescriptorSets[0].pBufferInfo = nullptr;
    WriteDescriptorSets[0].pTexelBufferView = nullptr;

    // Uniform Buffer Descriptor Set.
    VkDescriptorBufferInfo BufferDescriptorInfo;
    BufferDescriptorInfo.buffer = UniformBuffer;
    BufferDescriptorInfo.offset = 0;
    BufferDescriptorInfo.range = VK_WHOLE_SIZE;
    WriteDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteDescriptorSets[1].pNext = nullptr;
    WriteDescriptorSets[1].dstSet = DescriptorSets[0];
    WriteDescriptorSets[1].dstBinding = 1;
    WriteDescriptorSets[1].descriptorCount = 1;
    WriteDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    WriteDescriptorSets[1].pImageInfo = nullptr;
    WriteDescriptorSets[1].pBufferInfo = &BufferDescriptorInfo;
    WriteDescriptorSets[1].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(RenderDevice->LogicalDevice, 2, WriteDescriptorSets, 0, nullptr);

    // -------------------------------------------------------------------------------------------------------------
    // Free Descriptor Sets ----------------------------------------------------------------------------------------
    // NOTE: Only descriptor sets allocated from descriptor pool with flag VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    // can be allowed to be freed individually. Otherwise the individual descriptor are not allowerd to be freed. They are
    // freed when we reset the desciptor pool or destroy it.
    // -------------------------------------------------------------------------------------------------------------
    VK_CHECK(vkFreeDescriptorSets(RenderDevice->LogicalDevice, DescriptorPool, 1, DescriptorSets));

    // -------------------------------------------------------------------------------------------------------------
    // Reset the Descriptor Pool -----------------------------------------------------------------------------------
    // NOTE: Free all the descriptor sets allocated from this pool.
    // -------------------------------------------------------------------------------------------------------------
    VK_CHECK(vkResetDescriptorPool(RenderDevice->LogicalDevice, DescriptorPool, 0));


    // -------------------------------------------------------------------------------------------------------------
    // Destroy Command Pool
    // NOTE: All Descriptor sets allocated from this are freed. We have to make sure all the descriptor sets allocated
    // from this are not being referenced at this point by any of the commands that are being processed at the moment.
    // -------------------------------------------------------------------------------------------------------------
    vkDestroyDescriptorPool(RenderDevice->LogicalDevice, DescriptorPool, nullptr);
    DescriptorPool = VK_NULL_HANDLE;


    // -------------------------------------------------------------------------------------------------------------
    // Destroy DescriptorSet Layout --------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    vkDestroyDescriptorSetLayout(RenderDevice->LogicalDevice, DescriptorSetLayout, nullptr);
    DescriptorSetLayout = VK_NULL_HANDLE;

    // -------------------------------------------------------------------------------------------------------------
    // Destroy Image Sampler ---------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    vkDestroySampler(RenderDevice->LogicalDevice, ImageSampler, nullptr);

    return true;
}
#endif