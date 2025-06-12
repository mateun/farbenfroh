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

struct Pos3VertexUV {
  glm::vec3 pos;
  glm::vec2 uv;

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    return {
                { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Pos3VertexUV, pos) },
                { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Pos3VertexUV, uv) },
      };
  }
};

struct Pos3VertexUVNormal {
  glm::vec3 pos;
  glm::vec2 uv;
  glm::vec3 normal;

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    return {
          { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Pos3VertexUVNormal, pos) },
          { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Pos3VertexUVNormal, uv) },
          { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Pos3VertexUVNormal, normal) },
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
  VkIndexType indexType;
};

struct VulkanTexture {
  VkImageView textureImageView;
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



    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugUtilsMessengerEXT *pDebugMessenger);

    void createInstance();
    void setupDebugMessenger();
    void createQueryPool();

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void recordImgaeTransitionCommand(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
                                      VkImageLayout newLayout);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkDescriptorSetLayout createDescriptorSetLayout(std::vector<std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlags>> bindingInfos);
    void createDescriptorSetLayout();
    void createDescriptorPool();

    std::vector<VkDescriptorSet> createDescriptorSetsForLayout(VkDescriptorSetLayout layout,
      std::vector<std::tuple<uint32_t, VkBuffer, VkDeviceSize, VkDescriptorType>> binding_infos,
      std::vector<std::tuple<uint32_t, VkImageView, VkDeviceSize, VkDescriptorType, VkSampler>> image_infos);
    void createDescriptorSetsDefault();

    void updateDescriptorSetsForImages(VkDescriptorSet descriptorSet, uint32_t binding, VkImageView textureImageView,
                                       VkDeviceSize size, VkDescriptorType type, VkSampler textureSampler);

    void updateDescriptorSetsForBuffers(VkDescriptorSet descriptorSet, uint32_t binding, VkBuffer buffer, VkDeviceSize size, VkDescriptorType type);

    bool createValidationLayers();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();

    VkSampler createTextureSampler();

    void createImageViews();
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

    std::tuple<VkFramebuffer, renderer::Image, VkImageView, VkImage>createOffscreenFrameBuffer(int width, int height, VkFormat colorFormat, VkFormat depthFormat,
                                             VkRenderPass renderPass);

    void createFrameBuffers();
    void createDefaultTestGraphicsPipeline();

    VkRenderPass createCustomRenderPass(VkFormat colorFormat, VkFormat depthFormat);

    VkRenderPass getDefaultFrameBufferRenderPass();

    VkFramebuffer getDefaultFramebuffer(int index);

    void createRenderPass();


    uint32_t findMemoryType(const VkMemoryRequirements &memoryRequirements, VkPhysicalDeviceMemoryProperties memProperties, VkFlags properties);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void copyBuffer(VkDeviceSize bufferSize, VkBuffer sourceBuffer, VkBuffer targetBuffer);
    void updateUniformBuffer(int current_image);
    void createVertexBuffer();

    VkImageView createImageView(VkImage vk_image, VkFormat vk_format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

    void createDepthResources();

    VkBuffer createIndexBuffer(renderer::IndexBufferDesc ibd);

    void createIndexBuffer();
    void createUniformBuffers();
    void createCommandPool();
    void createSyncObjects();

    void drawFrame();

    void updateMappedMemory(void *mappedMemory, void *sourceData, VkDeviceSize size);

    void uploadData(VkDeviceMemory dstMemory, VkDeviceSize size, void *data);

    void createCommandBuffer();
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    std::vector<uint32_t> compileShader(const std::string &source, VulkanShaderType shaderType);

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkShaderModule createShaderModule(std::vector<uint8_t> spirv);
    VkShaderModule createShaderModule(std::vector<uint32_t> spirv);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void beginSecondaryCommandBuffer(VkCommandBuffer commandBuffer, VkRenderPass renderPass, int imageIndex, VkCommandBufferInheritanceInfo *
                                     inheritance_info);

    void beginFrameCommands(VkCommandBuffer commandBuffer);

    void endFrameCommands(VkCommandBuffer commandBuffer);

    void beginCustomRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer,
                               uint32_t width, uint32_t height);

    VkRenderPass beginRenderPass(VkCommandBuffer commandBuffer, int swapChainImageIndex);

    void endRenderPass(VkCommandBuffer commandBuffer);

    void recordSecondaryExecCommandBuffers(VkCommandBuffer primary, std::vector<VkCommandBuffer> secondaries);

    VkCommandBufferInheritanceInfo createInheritanceInfo(VkRenderPass renderPass, VkFramebuffer frameBuffer);

    void recordMultiMeshData(VkCommandBuffer commandBuffer, std::vector<VkBuffer> vertexBuffersParam,
                             std::vector<VkBuffer> indexBuffers, std::vector<VkIndexType> indexTypes,
                             VkPipelineLayout pipeline_layout, VkPipeline pipeline,
                             std::vector<std::vector<VkDescriptorSet>> descriptorSets, std::vector<int> instance_counts,
                             std::vector<int> instance_offsets, std::vector<uint32_t> num_indices);

    void recordMeshData(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer,
                        VkIndexType indexType, VkPipelineLayout pipeline_layout,
                        VkPipeline pipeline, std::vector<VkDescriptorSet> descriptorSets, int instance_count, int instance_offset, uint32_t
                        num_indices);

    VkBuffer createVertexBufferRaw(size_t size, void *data);

    VkBuffer createVertexBuffer(renderer::VertexBufferCreateInfo);

    void recordCustomCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkBuffer vertexBuffer, VkBuffer indexBuffer, int instance_count, VkPipeline, VkPipelineLayout,  std::vector<VkDescriptorSet>);

    size_t getNumberOfSwapChainImages();

    std::tuple<VkPipeline, VkPipelineLayout> createGraphicsPipeline(VkShaderModule vertexModule, VkShaderModule fragModule,
                                                                    std::vector<VkVertexInputAttributeDescription>
                                                                    attributeDescriptions, VkVertexInputBindingDescription bindingDescription,
                                                                    VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass);

    void executeCommandBuffers(std::vector<std::vector<VkCommandBuffer>> commandBuffers);

    void executeCommandBuffer(std::vector<VkCommandBuffer> command_buffers);

    VkImageView createTextureImageView(VkImage image, VkFormat format);

    VkImage createTextureImage(const renderer::Image &image, VkFormat format, VkDeviceSize imageSize);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void* mapMemory(VkDeviceMemory memory, int i, uint64_t size);

    void endSecondaryCommandBuffer(VkCommandBuffer vk_command_buffer);

    VkPipelineLayout getDefaultPipelineLayout();

    VkPipeline getDefaultPipeline();

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
    VkImageView _depthImageView;
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
    VkDebugUtilsMessengerEXT debugMessenger;

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
