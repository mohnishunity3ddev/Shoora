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

VkQueue
GetQueueHandle(shoora_vulkan_device *RenderDevice, shoora_queue_type QueueType)
{
    ASSERT(QueueType < QueueType_Count);

    VkQueue Result = RenderDevice->Queues[QueueType].Handle;
    ASSERT(Result != VK_NULL_HANDLE);
    return Result;
}

// TODO)): Read More About Transfer Queues, Sparse, Protected Queues
b32
CheckAvailableQueueFamilies(VkPhysicalDevice PhysicalDevice, shoora_queue_info *InOutRequiredQueueFamilyInfos,
                            const u32 RequiredQueueFamilyCount)
{
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
        for(u32 AvlQueueFamilyIndex = FoundQueueCount;
            AvlQueueFamilyIndex < AvailableQueueFamilyCount;
            ++AvlQueueFamilyIndex)
        {
            VkQueueFamilyProperties AvlQueueFamily = AvailableQueueFamilies[AvlQueueFamilyIndex];

            if((AvlQueueFamily.queueCount > 0))
            {
                if (((RequiredInfo->Type == QueueType_Graphics) &&
                     (AvlQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) ||
                    ((RequiredInfo->Type == QueueType_Compute) &&
                     (AvlQueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) ||
                    ((RequiredInfo->Type == QueueType_Transfer) &&
                     (AvlQueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                     !(AvlQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)))
                {
                    RequiredInfo->FamilyIndex = AvlQueueFamilyIndex;
                    if(AvlQueueFamily.queueCount < RequiredInfo->QueueCount)
                    {
                        RequiredInfo->QueueCount = AvlQueueFamily.queueCount;
                    }
                    ++FoundQueueCount;
                    break;
                }
            }
        }
    }

    b32 Result = (FoundQueueCount == RequiredQueueFamilyCount);
    return Result;
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

VkPhysicalDevice
PickPhysicalDevice(VkInstance Instance, shoora_device_create_info *DeviceCreateInfo)
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

        if(CheckAvailableQueueFamilies(PhysicalDevice, DesiredQueueFamilyInfos, RequiredQueueFamilyCount))
        {
            ++PhysicalDeviceScores[PhysicalDeviceIndex];
        }

        if(CheckForDedicatedGPU(&Properties))
        {
            PhysicalDeviceScores[PhysicalDeviceIndex] += 2;
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

u32
GetQueueIndexFromType(shoora_queue_type Type)
{
    u32 Result = (u32)Type;
    ASSERT(Result >= 0 && Result < 8);
    return Result;
}

inline shoora_vulkan_queue *
GetQueueFromType(shoora_vulkan_device *RenderDevice, shoora_queue_type Type)
{
    u32 QueueIndex = GetQueueIndexFromType(Type);

    shoora_vulkan_queue *Queue = RenderDevice->Queues + QueueIndex;

    ASSERT(Queue);
    return Queue;
}

void
FillRequiredDeviceQueueInfos(shoora_vulkan_device *RenderDevice, shoora_queue_info *QueueInfos, u32 QueueInfoCount,
                             VkDeviceQueueCreateInfo *OutQueueCreateInfos)
{
    RenderDevice->QueueTypeCount = QueueInfoCount;

    for(u32 Index = 0;
        Index < RenderDevice->QueueTypeCount;
        ++Index)
    {
        shoora_queue_info *Info = QueueInfos + Index;
        u32 DeviceQueueIndex = GetQueueIndexFromType(Info->Type);

        shoora_vulkan_queue *Queue = &RenderDevice->Queues[DeviceQueueIndex];
        Queue->Count = Info->QueueCount;
        Queue->FamilyIndex = Info->FamilyIndex;
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
        Index < RenderDevice->QueueTypeCount;
        ++Index)
    {
        shoora_vulkan_queue DeviceQueue = RenderDevice->Queues[Index];
        vkGetDeviceQueue(RenderDevice->LogicalDevice, DeviceQueue.FamilyIndex, 0, &DeviceQueue.Handle);
    }
}

void
CreateCommandPools(shoora_vulkan_device *RenderDevice, shoora_command_pool_create_info *CommandPoolInfos,
                   u32 CommandPoolCount)
{
    for(u32 Index = 0;
        Index < CommandPoolCount;
        ++Index)
    {
        shoora_command_pool_create_info *CreateInfo = CommandPoolInfos + Index;

        u32 QueueFamilyIndex = -1UL;
        VkCommandPool *CommandPool = 0;

        u32 QueueIndex = GetQueueIndexFromType(CreateInfo->QueueType);
        CommandPool = RenderDevice->CommandPools + Index;

        QueueFamilyIndex = RenderDevice->Queues[QueueIndex].FamilyIndex;
        ASSERT(QueueFamilyIndex >= 0);

        VkCommandPoolCreateInfo CommandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        CommandPoolCreateInfo.pNext = nullptr;
        CommandPoolCreateInfo.flags = CreateInfo->CreateFlags;
        CommandPoolCreateInfo.queueFamilyIndex = QueueFamilyIndex;

        VK_CHECK(vkCreateCommandPool(RenderDevice->LogicalDevice, &CommandPoolCreateInfo, 0, CommandPool));

        LogOutput(LogType_Info, "Created Command Pool for Queue(%s)!\n", GetQueueTypeName(CreateInfo->QueueType));
    }
}

void
ResetCommandPool(shoora_vulkan_device *RenderDevice, u32 InternalIndex, b32 ReleaseResources)
{
    VkCommandPool CommandPool = RenderDevice->CommandPools[InternalIndex];
    VkCommandPoolResetFlags ResetFlags = ReleaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0;

    VK_CHECK(vkResetCommandPool(RenderDevice->LogicalDevice, CommandPool, ResetFlags));
    LogOutput(LogType_Info, "Command Pool associated with Queue(%s) has been reset!\n",
              GetQueueTypeName((shoora_queue_type)(InternalIndex)));
}

// Resets all the command buffers allocated from this pool!
void
ResetAllCommandPools(shoora_vulkan_device *RenderDevice, b32 ReleaseResources)
{
    for (u32 Index = 0; Index < RenderDevice->QueueTypeCount; ++Index)
    {
        ResetCommandPool(RenderDevice, Index, ReleaseResources);
    }

    LogOutput(LogType_Info, "All Command Pools are Reset!\n");
}

void
CreateDeviceNQueuesNCommandPools(shoora_vulkan_context *Context, shoora_device_create_info *ShuraDeviceCreateInfo)
{
    VkPhysicalDevice PhysicalDevice = PickPhysicalDevice(Context->Instance, ShuraDeviceCreateInfo);
    Context->Device.PhysicalDevice = PhysicalDevice;
    vkGetPhysicalDeviceProperties(PhysicalDevice, &Context->Device.DeviceProperties);
    vkGetPhysicalDeviceFeatures(PhysicalDevice, &Context->Device.DeviceFeatures);

    VkDeviceQueueCreateInfo QueueCreateInfos[32] = {};
    FillRequiredDeviceQueueInfos(&Context->Device, ShuraDeviceCreateInfo->pQueueCreateInfos,
                                 ShuraDeviceCreateInfo->QueueCreateInfoCount, QueueCreateInfos);

    VkDeviceCreateInfo DeviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    DeviceCreateInfo.pNext = 0;
    DeviceCreateInfo.flags = 0;
    DeviceCreateInfo.queueCreateInfoCount = ShuraDeviceCreateInfo->QueueCreateInfoCount;
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos;
    DeviceCreateInfo.enabledLayerCount = 0;
    DeviceCreateInfo.ppEnabledLayerNames = 0;
    DeviceCreateInfo.enabledExtensionCount = ShuraDeviceCreateInfo->RequiredExtensionCount;
    DeviceCreateInfo.ppEnabledExtensionNames = ShuraDeviceCreateInfo->ppRequiredExtensions;
    DeviceCreateInfo.pEnabledFeatures = ShuraDeviceCreateInfo->DesiredFeatures;

    VK_CHECK(vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, 0, &Context->Device.LogicalDevice));

    // Get Vulkan Requested Queues
    AcquireRequiredDeviceQueueHandles(&Context->Device);

    CreateCommandPools(&Context->Device, ShuraDeviceCreateInfo->pCommandPoolCreateInfos,
                       ShuraDeviceCreateInfo->CommandPoolCount);

    LogOutput(LogType_Info, "Created Vulkan Logical Device, Got the Device Queues And Command Pool Created!\n");
}

void DestroyCommandPools(shoora_vulkan_device *RenderDevice)
{
    for(u32 Index = 0;
        Index < RenderDevice->QueueTypeCount;
        ++Index)
    {
        VkCommandPool CommandPool = RenderDevice->CommandPools[Index];

        if (CommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(RenderDevice->LogicalDevice, CommandPool, 0);
        }
    }

    LogOutput(LogType_Info, "%d Command Pools Destroyed!\n", RenderDevice->QueueTypeCount);
}

void
DestroyLogicalDevice(shoora_vulkan_device *RenderDevice)
{
    DestroyCommandPools(RenderDevice);

    vkDestroyDevice(RenderDevice->LogicalDevice, 0);

    LogOutput(LogType_Info, "Destroyed Vulkan Logical Device!\n");
}
