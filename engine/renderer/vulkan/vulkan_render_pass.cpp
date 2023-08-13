#include "vulkan_render_pass.h"
#include <memory>

// ---------------------------------------------------------------------------------------------------------------
//? Attachment Descriptions -                                                                                    |
//  Renderpasses represent a set of resources(images) called attachments which are used                          |
//  during rendering operations. These are divided into color, depth/stencil, input or resolve attachments.      |
//  Indices into the descriptions array are used for subpasses descriptions.
//  When we create framebuffers and specify exactly which image resource should be used for each attachment, we  |
//  define a list where each element corresponds to the element of the attachment descriptions array.
// ---------------------------------------------------------------------------------------------------------------
void
SpecifyAttachmentDescriptions()
{
    VkAttachmentDescription AttachmentDescriptions[8];
    // -----------------------------------------------------------------------------------------------------------
    //? COLOR ATTACHMENT EXAMPLE                                                                                 |
    // -----------------------------------------------------------------------------------------------------------
    VkAttachmentDescription *pColorAttachment = &AttachmentDescriptions[0];
    pColorAttachment->flags = 0;
    pColorAttachment->format = VK_FORMAT_R8G8B8_UNORM;
    pColorAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
    // -----------------------------------------------------------------------------------------------------------
    //? LoadOp:                                                                                                  |
    //  The type of operations that should be performed on the attachment's contents when the renderpass         |
    //  starts. CLEAR clears the contents. Loads preserves contents. Dont_Care means we will overwrite the       |
    //  contents, so don't care for its current contents. good for color attachments or depth/stencil attachment |
    // -----------------------------------------------------------------------------------------------------------
    pColorAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // -----------------------------------------------------------------------------------------------------------
    //? StoreOp:                                                                                                 |
    //  The type of operations that should happen to attachments after the renderpass has completed.             |
    //  STORE:  Contents should be preseved. Good for swapchain images which have to be presented.               |
    //  DONT_CARE: Contents are not needed after the rendering. Good for color, depth/stencil attachments        |
    // -----------------------------------------------------------------------------------------------------------
    pColorAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // -----------------------------------------------------------------------------------------------------------
    //? StencilLoadOp                                                                                            |
    //  Same as LoadOp, but for the stencil aspect of the depth/stencil attachment.                              |
    // -----------------------------------------------------------------------------------------------------------
    pColorAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pColorAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // -----------------------------------------------------------------------------------------------------------
    //? InitalLayout:                                                                                            |
    //  Layout of the attachment when the renderpass begins                                                      |
    // -----------------------------------------------------------------------------------------------------------
    pColorAttachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // -----------------------------------------------------------------------------------------------------------
    //? FinalLayout:                                                                                             |
    //  Layout of the attachment when the renderpass finishes. Defines how to attachment is going to be used.    |
    //  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR - Attachment has to be presented to the presentation image.              |
    // -----------------------------------------------------------------------------------------------------------
    pColorAttachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // -----------------------------------------------------------------------------------------------------------
    //? DEPTH ATTACHMENT EXAMPLE                                                                                 |
    // -----------------------------------------------------------------------------------------------------------
    VkAttachmentDescription *pDepthStencilAttachment = &AttachmentDescriptions[1];
    pDepthStencilAttachment->flags = 0;
    pDepthStencilAttachment->format = VK_FORMAT_D16_UNORM;
    pDepthStencilAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
    pDepthStencilAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    pDepthStencilAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    pDepthStencilAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pDepthStencilAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pDepthStencilAttachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pDepthStencilAttachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

    u32 AttachmentCount = 2;
    VkRenderPassCreateInfo RenderPassCreateInfo;
}

