//
// Created by mgrus on 26.05.2024.
//

#ifndef SIMPLE_KING_WALLCOLLIDER_H
#define SIMPLE_KING_WALLCOLLIDER_H

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>


class WallCollider : public JPH::ContactListener {

    virtual JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override
    {


        // TODO cast to specific object, e.g. Enemy, etc.
        // TODO think of how do we know what to cast to?

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2,
                                const JPH::ContactManifold &inManifold,
                                JPH::ContactSettings &ioSettings) override
    {

        auto go1 = (GameObject*)inBody1.GetUserData();
        auto go2 = (GameObject*)inBody2.GetUserData();
        if (go1) {
//            if (go1->type == GameObjectType::Projectile) {
//                // TODO hit something with the projectile
//            }
        }
    }

    virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2,
                                    const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
    {
        std::cout << "A contact was persisted" << std::endl;
    }

    virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
    {
        std::cout << "A contact was removed" << std::endl;
    }

};


#endif //SIMPLE_KING_WALLCOLLIDER_H
