#pragma once
#include "game_objects/lights/light_base.hpp"

struct PointLight : public Light {
    PointLight(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, float radius) : Light(pos, rot, scale), radius(radius) {
        // create shadow cubemap
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &shadowCubemap);
        glTextureStorage2D(shadowCubemap, 1, GL_DEPTH_COMPONENT32F, shadowWidth, shadowHeight);
        // set wrapping/magnification behavior
        glTextureParameteri(shadowCubemap, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(shadowCubemap, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(shadowCubemap, GL_TEXTURE_WRAP_R, GL_REPEAT);
        glTextureParameteri(shadowCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(shadowCubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // create shadow camera matrices
        shadowProjection = glm::perspectiveFov(glm::radians(90.0f), shadowWidth, shadowHeight, 1.0f, radius);
        shadowViews[0] = glm::lookAt(transform.position, transform.position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // right
        shadowViews[1] = glm::lookAt(transform.position, transform.position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // left
        shadowViews[2] = glm::lookAt(transform.position, transform.position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // top
        shadowViews[3] = glm::lookAt(transform.position, transform.position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)); // bottom
        shadowViews[4] = glm::lookAt(transform.position, transform.position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // back
        shadowViews[5] = glm::lookAt(transform.position, transform.position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // front
    }
    void adjust_viewport() {
        glViewport(0, 0, shadowWidth, shadowHeight);
    }
    void bind_write(int face) {
        Light::bind(0);
        glUniformMatrix4fv(4, 1, false, glm::value_ptr(shadowViews[face]));
        glUniformMatrix4fv(8, 1, false, glm::value_ptr(shadowProjection));
        glUniform1f(25, radius);
    }
    void bind_read(GLuint lightIndex, int texIndex) {
        Light::bind(lightIndex);
        glUniform1f(25 + lightIndex * 3, radius);
        glBindTextureUnit(texIndex, shadowCubemap);
    }

    std::array<glm::mat4x4, 6> shadowViews;
    glm::mat4x4 shadowProjection;
    GLuint shadowCubemap;
    float radius;
    float shadowWidth = 512;
    float shadowHeight = 512;
};