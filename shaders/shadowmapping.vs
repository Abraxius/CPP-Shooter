#version 460 core // OpenGL 4.6

// input (location matches vertex description)
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 col;
// output (location matches fragment shader "in")
layout (location = 0) out vec3 worldPos;
// uniforms (careful: uniform locations are shared with fragment/geometry shader) 
layout (location = 0) uniform mat4 modelMatrix;
layout (location = 4) uniform mat4 viewMatrix;
layout (location = 8) uniform mat4 perspectiveMatrix;
layout (location = 12) uniform mat3 normalMatrix;

void main() {
    gl_Position = modelMatrix * vec4(pos, 1.0);
    worldPos = gl_Position.xyz;
    gl_Position = viewMatrix * gl_Position;
    gl_Position = perspectiveMatrix * gl_Position;
}