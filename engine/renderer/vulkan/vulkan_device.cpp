#include <volk/volk.h>

#include "platform/platform.h"
#include "utils/utils.h"
#include "vulkan_device.h"
#include "vulkan_renderer.h"

// TODO)): Remove this!
#include <cstring>

const char *DeviceTypeNames[] =
{
    "Other",
    "Integrated GPU",
    "Discrete GPU",
    "Virtual GPU",
    "CPU",
};

const char *QueueTypeNames[] =
{
    "Graphics",
    "Compute",
    "Transfer",
    "Sparse",
    "Protected"
};

const char *
GetQueueTypeName(shoora_queue_type Type)
{
    const char *TypeName = QueueTypeNames[(u32)Type];
    return TypeName;
}

inline shoora_vulkan_queue *
GetQueueFromType(shoora_vulkan_device *RenderDevice, shoora_queue_type Type)
{
    ASSERT(Type < QueueType_Count);

    shoora_vulkan_queue *Result = nullptr;

    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        if (RenderDevice->QueueFamilies[Index].Type == Type)
        {
            Result = &RenderDevice->QueueFamilies[Index];
            break;
        }
    }

    ASSERT(Result != nullptr);
    return Result;
}

inline u32
GetInternalQueueFamilyFromType(shoora_vulkan_device *RenderDevice, shoora_queue_type Type)
{
    ASSERT(Type < QueueType_Count);

    u32 Result = -1UL;

    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        if (RenderDevice->QueueFamilies[Index].Type == Type)
        {
            Result = Index;
            break;
        }
    }

    ASSERT(Result != -1UL);
    return Result;
}

VkQueue
GetQueueHandle(shoora_vulkan_device *RenderDevice, shoora_queue_type Type)
{
    ASSERT(Type < QueueType_Count);

    VkQueue Result = VK_NULL_HANDLE;
    Result = GetQueueFromType(RenderDevice, Type)->Handle;

    ASSERT(Result != VK_NULL_HANDLE);
    return Result;
}

u32
GetQueueFamilyIndexFromType(shoora_vulkan_device *RenderDevice, shoora_queue_type Type)
{
    ASSERT(Type < QueueType_Count);

    u32 Result = -1UL;

    Result = GetQueueFromType(RenderDevice, Type)->FamilyIndex;

    ASSERT(Result != -1UL);
    return Result;
}

i32
GetDeviceMemoryType(shoora_vulkan_device *RenderDevice, u32 DesiredMemoryTypeBits,
                    VkMemoryPropertyFlags DesiredMemoryProperties)
{
    i32 Result = -1;


    // Iterate over Physical Device Memory Properties to see if the memory requirements for our buffer are
    // satisfied.
    for(u32 MemoryType = 0;
        MemoryType < RenderDevice->MemoryProperties.memoryTypeCount;
        ++MemoryType)
    {
        if((DesiredMemoryTypeBits & (1 << MemoryType)) &&
           ((RenderDevice->MemoryProperties.memoryTypes[MemoryType].propertyFlags & DesiredMemoryProperties) ==
             DesiredMemoryProperties))
        {
            Result = MemoryType;
        }
    }

    ASSERT(Result != -1);

    return Result;
}

#if 0
b32
GetSupportedDepthFormat(shoora_vulkan_device *RenderDevice, VkFormat *pDepthFormat)
{
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    VkFormat FormatList[] =
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto &Format : FormatList)
    {
        VkFormatProperties FormatProps;
        vkGetPhysicalDeviceFormatProperties(RenderDevice->PhysicalDevice, Format, &FormatProps);
        if (FormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *pDepthFormat = Format;
            return true;
        }
    }

    return false;
}
#endif

b32
CheckSurfaceSupport(VkPhysicalDevice PhysicalDevice, u32 QueueFamilyIndex, VkSurfaceKHR Surface)
{
    VkBool32 SurfaceSupported;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, QueueFamilyIndex, Surface, &SurfaceSupported));

    return SurfaceSupported;
}

