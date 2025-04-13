//
// Created by mgrus on 13.04.2025.
//

#include <windows.h>
#include <commctrl.h>

// Control IDs.
#define ID_EDIT_GAME_NAME         101
#define ID_EDIT_PROJECT_LOCATION  102
#define ID_BUTTON_BROWSE          103
#define ID_BUTTON_SAVE            104

// Window Procedure for our popup dialog.
static LRESULT CALLBACK NewGameDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            // "Game Name:" Label.
            CreateWindowEx(0, "STATIC", "Game Name:", WS_CHILD | WS_VISIBLE,
                10, 10, 80, 20,
                hwnd, NULL, hInst, NULL);

            // Game Name Edit control.
            CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                100, 10, 280, 20,
                hwnd, (HMENU)ID_EDIT_GAME_NAME, hInst, NULL);

            // "ProjectLocation:" Label.
            CreateWindowEx(0, "STATIC", "ProjectLocation:", WS_CHILD | WS_VISIBLE,
                10, 40, 80, 20,
                hwnd, NULL, hInst, NULL);

            // Project Location Edit control.
            CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                100, 40, 200, 20,
                hwnd, (HMENU)ID_EDIT_PROJECT_LOCATION, hInst, NULL);

            // Browse button (to select directory).
            CreateWindowEx(0, "BUTTON", "...",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                310, 40, 70, 20,
                hwnd, (HMENU)ID_BUTTON_BROWSE, hInst, NULL);

            // Save button.
            CreateWindowEx(0, "BUTTON", "Save",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                310, 80, 70, 25,
                hwnd, (HMENU)ID_BUTTON_SAVE, hInst, NULL);
            break;
        }

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case ID_BUTTON_BROWSE:
                {
                    // You could use SHBrowseForFolder here.
                    MessageBox(hwnd, "Browse directory clicked!", "Info", MB_OK);
                    break;
                }
                case ID_BUTTON_SAVE:
                {
                    // Retrieve text from the edit controls.
                    char gameName[256] = {0};
                    char projectLocation[256] = {0};
                    GetWindowText(GetDlgItem(hwnd, ID_EDIT_GAME_NAME), gameName, sizeof(gameName));
                    GetWindowText(GetDlgItem(hwnd, ID_EDIT_PROJECT_LOCATION), projectLocation, sizeof(projectLocation));

                    // Process and/or save the project settings.
                    MessageBox(hwnd, "Save clicked!", "Info", MB_OK);
                    break;
                }
                default:
                    break;
            }
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void createNewGameDialog(HINSTANCE hInstance, HWND parentWindow) {

    // Register a new window class for the popup dialog if not already done.
    static bool isRegistered = false;
    if (!isRegistered)
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc   = NewGameDialogProc;
        wc.hInstance     = hInstance;
        wc.lpszClassName = "PopupDialogClass";
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        isRegistered = true;
    }

    // Create the popup dialog, specifying hMainWnd as the parent.
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,          // Extended styles.
        "PopupDialogClass",           // Registered class name.
        "Project Settings",           // Window title.
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,  // Window style.
        CW_USEDEFAULT, CW_USEDEFAULT, // Position.
        400, 150,                     // Width, Height.
        parentWindow,                     // Parent window.
        NULL,                         // No menu.
        hInstance,
        NULL);

    if (hDlg)
    {
        ShowWindow(hDlg, SW_SHOW);
        UpdateWindow(hDlg);
    }

  }
