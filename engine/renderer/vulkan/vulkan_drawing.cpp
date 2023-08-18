#include "vulkan_drawing.h"

#if 0
void
ClearColorImage(VkCommandBuffer CmdBuffer)
{
    // TODO)): Make sure command buffer is recording. No RenderPass has started.

    // Image to be cleared
    VkImage Image;
    VkImageLayout ImageLayout;

    // List of mipmaps that should be cleared.
    VkImageSubresourceRange ImageSubresourceRange;
    ImageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageSubresourceRange.baseMipLevel = 0;
    ImageSubresourceRange.levelCount = 1;
    ImageSubresourceRange.baseArrayLayer = 0;
    ImageSubresourceRange.layerCount = 1;

    VkClearColorValue ClearColor;
    ClearColor.float32[0] = 1.0f;
    ClearColor.float32[1] = 1.0f;
    ClearColor.float32[2] = 1.0f;
    ClearColor.float32[3] = 1.0f;

    vkCmdClearColorImage(CmdBuffer, Image, ImageLayout, &ClearColor, 1, &ImageSubresourceRange);
}

void
ClearDepthStencilImage(VkCommandBuffer CmdBuffer)
{
    // TODO)): Make sure command buffer is recording. No RenderPass has started.

    VkImage DepthImage;
    VkImageLayout DepthImageLayout;

    // List of mipmaps that should be cleared.
    VkImageSubresourceRange ImageSubresourceRange;
    ImageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageSubresourceRange.baseMipLevel = 0;
    ImageSubresourceRange.levelCount = 1;
    ImageSubresourceRange.baseArrayLayer = 0;
    ImageSubresourceRange.layerCount = 1;

    VkClearDepthStencilValue ClearValue;
    ClearValue.depth = 0.0f;
    ClearValue.stencil = 0;

    vkCmdClearDepthStencilImage(CmdBuffer, DepthImage, DepthImageLayout, &ClearValue, 1, &ImageSubresourceRange);
}

void
ClearRenderPassAttachments(VkCommandBuffer CmdBuffer)
{
    // TODO)): Make sure you ARE INSIDE a renderpass.

    VkClearAttachment ClearAttachment;
    ClearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ClearAttachment.colorAttachment = 0;
    ClearAttachment.clearValue.color.float32[0] = 0.0f;
    ClearAttachment.clearValue.color.float32[1] = 0.0f;
    ClearAttachment.clearValue.color.float32[2] = 0.0f;
    ClearAttachment.clearValue.color.float32[3] = 0.0f;

    VkClearRect ClearRect;
    ClearRect.rect = {.offset = {}, .extent = {1920, 1080}};
    ClearRect.baseArrayLayer = 0;
    ClearRect.layerCount = 1;

    vkCmdClearAttachments(CmdBuffer, 1, &ClearAttachment, 1, &ClearRect);
}

void
BindVertexBuffers()
{

}

#endif