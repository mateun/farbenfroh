//
// Created by mgrus on 10.03.2025.
// A layer over Jolt physics library.
//



#include "PhysicsSystem.h"
#include <Jolt/Jolt.h>
#include <Jolt/Core/Core.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Constraints/Constraint.h>


void PhysicsSystem::initJolt() {

    using namespace JPH;
    RegisterDefaultAllocator();
}
