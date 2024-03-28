#pragma once

// Automatically creates a limited play area in which you can only move and spawns a wall around it
struct Terrain
{
    Terrain(int areaSizeX, int areaSizeZ)
        : areaSizeX(areaSizeX), areaSizeZ(areaSizeZ) {
            spawnWalls();
        }

    // Limits the range
    bool checkIsInArea(glm::vec3 position)
    {
        if (position.x > areaSizeX - offset || position.x < -areaSizeX + offset) 
            return false;

        if (position.z > areaSizeZ - offset || position.z < -areaSizeZ + offset)
            return false;

        return true;
    }

    std::list<Model> walls;

private:
    // Spawns walls and adds them to the list
    void spawnWalls() {
        for (int i = -1; i <= 1; i = i + 2) {
            Model firstNewWall = Model({areaSizeX * i, 0, 0}, {0, 0, 0}, {1, 5, areaSizeX}, "models/wall/cube.obj");
            walls.push_back(firstNewWall);

            Model secondNewWall = Model({0, 0, areaSizeZ * i}, {0, 0, 0}, {areaSizeZ, 5, 1}, "models/wall/cube.obj");
            walls.push_back(secondNewWall);
        }
    }

    int areaSizeX = 10;
    int areaSizeZ = 10;
    int offset = 2;
};