// TODO)): Read More About Transfer Queues, Sparse, Protected Queues
b32
CheckAvailableQueueFamilies(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR AvailableSurface,
                            shoora_queue_info *InOutRequiredQueueFamilyInfos, const u32 RequiredQueueFamilyCount)
{
    b32 IsSurfaceSupported = false;
    u32 AvailableQueueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &AvailableQueueFamilyCount, 0);
    ASSERT((AvailableQueueFamilyCount > 0) && (AvailableQueueFamilyCount <= 16));

    VkQueueFamilyProperties AvailableQueueFamilies[16];
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &AvailableQueueFamilyCount, AvailableQueueFamilies);

    u32 FoundQueueCount = 0;
    for (u32 RequiredQueueIndex = 0;
         RequiredQueueIndex < RequiredQueueFamilyCount;
         ++RequiredQueueIndex)
    {
        shoora_queue_info *RequiredInfo = InOutRequiredQueueFamilyInfos + RequiredQueueIndex;

        // NOTE: starts from FoundQueueCOunt since we want dedicated Queue Families.
        for(u32 AvlQueueFamilyIndex = 0;
            AvlQueueFamilyIndex < AvailableQueueFamilyCount;
            ++AvlQueueFamilyIndex)
        {
            VkQueueFamilyProperties AvlQueueFamily = AvailableQueueFamilies[AvlQueueFamilyIndex];

            if((AvlQueueFamily.queueCount > 0))
            {
                if (((RequiredInfo->Type == QueueType_Graphics) && (AvlQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) ||
                    ((RequiredInfo->Type == QueueType_Transfer) && (AvlQueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)) ||
                    ((RequiredInfo->Type == QueueType_Compute) && (AvlQueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)))
                {
                    ++FoundQueueCount;
                    IsSurfaceSupported = CheckSurfaceSupport(PhysicalDevice, AvlQueueFamilyIndex,
                                                             AvailableSurface);
                    if(FoundQueueCount >= RequiredQueueFamilyCount && IsSurfaceSupported)
                    {
                        break;
                    }
                }
            }
        }
    }

    b32 Result = ((FoundQueueCount >= RequiredQueueFamilyCount) && IsSurfaceSupported);
    return Result;
}

