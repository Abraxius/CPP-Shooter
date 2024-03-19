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
// #include "character/CharacterBaseTest.h"
// #include "character/CharacterController.hpp"
// #include "Raycaster/3DSage_Raycaster_v1.hpp"
// #include <Jolt/../HelloWorld/HelloWorld.cpp>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

// STL includes
#include <iostream>
#include <cstdarg>
#include <thread>

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

    // Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
    JPH_SUPPRESS_WARNINGS

    // All Jolt symbols are in the JPH namespace
    using namespace JPH;

    // If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
    using namespace JPH::literals;

    // We're also using STL classes in this example
    using namespace std;

    // Callback for traces, connect this to your own trace function if you have one
    static void TraceImpl(const char *inFMT, ...)
    {
        // Format the message
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the TTY
        cout << buffer << endl;
    }

#ifdef JPH_ENABLE_ASSERTS

    // Callback for asserts, connect this to your own assert handler if you have one
    static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
    {
        // Print to the TTY
        cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << endl;

        // Breakpoint
        return true;
    };

#endif // JPH_ENABLE_ASSERTS

    // Layer that objects can be in, determines which other objects it can collide with
    // Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
    // layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
    // but only if you do collision testing).
    namespace Layers
    {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer NUM_LAYERS = 2;
    };

    /// Class that determines if two object layers can collide
    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };

    // Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
    // a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
    // You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
    // many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
    // your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    };

    // BroadPhaseLayerInterface implementation
    // This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            // Create a mapping table from object to broad phase layer
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        virtual uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char *GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
        {
            switch ((BroadPhaseLayer::Type)inLayer)
            {
            case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                return "NON_MOVING";
            case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                return "MOVING";
            default:
                JPH_ASSERT(false);
                return "INVALID";
            }
        }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    private:
        BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    /// Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };

    // An example contact listener
    class MyContactListener : public ContactListener
    {
    public:
        // See: ContactListener
        virtual ValidateResult OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
        {
            cout << "Contact validate callback" << endl;

            // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
            return ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        virtual void OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
        {
            cout << "A contact was added" << endl;
        }

        virtual void OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
        {
            cout << "A contact was persisted" << endl;
        }

        virtual void OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
        {
            cout << "A contact was removed" << endl;
        }
    };

    // An example activation listener
    class MyBodyActivationListener : public BodyActivationListener
    {
    public:
        virtual void OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
        {
            cout << "A body got activated" << endl;
        }

        virtual void OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
        {
            cout << "A body went to sleep" << endl;
        }
    };

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
    // Funktion zum Zeichnen eines roten Strahls
    void display()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_LINES);
        glVertex3f(2.0, 4.0, 0.0);
        glVertex3f(10.0, 10.0, 0.0);
        glEnd();
        glFlush();
    }

    glm::vec3 GetRaycastDirection(float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
    {
        // Umrechnen von Mauskoordinaten in NDC-Koordinaten
        float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY) / screenHeight;

        // Erstellen eines Richtungsvektors im Viewport-Raum
        glm::vec4 ray_clip(ndcX, ndcY, -1.0, 1.0);
        glm::vec4 ray_eye = glm::inverse(projectionMatrix) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // Transformieren des Richtungsvektors in den Weltkoordinatenraum
        glm::vec4 ray_world = glm::inverse(viewMatrix) * ray_eye;
        glm::vec3 ray_direction = glm::normalize(glm::vec3(ray_world));

        return ray_direction;
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
                model.draw();
                // model2.draw();
                //  draw other light models
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
        model.draw();
        // model2.draw();
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
        if (Keys::down('e'))
            camera.translate(0.0f, movementSpeed, 0.0f);
        if (Keys::down('q'))
            camera.translate(0.0f, -movementSpeed, 0.0f);
        if (Keys::down('d'))
            camera.translate(movementSpeed, 0.0f, 0.0f);
        if (Keys::down('a'))
            camera.translate(-movementSpeed, 0.0f, 0.0f);

        glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (float)window.width / (float)window.height, 0.1f, 100.0f);

        if (Keys::pressed('o'))
        {
            // Mausposition erhalten
            float mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Raycast-Richtungsvektor von der Mitte der Kamera aus berechnen
            // glm::vec3 raycastDirection = GetRaycastDirection(window.width/2, window.height/2, window.width, window.height, viewMatrix, projectionMatrix);

            // Konvertiere Winkel in Radianten (falls noch nicht geschehen)
            float rotationX = glm::radians(camera.rotation.x);
            float rotationY = glm::radians(camera.rotation.y);
            float rotationZ = glm::radians(camera.rotation.z);

            // Erstelle Rotationsmatrizen für jede Achse
            glm::mat4 rotationMatrixX = glm::rotate(glm::mat4(1.0f), rotationX, glm::vec3(1.0f, 0.0f, 0.0f)); // Um die x-Achse drehen
            glm::mat4 rotationMatrixY = glm::rotate(glm::mat4(1.0f), rotationY, glm::vec3(0.0f, 1.0f, 0.0f)); // Um die y-Achse drehen
            glm::mat4 rotationMatrixZ = glm::rotate(glm::mat4(1.0f), rotationZ, glm::vec3(0.0f, 0.0f, 1.0f)); // Um die z-Achse drehen

            // Kombiniere die Rotationsmatrizen, um die Gesamtdrehung zu erhalten
            glm::mat4 totalRotationMatrix = rotationMatrixZ * rotationMatrixY * rotationMatrixX;

            // Richtungsvektor in Richtung der negativen z-Achse (vorwärts)
            glm::vec3 forwardDirection(0.0f, 0.0f, -1.0f);

            // Wende die Gesamtdrehung auf den Richtungsvektor an, um die Richtung im Raum zu erhalten
            glm::vec3 rayDirection = glm::vec3(totalRotationMatrix * glm::vec4(forwardDirection, 0.0f));

            std::cout << "Debug-Log: Der Wert von x ist " << rayDirection.x << " " << rayDirection.y << " " << rayDirection.z << std::endl;
            // Hier kannst du den Raycast verwenden, z.B. für Kollisionserkennung

            // OpenGL-Inhalte zeichnen
            // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // glLoadIdentity();

            // Zeichne einen roten Strahl für den Raycast
            //  DrawRaycast();

            // Rendern und Puffer tauschen
            // SDL_GL_SwapWindow(window);
        }

        // if (Keys::pressed('r'))
        //     Mix_PlayChannel(-1, audio.samples[0], 0);

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
    Model model = Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/sponza/sponza.obj");
    // Model model2 = Model({0, 0, 0}, {0, 0, 0}, {1000, 1000, 1000}, "models/house/cottage_obj.obj");
    Camera camera = Camera({1, 2, 1}, {0, 0, 0}, window.width, window.height);
    std::array<PointLight, 2> lights = {
        PointLight({1, 2, 0}, {0, 0, 0}, {1, 1, 1}, 30.0f),
        PointLight({2, 4, 1}, {0, 0, 0}, {1, 1, 1}, 30.0f),
    };
    // Audio audio;

    // CharacterController characterController = CharacterController();
};