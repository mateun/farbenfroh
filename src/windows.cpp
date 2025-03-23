#include <Windows.h>
#include <iostream>
#include <windowsx.h>
#include <stdio.h>
#include <GL/glew.h>
#include <opengl/wglext.h>
#include <inttypes.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <stdexcept>
#include <vector>
#include <XInput.h>
#include <engine/io/io.h>
#include <engine/vulkan/VulkanRenderer.h>


#include "../extlibs/imgui/imgui.h"
#include "../extlibs/imgui/imgui_impl_win32.h"
#include "../extlibs/imgui/imgui_impl_opengl3.h"
#include "engine/game/default_game.h"
#include "engine/editor/editor.h"
#include "engine/game/input.h"



#pragma comment(lib, "XInput.lib")



// This is called by the framework to receive the actual game from the user.
// All API methods will then be called on this game instance.
extern DefaultGame* getGame();

// This is implemented by the actual game itself.
// Gets called from within the windows messagepump loop.
extern bool gameLoop();

// This is implemented by the actual game.
// It gets called if the cmdline parameter "activeLoop" is set to true.
// Then this is the handoff point from here (windows framework) to the game.
// The game must then handle everything itself.
extern void gameInit(HWND, HINSTANCE);

// This is implemented normally in graphics.cpp
// and takes care of clearing all needed buffers,
// including eventual shadow map depth buffers etc.
extern void clearGLBuffers();

// This method creates the default shaders, quad vaos etc.
// This is all necessary to provide simple drawing features
// without any further setup from the game side.
// For drawing simple shapes, text etc.
extern void initDefaultGLObjects();

int window_width = 1280;
int window_height= 720;
int scaled_width = 1280;
int scaled_height = 720;
int dpiScaleFactor = 1;
bool fullscreen = false;
const char window_title[] = "Koenig 0.0.1";
void* backbufferBytes = nullptr;
float ftMicros = 0;
float ftMicrosAvg = 0;
float ftSeconds = 0;
float ftSecondsAvg = 0;
int currentFrame = 0;
int lastAvgFPS = 0;
bool gldirect = true;
int mouse_x = 0;
int mouse_y = 0;
int mouse_rel_x = 0;
int mouse_rel_y = 0;
int mouse_last_rel_x = 0;
int mouse_last_rel_y = 0;
bool lbuttonUp = false;
bool lbuttonDown = false;
bool windowClosed = false;
bool useMouse = true;
WPARAM lastKeyPress = 0;
static HWND window;
int64_t performance_frequency = 0;
static LARGE_INTEGER freq = {};

static DefaultGame* game = nullptr;

ImFont* boldFont = nullptr;

// Controller (GamePad) states.
// We need the previous state to enable single button press detection.
std::vector<XINPUT_STATE> controllerStates;
std::vector<XINPUT_STATE> prevControllerStates;

HWND getWindow() {
    return window;
}

void mouseVisible(bool val) {
    useMouse = val;
}

UINT GetDpi(HWND hWnd) {
    UINT dpi = 96; // Default DPI is 96 (100%)
    HMODULE hUser32 = LoadLibrary(TEXT("User32.dll"));
    if (hUser32) {
        auto pGetDpiForWindow = (UINT(WINAPI*)(HWND))GetProcAddress(hUser32, "GetDpiForWindow");
        if (pGetDpiForWindow) {
            dpi = pGetDpiForWindow(hWnd);
        }
        FreeLibrary(hUser32);
    }
    return dpi;
}

void initXInput() {
    controllerStates.clear();
    controllerStates.resize(XUSER_MAX_COUNT);
    prevControllerStates.clear();
    prevControllerStates.resize(XUSER_MAX_COUNT);
}


static bool pollController(int index) {
    XINPUT_STATE state;
    auto result = XInputGetState(index, &state);
    if (result == ERROR_SUCCESS) {
        controllerStates[index] = state;
    }
    return result == ERROR_SUCCESS;
}

static void postPollController(int index) {
    prevControllerStates[index] = controllerStates[index];
}