void
GetQueueFamiliesInfo(shoora_vulkan_device *RenderDevice, VkSurfaceKHR AvailableSurface,
                     shoora_queue_info *InOutRequiredQueueFamilyInfos, const u32 RequiredQueueFamilyCount)
{
    RenderDevice->QueueFamilyCount = RequiredQueueFamilyCount;

    u32 LocalPresentationQueueFamily = -1UL;
    u32 AvailableQueueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(RenderDevice->PhysicalDevice, &AvailableQueueFamilyCount, 0);
    ASSERT((AvailableQueueFamilyCount > 0) && (AvailableQueueFamilyCount <= 16));

    VkQueueFamilyProperties AvailableQueueFamilies[16];
    vkGetPhysicalDeviceQueueFamilyProperties(RenderDevice->PhysicalDevice, &AvailableQueueFamilyCount, AvailableQueueFamilies);

    u32 FoundQueueCount = 0;
    //? Look for Dedicated Queues. Look for a queue which supports presentation and is NOT a graphics queue.
    for (u32 QueueIndex = 0;
         QueueIndex < RequiredQueueFamilyCount;
         ++QueueIndex)
    {
        shoora_queue_info *RequiredInfo = InOutRequiredQueueFamilyInfos + QueueIndex;
        shoora_vulkan_queue *Queue = RenderDevice->QueueFamilies + QueueIndex;

        // NOTE: starts from FoundQueueCOunt since we want dedicated Queue Families.
        for(u32 AvlQueueFamilyIndex = 0;
            AvlQueueFamilyIndex < AvailableQueueFamilyCount;
            ++AvlQueueFamilyIndex)
        {
            VkQueueFamilyProperties AvlQueueFamily = AvailableQueueFamilies[AvlQueueFamilyIndex];

            if((AvlQueueFamily.queueCount > 0))
            {
                if (((RequiredInfo->Type == QueueType_Graphics) && (AvlQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) ||
                    ((RequiredInfo->Type == QueueType_Transfer) &&
                     (AvlQueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                     !(AvlQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                     !(AvlQueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) ||
                    ((RequiredInfo->Type == QueueType_Compute) &&
                     (AvlQueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                     !(AvlQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)))
                {
                    Queue->FamilyIndex = AvlQueueFamilyIndex;
                    if(AvlQueueFamily.queueCount < RequiredInfo->QueueCount)
                    {
                        Queue->Count = AvlQueueFamily.queueCount;
                    }
                    else
                    {
                        Queue->Count = RequiredInfo->QueueCount;
                    }
                    Queue->Type = RequiredInfo->Type;

                    if(RequiredInfo->Type != QueueType_Graphics)
                    {
                        b32 IsSurfaceSupported = CheckSurfaceSupport(RenderDevice->PhysicalDevice,
                                                                     AvlQueueFamilyIndex, AvailableSurface);
                        if (IsSurfaceSupported)
                        {
                            LocalPresentationQueueFamily = AvlQueueFamilyIndex;
                            RenderDevice->IsGraphicsQueueForPresentation = false;
                        }
                    }

                    if(RequiredInfo->Type == QueueType_Graphics)
                    {
                        RenderDevice->GraphicsQueueFamilyInternalIndex = QueueIndex;
                    }
                    else if(RequiredInfo->Type == QueueType_Transfer)
                    {
                        RenderDevice->TransferQueueFamilyInternalIndex = QueueIndex;
                    }
                    else if(RequiredInfo->Type == QueueType_Compute)
                    {
                        RenderDevice->ComputeQueueFamilyInternalIndex = QueueIndex;
                    }

                    ++FoundQueueCount;
                    break;
                }
            }
        }
    }

    //? Could not find dedicated queues.
    if(FoundQueueCount < RequiredQueueFamilyCount || LocalPresentationQueueFamily == -1UL)
    {
        for(u32 QueueIndex = 0;
            QueueIndex < RequiredQueueFamilyCount;
            ++QueueIndex)
        {
            shoora_queue_info *RequiredInfo = InOutRequiredQueueFamilyInfos + QueueIndex;
            shoora_vulkan_queue *Queue = RenderDevice->QueueFamilies + QueueIndex;

            if((Queue->FamilyIndex != -1UL) || (Queue->Count <= 0))
            {
                continue;
            }

            for(u32 AvlQueueFamilyIndex = 0;
                AvlQueueFamilyIndex < AvailableQueueFamilyCount;
                ++AvlQueueFamilyIndex)
            {
                VkQueueFamilyProperties AvlQueueFamily = AvailableQueueFamilies[AvlQueueFamilyIndex];

                if(AvlQueueFamily.queueCount > 0)
                {
                    if (((RequiredInfo->Type == QueueType_Graphics) && (AvlQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) ||
                        ((RequiredInfo->Type == QueueType_Transfer) && (AvlQueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)) ||
                        ((RequiredInfo->Type == QueueType_Compute) && (AvlQueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)))
                    {
                        Queue->FamilyIndex = AvlQueueFamilyIndex;
                        if (AvlQueueFamily.queueCount < RequiredInfo->QueueCount)
                        {
                            Queue->Count = AvlQueueFamily.queueCount;
                        }
                        else
                        {
                            Queue->Count = RequiredInfo->QueueCount;
                        }
                        Queue->Type = RequiredInfo->Type;

                        if(LocalPresentationQueueFamily == -1UL)
                        {
                            b32 IsSurfaceSupported = CheckSurfaceSupport(RenderDevice->PhysicalDevice,
                                                                         AvlQueueFamilyIndex, AvailableSurface);
                            if (IsSurfaceSupported)
                            {
                                LocalPresentationQueueFamily = AvlQueueFamilyIndex;
                                RenderDevice->IsGraphicsQueueForPresentation = true;
                            }
                        }

                        if (RequiredInfo->Type == QueueType_Graphics)
                        {
                            RenderDevice->GraphicsQueueFamilyInternalIndex = QueueIndex;
                        }
                        else if (RequiredInfo->Type == QueueType_Transfer)
                        {
                            RenderDevice->TransferQueueFamilyInternalIndex = QueueIndex;
                        }
                        else if (RequiredInfo->Type == QueueType_Compute)
                        {
                            RenderDevice->ComputeQueueFamilyInternalIndex = QueueIndex;
                        }

                        ++FoundQueueCount;
                        break;
                    }
                }
            }
        }
    }

    ASSERT(LocalPresentationQueueFamily != -1UL);

    RenderDevice->PresentationQueueIndex = LocalPresentationQueueFamily;
}

b32
CheckPhysicalDeviceExtensionsAvailability(VkPhysicalDevice PhysicalDevice, const char **DesiredDeviceExtensions,
                                          u32 DesiredDeviceExtensionCount)
{
    u32 AvailableDeviceExtensionCount = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevice, 0, &AvailableDeviceExtensionCount, 0));

    ASSERT((AvailableDeviceExtensionCount > 0) &&
           (AvailableDeviceExtensionCount <= 256));

    VkExtensionProperties AvailableDeviceExtensions[256];
    VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevice, 0, &AvailableDeviceExtensionCount,
                                                  AvailableDeviceExtensions));

    // TODO)):
    u32 FoundCount = 0;
    for(u32 ExtensionIndex = 0;
        ExtensionIndex < DesiredDeviceExtensionCount;
        ++ExtensionIndex)
    {
        const char *DesiredExtensionName = *(DesiredDeviceExtensions + ExtensionIndex);

        for(u32 AvailableExtIndex = 0;
            AvailableExtIndex < AvailableDeviceExtensionCount;
            ++AvailableExtIndex)
        {
            VkExtensionProperties AvailableExtension = AvailableDeviceExtensions[AvailableExtIndex];
            // TODO)): Implement strcmp functionality here!
            if(!strcmp(AvailableExtension.extensionName, DesiredExtensionName))
            {
                ++FoundCount;
                break;
            }
        }
    }

    b32 Result = (FoundCount == DesiredDeviceExtensionCount);
    return Result;
}

b32
CheckForDesiredFeatures(VkPhysicalDevice PhysicalDevice, const VkPhysicalDeviceFeatures *pDesiredFeatures)
{
    VkPhysicalDeviceFeatures AvailableDeviceFeatures = {};
    vkGetPhysicalDeviceFeatures(PhysicalDevice, &AvailableDeviceFeatures);

    // TODO)): This Method of checking features Looks sus!
    VkBool32 *DesiredFeaturePtr = (VkBool32 *)pDesiredFeatures;
    VkBool32 *AvailableFeaturePtr = (VkBool32 *)&AvailableDeviceFeatures;
    u32 MaxFeatureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    for(u32 FeatureIndex = 0;
        FeatureIndex < MaxFeatureCount;
        ++FeatureIndex)
    {
        VkBool32 DesiredFeature = *DesiredFeaturePtr++;
        if(DesiredFeature && (DesiredFeature != *AvailableFeaturePtr++))
        {
            return false;
        }
    }

    return true;
}

inline b32
CheckForDedicatedGPU(const VkPhysicalDeviceProperties *DeviceProperties)
{
    b32 Result = DeviceProperties->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    return Result;
}

b32
CheckForBasicSwapchainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
    u32 SupportedFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SupportedFormatCount, nullptr);

    u32  SupportedPresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &SupportedPresentModeCount, nullptr);

    b32 Result = (SupportedFormatCount > 0) && (SupportedPresentModeCount > 0);

    return Result;
}

