#include "renderer/renderer.h"

void updateUniforms(vk_context *vko, uint32_t currentFrame) {
    UniformBufferObject ubo = {0};
    vko->angle += 0.01;
    ubo.angle = vko->angle;
    memcpy(vko->uniformBuffersMapped[currentFrame], &ubo, sizeof(UniformBufferObject));
}

void mainLoop(vk_context *vko) {
    uint32_t currentFrame = 0;

    while (!glfwWindowShouldClose(vko->window)) {
        glfwPollEvents();
        if (glfwGetKey(vko->window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(vko->window, VK_TRUE);
        }
        updateUniforms(vko, currentFrame);
        drawFrame(vko, &currentFrame);
    }
}

void initializeTestVertexBuffer(vk_context *vko) {
    // staging
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    // recall that order of vertices is important as well. need to make sure right hand rule works
    // y is flipped, so -1 is top and 1 is bottom
    Vertex vertices[] = {     // don't need color
        { { -1.0f, -1.0f } }, // { 1.0f, 0.0f, 0.0f } }, // top left
        { {  1.0f,  1.0f } }, // { 0.0f, 1.0f, 0.0f } }, // bottom right
        { { -1.0f,  1.0f } }, // { 0.0f, 0.0f, 1.0f } }, // bottom left
        { {  1.0f,  1.0f } }, // { 1.0f, 0.0f, 0.0f } }, // bottom right
        { { -1.0f, -1.0f } }, // { 0.0f, 1.0f, 0.0f } }, // top left
        { {  1.0f, -1.0f } }, // { 0.0f, 0.0f, 1.0f } }  // top right
    };

    // staging buffer must be host visible and host coherent so it can be copied
    createBuffer(vko, sizeof(vertices), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingMemory);

    // copy data onto staging buffer
    void *data;
    vkMapMemory(vko->device, stagingMemory, 0, sizeof(vertices), 0, &data);
    memcpy(data, vertices, sizeof(vertices));
    vkUnmapMemory(vko->device, stagingMemory);

    // actual vertex buffer
    createBuffer(vko, sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    &vko->testVertexBuffer, &vko->testVertexBufferMemory);

    // copy from staging to test buffer
    copyBuffer(vko, stagingBuffer, vko->testVertexBuffer, sizeof(vertices));

    // destroy staging buffer
    vkDestroyBuffer(vko->device, stagingBuffer, NULL);
    vkFreeMemory(vko->device, stagingMemory, NULL);
}

int main() {
    vk_context vko = {0};

    // this will basically like a vulkan-c-reference, except with an extra compute shader + using a storage texture
    // using this: VK_DESCRIPTOR_TYPE_STORAGE_IMAGE

    initRenderer(&vko);
    initializeTestVertexBuffer(&vko);
    mainLoop(&vko);
    cleanupRenderer(&vko);
    return 0;
}