// -----------------------------------------------------------------------------------------------------------
//? SUBPASSES                                                                                                |
//  Subpass is a stage or phase of our rendering commands in which a subset of renderpass's attachments are  |
//  used(into which we render or from which we read data).                                                   |
//  A Render Pass requires atleast one subpass which is automatically exeucuted when the renderpass is       |
//  begun. Each subpass requires a description.                                                              |
// -----------------------------------------------------------------------------------------------------------
void
SpecifySubpassDescriptions()
{

    VkSubpassDescription SubpassDescriptions[8];

    VkAttachmentReference ColorAttachmentReference = {};
    // Index in the attachment descriptions array which we created when we created the renderpass object.
    ColorAttachmentReference.attachment = 0;
    // The layout of the attachment which will automatically be transitioned to at the beginning of the subpass.
    // Since we want to render into this color attachment, that's why mentioned this flag.
    ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthStencilAttachmentReference = {};
    // In the list of attachment descriptions defined in render pass creation, index 1 is the depth_stencil
    // attachment. That's why we specify 1 here.
    DepthStencilAttachmentReference.attachment = 1;
    // Render into this attachment.
    DepthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription *SubpassDescription = &SubpassDescriptions[0];
    SubpassDescription->flags = 0;
    // -----------------------------------------------------------------------------------------------------------
    //? PipelineType:                                                                                            |
    //  The type of Pipeline that will be used during the subpass. Currently only Graphics Pipeline is supported |
    //  inside a subpass.                                                                                        |
    // -----------------------------------------------------------------------------------------------------------
    SubpassDescription->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // -----------------------------------------------------------------------------------------------------------
    //? Input Attachments                                                                                        |
    //  Collection of attachments from which we will read data during the subpass.                               |
    // -----------------------------------------------------------------------------------------------------------
    SubpassDescription->inputAttachmentCount = 0;
    SubpassDescription->pInputAttachments = nullptr;
    // -----------------------------------------------------------------------------------------------------------
    //? Color Attachments                                                                                        |
    //  Collection of attachments into which we will render into during the subpass.                             |
    // -----------------------------------------------------------------------------------------------------------
    SubpassDescription->colorAttachmentCount = 1;
    SubpassDescription->pColorAttachments = &ColorAttachmentReference;
    // -----------------------------------------------------------------------------------------------------------
    //? Resolve Attachments                                                                                      |
    //  Resolve attachments specifies which color attachments need to be resolved from multi-sampled to          |
    //  single-sampled image at the end of the subpass.                                                          |
    // -----------------------------------------------------------------------------------------------------------
    SubpassDescription->pResolveAttachments = 0;
    SubpassDescription->pDepthStencilAttachment = &DepthStencilAttachmentReference;
    // -----------------------------------------------------------------------------------------------------------
    //? Preserve Attachments                                                                                     |
    //  Preserve attachments means that these attachments are not used in the subpass but they must be preserved |
    //  during the execution of the subpass.                                                                     |
    // -----------------------------------------------------------------------------------------------------------
    SubpassDescription->preserveAttachmentCount = 0;
    SubpassDescription->pPreserveAttachments = nullptr;
}

