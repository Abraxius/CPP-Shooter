#version 460 core // OpenGL 4.6

// input (location matches vertex shader "out")
layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uvCoord;
layout (location = 3) in vec4 vertCol;
// output
layout (location = 0) out vec4 pixelColor;

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
struct Light { // assuming this is a point light
    vec3 worldPos; // 23
    vec3 color;
    float radius;
};
// uniform constants
layout (location = 16) uniform Camera camera;
layout (location = 17) uniform Material material;
#define N_LIGHTS 2
layout (location = 23) uniform Light lights[N_LIGHTS]; // 23, 26

// texture samplers
layout (binding = 0) uniform sampler2D diffuseTexture;
layout (binding = 1) uniform samplerCube shadowMaps[N_LIGHTS]; // binding 1, 2

// indirect scattered light
vec3 calc_ambient() {
    float ambientStrength = 1.0; // ambient modifier
    return ambientStrength * material.ambient + material.diffuse * 0.1;
}
// direct light
vec3 calc_diffuse(uint i) {
    vec3 lightDir = normalize(lights[i].worldPos - worldPos); // unit vector from light to fragment
    float diffuseStrength = dot(normal, lightDir); // calc intensity of light
    diffuseStrength = max(diffuseStrength, 0.0); // filter out negative intensity
    return lights[i].color * diffuseStrength * material.diffuse;
}
// direct light specular highlights
vec3 calc_specular(uint i) {
    vec3 lightDir = normalize(lights[i].worldPos - worldPos); // unit vector from light to fragment
    vec3 cameraDir = normalize(camera.worldPos - worldPos); // unit vector from camera to fragment
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = material.shininessStrength; // specular modifier
    specularStrength *= pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    return lights[i].color * specularStrength * material.specular;
}
float calc_shadow(uint i) {
    vec3 lightDir = normalize(lights[i].worldPos - worldPos); // unit vector from light to fragment
    vec3 fragToLight = worldPos - lights[i].worldPos;
    float bias_max = 1.0;
    float bias_min = 0.005;
    float bias = max((1.0 - dot(normal, lightDir) * bias_max), bias_min);  
    float currentDepth = length(fragToLight);
    
    // percentage closer filter
    float samples = 4.0;
    float offset  = 0.1;
    float shadow = 0.0;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5)) {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5)) {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5)) {
                float closestDepth = texture(shadowMaps[i], fragToLight + vec3(x, y, z)).r; 
                closestDepth *= lights[i].radius;
                if(currentDepth - bias < closestDepth) shadow += 1.0;
            }
        }
    }
    return shadow / (samples * samples * samples);
}
float calc_shadow_perf(uint i) {
    vec3 lightDir = normalize(lights[i].worldPos - worldPos); // unit vector from light to fragment
    vec3 fragToLight = worldPos - lights[i].worldPos;
    float bias_max = 1.0;
    float bias_min = 0.005;
    float bias = max((1.0 - dot(normal, lightDir) * bias_max), bias_min);  
    float currentDepth = length(fragToLight);

    float closestDepth = texture(shadowMaps[i], fragToLight).r; 
    closestDepth *= lights[i].radius;
    float shadow = 0.0;
    if(currentDepth - bias < closestDepth) shadow += 1.0;
    return shadow;
}
vec4 calc_light() {
    // calculate lighting
    vec3 ambientColor = calc_ambient();
    vec3 specularColor = vec3(0.0, 0.0, 0.0);
    vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
    for (uint i = 0; i < N_LIGHTS; i++) {
        float shadow = calc_shadow(i);

        vec3 lightDir = normalize(lights[i].worldPos - worldPos); // unit vector from light to fragment
        float lightDist = length(lightDir);
        float linear = 0.14;
        float quad = 0.07;
        float attenuation = 1.0 / (1.0 + lightDist * linear + lightDist * lightDist * quad);
        // attenuation = 1.0;
        diffuseColor += calc_diffuse(i) * shadow * attenuation;
        specularColor += calc_specular(i) * shadow * attenuation;
    }

    // blend texture/vertex colors
    vec4 sampledColor = texture(diffuseTexture, uvCoord);
    vec4 color = mix(vertCol, sampledColor, material.diffuseBlend);

    // blend sampled color with light
    return vec4(color.rgb * (ambientColor + diffuseColor + specularColor), color.a);
}
vec4 calc_debug() {
    vec3 fragToLight = worldPos - lights[0].worldPos;
    float closestDepth = texture(shadowMaps[0], fragToLight).r; // depth as seen from light
    closestDepth *= lights[0].radius;
    return vec4(vec3(closestDepth / lights[0].radius), 1.0);
}

void main() {
    pixelColor = calc_light();
    // pixelColor = calc_debug();
}