float getControllerAxis(ControllerAxis axis, int index) {

    if (axis == ControllerAxis::LSTICK_X) {
        return controllerStates[index].Gamepad.sThumbLX / 32767.0f;
    } if ( axis == ControllerAxis::LSTICK_Y) {
        return controllerStates[index].Gamepad.sThumbLY / 32767.0f;
    } if ( axis == ControllerAxis::RSTICK_X) {
        return controllerStates[index].Gamepad.sThumbRX / 32767.0f;
    } if ( axis == ControllerAxis::RSTICK_Y) {
        return controllerStates[index].Gamepad.sThumbRY / 32767.0f;
    } if ( axis == ControllerAxis::L_TRIGGER) {
        return controllerStates[index].Gamepad.bLeftTrigger / 255.0f;
    } if ( axis == ControllerAxis::R_TRIGGER) {
        return controllerStates[index].Gamepad.bRightTrigger / 255.0f;
    }
    return 0;

}

bool controllerButtonPressed(ControllerButtons button, int controllerIndex) {

    auto xiButton = XINPUT_GAMEPAD_A;

    if (button == ControllerButtons::B_BUTTON) {
        xiButton = XINPUT_GAMEPAD_B;
    }
    else if (button == ControllerButtons::X_BUTTON) {
        xiButton = XINPUT_GAMEPAD_X;
    }
    else if (button == ControllerButtons::Y_BUTTON) {
        xiButton = XINPUT_GAMEPAD_Y;
    }
    else if (button == ControllerButtons::DPAD_DOWN) {
        xiButton = XINPUT_GAMEPAD_DPAD_DOWN;
    }
    else if (button == ControllerButtons::DPAD_UP) {
        xiButton = XINPUT_GAMEPAD_DPAD_UP;
    }
    else if (button == ControllerButtons::DPAD_RIGHT) {
        xiButton = XINPUT_GAMEPAD_DPAD_RIGHT;
    }
    else if (button == ControllerButtons::DPAD_LEFT) {
        xiButton = XINPUT_GAMEPAD_DPAD_LEFT;
    }
    else if (button == ControllerButtons::MENU) {
        xiButton = XINPUT_GAMEPAD_START;
    }
    else if (button == ControllerButtons::VIEW) {
        xiButton = XINPUT_GAMEPAD_BACK;
    }
    else if (button == ControllerButtons::LB) {
        xiButton = XINPUT_GAMEPAD_LEFT_SHOULDER;
    }
    else if (button == ControllerButtons::RB) {
        xiButton = XINPUT_GAMEPAD_RIGHT_SHOULDER;
    }

    bool wasPressed = (prevControllerStates[controllerIndex].Gamepad.wButtons & xiButton) != 0;
    bool isPressed = (controllerStates[controllerIndex].Gamepad.wButtons & xiButton) != 0;

    if (!wasPressed && isPressed) {
        return true;
    }

    return false;


}

/**
 * Retrieves a list of all active monitors in the system.
 *
 * @return The list of deviceNames of each active monitor.
 */
std::vector<std::string> getAllActiveMonitors() {
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(dd);
    int deviceIndex = 0;

    std::vector<std::string> activeMonitorNames;

    while (EnumDisplayDevices(nullptr, deviceIndex, &dd, 0)) {
        std::wcout << L"Monitor " << deviceIndex << L": " << dd.DeviceName << L" - " << dd.DeviceString << std::endl;
        if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE) {
            //std::wcout << L"  (Active)" << std::endl;

        }
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            //std::wcout << L"  (Primary Monitor)" << std::endl;
            activeMonitorNames.push_back(dd.DeviceName);
        }
        deviceIndex++;
    }

    return activeMonitorNames;
}