// --------------------------------------------------------------------------------------------------------------
//? SUBPASS DEPENDENCIES                                                                                        |
//  When operations in a subpass depend on the result of operations happening on a previous subpass, we need    |
//  to specify subpass dependencies. We also need them when there are operations recorded within a renderpass   |
//  and operations recorded before it. OR there are dependencies between operations recorded AFTER a render-    |
//  pass and those recorded within the renderpass. These are like setting up memory barriers                    |
//  These are also required when we want to set up image memory barriers inside a render pass. In which case,   |
//  we have to define a self dependency where srcSubpass and dstSubpass have the same index. Having done        |
//  this, now we can define a image memory barrier                                                              |
// --------------------------------------------------------------------------------------------------------------
void
SpecifySubpassDependencies()
{
    // --------------------------------------------------------------------------------------------------------------
    //? DEPENDENCY EXAMPLE: First Subpass draws geometry into color and depth attachment, the second subpass uses   |
    //? color data for post-processing.                                                                             |
    // --------------------------------------------------------------------------------------------------------------
    VkSubpassDependency SubpassDependencies[8];

    VkSubpassDependency *FirstDependency = &SubpassDependencies[0];
    // --------------------------------------------------------------------------------------------------------------
    //? srcSubpass, dstSubpass                                                                                      |
    //  The index of the subpass inside the renderpass which should be finished before the subpass specified in the |
    //  dstSubpass should be there in srcSubpass. If the operations are external to the RENDERPASS then             |
    //  VK_SUBPASS_EXTERNAL is specified. if src is external then dstSubpass depends on completion of operations    |
    //  happening before the start of the renderpass. If dstSubpass has VK_SUBPASS_EXTERNAL it means this           |
    //  srcSubpass should be completed before operations happening after the render pass.                           |
    // --------------------------------------------------------------------------------------------------------------
    FirstDependency->srcSubpass = 0;
    FirstDependency->dstSubpass = 1;
    // --------------------------------------------------------------------------------------------------------------
    //? srcStageMask / dstStageMask                                                                                 |
    //  srcStageMask is the stage which produces the results which the consuming commands in the dstStageMask       |
    //  depends on.                                                                                                 |
    //  dstStageMask is the stage is where the results produced by the srcStageMask is "consumed"                   |
    // --------------------------------------------------------------------------------------------------------------
    FirstDependency->srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    FirstDependency->dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    // --------------------------------------------------------------------------------------------------------------
    //? srcAccessMask / dstAccessMask
    //  srcAccessMask: The type of memory operations happening in the srcStageMask by the "producing" commands in
    //  the srcSubpass
    //  dstAccessMask: Type of memory operations happening in the dstStage by the "consuming" commands in the
    //  dstSubpass
    // --------------------------------------------------------------------------------------------------------------
    FirstDependency->srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    FirstDependency->dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    // --------------------------------------------------------------------------------------------------------------
    //? Dependency Flags
    //  if VK_DEPENDENCY_BY_REGION is specified then it means that for a specific memory region, the producing      |
    //  commands writing data to the region should complete before the consuming commands access that data from the |
    //  same region. if nothing is specified, then the memory is global and whatever image data the consuming       |
    //  commands will access should be finished before the consuming commands can be started.                       |
    //? VK_DEPENDENCY_BY_REGION_BIT Example Usage
    //  If we are doing post processing, the consuming stage will only need access to a fragment, the same fragment |
    //  which was written by the producing commands. The region here is basically a single pixel, because the size  |
    //  of a region may be different on various hardware platforms.                                                 |
    // --------------------------------------------------------------------------------------------------------------
    FirstDependency->dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
}

// ------------------------------------------------------------------------------------------------------------------
//? CREATING A RENDERPASS                                                                                           |
//  Rendering(Drawing geometry) can only happen inside a renderpass. when we want to do multiple things, like       |
//  rendering shadow maps, draw scene, post processing then we need to separate these into multiple subpasses       |
//  inside the renderpass So, here we need to mention all the attachments that the renderpass is going to use, all  |
//  the subpass descriptions and all the subpass dependencies should be mentioned when creating the render pass.    |
// ------------------------------------------------------------------------------------------------------------------
void
CreateRenderPass(shoora_vulkan_device *RenderDevice)
{
    VkAttachmentDescription AttachmentDescriptions[8];
    u32 AttachmentCount = 2;
    VkSubpassDescription SubpassDescriptions[8];
    u32 SubpassCount = 2;
    VkSubpassDependency SubpassDependencies[2];
    u32 SubpassDependencyCount = 2;

    VkRenderPassCreateInfo RenderPassCreateInfo = {};
    RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInfo.pNext = nullptr;
    RenderPassCreateInfo.flags = 0;
    RenderPassCreateInfo.attachmentCount = AttachmentCount;
    RenderPassCreateInfo.pAttachments = AttachmentDescriptions;
    RenderPassCreateInfo.subpassCount = SubpassCount;
    RenderPassCreateInfo.pSubpasses = SubpassDescriptions;
    RenderPassCreateInfo.dependencyCount = SubpassDependencyCount;
    RenderPassCreateInfo.pDependencies = SubpassDependencies;

    VkRenderPass RenderPassHandle;
    VK_CHECK(vkCreateRenderPass(RenderDevice->LogicalDevice, &RenderPassCreateInfo, nullptr, &RenderPassHandle));
    ASSERT(RenderPassHandle != VK_NULL_HANDLE);
    // --------------------------------------------------------------------------------------------------------------
    // IMPORTANT: NOTE:                                                                                             |
    // Now that we have created the renderpass, it is still just metadata. It explains which attachments are going  |
    // to be used inside subpasses, what their synchornization/dependencies are. Renderpasses still does not know   |
    // where to look for the actual image resources/memory. That is done using framebuffers.                        |
    // --------------------------------------------------------------------------------------------------------------
}

