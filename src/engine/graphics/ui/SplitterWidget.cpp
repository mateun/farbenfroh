//
// Created by mgrus on 29.03.2025.
//

#include "SplitterWidget.h"

#include <iostream>
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

    ;

}

void SplitterWidget::draw(float depth) {

    int splitterSize = 2;

    if (!splitter_initialized_) {
        splitter_initialized_ = true;
        // TODO we should very probably push our "special" members into our common child array so they
        // can participate in normal message flow etc.
        // OTOH this might complicate things with less control.
        // Anyhow this "should" work - the first, second semantic is a bit wonky anyway.
        addChild(first_);
        addChild(second_);
        if (type_ == SplitterType::Horizontal) {
            splitterPosition_ =  {0, global_size_.y/2};
        }
        else {
            splitterPosition_ = {global_size_.x/2 - 100, 0};
        }
    }

    MeshDrawData mdd;
    mdd.debug_id = 100;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);

    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.02, 0.02, 0.02, 1}}};
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

    // Get the parent-depth to be able to render ourselves on top:
    // FIXME: somehow the background panels of the splitter children are not showing up, as if they are below the current
    // background layer. Only see the black bg layer here.
    auto parentDepth = parent_.expired() ? depth : parent_.lock()->getZValue();


    // Calculate the drawing coordinates for the splitter itself
    if (type_ == SplitterType::Vertical) {
        mdd.location = {splitterPosition_.x + splitterSize/2, splitterPosition_.y, parentDepth + 0.01};
        mdd.scale = {splitterSize, global_size_.y, 1};
        mdd.debug_label = "vertical_splitter";
    } else {
        mdd.location = {splitterPosition_.x, splitterPosition_.y, parentDepth + 0.01};
        mdd.scale = {global_size_.x, splitterSize, 1};
        mdd.debug_label = "horizontal_splitter";
    }

    // Draw the splitter:
    Renderer::drawWidgetMeshDeferred(mdd, this);

    // Define origin and size of the two child panels with respect to the splitter:
    if (type_ == SplitterType::Vertical) {

        first_->setOrigin({global_origin_.x, global_origin_.y});
        second_->setOrigin({splitterPosition_.x + 5, 0});

        first_->setSize({splitterPosition_.x, global_size_.y});
        second_->setSize({global_size_.x - splitterPosition_.x, global_size_.y});
    } else {
        first_->setOrigin({global_origin_.x, global_origin_.y + splitterPosition_.y + 5});
        second_->setOrigin({global_origin_.x, global_origin_.y + 2});

        first_->setSize({global_size_.x, global_size_.y - splitterPosition_.y - 5});
        second_->setSize({global_size_.x,  splitterPosition_.y - 5});
    }

    // Render background panels for each child
#ifdef RENDER_SPLITTER_BGS_DEBUG
    if (type_ == SplitterType::Vertical) {
        // Left panel
        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{ "gradientTargetColor", glm::vec4{0.01, 0.01, 0.01, 1}}};
        mdd.color = {0.015, 0.015, 0.017, 1};
        //mdd.location = {global_origin_.x + 4, global_origin_.y + 4, parentDepth + 0.02};
        mdd.location = {global_origin_.x + 4, global_origin_.y + 4, parentDepth + 0.02};
        mdd.scale = {splitterPosition_.x - splitterSize - 1, global_size_.y - 5, 1};
        mdd.debug_label = "left_splitter_bg_panel";
        Renderer::drawWidgetMeshDeferred(mdd, this);

        // Right
        mdd.location = {splitterPosition_.x + splitterSize + 3, global_origin_.y + 2, parentDepth + 0.02};
        mdd.scale = {global_size_.x - splitterPosition_.x - (splitterSize + 5), global_size_.y - 5, 1};
        mdd.debug_label = "right_splitter_bg_panel";
        Renderer::drawWidgetMeshDeferred(mdd, this);
    } else {
        // Top
        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", glm::vec2{global_size_.x, global_size_.y - splitterPosition_.y}}, ShaderParameter{"viewPortOrigin", glm::vec2(global_origin_.x, splitterPosition_.y)}, ShaderParameter{ "gradientTargetColor", glm::vec4{0.01, 0.01, 0.01, 1}}};
        mdd.color = {0.015, 0.015, 0.017, 1};
        mdd.location = {global_origin_.x + 2, global_origin_.y + 10 + splitterPosition_.y, parentDepth + 0.02};
        mdd.scale = {global_size_.x - 4, splitterPosition_.y - splitterSize - 10, 1};
        mdd.debug_label = "top_splitter_bg_panel";
        Renderer::drawWidgetMeshDeferred(mdd, this);

        // Bottom
        mdd.location = {global_origin_.x + 2, global_origin_.y + 4, parentDepth + 0.02};
        mdd.scale = {global_size_.x - 4, splitterSize - 4, 1};
        mdd.debug_label = "bottom_splitter_bg_panel";
        Renderer::drawWidgetMeshDeferred(mdd, this);

    }
