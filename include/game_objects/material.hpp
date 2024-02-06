#pragma once

struct Material {
    void bind() {
        // bind each component to its location
        GLuint startLocation = 17;
        glUniform3f(startLocation++, ambient.r, ambient.g, ambient.b);
        glUniform3f(startLocation++, diffuse.r, diffuse.g, diffuse.b);
        glUniform3f(startLocation++, specular.r, specular.g, specular.b);
        glUniform1f(startLocation++, shininess);
        glUniform1f(startLocation++, shininessStrength);
        glUniform1f(startLocation++, diffuseBlend);

        if (diffuseBlend > 0.0f) {
            glBindTextureUnit(0, diffuseTexture);
        }
    }

    glm::vec3 ambient = glm::vec3(0.1f); // first slot
    glm::vec3 diffuse = glm::vec3(1.0f); // second slot
    glm::vec3 specular = glm::vec3(0.0f); // third slot
    float shininess = 32.0f; // fourth slot
    float shininessStrength = 1.0f; // fifth slot
    
    // there can be multiple ambient/diffuse/specular textures at once
    GLuint diffuseTexture;
    float diffuseBlend = 0.0f; // should be between 0 and 1
};