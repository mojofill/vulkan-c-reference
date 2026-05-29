#include "descriptors.h"
#include "buffer.h"

void createDescriptorSetLayout(vk_context *vko) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding imageLayoutBinding = {0};
    imageLayoutBinding.binding = 1;
    imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageLayoutBinding.descriptorCount = 1;
    imageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[2] = { uboLayoutBinding, imageLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = bindings;
    layoutInfo.bindingCount = 2;

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
    VkDescriptorPoolSize poolSizes[2]; // adjust for however many uniforms you want
    // only one singular uniform buffer object needs to be allocated
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = (uint32_t) MAX_FRAMES_IN_FLIGHT; 

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
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

        VkDescriptorImageInfo imageInfo = {0};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // doesnt matter what layout, so set it to be general layout
        imageInfo.imageView = vko->storageImageView;
        imageInfo.sampler = vko->storageSampler;

        // write to descriptor set
        VkWriteDescriptorSet descriptorWrites[2] = {0}; // can add more descriptor sets if necessary
        // such as samplers or storage buffers

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = vko->descriptorSets[i]; // write to this descriptor set
        descriptorWrites[0].dstBinding = 0; // write to this binding (binding = 0)
        descriptorWrites[0].dstArrayElement = 0; // can have descriptor arrays, but we are not, so put first; ie index=0
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1; // only updating one descriptor
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = vko->descriptorSets[i];
        descriptorWrites[1].dstBinding = 1; // binding = 1 in the shader
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(vko->device, 2, descriptorWrites, 0, NULL);
    }
}
