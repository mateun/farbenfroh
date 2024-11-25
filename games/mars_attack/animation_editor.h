//
// Created by mgrus on 13.01.2024.
//

#ifndef SIMPLE_KING_ANIMATION_EDITOR_H
#define SIMPLE_KING_ANIMATION_EDITOR_H

// This indicates a currently open menu
// This is a flat list, so the game must know
// that some items are in fact nested.
enum class MenuState {
    None,
    File,
    Edit,
    Import,
    Export,
    New,
    NewMap,
    Object,
    NewObject,

};

struct AnimEditorState {
    MenuState menuState = MenuState::None;
    MenuState prevMenuState = MenuState::None;
    MenuState lastMenuClicked = MenuState::None;
    bool menuIsClicked = false;

};


void updateAnimEditor();
void renderAnimEditor();

#endif //SIMPLE_KING_ANIMATION_EDITOR_H
