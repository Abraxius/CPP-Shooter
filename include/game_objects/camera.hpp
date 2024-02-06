#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp> // https://glm.g-truc.net/0.9.2/api/a00245.html
#include <glm/gtx/euler_angles.hpp> // https://glm.g-truc.net/0.9.1/api/a00251.html
#include <glm/gtc/type_ptr.hpp> // allows use of glm::value_ptr to get raw pointer to data

struct Camera {
    // constructor for perspective camera
    Camera(glm::vec3 position, glm::vec3 rotation, float width, float height)
    : position(position), rotation(glm::radians(rotation)) {
        float fov = glm::radians(70.0f);
        projectionMatrix = glm::perspectiveFov(fov, width, height, nearPlane, farPlane);
    }
    // constructor for orthographic camera
    Camera(glm::vec3 position, glm::vec3 rotation)
    : position(position), rotation(glm::radians(rotation)) {
        projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    }

    // translate relative to camera direction
    void translate(float x, float y, float z) {
        position += glm::quat(rotation) * glm::vec3(x, y, z);
    }

    void bind() {
        glm::mat4x4 viewMatrix(1.0f);
        viewMatrix = glm::rotate(viewMatrix, -rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        viewMatrix = glm::rotate(viewMatrix, -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        viewMatrix = glm::translate(viewMatrix, -position);

        glUniformMatrix4fv(4, 1, false, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(8, 1, false, glm::value_ptr(projectionMatrix));
        glUniform3f(16, position.x, position.y, position.z);
    }
    void bind_secondary() {
        glm::mat4x4 viewMatrix(1.0f);
        viewMatrix = glm::rotate(viewMatrix, -rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        viewMatrix = glm::rotate(viewMatrix, -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        viewMatrix = glm::translate(viewMatrix, -position);

        glUniformMatrix4fv(26, 1, false, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(30, 1, false, glm::value_ptr(projectionMatrix));
        glUniform3f(34, position.x, position.y, position.z);
    }

    glm::mat4x4 projectionMatrix;
    glm::vec3 position;
    glm::vec3 rotation; // euler rotation
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
};