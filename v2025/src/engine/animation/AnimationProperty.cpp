//
// Created by mgrus on 25.03.2025.
//

#include "AnimationProperty.h"



bool AnimationProperty::operator==(const AnimationProperty &other) const {
    if (other.propertyType == PropertyType::BOOL) {
        return other.boolValue == boolValue;
    }
    if (other.propertyType == PropertyType::INT) {
        return other.intValue == intValue;
    }
    if (other.propertyType == PropertyType::FLOAT) {
        return other.floatValue == floatValue;
    }
    if (other.propertyType == PropertyType::STRING) {
        return other.stringValue == stringValue;
    }
    return false;
}

bool AnimationProperty::operator>=(const AnimationProperty &other) const {
    if (other.propertyType == PropertyType::BOOL) {
        return other.boolValue >= boolValue;
    }
    if (other.propertyType == PropertyType::INT) {
        return intValue >= other.intValue;
    }
    if (other.propertyType == PropertyType::FLOAT) {
        return floatValue >= other.floatValue;
    }
    if (other.propertyType == PropertyType::STRING) {
        return stringValue >= other.stringValue;
    }
    return false;
}

bool AnimationProperty::operator<=(const AnimationProperty &other) const {
    if (other.propertyType == PropertyType::BOOL) {
        return boolValue <= other.boolValue;
    }
    if (other.propertyType == PropertyType::INT) {
        return intValue <= other.intValue;
    }
    if (other.propertyType == PropertyType::FLOAT) {
        return floatValue <= other.floatValue;
    }
    if (other.propertyType == PropertyType::STRING) {
        return stringValue <= other.stringValue;
    }
    return false;
}