// ---------------------------------------------------------------------------------------------------------------------
//? FRAMEBUFFERS                                                                                                       |
//  Framebuffers are used alongside renderpasses.                                                                      |
//  They specify what image resources are used for the attachments mentioned in the attachment descriptions.           |
//  They also specify the size of the renderable area. That's why when we need to record drawing operations, we not    |
//  only need to create a renderpass but also a framebuffer that completes it.                                         |
// ---------------------------------------------------------------------------------------------------------------------
void
CreateFramebuffer(shoora_vulkan_device *RenderDevice)
{
    VkRenderPass RenderPass;

    // ImageView handles that represent the image subresources which should be used for the renderpass attachments.
    VkImageView ImageViews[8];
    u32 ImageViewCount = 3;
    u32 RenderableWidth;
    u32 RenderableHeight;
    u32 FramebufferLayerCount;

    VkFramebufferCreateInfo FramebufferCreateInfo = {};
    FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferCreateInfo.pNext = nullptr;
    FramebufferCreateInfo.flags = 0;
    FramebufferCreateInfo.renderPass = RenderPass;
    FramebufferCreateInfo.attachmentCount = ImageViewCount;
    FramebufferCreateInfo.pAttachments = ImageViews;
    FramebufferCreateInfo.width = RenderableWidth;
    FramebufferCreateInfo.height = RenderableHeight;
    FramebufferCreateInfo.layers = FramebufferLayerCount;

    VkFramebuffer Framebuffer;
    VK_CHECK(vkCreateFramebuffer(RenderDevice->LogicalDevice, &FramebufferCreateInfo, nullptr, &Framebuffer));

    // --------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------
    // IMPORTANT: NOTE:                                                                                             |
    // --------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------
    //  It's not necessary that The Created Framebuffer can only be used with the current RenderPass. It can be     |
    //  used by other renderpasses which are compatible with the current renderpass that was bound to this          |
    //  framebuffer during it's creation.                                                                           |
    // *What are Compatible Renderpasses which can use the same framebuffer created here?                           |
    //  They have the same number of subpasses.                                                                     |
    //  Each subpass should have compatible set of color, input, resolve and preserve attachments.                  |
    //  The Format and the number of samples used in the attachments should be the same.                            |
    //  It is possible for the attachments here to have differnet initial and final layouts and differnet load and  |
    //  store operations and the renderpasses will still be compatible!                                             |
    //                                                                                                              |
    // *Framebuffers also define the size of the renderable area - the dimensions inside of which all the rendering |
    // *will be Confined.                                                                                           |
    //  But it is up to us that fragments/pixels outside of this renderable area range are not modified. For this,  |
    //  we have to set up Viewport and Scissor test during pipeline creation!                                       |
    //  We also need to make sure that all image subresources we attached here to the framebuffer are not modified  |
    //  or accessed externally whenever we are in the middle of a renderpass after it has begun and before it ends. |
    // --------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------
}


