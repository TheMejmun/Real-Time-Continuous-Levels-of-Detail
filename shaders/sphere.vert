#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec3 inUVW;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragWorldPos;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragUVW;


void main() {
    vec4 posWS = ubo.model * vec4(inPosition, 1.0);
    vec4 posSS = ubo.proj * ubo.view * posWS;
    gl_Position = posSS;

    fragPos = posSS;
    fragWorldPos = posWS;
    fragColor = inColor;
    fragNormal = (transpose(inverse(ubo.model)) * vec4(inNormal, 1.0)).xyz;
    fragUVW = inUVW;
}