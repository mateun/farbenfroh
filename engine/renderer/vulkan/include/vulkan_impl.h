//
// Created by mgrus on 04.06.2025.
//

#ifndef VULKAN_IMPL_H
#define VULKAN_IMPL_H


#include <array>
#include <optional>
#include <vector>
#include <Windows.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

struct PosColorVertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(PosColorVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(PosColorVertex, pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(PosColorVertex, color);

    return attributeDescriptions;
  }
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct VulkanShader {
  std::vector<uint32_t> spirv_code;
  VkShaderModule shader_module;


};

class VulkanRenderer {

  public:


    VulkanRenderer(HINSTANCE hInstance, HWND window);
    ~VulkanRenderer();
    void clearBuffers();
    void drawFrameExp();
    VkShaderModule createShaderModule(std::vector<uint8_t> spirv);
    VkShaderModule createShaderModule(std::vector<uint32_t> spirv);


    void createInstance();
    void createQueryPool();
    bool createValidationLayers();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createFrameBuffers();
    void createDefaultTestGraphicsPipeline();
    void createRenderPass();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);

    void copyBuffer(VkDeviceSize bufferSize, VkBuffer sourceBuffer, VkBuffer targetBuffer);

    void createVertexBuffer();
    void createIndexBuffer();
    void createCommandPool();
    void createSyncObjects();
    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
private:
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
    VkBuffer _vertexBuffer;

    std::vector<VkCommandBuffer> _imageAvailableCommandBuffers;

    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;

    VkSemaphore _imageAvailableSemaphore;
    VkSemaphore _renderFinishedSemaphore;
    VkFence _inFlightFence;
    VkQueryPool _queryPool;

    const std::vector<const char*> validationLayers = {
      "VK_LAYER_KHRONOS_validation"
    };

  const std::vector<PosColorVertex> vertices = {
    {{-1, -0.99f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, 0.999f}, {0.0f, 1.0f, 0.0f}},
    {{-1.0f, 0.999f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, -0.99f}, {0.0f, 1.0f, 0.0f}}
  };

  const std::vector<uint16_t> indices = {
    0, 1, 2, 0, 3,1
  };
};

void init_vulkan(HWND, HINSTANCE, bool useSRGB = false, int msaaSampleCount = 0);

#endif //VULKAN_IMPL_H
