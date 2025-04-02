//
// Created by mgrus on 29.03.2025.
//

#include "SplitterWidget.h"

#include <engine/game/Timing.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/Widget.h>
#include <engine/graphics/ui/CursorType.h>

#include "MessageHandleResult.h"


SplitterWidget::SplitterWidget(SplitterType type, std::shared_ptr<Widget> first, std::shared_ptr<Widget> second): type_(type), first_(first),
                                                                                  second_(second) {

}

void SplitterWidget::draw(float depth) {

    int splitterSize = 2;

    static bool splitterInitialized = false;
    if (!splitterInitialized) {
        splitterInitialized = true;
        if (type_ == SplitterType::Horizontal) {
            splitterPosition_ =  {0, size_.y/2};
        }
        else {
            splitterPosition_ = {size_.x/2 - 100, 0};
        }
    }

    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);

    mdd.camera_shared = getDefaultUICam();
    mdd.viewPortDimensions =  size_;
    mdd.setViewport = true;
    mdd.viewport = {origin_.x,  origin_.y, size_.x, size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.02, 0.02, 0.02, 1}}};
    if (mouse_over_splitter_) {
        mdd.color = {0.4, 0.12,0.01, 1};
        getApplication()->setSpecialCursor(CursorType::Resize);

    } else {
        mdd.color = {0.02, 0.02,.03, 1};
        getApplication()->unsetSpecialCursor();
    }

    if (type_ == SplitterType::Vertical) {
        mdd.location = {splitterPosition_.x + splitterSize/2, splitterPosition_.y, -0.5};
        mdd.scale = {splitterSize, size_.y, 1};

        // Calculate size and positions of the 2 children:
        first_->setOrigin({origin_.x, origin_.y});
        second_->setOrigin({splitterPosition_.x + 5, 0});

        first_->setSize({splitterPosition_.x, size_.y});
        second_->setSize({size_.x - splitterPosition_.x, size_.y});

    } else {
        mdd.location = {splitterPosition_.x, splitterPosition_.y, -0.5};
        mdd.scale = {size_.x, 5, 1};
    }

    Renderer::drawWidgetMeshDeferred(mdd, this);

    // Draw background gradients for debugging
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{ "gradientTargetColor", glm::vec4{0.01, 0.01, 0.01, 1}}};
    mdd.color = {0.015, 0.015, 0.017, 1};
    mdd.location = {origin_.x + 2, origin_.y + 2, -1.1};
    mdd.scale = {splitterPosition_.x - splitterSize - 1, size_.y - 5, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);

    mdd.location = {splitterPosition_.x + splitterSize + 3, origin_.y + 2, -1.1};
    mdd.scale = {size_.x - splitterPosition_.x - (splitterSize + 5), size_.y - 5, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);

    first_->draw();
    second_->draw();

}

MessageHandleResult SplitterWidget::onMessage(const UIMessage &message) {
    if (message.type == MessageType::MouseMove) {
        if (message.mouseMoveMessage.x >= splitterPosition_.x - 5 && message.mouseMoveMessage.x <= splitterPosition_.x + 5 &&
            message.mouseMoveMessage.y <= origin_.y + size_.y) {

            mouse_over_splitter_ = true;



        } else {
            // Only relieve this if we are not dragging already.
            if (!mouse_down_) {
                mouse_over_splitter_ = false;
            } else {
                if (type_ == SplitterType::Vertical) {
                    splitterPosition_.x = message.mouseMoveMessage.x;
                    if (splitterPosition_.x < first_->origin().x + first_->getMinSize().x) {
                        splitterPosition_.x = first_->origin().x + first_->getMinSize().x;
                    }
                    if (splitterPosition_.x > (second_->origin().x + second_->size().x) - second_->getMinSize().x) {
                        splitterPosition_.x = (second_->origin().x + second_->size().x) - second_->getMinSize().x;
                    }
                } else {
                    splitterPosition_.y = message.mouseMoveMessage.y;
                }

            }
        }
        return {true, "", true};
    }
    if (message.type == MessageType::MouseDown) {
        if (mouse_over_splitter_) {
            mouse_down_ = true;
        }
        return {true, "", true};
    }
    if (message.type == MessageType::MouseUp) {
        mouse_down_ = false;
        return {true, "", true};
    }

    return MessageHandleResult{false, "", false};
}
