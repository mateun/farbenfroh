//
// Created by mgrus on 08.03.2025.
//

#ifndef PERBONEBLENDDATA_H
#define PERBONEBLENDDATA_H
#include <map>
#include <string>
#include <optional>


/**
* Represents e.g. blend weights (future more..?)
* data which is useful when blending two animations.
*/
class PerBoneBlendData {

public:
   void addNamedBoneWeight(const std::string& boneName, float weight);

   std::optional<float> getWeightForBone(const std::string& boneName);

private:
   std::map<std::string, float> boneWeights;

};



#endif //PERBONEBLENDDATA_H
