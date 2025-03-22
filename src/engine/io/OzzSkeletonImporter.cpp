//
// Created by mgrus on 22.03.2025.
//

#include "skeleton_import.h"
#include <string>
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"
#include <iostream>

void OzzSkeletonImporter::importSkeleton(const std::string& filePath, std::shared_ptr<ozz::animation::Skeleton>& targetSkeleton) {
    ozz::io::File file(filePath.c_str(), "rb");
    if (!file.opened()) {
        std::cerr << "Failed to open skeleton file " << filePath << "."
                        << std::endl;
        return;
    }
    ozz::io::IArchive archive(&file);
    if (!archive.TestTag<ozz::animation::Skeleton>()) {
        std::cerr << "Failed to load skeleton instance from file "
                        << filePath << "." << std::endl;
        return;
    }

    // Once the tag is validated, reading cannot fail.
    auto skeleton = new ozz::animation::Skeleton();
    archive >> *skeleton;
    targetSkeleton.reset(skeleton);

}