std::vector<MonitorResolution> getMonitorResolutions(const std::string& deviceName) {
    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);
    int modeIndex = 0;

    std::vector<MonitorResolution> resolutions;

    //std::cout << "Available resolutions for " << deviceName << ":" << std::endl;
    while (EnumDisplaySettings(deviceName.c_str(), modeIndex++, &devMode)) {
//        std::wcout << L"  Resolution: " << devMode.dmPelsWidth << L"x" << devMode.dmPelsHeight
//                   << L" @ " << devMode.dmDisplayFrequency << L" Hz" << std::endl;

        resolutions.push_back({devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency,devMode.dmBitsPerPel});
    }

    return resolutions;

}

bool changeResolution(int width, int height, int refreshRate, const std::string& deviceName, bool goFullscreen) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    auto dpi = GetDpiForWindow(window);
    dpiScaleFactor = static_cast<float>(dpi) / 96.0f;
    scaled_width = width / dpiScaleFactor;
    scaled_height = height / dpiScaleFactor;

    window_width = width;
    window_height = height;

    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmDisplayFrequency = refreshRate;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

    // Apply the new display settings
    if (goFullscreen) {
        if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            MessageBox(nullptr, "Failed to change display settings", "Error", MB_OK | MB_ICONERROR);
            return false;
        }

        // Remove window borders and make it topmost
        SetWindowLong(window, GWL_STYLE, WS_POPUP);
        SetWindowPos(window, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
        ShowWindow(window, SW_MAXIMIZE);

        // Update our internal window size variables:

        fullscreen = true;

        return true;

    }
    // TODO: currently nothing done if none fullscreen change. then we assume we are developing
    //  and just stay with the bordered window.
    return true;

}

void exitFullscreen() {
    // Restore the display settings to default
    ChangeDisplaySettings(nullptr, 0);

    // Restore the window style
    SetWindowLong(window, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowPos(window, nullptr, 100, 100, window_width, window_height, SWP_FRAMECHANGED | SWP_NOZORDER); // example position and size
    ShowWindow(window, SW_NORMAL);
}

std::string GetArgumentValue(const std::string& argumentName, int argc, char* argv[])
{
    std::string prefix = argumentName + "=";
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.find(prefix) == 0)
        {
            return arg.substr(prefix.length());
        }
    }
    return "";
}

