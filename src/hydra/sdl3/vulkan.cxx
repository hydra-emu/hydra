#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include <SDL3/SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <string>
#include <vector>

#include <hydra/common/log.hxx>
#include <hydra/sdl3/window.hxx>

struct InnerContext
{
    VkInstance instance;
    VkAllocationCallbacks* allocator = nullptr;
    VkDebugReportCallbackEXT debugReport;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t queueFamily;
    VkQueue queue;
    VkDescriptorPool descriptorPool;
    VkPipelineCache pipelineCache;
    ImGui_ImplVulkanH_Window mainWindowData;
    bool swapchainRebuild = false;
};

constexpr int minImageCount = 2;
static bool debugVulkan = false; // TODO: make configurable at runtime

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
                                                    VkDebugReportObjectTypeEXT objectType,
                                                    uint64_t object, size_t location,
                                                    int32_t messageCode, const char* pLayerPrefix,
                                                    const char* pMessage, void* pUserData)
{
    (void)flags;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;
    (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType,
            pMessage);
    return VK_FALSE;
}

static void checkResult(VkResult result)
{
    if (result == 0)
        return;
    hydra::panic("Error: VkResult = {}", (int)result);
}

static bool hasExtension(const std::vector<VkExtensionProperties>& deviceExtensions,
                         const std::string& extension)
{
    bool found = std::any_of(
        deviceExtensions.begin(), deviceExtensions.end(),
        [&extension](const VkExtensionProperties& ext) { return extension == ext.extensionName; });
    return found;
}

static void mustExtension(const std::vector<VkExtensionProperties>& haveExtensions,
                          std::vector<const char*>& wantExtensions, const char* extension)
{
    if (hasExtension(haveExtensions, extension))
    {
        wantExtensions.push_back(extension);
    }
    else
    {
        hydra::panic("Vulkan extension {} not found", extension);
    }
}

static VkPhysicalDevice selectPhysicalDevice(hydra::SDL3::Context* context)
{
    InnerContext* ctx = (InnerContext*)context->inner;
    // TODO: allow for choosing GPU
    uint32_t gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(ctx->instance, &gpuCount, nullptr);
    checkResult(result);

    if (gpuCount == 0)
        hydra::panic("No GPU with Vulkan support found");

    std::vector<VkPhysicalDevice> gpus;
    gpus.resize(gpuCount);
    result = vkEnumeratePhysicalDevices(ctx->instance, &gpuCount, gpus.data());
    checkResult(result);

    // find and use first discrete gpu
    for (VkPhysicalDevice& device : gpus)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return device;
    }

    // otherwise use first gpu available
    if (gpuCount > 0)
        return gpus[0];

    return VK_NULL_HANDLE;
}