VkPhysicalDevice
PickPhysicalDevice(VkInstance Instance, shoora_device_create_info *DeviceCreateInfo, VkSurfaceKHR Surface,
                   u32 *PresentationFamily)
{
    const char **DesiredDeviceExtensions = DeviceCreateInfo->ppRequiredExtensions;
    const u32 DesiredDeviceExtensionCount = DeviceCreateInfo->RequiredExtensionCount;
    const VkPhysicalDeviceFeatures *DesiredFeatures = DeviceCreateInfo->DesiredFeatures;
    shoora_queue_info *DesiredQueueFamilyInfos = DeviceCreateInfo->pQueueCreateInfos;
    const u32 RequiredQueueFamilyCount  = DeviceCreateInfo->QueueCreateInfoCount;

    // NOTE: It is highly unlikely that a system has more than 64 GPUs.
    u32 AvailablePhysicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(Instance, &AvailablePhysicalDeviceCount, 0));

    ASSERT(AvailablePhysicalDeviceCount > 0 && AvailablePhysicalDeviceCount <= 64);

    VkPhysicalDevice AvailablePhysicalDevices[64];
    VK_CHECK(vkEnumeratePhysicalDevices(Instance, &AvailablePhysicalDeviceCount, AvailablePhysicalDevices));

    u32 PhysicalDeviceScores[64] = {};

    LogOutput(LogType_Info, "Available GPU Devices:\n");

    VkPhysicalDevice SelectedDevice = 0;
    u32 MaxScore = 0;
    u32 MaxScoreIndex = 0;
    for(u32 PhysicalDeviceIndex = 0;
        PhysicalDeviceIndex < AvailablePhysicalDeviceCount;
        ++PhysicalDeviceIndex)
    {
        VkPhysicalDevice PhysicalDevice = AvailablePhysicalDevices[PhysicalDeviceIndex];

        VkPhysicalDeviceProperties Properties;
        vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);

        if(CheckPhysicalDeviceExtensionsAvailability(PhysicalDevice, DesiredDeviceExtensions,
                                                     DesiredDeviceExtensionCount))
        {
            ++PhysicalDeviceScores[PhysicalDeviceIndex];
        }

        if(CheckForDesiredFeatures(PhysicalDevice, DesiredFeatures))
        {
            ++PhysicalDeviceScores[PhysicalDeviceIndex];
        }

        if(CheckAvailableQueueFamilies(PhysicalDevice, Surface, DesiredQueueFamilyInfos, RequiredQueueFamilyCount))
        {
            ++PhysicalDeviceScores[PhysicalDeviceIndex];
        }

        if(CheckForDedicatedGPU(&Properties))
        {
            PhysicalDeviceScores[PhysicalDeviceIndex] += 2;
        }

        if(CheckForBasicSwapchainSupport(PhysicalDevice, Surface))
        {
            ++PhysicalDeviceScores[PhysicalDeviceIndex];
        }

        if(PhysicalDeviceScores[PhysicalDeviceIndex] > MaxScore)
        {
            MaxScore = PhysicalDeviceScores[PhysicalDeviceIndex];
            MaxScoreIndex = PhysicalDeviceIndex;
        }

        LogOutput(LogType_Info, "Device %d: %s, Device Type: %s, Score: %d\n", PhysicalDeviceIndex,
                  Properties.deviceName, DeviceTypeNames[(u32)Properties.deviceType],
                  PhysicalDeviceScores[PhysicalDeviceIndex]);
    }

    SelectedDevice = AvailablePhysicalDevices[MaxScoreIndex];

    VkPhysicalDeviceProperties Properties;
    vkGetPhysicalDeviceProperties(SelectedDevice, &Properties);

    LogOutput(LogType_Info, "Selected GPU Properties:\n");
    LogOutput(LogType_Info, "Device Name: %s\n", Properties.deviceName);
    LogOutput(LogType_Info, "Device Score: %d\n", PhysicalDeviceScores[MaxScoreIndex]);
    LogOutput(LogType_Info, "Device Type: %s\n", DeviceTypeNames[(u32)Properties.deviceType]);

    return SelectedDevice;
}

