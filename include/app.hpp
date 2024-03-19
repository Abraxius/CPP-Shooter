#pragma once
// External libraries
#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
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
#include "audio.hpp"
#include "game_objects/model.hpp"
#include "game_objects/lights/light_point.hpp"
#include "game_objects/camera.hpp"

// Jolt
#include <Jolt/Jolt.h>

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

// Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
JPH_SUPPRESS_WARNINGS

// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

// We're also using STL classes in this example
// using namespace std;

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
	std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{
	// Print to the TTY
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

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
	virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
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

	virtual uint					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
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
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
	{
		std::cout << "Contact validate callback" << std::endl;

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		std::cout << "A contact was added" << std::endl;
	}

	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		std::cout << "A contact was persisted" << std::endl;
	}

	virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
	{
		std::cout << "A contact was removed" << std::endl;
	}
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
	virtual void		OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		std::cout << "A body got activated" << std::endl;
	}

	virtual void		OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		std::cout << "A body went to sleep" << std::endl;
	}
};

struct App {
    App() {
        // create frame buffer for shadow mapping pipeline
        glCreateFramebuffers(1, &shadowPipeline.framebuffer);
        // attach texture to frame buffer (only draw to depth, no color output!)
        glNamedFramebufferReadBuffer(shadowPipeline.framebuffer, GL_NONE);
        glNamedFramebufferDrawBuffer(shadowPipeline.framebuffer, GL_NONE);

        // start_physics();
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
                // model.draw();
                // model2.draw();
                for (auto& model : models) model.draw();
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
        // model.draw();
        // model2.draw();
    }
    void handle_inputs() {
        // draw wireframe while holding f
        if (Keys::down('f')) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // capture mouse for better camera controls
        if (Keys::pressed(SDL_KeyCode::SDLK_ESCAPE)) SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());

        // camera movement
        float movementSpeed = timer.get_delta() * 2.0f; // scale speed relative to framerate
        if (Keys::pressed(SDL_KeyCode::SDLK_LSHIFT)) movementSpeed *= 3.0f; // sprint button
        if (Keys::down('s')) camera.translate(0.0f, 0.0f, movementSpeed);
        if (Keys::down('w')) camera.translate(0.0f, 0.0f, -movementSpeed);
        if (Keys::down('e')) camera.translate(0.0f, movementSpeed, 0.0f);
        if (Keys::down('q')) camera.translate(0.0f, -movementSpeed, 0.0f);
        if (Keys::down('d')) camera.translate(movementSpeed, 0.0f, 0.0f);
        if (Keys::down('a')) camera.translate(-movementSpeed, 0.0f, 0.0f);

        // start physics
        if (Keys::down('p')) start_physics();
        
        // if (Keys::pressed('r')) Mix_PlayChannel(-1, audio.samples[0], 0);

