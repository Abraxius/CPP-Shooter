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
    Enemy(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::string path, float health)
        : Model(pos, rot, scale, path), health(health) {}

    void hit(int damage) {
        std::cout << "Enemy hit! (" << damage << "damage)" << std::endl;
        health = health - damage;
        if(health <= 0) die();
    }
    void die() {
        std::cout << "Enemy died!" << std::endl;
    }

     // Methode zur Überprüfung, ob der Spieler im Sichtfeld des Feindes ist
    bool isPlayerInSight(glm::vec3 playerPosition) {
        // Pseudocode: Überprüfen, ob der Spieler im Sichtfeld und innerhalb der Sichtweite des Feindes ist
        // Berechnen Sie den Vektor vom Feind zum Spieler
        glm::vec3 toPlayer = playerPosition - this->transform.position;
        // Überprüfen Sie, ob der Spieler im Sichtfeld des Feindes ist (z.B. durch Dot-Produkt)
        float dotProduct = glm::dot(glm::normalize(toPlayer), {0,0,1});
        if (dotProduct > cos(fieldOfView / 2) && glm::length(toPlayer) < sightDistance) {
            return true;
        }
        return false;
    }

private:
    float health;
    float fieldOfView = 90.f;
    float sightDistance = 10.f;
};