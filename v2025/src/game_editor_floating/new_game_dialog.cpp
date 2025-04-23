//
// Created by mgrus on 13.04.2025.
//

#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <shlobj_core.h>
#include <shobjidl_core.h>
#include <string>
#include <fstream>
#include <uxtheme.h>
#include <engine/io/io.h>
#include <engine/io/nljson.h>

#include "editor_data.h"

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern void onNewGameCreated(const NewGameData& data);

// Global variables for our controls
static HWND lblGameNameErrorMessage;
static HWND lblLocationError;
static HWND txtGameName;
static HWND txtLocation;

// Control IDs.
#define ID_EDIT_GAME_NAME         101
#define ID_EDIT_PROJECT_LOCATION  102
#define ID_BUTTON_BROWSE          103
#define ID_BUTTON_SAVE            104
#define ID_BUTTON_CANCEL          105
#define ID_LABEL_ERRORS           106

static const std::wstring default_location = L"C:\\dev\\temp";
static const std::wstring default_project_json_file = L"project.json";

void onSaveClicked(HWND hwnd) {

    // Retrieve text from the edit controls.
    wchar_t projectName[256] = {0};
    wchar_t projectLocation[256] = {0};
    GetWindowText(GetDlgItem(hwnd, ID_EDIT_GAME_NAME), projectName, sizeof(projectName));
    GetWindowText(GetDlgItem(hwnd, ID_EDIT_PROJECT_LOCATION), projectLocation, sizeof(projectLocation));

    bool valid = true;
    if (lstrlenW(projectName) == 0) {
        std::wstring error = L"Game name missing";
        SetWindowText(GetDlgItem(hwnd, ID_LABEL_ERRORS), error.c_str());
        SetFocus(txtGameName);
        ShowWindow(lblGameNameErrorMessage, SW_SHOW);
        valid = false;
    }
    if (lstrlenW(projectLocation) == 0) {
        std::wstring error = L"Game location missing";
        SetWindowText(lblLocationError, error.c_str());
        SetFocus(txtLocation);
        ShowWindow(lblLocationError, SW_SHOW);
        valid =false;
    }
    if (valid) {
        std::wstring projectFolder = std::wstring(projectLocation) + L"/" + std::wstring(projectName);
        std::wstring fileName = projectFolder + L"/" + default_project_json_file;

        auto folderError = ((SHCreateDirectoryExW(hwnd, projectFolder.c_str(), nullptr) != ERROR_SUCCESS &&GetLastError() != ERROR_ALREADY_EXISTS));
        if (folderError) {
            // TODO error display
            return;
        }

        nlohmann::json j;
        j["projectName"] = WStringToUtf8(std::wstring(projectName));
        j["engineVersion"] = "1.0";

        std::ofstream f(fileName);
        f << j.dump(4);

        NewGameData d;
        d.projectLocation = WStringToUtf8(projectFolder);
        d.projectName  = WStringToUtf8(projectName);
        onNewGameCreated(d);

        DestroyWindow(hwnd);


    }
}

std::wstring openDirectoryDialog(HWND hwnd) {
    IFileDialog* pFileDialog = nullptr;
    std::wstring folderPath;

    // Create the File Open Dialog object.
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (SUCCEEDED(hr)) {
        // Set options on the dialog.
        DWORD dwOptions;
        if (SUCCEEDED(pFileDialog->GetOptions(&dwOptions))) {
            pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        // Show the dialog.
        hr = pFileDialog->Show(hwnd);
        if (SUCCEEDED(hr)) {
            // Get the result (the selected folder).
            IShellItem* pItem = nullptr;
            hr = pFileDialog->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr)) {
                    folderPath = pszPath;
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }
        pFileDialog->Release();
    }
    return folderPath;
}