static void setupVulkan(hydra::SDL3::Context* context,
                        std::vector<const char*>& wantInstanceExtensions)
{
    InnerContext* ctx = (InnerContext*)context->inner;
    VkResult result;
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "hydra";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Get extensions available on the system
    uint32_t haveInstanceExtensionCount;
    std::vector<VkExtensionProperties> haveInstanceExtensions;
    vkEnumerateInstanceExtensionProperties(nullptr, &haveInstanceExtensionCount, nullptr);
    haveInstanceExtensions.resize(haveInstanceExtensionCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &haveInstanceExtensionCount,
                                                    haveInstanceExtensions.data());
    checkResult(result);

    if (hasExtension(haveInstanceExtensions,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        wantInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    // These are needed for OpenGL interoperability
    mustExtension(haveInstanceExtensions, wantInstanceExtensions,
                  VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    mustExtension(haveInstanceExtensions, wantInstanceExtensions,
                  VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    mustExtension(haveInstanceExtensions, wantInstanceExtensions,
                  VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);

    // If using MacOS with the latest MoltenVK sdk, you may get VK_ERROR_INCOMPATIBLE_DRIVER
    // returned from vkCreateInstance. According to the Getting Start Notes. Be-
    // ginning with the 1.3.216 Vulkan SDK, the VK_KHR_PORTABILITY_subset
    // extension is mandatory.
#ifdef HYDRA_LINUX
    mustExtension(haveInstanceExtensions, wantInstanceExtensions,
                  VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    if (debugVulkan)
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        bool found = std::any_of(
            layerProperties.begin(), layerProperties.end(), [](const VkLayerProperties& layer) {
                return std::string(layer.layerName) == "VK_LAYER_KHRONOS_validation";
            });

        if (!found)
        {
            hydra::log("VK_LAYER_KHRONOS_validation not found, disabling debug mode");
            debugVulkan = false;
        }
        else
        {
            const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = layers;
            wantInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
    }

    // Create instance
    createInfo.enabledExtensionCount = wantInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = wantInstanceExtensions.data();
    result = vkCreateInstance(&createInfo, ctx->allocator, &ctx->instance);
    checkResult(result);

    if (debugVulkan)
    {
        auto vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
                ctx->instance, "vkCreateDebugReportCallbackEXT");
        VkDebugReportCallbackCreateInfoEXT debugReportInfo = {};
        debugReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugReportInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debugReportInfo.pfnCallback = debugCallback;
        debugReportInfo.pUserData = nullptr;
        result = vkCreateDebugReportCallbackEXT(ctx->instance, &debugReportInfo, ctx->allocator,
                                                &ctx->debugReport);
        checkResult(result);
    }

    // Get physical device
    ctx->physicalDevice = selectPhysicalDevice(context);

    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &count, nullptr);
    VkQueueFamilyProperties* queues = new VkQueueFamilyProperties[count];
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &count, queues);
    for (uint32_t i = 0; i < count; i++)
    {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            ctx->queueFamily = i;
            break;
        }
    }
    delete[] queues;

    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    // Enumerate physical device extensions
    uint32_t physicalDeviceExtensionCount;
    std::vector<VkExtensionProperties> physicalDeviceExtensions;
    vkEnumerateDeviceExtensionProperties(ctx->physicalDevice, nullptr,
                                         &physicalDeviceExtensionCount, nullptr);
    physicalDeviceExtensions.resize(physicalDeviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(ctx->physicalDevice, nullptr,
                                         &physicalDeviceExtensionCount,
                                         physicalDeviceExtensions.data());

    const float queuePriority[] = {1.0f};
    VkDeviceQueueCreateInfo queueInfo[1] = {};
    queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[0].queueFamilyIndex = ctx->queueFamily;
    queueInfo[0].queueCount = 1;
    queueInfo[0].pQueuePriorities = queuePriority;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
    create_info.pQueueCreateInfos = queueInfo;
    create_info.enabledExtensionCount = deviceExtensions.size();
    create_info.ppEnabledExtensionNames = deviceExtensions.data();
    result = vkCreateDevice(ctx->physicalDevice, &create_info, ctx->allocator, &ctx->device);
    checkResult(result);
    vkGetDeviceQueue(ctx->device, ctx->queueFamily, 0, &ctx->queue);

    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();
    result = vkCreateDescriptorPool(ctx->device, &pool_info, ctx->allocator, &ctx->descriptorPool);
    checkResult(result);
}

static void setupVulkanWindow(hydra::SDL3::Context* context, ImGui_ImplVulkanH_Window* wd,
                              VkSurfaceKHR surface, int width, int height)
{
    InnerContext* ctx = (InnerContext*)context->inner;
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physicalDevice, ctx->queueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical ctx->device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM,
                                                  VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM,
                                                  VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        ctx->physicalDevice, wd->Surface, requestSurfaceImageFormat,
        (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef APP_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
                                        VK_PRESENT_MODE_FIFO_KHR};
#else
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif

    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        ctx->physicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

    // Create SwapChain, RenderPass, Framebuffer, etc.
    ImGui_ImplVulkanH_CreateOrResizeWindow(ctx->instance, ctx->physicalDevice, ctx->device, wd,
                                           ctx->queueFamily, ctx->allocator, width, height,
                                           minImageCount);
}

static void frameRender(hydra::SDL3::Context* context, ImGui_ImplVulkanH_Window* wd,
                        ImDrawData* draw_data)
{
    InnerContext* ctx = (InnerContext*)context->inner;
    VkResult result;
    VkSemaphore image_acquired_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    result = vkAcquireNextImageKHR(ctx->device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore,
                                   VK_NULL_HANDLE, &wd->FrameIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        ctx->swapchainRebuild = true;
        return;
    }
    checkResult(result);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        result = vkWaitForFences(ctx->device, 1, &fd->Fence, VK_TRUE,
                                 UINT64_MAX); // wait indefinitely instead of periodically checking
        checkResult(result);

        result = vkResetFences(ctx->device, 1, &fd->Fence);
        checkResult(result);
    }
    {
        result = vkResetCommandPool(ctx->device, fd->CommandPool, 0);
        checkResult(result);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        result = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        checkResult(result);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        result = vkEndCommandBuffer(fd->CommandBuffer);
        checkResult(result);
        result = vkQueueSubmit(ctx->queue, 1, &info, fd->Fence);
        checkResult(result);
    }
}

static void framePresent(hydra::SDL3::Context* context, ImGui_ImplVulkanH_Window* wd)
{
    InnerContext* ctx = (InnerContext*)context->inner;
    if (ctx->swapchainRebuild)
        return;

    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;

    VkResult result = vkQueuePresentKHR(ctx->queue, &info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        ctx->swapchainRebuild = true;
        return;
    }
    checkResult(result);

    wd->SemaphoreIndex =
        (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

namespace hydra::SDL3::Vk
{

    Context* init()
    {
        Context* context = new Context();
        InnerContext* ctx = new InnerContext();
        context->inner = ctx;

        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            hydra::panic("SDL_Init: {}", SDL_GetError());
        }

        uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        context->window = SDL_CreateWindow("hydra", 1270, 720, flags);

        if (!context->window)
        {
            hydra::panic("SDL_CreateWindow retuned null: {}", SDL_GetError());
        }

        SDL_SetWindowPosition(context->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        uint32_t neededExtensionCount = 0;
        const char* const* neededExtensions =
            SDL_Vulkan_GetInstanceExtensions(&neededExtensionCount);
        std::vector<const char*> neededInstanceExtensions(neededExtensions,
                                                          neededExtensions + neededExtensionCount);
        setupVulkan(context, neededInstanceExtensions);

        // Create Window Surface
        VkSurfaceKHR surface;
        if (SDL_Vulkan_CreateSurface(context->window, ctx->instance, ctx->allocator, &surface) == 0)
        {
            hydra::panic("SDL_Vulkan_CreateSurface: {}", SDL_GetError());
        }

        int w, h;
        SDL_GetWindowSize(context->window, &w, &h);
        ImGui_ImplVulkanH_Window* wd = &ctx->mainWindowData;
        setupVulkanWindow(context, wd, surface, w, h);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForVulkan(context->window);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = ctx->instance;
        initInfo.PhysicalDevice = ctx->physicalDevice;
        initInfo.Device = ctx->device;
        initInfo.QueueFamily = ctx->queueFamily;
        initInfo.Queue = ctx->queue;
        initInfo.PipelineCache = ctx->pipelineCache;
        initInfo.DescriptorPool = ctx->descriptorPool;
        initInfo.Subpass = 0;
        initInfo.MinImageCount = minImageCount;
        initInfo.ImageCount = wd->ImageCount;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = ctx->allocator;
        initInfo.CheckVkResultFn = checkResult;
        ImGui_ImplVulkan_Init(&initInfo, wd->RenderPass);

        return context;
    }

    void Shutdown(Context* context)
    {
        InnerContext* ctx = (InnerContext*)context->inner;
        VkResult result = vkDeviceWaitIdle(ctx->device);
        checkResult(result);

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        ImGui_ImplVulkanH_DestroyWindow(ctx->instance, ctx->device, &ctx->mainWindowData,
                                        ctx->allocator);

        vkDestroyDescriptorPool(ctx->device, ctx->descriptorPool, ctx->allocator);

        if (debugVulkan)
        {
            // Remove the debug report callback
            auto vkDestroyDebugReportCallbackEXT =
                (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
                    ctx->instance, "vkDestroyDebugReportCallbackEXT");
            vkDestroyDebugReportCallbackEXT(ctx->instance, ctx->debugReport, ctx->allocator);
        }

        vkDestroyDevice(ctx->device, ctx->allocator);
        vkDestroyInstance(ctx->instance, ctx->allocator);

        SDL_DestroyWindow(context->window);
        SDL_Quit();

        delete (InnerContext*)ctx;
        delete context;
    }

    void startFrame(Context* context)
    {
        InnerContext* ctx = (InnerContext*)context->inner;
        // Resize swap chain?
        if (ctx->swapchainRebuild)
        {
            int width, height;
            SDL_GetWindowSize(context->window, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(minImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(
                    ctx->instance, ctx->physicalDevice, ctx->device, &ctx->mainWindowData,
                    ctx->queueFamily, ctx->allocator, width, height, minImageCount);
                ctx->mainWindowData.FrameIndex = 0;
                ctx->swapchainRebuild = false;
            }
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void endFrame(Context* context)
    {
        InnerContext* ctx = (InnerContext*)context->inner;
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized =
            (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            ImGui_ImplVulkanH_Window* wd = &ctx->mainWindowData;
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            frameRender(context, wd, draw_data);
            framePresent(context, wd);
        }
    }

} // namespace hydra::SDL3::Vk