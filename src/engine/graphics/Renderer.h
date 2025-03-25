//
// Created by mgrus on 25.03.2025.
//

#ifndef RENDERER_H
#define RENDERER_H
#include <memory>


class Texture;
class MeshDrawData;

class Renderer {

  public:

    static void drawMesh(const MeshDrawData& mdd);
    static std::shared_ptr<Texture> getDefaultNormalMap();

};



#endif //RENDERER_H
