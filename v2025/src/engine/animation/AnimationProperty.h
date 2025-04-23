//
// Created by mgrus on 17.02.2025.
//

#ifndef ANIMATIONPROPERTY_H
#define ANIMATIONPROPERTY_H

#include <string>

enum class PropertyType {
    INT,
    FLOAT,
    STRING,
    BOOL
};

class AnimationProperty {


public:
    PropertyType propertyType;
    int intValue = -1;
    float floatValue = -1.0f;
    std::string stringValue = "undefined";
    bool boolValue = false;
    bool operator==(const AnimationProperty &) const;
    bool operator>=(const AnimationProperty &other) const;
    bool operator<=(const AnimationProperty &other) const;




};



#endif //ANIMATIONPROPERTY_H
