#ifndef VK_TYPES_H
#define VK_TYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#define GLFW_INCLUDE_NONE
// for protection against vulkan
#include <GLFW/glfw3.h>

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