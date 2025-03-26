//
// Created by mgrus on 26.03.2025.
//

#ifndef UISYSTEM_H
#define UISYSTEM_H

#include <optional>
#include <string>
#include <vector>
#include <glm\glm.hpp>
#include <engine/graphics/UIButton.h>
#include <engine/graphics/TextureTile.h>
#include <engine/graphics/SpriteBatch.h>

namespace gru {

    /**
        * This class provides methods to register
        * different UI elements such as buttons, textboxes etc.
        * Once per frame, the different query methods can be called to
        * know if e.g. a button has been pressed or text has been entered,
        * a combo-box item has been selected etc.
        */
    class UISystem  {

    public:
        void registerButton(glm::vec2 pos, glm::vec2 size, const std::string& name, TextureTile* textureTile) {
            auto button = UIButton {pos, size, name};
            button.savedSize = size;
            if (_spriteBatch) {
                button.spriteIndex = (_spriteBatch->addSprite(pos, size, textureTile))-1;
                button.spriteBatch = _spriteBatch;

            }
            _buttons.push_back(button);
        }

        // Optionally register a spritebatch.
        // This will then automatically register the sprites for the buttons.
        void registerSpriteBatch(SpriteBatch* spriteBatch) {
            _spriteBatch = spriteBatch;
        }

        // Returns the name of the hovered UIButton,
        // or an emtpy string if none was hovered.
        std::optional<UIButton> queryHovered(int mouseX, int mouseY) {
            std::optional<UIButton> hoveredButton;
            for (auto b : _buttons) {
                auto buttonLeft = b.pos.x - b.size.x/2;
                auto buttonRight = buttonLeft + b.size.x;
                auto buttonBottom = b.pos.y - b.size.y / 2;
                auto buttonTop = buttonBottom + b.size.y;
                if (mouseX >= buttonLeft && mouseX <= buttonRight
                        && mouseY >= buttonBottom && mouseY <= buttonTop) {
                    hoveredButton = b;
                    break;
                        }
            }
            return hoveredButton;
        }

        std::vector<UIButton> buttons() { return _buttons; }

    private:



        std::vector<UIButton> _buttons;
        SpriteBatch* _spriteBatch = nullptr;

    };

} // gru

#endif //UISYSTEM_H
