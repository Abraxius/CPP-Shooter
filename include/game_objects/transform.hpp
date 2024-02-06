#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp> // https://glm.g-truc.net/0.9.2/api/a00245.html
#include <glm/gtx/euler_angles.hpp> // https://glm.g-truc.net/0.9.1/api/a00251.html
#include <glm/gtc/type_ptr.hpp> // allows use of glm::value_ptr to get raw pointer to data

struct Transform {
    Transform(
        glm::vec3 pos = glm::vec3(0.0f), 
        glm::vec3 rot = glm::vec3(0.0f), 
        glm::vec3 scale = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scale) {
        }

    void bind() {
        glm::mat4x4 modelMatrix(1.0f); // set matrix to identity

        // calculate model matrix
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix *= glm::yawPitchRoll(rotation.x, rotation.y, rotation.z);
        modelMatrix = glm::scale(modelMatrix, scale);

        // calculate normal matrix (only need rotation)
        glm::mat3x3 normalMatrix(1.0f);
        normalMatrix = glm::mat3x3(glm::yawPitchRoll(rotation.x, rotation.y, rotation.z));

        glUniformMatrix4fv(0, 1, false, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(12, 1, false, glm::value_ptr(normalMatrix));
    }

    glm::vec3 position;
    glm::vec3 rotation; // euler
    glm::vec3 scale;
};