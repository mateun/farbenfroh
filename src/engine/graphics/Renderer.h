//
// Created by mgrus on 25.03.2025.
//

#ifndef RENDERER_H
#define RENDERER_H
#include <memory>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <engine/graphics/Camera.h>
#include <engine/graphics/Bitmap.h>
#include <engine/lighting/Light.h>
#include <engine/graphics/Mesh.h>
#include <engine/io/json.h>

#include "Application.h"
#include "PlanePivot.h"

class Texture;
class MeshDrawData;

class Renderer {

  public:

    static Renderer* getInstance();
    static void drawMesh(const MeshDrawData& mdd);
    static std::shared_ptr<Texture> getDefaultNormalMap();


private:
    Renderer() = default;
    inline static Renderer* instance_ = nullptr;

};



#endif //RENDERER_H
