//
// Created by mgrus on 04.06.2025.
//

#include "../include/vulkan_impl.h"

#include <algorithm>
#include <iostream>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <shaderc/shaderc.hpp>


//#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <complex.h>
#include <engine.h>
#include <unordered_map>


static const int MAX_FRAMES_IN_FLIGHT = 1;
static VulkanRenderer* vulkan_renderer = nullptr;

// Handle system
static uint32_t nextHandleId = 1; // start at 1 to reserve 0 as "invalid"
static uint32_t nextVBHandleId = 1;
static uint32_t nextIBHandleId = 1;
static uint32_t nextTextureHandleId = 1;
static std::unordered_map<uint32_t, VulkanShader> shaderMap;
static std::unordered_map<uint32_t, VulkanVertexBuffer> vertexBufferMap;
static std::unordered_map<uint32_t, VulkanIndexBuffer> indexBufferMap;
static std::unordered_map<uint32_t, VulkanTexture> textureMap;

void VulkanRenderer::createQueryPool() {
    VkQueryPoolCreateInfo queryPoolInfo{};
    queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolInfo.queryCount = 2; // one for start, one for end

    if (vkCreateQueryPool(_device, &queryPoolInfo, nullptr, &_queryPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create query pool!");
    }
}

void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::recordImgaeTransitionCommand(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void VulkanRenderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    recordImgaeTransitionCommand(commandBuffer, image, oldLayout, newLayout);


    endSingleTimeCommands(commandBuffer);
}

VkDescriptorSetLayout VulkanRenderer::createDescriptorSetLayout(std::vector<std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlags>> bindingInfos) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (auto bi : bindingInfos) {
        
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = std::get<0>(bi);
        layoutBinding.descriptorType = std::get<1>(bi);
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = std::get<2>(bi);
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBindings.push_back(layoutBinding);
    }

    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = layoutBindings.size();
    layoutInfo.pBindings = layoutBindings.data();
    vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &descriptorSetLayout);

    return descriptorSetLayout;

}

void VulkanRenderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }




}

void VulkanRenderer::createDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT * 40},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * 40},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * 40}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)  * 40;   // max. 10 sets for now from this pool..
    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

std::vector<VkDescriptorSet> VulkanRenderer::createDescriptorSetsForLayout(VkDescriptorSetLayout layout,
    std::vector<std::tuple<uint32_t, VkBuffer, VkDeviceSize, VkDescriptorType>> binding_infos,
    std::vector<std::tuple<uint32_t, VkImageView, VkDeviceSize, VkDescriptorType, VkSampler>> image_infos) {

    std::vector layouts(MAX_FRAMES_IN_FLIGHT, layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // We create one descriptor set per frame-in-flight.
    // So we always have one ready to use, regardless which frame we are
    // currently handling:
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for (auto bi : binding_infos) {
            auto binding = std::get<0>(bi);
            auto buffer = std::get<1>(bi);
            auto size = std::get<2>(bi);
            auto type = std::get<3>(bi);
            updateDescriptorSetsForBuffers(descriptorSets[i], binding, buffer, size, type);
        }

        for (auto ii : image_infos) {
            auto binding = std::get<0>(ii);
            auto textureImageView = std::get<1>(ii);
            auto size = std::get<2>(ii);
            auto type = std::get<3>(ii);
            auto textureSampler = std::get<4>(ii);
            updateDescriptorSetsForImages(descriptorSets[i], binding, textureImageView, size, type, textureSampler);
        }

    }

    return descriptorSets;

}

void VulkanRenderer::createDescriptorSetsDefault() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }


    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        updateDescriptorSetsForBuffers(_descriptorSets[i], 0, _uniformBuffers[i], sizeof(UniformBufferObject), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }

}

void VulkanRenderer::updateDescriptorSetsForImages(VkDescriptorSet descriptorSet, uint32_t binding, VkImageView textureImageView, VkDeviceSize size, VkDescriptorType type, VkSampler textureSampler) {

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);

}


void VulkanRenderer::updateDescriptorSetsForBuffers(VkDescriptorSet descriptorSet, uint32_t binding, VkBuffer buffer, VkDeviceSize size, VkDescriptorType type) {

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = size;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = type;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);

}

VulkanRenderer::VulkanRenderer(HINSTANCE hInstance, HWND window) : _hInstance(hInstance), _window(window) {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createQueryPool();
    createSwapChain();
    createImageViews();
    createRenderPass();

    createDescriptorSetLayout();
    //createDefaultTestGraphicsPipeline();
    createDepthResources();
    createFrameBuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSetsDefault();
    createCommandBuffer();
    createSyncObjects();

}

VulkanRenderer::~VulkanRenderer() {

    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    // vkDestroyImage(_device, textureImage, nullptr);
    // vkFreeMemory(device, textureImageMemory, nullptr);
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    for (auto imageView : _swapChainImageViews) {
        vkDestroyImageView(_device, imageView, nullptr);
    }



    vkDestroyDevice(_device, nullptr);
    vkDestroyInstance(_instance, nullptr);

}

