#pragma once

// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>

#include "game_objects/model.hpp"
#include "enemy_system/enemy.hpp"

#include <random>
#include "wave.hpp"
#include <string>

// Creates enemys and manages them in lists
struct EnemySystem
{
    EnemySystem(Wave waves[], size_t size)
    {
        enemyWaves = waves;
        numberOfWaves = size;
        numberOfEnemies = 1;

        std::cout << "Number of Waves: " << numberOfWaves << std::endl;
    }

    void start() {
        startWave(enemyWaves[0]);
    }

    void startWave(Wave wave) {
        activeWave++;
        std::cout << "Wave " << activeWave << " started!" << std::endl;
        // Basic Zombies
        for (int i = 0; i < wave.basicZombies; i++)
            spawnEnemy("models/zombie/Enemy Zombie.obj");
    }

    void spawnEnemy(std::string path)
    {
        // Controls that no zombies spawn directly in the player spawn area
        std::pair<int, int> cords;
        do
        {
            cords = generateUniqueRandomCoordinate(-20, 20, -20, 20);
        } while (cords.first < 5 && cords.first > -5 || cords.second < 5 && cords.second > -5);

        Enemy newEnemy = Enemy({cords.first, 0, cords.second}, {0, 0, 0}, {1, 1, 1}, path);
        newEnemy.ID = spawnedEnemies;
        enemies.insert(enemies.end(), newEnemy);
        spawnedEnemies++;
    }

    void deleteEnemies(int id, std::list<Enemy> &listToDeleteFrom)
    {
        for (auto it = listToDeleteFrom.begin(); it != listToDeleteFrom.end();)
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

        // Check if all enemy are dead
        if (enemies.size() <= 0) {
            std::cout << "All Enemies of this wave are dead!" << std::endl;
            startWave(enemyWaves[activeWave]);
        }
    }

private:
    int activeWave = 0;
    int numberOfWaves;
    Wave * enemyWaves;
    std::set<std::pair<int, int>> existingCoordinates;

    std::pair<int, int> generateUniqueRandomCoordinate(int minX, int maxX, int minY, int maxY)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> disX(minX, maxX);
        std::uniform_int_distribution<> disY(minY, maxY);

        std::pair<int, int> generatedCoordinate;
        do
        {
            generatedCoordinate = std::make_pair(disX(gen), disY(gen));
        } while (existingCoordinates.find(generatedCoordinate) != existingCoordinates.end());

        return generatedCoordinate;
    }

public:
    std::list<Enemy> enemies;
    int numberOfEnemies;
    int spawnedEnemies = 0;
};