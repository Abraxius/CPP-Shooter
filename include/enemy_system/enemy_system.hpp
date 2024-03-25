#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>

#include "game_objects/model.hpp"
#include "enemy_system/enemy.hpp"

struct EnemySystem {
    EnemySystem(int count) {
        numberOfEnemies = count;
    }

    void spawnEnemys() {
        for (int i = 0; i < numberOfEnemies; i++) {
            // Enemy newEnemy = Enemy({2 + (i + 2), 0, 0}, {0, 0, 0}, {.02, .02, .02}, "models/monkey/untitled.obj", 100.f);
            Enemy newEnemy = Enemy({6 + (i + 2), 0, 0}, {0, 0, 0}, {1, 1, 1}, "models/zombie/Enemy Zombie.obj", 100.f);
            enemies.insert(enemies.end(), newEnemy);
        }
    }

public:
    std::list<Enemy> enemies;
    int numberOfEnemies;
};