void VulkanRenderer::clearBuffers() {

}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageType,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                             pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << "Validation layer error: " << pCallbackData->pMessage << std::endl;
    } else {
        std::cout << "Validation layer debug msg: " << pCallbackData->pMessage << std::endl;
    }


    return VK_FALSE;
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {}; // zero everything
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
}

VkResult VulkanRenderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanRenderer::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Generic Vulkan Game";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "Available extensions:\n";
    for (const auto& ext : extensions) {
        std::cout << "\t" << ext.extensionName << "\n";
    }

    std::vector<const char*> extensionsRequested;
    extensionsRequested.push_back(VK_KHR_SURFACE_EXTENSION_NAME); // Always required for surface creation
    extensionsRequested.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    extensionsRequested.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Required for debug messenger


    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionsRequested.size();
    createInfo.ppEnabledExtensionNames = extensionsRequested.data();
    if (createValidationLayers()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    for (const auto& layer : validationLayers) {
        std::cout << "Available layer: " << layer << std::endl;
    }

    if (vkCreateInstance(&createInfo, NULL, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Vulkan instance");
    }


}

void VulkanRenderer::setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

bool VulkanRenderer::createValidationLayers() {

#ifdef NO_VALIDATION_LAYERS
    return false;
#endif

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());


    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
        return false;
        }
    }

    return true;

}

void VulkanRenderer::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        exit(1);
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            _physicalDevice = device;
            break;
        }
    }

    if (_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }



}

void VulkanRenderer::createLogicalDevice() {
    auto familyIndices = findQueueFamilies(_physicalDevice);


    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = familyIndices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    auto queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo logicalDeviceCreateInfo{};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    logicalDeviceCreateInfo.queueCreateInfoCount = 1;
    logicalDeviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    logicalDeviceCreateInfo.enabledExtensionCount = 1;
    logicalDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();


    if (vkCreateDevice(_physicalDevice, &logicalDeviceCreateInfo, nullptr, &_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device");
    }

    // Retrieve our single graphics queue for now
    vkGetDeviceQueue(_device, familyIndices.graphicsFamily.value(), 0, &_graphicsQueue);



    float queuePrio = 1.0;
    VkDeviceQueueCreateInfo presentQueueCreateInfo{};
    presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    presentQueueCreateInfo.queueFamilyIndex = familyIndices.presentFamily.value();
    presentQueueCreateInfo.queueCount = 1;
    presentQueueCreateInfo.pQueuePriorities = &queuePrio;

    vkGetDeviceQueue(_device, familyIndices.presentFamily.value(), 0, &_presentQueue);

}

void VulkanRenderer::createSurface() {
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hwnd = _window;
    surfaceCreateInfo.hinstance = _hInstance;


    if (vkCreateWin32SurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

}

void VulkanRenderer::createSwapChain() {
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR > surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, surfaceFormats.data());

    for (auto f : surfaceFormats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            _surfaceFormat = f;
            break;
        }

    }


    RECT r;
    GetWindowRect(_window, &r);

    // Calculate the surface extent:
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);
    _extent.width = r.right-r.left;
    _extent.height = r.bottom-r.top;
    _extent.width = std::clamp(_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    _extent.height = std::clamp(_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    // Prepare swap chain creation info struct:
    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = _surface;
    swapChainInfo.imageFormat = _surfaceFormat.format;
    swapChainInfo.imageColorSpace = _surfaceFormat.colorSpace;
    swapChainInfo.minImageCount = 2;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageExtent = _extent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device, &swapChainInfo, nullptr, &_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain");
    }

    // We aquire the actual images into which we render. First we find out, how many images have been created
    // in our swapchain.
    uint32_t swapChainImageCount = 0;
    if (vkGetSwapchainImagesKHR(_device, _swapChain, &swapChainImageCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to get swap chain image count.");
    }
    // Now actually retrieve the images.
    _swapChainImages.resize(swapChainImageCount);
    _swapChainImages.resize(swapChainImageCount);
    if (vkGetSwapchainImagesKHR(_device, _swapChain, &swapChainImageCount, _swapChainImages.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to get swap chain image.");
    }
}

VkSampler VulkanRenderer::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler textureSampler;
    if (vkCreateSampler(_device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    return textureSampler;
}

void VulkanRenderer::createImageViews() {
    _swapChainImageViews.resize(_swapChainImages.size());

    for (size_t i = 0; i < _swapChainImages.size(); i++) {
        _swapChainImageViews[i] = createImageView(_swapChainImages[i], _surfaceFormat.format);
        // VkImageViewCreateInfo createInfo{};
        // createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // createInfo.image = _swapChainImages[i];
        // createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // createInfo.format = _surfaceFormat.format;
        // createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // createInfo.subresourceRange.baseMipLevel = 0;
        // createInfo.subresourceRange.levelCount = 1;
        // createInfo.subresourceRange.baseArrayLayer = 0;
        // createInfo.subresourceRange.layerCount = 1;
        // if (vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS) {
        //     throw std::runtime_error("failed to create image view!");
        // }
    }
}

void VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage&image, VkDeviceMemory&imageMemory) {

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, image, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements, memProperties, properties);

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_device, image, imageMemory, 0);
}

