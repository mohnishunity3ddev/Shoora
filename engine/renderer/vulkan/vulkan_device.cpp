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

// TODO)): Read More About Transfer Queues, Sparse, Protected Queues
b32
CheckAvailableQueueFamilies(VkPhysicalDevice PhysicalDevice, shura_queue_info *InOutRequiredQueueFamilyInfos,
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
        shura_queue_info *RequiredInfo = InOutRequiredQueueFamilyInfos + RequiredQueueIndex;

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
PickPhysicalDevice(VkInstance Instance, shura_device_create_info *DeviceCreateInfo)
{
    const char **DesiredDeviceExtensions = DeviceCreateInfo->ppRequiredExtensions;
    const u32 DesiredDeviceExtensionCount = DeviceCreateInfo->RequiredExtensionCount;
    const VkPhysicalDeviceFeatures *DesiredFeatures = DeviceCreateInfo->DesiredFeatures;
    shura_queue_info *DesiredQueueFamilyInfos = DeviceCreateInfo->pQueueCreateInfos;
    const u32 RequiredQueueFamilyCount  = DeviceCreateInfo->QueueCreateInfoCount;

    // NOTE: It is highly unlikely that a system has more than 64 GPUs.
    u32 AvailablePhysicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(Instance, &AvailablePhysicalDeviceCount, 0));

    ASSERT(AvailablePhysicalDeviceCount > 0 && AvailablePhysicalDeviceCount <= 64);

    VkPhysicalDevice AvailablePhysicalDevices[64];
    VK_CHECK(vkEnumeratePhysicalDevices(Instance, &AvailablePhysicalDeviceCount, AvailablePhysicalDevices));

    u32 PhysicalDeviceScores[64] = {};

    LogOutput("Available GPU Devices:\n");

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

        LogOutput("Device %d: %s, Device Type: %s, Score: %d\n", PhysicalDeviceIndex, Properties.deviceName,
                  DeviceTypeNames[(u32)Properties.deviceType], PhysicalDeviceScores[PhysicalDeviceIndex]);
    }

    SelectedDevice = AvailablePhysicalDevices[MaxScoreIndex];

    VkPhysicalDeviceProperties Properties;
    vkGetPhysicalDeviceProperties(SelectedDevice, &Properties);

    LogOutput("Selected GPU Properties:\n");
    LogOutput("Device Name: %s\n", Properties.deviceName);
    LogOutput("Device Score: %d\n", PhysicalDeviceScores[MaxScoreIndex]);
    LogOutput("Device Type: %s\n", DeviceTypeNames[(u32)Properties.deviceType]);

    return SelectedDevice;
}

void
GetRequiredDeviceQueueInfos(VkDeviceQueueCreateInfo *OutQueueCreateInfos, const shura_queue_info *DesiredQueueInfos,
                        const u32 DesiredQueueInfoCount)
{
    for(u32 Index = 0;
        Index < DesiredQueueInfoCount;
        ++Index)
    {
        VkDeviceQueueCreateInfo *QueueCreateInfo = OutQueueCreateInfos + Index;
        const shura_queue_info *DesiredQueueInfo = DesiredQueueInfos + Index;

        QueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo->pNext = 0;
        QueueCreateInfo->flags = 0;
        QueueCreateInfo->queueFamilyIndex = DesiredQueueInfo->FamilyIndex;
        QueueCreateInfo->queueCount = DesiredQueueInfo->QueueCount;
        QueueCreateInfo->pQueuePriorities = DesiredQueueInfo->Priorities;
    }
}

void
GetRequiredDeviceQueues(shura_vulkan_context *Context,
                        const shura_queue_info *RequiredQueueInfos,
                        const u32 RequiredQueueInfoCount)
{
    for(u32 QueueInfoIndex = 0;
        QueueInfoIndex < RequiredQueueInfoCount;
        ++QueueInfoIndex)
    {
        const shura_queue_info *QueueInfo = RequiredQueueInfos + QueueInfoIndex;

        VkQueue *QueueHandleDestination = 0;
        switch(QueueInfo->Type)
        {
            case QueueType_Graphics: { QueueHandleDestination = &Context->GraphicsQueue; } break;
            case QueueType_Compute:  { QueueHandleDestination = &Context->ComputeQueue;  } break;
            case QueueType_Transfer: { QueueHandleDestination = &Context->TransferQueue; } break;
            default: { ASSERT(!"Queue Type Not Supported right now!") } break;
        }

        vkGetDeviceQueue(Context->LogicalDevice, QueueInfo->FamilyIndex, 0, QueueHandleDestination);
    }
}

void
CreateLogicalDeviceAndGetQueues(shura_vulkan_context *VulkanContext,
                                shura_device_create_info *ShuraDeviceCreateInfo)
{
    VkPhysicalDevice PhysicalDevice = PickPhysicalDevice(VulkanContext->Instance, ShuraDeviceCreateInfo);

    VkDeviceQueueCreateInfo QueueCreateInfos[32];
    GetRequiredDeviceQueueInfos(QueueCreateInfos, ShuraDeviceCreateInfo->pQueueCreateInfos,
                                ShuraDeviceCreateInfo->QueueCreateInfoCount);

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

    VK_CHECK(vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, 0, &VulkanContext->LogicalDevice));

    // Get Vulkan Requested Queues
    GetRequiredDeviceQueues(VulkanContext, ShuraDeviceCreateInfo->pQueueCreateInfos,
                            ShuraDeviceCreateInfo->QueueCreateInfoCount);

    LogOutput("Created Vulkan Logical Device And Got the Device Queues.\n");
}

void
DestroyLogicalDevice(shura_vulkan_context *VulkanContext)
{
    vkDestroyDevice(VulkanContext->LogicalDevice, 0);
    LogOutput("Destroyed Vulkan Logical Device!\n");
}
