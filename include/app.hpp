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
#include "game_objects/skybox.hpp"

#include "enemy_system/enemy_system.hpp"
#include "game_objects/player.hpp"
#include <Jolt/Jolt.h>

#include "character/weapon.hpp"
#include "character/raycastHit.hpp"
#include <list>

struct App
{
    App()
    {
        // create frame buffer for shadow mapping pipeline
        glCreateFramebuffers(1, &shadowPipeline.framebuffer);
        // attach texture to frame buffer (only draw to depth, no color output!)
        glNamedFramebufferReadBuffer(shadowPipeline.framebuffer, GL_NONE);
        glNamedFramebufferDrawBuffer(shadowPipeline.framebuffer, GL_NONE);


        skyboxPipeline.bind();

        // Spawn Zombies
        enemySystem.spawnEnemys();
        std::cout << "All models loaded!" << std::endl;
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

            if (startScreen)
            {
                if (firstStart)
                {
                    SDL_SetRelativeMouseMode(false);
                    firstStart = false;
                }

                imgui_begin();
                draw_start_ui();
                handle_inputs_startScreen();
                imgui_end();
            }
            else if (gameScreen)
            {
                if (firstStart)
                {
                    SDL_SetRelativeMouseMode(true);
                    firstStart = false;
                }

                handle_inputs();
                imgui_begin();
                draw();
                draw_ui();
                imgui_end();

                weapon.update();
                updateGame();
            }
            else if (endScreen)
            {
                if (firstStart)
                {
                    SDL_SetRelativeMouseMode(false);
                    firstStart = false;
                }

                imgui_begin();
                draw_end_ui();
                handle_inputs_endScreen();
                imgui_end();
            }

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
    void draw_start_ui()
    {
        ImVec2 start_window_size = {750, 500};
        ImGui::SetNextWindowPos({(ImGui::GetIO().DisplaySize.x / 2) - (start_window_size.x / 2), (ImGui::GetIO().DisplaySize.y / 2) - (start_window_size.y / 2)});
        ImGui::SetNextWindowSize(start_window_size);
        ImGui::Begin("Mainmenu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("");
        TextCentered("Mainmenu");
        ImGui::Text("");
        ImGui::Text("");
        TextCentered("Welcome to our Zombie Game which was programmed with OpenGL and SDL2.");
        ImGui::Text("");
        TextCentered("Kill as many zombies as you can and see how long you can stay alive!");
        ImGui::Text("");
        ImGui::Text("");
        ImGui::Text("");
        TextCentered("Remember that zombies can only die from headshots!!!");
        ImGui::Text("");
        ImGui::Text("");
        ImGui::Text("");
        TextCentered("To start the game press Q");
        ImGui::End();
    }

    void draw_end_ui()
    {
        ImVec2 end_window_size = {750, 500};
        ImGui::SetNextWindowPos({(ImGui::GetIO().DisplaySize.x / 2) - (end_window_size.x / 2), (ImGui::GetIO().DisplaySize.y / 2) - (end_window_size.y / 2)});
        ImGui::SetNextWindowSize(end_window_size);
        ImGui::Begin("Mainmenu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("");
        TextCentered("You lose!");
        ImGui::Text("");
        ImGui::Text("");
        TextCentered("You have died!");
        ImGui::Text("");
        char buffer[50];
        std::sprintf(buffer, "Your score is: %d", player.zombiesKilled);
        std::string tmpText = buffer;
        TextCentered(tmpText);
        ImGui::Text("");
        ImGui::Text("");
        ImGui::Text("");
        TextCentered("To exit the game press E");
        ImGui::End();
    }

    void loseGame() {
        gameScreen = false;
        firstStart = true;
        endScreen = true;
    }

    void TextCentered(std::string text)
    {
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(text.c_str());
    }

    void draw_ui()
    {
        // Performance stats
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetNextWindowPos({20, 20});
        ImGui::Begin("FPS_Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::Text("%.1f ms", ImGui::GetIO().DeltaTime * 1000.0f);
        ImGui::End();

        // Crosshair
        auto draw = ImGui::GetBackgroundDrawList();
        draw->AddCircle(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), 6, IM_COL32(0, 0, 255, 255), 100, 2.0f);

        // Player stats
        ImVec2 player_window_size = {200, 150};
        ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x - (player_window_size.x + 20), ImGui::GetIO().DisplaySize.y - (player_window_size.y + 20)});
        ImGui::SetNextWindowSize({200, 150});
        ImGui::Begin("Player stats", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("Player");
        ImGui::Text("%.1f HP", player.health);
        ImGui::Text("%.1f Stamina", player.stamina);
        ImGui::Text("Munition");
        ImGui::Text("%.d / %.d", weapon.bullets, weapon.magazine);
        ImGui::End();

        // Gameplay info
        ImVec2 gameplay_window_size = {250, 100};
        ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x - (gameplay_window_size.x + 20), 20});
        ImGui::SetNextWindowSize({250, 100});
        ImGui::Begin("Gameplay info", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("%d Zombies left", enemySystem.enemies.size());
        ImGui::Text("%d Zombies killed", player.zombiesKilled);
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
                for (auto &model : models)
                    model.draw();

                for (auto &projectiles : weapon.projectilesList)
                    projectiles.draw();

                weaponModel.draw();

                // draw Enemys
                for (auto &enemy : enemySystem.enemies)
                    enemy.draw();

                for (auto &wall : player.map.walls)
                    wall.draw();

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

        //
        glBindFramebuffer(GL_FRAMEBUFFER, skyboxPipeline.framebuffer);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skyboxPipeline.bind();
        // set framebuffer texture and clear it
        skybox.bind();
        // glUniform1i(glGetUniformLocation(2, "skybox"), 0);

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
        for (auto &model : models)
            model.draw();

        for (auto &projectiles : weapon.projectilesList)
            projectiles.draw();

        weaponModel.draw();

        for (auto &enemy : enemySystem.enemies)
            enemy.draw();

        for (auto &wall : player.map.walls)
            wall.draw();
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

    void handle_inputs_startScreen()
    {
        if (Keys::down('q'))
        {
            std::cout << "start game" << std::endl;
            firstStart = true;
            startScreen = false;
            gameScreen = true;
        }
    }

    void handle_inputs_endScreen()
    {
        if (Keys::down('e'))
        {
            std::cout << "restart game" << std::endl;

            cleanup();
            std::exit(0);      
        }
    }

    void cleanup() {

        weapon.projectilesList.clear();
        player.map.walls.clear();
        enemySystem.enemies.clear();
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

        // player movement
        float movementSpeed = timer.get_delta() * player.movementSpeed;
        if (Keys::down(SDL_KeyCode::SDLK_LSHIFT) && player.stamina > 0.5f)
        {
            movementSpeed *= player.sprintSpeed; // sprint button
            player.decreaseStamina(.7f);
        }

        if (Keys::down('s'))
            player.move(0.0f, 0.0f, movementSpeed);
        if (Keys::down('w'))
            player.move(0.0f, 0.0f, -movementSpeed);
        if (Keys::down('d'))
            player.move(movementSpeed, 0.0f, 0.0f);
        if (Keys::down('a'))
            player.move(-movementSpeed, 0.0f, 0.0f);

        if (Keys::down('r'))
            weapon.reload();

        if (Mouse::down(1))
        {
            Ray ray = raycastHit.getRaycast(window, camera);

            if (weapon.fire())
            {
                weapon.shootProjectile(player.position, player.rotation);

                for (auto &enemie : enemySystem.enemies)
                {
                    if (raycastHit.isCollision(ray, enemie.sphereCollider))
                    {
                        enemie.hit(50.0f);
                    }
                }
            }
            else
            {
                weapon.noFire();
            }
        }

        if (Keys::down('l'))
        {
            // enemySystem.spawnEnemys();
        }
        // if (Keys::pressed('r')) Mix_PlayChannel(-1, audio.samples[0], 0);

        // Schwerkraft und Springen
        float jumpHeight = 5.0f; // Maximale Höhe des Sprungs
        float jumpSpeed = 0.1f;  // Geschwindigkeit des Sprungs
        float gravity = 0.05f;   // Schwerkraft

        if (Keys::down(32) && onGround)
        {
            jumping = true;
        }

        if (jumping)
        {
            player.move(0.0f, jumpSpeed, 0.0f);
            if (jumpHeight < player.position.y)
            {
                jumping = false;
            }
        }

        if (player.position.y > 2)
        {
            onGround = false;
        }
        else
        {
            onGround = true;
        }

        if (!onGround)
        {
            player.move(0.0f, -gravity, 0.0f);
        }
        else
        {
            player.position.y = 2;
        }

        player.rotation.x -= player.rotationSpeed * Mouse::delta().second;
        player.rotation.y -= player.rotationSpeed * Mouse::delta().first;

        // Sync Camera and Player
        camera.position = player.position;
        camera.rotation = player.rotation;

        // Berechnen der neuen Position der Waffe basierend auf der Kamerarotation und der Offset-Position
        glm::vec3 weaponPosition = camera.position + (glm::quat(camera.rotation) * glm::vec3(0.25f, -0.5f, -1.0f));

        float pi = 3.14159265358979323846f;

        weaponModel.transform.position = weaponPosition;
        weaponModel.transform.rotation = glm::vec3(camera.rotation.y + pi, -camera.rotation.x, camera.rotation.z); // ToDo: Ich hab keine Ahnung warum diese Mathe funktioniert??? WTF???? Später nochmal anschauen!!!
    }

    void updateGame()
    {
        for (auto &enemy : enemySystem.enemies)
        {
            // Check if player is seen by enemy
            if (enemy.isPlayerInSight(player.position))
            {
                // std::cout << "Spieler wurde vom Zombie gesehen!" << std::endl;
                //  Move Enemy towards Player
                glm::vec3 direction = glm::normalize(player.position - enemy.transform.position);
                enemy.rotateTowards(player.position);

                float movementSpeed = timer.get_delta() * enemy.movementSpeed;
                enemy.transform.position += direction * movementSpeed;
                enemy.transform.position.y = 0.f;
                enemy.sphereCollider.center = enemy.transform.position;
                enemy.sphereCollider.center.y = 2.5f;
            }
            // Check if player is hit by enemy
            float distanceToEnemy = glm::distance(player.position, enemy.transform.position);
            // std::cout << distanceToEnemy << std::endl;
            float collisionRadius = 2.5f;
            if (distanceToEnemy <= collisionRadius)
            {
                // std::cout << "sadasd" << std::endl;
                player.takeDamage(enemy.damage);
            }

            // Check if enemy died
            if (enemy.died)
            {
                player.zombiesKilled++;
            }
        }

        // Update projectiles
        for (auto &projectiles : weapon.projectilesList)
        {
            // ToDo: Steuer die Kugel bewegung
            float movementSpeed = timer.get_delta() * projectiles.movementSpeed;
            projectiles.move(0.0f, 0.0f, -movementSpeed);

            if (projectiles.maxFlyDistanceAchieved())
            {
                // std::cout << "Kugel hat ihr Ziel erreicht" << std::endl;
                weapon.deleteProjectile(projectiles);
            }
            else
            {
                // std::cout << "Kugel flieeeeeeeeg" << std::endl;
            }
        }

        // Remove dead enemies
        // enemySystem.enemies.remove_if([](const Enemy& enemy) {
        //     return enemy.died;
        // });

        // Add Player stamina
        player.increaseStamina(.08f);

        if (!player.isAlive()) {
            loseGame();
        }
    }

    bool startScreen = true;
    bool gameScreen = false;
    bool endScreen = false;
    bool firstStart = true;

private:
    Timer timer;
    Window window = Window(1280, 720, 4);
    bool bRunning = true;
    bool bShadowmapsRendered = false;
    // render resources
    Pipeline colorPipeline = Pipeline("shaders/default.vs", "shaders/default.fs");
    Pipeline shadowPipeline = Pipeline("shaders/shadowmapping.vs", "shaders/shadowmapping.fs");
    Pipeline skyboxPipeline = Pipeline("shaders/skybox.vs", "shaders/skybox.fs");
    Skybox skybox = Skybox();

    Player player = Player({1, 2, 1}, {0, 0, 0}, 100.f, 100.f, 2.f, 3.f, 0.001f);
    Camera camera = Camera({1, 2, 1}, {0, 0, 0}, window.width, window.height);

    std::array<PointLight, 1> lights = {
        PointLight({10, 20, 0}, {0, 0, 0}, {1, 1, 1}, 100.0f),
    };

    Model weaponModel = Model({1, 1, 1}, {0, 0, 0}, {0.2f, 0.2f, 0.2f}, "models/weapon/M4a1.obj");

    std::array<Model, 1> models = {        
        Model({0, 0, 0}, {0, 0, 0}, {1, 1, 1}, "models/Environment/environment_low3.obj"),
    };

    Weapon weapon;
    RaycastHit raycastHit;

    //  Audio audio;
    EnemySystem enemySystem = EnemySystem(1);

    bool onGround = true;
    bool jumping = false;
};