// ---------------------------------------------------------------------------------------------------------------------
//? EXAMPLE: RenderPass for Geometric Rendering(Draw Scene) and PostProcess Subpasses                                  |
// ---------------------------------------------------------------------------------------------------------------------
void
CreateRenderPassForGeometricRenderingAndPostProcess(shoora_vulkan_device *RenderDevice)
{
    VkAttachmentDescription Attachments[3];
    u32 AttachmentCount = 3;

    VkAttachmentDescription *FirstColorAttachmentDescription = &Attachments[0];
    FirstColorAttachmentDescription->flags = 0;
    FirstColorAttachmentDescription->format = VK_FORMAT_R8G8B8A8_UNORM;
    FirstColorAttachmentDescription->samples = VK_SAMPLE_COUNT_1_BIT;
    FirstColorAttachmentDescription->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    FirstColorAttachmentDescription->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    FirstColorAttachmentDescription->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    FirstColorAttachmentDescription->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    FirstColorAttachmentDescription->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    FirstColorAttachmentDescription->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription *DepthStencilAttachmentDescription = &Attachments[1];
    DepthStencilAttachmentDescription->flags = 0;
    DepthStencilAttachmentDescription->format = VK_FORMAT_D16_UNORM;
    DepthStencilAttachmentDescription->samples = VK_SAMPLE_COUNT_1_BIT;
    DepthStencilAttachmentDescription->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthStencilAttachmentDescription->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthStencilAttachmentDescription->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    DepthStencilAttachmentDescription->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthStencilAttachmentDescription->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthStencilAttachmentDescription->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription *SecondColorAttachmentDescription = &Attachments[2];
    SecondColorAttachmentDescription->flags = 0;
    SecondColorAttachmentDescription->format = VK_FORMAT_R8G8B8A8_UNORM;
    SecondColorAttachmentDescription->samples = VK_SAMPLE_COUNT_1_BIT;
    SecondColorAttachmentDescription->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    SecondColorAttachmentDescription->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    SecondColorAttachmentDescription->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    SecondColorAttachmentDescription->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    SecondColorAttachmentDescription->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    SecondColorAttachmentDescription->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    VkSubpassDescription Subpasses[2];
    VkAttachmentReference DepthStencilAttachment;
    DepthStencilAttachment.attachment = 1;
    DepthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference FirstColorAttachment;
    FirstColorAttachment.attachment = 0;
    FirstColorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    Subpasses[0].flags = 0;
    Subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpasses[0].inputAttachmentCount = 0;
    Subpasses[0].pInputAttachments = nullptr;
    Subpasses[0].colorAttachmentCount = 1;
    Subpasses[0].pColorAttachments = &FirstColorAttachment;
    Subpasses[0].pResolveAttachments = 0;
    Subpasses[0].pDepthStencilAttachment = &DepthStencilAttachment;
    Subpasses[0].preserveAttachmentCount = 0;
    Subpasses[0].pPreserveAttachments = nullptr;

    VkAttachmentReference InputAttachment;
    InputAttachment.attachment = 0;
    InputAttachment.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentReference SecondColorAttachment;
    SecondColorAttachment.attachment = 2;
    SecondColorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    Subpasses[1].flags = 0;
    Subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpasses[1].inputAttachmentCount = 1;
    Subpasses[1].pInputAttachments = &InputAttachment;
    Subpasses[1].colorAttachmentCount = 1;
    Subpasses[1].pColorAttachments = &SecondColorAttachment;
    Subpasses[1].pResolveAttachments = 0;
    Subpasses[1].pDepthStencilAttachment = nullptr;
    Subpasses[1].preserveAttachmentCount = 0;
    Subpasses[1].pPreserveAttachments = nullptr;

    VkSubpassDependency SubpassDependency;
    SubpassDependency.srcSubpass = 0;
    SubpassDependency.dstSubpass = 1;
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    SubpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    SubpassDependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    SubpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo RenderPassCreateInfo = {};
    RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInfo.pNext = nullptr;
    RenderPassCreateInfo.flags = 0;
    RenderPassCreateInfo.attachmentCount = 3;
    RenderPassCreateInfo.pAttachments = Attachments;
    RenderPassCreateInfo.subpassCount = 2;
    RenderPassCreateInfo.pSubpasses = Subpasses;
    RenderPassCreateInfo.dependencyCount = 1;
    RenderPassCreateInfo.pDependencies = &SubpassDependency;

    VkRenderPass RenderPass;
    VK_CHECK(vkCreateRenderPass(RenderDevice->LogicalDevice, &RenderPassCreateInfo, nullptr, &RenderPass));
    ASSERT(RenderPass != VK_NULL_HANDLE);
}

