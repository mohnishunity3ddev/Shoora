#define VOLK_IMPLEMENTATION
#include <volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <cstdio>

#define ARRAY_SIZE(Arr) sizeof(Arr) / sizeof(Arr[0])
#define ASSERT(Expression)                                                                                        \
    if (!(Expression))                                                                                            \
    {                                                                                                             \
        *((volatile int *)0) = 0;                                                                                 \
    }
#define VK_CHECK(Call)                                                                                            \
    {                                                                                                             \
        VkResult Res = Call;                                                                                      \
        ASSERT(Res == VK_SUCCESS);                                                                                \
    }

typedef struct SwapChain
{
    VkSwapchainKHR _swapchain = 0;
    uint32_t swapchainImageCount = 0;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;
} SwapChain;

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                  VkDebugUtilsMessageTypeFlagsEXT message_types,
                  const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    switch (message_severity)
    {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        std::cout << "Validation Layer [ERROR]: " << callback_data->pMessage << "\n";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        std::cout << "Validation Layer [WARNING]: " << callback_data->pMessage << "\n";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        break;
    }
    return VK_FALSE;
}

VkInstance
createInstance()
{
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pApplicationName = "Niagara";
    app_info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    app_info.pEngineName = "Niagara";
    app_info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);

    VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instance_create_info.pApplicationInfo = &app_info;

#ifdef _DEBUG
    const char *debugLayers[] = {"VK_LAYER_KHRONOS_validation"};
    instance_create_info.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
    instance_create_info.ppEnabledLayerNames = debugLayers;
#endif
    
    const char *extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME,
                                "VK_KHR_win32_surface",
                                VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    instance_create_info.enabledExtensionCount = ARRAY_SIZE(extensions);
    instance_create_info.ppEnabledExtensionNames = extensions;

    VkInstance instance = 0;
    VK_CHECK(vkCreateInstance(&instance_create_info, 0, &instance));
    return instance;
}

void
setupDebugUtils(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger)
{
#if defined(_DEBUG)
    uint32_t log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    debug_create_info.pfnUserCallback = vk_debug_callback;
    debug_create_info.pUserData = 0;

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                                                                                    instance, "vkCreateDebugUtilsM"
                                                                                              "essengerEXT");

    ASSERT(func);
    VK_CHECK(func(instance, &debug_create_info, 0, debug_messenger));
#endif
}

bool
supportsPresentation(VkPhysicalDevice physicalDevice, uint32_t familyIndex)
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, familyIndex);
#else
#error Unsupported platform
#endif
}

uint32_t
getGraphicsFamilyIndex(VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, 0);

    if (queueFamilyCount != 0)
    {
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                return i;
        }
    }
    return VK_QUEUE_FAMILY_IGNORED;
}

VkPhysicalDevice
pickPhysicalDevice(VkPhysicalDevice *physicalDevices, uint32_t physicalDeviceCount)
{
    VkPhysicalDevice discrete = 0;
    VkPhysicalDevice fallback = 0;

    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
        printf("GPU%d: %s\n", i, properties.deviceName);

        uint32_t familyIndex = getGraphicsFamilyIndex(physicalDevices[i]);
        if (familyIndex == VK_QUEUE_FAMILY_IGNORED)
        {
            continue;
        }

        if (!supportsPresentation(physicalDevices[i], familyIndex))
        {
            continue;
        }

        if (!discrete && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            discrete = physicalDevices[i];
        }

        if (!fallback)
        {
            fallback = physicalDevices[i];
        }
    }

    auto deviceToReturn = discrete ? discrete : fallback;
    if (deviceToReturn)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(deviceToReturn, &props);
        printf("Seleting GPU: %s\n", props.deviceName);
    }
    else
    {
        printf("No supported GPUs found!\n");
    }

    return deviceToReturn;
}

VkDevice
createLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t familyIndex)
{
    float queuePriorities[]{1.0f};

    VkDeviceQueueCreateInfo queue_create_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue_create_info.queueFamilyIndex = familyIndex;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = queuePriorities;

    VkDevice device = 0;
    VkDeviceCreateInfo device_create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    const char *extensionNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = &extensionNames;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;
    VK_CHECK(vkCreateDevice(physicalDevice, &device_create_info, 0, &device));

    return device;
}

