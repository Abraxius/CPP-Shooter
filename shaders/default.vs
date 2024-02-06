#version 460 core // OpenGL 4.6

// input (location matches vertex description)
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 col;
// output (location matches fragment shader "in")
layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec2 uvCoord;
layout (location = 3) out vec4 vertCol;
// uniforms (careful: uniform locations are shared with fragment shader) 
layout (location = 0) uniform mat4 modelMatrix;         // locations:  0,  1,  2,  3
layout (location = 4) uniform mat4 viewMatrix;          // locations:  4,  5,  6,  7
layout (location = 8) uniform mat4 perspectiveMatrix;   // locations:  8,  9, 10, 11
layout (location = 12) uniform mat3 normalMatrix;       // locations:  12, 13, 14, 15

void main() {
    // gl_Position is a predefined vertex shader output
    gl_Position = modelMatrix * vec4(pos, 1.0);
    worldPos = gl_Position.xyz;
    gl_Position = viewMatrix * gl_Position;
    gl_Position = perspectiveMatrix * gl_Position;

    normal = normalMatrix * norm; // we do not want to translate/scale the normal
    uvCoord = uv;
    vertCol = col;
}