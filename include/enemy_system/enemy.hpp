#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp> // https://glm.g-truc.net/0.9.2/api/a00245.html
#include <glm/gtx/euler_angles.hpp> // https://glm.g-truc.net/0.9.1/api/a00251.html
#include <glm/gtc/type_ptr.hpp> // allows use of glm::value_ptr to get raw pointer to data

#include "game_objects/model.hpp"

struct Enemy : public Model {
    Enemy(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::string path, int health)
        : Model(pos, rot, scale, path), health(health) {}

    void hit(int damage) {
        std::cout << "Enemy hit! (" << damage << "damage)" << std::endl;
        health = health - damage;
        if(health <= 0) die();
    }
    void die() {
        std::cout << "Enemy died!" << std::endl;
    }

public:
    int health;
};