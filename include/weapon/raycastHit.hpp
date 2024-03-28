#pragma once

struct Ray
{
    glm::vec3 orig; // Origin of the ray
    glm::vec3 dir;  // Direction of the ray
};

struct Sphere
{
    glm::vec3 center; // Center of the sphere
    float radius;     // Radius of the sphere

    Sphere(const glm::vec3 &center, float radius)
        : center(center), radius(radius) {}
};

// Manages methods for creating a raycast and calculating whether it collides with a created sphere
struct RaycastHit
{
    // This is where the rough calculation of whether the ray will hit the sphere takes place
    bool intersectRaySphere(const Ray &ray, const Sphere &sphere, float &t0, float &t1)
    {
        // Translate ray origin to sphere center
        glm::vec3 L = sphere.center - ray.orig;

        float tca = glm::dot(L, ray.dir);
        if (tca < 0)
            return false; // Ray is pointing away from the sphere

        float d2 = glm::dot(L, L) - tca * tca;
        if (d2 > sphere.radius * sphere.radius)
            return false; // Ray misses the sphere

        float thc = sqrt(sphere.radius * sphere.radius - d2);
        t0 = tca - thc;
        t1 = tca + thc;

        return true;
    }

    // Creates a raycast from the center of the camera in the viewing direction
    Ray getRaycast(Window &window, Camera &camera)
    {
        // Get mouseposition
        std::pair<float, float> mousePosition = {window.width / 2, window.height / 2}; // getMousePosition();

        // transform in 3d normalised device coordinates
        float x = (2.0f * mousePosition.first) / window.width - 1.0f;
        float y = 1.0f - (2.0f * mousePosition.second) / window.height; //rotate y, because it currently starts at 0 at the top left
        float z = 1.0f;
        glm::vec3 ray_nds = glm::vec3(x, y, z);

        // 4d homogeneous clip coordinates
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

        // 4d eye (camera) coordinates
        glm::vec4 ray_eye = glm::inverse(camera.projectionMatrix) * ray_clip;

        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // 4d world coordinates
        glm::vec3 ray_wor = (glm::inverse(camera.viewMatrix) * ray_eye); 
        ray_wor = glm::normalize(ray_wor);

        // Origin des Strahls
        glm::vec3 ray_origin = camera.position;

        Ray ray;
        ray.orig = ray_origin;
        ray.dir = ray_wor;

        return ray;
    }

    // Calls up the method for calculating whether the ray hits the sphere and calculates the exact intersection point
    bool isCollision(Ray &ray, Sphere &sphere)
    {
        float t0, t1;

        if (intersectRaySphere(ray, sphere, t0, t1))
        {
            //std::cout << "Ray intersects the sphere." << std::endl;

            glm::vec3 Phit = ray.orig + ray.dir * t0;              // Calculating the intersection point
            glm::vec3 Nhit = glm::normalize(Phit - sphere.center); // Calculate the normal at the intersection

            //std::cout << "Intersection point: (" << Phit.x << ", " << Phit.y << ", " << Phit.z << ")" << std::endl;
            //std::cout << "Normal at intersection: (" << Nhit.x << ", " << Nhit.y << ", " << Nhit.z << ")" << std::endl;

            return true;
        }
        else
        {
            //std::cout << "Ray does not intersect the sphere." << std::endl;

            return false;
        }
    }
};
