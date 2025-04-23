//
// Created by mgrus on 20.10.2024.
//

#include "InsideShipState.h"
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <stdio.h>

void king::InsideShipState::update() {
    kingGame->updateCameraMover();

    // Physics update
    {
        using namespace JPH;
        static uint64_t step = 0;
        if (bodyInterface.IsActive(sphereId)) {
            ++step;
            RVec3 position = bodyInterface.GetCenterOfMassPosition(sphereId);
            JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(sphereId);
            cout << "Step " << step << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << endl;

            // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
            const int cCollisionSteps = 1;

            // We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
            const float cDeltaTime = 1.0f / 60.0f;

            // Step the world
            kingGame->getPhysicsSystem()->Update(cDeltaTime, cCollisionSteps, kingGame->getTempAllocator(), kingGame->getJobSystem());
            ball->setWorldLocation({position.GetX(), position.GetY(), position.GetZ()});
        }
    }

}

void king::InsideShipState::render() {

    bindCamera(kingGame->getGameplayCamera());

    // Draw Grid for orientation
    {
        scale({1, 1, 1});
        foregroundColor({0.3, 0.6, 0.2, .1});
        location(glm::vec3{0, 0, 0});
        gridLines(100);
        drawGrid();
    }

    lightingOn();
    bindTexture(kingGame->getTextureByName("cockpit_albedo"));
    bindMesh(kingGame->getMeshByName("spaceship_full_inside"));
    drawMesh();

    // Render physics sphere (jolt)
    {
        wireframeOn();
        bindTexture(nullptr);
        foregroundColor({1, 0, 0, 1});
        location(ball->getWorldLocation());
        ball->render();
    }


    // Crosshair in the middle of the screen
    {
        bindCamera(kingGame->getUICamera());
        wireframeOn();
        lightingOff();
        foregroundColor({1, 1, 1, 1});
        scale({16, 16, 1});
        location({window_width / 2, window_height / 2, -2});
        drawPlane();
        wireframeOff();
    }

    kingGame->renderFPS();
}

void king::InsideShipState::init() {
    printf("in init of InsideShip\n");
    auto game = kingGame;
    game->getCameraMover()->setMovementSpeed(15);
    game->getCameraMover()->setFixedPlaneForwardMovement(true);

    // Physics initialization:
    {

        using namespace JPH;
        // Next we can create a rigid body to serve as the floor, we make a large box
        // Create the settings for the collision volume (the shape).
        // Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
        BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
        auto mat = new PhysicsMaterial();

        floor_shape_settings.mMaterial = mat;
        floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

        // Create the shape
        auto floor_shape_result = floor_shape_settings.Create();
        auto floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()


        // Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
        // If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
        using namespace JPH::literals;

        auto floor_settings = new BodyCreationSettings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

        // Create the actual rigid body
        auto floor = bodyInterface.CreateBody(*floor_settings); // Note that if we run out of bodies this can return nullptr
        floor->SetRestitution(0.8);
        floor->SetFriction(0.1);

        // Add it to the world
        bodyInterface.AddBody(floor->GetID(), EActivation::DontActivate);

        // Now create a dynamic body to bounce on the floor
        // Note that this uses the shorthand version of creating and adding a body to the world
        auto sphere_settings = new BodyCreationSettings(new SphereShape(5), RVec3(0.0_r, 40.0_r, -20.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
        sphereId = bodyInterface.CreateAndAddBody(*sphere_settings, EActivation::Activate);
        bodyInterface.SetFriction(sphereId, 0.1);

        // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
        // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
        //bodyInterface.SetLinearVelocity(sphereId, JPH::Vec3(2.0f, -5.0f, 0.0f));
        //bodyInterface.SetGravityFactor(sphereId, 0.1);



        // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
        // You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
        // Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
        game->getPhysicsSystem()->OptimizeBroadPhase();

        ball = new fireball::Sphere(5, {0, 40, -20});
    }
}

king::InsideShipState::InsideShipState(king::KingGame *game, JPH::BodyInterface& bodyInterface) : KingGameState(game), bodyInterface(bodyInterface) {

}
