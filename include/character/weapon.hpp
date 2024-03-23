#pragma once

struct Weapon
{
    bool isReloading;
    bool isAutomatic;
    bool isFired;
    bool isAim;

    unsigned int shotRate;
    unsigned int lastShot;
    unsigned int bullets = 6;
    unsigned int magazine = 6;
    unsigned int availableBullets = 30;

    // ToDo: für später
    // float precision;
    // float aimPrecision;
    // std::string name;

    // ToDo: Für später
    // void aim() {}

    bool fire() // ToDo: für später glm::vec3& direction,glm::vec3& startpoint,glm::vec3& camdirection
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
                    // ToDo: für später, minimale Abweichungen beim Schießen, Ungenauigkeiten
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
                    // Mix_PlayChannel(-1,firesound,0);
                    bullets--;
                    std::cout << "Munition: " << bullets << std::endl;

                    return 1;
                }
            }
            else
            {
                // Mix_PlayChannel(-1,emptysound,0);
                reload();
                return 0;
            }
        }
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
            std::cout << "Nachgeladen: " << bullets << std::endl;
            // Mix_PlayChannel(-1,reloadsound,0);
            return true;
        }
        return false;
    }

    void update()
    {
        lastShot++;
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