std::tuple<VkFramebuffer, renderer::Image, VkImageView, VkImage> VulkanRenderer::createOffscreenFrameBuffer(int width, int height, VkFormat colorFormat,
                                                         VkFormat depthFormat, VkRenderPass renderPass) {

    renderer::Image image;
    image.width = width;
    image.height = height;
    image.channels = 4;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;

    createImage(width, height, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,  colorImage, colorImageMemory);
    auto colorImageView = createImageView(colorImage, colorFormat);

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;

    createImage(width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,  depthImage, depthImageMemory);
    auto depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    std::array attachments = {
        colorImageView,
        depthImageView
    };
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(std::size(attachments));
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    VkFramebuffer frameBuffer;

    if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }

    return {frameBuffer, image, colorImageView, colorImage} ;
}

void VulkanRenderer::createFrameBuffers() {
    _swapChainFramebuffers.resize(_swapChainImages.size());
    std::array<VkImageView,2> attachments;
    for (size_t i = 0; i < _swapChainImageViews.size(); i++) {
         attachments = {
            _swapChainImageViews[i],
            _depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(std::size(attachments));
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _extent.width;
        framebufferInfo.height = _extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

VkShaderModule VulkanRenderer::createShaderModule(std::vector<uint8_t> spirv) {
    auto shaderCreateInfo = VkShaderModuleCreateInfo();
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.pNext = nullptr;
    shaderCreateInfo.flags = 0;
    shaderCreateInfo.codeSize = spirv.size();
    shaderCreateInfo.pCode = reinterpret_cast<uint32_t *>(spirv.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

VkShaderModule VulkanRenderer::createShaderModule(std::vector<uint32_t> spirv) {
    auto shaderCreateInfo = VkShaderModuleCreateInfo();
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.pNext = nullptr;
    shaderCreateInfo.flags = 0;
    shaderCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
    shaderCreateInfo.pCode = spirv.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::tuple<VkPipeline, VkPipelineLayout> VulkanRenderer::createGraphicsPipeline(VkShaderModule vertexModule, VkShaderModule fragModule,
                                                  std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
                                                  VkVertexInputBindingDescription bindingDescription,
                                                  VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass) {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // How our vertex data looks like:
    //auto bindingDescription = PosColorVertex::getBindingDescription();
    //auto attributeDescriptions = PosColorVertex::getAttributeDescriptions();


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Describe the geometry and if primitive restart is enabled:
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) _extent.width;
    viewport.height = (float) _extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipeline_layout;
    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return {pipeline, pipeline_layout};

}

void VulkanRenderer::executeCommandBuffers(std::vector<std::vector<VkCommandBuffer>> commandBuffers) {
    vkWaitForFences(_device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device, 1, &_inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = commandBuffers[imageIndex].size();
    submitInfo.pCommandBuffers = commandBuffers[imageIndex].data();

    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(_presentQueue, &presentInfo);

}


void VulkanRenderer::executeCommandBuffer(std::vector<VkCommandBuffer> commandBuffers) {
    vkWaitForFences(_device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device, 1, &_inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }


#ifdef PERFORMANCE_TESTS
    auto end = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(end - cpu_start).count();
    std::cout << "CPU time: " << elapsedMs << " ms" << std::endl;
#endif

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(_presentQueue, &presentInfo);

}

VkImageView VulkanRenderer::createTextureImageView(VkImage image, VkFormat format) {
    auto textureImageView = createImageView(image, format);
    return textureImageView;
}



VkImage VulkanRenderer::createTextureImage(const renderer::Image &image, VkFormat format, VkDeviceSize imageSize) {

    //VkDeviceSize imageSize = image.width * image.height * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, image.pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(_device, stagingBufferMemory);

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    createImage(image.width, image.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));

    transitionImageLayout(textureImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

    return textureImage;
}

VkCommandBuffer VulkanRenderer::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void * VulkanRenderer::mapMemory(VkDeviceMemory memory, int offset, uint64_t size) {
    void* mappedData;
    vkMapMemory(_device, memory, offset, size, 0, &mappedData);
    return mappedData;
}

void VulkanRenderer::endSecondaryCommandBuffer(VkCommandBuffer vk_command_buffer) {
    vkEndCommandBuffer(vk_command_buffer);
}

VkPipelineLayout VulkanRenderer::getDefaultPipelineLayout() {
    return _pipelineLayout;
}

VkPipeline VulkanRenderer::getDefaultPipeline() {
    return _graphicsPipeline;
}


void VulkanRenderer::createDefaultTestGraphicsPipeline() {

    auto vertShaderCode = R"(
        #version 450

        layout(binding = 0) uniform UniformBufferObject {
            mat4 model;
            mat4 view;
            mat4 proj;
        } ubo;

        layout(location = 0) in vec2 inPosition;
        layout(location = 1) in vec3 inColor;

        vec2 positions[3] = vec2[](
            vec2(0.0, -0.5),
            vec2(0.5, 0.5),
            vec2(-0.5, 0.5)
        );

        void main() {
            //gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
            gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
        }

    )";
    auto fragShaderCode = R"(
        #version 450

        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vec4(.5, 0.0, 0.5, 1.0);
        }

    )";
    auto vert_handle = renderer::compileVertexShader(vertShaderCode);
    auto frag_handle = renderer::compileFragmentShader(fragShaderCode);
    auto vert_spirv = shaderMap[vert_handle.id].spirv_code;
    auto frag_spirv = shaderMap[frag_handle.id].spirv_code;

    // auto vert_spirv = read_file_binary_to_vector("../../assets/vk_shaders/basic_vert.spr");
    // auto frag_spirv = read_file_binary_to_vector("../../assets/vk_shaders/basic_frag.spr");
    // auto vert_handle = renderer::ShaderHandle{nextHandleId};
    // shaderMap[nextHandleId] = VulkanShader {vert_spirv, {}} ;
    // nextHandleId++;
    // auto frag_handle = renderer::ShaderHandle{nextHandleId};
    // shaderMap[nextHandleId] = VulkanShader {frag_spirv, {}} ;
    // nextHandleId++;

    auto vert_module =  createShaderModule(vert_spirv);
    auto frag_module = createShaderModule(frag_spirv);
    shaderMap[vert_handle.id].shader_module = vert_module;
    shaderMap[frag_handle.id].shader_module = frag_module;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vert_module;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = frag_module;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // How our vertex data looks like:
    auto bindingDescription = PosColorVertex::getBindingDescription();
    auto attributeDescriptions = PosColorVertex::getAttributeDescriptions();


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Describe the geometry and if primitive restart is enabled:
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) _extent.width;
    viewport.height = (float) _extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;

    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(_device, frag_module, nullptr);
    vkDestroyShaderModule(_device, vert_module, nullptr);
}

VkRenderPass VulkanRenderer::createCustomRenderPass(VkFormat colorFormat, VkFormat depthFormat) {
    VkRenderPass renderPass;
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = colorFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    return renderPass;
}

VkRenderPass VulkanRenderer::getDefaultFrameBufferRenderPass() {
    return _renderPass;
}

VkFramebuffer VulkanRenderer::getDefaultFramebuffer(int index) {
    return _swapChainFramebuffers[index];
}

void VulkanRenderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;      // TODO query actual available depthformats and choose a good one
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

uint32_t VulkanRenderer::findMemoryType(const VkMemoryRequirements &memoryRequirements, VkPhysicalDeviceMemoryProperties memProperties, VkMemoryPropertyFlags properties) {
    uint32_t typeFilter = memoryRequirements.memoryTypeBits;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;

        }
    }

    return 0;
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memoryRequirements);
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements, memProperties, properties);

    if (vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);


}

