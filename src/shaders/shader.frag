#version 450

layout(location = 0) in vec2 outNDC;
layout(location = 0) out vec4 outColor;
layout(binding = 1, rgba32f) uniform readonly image2D outputImage;

void main() {
    vec2 uv = outNDC * 0.5 + 0.5;
    ivec2 imgSize = imageSize(outputImage);
    ivec2 texCoord = ivec2(uv * vec2(imgSize));

    outColor = imageLoad(outputImage, texCoord);
}