        // camera rotation
        float rotationSpeed = 0.001f;
        camera.rotation.x -= rotationSpeed * Mouse::delta().second;
        camera.rotation.y -= rotationSpeed * Mouse::delta().first;
    }
    void start_physics() {
        // Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
        // This needs to be done before any other Jolt function is called.
        RegisterDefaultAllocator();

        // Install trace and assert callbacks
        Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

        // Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
        // It is not directly used in this example but still required.
        Factory::sInstance = new Factory();

        // Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
        // If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
        // If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
        RegisterTypes();

        // We need a temp allocator for temporary allocations during the physics update. We're
        // pre-allocating 10 MB to avoid having to do allocations during the physics update.
        // B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
        // If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
        // malloc / free.
        TempAllocatorImpl temp_allocator(10 * 1024 * 1024);

        // We need a job system that will execute physics jobs on multiple threads. Typically
        // you would implement the JobSystem interface yourself and let Jolt Physics run on top
        // of your own job scheduler. JobSystemThreadPool is an example implementation.
        JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

        // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint cMaxBodies = 1024;

        // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
        const uint cNumBodyMutexes = 0;

        // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
        // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
        // too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint cMaxBodyPairs = 1024;

        // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
        // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
        const uint cMaxContactConstraints = 1024;

        // Create mapping table from object layer to broadphase layer
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        BPLayerInterfaceImpl broad_phase_layer_interface;

        // Create class that filters object vs broadphase layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

        // Create class that filters object vs object layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;

        // Now we can create the actual physics system.
        PhysicsSystem physics_system;
        physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

        // A body activation listener gets notified when bodies activate and go to sleep
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        MyBodyActivationListener body_activation_listener;
        physics_system.SetBodyActivationListener(&body_activation_listener);

        // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        MyContactListener contact_listener;
        physics_system.SetContactListener(&contact_listener);

        // The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
        // variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
        BodyInterface &body_interface = physics_system.GetBodyInterface();

        // Next we can create a rigid body to serve as the floor, we make a large box
        // Create the settings for the collision volume (the shape).
        // Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
        BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));

        // Create the shape
        ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
        ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

        // Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
        BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

        // Create the actual rigid body
        Body *floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

        // Add it to the world
        body_interface.AddBody(floor->GetID(), EActivation::DontActivate);

        // Now create a dynamic body to bounce on the floor
        // Note that this uses the shorthand version of creating and adding a body to the world
        BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
        BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);

        // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
        // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
        body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));

        // We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
        const float cDeltaTime = 1.0f / 60.0f;

        // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
        // You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
        // Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
        physics_system.OptimizeBroadPhase();

        // Now we're ready to simulate the body, keep simulating until it goes to sleep
        uint step = 0;
        while (body_interface.IsActive(sphere_id))
        {
            // Next step
            ++step;

            // Output current position and velocity of the sphere
            RVec3 position = body_interface.GetCenterOfMassPosition(sphere_id);
            Vec3 velocity = body_interface.GetLinearVelocity(sphere_id);
            std::cout << "Step " << step << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << std::endl;

            // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
            const int cCollisionSteps = 1;

            // Step the world
            physics_system.Update(cDeltaTime, cCollisionSteps, &temp_allocator, &job_system);
        }

        // Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
        body_interface.RemoveBody(sphere_id);

        // Destroy the sphere. After this the sphere ID is no longer valid.
        body_interface.DestroyBody(sphere_id);

        // Remove and destroy the floor
        body_interface.RemoveBody(floor->GetID());
        body_interface.DestroyBody(floor->GetID());

        // Unregisters all types with the factory and cleans up the default material
        UnregisterTypes();

        // Destroy the factory
        delete Factory::sInstance;
        Factory::sInstance = nullptr;
    }

private:
    Timer timer;
    Window window = Window(1280, 720, 4);
    bool bRunning = true;
    bool bShadowmapsRendered = false;
    // render resources
    Pipeline colorPipeline = Pipeline("shaders/default.vs", "shaders/default.fs");
    Pipeline shadowPipeline = Pipeline("shaders/shadowmapping.vs", "shaders/shadowmapping.fs");
    // Model model = Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/sponza/sponza.obj");
    // Model model2 = Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/sponza/monkey.obj");
    Camera camera = Camera({1, 2, 1}, {0, 0, 0}, window.width, window.height);
    std::array<PointLight, 2> lights = {
        PointLight({1, 2, 0}, {0, 0, 0}, {1, 1, 1}, 30.0f),
        PointLight({2, 4, 1}, {0, 0, 0}, {1, 1, 1}, 30.0f),
    };

    std::array<Model, 2> models = {
        // Model({0, 0, 0}, {0, 0, 0}, {.01, .01, .01}, "models/sponza/sponza.obj"),
        Model({0, 0, 0}, {0, 0, 0}, {1, 1, 1}, "models/ground/ground.obj"),
        Model({4, 0, 0}, {0, 0, 0}, {.02, .02, .02}, "models/monkey/untitled.obj"),
    };
    // Audio audio;
};