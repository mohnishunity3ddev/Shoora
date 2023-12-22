#include "vulkan_geometry.h"

#include "vulkan_buffer.h"
#include "vulkan_image.h"
#include "vulkan_descriptor_sets.h"
#include "vulkan_pipeline.h"
#include <loaders/meshes/mesh.h>


void
CreateImageBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_geometry *Geometry)
{
    Geometry->ImageBuffers = (shoora_vulkan_image_sampler *)malloc(Geometry->Model.TextureCount *
                                                                 sizeof(shoora_vulkan_image_sampler));
    for(u32 Index = 0;
        Index < Geometry->Model.TextureCount;
        ++Index)
    {
        CreateCombinedImageSampler(RenderDevice, &Geometry->Model.Textures[Index].ImageData, VK_SAMPLE_COUNT_1_BIT,
                                   &Geometry->ImageBuffers[Index]);
    }
}

void
UpdateGeometryUniformBuffers(shoora_vulkan_geometry *Geometry, shoora_camera *Camera, const Shu::mat4f &Projection)
{
    shader_data *ShaderData = &Geometry->ShaderData;
    ShaderData->Values.Projection = Projection;
    ShaderData->Values.View = Camera->GetViewMatrix(ShaderData->Values.View);
    ShaderData->Values.ViewPosition = Shu::Vec4f(Camera->Pos.x, Camera->Pos.y, Camera->Pos.z, 1.0f);
    ShaderData->Values.LightPosition = Shu::Vec4f(0.0f, 5.0f, 0.0f, 1.0f);
    memcpy(Geometry->ShaderData.Buffer.pMapped, &Geometry->ShaderData.Values, sizeof(shader_data));
}

void
CreateUniformBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_geometry *Geometry, shoora_camera *Camera, const Shu::mat4f &Projection)
{
    Geometry->ShaderData.Buffer = CreateBuffer(RenderDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VK_SHARING_MODE_EXCLUSIVE,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                             nullptr, sizeof(shader_data));
    UpdateGeometryUniformBuffers(Geometry, Camera, Projection);
}

