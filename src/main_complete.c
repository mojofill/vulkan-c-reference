#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

VkShaderModule load_shader(VkDevice device, const char* path) {
    FILE *f;
    long len;
    char *buffer;

    f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "Failed to create shader from path at %s\n", path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    assert(len > 0);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(len); // size of char = 1
    assert(buffer != NULL);
    fread(buffer, 1, len, f);
    fclose(f);

    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = len;
    createInfo.pCode = (uint32_t*)buffer; // pointers can be casted to anything, bc at the end of the day its all just bytes. also 32bits = 4 bytes. this type casting just splits the 1 byte char array into 4 byte 32-bit words

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        fprintf(stderr, "Failed to load shader module at path %s\n", path);
        exit(1);
    }

    free(buffer);
    return shaderModule;
}

// finds searches GPU memory types for one that is host visible + coherent
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties; // after getting memory *requirements* now its time to see if our physical device can meet those requirements
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && // checks if gpu allows the given memory type
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) { // checks if this memory type on the gpu has all the properties you want
            return i; // this memory type is good, this is all i need
        }
    }

    printf("Failed to find suitable memory type!\n");
    exit(1);
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no opengl

    GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan C Reference Windows", NULL, NULL);

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        return 1;
    }

    // 1st step: vulkan app info
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan C", // p stands for pointer. this means char pointer
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2
    };

    // 2nd step: glfw extensions
    uint32_t glfwExtensionCount;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    // for now, see if i dont need the portability extension. that might be unique to mac

    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions, // pointer to pointer = array of strings
        // on macos, i would need .flags as well for KHR_PORTABILITY
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = layers
    };

    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance\n");
        return 1;
    }

    // 3rd step: create surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) { // glfw connects vk graphics to os window rendering
        fprintf(stderr, "Failed to create surface\n");
        return 1;
    }

    // 4th step: enumerate physical devices
    uint32_t physicalDeviceCount = 0;

    vkEnumeratePhysicalDevices(vko->instance, &physicalDeviceCount, NULL);

    if (physicalDeviceCount == 0) {
        fprintf(stderr, "No Vulkan-capable GPUs found.\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *physicalDevices =
        malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

    vkEnumeratePhysicalDevices(vko->instance, &physicalDeviceCount, physicalDevices);

    // printf("Found %u graphics device(s):\n", physicalDeviceCount);

    int bestDevice = -1;
    int bestScore = -1;

    for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

        int score = 0;
        // printf("  [%u] %s\n", i, properties.deviceName);

        switch (properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 1000;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 500;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                score += 300;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                score += 100;
                break;

            default:
                break;
        }

        // Prefer newer Vulkan versions.
        score += properties.apiVersion;

        if (score > bestScore) {
            bestScore = score;
            bestDevice = i;
        }
    }

    if (bestDevice == -1) {
        fprintf(stderr, "Failed to select a physical device.\n");
        free(physicalDevices);
        exit(EXIT_FAILURE);
    }

    vko->physicalDevice = physicalDevices[bestDevice];

    VkPhysicalDeviceProperties selectedProperties;
    vkGetPhysicalDeviceProperties(vko->physicalDevice, &selectedProperties);

    printf("Selected GPU: %s\n", selectedProperties.deviceName);

    free(physicalDevices);

    // 5th step: query queue families (properties of the physical device, the actual gpu hardware)
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    if (queueFamilyCount == 0) {
        fprintf(stderr, "No queue families found. This should be IMPOSSIBLE but i'll add this anyways\n");
        return 1;
    }
    VkQueueFamilyProperties *queueFamilyProperties = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);

    // gpu = big company that has multiple departments (families)
    // each queue (employee) in a family (department) must follow their rules (ie a tranfer only family can only transfer memory, cannot compute)
    
    // for now, i just need the graphics family and the present family
    int graphicsFamily = -1;
    int presentFamily = -1;

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }

        VkBool32 presentSupport; // bools are 32 bit unsigned ints for cross platform purposes - different os have different byte sizes for bools
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) {
            presentFamily = i;
        }

        if (graphicsFamily != -1 && presentFamily != -1) break;
    }

    if (graphicsFamily == -1 || presentFamily == -1) {
        fprintf(stderr, "Failed to find required queue families\n");
        return 1;
    }

    free(queueFamilyProperties);

    // 6th step: create logical device
    // physical device = gpu, logical device = VK interpretation of gpu, interface between software and hardware
    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // need to add swapchain extension for the ping ponging

    // create device queue
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfos[2] = {0};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = graphicsFamily; // just so happens same index for both graphics and present queue family
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &queuePriority; // since theres only one queue, just set this one to 1.0f

    // create logical device
    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1; // only one queue was created
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.enabledExtensionCount = 1; // one swapchain extension
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device\n");
        return 1;
    }

    // right after i create a logical device, i should create a "logical queue" as well - again, im creating an interface between hardware and software
    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);

    // as well as surface capabilities
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    // pick surface format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
    VkSurfaceFormatKHR *formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats);

    VkSurfaceFormatKHR surfaceFormat = formats[0];

    free(formats);
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    VkSurfacePresentModeKHR *presentModes = malloc(presentModeCount * sizeof(VkSurfacePresentModeKHR));
    
    // pick mailbox if available, otherwise fifo (always available)
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (presentModes[i].presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    free(presentModes);

    // create swapchain
    VkSwapchainCreateInfoKHR swapchainInfo = {0};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.imageExtent = surfaceCapabilities.currentExtent; // current window size
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.minImageCount = surfaceCapabilities.minImageCount + 1; // one extra buffer - triple rendering
    if (surfaceCapabilities.maxImageCount > 0 && swapchainInfo.minImageCount > surfaceCapabilities.maxImageCount) {
        // if it doesnt allow triple rendering, ie our supposed minImageCount is greater than the maximum, then ya cant
        swapchainInfo.minImageCount = surfaceCapabilities.maxImageCount;
    }
    swapchainInfo.imageArrayLayers = 1; // just need one layer per image
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // these swapchain images are ONLY used for rendering images, nothing else
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // images cannot be accessed concurrently
    swapchainInfo.preTransform = surfaceCapabilities.currentTransform; // some bullshit, this only matters for phones when they are rotated
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // the WINDOW itself is opaque, not the image itself.
    swapchainInfo.clipped = VK_TRUE; // pixels outside of current window size are not rendered
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;
    vkCreateSwapchainKHR(device, &swapchainInfo, NULL, &swapchain);

    // get swapchain images, then swapchain image view
    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
    VkImage *swapchainImages = malloc(sizeof(VkImage) * swapchainImageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);

    // set image views
    VkImageView *swapchainImageViews = malloc(sizeof(VkImageView) * swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; i++) {
        VkImageViewCreateInfo viewInfo = {0};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // of course all swapchain images are 2D
        viewInfo.format = surfaceFormat.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // rgba -> rgba
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseMipLevel = 0;

        if (vkCreateImageView(device, &viewInfo, NULL, &swapchainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create swapchain image view %d\n", i);
            return 1;
        }
    }

    // time to create a simple render pass
    // what is a render pass?

    // a color attachment is exactly the image. attachment description = image description = image metadata

    VkAttachmentDescription colorAttachment = {0}; // color attachment = the actual image on the swapchain
    colorAttachment.format = surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // multisample anti-aliasing (more samples = less edges. for now dont need anti-aliasing)
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // op = operation. clear = clear to some color. load = keep previous contents (would need this for images that persist over frames). dont care = previous contents are garbage, just leave it idc
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // operation store the previous frame's data
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // dont care about stencils bc simple graphics rn. a stencil is a region of area that is kept, everything else discarded
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // undefined = vulkan discards previous image layout data. works with op clear for loadOp
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // after rendering finishes, this image will be used for presentation
    // image layout = description for how the GPU can access the image *in memory*

    // color attachment = there exists *one* image that is for rendering. currently, we havent described *which* image yet. so far we just have 3 images, and defined one color attachment to make one of them a renderer

    VkAttachmentReference colorAttachmentRef = {0}; // this is how a subpass gets access to the attachment.
    colorAttachmentRef.attachment = 0; // pAttachments[0] - there is an array of attachments, our attachment index is 0 b/c we only have one attachment right now
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // create the subpass. right now, we just have one subpass. this is the vertex -> fragment stuff
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // running GRAPHICS pipeline - not a compute pipeline
    subpass.colorAttachmentCount = 1; // one SINGULAR color output, which is attachment #0. attachment = "logical" i
    subpass.pColorAttachments = &colorAttachmentRef; // array decay, for single length array just pointer to first

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create render pass\n");
        return 1;
    }

    // finally, must create framebuffer (links image (from swapchain) to color attachments (in the render pass))

    VkFramebuffer* swapchainFramebuffers = malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    
    for (uint32_t i = 0; i < swapchainImageCount; i++) {
        VkImageView attachments[] = { swapchainImageViews[i] }; // try manual array decay after
        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        // framebufferInfo.pAttachments = attachments;
        framebufferInfo.pAttachments = &swapchainImageViews[i];
        framebufferInfo.width = surfaceCapabilities.currentExtent.width;
        framebufferInfo.height = surfaceCapabilities.currentExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapchainFramebuffers[i]) != VK_SUCCESS) {
            printf("Failed to create framebuffer %d!\n", i);
            return 1;
        }
    }

    // correct mental model: render pass = PLAN for rendering. is a sequence of subpasses (directed graph, are ordered sequentially)
    // attachments = logical images in the render pass. think placeholders/variables.
    // subpass = one single rendering phase. render pass = sequence/combination of subpasses
    // framebuffer = actually putting the swapchain images into the render pass PLAN
    // swapchain = where the *presentable* images all live. each image has its own framebuffer, and each image uses the same render pass (or we can create more plans)

    typedef struct {
        float pos[2];
        float color[3];
    } Vertex;

    Vertex vertices[] = {
        { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } }, // bottom center, red
        { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },  // top right, green
        { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }  // top left, blue
    };

    VkShaderModule vertShader = load_shader(device, "./src/spvs/vert.spv");
    VkShaderModule fragShader = load_shader(device, "./src/spvs/frag.spv");

    // pipeline shader stage setup
    VkPipelineShaderStageCreateInfo vertStage = {0};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertShader;
    vertStage.pName = "main"; // needs to be main - this is the void main() thing in the shader

    VkPipelineShaderStageCreateInfo fragStage = {0};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.module = fragShader;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertStage, fragStage};

    // add vertex attributes

    VkVertexInputBindingDescription bindingDesc = {0}; // this is buffer binding
    // only have one vertex right now, so just bind it to the first index 0. for more buffers bind to index=1 and so forth
    bindingDesc.binding = 0; // what does this mean? how many bindings do i have? is this like opengl where i first have to bind a vertex buffer to GL_ARRAY_BUFFER?
    bindingDesc.stride = sizeof(Vertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // add vertex attributes. this is attribute binding
    VkVertexInputAttributeDescription attrDesc[2];
    // vec2 inPos at location=0
    attrDesc[0].binding = 0; // uses vertex buffer at binding=0
    attrDesc[0].location = 0; // in vertex shader: location=0
    attrDesc[0].format = VK_FORMAT_R32G32_SFLOAT; // vec2
    attrDesc[0].offset = offsetof(Vertex, pos);

    // vec3 color at location=1
    attrDesc[1].binding = 0; // uses vertex buffer at binding=0
    attrDesc[1].location = 1;
    attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
    attrDesc[1].offset = offsetof(Vertex, color);

    // vertex input - this is how the vertex data gets into the pipeline
    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &bindingDesc;
    vertexInput.vertexAttributeDescriptionCount = 2;
    vertexInput.pVertexAttributeDescriptions = attrDesc;

    // upload vertex buffer data to gpu
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory; // gpu memory

    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // this buffer cannot be accessed by two different queue families at the same time. since we only have one queue family (present and graphics queue family same index) this does not matter much

    if (vkCreateBuffer(device, &bufferInfo, NULL, &vertexBuffer) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vertex buffer\n");
        return 1;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocVertexGPUMemInfo = {0};
    allocVertexGPUMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocVertexGPUMemInfo.allocationSize = memRequirements.size; // is this the same as sizeof(vertices)? no, because these are float aligned. mem requires is important for proper gpu memory allocation

    allocVertexGPUMemInfo.memoryTypeIndex = findMemoryType(
        physicalDevice,
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    if (vkAllocateMemory(device, &allocVertexGPUMemInfo, NULL, &vertexBufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate vertex buffer memory\n");
        return 1;
    }

    vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0); // bind specified memory for the vertex buffer to use

    void *data;
    vkMapMemory(device, vertexBufferMemory, 0, sizeof(vertices), 0, &data); // mapping cpu memory -> use sizeof(vertices)
    // prolly just creates a pointer in vulkan to data, so when i do shit to data like memcpy, the pointer in vulkan remembers it
    memcpy(data, vertices, sizeof(vertices));
    vkUnmapMemory(device, vertexBufferMemory);

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0}; // what exactly is an input assembly? input = vertex input?
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // this is like opengl draw triangles, triangle strip, etc
    inputAssembly.primitiveRestartEnable = VK_FALSE; // doesnt really matter, only matters for triangle strips

    VkViewport viewport = {0}; // viewports turn ndc coordinates into framebuffer/screen pxiels
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)surfaceCapabilities.currentExtent.width;
    viewport.height = (float)surfaceCapabilities.currentExtent.height;
    viewport.minDepth = 0.0f; // near plane
    viewport.maxDepth = 1.0f; // far plane (this only matters for 3d)

    // important to remember - vulkan uses 0..1 ndc coordinates, unlike opengl which uses -1..1

    // scissor = clips pixels/fragments to a rectangle. fragments outside of rectangle are discarded

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0,0};
    scissor.extent = surfaceCapabilities.currentExtent; // scissor size = current window size

    VkPipelineViewportStateCreateInfo viewportState = {0}; // adds viewport to the graphics pipeline
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport; // can make these dynamic by setting to NULL and use vkCmdSet...
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor; // can make these dynamic by setting to NULL and use vkCmdSet...

    // come back to rasterizer to experiment

    // rasterizer. this part i dont realy like - id rather have complete control over rasterization. however, since this is currently 2d, im fine with this
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // since only 2d right now, dont need this
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // no fragments are generated if rasterizer discard is enabled
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // generate fragments for every pixel inside triangle
    rasterizer.lineWidth = 1.0f; // this seems cool
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // see what happens if i flip the vertex coordainte order - if this works, then it should turn invisible
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // these two work together - front face should be clockwise - also vulkan y axis is flipped (like javascript canvas, *unlike* opengl)
    rasterizer.depthBiasEnable = VK_FALSE;

    // multisampling (anti aliasing)
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE; // no fragment shader per sample - vk false means fragment shader per pixel. off = faster, on = higher quality
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // only one sample

    // color blend = blending the rgba channels
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = // this just defines what colors fragment shader can write to
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; // this says fragment shader output overwrites framebuffer - what happens if it is on?

    // this just joins all the color blend attachments
    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // pipeline layout = defines how shaders access resources
    // this will come in useful for future experiments
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VkPipelineLayout pipelineLayout;
    vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout);

    // now finally, put everything together into a graphics pipeline object
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2; // vertex + fragment
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // ??? i think this is an index? this means it targets subpass 0
    // subpasses are only for render passes aye man these will make sense in the future. think subpasses either read or write to attachments in the swapchain

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline\n");
        VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline);
        printf("pipeline result = %d\n", res);
        return 1;
    }

    // almost done!! the last part is just command buffers
    
    // command pools = command buffer memory allocator, helps make efficient reallocations between command buffers
    // these are *per queue families* - ie each queue family that takes in specific types of commands has its own command pool/allocator
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsFamily; // same index for present as well

    VkCommandPool commandPool;
    vkCreateCommandPool(device, &poolInfo, NULL, &commandPool);

    // allocate command buffers
    VkCommandBuffer *commandBuffers = malloc(sizeof(VkCommandBuffer) * swapchainImageCount); // am i creating a command buffer for each swapchain image? im not really following here. i get the code, but why?

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // what is this? vulkan didnt do a good job explaining this, but lowkey this doesnt seem too important so i can leave this
    allocInfo.commandBufferCount = swapchainImageCount;

    vkAllocateCommandBuffers(device, &allocInfo, commandBuffers);

    // finally, record commands (per swapchain image)
    for (uint32_t i = 0; i < swapchainImageCount; i++) {
        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfoCmd = {0}; // most basic command is starting a render pass
        renderPassInfoCmd.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfoCmd.renderPass = renderPass;
        renderPassInfoCmd.framebuffer = swapchainFramebuffers[i];
        renderPassInfoCmd.renderArea.offset = (VkOffset2D){0,0};
        renderPassInfoCmd.renderArea.extent = surfaceCapabilities.currentExtent;

        // VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkClearValue clearColor = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfoCmd.clearValueCount = 1;
        renderPassInfoCmd.pClearValues = &clearColor;

        // render pass = the "plan" that says what it will do after it gets the image data. its currently working with a color attachment as a placeholder. needs pipeline to actually supply the data to the color attachment

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfoCmd, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        // draw triangle
        // must bind vertex buffer before draw (like opengl)
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, (VkDeviceSize[]){0});
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        vkEndCommandBuffer(commandBuffers[i]);
    }

    // semaphores are what keeps cpu and gpu synchronized. will wait until gpu is done before rendering
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // initially signal is on, this way we dont wait indefinitely

    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlightFence;

    if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailable) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinished) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, NULL, &inFlightFence)) {
        printf("Failed to create semaphores & fence!\n");
        return 1;
    }

    // important mental concept: i have to record all the commands before rendering starts. while rendering in render loop, i can only execute/submit commands. i cannot make more commands or delete commands.

    // finally!! after 600 lines!! im at the render loop!!!
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        // vkResetFences(device, 1, &inFlightFence);

        // 1. acquire next swapchain image
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

        // semaphore *signal when gpu has reached a point*. it can be any point
        // in this case, we are asking for the next image available to render on the swapchain

        if (result != VK_SUCCESS) {
            fprintf(stderr, "Failed to submit draw command buffer\n");
            continue; // try next frame
        }

        // submit command buffer for rendering
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo = {0};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailable; // wait until image is ready
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex]; // dont i need to clear this first? or will it just rewrite over it?
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinished; // signal when rendering is done

        // can add fences in here instead of null handle
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            printf("Failed to submit draw command buffer!\n");
            continue;
        }

        // 3. Present the rendered image
        VkPresentInfoKHR presentInfo = {0};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinished;      // wait until rendering finished
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = NULL;

        result = vkQueuePresentKHR(graphicsQueue, &presentInfo);
        if (result != VK_SUCCESS) {
            printf("Failed to present swapchain image!\n");
        }

        // Optional: wait for queue idle (simpler for learning)
        vkQueueWaitIdle(graphicsQueue);
    }

    vkDeviceWaitIdle(device);
    
    vkDestroySemaphore(device, imageAvailable, NULL);
    vkDestroySemaphore(device, renderFinished, NULL);
    vkDestroyFence(device, inFlightFence, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);
    vkDestroyShaderModule(device, vertShader, NULL);
    vkDestroyShaderModule(device, fragShader, NULL);
    vkDestroyPipeline(device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);
    for (int i = 0; i < swapchainImageCount; i++) {
        vkDestroyFramebuffer(device, swapchainFramebuffers[i], NULL);
        vkDestroyImageView(device, swapchainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroyBuffer(device, vertexBuffer, NULL);
    vkFreeMemory(device, vertexBufferMemory, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    free(glfwExtensions);
    free(swapchainImages);
    free(swapchainImageViews);

    return 0;
}
