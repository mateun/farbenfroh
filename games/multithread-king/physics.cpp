//
// Created by mgrus on 29.05.2024.
//

#include "physics.h"
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

namespace mtking {
    Physics::Physics() {
        JPH::RegisterDefaultAllocator();
        // Create a factory
        JPH::Factory::sInstance = new JPH::Factory();

        // Register all Jolt physics types
        JPH::RegisterTypes();
        JPH::TempAllocatorImpl* temp_allocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
        JPH::JobSystemThreadPool* job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);

        const uint32_t cMaxBodies = 1024;
        const uint32_t cNumBodyMutexes = 0;
        const uint32_t cMaxContactConstraints = 1024;
        const uint32_t cMaxBodyPairs = 1024;

//        auto broadPhaseLayerInterface = new JPH::BPLayerInterfaceImpl();
//        ObjectVsBroadPhaseLayerFilterImpl* objectVsBroadPhaseLayerFilter = new ObjectVsBroadPhaseLayerFilterImpl;
//        ObjectLayerPairFilterImpl* objectLayerPairFilter = new ObjectLayerPairFilterImpl();
//        auto contactListener = new TankofantContactListener();

//        auto physicsSystem = new JPH::PhysicsSystem();
//        physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
//                            cMaxContactConstraints, *broadPhaseLayerInterface,
//                            *objectVsBroadPhaseLayerFilter,
//                            *objectLayerPairFilter);
//        physicsSystem->SetContactListener(contactListener);
//        gameState->physicsData  = new PhysicsData { physicsSystem, physicsSystem->GetBodyInterface(),
//                                                    temp_allocator,
//                                                    job_system};
//
//        // Floor
//        JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
//        JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
//        JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
//        using namespace JPH::literals;
//        JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r, -1.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
//        JPH::Body *floor = gameState->physicsData->bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
//        gameState->physicsData->bodyInterface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);
//        gameState->physicsData->physicsSystem->OptimizeBroadPhase();
    }

    void Physics::update() {
            float cDeltaTime = 1.0f/60.0f;
//            for (auto go : gameState->allGameObjects) {
//
//                if (gameState->physicsData->bodyInterface.IsActive(go->physicsBody->GetID())) {
//                    JPH::RVec3 position = gameState->physicsData->bodyInterface.GetCenterOfMassPosition(go->physicsBody->GetID());
//                    JPH::Vec3 velocity = gameState->physicsData->bodyInterface.GetLinearVelocity(go->physicsBody->GetID());
//                    go->location = {position.GetX(), position.GetY() - 0.2f, position.GetZ()};
//                    printf("loc: %f/%f/%f\n", go->location.x, go->location.y, go->location.z);
//                    gameState->physicsData->physicsSystem->Update(cDeltaTime, 1,
//                                                                  gameState->physicsData->tempAllocator,
//                                                                  gameState->physicsData->jobSystem);
//                }
//
//
//            }

    }
} // mtking