void
SetupDescriptors(shoora_vulkan_device *RenderDevice, shoora_vulkan_geometry *Geometry)
{
    VkDescriptorPoolSize PoolSizes[2];
    PoolSizes[0] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    // Each material uses color, normal map and a metallic map.
    PoolSizes[1] = GetDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Geometry->Model.MaterialCount*3);
    const u32 MaxSetCount = Geometry->Model.TextureCount + 1;
    CreateDescriptorPool(RenderDevice, ARRAY_SIZE(PoolSizes), PoolSizes, 100, &Geometry->DescriptorPool);

    // Descriptor Set layouts for the scene. set 0 = matrices set 1 = textures
    VkDescriptorSetLayoutBinding SetLayoutBindings[3];
    SetLayoutBindings[0] = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {};
    DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DescriptorSetLayoutCreateInfo.bindingCount = 1;
    DescriptorSetLayoutCreateInfo.pBindings = SetLayoutBindings;
    VK_CHECK(vkCreateDescriptorSetLayout(RenderDevice->LogicalDevice, &DescriptorSetLayoutCreateInfo, nullptr, &Geometry->MatricesSetLayout));
    SetLayoutBindings[0] = GetDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    SetLayoutBindings[1] = GetDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    SetLayoutBindings[2] = GetDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    DescriptorSetLayoutCreateInfo.bindingCount = 3;
    DescriptorSetLayoutCreateInfo.pBindings = SetLayoutBindings;
    VK_CHECK(vkCreateDescriptorSetLayout(RenderDevice->LogicalDevice, &DescriptorSetLayoutCreateInfo, nullptr, &Geometry->TexturesSetLayout));

    // Pipeline layout
    VkDescriptorSetLayout SetLayouts[2] = { Geometry->MatricesSetLayout, Geometry->TexturesSetLayout };
    // Push Constants
    VkPushConstantRange PushConstantRange = {};
    PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    PushConstantRange.offset = 0;
    PushConstantRange.size = sizeof(Shu::mat4f);
    CreatePipelineLayout(RenderDevice, 2, SetLayouts, 1, &PushConstantRange, &Geometry->PipelineLayout);

    // Descritpro Set for matrices.
    shoora_vulkan_buffer *VertBuffer = &Geometry->VertBuffers.VertexBuffer;
    AllocateDescriptorSets(RenderDevice, Geometry->DescriptorPool, 1, &SetLayouts[0], &Geometry->DescriptorSet);
    shoora_vulkan_buffer *UniformBuffer = &Geometry->ShaderData.Buffer;
    UpdateBufferDescriptorSet(RenderDevice, Geometry->DescriptorSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                              UniformBuffer->Handle, UniformBuffer->MemSize);

    // Descriptor Set for materials.
    for(u32 MaterialIndex = 0;
        MaterialIndex < Geometry->Model.MaterialCount;
        ++MaterialIndex)
    {
        shoora_model_material *Mat = Geometry->Model.Materials + MaterialIndex;
        AllocateDescriptorSets(RenderDevice, Geometry->DescriptorPool, 1, &Geometry->TexturesSetLayout,
                               &Mat->DescriptorSet);

        VkWriteDescriptorSet WriteSets[3];
        u32 WriteSetCount = 0;
        if(Mat->BaseColorTextureIndex >= 0)
        {
            shoora_vulkan_image_sampler *ColorMap = Geometry->ImageBuffers + Mat->BaseColorTextureIndex;
            VkDescriptorImageInfo ColorMapInfo = GetImageDescriptorInfo(ColorMap);
            WriteSets[WriteSetCount++] = GetWriteDescriptorSet(Mat->DescriptorSet,
                                                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0,
                                                               &ColorMapInfo);
        }

        if(Mat->NormalTextureIndex >= 0)
        {
            shoora_vulkan_image_sampler *NormalMap = Geometry->ImageBuffers + Mat->NormalTextureIndex;
            VkDescriptorImageInfo NormalMapInfo = GetImageDescriptorInfo(NormalMap);
            WriteSets[WriteSetCount++] = GetWriteDescriptorSet(Mat->DescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                    &NormalMapInfo);
        }

        if(Mat->MetallicTextureIndex >= 0)
        {
            shoora_vulkan_image_sampler *MetallicMap = Geometry->ImageBuffers + Mat->MetallicTextureIndex;
            VkDescriptorImageInfo MetallicMapInfo = GetImageDescriptorInfo(MetallicMap);
            WriteSets[WriteSetCount++] = GetWriteDescriptorSet(Mat->DescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2,
                                                    &MetallicMapInfo);
        }

        vkUpdateDescriptorSets(RenderDevice->LogicalDevice, WriteSetCount, WriteSets, 0, nullptr);
    }
}

struct material_specialization_data
{
    VkBool32 AlphaMask;
    f32 AlphaMaskCutoff;
};