// ----------------------------------------------------------------------------------------------------------------------
//? EXAMPLE: RenderPass for Geometric Rendering(Draw Scene) and PostProcess Subpasses                                   |
//  Rendering a 3D Scene involves getting a color and depth attachment                                                  |
// ----------------------------------------------------------------------------------------------------------------------
void
CreateImageForFramebuffer(shoora_vulkan_device *RenderDevice, VkCommandBuffer CmdBuffer, VkImage *Image, VkDeviceMemory *Memory,
                          VkFormat Format, VkImageUsageFlags UsageFlags, VkImageAspectFlags AspectFlags, VkImageView *ImageView,
                          u32 ImageWidth, u32 ImageHeight, u32 BytesPerPixel)
{
    VkImage ImageHandle = VK_NULL_HANDLE;

    VkImageCreateInfo ImageCreateInfo = {};
    ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInfo.pNext = nullptr;
    ImageCreateInfo.flags = 0;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format = Format;
    ImageCreateInfo.extent = {.width = ImageWidth, .height = ImageHeight, .depth = 1};
    ImageCreateInfo.mipLevels = 0;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.usage = UsageFlags;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.queueFamilyIndexCount = 0;
    ImageCreateInfo.pQueueFamilyIndices = nullptr;
    ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkCreateImage(RenderDevice->LogicalDevice, &ImageCreateInfo, nullptr, &ImageHandle);
    ASSERT(ImageHandle != VK_NULL_HANDLE);

    VkImageSubresourceRange SubresourceRange;
    SubresourceRange.aspectMask = AspectFlags;
    SubresourceRange.baseMipLevel = 0;
    SubresourceRange.levelCount = 1;
    SubresourceRange.baseArrayLayer = 0;
    SubresourceRange.layerCount = 1;
    VkImageMemoryBarrier ImageMemBarrier = {};
    ImageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImageMemBarrier.pNext = nullptr;
    ImageMemBarrier.srcAccessMask = 0;
    ImageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    ImageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ImageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemBarrier.image = ImageHandle;
    ImageMemBarrier.subresourceRange = SubresourceRange;

    VkMemoryRequirements ImageMemRequirements;
    vkGetImageMemoryRequirements(RenderDevice->LogicalDevice, ImageHandle, &ImageMemRequirements);
    VkMemoryAllocateInfo ImageAllocInfo = {};
    ImageAllocInfo.pNext = nullptr;
    ImageAllocInfo.allocationSize = ImageMemRequirements.size;
    ImageAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, ImageMemRequirements.memoryTypeBits,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &ImageAllocInfo, nullptr, Memory));
    VK_CHECK(vkBindImageMemory(RenderDevice->LogicalDevice, ImageHandle, *Memory, 0));

    // Get Staging Buffer, Load image data there, Get Image requirements, allocate device memory, Copy Image data
    // from buffer to device memory. Create ImageView.
    VkBuffer StagingBuffer;
    VkDeviceMemory StagingMemory;
    void *pMapped = 0;
    VkDeviceSize ImageSize = ImageWidth * ImageHeight * BytesPerPixel;
    void *pImageData;

    VkBufferCreateInfo BufferCreateInfo = {};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.pNext = nullptr;
    BufferCreateInfo.flags = 0;
    BufferCreateInfo.size = ImageSize;
    BufferCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    BufferCreateInfo.queueFamilyIndexCount = 0;
    BufferCreateInfo.pQueueFamilyIndices = nullptr;

    VK_CHECK(vkCreateBuffer(RenderDevice->LogicalDevice, &BufferCreateInfo, nullptr, &StagingBuffer));
    ASSERT(StagingBuffer != VK_NULL_HANDLE);

    VkMemoryRequirements BufferMemoryReqs;
    vkGetBufferMemoryRequirements(RenderDevice->LogicalDevice, StagingBuffer, &BufferMemoryReqs);

    VkMemoryAllocateInfo BufferAllocInfo = {};
    BufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    BufferAllocInfo.pNext = nullptr;
    BufferAllocInfo.allocationSize = BufferMemoryReqs.size;
    BufferAllocInfo.memoryTypeIndex = GetDeviceMemoryType(RenderDevice, BufferMemoryReqs.memoryTypeBits,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(RenderDevice->LogicalDevice, &BufferAllocInfo, nullptr, &StagingMemory));
    VK_CHECK(vkBindBufferMemory(RenderDevice->LogicalDevice, StagingBuffer, StagingMemory, 0));
    VK_CHECK(vkMapMemory(RenderDevice->LogicalDevice, StagingMemory, 0, ImageSize, 0, &pMapped));
    ASSERT(pMapped != nullptr);
    memcpy(pMapped, pImageData, ImageSize);
    // If the host visible memory is not host coherent. We need to flush the memory so that the results would be
    // visible to the GPU instantly after writing.
    b32 NeedsFlushing = true;
    if (NeedsFlushing)
    {
        VkMappedMemoryRange MemMappedRange = {};
        MemMappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        MemMappedRange.pNext = nullptr;
        MemMappedRange.memory = StagingMemory;
        MemMappedRange.offset = 0;
        MemMappedRange.size = BufferMemoryReqs.size;
        VK_CHECK(vkFlushMappedMemoryRanges(RenderDevice->LogicalDevice, 1, &MemMappedRange));
    }
    vkUnmapMemory(RenderDevice->LogicalDevice, StagingMemory);

    VkBufferImageCopy CopyRegion = {};
    CopyRegion.bufferOffset = 0;
    CopyRegion.bufferRowLength = 0;
    CopyRegion.bufferImageHeight = 0;
    VkImageSubresourceLayers ImageSubresource = {};
    ImageSubresource.aspectMask = AspectFlags;
    ImageSubresource.mipLevel = 0;
    ImageSubresource.baseArrayLayer = 0;
    ImageSubresource.layerCount = 1;
    CopyRegion.imageSubresource = ImageSubresource;
    CopyRegion.imageOffset = {};
    CopyRegion.imageExtent = {.width = ImageWidth, .height = ImageHeight, .depth = 1};

    vkCmdPipelineBarrier(CmdBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &ImageMemBarrier);
    vkCmdCopyBufferToImage(CmdBuffer, StagingBuffer, ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &CopyRegion);
    ImageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    ImageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    ImageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ImageMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier(CmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &ImageMemBarrier);

    vkFreeMemory(RenderDevice->LogicalDevice, StagingMemory, nullptr);
    vkDestroyBuffer(RenderDevice->LogicalDevice, StagingBuffer, nullptr);

    VkImageViewCreateInfo ImageViewCreateInfo = {};
    ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewCreateInfo.pNext = nullptr;
    ImageViewCreateInfo.flags = 0;
    ImageViewCreateInfo.image = ImageHandle;
    ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ImageViewCreateInfo.format = Format;
    ImageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                      VK_COMPONENT_SWIZZLE_A};
    VkImageSubresourceRange SubRange = {};
    SubRange.aspectMask = AspectFlags;
    SubRange.baseMipLevel = 0;
    SubRange.levelCount = 1;
    SubRange.baseArrayLayer = 0;
    SubRange.layerCount = 1;
    ImageViewCreateInfo.subresourceRange = SubRange;

    VK_CHECK(vkCreateImageView(RenderDevice->LogicalDevice, &ImageViewCreateInfo, nullptr, ImageView));
    *Image = ImageHandle;
}

