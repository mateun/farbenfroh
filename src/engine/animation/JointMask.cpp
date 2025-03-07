//
// Created by mgrus on 07.03.2025.
//

#include "JointMask.h"

JointMask::JointMask(Skeleton* skeleton, MaskType maskType, HierarchyDirection direction, const std::string& startJointName) :
    maskType(maskType), hierarchyDirection(direction), skeleton(skeleton), hierarchyStartJointName(startJointName) {

    if (maskType == MaskType::Hierarchy) {
        for (auto j : skeleton->joints) {
            if (j->name == startJointName) {
                hierarchyStartJoint = j;
                break;
            }
        }
       if (!hierarchyStartJoint) {
           throw new std::runtime_error("joint start for hierarchy not found in skeleton!");
       }
    }

}

bool JointMask::isPartOfMask(Joint* candidate, Joint* startJoint) {
    if (maskType == MaskType::Hierarchy) {
        // Only search in children
        if (hierarchyDirection == HierarchyDirection::down) {
            for (auto childJoint : startJoint->children) {
                if (childJoint->name == candidate->name) {
                    return true;
                }
                return  isPartOfMask(candidate, childJoint);
            }
        } else {
            auto parent = startJoint->parent;
            if (parent->name == candidate->name) {
                return true;
            }
            return isPartOfMask(candidate, parent);

        }

        return false;
    }

    if (maskType == MaskType::List) {
        for (auto j : joints) {
            if (j->name == candidate->name) {
                return true;
            }
        }
        return false;
    }

}

bool JointMask::isPartOfMask(Joint *candidate) {
    return isPartOfMask(candidate, hierarchyStartJoint);
}

void JointMask::addJoint(Joint *joint) {
    joints.push_back(joint);
}
