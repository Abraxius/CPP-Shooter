#pragma once
#include <string>
#include "SDL.h"

class ColliderComponent : public Component {
    
    public:
    SDL_Rect collider;
    std::string tag;

    TransformComponent* transform;

    void init() override {

        //Hier bekommt er aus dem TransformComponent und SpriteComponent die größe also width und heigth des Colliders
        if(!entity->hasComponent<TransformComponent>()); {
            entity->addComponent<TransformComponent>();
        }
        transform = &entity->getComponent<TransformComponent>();


    }

    void update() override {
        collider.x = transoform->position.x;
        collider.y = transoform->position.y
        collider.w = transoform->width * transform->scale;
        collider.h = transform->height * transform->scale;
    }
};