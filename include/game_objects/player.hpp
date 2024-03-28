#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp> // https://glm.g-truc.net/0.9.2/api/a00245.html
#include <glm/gtx/euler_angles.hpp> // https://glm.g-truc.net/0.9.1/api/a00251.html
#include <glm/gtc/type_ptr.hpp> // allows use of glm::value_ptr to get raw pointer to data

#include "weapon/weapon.hpp"
#include "weapon/projectile.hpp"

#include "terrain.hpp"

// Manages the player's life
struct Player {
    Player(glm::vec3 position, glm::vec3 rotation, float health, float stamina, float movementSpeed, float sprintSpeed, float rotationSpeed)
    : position(position), rotation(glm::radians(rotation)), health(health), stamina(stamina), movementSpeed(movementSpeed), sprintSpeed(sprintSpeed), rotationSpeed(rotationSpeed) {}

    // Enables movement only in the horizontal plane without flying
    void move(float x, float y, float z) {
        glm::vec3 tmpRotation = glm::vec3(0, rotation.y, rotation.z); 
        glm::vec3 tmpPosition = position + glm::quat(tmpRotation) * glm::vec3(x,y,z);

        if (map.checkIsInArea(tmpPosition)) {
            position = tmpPosition;
        }
    }

    void takeDamage(int damage) {
        std::cout << "Player hit! (" << damage << "damage)" << std::endl;
        health = health - damage;
        std::cout << "Player HP: " << health << std::endl;
        if(health <= 0) die();
    }

    void die() {
        std::cout << "Player died!" << std::endl;
    }

    bool isAlive() {
        if(health <= 0) 
            return false;
        else 
            return true;
    }

    void increaseStamina(float value) {
        float newStamina = stamina + value;
        if (newStamina > 100) stamina = 100;
        else stamina = newStamina;
    }

    void decreaseStamina(float value) {
        float newStamina = stamina - value;
        if (newStamina < 0) stamina = 0;
        else stamina = newStamina;
    }
    
public:
    float health;
    float stamina;

    float movementSpeed;
    float sprintSpeed;
    float rotationSpeed;

    int zombiesKilled = 0;

    glm::vec3 position;
    glm::vec3 rotation; // euler rotation
    
    // Create the limited map
    Terrain map = Terrain(40,40);
};