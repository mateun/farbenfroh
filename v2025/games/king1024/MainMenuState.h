//
// Created by mgrus on 31.10.2024.
//

#ifndef SIMPLE_KING_MAINMENUSTATE_H
#define SIMPLE_KING_MAINMENUSTATE_H

#include "king_game.h"
#include "../../src/engine/animation/AnimationPlayer.h"

namespace king {


    class MainMenuState : public KingGameState {

    public:
        MainMenuState(KingGame* game);
        void update() override;
        void render() override;
        void init() override;

        gru::UISystem *uiSystem = nullptr;
        gru::SpriteBatch *uiSpriteBatch = nullptr;
        FBButton *buttonStart = nullptr;
        FBButton *buttonExit = nullptr;
        FBButton *buttonSettings = nullptr;
        FBButton *buttonMultiplayer = nullptr;
        bool startHovered = false;

    private:
        Animation* peasentIdleAnimation = nullptr;
        AnimationPlayer* animationPlayer = nullptr;
        int menuIndex=0;

    };

}






#endif //SIMPLE_KING_MAINMENUSTATE_H