void
FillRequiredDeviceQueueInfos(shoora_vulkan_device *RenderDevice, VkDeviceQueueCreateInfo *OutQueueCreateInfos)
{
    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        shoora_vulkan_queue *Queue = RenderDevice->QueueFamilies + Index;
        // u32 DeviceQueueIndex = GetQueueIndexFromType(Info->Type);

        Queue->Handle = VK_NULL_HANDLE;

        VkDeviceQueueCreateInfo *QueueCreateInfo = OutQueueCreateInfos + Index;
        QueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo->queueFamilyIndex = Queue->FamilyIndex;
        QueueCreateInfo->queueCount = Queue->Count;
        QueueCreateInfo->pQueuePriorities = Queue->Priorities;
    }
}

void
AcquireRequiredDeviceQueueHandles(shoora_vulkan_device *RenderDevice)
{
    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        shoora_vulkan_queue *DeviceQueue = RenderDevice->QueueFamilies + Index;
        vkGetDeviceQueue(RenderDevice->LogicalDevice, DeviceQueue->FamilyIndex, 0, &DeviceQueue->Handle);
        ASSERT(DeviceQueue->Handle != VK_NULL_HANDLE);
    }

    RenderDevice->GraphicsQueue = RenderDevice->QueueFamilies[RenderDevice->GraphicsQueueFamilyInternalIndex].Handle;
    RenderDevice->TransferQueue = RenderDevice->QueueFamilies[RenderDevice->TransferQueueFamilyInternalIndex].Handle;
    RenderDevice->ComputeQueue = RenderDevice->QueueFamilies[RenderDevice->ComputeQueueFamilyInternalIndex].Handle;
}