void
CreateRenderPassAndFramebufferWithColorAndDepthAttachments(shoora_vulkan_device *RenderDevice, VkCommandBuffer CmdBuffer)
{
    VkImage Image;
    VkImageView ImageView;
    VkDeviceMemory ImageMemory;
    VkImageUsageFlags UsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                   VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageForFramebuffer(RenderDevice, CmdBuffer, &Image, &ImageMemory, VK_FORMAT_R8G8B8A8_UNORM, UsageFlags,
                              VK_IMAGE_ASPECT_COLOR_BIT, &ImageView, 1920, 1080, 4);

    VkImage DepthImage;
    VkImageView DepthImageView;
    VkDeviceMemory DepthImageMemory;
    VkImageUsageFlags DepthUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageForFramebuffer(RenderDevice, CmdBuffer, &DepthImage, &DepthImageMemory, VK_FORMAT_D16_UNORM, UsageFlags,
                              VK_IMAGE_ASPECT_DEPTH_BIT, &DepthImageView, 1920, 1080, 4);

    VkAttachmentDescription Attachments[2];
    Attachments[0].flags = 0;
    Attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    Attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    Attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    Attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    Attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    Attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    Attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Attachments[1].flags = 0;
    Attachments[1].format = VK_FORMAT_D16_UNORM;
    Attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    Attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    Attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    Attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    Attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    Attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference DepthStencilAttachment;
    DepthStencilAttachment.attachment = 1;
    DepthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference ColorAttachment;
    ColorAttachment.attachment = 1;
    ColorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkSubpassDescription SubpassDescription;
    SubpassDescription.flags = 0;
    SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDescription.inputAttachmentCount = 0;
    SubpassDescription.pInputAttachments = nullptr;
    SubpassDescription.colorAttachmentCount = 1;
    SubpassDescription.pColorAttachments = &ColorAttachment;
    SubpassDescription.pResolveAttachments = nullptr;
    SubpassDescription.pDepthStencilAttachment = &DepthStencilAttachment;
    SubpassDescription.preserveAttachmentCount = 0;
    SubpassDescription.pPreserveAttachments = nullptr;

    //! This is to make sure that the images are rendered into completely within the renderpass.
    //! This can be confusing. Like VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT comes AFTER
    //! VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT is started then why are they are in the wrong src/dst order?
    //! These stages mentioned here are not from the same subpass. What this is saying is:
    //! Make sure COLOR_ATTACHMENT_OUTPUT is completed in subpass 0(meaning the attachments are rendered to)
    //! before anyone else external to this renderpass SHADER_READs these attachments in their FRAGMENT_SHADER_BIT
    VkSubpassDependency SubpassDependency;
    SubpassDependency.srcSubpass = 0;
    SubpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    SubpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    SubpassDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    SubpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo RenderPassCreateInfo;
    RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInfo.pNext = nullptr;
    RenderPassCreateInfo.flags = 0;
    RenderPassCreateInfo.attachmentCount = 2;
    RenderPassCreateInfo.pAttachments = Attachments;
    RenderPassCreateInfo.subpassCount = 1;
    RenderPassCreateInfo.pSubpasses = &SubpassDescription;
    RenderPassCreateInfo.dependencyCount = 1;
    RenderPassCreateInfo.pDependencies = &SubpassDependency;

    VkRenderPass RenderPass;
    VK_CHECK(vkCreateRenderPass(RenderDevice->LogicalDevice, &RenderPassCreateInfo, nullptr, &RenderPass));

    VkImageView FramebufferViews[2] = {ImageView, DepthImageView};
    VkFramebufferCreateInfo FramebufferCreateInfo;
    FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferCreateInfo.pNext = nullptr;
    FramebufferCreateInfo.flags = 0;
    FramebufferCreateInfo.renderPass = RenderPass;
    FramebufferCreateInfo.attachmentCount = 2;
    FramebufferCreateInfo.pAttachments = FramebufferViews;
    FramebufferCreateInfo.width = 1920;
    FramebufferCreateInfo.height = 1080;
    FramebufferCreateInfo.layers = 1;

    VkFramebuffer Framebuffer;
    VK_CHECK(vkCreateFramebuffer(RenderDevice->LogicalDevice, &FramebufferCreateInfo, nullptr, &Framebuffer));


}

