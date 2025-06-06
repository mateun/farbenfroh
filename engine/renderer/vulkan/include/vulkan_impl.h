//
// Created by mgrus on 04.06.2025.
//

#ifndef VULKAN_IMPL_H
#define VULKAN_IMPL_H


#include <array>
#include <optional>
#include <vector>
#include <Windows.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <renderer.h>
#include <shaderc/shaderc.h>

enum class VulkanShaderType {
  Vertex,
  Fragment,
  Geometry,
  Compute,
  TessControl,
  TessEvaluation,
  RayGen,
  Miss,
  ClosestHit

};

shaderc_shader_kind to_shaderc_kind(VulkanShaderType shaderType);



template<typename T>
struct VulkanVertexDescriber {

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(T);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      return T::getAttributeDescriptions(); // delegate to T
    }


};

struct Pos2Vertex {
  glm::vec2 pos;

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    return {
              { 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Pos2Vertex, pos) },
    };

  }
};

struct Pos3Vertex {
  glm::vec3 pos;

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    return {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Pos3Vertex, pos) },
      };

  }
};

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

struct VulkanVertexBuffer {
  VkBuffer buffer;

};

struct VulkanIndexBuffer {
  VkBuffer buffer;
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};



class VulkanRenderer {

  public:
    VulkanRenderer(HINSTANCE hInstance, HWND window);
    ~VulkanRenderer();
    void clearBuffers();
    void drawFrameExp();

    VkCommandBuffer createCommandBuffer(int imageIndex);

    void createInstance();
    void createQueryPool();

    VkDescriptorSetLayout createDescriptorSetLayout(std::vector<std::tuple<uint32_t, VkDescriptorType>> bindingInfos);
    void createDescriptorSetLayout();
    void createDescriptorPool();

    // We need the ordered_buffers to hold VkBuffers as the underlying buffers per binding.
    // so if binding 0 is a UBO, we need a buffer for that, binding 1 may be an SSBO, we need the underlying SSBO buffer
    // at index 1.
    void createDescriptorSetsForLayout(VkDescriptorSetLayout layout, std::vector<VkBuffer> ordered_buffers);
    void createDescriptorSetsDefault();

    void updateDescriptorSets(uint32_t binding, VkBuffer buffer, VkDeviceSize size, VkDescriptorType type);

    bool createValidationLayers();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createFrameBuffers();
    void createDefaultTestGraphicsPipeline();
    void createRenderPass();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void copyBuffer(VkDeviceSize bufferSize, VkBuffer sourceBuffer, VkBuffer targetBuffer);
    void updateUniformBuffer(int current_image);
    void createVertexBuffer();

    VkBuffer createIndexBuffer(renderer::IndexBufferDesc ibd);

    void createIndexBuffer();
    void createUniformBuffers();
    void createCommandPool();
    void createSyncObjects();

    void drawFrame();
    void uploadData(VkDeviceMemory dstMemory, VkDeviceSize size, void *data);

    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    std::vector<uint32_t> compileShader(const std::string &source, VulkanShaderType shaderType);

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkShaderModule createShaderModule(std::vector<uint8_t> spirv);
    VkShaderModule createShaderModule(std::vector<uint32_t> spirv);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    template<typename T>
    VkBuffer createVertexBuffer(renderer::VertexBufferCreateInfo<T>);

    void recordCustomCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkBuffer vertexBuffer, VkBuffer indexBuffer, int instance_count);

    size_t getNumberOfSwapChainImages();

    VkPipeline createGraphicsPipeline(VkShaderModule vertexModule, VkShaderModule fragModule, std::vector<VkVertexInputAttributeDescription>
                                      attributeDescriptions, VkVertexInputBindingDescription bindingDescription);

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
    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _graphicsPipeline;
    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    VkBuffer _vertexBuffer;

    std::vector<VkCommandBuffer> _imageAvailableCommandBuffers;

    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;

    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    std::vector<void*> _uniformBuffersMapped;

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

VulkanRenderer* get_vulkan_renderer();
void init_vulkan(HWND, HINSTANCE, bool useSRGB = false, int msaaSampleCount = 0);

#endif //VULKAN_IMPL_H