void VulkanRenderer::copyBuffer(VkDeviceSize bufferSize, VkBuffer sourceBuffer, VkBuffer targetBuffer) {
    // VkCommandBufferAllocateInfo allocInfo{};
    // allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    // allocInfo.commandPool = _commandPool;
    // allocInfo.commandBufferCount = 1;
    // VkCommandBuffer commandBuffer;
    // vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);
    // VkCommandBufferBeginInfo beginInfo{};
    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    // vkBeginCommandBuffer(commandBuffer, &beginInfo);
    //
    // VkBufferCopy copyRegion{};
    // copyRegion.srcOffset = 0;
    // copyRegion.dstOffset = 0;
    // copyRegion.size = bufferSize;
    // vkCmdCopyBuffer(commandBuffer, sourceBuffer, targetBuffer, 1, &copyRegion);
    // vkEndCommandBuffer(commandBuffer);
    //
    // VkSubmitInfo submitInfo{};
    // submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = &commandBuffer;
    // vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    // vkQueueWaitIdle(_graphicsQueue);
    //
    // vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, targetBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::updateUniformBuffer(int current_image) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), _extent.width / (float) _extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    memcpy(_uniformBuffersMapped[current_image], &ubo, sizeof(ubo));
}


VkBuffer VulkanRenderer::createVertexBuffer(renderer::VertexBufferCreateInfo vertex_buffer_create_info) {
    auto vb = createVertexBufferRaw(vertex_buffer_create_info.data.size() * sizeof(vertex_buffer_create_info.data[0]), vertex_buffer_create_info.data.data());
    return vb;
}



VkBuffer VulkanRenderer::createVertexBufferRaw(size_t size, void *data) {
    VkDeviceSize bufferSize = size;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* targetData;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &targetData);
    memcpy(targetData, data, bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    VkBuffer vb;
    VkDeviceMemory vertexBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vb, vertexBufferMemory);


    // Now the actual copy
    copyBuffer(bufferSize, stagingBuffer, vb);

    // Cleanup our staging buffer, we no longer need it:
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

    return vb;
}



void VulkanRenderer::createVertexBuffer() {
    // Use a staging buffer, accessed by host and the "real" vertex buffer, GPU only.
    // We copy data from CPU to the staging buffer, then to the actual vertex buffer on gpu. only once.
    // Every draw will then be done from the GPU vertex buffer.
    VkDeviceSize bufferSize = vertices.size() * sizeof(vertices[0]);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    VkDeviceMemory vertexBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, vertexBufferMemory);

    // Now the actual copy
    copyBuffer(bufferSize, stagingBuffer, _vertexBuffer);

    // Cleanup our staging buffer, we no longer need it:
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;

}

