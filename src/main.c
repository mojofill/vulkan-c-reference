#include "renderer/renderer.h"

void mainLoop(vk_context *vko) {
    uint32_t currentFrame = 0;

    while (!glfwWindowShouldClose(vko->window)) {
        glfwPollEvents();
        if (glfwGetKey(vko->window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(vko->window, VK_TRUE);
        }
        drawFrame(vko, &currentFrame);
    }
}

void initializeTestVertexBuffer(vk_context *vko) {
    // staging
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    // recall that order of vertices is important as well. need to make sure right hand rule works
    Vertex vertices[] = {
        { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } }, // top center, red
        { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },  // bottom right, green
        { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }  // bottom left, blue
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

    // in the future, i should make a large parameter struct that holds all the
    // different arguments that i want to pass in
    // for example, i want to be able to directly call here that i want to use
    // topology points
    // but this is good enough for now!

    initRenderer(&vko);
    initializeTestVertexBuffer(&vko);
    mainLoop(&vko);
    cleanupRenderer(&vko);
    return 0;
}
