#pragma once
// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>
//#include <SDL3_mixer/SDL_mixer.h>
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
//#include "audio.hpp"
#include "game_objects/model.hpp"
#include "game_objects/lights/light_point.hpp"
#include "game_objects/camera.hpp"
#include "enemy_system/enemy_system.hpp"
#include "game_objects/player.hpp"
#include <Jolt/Jolt.h>

struct App {
    App() {
        // create frame buffer for shadow mapping pipeline
        glCreateFramebuffers(1, &shadowPipeline.framebuffer);
        // attach texture to frame buffer (only draw to depth, no color output!)
        glNamedFramebufferReadBuffer(shadowPipeline.framebuffer, GL_NONE);
        glNamedFramebufferDrawBuffer(shadowPipeline.framebuffer, GL_NONE);
    }

    int run() {
        while(bRunning) {
            Input::flush(); // flush input from last frame
            timer.update(); // update delta time
            
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EventType::SDL_EVENT_QUIT) bRunning = false;
                ImGui_ImplSDL3_ProcessEvent(&event);
                window.handle_event(event); // handle window resize and such events
                Input::register_event(event); // handle keyboard/mouse events
            }
            handle_inputs();
            imgui_begin();
            draw();
            draw_ui();
            imgui_end();

            updateGame();

            // present drawn frame to the screen
            window.swap();
        }

        return 0;
    }

private:
    void imgui_begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }
    void imgui_end() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void draw_ui() {
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::Begin("FPS_Overlay", nullptr, ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoNav);
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::Text("%.1f ms", ImGui::GetIO().DeltaTime * 1000.0f);
        ImGui::End();
    }
    void draw() {
        // first pass: render shadow map
        glBindFramebuffer(GL_FRAMEBUFFER, shadowPipeline.framebuffer);
        shadowPipeline.bind();
        // for each light
        for (size_t iLight = 0; iLight < lights.size(); iLight++) {
            if (bShadowmapsRendered) break;
            lights[iLight].adjust_viewport();
            // render each cubemap face
            for (int face = 0; face < 6; face++) {
                // set framebuffer texture and clear it
                glNamedFramebufferTextureLayer(shadowPipeline.framebuffer, GL_DEPTH_ATTACHMENT, lights[iLight].shadowCubemap, 0, face);
                glClear(GL_DEPTH_BUFFER_BIT);
                // bind resources to pipeline
                lights[iLight].bind_write(face);
                // draw models
                for (auto& model : models) model.draw();
                // draw Enemys
                for (auto& enemy : enemySystem.enemies) enemy.draw();
                // draw other light models
                for (size_t i = 0; i < lights.size(); i++) {
                    if (i != iLight) lights[i].draw();
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
        for (size_t iLight = 0; iLight < lights.size(); iLight++) {
            lights[iLight].bind_read(iLight, iLight + 1);
        }
        // draw models
        for (auto& light : lights) light.draw();
        for (auto& model : models) model.draw();

        for (auto& enemy : enemySystem.enemies) enemy.draw();
    }
    void handle_inputs() {
        // draw wireframe while holding f
        if (Keys::down('f')) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // capture mouse for better camera controls
        if (Keys::pressed(SDL_KeyCode::SDLK_ESCAPE)) SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());

        // camera movement
        float movementSpeed = timer.get_delta() * player.movementSpeed; // scale speed relative to framerate
        if (Keys::pressed(SDL_KeyCode::SDLK_LSHIFT)) movementSpeed *= player.sprintSpeed; // sprint button
        if (Keys::down('s')) player.move(0.0f, 0.0f, movementSpeed);
        if (Keys::down('w')) player.move(0.0f, 0.0f, -movementSpeed);
        if (Keys::down('d')) player.move(movementSpeed, 0.0f, 0.0f);
        if (Keys::down('a')) player.move(-movementSpeed, 0.0f, 0.0f);
        
        player.rotation.x -= player.rotationSpeed * Mouse::delta().second;
        player.rotation.y -= player.rotationSpeed * Mouse::delta().first;

        // Sync Camera and Player
        camera.position = player.position;
        camera.rotation = player.rotation;

        // Log for Debugging
        // std::cout << "Move Speed: " << player.movementSpeed << std::endl;
        // std::cout << "Rot Speed: " << player.rotationSpeed << std::endl;
        // std::cout << "Sprint Speed: " << player.sprintSpeed << std::endl;
        // std::cout << "Player Pos: " << player.position.x << "X" << std::endl;
        // std::cout << "Camera Pos: " << player.position.x << "X" << std::endl;
        // std::cout << "Player Pos: " << player.position.y << "Y" << std::endl;
        // std::cout << "Camera Pos: " << player.position.y << "Y" << std::endl;

        // spawn enemys
        if (Keys::down('l')) enemySystem.spawnEnemys();
        
        //if (Keys::pressed('r')) Mix_PlayChannel(-1, audio.samples[0], 0);
    }

    void updateGame() {
        for (auto& enemy : enemySystem.enemies) {
            // Check if player is seen by enemy
            if (enemy.isPlayerInSight(player.position)) {
                std::cout << "Spieler wurde vom Zombie gesehen!" << std::endl;
                // Move Enemy towards Player
                glm::vec3 direction = glm::normalize(player.position - enemy.transform.position);
                float movementSpeed = timer.get_delta() * enemy.movementSpeed;
                enemy.transform.position += direction * movementSpeed;
            }
            // Check if player is hit by enemy
            float distanceToEnemy = glm::distance(player.position, enemy.transform.position);
            float collisionRadius = 2.0f;
            if (distanceToEnemy <= collisionRadius) {
                player.takeDamage(enemy.damage);
            }
        }
    }

private:
    Timer timer;
    Window window = Window(1280, 720, 4);
    bool bRunning = true;
    bool bShadowmapsRendered = false;
    // render resources
    Pipeline colorPipeline = Pipeline("shaders/default.vs", "shaders/default.fs");
    Pipeline shadowPipeline = Pipeline("shaders/shadowmapping.vs", "shaders/shadowmapping.fs");
    //Model model = Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/ground/ground.obj");

    Player player = Player({1, 1.8f, 1}, {0, 0, 0}, 100.f, 2.f, 3.f, 0.001f);
    Camera camera = Camera({1, 1.8f, 1}, {0, 0, 0}, window.width, window.height);
    
    std::array<PointLight, 2> lights = {
        PointLight({10, 4, 0}, {0, 0, 0}, {1, 1, 1}, 100.0f),
        PointLight({-10, 4, 0}, {0, 0, 0}, {1, 1, 1}, 100.0f),
    };

    std::array<Model, 1> models = {
        // Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/sponza/sponza.obj"),
        Model({0, 0, 0}, {0, 0, 0}, {1, 1, 1}, "models/ground/ground.obj"),
        // Model({2, 0, 0}, {0, 0, 0}, {10, 10, 10}, "models/zombie/zombie.obj"),
        // Model({4, 0, 0}, {0, 0, 0}, {.02, .02, .02}, "models/monkey/untitled.obj"),
        // Model({2, 1, 1}, {0, 0, 0}, {.02, .02, .02}, "models/monkey/untitled.obj"),
    };

    EnemySystem enemySystem = EnemySystem(1);

    //Audio audio;
};