void enableVsync(bool enable) {
	((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(enable);
}

void initGLContext(HWND hwnd, HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    // 1. Register a dummy window class
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_OWNDC;                  // get our own DC for the window
    wc.lpfnWndProc   = DefWindowProcA;            // a trivial wndproc
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "Dummy_WGL";

    if (!RegisterClassA(&wc)) {
        // handle error
    }

    // 2. Create the hidden (dummy) window
    HWND dummyWnd = CreateWindowA(
        "Dummy_WGL",            // class name
        "Dummy OpenGL Window",  // window name
        WS_OVERLAPPEDWINDOW,    // style
        CW_USEDEFAULT, CW_USEDEFAULT, // position
        CW_USEDEFAULT, CW_USEDEFAULT, // size
        NULL,                   // parent
        NULL,                   // menu
        wc.hInstance,
        NULL
    );
    if (!dummyWnd) {
        // handle error
        throw std::runtime_error("Failed to create window");
    }

    // 3. Get the DC for the dummy window
    HDC dummyDC = GetDC(dummyWnd);

    int windowsChosenFormat = ChoosePixelFormat(dummyDC, &pfd);
    SetPixelFormat(dummyDC, windowsChosenFormat, &pfd);

    HGLRC baseContext = wglCreateContext(dummyDC);
    BOOL ok = wglMakeCurrent (dummyDC, baseContext);
    if (!ok) {
        printf("error");
        exit(1);
    }

    int gl46_attribs[] =
            {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 6,
                    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    0,
            };


    //HGLRC wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList)
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    auto createContextAttribsProc = (PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress("wglCreateContextAttribsARB");

    int pixelAttribs[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE, // <--- Enable sRGB framebuffer
        WGL_SAMPLE_BUFFERS_ARB, 1,      // <-- Enable multi-sample buffer
        WGL_SAMPLES_ARB,        4,      // <-- # of samples
        0                       // End
    };

    int pixelFormat;
    UINT numFormats; // will receive how many formats match

    // Find a pixel format that matches our criteria
    bool success = wglChoosePixelFormatARB(hdc, pixelAttribs, nullptr, 1,
                                           &pixelFormat, &numFormats);

    if ( ! success || numFormats ==0 ) {
        throw std::runtime_error("Failed to create OpenGL context with MSAA 4x");
    }

    HDC ourWindowHandleToDeviceContext = GetDC(hwnd);
    SetPixelFormat(ourWindowHandleToDeviceContext, pixelFormat, &pfd);

    auto coreRenderContext = createContextAttribsProc(hdc, nullptr, gl46_attribs);
    wglDeleteContext(baseContext);
    ok = wglMakeCurrent(ourWindowHandleToDeviceContext, coreRenderContext);
    if (!ok) {
        printf("error");
        exit(1);
    }

    // Initialize GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        // GLEW initialization failed
        fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(err));
        exit(1);
    }


    const GLubyte *GLVersionString = glGetString(GL_VERSION);
    char buf[200];
    sprintf_s(buf, "gl version: %s\n", GLVersionString);
    OutputDebugStringA(buf);
    printf("%s", buf);

#ifdef RENDER2D
    // Setup the fullscreen texture
    BITMAPINFO bbInfo;
    bbInfo.bmiHeader.biBitCount = 32;
    bbInfo.bmiHeader.biCompression = BI_RGB;
    bbInfo.bmiHeader.biPlanes = 1;
    bbInfo.bmiHeader.biHeight = window_height;
    bbInfo.bmiHeader.biWidth = window_width;
    bbInfo.bmiHeader.biSize = sizeof(bbInfo.bmiHeader);
    bbInfo.bmiHeader.biSizeImage = 0;
    bbInfo.bmiHeader.biXPelsPerMeter = 0;
    bbInfo.bmiHeader.biYPelsPerMeter = 0;
    bbInfo.bmiHeader.biClrUsed = 0;
    bbInfo.bmiHeader.biClrImportant = 0;

    int bytesPerPixel = 4;
    int bitmapMemorySize = bbInfo.bmiHeader.biWidth * bbInfo.bmiHeader.biHeight * bytesPerPixel;
    backbufferBytes = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, backbufferBytes);

    // Onetime ortho projection setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ar = (float) window_height / (float) window_width;
    glOrtho(0, window_width, 0, window_height, 0, 1);
    glViewport(0, 0, window_width, window_height);
    glEnable(GL_TEXTURE_2D);

    auto err = glGetError();
    if (err != 0) {
        printf("gl error: %d\n", err);
        exit(err);
    }

#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_FRAMEBUFFER_SRGB);
    //glEnable(GL_POLYGON_SMOOTH);
    glViewport(0, 0, window_width, window_height);
    enableVsync(false);

    GLint samples = 0, sampleBuffers = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
    printf("MSAA: %d sample buffers, %d samples\n", sampleBuffers, samples);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();

    io.Fonts->AddFontDefault(); // Default font
    boldFont = io.Fonts->AddFontFromFileTTF("../assets/BowlbyOneSC-Regular.ttf", 16.0f);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

    HDC hdc;
    RECT rect;
    TEXTMETRIC tm;
    PAINTSTRUCT ps;

   // lbuttonUp = false;

    static LONGLONG lastTick = 0;

    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam)) {
        return true;
    }


    switch (message) {

        case WM_CLOSE:
        if (game) delete game;
        game = nullptr;
        DestroyWindow(hwnd);
        windowClosed = true;
        break;

        case WM_SETFOCUS:
            // Hide the cursor when the window gains focus
            ShowCursor(useMouse);
            return 0;

        case WM_KILLFOCUS:
            ShowCursor(useMouse);
            return 0;

        // For raw mouse input
        case WM_INPUT:
        {
            UINT dwSize;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if (lpb == NULL) {
                return 0;
            }

            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
                delete[] lpb;
                return 0;
            }

            RAWINPUT* raw = (RAWINPUT*)lpb;
            if (raw->header.dwType == RIM_TYPEMOUSE) {

                mouse_rel_x = raw->data.mouse.lLastX;
                mouse_rel_y = raw->data.mouse.lLastY;


            }

            delete[] lpb;
            return 0;
        }

        case WM_MOUSEMOVE:
            if (useMouse) {
                mouse_x = GET_X_LPARAM(lParam);
                mouse_y = window_height - GET_Y_LPARAM(lParam);
            }

        break;

        case WM_LBUTTONDOWN:
            lbuttonDown = true;
            break;

        case WM_LBUTTONUP:
            lbuttonUp = true;
            lbuttonDown = false;
            break;
        case WM_SYSKEYDOWN:
            lastKeyPress = wParam;

        case WM_KEYDOWN:
            lastKeyPress = wParam;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            ShowCursor(TRUE);
            break;

        case WM_PAINT: {
            hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_SETCURSOR:{
            if (!useMouse) {
                SetCursor(nullptr); // Hide the cursor when over this window
            }
            return TRUE;
//            if (LOWORD(lParam) == HTCLIENT) {
//                SetCursor(NULL);
//                return TRUE;
//            }
        }
        default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void present3D(HDC hdc) {
    SwapBuffers(hdc);
}

void present(HDC hdc) {
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, backbufferBytes);
	auto err = glGetError();
	if (err != 0) {
		exit(err);
	}

	auto width = window_width;
	auto height = window_height;


    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 0); glVertex3f(0.f, 0.f, -.10f);
    glTexCoord2f(1, 0); glVertex3f(width, 0.f, -.10f);
    glTexCoord2f(1, 1); glVertex3f(width, height, -.10f);

    glTexCoord2f(0, 0); glVertex3f(0.f, 0.f, -.10f);
    glTexCoord2f(1, 1); glVertex3f(width, height, -.10f);
    glTexCoord2f(0, 1); glVertex3f(0.f, height, -.10f);

    glEnd();
    glFlush();

    SwapBuffers(hdc);
}
// We support Windows VK_ macros here.
bool keyPressed(int key) {
//    return GetAsyncKeyState(key) & 0x01;
    return key == lastKeyPress;
}