VkSurfaceKHR
createSurface(GLFWwindow *window, VkInstance instance)
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    info.hinstance = GetModuleHandle(0);
    info.hwnd = glfwGetWin32Window(window);
    VkSurfaceKHR surface{};
    vkCreateWin32SurfaceKHR(instance, &info, 0, &surface);
    ASSERT(surface);
    return surface;
#else
#error Unsupported platform
#endif
}

VkSurfaceFormatKHR
getSwapchainFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t formatCount{};
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, 0);
    if (formatCount != 0)
    {
        surfaceFormats.resize(formatCount);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                                      surfaceFormats.data()));
        return surfaceFormats[0];
    }
    VkSurfaceFormatKHR surfaceFormat{};
    return surfaceFormat;
}

VkImageView
createImageView(VkDevice device, VkImage image, VkFormat format)
{
    VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;
    VkImageView imageView;
    VK_CHECK(vkCreateImageView(device, &viewInfo, 0, &imageView));
    return imageView;
}

VkFramebuffer
createFramebuffer(VkDevice device, VkRenderPass renderPass, VkImageView imageView, uint32_t width, uint32_t height)
{
    VkFramebufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &imageView;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer;
    VK_CHECK(vkCreateFramebuffer(device, &createInfo, 0, &framebuffer));
    return framebuffer;
}

void
destroySwapchain(VkDevice device, SwapChain &swapchain)
{
    VK_CHECK(vkDeviceWaitIdle(device));

    for (const auto &framebuffer : swapchain.swapchainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, 0);
    }

    for (const auto &imageView : swapchain.swapchainImageViews)
    {
        vkDestroyImageView(device, imageView, 0);
    }

    vkDestroySwapchainKHR(device, swapchain._swapchain, 0);
}

void
createSwapchain(SwapChain &swapchain, VkDevice device, VkRenderPass renderPass, VkSurfaceKHR surface,
                uint32_t familyIndex, int windowWidth, int windowHeight, VkSurfaceFormatKHR surfaceFormat)
{
    if (swapchain._swapchain && swapchain.swapchainImageCount > 0)
        destroySwapchain(device, swapchain);

    VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = surface;
    createInfo.minImageCount = 2;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent.width = windowWidth;
    createInfo.imageExtent.height = windowHeight;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices = &familyIndex;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain._swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain._swapchain, &swapchain.swapchainImageCount, 0));
    if (swapchain.swapchainImageCount > 0)
    {
        if (swapchain.swapchainImages.size() != swapchain.swapchainImageCount)
            swapchain.swapchainImages.resize(swapchain.swapchainImageCount);

        if (swapchain.swapchainImageViews.size() != swapchain.swapchainImageCount)
            swapchain.swapchainImageViews.resize(swapchain.swapchainImageCount);

        if (swapchain.swapchainFramebuffers.size() != swapchain.swapchainImageCount)
            swapchain.swapchainFramebuffers.resize(swapchain.swapchainImageCount);

        VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain._swapchain, &swapchain.swapchainImageCount,
                                         swapchain.swapchainImages.data()));
    }

    for (uint32_t i = 0; i < swapchain.swapchainImageCount; ++i)
    {
        swapchain.swapchainImageViews[i] = createImageView(device, swapchain.swapchainImages[i],
                                                           surfaceFormat.format);
    }

    for (uint32_t i = 0; i < swapchain.swapchainImageCount; ++i)
    {
        swapchain.swapchainFramebuffers[i] = createFramebuffer(device, renderPass,
                                                               swapchain.swapchainImageViews[i], windowWidth,
                                                               windowHeight);
    }
}

VkSemaphore
createSemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo createInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore semaphore;
    VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &semaphore));
    return semaphore;
}

VkCommandPool
createCommandPool(VkDevice device, uint32_t familyIndex)
{
    VkCommandPool commandPool{};
    VkCommandPoolCreateInfo poolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = familyIndex;
    VK_CHECK(vkCreateCommandPool(device, &poolCreateInfo, 0, &commandPool));
    return commandPool;
}

VkRenderPass
createRenderPass(VkDevice device, VkFormat format)
{
    VkAttachmentReference colorAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    VkAttachmentDescription attachmentDescriptions[1]{};
    attachmentDescriptions[0].format = format;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkRenderPass renderpass;
    VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    createInfo.attachmentCount = ARRAY_SIZE(attachmentDescriptions);
    createInfo.pAttachments = attachmentDescriptions;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;

    VK_CHECK(vkCreateRenderPass(device, &createInfo, 0, &renderpass));

    return renderpass;
}

