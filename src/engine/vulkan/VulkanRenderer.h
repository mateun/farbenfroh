//
// Created by mgrus on 02.02.2025.
//

#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <optional>
#include <Windows.h>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class VulkanRenderer {

  public:

    VulkanRenderer(HINSTANCE hInstance, HWND window);
    ~VulkanRenderer();
    void clearBuffers();
    void drawFrame();

  private:
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createFrameBuffers();
    void createGraphicsPipeline();
    void createRenderPass();
    void createCommandPool();
    void createSyncObjects();


    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkShaderModule createShaderModule(uint8_t * vert_shader_code, uint32_t size);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    HINSTANCE _hInstance;
    HWND _window;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkSurfaceFormatKHR _surfaceFormat;
    VkSurfaceKHR _surface;
    VkExtent2D _extent;
    VkSwapchainKHR _swapChain;
    std::vector<VkImage> _swapChainImages;
    std::vector<VkImageView> _swapChainImageViews;
    VkRenderPass _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _graphicsPipeline;
    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;

    VkSemaphore _imageAvailableSemaphore;
    VkSemaphore _renderFinishedSemaphore;
    VkFence _inFlightFence;
};



#endif //VULKANRENDERER_H
