//
// Created by mgrus on 10.03.2025.
//

#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

enum class PhysicsSystemProvider {
  Internal,
  Jolt,
  PhysX,
  Bullet
};

/**
* Represents an interface to physics functionality, such as collision detection.
* Also serves an abstracton over potential 3rd party libraries such as PhysX or Jolt etc.
*/
class PhysicsSystem {

public:
  PhysicsSystem(PhysicsSystemProvider provider);

private:
  void initJolt();

};



#endif //PHYSICSSYSTEM_H
