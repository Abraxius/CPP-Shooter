#pragma once
#include "game_objects/model.hpp"

struct Light {
    Light(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) 
    : transform(pos, rot, scale) {
        mesh.load_sphere(36, 36, true);
        material.diffuse = lightColor;
    }

    // bind light properties
    void bind(GLuint lightIndex) {
        glUniform3f(23 + lightIndex * 3, transform.position.x, transform.position.y, transform.position.z);
        glUniform3f(24 + lightIndex * 3, lightColor.x, lightColor.y, lightColor.z);
    }
    
    // draw the object that represents the light
    void draw() {
        transform.bind();
        material.bind();
        mesh.draw();
    }

    glm::vec3 lightColor = glm::vec3(.992f, .984f, .827f);
    Transform transform;
    Material material;
    Mesh mesh;
};