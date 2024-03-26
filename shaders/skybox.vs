#version 460 core // OpenGL 4.6

layout (location = 0) in vec3 aPos;

// out vec3 TexCoords;

layout (location = 0) out vec3 TexCoords;

layout (location = 4) uniform mat4 view;
layout (location = 8) uniform mat4 projection;
// uniform mat4 projection;
// uniform mat4 view;

void main()
{
    TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}  