void
CreateCommandPools(shoora_vulkan_device *RenderDevice)
{
    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        shoora_vulkan_command_pool *ShCmdPool = RenderDevice->CommandPools + Index;
        shoora_vulkan_command_pool *ShTransientCmdPool = RenderDevice->TransientCommandPools + Index;

        shoora_vulkan_queue *QueueFamily = RenderDevice->QueueFamilies + Index;
        ShCmdPool->Type = QueueFamily->Type;
        ShTransientCmdPool->Type = QueueFamily->Type;

        u32 QueueFamilyIndex = QueueFamily->FamilyIndex;
        ASSERT(QueueFamilyIndex >= 0);

        VkCommandPoolCreateInfo CommandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        CommandPoolCreateInfo.pNext = nullptr;
        CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        CommandPoolCreateInfo.queueFamilyIndex = QueueFamilyIndex;

        VK_CHECK(vkCreateCommandPool(RenderDevice->LogicalDevice, &CommandPoolCreateInfo, 0, &ShCmdPool->Handle));
        ShCmdPool->IsTransient = false;

        CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        VK_CHECK(vkCreateCommandPool(RenderDevice->LogicalDevice, &CommandPoolCreateInfo, 0, &ShTransientCmdPool->Handle));
        ShTransientCmdPool->IsTransient = true;

        if(QueueFamily->Type == QueueType_Graphics)
        {
            RenderDevice->GraphicsCommandPool = ShCmdPool->Handle;
            RenderDevice->GraphicsCommandPoolTransient = ShTransientCmdPool->Handle;
        }
        else if(QueueFamily->Type == QueueType_Transfer)
        {
            RenderDevice->TransferCommandPool = ShCmdPool->Handle;
            RenderDevice->TransferCommandPoolTransient = ShTransientCmdPool->Handle;
        }

        LogOutput(LogType_Info, "Created Reset and Transient Command Pool for Queue(%s)!\n",
                  GetQueueTypeName(QueueFamily->Type));
    }
}

VkCommandPool
GetCommandPoolByQueueType(shoora_vulkan_device *RenderDevice, shoora_queue_type QueueType, b32 IsTransient)
{
    VkCommandPool Result = VK_NULL_HANDLE;
    shoora_vulkan_command_pool *pCmdPools = IsTransient ? RenderDevice->TransientCommandPools
                                                        : RenderDevice->CommandPools;

    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        shoora_vulkan_command_pool Pool = pCmdPools[Index];
        if(Pool.Type == QueueType)
        {
            ASSERT(Pool.IsTransient == IsTransient);
            Result = Pool.Handle;
        }
    }

    ASSERT(Result != VK_NULL_HANDLE);

    return Result;
}

// Resets all the command buffers allocated from this pool!
void
ResetAllCommandPools(shoora_vulkan_device *RenderDevice, b32 ReleaseResources)
{
    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        VkCommandPoolResetFlags ResetFlags = ReleaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0;
        VK_CHECK(vkResetCommandPool(RenderDevice->LogicalDevice, RenderDevice->CommandPools[Index].Handle,
                                    ResetFlags));
        VK_CHECK(vkResetCommandPool(RenderDevice->LogicalDevice, RenderDevice->TransientCommandPools[Index].Handle,
                                    ResetFlags));
    }

    LogOutput(LogType_Info, "All Command Pools are Reset!\n");
}

