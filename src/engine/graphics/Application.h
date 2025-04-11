//
// Created by mgrus on 23.03.2025.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <complex>
#include <Windows.h>
#include <commctrl.h>
#include <memory>
#include <string>
#include <glm/glm.hpp>

#include <engine/graphics/RenderBackend.h>
#include <engine/graphics/RawWin32Message.h>
#include <engine/graphics/ui/MessageDispatcher.h>

#include "ui/CentralSubMenuManager.h"
#include "ui/CursorType.h"

namespace win32 {
    class WinMenu;
    class WinMenuBar;
}


struct NativeVSplitter {
    HWND left;
    HWND right;
};


class Logger;
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

    void setMainMenuBar(const std::shared_ptr<MenuBar> &mainMenuBar);

    HBITMAP loadBitmapFromFile(const std::wstring &fileName);

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

    int width();
    int height();

    std::shared_ptr<Widget> getMenuBar();

    void log(const std::string& msg);

    void createNativeToolbar();
    void addMainToolbarButton(const std::wstring& bitmapFileName);

    std::shared_ptr<win32::WinMenuBar> createNativeMenuBar();
    std::shared_ptr<win32::WinMenu> createNativeMenu(const std::string& name);
    HWND createNativePanel(glm::vec2 position, glm::vec2 size, int id, glm::ivec3 bgColor);
    HWND createNativeVSplitter(HWND hwnd, HWND main_3d_content_pane, int id);

    // Set toolbar to 0 if we don't have one.
    HWND createNativeTreeView(glm::ivec2 pos, glm::ivec2 size, int id);
    HTREEITEM addTreeItem(HWND treeViewer, const std::string text, HTREEITEM parent = TVI_ROOT, HTREEITEM insertAfter = TVI_LAST);

    HWND hwnd();

protected:

    // Gets called right after the successful construction, must be implemented by concrete
    // application subclasses.
    virtual void onCreated() = 0;

    // Assign the top level widget for this application.
    void setTopLevelWidget(const std::shared_ptr<Widget>& widget);

    // Allows subclasses to implement logic which needs to run every frame
    virtual void doFrame();

    bool temp_ignore_messages_ = false;
    HIMAGELIST main_toolbar_image_list_ = nullptr;

private:
    void clearClosedFloatingWindows();

    void mainLoop();



    static ::LRESULT CALLBACK AppWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    int height_ = -1;
    int width_ = -1;
    int fullscreen = 0;
    HINSTANCE h_instance_;
    HWND window_;
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
    HCURSOR hand_cursor_;
    uint64_t message_count = 0;
    std::shared_ptr<TrueTypeFont> font_;
    std::shared_ptr<MenuBar> main_menu_bar_;
    std::shared_ptr<Logger> logger_;
    std::stringstream log_stream_;

    static const int IDM_NEW=101;
    static const int IDM_OPEN=102;
    static const int IDM_SAVE=103;

    // The main toolbar
    // TODO wrap into reusable widget
    HWND hWndToolbar;

    // Native control wrappers
    std::shared_ptr<win32::WinMenuBar> native_main_menu_bar_;

};

std::shared_ptr<Application> getApplication();

#endif //APPLICATION_H