VkShaderModule
loadShader(VkDevice device, const char *path)
{
    FILE *file = fopen(path, "rb");

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    long length_aligned = (length + 3) & ~3;

    ASSERT(length >= 0);
    fseek(file, 0, SEEK_SET);


    char *buffer = new char[length_aligned];
    ASSERT(buffer);

    size_t rc = fread(buffer, 1, length, file);
    ASSERT(rc == length);
    fclose(file);

    VkShaderModuleCreateInfo createInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.codeSize = length_aligned;
    createInfo.pCode = reinterpret_cast<uint32_t *>(buffer);

    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, 0, &shaderModule);

    return shaderModule;
}

VkPipelineLayout
createPipelineLayout(VkDevice device)
{
    VkPipelineLayoutCreateInfo layoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    VkPipelineLayout pipelineLayout{};
    VK_CHECK(vkCreatePipelineLayout(device, &layoutCreateInfo, 0, &pipelineLayout));
    return pipelineLayout;
}

VkPipeline
createGraphicsPipeline(VkDevice device, VkPipelineCache pipelineCache, VkRenderPass renderPass,
                       VkPipelineLayout pipelineLayout, VkShaderModule vsModule, VkShaderModule fragModule)
{
    VkGraphicsPipelineCreateInfo createInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

    VkPipelineShaderStageCreateInfo vertStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vsModule;
    vertStage.pName = "main";
    VkPipelineShaderStageCreateInfo fragStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName = "main";
    VkPipelineShaderStageCreateInfo stages[]{vertStage, fragStage};
    createInfo.stageCount = ARRAY_SIZE(stages);
    createInfo.pStages = stages;

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    createInfo.pVertexInputState = &vertexInputStateCreateInfo;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

    VkPipelineViewportStateCreateInfo viewportInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    createInfo.pViewportState = &viewportInfo;
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizerInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizerInfo.lineWidth = 1.0f;
    createInfo.pRasterizationState = &rasterizerInfo;

    VkPipelineColorBlendAttachmentState colorBlendState{};
    colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendState;
    createInfo.pColorBlendState = &colorBlendInfo;

    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dynamicState.dynamicStateCount = ARRAY_SIZE(dynamicStates);
    dynamicState.pDynamicStates = dynamicStates;
    createInfo.pDynamicState = &dynamicState;

    createInfo.layout = pipelineLayout;
    createInfo.renderPass = renderPass;

    VkPipeline pipeline;
    VK_CHECK(vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, 0, &pipeline));
    return pipeline;
}

VkImageMemoryBarrier
imageBarrier(VkAccessFlags srcAccessMask, VkImageLayout oldLayout, VkAccessFlags dstAccessMask,
             VkImageLayout newLayout, VkImage image)
{
    VkImageMemoryBarrier _imageBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    _imageBarrier.srcAccessMask = srcAccessMask;
    _imageBarrier.dstAccessMask = dstAccessMask;
    _imageBarrier.oldLayout = oldLayout;
    _imageBarrier.newLayout = newLayout;
    _imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _imageBarrier.image = image;
    _imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    _imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return _imageBarrier;
}

void
destroyDebugUtils(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger)
{
#if defined(_DEBUG)
    if (*debug_messenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                                                                                        instance,
                                                                                        "vkDestroyDebugUtilsMessen"
                                                                                        "gerEXT");
        ASSERT(func);
        func(instance, *debug_messenger, 0);
    }
#endif
}

int
main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    volkInitialize();

    VkInstance instance = createInstance();
    ASSERT(instance);
    volkLoadInstance(instance);

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger{};
    setupDebugUtils(instance, &debug_messenger);
    ASSERT(debug_messenger);
