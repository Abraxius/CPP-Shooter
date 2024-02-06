#version 460 core // OpenGL 4.6

// input (location matches vertex shader "out")
layout (location = 0) in vec3 worldPos;

struct Camera {
    vec3 worldPos; // 16
};
struct Material {
    vec3 ambient; // 17
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float shininessStrength;
    float diffuseBlend;
};
struct Light {
    vec3 worldPos; // 23
    vec3 color;
    float radius;
};
// uniform constants
layout (location = 16) uniform Camera camera;
layout (location = 17) uniform Material material;
layout (location = 23) uniform Light light;

void main() {
    // do not write any color output
    float lightDistance = length(worldPos - light.worldPos);
    lightDistance = lightDistance / light.radius;
    gl_FragDepth = lightDistance;
}