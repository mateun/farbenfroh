//
// Created by mgrus on 09.03.2025.
//

#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

// Drawable UI element
class Widget {
public:
    Widget() {
        init();
    };
    virtual void render() {
        MeshDrawData mdd;
        mdd.mesh = quadMesh.get();
        mdd.shader = _shader;
        mdd.location = position;
        mdd.scale = {scale.x, scale.y, 1};
        mdd.color = {1, 1, 0.5, 0.5};
        mdd.camera = _uiCamera;
        mdd.subroutineFragBind = "calculateSingleColor";
        drawMesh(mdd);

    };

    virtual void update() {

    }

    void setPosition(float x, float y) {
        position.x = x;
        position.y = y;
    }

    void setScale(float x, float y) {
        scale.x = x;
        scale.y = y;
    }

protected:
    glm::vec3 position = {0, 0, -1};
    glm::vec2 scale = {64, 64};
    std::unique_ptr<Mesh>  quadMesh = nullptr;
    Camera * _uiCamera = nullptr;
    Shader * _shader = nullptr;

    void init() {
        quadMesh = createQuadMesh(PlanePivot::bottomleft);
        _uiCamera = new Camera();
        _uiCamera->location = {0, 0, 1};
        _uiCamera->lookAtTarget = {0, 0, -1};
        _uiCamera->type = CameraType::Ortho;

        _shader = new Shader();
        _shader->initFromFiles("../assets/shaders/base_static_mesh.vert", "../assets/shaders/base.frag");
    }

};

class ContainerWidget : public Widget {


protected:
    std::vector<Widget*> children;
    Widget* parent = nullptr;
};

enum class SplitType {
    SPLIT_VERTICAL,
    SPLIT_HORIZONTAL
};

class SplitPane : public ContainerWidget {

public:
    SplitPane(Widget* first, Widget* second, SplitType splitType = SplitType::SPLIT_VERTICAL) :_splitType(splitType), _first(first), _second(second) {
    }
    void render() override {

    }

    void update() override {

    }

protected:
    SplitType _splitType;
    Widget* _first = nullptr;
    Widget* _second = nullptr;
    int splitLocation = 100;

};


class DefaultGame;

class LevelEditor : public GameLevel {

public:
    LevelEditor(DefaultGame* game, const std::string& name="levelEditor");
    void render() override;
    void update() override;
    void init() override;

private:
    std::unique_ptr<Mesh> quadMesh = nullptr;
    Widget * baseWidget = nullptr;
};



#endif //LEVELEDITOR_H