// Window Procedure for our popup dialog.
static LRESULT CALLBACK NewGameDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            // "Game Name:" Label.
            auto lblProject = CreateWindowEx(0, WC_STATIC, L"Project Name:", WS_CHILD | WS_VISIBLE,
                8, 16, 150, 24,
                hwnd, NULL, hInst, NULL);

            // Game Name Edit control.
            txtGameName = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                112, 8, 320, 24,
                hwnd, (HMENU)ID_EDIT_GAME_NAME, hInst, NULL);
            SetWindowTheme(txtGameName, L"Explorer", NULL);
            // A placeholder for showing validation errors
            lblGameNameErrorMessage = CreateWindowEx(0, WC_STATIC, L"Errors:", WS_CHILD | WS_VISIBLE,
                112, 36, 320, 24, hwnd, (HMENU) ID_LABEL_ERRORS, hInst, NULL);
            ShowWindow(lblGameNameErrorMessage, SW_HIDE);


            // "ProjectLocation:" Label.
            auto lblLocation = CreateWindowEx(0, WC_STATIC, L"Project Folder:", WS_CHILD | WS_VISIBLE,
                8, 72, 150, 24,
                hwnd, NULL, hInst, NULL);

            // Project Location Edit control.
            txtLocation = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                112, 64, 272, 24,
                hwnd, (HMENU)ID_EDIT_PROJECT_LOCATION, hInst, NULL);
            lblLocationError = CreateWindowEx(0, WC_STATIC, L"Errors:", WS_CHILD | WS_VISIBLE,
                112, 92, 320, 24, hwnd, (HMENU) ID_LABEL_ERRORS, hInst, NULL);
            ShowWindow(lblLocationError, SW_HIDE);

            // Browse button (to select directory).
            auto btnSelectLocation = CreateWindowEx(0, WC_BUTTON, L"...",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                392, 64, 40, 24,
                hwnd, (HMENU)ID_BUTTON_BROWSE, hInst, NULL);

            // Save button.
            auto btnSave = CreateWindowEx(0, WC_BUTTON, L"Save",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_DEFPUSHBUTTON | WS_TABSTOP,
                356, 120, 76, 25,
                hwnd, (HMENU)ID_BUTTON_SAVE, hInst, NULL);


            // Cancel button
            auto btnCancel = CreateWindowEx(0, WC_BUTTON, L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                272, 120, 76, 25,
                hwnd, (HMENU)ID_BUTTON_CANCEL, hInst, NULL);

            SetFocus(txtGameName);



            //SetWindowTheme(hButton, L"Explorer", NULL);

            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            // or even better for Windows 10/11:
            hFont = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            auto errorFont = CreateFontW(-10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            SendMessage(lblProject, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(btnSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(btnCancel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(txtGameName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(lblLocation, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(txtLocation, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(btnSelectLocation, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(lblGameNameErrorMessage, WM_SETFONT, (WPARAM)errorFont, TRUE);
            SendMessage(lblLocationError, WM_SETFONT, (WPARAM)errorFont, TRUE);

            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE)
                SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
            if (wParam == VK_RETURN)
                SendMessage(hwnd, WM_COMMAND, IDOK, 0);
            if (wParam == VK_TAB)
            {
                HWND hNext = GetNextDlgTabItem(hwnd, GetFocus(), !(GetKeyState(VK_SHIFT) & 0x8000));
                if (hNext)
                    SetFocus(hNext);
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
        {
            HDC hdc = (HDC)wParam;
            HWND hCtl = (HWND)lParam;

            // Red text color for our error labels:
            if (hCtl == lblGameNameErrorMessage || hCtl == lblLocationError)
            {
                SetTextColor(hdc, RGB(192, 0, 0));        // red text
                SetBkMode(hdc, TRANSPARENT);              // transparent background
                return (INT_PTR)GetSysColorBrush(COLOR_WINDOW); // match edit bg
            }

            SetBkMode(hdc, TRANSPARENT);
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
        }
        case WM_GETDLGCODE:
            return DLGC_WANTTAB | DLGC_WANTARROWS;

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            if (wmId == IDOK)
                wmId = ID_BUTTON_SAVE;

            switch (wmId)
            {
                case ID_BUTTON_BROWSE:
                {
                    // You could use SHBrowseForFolder here.
                    auto folder = openDirectoryDialog(hwnd);
                    SetWindowText(GetDlgItem(hwnd, ID_EDIT_PROJECT_LOCATION), folder.c_str());
                    break;
                }
                case IDOK:
                case ID_BUTTON_SAVE:
                {
                    onSaveClicked(hwnd);
                    break;
                }
                case IDCANCEL:
                case ID_BUTTON_CANCEL:
                {
                    DestroyWindow(hwnd); // or SendMessage(hwnd, WM_CLOSE, 0, 0);
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
        wc.lpszClassName = L"PopupDialogClass";
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        isRegistered = true;
    }

    int dialogWidth = 464;
    int dialogHeight = 196;

    RECT mainWindowRect;
    GetWindowRect(parentWindow, &mainWindowRect);
    int posX = mainWindowRect.left +  (mainWindowRect.right - mainWindowRect.left) / 2 - dialogWidth/2;
    int posY = mainWindowRect.top +  (mainWindowRect.bottom - mainWindowRect.top) / 2 - dialogHeight/2;


    // Create the popup dialog, specifying hMainWnd as the parent.
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,          // Extended styles.
        L"PopupDialogClass",           // Registered class name.
        L"Create New Game Project",           // Window title.
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,  // Window style.
        posX, posY, // Position.
        dialogWidth, dialogHeight,                     // Width, Height.
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