#endif

    // Renader the childs themselves,
    // on top of the background panels
    // We have drawn the background gradients at +0.01 above the parent,
    // so for the actual children we put parentDepth + 0.02 as the depth override value.
    first_->draw(parentDepth + 0.03);
    second_->draw(parentDepth + 0.03);


}

MessageHandleResult SplitterWidget::onMessage(const UIMessage &message) {

    if (message.type == MessageType::MouseMove) {
        //std::cout << "in onMessage [" << std::to_string(message.num) << "] for splitter widget: " << this->id_ << " from " << message.sender <<  std::endl;
        MessageHandleResult message_handle_result = {false, "", false};
        bool overSplit = false;
        // Transform the global mouse coordinates into widget local mouse coordinates:
        glm::vec2 localMouse = {message.mouseMoveMessage.x  - global_origin_.x, message.mouseMoveMessage.y - global_origin_.y};
        if (type_ == SplitterType::Vertical) {
            if (localMouse.x >= splitterPosition_.x - 5 && localMouse.x <= splitterPosition_.x + 5 &&
                localMouse.y <= global_size_.y) {
                mouse_over_splitter_ = true;
                overSplit = true;
                message_handle_result.wasHandled = true;
                std::cout << "over vertical splitter for widget: " << id_ << std::endl;
                return message_handle_result;
            }
        } else {
            if (localMouse.y >= splitterPosition_.y - 5
                && localMouse.y <= splitterPosition_.y + 5
                ) {
                mouse_over_splitter_ = true;
                overSplit = true;
                message_handle_result.wasHandled = true;
                std::cout << "over horizontal splitter for widget: " << id_ << std::endl;
                return message_handle_result;
            }
        }

        if (!overSplit) {
            // Only relieve this if we are not dragging already.
            if (!dragging_) {
                mouse_over_splitter_ = false;
                message_handle_result.wasHandled = false;
            } else {
                std::cout << "dragging splitter in " << id_ << std::endl;
                if (type_ == SplitterType::Vertical) {
                    splitterPosition_.x = localMouse.x;
                    if (splitterPosition_.x < first_->origin().x + first_->getMinSize().x + 10) {
                        splitterPosition_.x = first_->origin().x + first_->getMinSize().x + 10;
                    }
                    if (splitterPosition_.x > (second_->origin().x + second_->size().x) - second_->getMinSize().x - 20) {
                        splitterPosition_.x = (second_->origin().x + second_->size().x) - second_->getMinSize().x - 20;
                    }
                } else {
                    splitterPosition_.y = localMouse.y;
                    if (splitterPosition_.y > global_size_.y - first_->getMinSize().y - 5) {
                        splitterPosition_.y = global_size_.y - first_->getMinSize().y - 5;
                    }
                    if (splitterPosition_.y < (second_->origin().y + second_->getMinSize().y + 4)) {
                        splitterPosition_.y = second_->origin().y + second_->getMinSize().y + 4;
                    }
                }
                message_handle_result.wasHandled = true;

            }
        }

        // We return our result if we handled the message.
        if (message_handle_result.wasHandled) {
            return message_handle_result;
        }
        // Otherwise we delegate to the super implementation
        return Widget::onMessage(message);

    }


    if (message.type == MessageType::MouseDown) {
        if (mouse_over_splitter_) {
            dragging_ = true;
            std::cout << "turned dragging on for widget: " << id_ << std::endl;
            return {true, "", true};
        }



    }
    if (message.type == MessageType::MouseUp) {
        if (mouse_over_splitter_) {
            dragging_ = false;
            std::cout << "turned dragging off for widget: " << id_ << std::endl;
            return {true, "", true};
        }

    }

    return Widget::onMessage(message);
}