bool isKeyDown(int key) {
    return GetKeyState(key) & 0x8000;
}

// Necessary to keep our program going
// and interact with windows.
bool processOSMessages() {
    MSG msg;
    bool shouldRun = true;

    // Process EVERY message per frame.
    // If this is too many (e.g. due to high frequency mouse or other HID)
    // we need to clamp the max. messages per frame.
    // But NEVER we should process 1 message per frame.
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
        if (msg.message == WM_QUIT) {
            shouldRun = false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return shouldRun;

}


void mainLoop(HWND hwnd, VulkanRenderer* vulkanRenderer) {
	LARGE_INTEGER startticks;
	LARGE_INTEGER endticks;

    initXInput();

	auto hdc = GetDC(hwnd);
	bool running = true;

    // We store the last 1000 entries in our frametime buffer
    // and then calculate the average fps in there.
    static std::vector<float> ftbuffer(1000);
    static std::vector<float> ftMicrosbuffer(1000);

	MSG msg;
	while (running && !windowClosed) {
        QueryPerformanceCounter(&startticks);
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

	    if (vulkanRenderer) {
	        vulkanRenderer->clearBuffers();
	    } else if (gldirect) {
            clearGLBuffers();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
        } else {
            // Clear the backbuffer to black
            // 64 bytes on each loop so we are quicker/less iterations
            auto* bb64 = static_cast<uint64_t *>(backbufferBytes);
            for (int i = 0; i < (window_width*window_height)/2; i++) {
                *bb64 = 0;
                *bb64++;
            }
        }


	    // We might have been deleted async
	    // by the user closing the main window.
	    if (game) {
            running = game->shouldStillRun();
            pollController(0);
            game->update();
            postPollController(0);
            game->render();
	    }

        lastKeyPress = 0;
        lbuttonUp = false;




        if (vulkanRenderer) {
            vulkanRenderer->drawFrame();
        } else if (gldirect) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            present3D(hdc);
        }
	    else {
            present(hdc);
        }

        QueryPerformanceCounter(&endticks);
        auto ticks = endticks.QuadPart - startticks.QuadPart;
        ftSeconds = ticks / (float) freq.QuadPart;
        ftMicros = (float) ftSeconds * 1000.0f * 1000.0f;
	    currentFrame++;

        // Calculate average frametimes and fps.
	    // Every 1000 frames we take the average.
	    // Clear the buffer and collect the next 1000 frames for averaging.
	    ftbuffer.push_back(ftSeconds);
	    ftMicrosbuffer.push_back(ftMicros);
	    if (ftbuffer.size() > 999) {
	        float sum = 0;
	        float sumMicros = 0;
	        for (int i = 0; i < ftbuffer.size(); i++) {
	            sum += ftbuffer[i];
	            sumMicros += ftMicrosbuffer[i];
	        }
	        ftMicrosAvg = sumMicros / ftbuffer.size();
	        ftSecondsAvg = sum / ftbuffer.size();
	        lastAvgFPS = 1/ftSecondsAvg;
	        ftbuffer.clear();
	        ftMicrosbuffer.clear();
	    }

	}

}

void shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

#define USE_VULKAN
#ifdef USE_VULKAN
VkInstance createVulkanInstance(HINSTANCE hInst, HWND hwnd) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Game";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "FerkEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "Available extensions:\n";
    for (const auto& ext : extensions) {
        std::cout << "\t" << ext.extensionName << "\n";
    }

    std::vector<const char*> extensionsRequested;
    extensionsRequested.push_back(VK_KHR_SURFACE_EXTENSION_NAME); // Always required for surface creation
    extensionsRequested.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    // Validation layers
    const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
    };
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionsRequested.size();
    createInfo.ppEnabledExtensionNames = extensionsRequested.data();
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validationLayers.data();

    // Instance creation
    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        exit(1);
    }

    // Device selection
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        exit(1);
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    vkGetPhysicalDeviceProperties(devices[0], &physicalDeviceProperties);
    // Print out the physical props of our first device
    {
        printf("device name: %s\n", physicalDeviceProperties.deviceName);
        printf("max clip distances: %u\n", physicalDeviceProperties.limits.maxClipDistances);
        auto minorVersion = VK_API_VERSION_MINOR(physicalDeviceProperties.apiVersion);
        printf("minor version: %u\n", minorVersion);
    }


    // We select the first device in the system for our use:
    auto device = devices[0];



    // Queues are where the real work takes place.
    // QueueFamilies show which capabilities are available, and per family, we can have n queues to
    // actually carry out the work.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamilyIndex = 0;
    for (const auto& queueFamily : queueFamilies) {

        // We just look for the graphics capability.
        // But just for fun we also look for GPU_COMPUTE caps:
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            printf("can compute as well!");
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            if (vkGetPhysicalDeviceWin32PresentationSupportKHR(device, queueFamilyIndex)) {
                break;
            } else {
                exit(2);
            }

        }

        queueFamilyIndex++;
    }



    auto queueFamilyWithGraphicsCaps = queueFamilies[queueFamilyIndex];
    auto queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo logicalDeviceCreateInfo{};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    logicalDeviceCreateInfo.queueCreateInfoCount = 1;
    logicalDeviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    logicalDeviceCreateInfo.enabledExtensionCount = 1;
    logicalDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkDevice logicalDevice;
    if (vkCreateDevice(device, &logicalDeviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
        exit(1);
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(logicalDevice, queueFamilyIndex, 0, &graphicsQueue);

    // Surface creation
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hwnd = hwnd;
    surfaceCreateInfo.hinstance = hInst;

    VkSurfaceKHR surface;
    if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS) {
        exit(1);
    }

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR > surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats.data());
    VkSurfaceFormatKHR goodFormat;
    for (auto f : surfaceFormats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            printf("found good format format");
            goodFormat = f;
            break;
        }

    }


    // We just assume for now our surface has presentation support:
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, surface, &presentSupport);
    if (presentSupport == VK_FALSE) {
        exit(1);
    }
    VkQueue presentQueue;
    float queuePrio = 1.0;
    VkDeviceQueueCreateInfo presentQueueCreateInfo{};
    presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    presentQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    presentQueueCreateInfo.queueCount = 1;
    presentQueueCreateInfo.pQueuePriorities = &queuePrio;

    vkGetDeviceQueue(logicalDevice, queueFamilyIndex, 0, &presentQueue);

    // Command Pool creation
    VkCommandPool commandPool;
    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    if (vkCreateCommandPool(logicalDevice, &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        exit(1);
    }

    // Command buffer creation
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo bufferAllocateInfo = {};
    bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocateInfo.commandPool = commandPool;
    bufferAllocateInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(logicalDevice, &bufferAllocateInfo, &commandBuffer) != VK_SUCCESS) {
        exit(2);
    }

    // Fences creation
    // To wait for a given command buffer on the GPU
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS) {
        exit(3);
    }

    // Creating a swap chain
    VkSurfaceFormatKHR surfaceFormat = {};
    surfaceFormat.format = goodFormat.format;
    surfaceFormat.colorSpace = goodFormat.colorSpace;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D extent = {};
    extent.width = window_width;
    extent.height = window_height;
    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = surface;
    swapChainInfo.imageFormat = goodFormat.format;
    swapChainInfo.imageColorSpace = goodFormat.colorSpace;
    swapChainInfo.minImageCount = 2;
    swapChainInfo.imageArrayLayers = surfaceFormat.format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = extent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = presentMode;
    swapChainInfo.clipped = VK_TRUE;
    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(logicalDevice, &swapChainInfo, nullptr, &swapChain) != VK_SUCCESS) {
        exit(4);
    }

    // We aquire the actual images into which we render. First we find out, how many images have been created
    // in our swapchain.
    uint32_t swapChainImageCount = 0;
    if (vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, nullptr) != VK_SUCCESS) {
        exit(5);
    }
    // Now actually retrieve the images.
    std::vector<VkImage> swapChainImages(swapChainImageCount);
    swapChainImages.resize(swapChainImageCount);
    if (vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, swapChainImages.data()) != VK_SUCCESS) {
        exit(6);
    }

    // In order to render (write) into an image, we must aquire it.
    uint32_t imageIndex = 0;
    if (vkAcquireNextImageKHR(logicalDevice, swapChain, 100, {}, {}, &imageIndex) != VK_SUCCESS) {
        exit(7);
    }

    // TODO actual rendering pipeline
    // Create our first shader object
    uint32_t lengthInBytes = 0;
    auto shaderCode = readFileBinary("../assets/vulkan_shaders/simple.spv", &lengthInBytes);
    if (!shaderCode) {
        exit(8);
    }
    auto shaderCreateInfo = VkShaderModuleCreateInfo();
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.pNext = nullptr;
    shaderCreateInfo.flags = 0;
    shaderCreateInfo.codeSize = lengthInBytes;
    shaderCreateInfo.pCode = reinterpret_cast<uint32_t *>(shaderCode);

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        exit(9);
    }


    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.pNext = nullptr;
    pipelineCacheCreateInfo.flags = 0;
    pipelineCacheCreateInfo.initialDataSize = 0;
    pipelineCacheCreateInfo.pInitialData = nullptr;

    // Layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.setLayoutCount = 1;

    std::vector<VkComputePipelineCreateInfo> pipelineCreateInfos;
    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.flags = 0;
    computePipelineCreateInfo.layout = {};
    computePipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computePipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipelineCreateInfo.stage.pNext = nullptr;
    computePipelineCreateInfo.stage.module = shaderModule;
    computePipelineCreateInfo.stage.pName = "main";
    computePipelineCreateInfo.stage.pSpecializationInfo = nullptr;
    pipelineCreateInfos.push_back(computePipelineCreateInfo);

    VkPipelineCache pipelineCache;
    vkCreatePipelineCache(logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache);

    std::vector<VkPipeline> computePipelines;
    computePipelines.resize(1);
    if (vkCreateComputePipelines(logicalDevice, pipelineCache, 1, pipelineCreateInfos.data(), nullptr, computePipelines.data()) != VK_SUCCESS) {
        exit(10);
    }

    return instance;
}
#endif

