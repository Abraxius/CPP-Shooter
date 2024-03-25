#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp> // https://glm.g-truc.net/0.9.2/api/a00245.html
#include <glm/gtx/euler_angles.hpp>     // https://glm.g-truc.net/0.9.1/api/a00251.html
#include <glm/gtc/type_ptr.hpp>         // allows use of glm::value_ptr to get raw pointer to data

#include "game_objects/model.hpp"

#include "character/raycastHit.hpp"

struct Projectile : public Model
{
    Projectile(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::string path)
        : Model(pos, rot, scale, path)
    {
        sphereCollider.center = pos;
        startPoint = pos;
    }

    void move(float x, float y, float z)
    {
        transform.position += glm::quat(transform.rotation) * glm::vec3(x, y, z);
    }

    bool maxFlyDistanceAchieved()
    {

        if (distanceBetweenPoints(startPoint, transform.position) > maxFlyDistance)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

public:
    float movementSpeed = 100.f;
    glm::vec3 dir;
    float damage = 20.f;
    Sphere sphereCollider = Sphere(glm::vec3(1, 1, 1), .1f);

private:
    float maxFlyDistance = 90.f;
    glm::vec3 startPoint;
    
    float distanceBetweenPoints(const glm::vec3 &p1, const glm::vec3 &p2)
    {
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float dz = p2.z - p1.z;

        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};