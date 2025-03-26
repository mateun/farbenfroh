//
// Created by mgrus on 25.03.2025.
//

#ifndef INPUT_H
#define INPUT_H

#include <cinttypes>
#include <engine/graphics/Application.h>

enum class ControllerAxis {
    LSTICK_X,
    LSTICK_Y,
    RSTICK_X,
    RSTICK_Y,
    R_TRIGGER,
    L_TRIGGER

};

enum class ControllerButtons {
    A_BUTTON,
    B_BUTTON,
    X_BUTTON,
    Y_BUTTON,
    RB,
    LB,
    DPAD_DOWN,
    DPAD_UP,
    DPAD_RIGHT,
    DPAD_LEFT,
    MENU,
    VIEW
};



class Input {
    friend class Application;

  public:
    virtual ~Input() = default;

    static Input* getInstance();

    virtual bool wasKeyPressed(int key) const = 0;

    // This might issue some OS calls to get the latest mouse positions etc.
    // Should only be done once per frame.
    void update();

    static bool isKeyDown(char key);
    int mouse_x();
    int mouse_y();



    // Must be called every frame for each controller index:
    virtual bool pollController(int index) = 0;

    // Has to be called at the end of all single press controller queries to
    // avoid longer running detections:
    virtual void postPollController(int index) = 0;

    virtual float getControllerAxis(ControllerAxis axis, int index) = 0;
    virtual bool controllerButtonPressed(ControllerButtons button, int controllerIndex) = 0;


protected:
    virtual void init() = 0;
    virtual void updateLastKeyPress(uint32_t key_code) = 0;


    int mouse_x_ = 0;
    int mouse_y_ = 0;
    int lastKeyPressed_ = 0;

private:
    Input* instance;


};






#endif //INPUT_H