void
SetupPipeline(shoora_vulkan_device *RenderDevice, VkRenderPass RenderPass, shoora_vulkan_geometry *Geometry,
              const char *VertexShaderFile, const char *FragmentShaderFile)
{
    auto InputAssemblyInfo = GetInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    auto RasterizationInfo = GetRasterizationInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
                                                  VK_FRONT_FACE_CLOCKWISE);
    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
    ColorBlendAttachmentState.colorWriteMask = 0xF;
    ColorBlendAttachmentState.blendEnable = VK_FALSE;
    auto ColorBlendInfo = GetPipelineColorBlendInfo(1, &ColorBlendAttachmentState);
    auto DepthStencilInfo = GetPipelineDepthStencilInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
    auto ViewportInfo = GetPipelineViewportInfo(1, 1);
    auto MultiSampleInfo = GetPipelineMultiSampleInfo(RenderDevice->MsaaSamples, 0);
    VkDynamicState DynamicStates[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    auto DynamicStateInfo = GetPipelineDynamicStateInfo(ARRAY_SIZE(DynamicStates), DynamicStates, 0);

    VkPipelineShaderStageCreateInfo ShaderStageCreateInfos[2];
    ShaderStageCreateInfos[0] = GetShaderStageInfo(RenderDevice, VertexShaderFile, VK_SHADER_STAGE_VERTEX_BIT,
                                                   "main");
    ShaderStageCreateInfos[1] = GetShaderStageInfo(RenderDevice, FragmentShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   "main");

    auto VertexBindingDesc = GetVertexBindingDescription();
    VkVertexInputAttributeDescription AttributeDescriptions[] = {
        GetVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT,     OFFSET_OF(shoora_vertex_info, Pos)),
        GetVertexAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT,     OFFSET_OF(shoora_vertex_info, Normal)),
        GetVertexAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT,        OFFSET_OF(shoora_vertex_info, UV)),
        GetVertexAttributeDescription(0, 3, VK_FORMAT_R32G32B32_SFLOAT,     OFFSET_OF(shoora_vertex_info, Color)),
        GetVertexAttributeDescription(0, 4, VK_FORMAT_R32G32B32A32_SFLOAT,  OFFSET_OF(shoora_vertex_info, Tangent))
    };

    auto VertexInputInfo = GetPipelineVertexInputInfo(1, &VertexBindingDesc, ARRAY_SIZE(AttributeDescriptions),
                                                      AttributeDescriptions);

    auto PipelineCreateInfo = GetPipelineCreateInfo(Geometry->PipelineLayout, RenderPass);
    PipelineCreateInfo.pVertexInputState = &VertexInputInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizationInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendInfo;
    PipelineCreateInfo.pMultisampleState = &MultiSampleInfo;
    PipelineCreateInfo.pViewportState = &ViewportInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilInfo;
    PipelineCreateInfo.pDynamicState = &DynamicStateInfo;
    PipelineCreateInfo.stageCount = ARRAY_SIZE(ShaderStageCreateInfos);
    PipelineCreateInfo.pStages = ShaderStageCreateInfos;

    for(u32 MaterialIndex = 0;
        MaterialIndex < Geometry->Model.MaterialCount;
        ++MaterialIndex)
    {
        shoora_model_material *Mat = Geometry->Model.Materials + MaterialIndex;

        material_specialization_data MaterialSpecialData = {};
        MaterialSpecialData.AlphaMask = (Mat->AlphaMode == shoora_model_alpha_mode::AlphaMode_Mask);
        MaterialSpecialData.AlphaMaskCutoff = (Mat->AlphaCutoff);

        VkSpecializationMapEntry SpecialEntries[2] = {
            GetSpecializationMapEntry(0, OFFSET_OF(material_specialization_data, AlphaMask),
                                      sizeof(material_specialization_data::AlphaMask)),
            GetSpecializationMapEntry(1, OFFSET_OF(material_specialization_data, AlphaMaskCutoff),
                                      sizeof(material_specialization_data::AlphaMaskCutoff))
        };

        auto SpecializationInfo = GetSpecializationInfo(ARRAY_SIZE(SpecialEntries), SpecialEntries,
                                                        sizeof(material_specialization_data),
                                                        &MaterialSpecialData);
        ShaderStageCreateInfos[1].pSpecializationInfo = &SpecializationInfo;
        RasterizationInfo.cullMode = Mat->DoubleSided ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
        // RasterizationInfo.cullMode = VK_CULL_MODE_NONE;

        VK_CHECK(vkCreateGraphicsPipelines(RenderDevice->LogicalDevice, VK_NULL_HANDLE, 1, &PipelineCreateInfo,
                                           nullptr, &Mat->Pipeline));

    }
    
    vkDestroyShaderModule(RenderDevice->LogicalDevice, ShaderStageCreateInfos[0].module, nullptr);
    vkDestroyShaderModule(RenderDevice->LogicalDevice, ShaderStageCreateInfos[1].module, nullptr);
}

void
DrawGeometryNode(VkCommandBuffer CommandBuffer, VkPipelineLayout PipelineLayout, shoora_model_node *Node,
               shoora_vulkan_geometry *Geometry)
{
    if(Node->Mesh.PrimitiveCount > 0)
    {
        Shu::mat4f NodeMatrix = Node->ModelMatrix;
        shoora_model_node *ParentNode = Node->ParentNode;

        while(ParentNode)
        {
            NodeMatrix = ParentNode->ModelMatrix*NodeMatrix;
            ParentNode = ParentNode->ParentNode;
        }

        vkCmdPushConstants(CommandBuffer, PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Shu::mat4f),
                           &NodeMatrix);
        for(u32 PrimitiveIndex = 0;
            PrimitiveIndex < Node->Mesh.PrimitiveCount;
            ++PrimitiveIndex)
        {
            shoora_mesh_primitive *Primitive = Node->Mesh.Primitives + PrimitiveIndex;
            if(Primitive->IndexCount > 0)
            {
                shoora_model_material *PrimitiveMaterial = Geometry->Model.Materials + Primitive->MaterialIndex;

                vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PrimitiveMaterial->Pipeline);
                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 1, 1,
                                        &PrimitiveMaterial->DescriptorSet, 0, nullptr);
                vkCmdDrawIndexed(CommandBuffer, Primitive->IndexCount, 1, Primitive->FirstIndex, 0, 0);
            }
        }
    }
    for(u32 ChildIndex = 0;
        ChildIndex < Node->ChildrenCount;
        ++ChildIndex)
    {
        shoora_model_node *ChildNode = Node->ChildNodes[ChildIndex];
        DrawGeometryNode(CommandBuffer, PipelineLayout, ChildNode, Geometry);
    }
}