#endif

    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount = ARRAY_SIZE(physicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
    ASSERT(physicalDevice);

    uint32_t graphicsFamily = getGraphicsFamilyIndex(physicalDevice);
    VkDevice device = createLogicalDevice(physicalDevice, graphicsFamily);

    volkLoadDevice(device);

    GLFWwindow *window = glfwCreateWindow(1600, 900, "Niagara", 0, 0);
    ASSERT(window);

    VkSurfaceKHR surface = createSurface(window, instance);
    ASSERT(surface);

    VkBool32 presentSupported = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsFamily, surface, &presentSupported));

    int windowWidth = 0, windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    auto surfaceFormat = getSwapchainFormat(physicalDevice, surface);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

    VkSemaphore acquireSemaphore = createSemaphore(device);
    ASSERT(acquireSemaphore);

    VkSemaphore releaseSemaphore = createSemaphore(device);
    ASSERT(releaseSemaphore);

    VkQueue queue{};
    vkGetDeviceQueue(device, graphicsFamily, 0, &queue);

    VkRenderPass renderPass = createRenderPass(device, surfaceFormat.format);
    ASSERT(renderPass);

    SwapChain swapchain{};

    createSwapchain(swapchain, device, renderPass, surface, graphicsFamily, windowWidth, windowHeight,
                    surfaceFormat);

    VkShaderModule vertShader = loadShader(device, "shaders/spirv/triangle.vert.spv");
    ASSERT(vertShader);

    VkShaderModule fragShader = loadShader(device, "shaders/spirv/triangle.frag.spv");
    ASSERT(fragShader);

    VkPipelineCache pipelineCache = 0;

    VkPipelineLayout pipelineLayout = createPipelineLayout(device);
    VkPipeline trianglePipeline = createGraphicsPipeline(device, pipelineCache, renderPass, pipelineLayout,
                                                         vertShader, fragShader);

    VkCommandPool commandPool = createCommandPool(device, graphicsFamily);
    ASSERT(commandPool);

    VkCommandBuffer commandBuffer{};
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo cbBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkClearColorValue color = {48.f / 255.f, 10.f / 255.f, 36.f / 255.f, 1};
    VkClearValue clearValue = {color};

    VkPipelineStageFlags stages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo queueSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    queueSubmitInfo.waitSemaphoreCount = 1;
    queueSubmitInfo.pWaitSemaphores = &acquireSemaphore;
    queueSubmitInfo.pWaitDstStageMask = &stages;
    queueSubmitInfo.commandBufferCount = 1;
    queueSubmitInfo.pCommandBuffers = &commandBuffer;
    queueSubmitInfo.signalSemaphoreCount = 1;
    queueSubmitInfo.pSignalSemaphores = &releaseSemaphore;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int currentWidth = 0, currentHeight = 0;
        glfwGetWindowSize(window, &currentWidth, &currentHeight);
        if (currentWidth != windowWidth || currentHeight != windowHeight)
        {
            windowWidth = currentWidth;
            windowHeight = currentHeight;
            createSwapchain(swapchain, device, renderPass, surface, graphicsFamily, windowWidth, windowHeight,
                            surfaceFormat);
        }

        uint32_t imageIndex = 0;
        VK_CHECK(vkAcquireNextImageKHR(device, swapchain._swapchain, ~0ull, acquireSemaphore, VK_NULL_HANDLE,
                                       &imageIndex));

        VK_CHECK(vkResetCommandPool(device, commandPool, 0));

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &cbBeginInfo));

        VkRenderPassBeginInfo passBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        passBeginInfo.renderPass = renderPass;
        passBeginInfo.framebuffer = swapchain.swapchainFramebuffers[imageIndex];
        passBeginInfo.renderArea.extent.width = windowWidth;
        passBeginInfo.renderArea.extent.height = windowHeight;
        passBeginInfo.clearValueCount = 1;
        passBeginInfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(commandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{0, float(windowHeight), float(windowWidth), -float(windowHeight)};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        VkRect2D scissor{{0, 0}, {uint32_t(windowWidth), uint32_t(windowHeight)}};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));

        vkQueueSubmit(queue, 1, &queueSubmitInfo, 0);

        VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain._swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &releaseSemaphore;
        VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));

        VK_CHECK(vkDeviceWaitIdle(device));
    }

    VK_CHECK(vkDeviceWaitIdle(device));
    vkDestroyShaderModule(device, fragShader, 0);
    vkDestroyShaderModule(device, vertShader, 0);
    vkDestroyPipelineLayout(device, pipelineLayout, 0);
    vkDestroyPipeline(device, trianglePipeline, 0);
    vkDestroyCommandPool(device, commandPool, 0);
    destroySwapchain(device, swapchain);
    vkDestroyRenderPass(device, renderPass, 0);
    vkDestroySemaphore(device, acquireSemaphore, 0);
    vkDestroySemaphore(device, releaseSemaphore, 0);
    glfwDestroyWindow(window);
    vkDestroySurfaceKHR(instance, surface, 0);
    vkDestroyDevice(device, 0);
#if defined(_DEBUG)
    destroyDebugUtils(instance, &debug_messenger);
#endif
    vkDestroyInstance(instance, 0);
    return 0;
}