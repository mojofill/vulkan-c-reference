#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObject {
    float angle;
};

void main() {
    fragColor = inColor;
    float c = cos(angle);
    float s = sin(angle);
    float x = inPos.x * c - inPos.y * s;
    float y = inPos.x * s + inPos.y * c;
    gl_Position = vec4(x, y, 0.0, 1.0);
}
