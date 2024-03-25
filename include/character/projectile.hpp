#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp> // https://glm.g-truc.net/0.9.2/api/a00245.html
#include <glm/gtx/euler_angles.hpp> // https://glm.g-truc.net/0.9.1/api/a00251.html
#include <glm/gtc/type_ptr.hpp> // allows use of glm::value_ptr to get raw pointer to data

#include "game_objects/model.hpp"

#include "character/raycastHit.hpp"

struct Projectile : public Model {
    Projectile(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::string path)
        : Model(pos, rot, scale, path) {
            sphereCollider.center = pos;    
        }

public:
    float movementSpeed = 2.f;
    glm::vec3 dir;
    float damage = 20.f;
    Sphere sphereCollider = Sphere(glm::vec3(1,1,1), .1f);

private:
    float flyDistance = 90.f;
};