#version 450

layout(location = 0) in vec2 inPos;
layout(location = 0) out vec2 outNDC;

layout(binding = 0) uniform UniformBufferObject {
    float angle;
};

void main() {
    // float c = cos(angle);
    // float s = sin(angle);
    // float x = inPos.x * c - inPos.y * s;
    // float y = inPos.x * s + inPos.y * c;
    // gl_Position = vec4(x, y, 0.0, 1.0);

    outNDC = inPos;

    gl_Position = vec4(inPos, 0.0, 1.0);
}