void
CleanupGeometry(shoora_vulkan_device *RenderDevice, shoora_vulkan_geometry *Geometry)
{
    shoora_model *Model = &Geometry->Model;

    DestroyBuffer(RenderDevice, &Geometry->ShaderData.Buffer);
    DestroyVertexBuffer(RenderDevice, &Geometry->VertBuffers.VertexBuffer, &Geometry->VertBuffers.IndexBuffer);

    for(u32 TexIndex = 0;
        TexIndex < Model->TextureCount;
        ++TexIndex)
    {
        shoora_model_texture *Tex = Model->Textures + TexIndex;
        u8 *ImageData = Tex->ImageData.Data;
        if(ImageData)
        {
            free(ImageData);
            ImageData = nullptr;
        }

        vkDestroySampler(RenderDevice->LogicalDevice, Geometry->ImageBuffers[TexIndex].Sampler, nullptr);
        DestroyImage2D(RenderDevice, &Geometry->ImageBuffers[TexIndex].Image);
    }

    vkDestroyDescriptorSetLayout(RenderDevice->LogicalDevice, Geometry->MatricesSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(RenderDevice->LogicalDevice, Geometry->TexturesSetLayout, nullptr);
    vkDestroyPipelineLayout(RenderDevice->LogicalDevice, Geometry->PipelineLayout, nullptr);
    vkDestroyDescriptorPool(RenderDevice->LogicalDevice, Geometry->DescriptorPool, nullptr);

    for(u32 MatIndex = 0;
        MatIndex < Model->MaterialCount;
        ++MatIndex)
    {
        shoora_model_material *Mat = Model->Materials + MatIndex;

        vkDestroyPipeline(RenderDevice->LogicalDevice, Mat->Pipeline, nullptr);
    }
}

void
SetupGeometry(shoora_vulkan_device *RenderDevice, shoora_vulkan_geometry *Geometry, shoora_camera *Camera,
              const Shu::mat4f &Projection, VkRenderPass RenderPass, const char *MeshFile,
              const char *VertexShaderFile, const char *FragmentShaderFile)
{
    shoora_model *Model = &Geometry->Model;
    LoadModel(Model, MeshFile);
    CreateVertexBuffers(RenderDevice, Model, &Geometry->VertBuffers);
    CreateUniformBuffers(RenderDevice, Geometry, Camera, Projection);
    CreateImageBuffers(RenderDevice, Geometry);
    SetupDescriptors(RenderDevice, Geometry);
    SetupPipeline(RenderDevice, RenderPass, Geometry, VertexShaderFile, FragmentShaderFile);
}

void
DrawGeometry(VkCommandBuffer DrawCmdBuffer, shoora_vulkan_geometry *Geometry)
{
    vkCmdBindDescriptorSets(DrawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Geometry->PipelineLayout, 0, 1,
                            &Geometry->DescriptorSet, 0, nullptr);

    VkDeviceSize Offsets[1] = {0};
    vkCmdBindVertexBuffers(DrawCmdBuffer, 0, 1, &Geometry->VertBuffers.VertexBuffer.Handle, Offsets);
    vkCmdBindIndexBuffer(DrawCmdBuffer, Geometry->VertBuffers.IndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);

    // NOTE: Render all nodes.
    for(u32 NodeIndex = 0;
        NodeIndex < Geometry->Model.NodeCount;
        ++NodeIndex)
    {
        shoora_model_node *Node = Geometry->Model.Nodes[NodeIndex];
        DrawGeometryNode(DrawCmdBuffer, Geometry->PipelineLayout, Node, Geometry);
    }
}
