//
// Created by mgrus on 25.03.2025.
//

#ifndef RENDERER_H
#define RENDERER_H
#include <memory>
#include <map>
#include <engine/graphics/Widget.h>
#include <engine/graphics/MeshDrawData.h>

class Texture;


/**
* Does the actual GL state management drawing calls.
*/
class Renderer {

  public:


    /**
    * This method issues a glDraw call immediately, based on the provided draw data.
    * As this translates one-to-one to a gl draw call, it may be bad for performance,
    * when called often.
    * A draw call is a rather expensive operation, so it is best use for
    * "high poly" meshes, which are only drawn very few times per frame.
    */
    static void drawMesh(const MeshDrawData& mdd);

    /**
    * This method does not issue a GL draw call but collects the provided MeshDrawData.
    * The actual drawing is only done via "submitDeferredCalls".
    * May be better for performance if a lot of draw calls are necessary, e.g.
    * UI or bullet hell, particles etc.
    */
    static void drawWidgetMeshDeferred(const MeshDrawData& mdd, const Widget* widget);

    /**
    * This creates at minimum one (and as few as possible...) GL draw calls based on
    * previous invocations of drawMeshDeferred().
    *
    * The list of "draw calls" is then discarded and the next "drawMeshDeferred" starts
    * a new batch.
    *
    */
    static void submitDeferredWidgetCalls();


    /**
    * Provides a default 1x1 normal map to be used for shaders which demand a normal map but
    * you do not have one naturally.
    */
    static std::shared_ptr<Texture> getDefaultNormalMap();


    /**
    * This list is used for deferred rendering to collect the incoming MeshDrawData items.
    */
    inline static std::map<const Widget*, MeshDrawData> batchedDrawData_ = {};
};



#endif //RENDERER_H
