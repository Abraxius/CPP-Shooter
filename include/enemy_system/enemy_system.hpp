#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>

#include "game_objects/model.hpp"
#include "enemy_system/enemy.hpp"

struct EnemySystem {
    EnemySystem(int count) {
        numberOfEnemys = count;
    }

    void spawnEnemys() {
        for (int i = 0; i < numberOfEnemys; i++) {
            enemys.insert(enemys.end(), Enemy({2 + (i + 2), 1, 1}, {0, 0, 0}, {.02, .02, .02}, "models/monkey/untitled.obj", 100));
        }
    }

public:
    std::list<Enemy> enemys;
    int numberOfEnemys;
};