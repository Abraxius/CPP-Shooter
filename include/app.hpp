#pragma once
// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>

// #include <SDL3_mixer/SDL_mixer.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
// C++ standard library
#include <vector>
#include <iostream>
// Project-local headers
using namespace gl;
#include "utils.hpp"
#include "window.hpp"
#include "pipeline.hpp"
#include "input.hpp"
#include "timer.hpp"
// #include "audio.hpp"
#include "game_objects/model.hpp"
#include "game_objects/lights/light_point.hpp"
#include "game_objects/camera.hpp"
#include <Jolt/Jolt.h>
#include "character/vector.h"
#include "character/collision.h"
#include "character/zombies.h"
// #include "character/weapon.h"

struct Ray
{
    glm::vec3 orig; // Origin of the ray
    glm::vec3 dir;  // Direction of the ray
};

struct Sphere
{
    glm::vec3 center; // Center of the sphere
    float radius;     // Radius of the sphere

    // Constructor
    Sphere(const glm::vec3 &center, float radius)
        : center(center), radius(radius) {}
};

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

struct App
{
    App()
    {
        // create frame buffer for shadow mapping pipeline
        glCreateFramebuffers(1, &shadowPipeline.framebuffer);
        // attach texture to frame buffer (only draw to depth, no color output!)
        glNamedFramebufferReadBuffer(shadowPipeline.framebuffer, GL_NONE);
        glNamedFramebufferDrawBuffer(shadowPipeline.framebuffer, GL_NONE);
    }

    int run()
    {
        while (bRunning)
        {
            Input::flush(); // flush input from last frame
            timer.update(); // update delta time

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EventType::SDL_EVENT_QUIT)
                    bRunning = false;
                ImGui_ImplSDL3_ProcessEvent(&event);
                window.handle_event(event);   // handle window resize and such events
                Input::register_event(event); // handle keyboard/mouse events
            }
            handle_inputs();
            imgui_begin();
            draw();
            draw_ui();
            imgui_end();

            // present drawn frame to the screen
            window.swap();
        }

        return 0;
    }

