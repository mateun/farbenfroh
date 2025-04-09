//
// Created by mgrus on 23.03.2025.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <complex>
#include <Windows.h>
#include <memory>
#include <string>
#include <glm/glm.hpp>

#include <engine/graphics/RenderBackend.h>
#include <engine/graphics/RawWin32Message.h>
#include <engine/graphics/ui/MessageDispatcher.h>

#include "ui/CentralSubMenuManager.h"
#include "ui/CursorType.h"

class TrueTypeFont;
class FloatingWindow;
class FocusManager;
class Widget;
class FrameMessageSubscriber;


class Application {
  public:
    Application(int w, int h, bool fullscreen);
    ~Application();


    void initialize(HINSTANCE hinstance, HINSTANCE h_prev_instance, LPSTR lpstr, int n_show_cmd);
    int run();

    bool changeResolution(int width, int height, int refreshRate, const std::string &deviceName, bool goFullscreen);
    std::vector<RawWin32Message> getLastMessages();

    RenderBackend* getRenderBackend() const;

    void addMessageSubscriber(std::shared_ptr<FrameMessageSubscriber> subscriber);

    int scaled_width();
    int scaled_height();

    std::shared_ptr<Widget> getTopLevelWidget();

    // This lets the application code set the desired windows cursor (e.g. resize cursor)
    // and not have it reset automatically to the default pointer cursor immediately.
    void setAllowCursorOverride(bool allow);
    bool allowCursorOverride();

    void setSpecialCursor(CursorType cursor);

    // This makes the application use the default pointer arrow cursor when in the main windows
    // client area.
    void unsetSpecialCursor();

    std::shared_ptr<CentralSubMenuManager> getCentralSubMenuManager();

    std::shared_ptr<FocusManager> getFocusManager();

    std::vector<std::shared_ptr<FloatingWindow>> getFloatingWindows();

    glm::vec2 getCurrentMousePos();

    void addFloatingWindow(std::shared_ptr<FloatingWindow> window);
    void removeFloatingWindow(std::shared_ptr<FloatingWindow> floating_window);

    std::shared_ptr<TrueTypeFont> getDefaultMenuFont();

protected:

    // Gets called right after the successful construction, must be implemented by concrete
    // application subclasses.
    virtual void onCreated() = 0;

    void setTopLevelWidget(const std::shared_ptr<Widget>& widget);



protected:
    virtual void doFrame();

private:
    void clearClosedFloatingWindows();

    void mainLoop();



    static LRESULT CALLBACK AppWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    int height_ = -1;
    int width_ = -1;
    int fullscreen = 0;
    HWND _window;
    HDC hdc;
    std::shared_ptr<Widget> topLevelWidget;
    std::vector<std::shared_ptr<FloatingWindow>> floating_windows_;
    std::vector<std::shared_ptr<FloatingWindow>> floating_windows_closed_;
    std::unique_ptr<RenderBackend> render_backend_;
    int scaled_width_ = -1;
    int scaled_height_ = -1;
    std::vector<RawWin32Message> frame_messages_;
    std::vector<std::shared_ptr<FrameMessageSubscriber>> messageSubscribers;
    std::shared_ptr<FocusManager> focus_manager_;
    std::shared_ptr<FocusBasedMessageDispatcher> focus_based_message_dispatcher_;
    std::shared_ptr<SimpleMessageDispatcher> simple_message_dispatcher_;
    std::shared_ptr<CentralSubMenuManager> central_submenu_manager_;
    bool allow_cursor_override_ = false;
    HCURSOR resize_cursor_horizontal_;
    HCURSOR resize_cursor_vertical_;
    HCURSOR text_edit_cursor;
    uint64_t message_count = 0;
    std::shared_ptr<TrueTypeFont> font_;
};

std::shared_ptr<Application> getApplication();

#endif //APPLICATION_H
