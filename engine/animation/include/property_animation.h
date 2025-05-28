//
// Created by mgrus on 28.05.2025.
//

#ifndef PROPERTY_ANIMATION_H
#define PROPERTY_ANIMATION_H
#include <vector>
#include <iostream>

// Typed key.
// Values between keys are linearly interpolated.
template<typename  T>
struct PropertyAnimKey {
    float time_stamp;
    T val;
};

// Abstract superclass for any animation track
struct IPropertyAnimTrack {
    virtual ~IPropertyAnimTrack() = default;
    virtual void eval(float delta) = 0;
};

// A type templated animation track.
// As it is typed, it holds the actual property pointer.
template<typename T>
struct PropertyAnimTrack : IPropertyAnimTrack {
    T* prop;
    std::vector<PropertyAnimKey<T>> keys;
    float current_time = 0.0f;
    void eval(float delta) override; ;
};




// A PropertyAnimation can animate properties of types
// which support operator+, operater- and operator*.
// These operators are necessary for interpolation.
// One animation can support different properties throught different tracks,
// and the properties can be mixed and matched.
// For example, properties of the same object can be animated.
// But also, properties of different objects can be animated within one Animation.
// The update animation must be regularly called (best once per frame) so the
// animation drives forward.
struct PropertyAnimation {
    void update(float delta);
    std::vector<IPropertyAnimTrack*> tracks;
};


template<typename T>
void PropertyAnimTrack<T>::eval(float delta) {
    current_time += delta;
    std::cout << "delta: " << delta << " current_time: " << current_time << std::endl;

    if (keys.size() < 2 || !prop) return;

    // Clamp to first or last key
    if (current_time <= keys.front().time_stamp) {
        *prop = keys.front().val;
        return;
    }
    if (current_time >= keys.back().time_stamp) {
        *prop = keys.back().val;
        return;
    }

    // Get the key before and after our current time
    PropertyAnimKey<T>* before_key = nullptr;
    PropertyAnimKey<T>* after_key = nullptr;
    for (size_t i = 1; i < keys.size(); ++i) {
        if (keys[i].time_stamp >= current_time) {
            std::cout << "from key: " << (i-1) << " to_key: " << (i) << std::endl;
            before_key = &keys[i - 1];
            after_key = &keys[i];
            break;
        }
    }

    if (!before_key || !after_key) return;

    float diff_between_keys = after_key->time_stamp - before_key->time_stamp;
    float partial = current_time - before_key->time_stamp;
    float percentage = partial / diff_between_keys;
    auto interp_val = before_key->val + ((after_key->val - before_key->val) * percentage);
    *prop = interp_val;
    std::cout << "setting actual interpolated value" << std::endl;
}


#endif //PROPERTY_ANIMATION_H