private:
    void imgui_begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }
    void imgui_end()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void draw_ui()
    {
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::Begin("FPS_Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::Text("%.1f ms", ImGui::GetIO().DeltaTime * 1000.0f);
        ImGui::End();
    }
    void draw()
    {
        // first pass: render shadow map
        glBindFramebuffer(GL_FRAMEBUFFER, shadowPipeline.framebuffer);
        shadowPipeline.bind();
        // for each light
        for (size_t iLight = 0; iLight < lights.size(); iLight++)
        {
            if (bShadowmapsRendered)
                break;
            lights[iLight].adjust_viewport();
            // render each cubemap face
            for (int face = 0; face < 6; face++)
            {
                // set framebuffer texture and clear it
                glNamedFramebufferTextureLayer(shadowPipeline.framebuffer, GL_DEPTH_ATTACHMENT, lights[iLight].shadowCubemap, 0, face);
                glClear(GL_DEPTH_BUFFER_BIT);
                // bind resources to pipeline
                lights[iLight].bind_write(face);
                // draw models
                // model.draw();
                for (auto &model : models)
                    model.draw();
                // draw other light models
                for (size_t i = 0; i < lights.size(); i++)
                {
                    if (i != iLight)
                        lights[i].draw();
                }
            }
        }
        bShadowmapsRendered = true;

        // second pass: render color map
        glViewport(0, 0, window.width, window.height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        colorPipeline.bind();
        // bind resources to pipeline
        camera.bind();
        for (size_t iLight = 0; iLight < lights.size(); iLight++)
        {
            lights[iLight].bind_read(iLight, iLight + 1);
        }
        // draw models
        for (auto &light : lights)
            light.draw();
        // model.draw();
        for (auto &model : models)
            model.draw();
    }

    std::pair<float, float> getMousePosition()
    {
        float x, y;
        SDL_GetMouseState(&x, &y);
        return std::make_pair(x, y);
    }

    // Formel um das Vorzeichen Problem bei der Analythischen zu beheben
    bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
    {
        float discr = b * b - 4 * a * c;
        if (discr < 0)
            return false;
        else if (discr == 0)
            x0 = x1 = -0.5 * b / a;
        else
        {
            float q = (b > 0) ? -0.5 * (b + sqrt(discr)) : -0.5 * (b - sqrt(discr));
            x0 = q / a;
            x1 = c / q;
        }
        if (x0 > x1)
            std::swap(x0, x1);

        return true;
    }

    void handle_inputs()
    {
        // draw wireframe while holding f
        if (Keys::down('f'))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // capture mouse for better camera controls
        if (Keys::pressed(SDL_KeyCode::SDLK_ESCAPE))
            SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());

        // camera movement
        float movementSpeed = timer.get_delta() * 2.0f; // scale speed relative to framerate
        if (Keys::pressed(SDL_KeyCode::SDLK_LSHIFT))
            movementSpeed *= 3.0f; // sprint button
        if (Keys::down('s'))
            camera.translate(0.0f, 0.0f, movementSpeed);
        if (Keys::down('w'))
            camera.translate(0.0f, 0.0f, -movementSpeed);
        // if (Keys::down('e')) camera.translate(0.0f, movementSpeed, 0.0f);
        // if (Keys::down('q')) camera.translate(0.0f, -movementSpeed, 0.0f);
        if (Keys::down('d'))
            camera.translate(movementSpeed, 0.0f, 0.0f);
        if (Keys::down('a'))
            camera.translate(-movementSpeed, 0.0f, 0.0f);

        // if (Keys::pressed('r')) Mix_PlayChannel(-1, audio.samples[0], 0);

        if (Mouse::down('1') || Keys::down('x'))
        {
            // Raycast vom Mauszeiger aus
            //------------------------------------------------------------------ 

            // Get mouseposition
            std::pair<float, float> mousePosition = { window.width/2, window.height/2 }; //getMousePosition();

            // transform in 3d normalised device coordinates
            float x = (2.0f * mousePosition.first) / window.width - 1.0f;
            float y = 1.0f - (2.0f * mousePosition.second) / window.height; // drehen y, weil es aktuell links oben bei 0 anfängt
            float z = 1.0f;
            glm::vec3 ray_nds = glm::vec3(x, y, z);

            // 4d homogeneous clip coordinates
            glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

            // 4d eye (camera) coordinates
            glm::vec4 ray_eye = glm::inverse(camera.projectionMatrix) * ray_clip;

            ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

            // 4d world coordinates
            glm::vec3 ray_wor = (glm::inverse(camera.viewMatrix) * ray_eye); // hier wurde einfach .xyz weggelöscht schau ob das so funktioneirt!
            //  don't forget to normalise the vector at some point
            ray_wor = glm::normalize(ray_wor);

            // Origin des Strahls
            glm::vec3 ray_origin = camera.position;
            // std::cout << "Raycast: x:" << ray_wor.x << "y:" << ray_wor.y << "z:" << ray_wor.z << std::endl;

            // Ray vs Sphere
            //--------------------------------------------------------------------
            Ray ray;
            ray.orig = ray_origin;              
            ray.dir = ray_wor; 

            float t0, t1;

            if (intersectRaySphere(ray, sphereTest, t0, t1))
            {
                std::cout << "Ray intersects the sphere." << std::endl;

                glm::vec3 Phit = ray.orig + ray.dir * t0;                   // Berechnen des Schnittpunktes
                glm::vec3 Nhit = glm::normalize(Phit - sphereTest.center);  // Berechnen Sie die Normale im Schnittpunkt

                std::cout << "Intersection point: (" << Phit.x << ", " << Phit.y << ", " << Phit.z << ")" << std::endl;
                std::cout << "Normal at intersection: (" << Nhit.x << ", " << Nhit.y << ", " << Nhit.z << ")" << std::endl;
            }
            else
            {
                std::cout << "Ray does not intersect the sphere." << std::endl;
            }
            //-------------------------------------------------------------------------
        }
        // camera rotation
        float rotationSpeed = 0.001f;
        camera.rotation.x -= rotationSpeed * Mouse::delta().second;
        camera.rotation.y -= rotationSpeed * Mouse::delta().first;
    }

private:
    Timer timer;
    Window window = Window(1280, 720, 4);
    bool bRunning = true;
    bool bShadowmapsRendered = false;
    // render resources
    Pipeline colorPipeline = Pipeline("shaders/default.vs", "shaders/default.fs");
    Pipeline shadowPipeline = Pipeline("shaders/shadowmapping.vs", "shaders/shadowmapping.fs");
    // Model model = Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/ground/ground.obj");
    Camera camera = Camera({1, 2, 1}, {0, 0, 0}, window.width, window.height);

    std::array<PointLight, 2> lights = {
        PointLight({1, 1, 1}, {0, 0, 0}, {1, 1, 1}, 100.0f),
        PointLight({-10, 4, 0}, {0, 0, 0}, {1, 1, 1}, 100.0f),
    };

    std::array<Model, 2> models = {
        // Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/sponza/sponza.obj"),
        Model({0, 0, 0}, {0, 0, 0}, {1, 1, 1}, "models/ground/ground.obj"),
        Model({4, 0, 0}, {0, 0, 0}, {.02, .02, .02}, "models/monkey/untitled.obj"),
    };



    //ToDo: Die einzelne Sphere durch ein Array oder Liste ersetzen
    //std::vector<std::unique_ptr<Object>> objects;
    Sphere sphereTest = Sphere({1, 1, 1}, 1.0f);

    //  Audio audio;
};