//? Begin a render pass automatically starts its first subpass.
void
BeginRenderPass(shoora_vulkan_device *RenderDevice)
{
    VkCommandBuffer CmdBuffer;
    VkRenderPass RenderPass;
    VkFramebuffer Framebuffer;

    VkExtent2D RenderArea = {.width = 1920, .height = 1080};
    VkRect2D FramebufferRenderArea = {.extent = RenderArea};
    u32 AttachmentCount = 2;

    VkClearValue ClearColors[2] = {};

    // No Secondary command buffers are used.
    VkSubpassContents SubpassContents = VK_SUBPASS_CONTENTS_INLINE;

    VkRenderPassBeginInfo RenderPassBeginInfo = {};
    RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBeginInfo.pNext = nullptr;
    RenderPassBeginInfo.renderPass = RenderPass;
    RenderPassBeginInfo.framebuffer = Framebuffer;
    RenderPassBeginInfo.renderArea = FramebufferRenderArea;
    RenderPassBeginInfo.clearValueCount = 2;
    RenderPassBeginInfo.pClearValues = ClearColors;

    vkCmdBeginRenderPass(CmdBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void
ProceedToNextSubpass()
{
    VkCommandBuffer CmdBuffer;
    ASSERT(!"Make sure Render Pass has started!");
    vkCmdNextSubpass(CmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void
EndRenderPass()
{
    VkCommandBuffer CmdBuffer;
    vkCmdEndRenderPass(CmdBuffer);
}

void
DestroyFramebuffer(shoora_vulkan_device *RenderDevice, VkFramebuffer Framebuffer)
{
    vkDestroyFramebuffer(RenderDevice->LogicalDevice, Framebuffer, nullptr);
}

void
DestroyRenderPass(shoora_vulkan_device *RenderDevice, VkRenderPass RenderPass)
{
    vkDestroyRenderPass(RenderDevice->LogicalDevice, RenderPass, nullptr);
}
