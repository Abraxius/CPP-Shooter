#version 460 core // OpenGL 4.6

out vec4 FragColor;

// in vec3 TexCoords;
layout (location = 0) in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}