void VulkanRenderer::createDepthResources() {
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    createImage(_extent.width, _extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,  depthImage, depthImageMemory);
    _depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);


}

VkBuffer VulkanRenderer::createIndexBuffer(renderer::IndexBufferDesc ibd) {
    VkDeviceSize bufferSize = ibd.size_in_bytes;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, ibd.data, (size_t) bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    VkBuffer ib;
    VkDeviceMemory ibMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ib, ibMemory);

    copyBuffer(bufferSize, stagingBuffer, ib);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

    return ib;

}

void VulkanRenderer::createIndexBuffer() {
    VkDeviceSize bufferSize = indices.size() * sizeof(indices[0]);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void *data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);

    copyBuffer(bufferSize, stagingBuffer, _indexBuffer);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
    
}

void VulkanRenderer::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    _uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _uniformBuffers[i], _uniformBuffersMemory[i]);

        vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }
}


void VulkanRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanRenderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
    }

}

void VulkanRenderer::drawFrame() {
    vkWaitForFences(_device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device, 1, &_inFlightFence);

    // We assume updated uniforms for every sprite

}

void VulkanRenderer::updateMappedMemory(void* mappedMemory, void* sourceData, VkDeviceSize size) {
    memcpy(mappedMemory, sourceData, size);
}

void VulkanRenderer::uploadData(VkDeviceMemory dstMemory, VkDeviceSize size, void *sourceData) {
    void *data;
    vkMapMemory(_device, dstMemory, 0, size, 0, &data);
    memcpy(data, sourceData, size);
    vkUnmapMemory(_device, dstMemory);
}

void VulkanRenderer::drawFrameExp() {
    vkWaitForFences(_device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device, 1, &_inFlightFence);

#ifdef PERFORMANCE_TESTS
    // Performance queries
    {
        uint64_t timestamps[2] = {};
        vkGetQueryPoolResults(
            _device,
            _queryPool,
            0,
            2,
            sizeof(timestamps),
            timestamps,
            sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT
        );

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

        double timestampPeriod = properties.limits.timestampPeriod; // in nanoseconds per tick

        double gpuTimeNs = double(timestamps[1] - timestamps[0]) * timestampPeriod;
        double gpuTimeMs = gpuTimeNs / 1'000'000.0;
        std::cout << "GPU time: " << gpuTimeMs << " ms" << std::endl;

    }

    auto cpu_start = std::chrono::high_resolution_clock::now();
#endif

    updateUniformBuffer(0);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    // static bool firstFrame = true;
    // if (firstFrame) {
    //     firstFrame = false;
    //     vkResetCommandBuffer(_commandBuffer, 0);
    //     recordCommandBuffer(_commandBuffer, imageIndex);
    // }


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_imageAvailableCommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }


#ifdef PERFORMANCE_TESTS
    auto end = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(end - cpu_start).count();
    std::cout << "CPU time: " << elapsedMs << " ms" << std::endl;
#endif

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(_presentQueue, &presentInfo);
}


VkCommandBuffer VulkanRenderer::createCommandBuffer(VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }


    //vkResetCommandBuffer(commandBuffer, 0);

    return commandBuffer;


}

void VulkanRenderer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(_device, &allocInfo, &_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    // Create additional command buffer for each image in our swapchain:
    for (size_t i = 0; i < _swapChainImages.size(); i++) {
        VkCommandBuffer commandBuffer;

        if (vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        _imageAvailableCommandBuffers.push_back(commandBuffer);
        vkResetCommandBuffer(commandBuffer, 0);
        //recordCommandBuffer(commandBuffer, i);
    }
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {


    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    vkCmdResetQueryPool(commandBuffer, _queryPool, 0, 2);



    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _extent;
    VkClearValue clearColor = {{{0.1f, 0.0f, 0.4f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_extent.width);
    viewport.height = static_cast<float>(_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    //vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _extent;
    //vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

#ifndef NOT_ONLY_CLEAR
    VkBuffer vertexBuffers[] = {_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _queryPool, 0);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[0], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _queryPool, 1);

#endif
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

}

QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamilyIndex = 0;
    for (const auto& queueFamily : queueFamilies) {

        // // We just look for the graphics capability.
        // // But just for fun we also look for GPU_COMPUTE caps:
        // if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
        //     printf("can compute as well!");
        // }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            if (vkGetPhysicalDeviceWin32PresentationSupportKHR(device, queueFamilyIndex)) {

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, _surface, &presentSupport);

                if (presentSupport) {
                    indices.graphicsFamily = queueFamilyIndex;
                    indices.presentFamily = queueFamilyIndex;
                    break;
                }
            }

        }

        queueFamilyIndex++;
    }


    return indices;
}

void VulkanRenderer::beginSecondaryCommandBuffer(VkCommandBuffer commandBuffer, VkRenderPass renderPass, int imageIndex,
    VkCommandBufferInheritanceInfo* inheritance_info) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = inheritance_info;


    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void VulkanRenderer::beginFrameCommands(VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkResetCommandBuffer(commandBuffer, 0);
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording custom command buffer!");
    }
}