VkSampleCountFlagBits
GetMaxUsableSampleCount(const VkPhysicalDeviceProperties *GPUProperties)
{
    VkSampleCountFlagBits Result = {};
    VkSampleCountFlags SampleCountLimit = GPUProperties->limits.framebufferColorSampleCounts &
                                          GPUProperties->limits.framebufferDepthSampleCounts;

    if(SampleCountLimit & VK_SAMPLE_COUNT_64_BIT)
    {
        Result = VK_SAMPLE_COUNT_64_BIT;
    }
    else if(SampleCountLimit & VK_SAMPLE_COUNT_32_BIT)
    {
        Result = VK_SAMPLE_COUNT_32_BIT;
    }
    else if(SampleCountLimit & VK_SAMPLE_COUNT_16_BIT)
    {
        Result = VK_SAMPLE_COUNT_16_BIT;
    }
    else if(SampleCountLimit & VK_SAMPLE_COUNT_8_BIT)
    {
        Result = VK_SAMPLE_COUNT_8_BIT;
    }
    else if(SampleCountLimit & VK_SAMPLE_COUNT_4_BIT)
    {
        Result = VK_SAMPLE_COUNT_4_BIT;
    }
    else if(SampleCountLimit & VK_SAMPLE_COUNT_2_BIT)
    {
        Result = VK_SAMPLE_COUNT_2_BIT;
    }

    return Result;
}

void
CreateDeviceAndQueues(shoora_vulkan_context *Context, shoora_device_create_info *ShuraDeviceCreateInfo)
{
    VkPhysicalDevice PhysicalDevice = PickPhysicalDevice(Context->Instance, ShuraDeviceCreateInfo,
                                                         Context->Swapchain.Surface,
                                                         &Context->Device.PresentationQueueIndex);
    Context->Device.PhysicalDevice = PhysicalDevice;
    GetQueueFamiliesInfo(&Context->Device, Context->Swapchain.Surface, ShuraDeviceCreateInfo->pQueueCreateInfos,
                         ShuraDeviceCreateInfo->QueueCreateInfoCount);

    vkGetPhysicalDeviceProperties(PhysicalDevice, &Context->Device.DeviceProperties);
    vkGetPhysicalDeviceFeatures(PhysicalDevice, &Context->Device.Features);
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &Context->Device.MemoryProperties);

#if SHU_VK_ENABLE_MSAA
    Context->Device.MsaaSamples = GetMaxUsableSampleCount(&Context->Device.DeviceProperties);
#else
    Context->Device.MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
#endif

    VkDeviceQueueCreateInfo QueueCreateInfos[32] = {};
    FillRequiredDeviceQueueInfos(&Context->Device, &QueueCreateInfos[0]);

    VkDeviceCreateInfo DeviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    DeviceCreateInfo.pNext = 0;
    DeviceCreateInfo.flags = 0;
    DeviceCreateInfo.queueCreateInfoCount = Context->Device.QueueFamilyCount;
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos;
    DeviceCreateInfo.enabledLayerCount = 0;
    DeviceCreateInfo.ppEnabledLayerNames = 0;
    DeviceCreateInfo.enabledExtensionCount = ShuraDeviceCreateInfo->RequiredExtensionCount;
    DeviceCreateInfo.ppEnabledExtensionNames = ShuraDeviceCreateInfo->ppRequiredExtensions;
    DeviceCreateInfo.pEnabledFeatures = ShuraDeviceCreateInfo->DesiredFeatures;

    VK_CHECK(vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, 0, &Context->Device.LogicalDevice));

    // Get Vulkan Requested Queues
    AcquireRequiredDeviceQueueHandles(&Context->Device);

    LogOutput(LogType_Info, "Created Vulkan Logical Device, Got the Device Queues And Command Pool Created!\n");
}

void
DestroyCommandPools(shoora_vulkan_device *RenderDevice)
{
    for(u32 Index = 0;
        Index < RenderDevice->QueueFamilyCount;
        ++Index)
    {
        VkCommandPool CommandPool = RenderDevice->CommandPools[Index].Handle;
        if (CommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(RenderDevice->LogicalDevice, CommandPool, 0);
        }

        VkCommandPool TransientCommandPool = RenderDevice->TransientCommandPools[Index].Handle;
        if (TransientCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(RenderDevice->LogicalDevice, TransientCommandPool, 0);
        }
    }

    LogOutput(LogType_Info, "%d Command Pools Destroyed!\n", RenderDevice->QueueFamilyCount);
}

void
DestroyLogicalDevice(shoora_vulkan_device *RenderDevice)
{
    DestroyCommandPools(RenderDevice);

    vkDestroyDevice(RenderDevice->LogicalDevice, 0);

    LogOutput(LogType_Info, "Destroyed Vulkan Logical Device!\n");
}
