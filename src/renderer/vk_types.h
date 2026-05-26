#ifndef VK_TYPES_H
#define VK_TYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#define GLFW_INCLUDE_NONE
// for protection against vulkan
#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 2
#define UNIFORM_BUFFER_COUNT MAX_FRAMES_IN_FLIGHT

typedef struct Vertex {
    float pos[2];
    float color[3];
} Vertex;

typedef struct vk_context {
    // GLFW + Vulkan initialization
    GLFWwindow *window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    uint32_t queueFamilyCount;
    uint32_t graphicsFamilyIndex;
    uint32_t presentFamilyIndex;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    // Swapchain and swapchain support
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkSwapchainKHR swapchain;
    uint32_t swapchainImageCount;
    VkImage *swapchainImages;
    VkImageView *swapchainImageViews;

    // Vertex buffer information
    VkVertexInputBindingDescription bindingDesc; // works for now, in the future when using more types of vertices, need a more robust system
    VkVertexInputAttributeDescription attrDescs[2]; // come back here when adding more attributes to a vertex
    VkBuffer testVertexBuffer;
    VkDeviceMemory testVertexBufferMemory;

    // Render pass + graphics pipeline
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer *swapchainFramebuffers;

    // Descriptors (uniforms)
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[UNIFORM_BUFFER_COUNT];

    // just for testing!
    float angle;

    // Uniform buffer
    VkBuffer uniformBuffers[UNIFORM_BUFFER_COUNT];
    VkDeviceMemory uniformBufferMemories[UNIFORM_BUFFER_COUNT];
    void *uniformBuffersMapped[UNIFORM_BUFFER_COUNT];

    // Dynamic pipeline bindings
    VkViewport viewport;
    VkRect2D scissor;

    // Synchronization objects
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    uint8_t framebufferResized;

    // Commands
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
} vk_context;

#endif