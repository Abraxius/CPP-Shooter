#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>

#include "game_objects/model.hpp"
#include "enemy_system/enemy.hpp"

#include <random>

struct EnemySystem {
    EnemySystem(int count) {
        numberOfEnemies = count;
    }

    void spawnEnemys() {
        for (int i = 0; i < numberOfEnemies; i++) {
            //std::pair<int, int> cords = generateUniqueRandomCoordinate(-20, 20, -20, 20);
        std::pair<int, int> cords;
        do {
            cords = generateUniqueRandomCoordinate(-20, 20, -20, 20);
        } while (cords.first < 5 && cords.first > -5 || cords.second < 5 && cords.second > -5);
            // Enemy newEnemy = Enemy({2 + (i + 2), 0, 0}, {0, 0, 0}, {.02, .02, .02}, "models/monkey/untitled.obj", 100.f);
            Enemy newEnemy = Enemy({cords.first, 0, cords.second}, {0, 0, 0}, {1, 1, 1}, "models/zombie/Enemy Zombie.obj", 100.f);
            newEnemy.ID = spawnedEnemies;
            enemies.insert(enemies.end(), newEnemy);
            spawnedEnemies++;
        }
    }

    void deleteEnemies(int id, std::list<Enemy> &listToDeleteFrom)
    {
        for (auto it = listToDeleteFrom.begin(); it != listToDeleteFrom.end(); /* don't increment here */)
        {
            if (it->ID == id)
            {
                std::cout << "Enemie deleted" << std::endl;
                it = listToDeleteFrom.erase(it);
                spawnedEnemies--;
            }
            else
            {
                ++it;
            }
        }
    }

private:
    std::set<std::pair<int, int>> existingCoordinates;

    std::pair<int, int> generateUniqueRandomCoordinate(int minX, int maxX, int minY, int maxY) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> disX(minX, maxX);
        std::uniform_int_distribution<> disY(minY, maxY);

        std::pair<int, int> generatedCoordinate;
        do {
            generatedCoordinate = std::make_pair(disX(gen), disY(gen));
        } while (existingCoordinates.find(generatedCoordinate) != existingCoordinates.end());

        return generatedCoordinate;
    }

public:
    std::list<Enemy> enemies;
    int numberOfEnemies;
    int spawnedEnemies = 0;
};