void VulkanRenderer::endFrameCommands(VkCommandBuffer commandBuffer) {
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanRenderer::beginCustomRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer, uint32_t width, uint32_t height) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = frameBuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {width, height};
    std::array<VkClearValue,2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

}

VkRenderPass VulkanRenderer::beginRenderPass(VkCommandBuffer commandBuffer, int swapChainImageIndex) {

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[swapChainImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _extent;
    std::array<VkClearValue,2> clearValues{};
    clearValues[0].color = {{0.1f, 0.0f, 0.4f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);



    return _renderPass;
}

void VulkanRenderer::endRenderPass(VkCommandBuffer commandBuffer) {
    vkCmdEndRenderPass(commandBuffer);
}

void VulkanRenderer::recordSecondaryExecCommandBuffers(VkCommandBuffer primary, std::vector<VkCommandBuffer> secondaries) {
    vkCmdExecuteCommands(primary, static_cast<uint32_t>(secondaries.size()), secondaries.data());
}

VkCommandBufferInheritanceInfo VulkanRenderer::createInheritanceInfo(VkRenderPass renderPass, VkFramebuffer frameBuffer) {
    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = renderPass;        // same as used in vkCmdBeginRenderPass
    inheritanceInfo.subpass = 0;                    // usually 0 unless you're inside a multi-subpass setup
    inheritanceInfo.framebuffer = frameBuffer;      // can be VK_NULL_HANDLE if not framebuffer-dependent
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags = 0;
    inheritanceInfo.pipelineStatistics = 0;
    return inheritanceInfo;
}

void VulkanRenderer::recordMultiMeshData(VkCommandBuffer commandBuffer, std::vector<VkBuffer> vertexBuffersParam, std::vector<VkBuffer> indexBuffers, std::vector<VkIndexType> indexTypes, VkPipelineLayout pipeline_layout, VkPipeline pipeline,
    std::vector<std::vector<VkDescriptorSet>> descriptorSets, std::vector<int> instance_counts, std::vector<int> instance_offsets, std::vector<uint32_t> num_indices) {

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    for (int i = 0; i < vertexBuffersParam.size(); i++) {

        VkBuffer vertexBuffers[] = {vertexBuffersParam[i]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffers[i], 0, indexTypes[i]);
        //vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _queryPool, 0);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptorSets[i][0], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, num_indices[i], instance_counts[i], 0, 0, instance_offsets[i]);
    }
    //vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _queryPool, 1);
}


void VulkanRenderer::recordMeshData(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer, VkIndexType indexType, VkPipelineLayout pipeline_layout, VkPipeline pipeline,
    std::vector<VkDescriptorSet> descriptorSets, int instance_count, int instance_offset, uint32_t num_indices) {

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, indexType);
    //vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _queryPool, 0);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptorSets[0], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, num_indices, instance_count, 0, 0, instance_offset);
    //vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _queryPool, 1);
}

void VulkanRenderer::recordCustomCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkBuffer vertexBuffer, VkBuffer indexBuffer, int instance_count,
        VkPipeline pipeline, VkPipelineLayout pipeline_layout, std::vector<VkDescriptorSet> descriptorSets) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkResetCommandBuffer(commandBuffer, 0);

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording custom command buffer!");
    }

    vkCmdResetQueryPool(commandBuffer, _queryPool, 0, 2);



    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _extent;
    std::array<VkClearValue,2> clearValues{};
    clearValues[0].color = {{0.1f, 0.0f, 0.4f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

#ifndef NOT_ONLY_CLEAR
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _queryPool, 0);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptorSets[0], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), instance_count, 0, 0, 0);
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _queryPool, 1);

#endif
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

size_t VulkanRenderer::getNumberOfSwapChainImages() {
    return _swapChainImages.size();
}

shaderc_shader_kind to_shaderc_kind(VulkanShaderType shaderType) {
    switch (shaderType) {
        case VulkanShaderType::Vertex:
            return shaderc_glsl_vertex_shader;
        case VulkanShaderType::Fragment:
            return shaderc_glsl_fragment_shader;
        case VulkanShaderType::Geometry:
            return shaderc_glsl_geometry_shader;
        case VulkanShaderType::Compute:
            return shaderc_glsl_compute_shader;
        case VulkanShaderType::TessControl:
            return shaderc_glsl_tess_control_shader;
        default: return {};
    }
}

std::vector<uint32_t> VulkanRenderer::compileShader(const std::string& source, VulkanShaderType shaderType) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    auto preprocess_result = compiler.PreprocessGlsl(source, to_shaderc_kind(shaderType), "vs", options);
    if (preprocess_result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cout << "preprocess error" << preprocess_result.GetErrorMessage() << std::endl;
        exit(1);
    }

    const std::string prep_code(preprocess_result.begin(), preprocess_result.end());

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(prep_code, to_shaderc_kind(shaderType), "vs", options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cout << "shader compilation error: " << std::endl <<result.GetErrorMessage() << std::endl;
        exit(1);
    }

    std::vector spirv(result.cbegin(), result.cend());
    std::cout << "SPIR-V magic: 0x" << std::hex << spirv[0] << std::dec << std::endl;
    if (spirv.size() == 0 || spirv.size()* sizeof(uint32_t) %4 != 0) {
        std::cerr << "spirv size mismatch" << std::endl;
        exit(1);
    }

    return spirv;
}


bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
    printf("device name: %s\n", physicalDeviceProperties.deviceName);
    printf("max clip distances: %u\n", physicalDeviceProperties.limits.maxClipDistances);
    auto minorVersion = VK_API_VERSION_MINOR(physicalDeviceProperties.apiVersion);
    printf("minor version: %u\n", minorVersion);

    vkGetPhysicalDeviceFeatures(device, &physicalDeviceFeatures);

    return (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        && findQueueFamilies(device).isComplete() && checkDeviceExtensionSupport(device);
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    // TODO add checking for e.g. swap chain support here
    return true;
}
namespace renderer {

    void drawMesh(renderer::Mesh m, const std::string& debugInfo ) {
        vulkan_renderer->drawFrameExp();
    }

    VkIndexType getIndexType(const IndexBufferDesc& ibd) {
        switch (ibd.format) {
            case 5123: return VK_INDEX_TYPE_UINT16;
            case 5125: return VK_INDEX_TYPE_UINT32;
            default: return VK_INDEX_TYPE_UINT32;
        }

    }

    IndexBufferHandle createIndexBufferVulkan(const IndexBufferDesc& ibd) {
        auto ib = get_vulkan_renderer()->createIndexBuffer(ibd);
        IndexBufferHandle ibh = { nextIBHandleId++};
        indexBufferMap[ibh.id] = VulkanIndexBuffer{ib, getIndexType(ibd)};
        return ibh;
    }

    VertexBufferHandle createVertexBufferVulkan(VertexBufferCreateInfo create_info) {
        auto vb = get_vulkan_renderer()->createVertexBuffer(create_info);
        VertexBufferHandle vbh = { nextVBHandleId++};
        vertexBufferMap[vbh.id] = VulkanVertexBuffer{vb};
        return vbh;
    }

    Mesh createMeshVulkan(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> & attributes, size_t index_count) {
        Mesh mesh;
        mesh.index_count = index_count;
        mesh.index_buffer = ibo;
        mesh.vertex_buffer = vbo;
        return mesh;
    }

    Mesh importMeshVulkan(const std::string& filename) {
        return parseGLTF(filename);
    }

    TextureHandle createTextureVulkan(const Image &srcImage, TextureFormat textureFormat) {
        auto vk_format = VK_FORMAT_R8G8B8A8_UNORM;
        if (textureFormat == TextureFormat::R8) {
            vk_format = VK_FORMAT_R8_UNORM;
        }
        auto image = get_vulkan_renderer()->createTextureImage(srcImage, vk_format, srcImage.width * srcImage.height);
        auto image_view = get_vulkan_renderer()->createTextureImageView(image, vk_format);
        TextureHandle th = { nextTextureHandleId++};
        textureMap[th.id] = VulkanTexture{image_view};
        return th;
    }




    ShaderHandle renderer::compileVertexShader(const std::string& source) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        // options.SetTargetSpirv(shaderc_spirv_version_1_3);
        // options.SetSourceLanguage(shaderc_source_language_glsl);
        // options.SetOptimizationLevel(shaderc_optimization_level_zero); // or desired level
        // options.SetGenerateDebugInfo();
        // options.SetWarningsAsErrors();

        auto preprocess_result = compiler.PreprocessGlsl(source, shaderc_glsl_vertex_shader, "vs", options);
        if (preprocess_result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cout << "preprocess error" << preprocess_result.GetErrorMessage() << std::endl;
            exit(1);
        }

        const std::string prep_code(preprocess_result.begin(), preprocess_result.end());

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(prep_code, shaderc_shader_kind::shaderc_glsl_vertex_shader, "", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cout << "shader compilation error: " << std::endl <<result.GetErrorMessage() << std::endl;
            exit(1);
        }
        std::vector<uint32_t> spirv(result.cbegin(), result.cend());
        std::cout << "SPIR-V magic: 0x" << std::hex << spirv[0] << std::dec << std::endl;
        if (spirv.size() == 0 || spirv.size()* sizeof(uint32_t) %4 != 0) {
            std::cerr << "spirv size mismatch" << std::endl;
            exit(1);
        }

#define skip_module_creation
#ifndef skip_module_creation

    auto module = vulkan_renderer->createShaderModule(spirv);

#endif

