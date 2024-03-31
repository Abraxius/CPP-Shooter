#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp> // https://glm.g-truc.net/0.9.2/api/a00245.html
#include <glm/gtx/euler_angles.hpp> // https://glm.g-truc.net/0.9.1/api/a00251.html
#include <glm/gtc/type_ptr.hpp> // allows use of glm::value_ptr to get raw pointer to data

#include "game_objects/model.hpp"

#include "weapon/raycastHit.hpp"

// Enemy class
struct Enemy : public Model {
    Enemy(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::string path)
        : Model(pos, rot, scale, path), health(health) {
            sphereCollider.center = pos;
            sphereCollider.center.y = 2.5f;
        }

    // The unit takes damage
    void hit(int damage) {
        std::cout << "Enemy hit! (" << damage << "damage)" << std::endl;
        health = health - damage;
        if(health <= 0) die();
    }

    // The unit die
    void die() {
        std::cout << "Enemy died!" << std::endl;
        died = true;
    }

     // Method for checking whether the player is in the enemy's field of vision
    bool isPlayerInSight(glm::vec3 playerPosition) {
        // Calculate the vector from the enemy to the player
        glm::vec3 toPlayer = playerPosition - this->transform.position;

        // Extract the Y-axis rotation from the quaternion
        glm::quat rotation = this->transform.rotation;
        float angle = atan2(2.0f * (rotation.y * rotation.z + rotation.w * rotation.x), rotation.w * rotation.w - rotation.x * rotation.x - rotation.y * rotation.y + rotation.z * rotation.z); // Extrahiere die Drehung um die Y-Achse

        // Calculate the zombie's line of sight based on its rotation
        glm::vec3 forwardDirection = glm::normalize(glm::vec3(glm::sin(angle), 0.0f, glm::cos(angle)));

        // Check whether the player is in the zombie's field of vision
        float dotProduct = glm::dot(glm::normalize(toPlayer), forwardDirection);
        if (dotProduct > cos(fieldOfView / 2) && glm::length(toPlayer) < sightDistance) {
            playerVisible = true;
            return true;
        }

        playerVisible = true;
        return false;
    }

    // Function to rotate the enemy towards a target position (player in this case)
    void rotateTowards(const glm::vec3& targetPosition) {
        glm::vec3 direction = glm::normalize(targetPosition - transform.position);
        // Calculate the angle of rotation around the Y-axis
        float angleY = atan2(direction.x, direction.z);
        // Set the new Y rotation
        this->transform.rotation.x = angleY;
    }

public:
    float movementSpeed = 2.5f;
    float damage = 20.f;
    Sphere sphereCollider = Sphere(glm::vec3(1,1,1), .4f);
    bool died = false;
    int ID;

private:
    bool playerVisible;
    float health = 100.f;
    float fieldOfView = 90.f;
    float sightDistance = 40.f;
};