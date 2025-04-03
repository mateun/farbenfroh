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

    // TODO we should very probably push our "special" members into our common child array so they
    // can participate in normal message flow etc.
    // OTOH this might complicate things with less control.
    // Anyhow this "should" work - the first, second semantic is a bit wonky anyway.
    //children_.push_back(first);
    //children_.push_back(second);

}

void SplitterWidget::draw(float depth) {

    int splitterSize = 2;


    if (!splitter_initialized_) {
        splitter_initialized_ = true;
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

    mdd.viewPortDimensions =  size_;
    mdd.setViewport = true;
    mdd.viewport = {origin_.x,  origin_.y, size_.x, size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.02, 0.02, 0.02, 1}}};
    if (mouse_over_splitter_) {
        mdd.color = {0.4, 0.12,0.01, 1};
        if (type_ == SplitterType::Vertical) {
            getApplication()->setSpecialCursor(CursorType::ResizeHorizontal);
        } else {
            getApplication()->setSpecialCursor(CursorType::ResizeVertical);
        }


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

        // Calculate size and positions of the 2 children:
        first_->setOrigin({origin_.x, origin_.y + splitterPosition_.y});
        second_->setOrigin({origin_.x, origin_.y});

        first_->setSize({size_.x, size_.y - splitterPosition_.y});
        second_->setSize({size_.x,  splitterPosition_.y});
    }

    Renderer::drawWidgetMeshDeferred(mdd, this);

    if (type_ == SplitterType::Vertical) {
        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{ "gradientTargetColor", glm::vec4{0.01, 0.01, 0.01, 1}}};
        mdd.color = {0.015, 0.015, 0.017, 1};
        mdd.location = {origin_.x + 2, origin_.y + 2, -1.1};
        mdd.scale = {splitterPosition_.x - splitterSize - 1, size_.y - 5, 1};
        Renderer::drawWidgetMeshDeferred(mdd, this);

        mdd.location = {splitterPosition_.x + splitterSize + 3, origin_.y + 2, -1.1};
        mdd.scale = {size_.x - splitterPosition_.x - (splitterSize + 5), size_.y - 5, 1};
        Renderer::drawWidgetMeshDeferred(mdd, this);
    } else {
        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", glm::vec2{size_.x, size_.y - splitterPosition_.y}}, ShaderParameter{"viewPortOrigin", glm::vec2(origin_.x, splitterPosition_.y)}, ShaderParameter{ "gradientTargetColor", glm::vec4{0.01, 0.01, 0.01, 1}}};
        mdd.color = {0.015, 0.015, 0.017, 1};
        mdd.location = {origin_.x + 2, origin_.y + 2 + splitterPosition_.y, -1.1};
        mdd.scale = {size_.x - 2, splitterPosition_.y - 2, 1};
        Renderer::drawWidgetMeshDeferred(mdd, this);
    }


    first_->draw();
    second_->draw();

}

MessageHandleResult SplitterWidget::onMessage(const UIMessage &message) {
    if (message.type == MessageType::MouseMove) {
        MessageHandleResult message_handle_result = {false, "", false};
        bool overSplit = false;
        if (type_ == SplitterType::Vertical) {
            if (message.mouseMoveMessage.x >= splitterPosition_.x - 5 && message.mouseMoveMessage.x <= splitterPosition_.x + 5 &&
                message.mouseMoveMessage.y <= origin_.y + size_.y) {
                mouse_over_splitter_ = true;
                overSplit = true;
                message_handle_result.wasHandled = true;

            }
        } else {
            if (message.mouseMoveMessage.x >= origin_.x && message.mouseMoveMessage.x <= size_.x &&
                message.mouseMoveMessage.y >= splitterPosition_.y - 5 && message.mouseMoveMessage.y <= splitterPosition_.y + 5) {
                mouse_over_splitter_ = true;
                overSplit = true;
                message_handle_result.wasHandled = true;
            }
        }

        if (!overSplit) {
            // Only relieve this if we are not dragging already.
            if (!mouse_down_) {
                mouse_over_splitter_ = false;
                message_handle_result.wasHandled = true;
            } else {
                if (type_ == SplitterType::Vertical) {
                    splitterPosition_.x = message.mouseMoveMessage.x;
                    if (splitterPosition_.x < first_->origin().x + first_->getMinSize().x + 10) {
                        splitterPosition_.x = first_->origin().x + first_->getMinSize().x + 10;
                    }
                    if (splitterPosition_.x > (second_->origin().x + second_->size().x) - second_->getMinSize().x - 20) {
                        splitterPosition_.x = (second_->origin().x + second_->size().x) - second_->getMinSize().x - 20;
                    }
                } else {
                    splitterPosition_.y = message.mouseMoveMessage.y;
                    if (splitterPosition_.y > size_.y - first_->getMinSize().y - 35) {
                        splitterPosition_.y = size_.y - first_->getMinSize().y - 35;
                    }
                    if (splitterPosition_.y < (second_->origin().y + second_->getMinSize().y + 40)) {
                        splitterPosition_.y = second_->origin().y + second_->getMinSize().y + 40;
                    }
                }
                message_handle_result.wasHandled = true;

            }
        }
        if (message_handle_result.wasHandled) {
            return message_handle_result;
        }

        return Widget::onMessage(message);
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

    return Widget::onMessage(message);
}
