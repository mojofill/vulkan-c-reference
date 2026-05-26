#include "descriptors.h"
#include "buffer.h"

void createDescriptorSetLayout(vk_context *vko) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = &uboLayoutBinding;
    layoutInfo.bindingCount = 1;

    if (vkCreateDescriptorSetLayout(vko->device, &layoutInfo, NULL, &vko->descriptorSetLayout) != VK_SUCCESS) {
        printf("Failed to create descriptor set layout\n");
        exit(1);
    }
}

void createUniformBuffer(vk_context *vko) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    // create one for each image in flight (ie uniform buffer count)
    for (int i = 0; i < UNIFORM_BUFFER_COUNT; i++) {
        createBuffer(vko, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vko->uniformBuffers[i], &vko->uniformBufferMemories[i]);
        vkMapMemory(vko->device, vko->uniformBufferMemories[i], 0, bufferSize, 0, &vko->uniformBuffersMapped[i]);
    }
}

void createDescriptorPool(vk_context *vko) {
    VkDescriptorPoolSize poolSizes[1]; // adjust for however many uniforms you want
    // only one singular uniform buffer object needs to be allocated
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = (uint32_t) MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(vko->device, &poolInfo, NULL, &vko->descriptorPool) != VK_SUCCESS) {
        printf("failed to create descriptor pool\n");
        exit(1);
    }
}

void createDescriptorSets(vk_context *vko) {
    VkDescriptorSetAllocateInfo allocInfo = {0};

    // need identical layouts for each MAX_FRAMES_IN_FLIGHT descriptor set
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {0};
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = vko->descriptorSetLayout;

    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vko->descriptorPool;
    allocInfo.descriptorSetCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(vko->device, &allocInfo, vko->descriptorSets) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor sets\n");
        exit(1);
    }

    // populate descriptors
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // add buffer to descriptor set
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = vko->uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // write to descriptor set
        VkWriteDescriptorSet descriptorWrites[1] = {0}; // can add more descriptor sets if necessary
        // such as samplers or storage buffers

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = vko->descriptorSets[i]; // write to this descriptor set
        descriptorWrites[0].dstBinding = 0; // write to this binding (binding = 0)
        descriptorWrites[0].dstArrayElement = 0; // can have descriptor arrays, but we are not, so put first; ie index=0
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1; // only updating one descriptor
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(vko->device, 1, descriptorWrites, 0, NULL);
    }
}
