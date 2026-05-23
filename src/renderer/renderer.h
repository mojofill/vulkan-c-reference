#ifndef RENDERER_H
#define RENDERER_H

// Scope of file: initializes Vulkan + GLFW + screen related stuff

#include "vk_types.h"

#define MAX_FRAMES_IN_FLIGHT 2

static void initWindow(vk_context *vko);
static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
static void createInstance(vk_context *vko);
static void createSurface(vk_context *vko);
static void pickPhysicalDevice(vk_context *vko);
static void findQueueFamilies(vk_context *vko);
static void createLogicalDevice(vk_context *vko);
static void createRenderpass(vk_context *vko);
static void createSyncObjects(vk_context *vko);

// No uniforms to put in. Populate these when adding uniforms
// static void createCameraUniformBuffer(vk_context *vko) Rename this/create as many as necessary
// static void createDescriptorPool(vk_context *vko)
// static void createDescriptorSets(vk_context *vko)

// public functions
void drawFrame(vk_context *vko, uint32_t *currentFrame);
void cleanupRenderer(vk_context *vko);

#endif