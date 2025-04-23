//
// Created by mgrus on 22.03.2025.
//

#ifndef SKELETON_IMPORT_H
#define SKELETON_IMPORT_H

#include <string>
#include <memory>
#include <ozz/animation/runtime/skeleton.h>


class OzzSkeletonImporter {
public:
    void importSkeleton(const std::string& filename, std::shared_ptr<ozz::animation::Skeleton>& target);

};

#endif //SKELETON_IMPORT_H