        ShaderHandle handle = ShaderHandle {nextHandleId};
        shaderMap[nextHandleId] = VulkanShader {spirv, {}};
        nextHandleId++;
        return handle;
    }

    ShaderHandle renderer::compileFragmentShader(const std::string& source) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetTargetSpirv(shaderc_spirv_version_1_3);
        options.SetSourceLanguage(shaderc_source_language_glsl);
        options.SetOptimizationLevel(shaderc_optimization_level_zero); // or desired level
        options.SetGenerateDebugInfo();
        options.SetWarningsAsErrors();

        auto preprocess_result = compiler.PreprocessGlsl(source, shaderc_glsl_fragment_shader, "fs", options);
        if (preprocess_result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cout << "preprocess error" << preprocess_result.GetErrorMessage() << std::endl;
            exit(123121);
        }

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, shaderc_shader_kind::shaderc_glsl_fragment_shader, "fs", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cout << "shader compilation error: " << std::endl <<result.GetErrorMessage() << std::endl;
            //handle errors
            exit(123123);
        }
        std::vector<uint32_t> spirv(result.cbegin(), result.cend());
        if (spirv.size() == 0 || spirv.size()* sizeof(uint32_t) %4 != 0) {
            std::cerr << "spirv size mismatch" << std::endl;
            exit(1);
        }

        ShaderHandle handle = ShaderHandle {nextHandleId};
        shaderMap[nextHandleId] = VulkanShader {spirv, {}} ;
        nextHandleId++;
        return handle;
    }


    void renderer::bindTexture(TextureHandle texture) {

    }

    void renderer::present(HDC hdc) {

    }

    void renderer::clear() {

    }

    void renderer::setClearColor(float r, float g, float b, float a) {

    }


    ProgramHandle linkShaderProgram(ShaderHandle vertexShader, ShaderHandle fragmentShader)
    {
        return {};
    }

    void*  getVertexBufferForHandleVulkan(VertexBufferHandle vbh) {
        return vertexBufferMap[vbh.id].buffer;
    }

    void*  getIndexBufferForHandleVulkan(IndexBufferHandle ibh) {
        return indexBufferMap[ibh.id].buffer;
    }

    void* getNativeIndexBufferStructForHandleVulkan(IndexBufferHandle ibh) {
        return &indexBufferMap[ibh.id];
    }

    void* getTextureForHandleVulkan(TextureHandle th) {
        return textureMap[th.id].textureImageView;
    }

    Mesh drawTextIntoQuadVulkan(FontHandle fontHandle, const std::string& text) {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> indices;

        drawTextIntoQuadGeometry(fontHandle, text, positions, uvs, indices);


        std::vector<Pos3VertexUV> vertices;
        for (int i = 0; i < positions.size(); i++) {
            vertices.push_back({positions[i], uvs[i]});

        }

        VertexBufferCreateInfo vbci;
        for (auto v : vertices) {
            vbci.data.push_back(v.pos.x);
            vbci.data.push_back(v.pos.y);
            vbci.data.push_back(v.pos.z);
            vbci.data.push_back(v.uv.x);
            vbci.data.push_back(v.uv.y);
        }

        auto vbo = get_vulkan_renderer()->createVertexBuffer(vbci);
        VertexBufferHandle vbh = {nextVBHandleId++};
        vertexBufferMap[vbh.id] = VulkanVertexBuffer{vbo};

        IndexBufferDesc ibd{};
        ibd.data = indices.data();
        ibd.size_in_bytes = indices.size() * sizeof(uint32_t);
        auto ibo = get_vulkan_renderer()->createIndexBuffer(ibd);
        IndexBufferHandle ibh = {nextIBHandleId++};
        indexBufferMap[ibh.id] = VulkanIndexBuffer{ibo};


        Mesh mesh;
        mesh.index_count = indices.size();
        mesh.index_buffer = ibh;
        mesh.vertex_buffer = vbh;

        return mesh;
    }


    template<>
    bool setShaderValue<glm::vec2>(ProgramHandle program, const std::string& name, const glm::vec2& value) {

        return false;
    }



    template<>
    bool setShaderValue<float>(ProgramHandle program, const std::string& name, const float& value) {
       return false;
    }

    template<>
    bool setShaderValue<glm::mat4>(ProgramHandle program, const std::string& name, const glm::mat4& value) {
       return false;
    }

    template bool setShaderValue<glm::mat4>(ProgramHandle, const std::string&, const glm::mat4&);
    template bool setShaderValue<float>(ProgramHandle, const std::string&, const float&);
    template bool setShaderValue<glm::vec2>(ProgramHandle, const std::string&, const glm::vec2&);


}
VulkanRenderer* get_vulkan_renderer() {
    assert(vulkan_renderer != nullptr);
    return vulkan_renderer;
}

void init_vulkan(HWND hwnd, HINSTANCE hinst, bool useSRGB, int msaaSampleCount) {
    if (!vulkan_renderer) {
        vulkan_renderer = new VulkanRenderer(hinst, hwnd);
    }

    renderer::registerCreateIndexBuffer(&renderer::createIndexBufferVulkan);
    renderer::registerDrawTextIntoQuad(&renderer::drawTextIntoQuadVulkan);
    renderer::registerGetVertexBufferForHandle(&renderer::getVertexBufferForHandleVulkan);
    renderer::registerGetIndexBufferForHandle(&renderer::getIndexBufferForHandleVulkan);
    renderer::registerGetTextureForHandle(&renderer::getTextureForHandleVulkan);
    renderer::registerCreateTexture(&renderer::createTextureVulkan);
    renderer::registerImportMesh(&renderer::importMeshVulkan);
    renderer::registerCreateVertexBuffer(&renderer::createVertexBufferVulkan);
    renderer::registerCreateMesh(&renderer::createMeshVulkan);
    renderer::registerGetNativeIndexBufferStructForHandle(&renderer::getNativeIndexBufferStructForHandleVulkan);

}
