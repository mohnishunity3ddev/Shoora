#include "vulkan_descriptor_sets.h"

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

    if(ShCreateInfo.HasAnisotropy && RenderDevice->DeviceFeatures.samplerAnisotropy)
    {
        CreateInfo.anisotropyEnable = true;

        u32 MaxAnisotropyAvailable = RenderDevice->DeviceProperties.limits.maxSamplerAnisotropy;
        f32 AnisotropyQuality = (f32)(ShCreateInfo.AnisotropyQuality + 1.0f);
        f32 MaxQuality = (f32)(shoora_quality::Quality_Count);
        f32 Coeff = (AnisotropyQuality / MaxQuality);
        ASSERT(Coeff <= 1.0f);

        CreateInfo.maxAnisotropy = MaxAnisotropyAvailable * Coeff;
    }

    CreateInfo.unnormalizedCoordinates = !ShCreateInfo.HasNormalizedCoordinates;

    if(ShCreateInfo.ShouldCompare)
    {
        CreateInfo.compareEnable = true;
        CreateInfo.compareOp = ShCreateInfo.ComparisonOp;
    }

    if (ShCreateInfo.HasMipMaps)
    {
        CreateInfo.mipmapMode = ShCreateInfo.MipMapInfo.MipmapMode;
        CreateInfo.mipLodBias = ShCreateInfo.MipMapInfo.mipLodBias;
        CreateInfo.minLod = ShCreateInfo.MipMapInfo.MinLod;
        CreateInfo.maxLod = ShCreateInfo.MipMapInfo.MaxLod;
    }

    VK_CHECK(vkCreateSampler(RenderDevice->LogicalDevice, &CreateInfo, 0, Sampler));
    ASSERT(*Sampler != VK_NULL_HANDLE);
    return true;
}

// NOTE: Sampeld Images are used as a source of image data inside shaders.
b32
CreateSampledImage(shoora_vulkan_device *RenderDevice, VkImage ImageHandle, VkFormat ImageFormat, b32 IsLinearTiling)
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

    if(IsLinearTiling &&
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

// NOTE: We create a combined image sampler and a uniform buffer
// 1. Prepare a descriptor set layout mentioning that the descriptors will have an image sampler and a uniform buffer.
// 2. Create a Descriptor Pool to allocate descriptor sets out of.
// 3. Allocate a Descriptor set out of it.
// 4. Update the descriptor set with these resources.
// 5. Bind the command buffer with the descriptor so that they can be accessed from within the shaders.
b32
CreateTextureAndUniformBufferDescriptor(shoora_vulkan_device *RenderDevice)
{
    VkSampler ImageSampler = VK_NULL_HANDLE;
    shoora_sampler_create_info SamplerCreateInfo = {};
    SamplerCreateInfo.SamplerFilter = VK_FILTER_LINEAR;
    SamplerCreateInfo.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCreateInfo.HasAnisotropy = true;
    SamplerCreateInfo.HasNormalizedCoordinates = true;
    CreateSampler(RenderDevice, SamplerCreateInfo, &ImageSampler);


    // TODO)): Implement the rest of these.
    // NOTE: Create a combined Image Sampler(Image, ImageView, Sampler)
    VkImageType ImageType = VK_IMAGE_TYPE_2D;
    VkFormat ImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageAspectFlags ImageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
    


    VkImage SampledImage = VK_NULL_HANDLE;


    VkImageViewType ImageViewType = VK_IMAGE_VIEW_TYPE_2D;
    VkImageView SampledImageView = VK_NULL_HANDLE;
    // TODO)): Create2DImageForSampling
    // TODO)): Create Corresponding Image View

    VkBuffer UniformBuffer;


}
