#pragma once

#include "weapon/projectile.hpp"

struct Weapon
{
    bool isReloading = false;
    bool isAutomatic = false;
    bool isFired = false;
    bool isAim = false;

    unsigned int shotRate = 200;
    unsigned int lastShot = 200;
    unsigned int bullets = 6;
    unsigned int magazine = 6;
    unsigned int availableBullets = 30;

    unsigned int currentReloadTime = 0;
    unsigned int reloadTime = 300;

    std::list<Projectile> projectilesList;
    int projectNumbers = 0;

    // Creates the projectile and adds it to the list
    void shootProjectile(glm::vec3 playerPos, glm::vec3 playerRot)
    {
        glm::vec3 spawnPosition = playerPos += glm::quat(playerRot) * glm::vec3(0, 0, -1.5f);
        Projectile newProjectile = Projectile(playerPos, playerRot, {0.2, 0.2, 0.2}, "models/test/cube.obj");
        newProjectile.ID = projectNumbers;
        projectilesList.push_back(newProjectile);

        projectNumbers += 1;
    }

    // Delte the projectile and remove it from the list
    void deleteProjectile(int id, std::list<Projectile> &listToDeleteFrom)
    {
        for (auto it = listToDeleteFrom.begin(); it != listToDeleteFrom.end();)
        {
            if (it->ID == id)
            {
                std::cout << "Projectile deleted" << std::endl;
                it = listToDeleteFrom.erase(it);
                projectNumbers--;
            }
            else
            {
                ++it;
            }
        }
    }

    // Called before shooting. This is where you check whether you can shoot at all, whether there is ammunition or whether the cooldown period is over.
    bool fire() 
    {
        if (isReloading)
        {
            return 0;
        }
        if ((!isAutomatic && !isFired) || isAutomatic)
        {
            if (lastShot >= shotRate)
            {
                if (bullets > 0)
                {
                    // ToDo: For later, minimal deviations when shooting, inaccuracies
                    /*if (isAim)
                    {
                        direction.x = camdirection.x + ((float)(rand() % 2 - 1) / aimPrecision);
                        direction.y = camdirection.y + ((float)(rand() % 2 - 1) / aimPrecision);
                        direction.z = camdirection.z + ((float)(rand() % 2 - 1) / aimPrecision);
                    }
                    else
                    {
                        direction.x = camdirection.x + ((float)(rand() % 2 - 1) / precision);
                        direction.y = camdirection.y + ((float)(rand() % 2 - 1) / precision);
                        direction.z = camdirection.z + ((float)(rand() % 2 - 1) / precision);
                    }*/

                    isFired = true;
                    lastShot = 0;

                    // Play shot sound.
                    // ToDo: If sdl3 mixer works, integrate audio here!
                    // Mix_PlayChannel(-1, audio.samples[0], 0);

                    bullets--;
                    std::cout << "Munition: " << bullets << std::endl;

                    return 1;
                }
                else
                {
                    // Play empty sound.
                    // ToDo: If sdl3 mixer works, integrate audio here!
                    // Mix_PlayChannel(-1, audio.samples[0], 0);

                    reload();
                    return 0;
                }
            }
        }
        return 0;
    }

    void noFire()
    {
        isFired = false;
    }

    bool reload()
    {
        if (!isReloading && magazine != bullets)
        {
            isReloading = true;
            if (availableBullets + bullets > magazine)
            {
                availableBullets -= magazine - bullets;
                bullets = magazine;
            }
            else
            {
                bullets = availableBullets + bullets;
                availableBullets = 0;
            }
            std::cout << "Loading!" << std::endl;

            // Play reload sound.
            // ToDo: If sdl3 mixer works, integrate audio here!
            // Mix_PlayChannel(-1, audio.samples[0], 0);

            return true;
        }
        return false;
    }

    // Manages the reload time
    void update()
    {
        if (!isReloading)
            lastShot++;
        else
        {
            if (currentReloadTime >= reloadTime)
            {
                currentReloadTime = 0;
                std::cout << "Reloaded: " << bullets << std::endl;
                isReloading = false;
            }
            else
            {
                currentReloadTime++;
            }
        }
    }

    void addBullets(unsigned int num)
    {
        availableBullets += num;
    }

    void setBullets(unsigned int num)
    {
        availableBullets = num;
    }

    bool isAimed()
    {
        return isAim;
    }
};