void registerRawInput(HWND hwnd)
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic desktop controls
    rid.usUsage = 0x02;     // Mouse
    rid.dwFlags = RIDEV_INPUTSINK; // Receive input even when not in focus
    rid.hwndTarget = hwnd;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        // Handle the error
        MessageBox(NULL, "Failed to register raw input device.", "Error", MB_ICONERROR);
    }
}



void _initDX11(HWND hwin, HINSTANCE hInst) {

}


#ifdef USE_RAW_WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
			HINSTANCE hPrevInstance,
			LPSTR lpCmdLine,
			int nShowCmd) {


    QueryPerformanceFrequency(&freq);
    performance_frequency = freq.QuadPart;

    VulkanRenderer* vulkanRenderer = nullptr;

    // Allocate our game
    game = getGame();
    auto editorFlag = GetArgumentValue("editor", __argc, __argv);
    if ( editorFlag == "true" ) {
        game = new editor::EditorGame();
        window_width = 1920;
        window_height= 900;

    }



#ifdef _WITH_CONSOLE
    // Allocate a dedicated console and redirect stdout.
    // Should only be done for debugging purposes, not in the final production build.
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

    // Window setup
    const char g_szClassName[] = "winClass";
    WNDCLASSEX wc;
    wc.lpszMenuName = NULL;
    wc.hInstance = hInstance;

    wc.lpszClassName = g_szClassName;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    //wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hCursor = NULL;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.lpfnWndProc = WndProc;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        exit(1);
    }

    RECT corrRect = {0, 0, window_width, window_height};
    AdjustWindowRect(&corrRect, WS_OVERLAPPEDWINDOW, false);

    auto winWidthHalf = (corrRect.right  - corrRect.left) / 2;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);


    HWND hwnd = CreateWindow(
			 g_szClassName,
             (game->getName() + " (" + game->getVersion() + ")").c_str(),
			 //WS_POPUP,
			 WS_OVERLAPPEDWINDOW,
			 screenWidth/2 - winWidthHalf, 0, corrRect.right - corrRect.left, corrRect.bottom - corrRect.top,
			 NULL, NULL, hInstance, NULL);

    window = hwnd;

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, SW_NORMAL);
    UpdateWindow(hwnd);
    HDC hdc = GetDC(hwnd);
    registerRawInput(hwnd);

    if (editorFlag == "true") {
        changeResolution(1920, 900, 60, "foo");
    }

    // Check if this game provides its own loop, i.e. is the driver of the loop.
    // In this case  we only initialize the game and leave it to the game to take care of everything else.
    // If false, we provide the main loop within the framework and callback into the game for updates and renders
    // during each frame.
    std::string activeLoop = GetArgumentValue("activeLoop", __argc, __argv);
    if (activeLoop == "true") {
        // Not much help here from the framework. The window is created,
        // now it's up to the game to handle everything else.
        // No renderer is provided, all must be done by the game itself.
        // We just call the games init method here.
        auto game = getGame();
        game->init();

    } else {
        // We are in the managed mode here. Some default renderer backends are supported:
        // - DX11
        // - OpenGL 4.5
        // - Vulkan 1.3
        std::string renderer = GetArgumentValue("renderer", __argc, __argv);
        if (renderer == "dx11") {
            _initDX11(hwnd, hInstance);
        }
        else if (renderer == "opengl" || renderer == "") {
            initGLContext(hwnd, hdc);
            initDefaultGLObjects();
        }
        else if (renderer == "vulkan") {

#ifdef USE_VULKAN
            //createVulkanInstance(hInstance, hwnd);
            vulkanRenderer = new VulkanRenderer(hInstance, hwnd);
#endif
        }

        game->init();
        mainLoop(hwnd, vulkanRenderer);
    }


    return 0;

}
#endif

