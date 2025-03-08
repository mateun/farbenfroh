//
// Created by mgrus on 07.03.2025.
//

#ifndef JOINTMASK_H
#define JOINTMASK_H


enum class MaskType {
  Hierarchy,
  List
};

enum class HierarchyDirection {
    up, down,
};

/**
* Defines a set of joints which will exclusively participate in an animation.
* If the animation is set to masked, then only the qualifying bones will be part.
* Two types:
* - hierarchical: from a named joint down or up in the hierarchy
* - list: a list of joints forming the mask
*/
class JointMask {

  public:
    JointMask(Skeleton* skeleton, MaskType maskType, HierarchyDirection direction = HierarchyDirection::up, const std::string& startJointName= "");



    bool isPartOfMask(Joint* candidate);

    void addJointByName(const std::string &name);

    void addJoint(Joint* joint);


private:
    MaskType maskType;
    Skeleton * skeleton = nullptr;

    // only used if the type is hierarchical
    HierarchyDirection hierarchyDirection;
    std::string hierarchyStartJointName;
    Joint* hierarchyStartJoint = nullptr;

    // Only used if the type is list
    std::vector<Joint*> joints;

    bool isPartOfMask(Joint *candidate, Joint *startJoint);

};



#endif //JOINTMASK_H
