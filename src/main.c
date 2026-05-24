#include "renderer/renderer.h"

void mainLoop(vk_context *vko) {
    uint32_t currentFrame = 0;

    while (!glfwWindowShouldClose(vko->window)) {
        glfwPollEvents();
        drawFrame(vko, &currentFrame);
    }
}

int main() {
    vk_context vko = {0};
    printf("initializing renderer...\n");

    // in the future, i should make a large parameter struct that holds all the
    // different arguments that i want to pass in
    // for example, i want to be able to directly call here that i want to use
    // topology points
    // but this is good enough for now!

    initRenderer(&vko);
    mainLoop(&vko);
    cleanupRenderer(&vko);
    return 0;
}
