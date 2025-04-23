//
// Created by mgrus on 11.04.2025.
//

#include <windowsx.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/win32_based_ui/MenuBar.h>
#include <engine/graphics/win32_based_ui/Menu.h>

std::shared_ptr<win32::WinMenuBar> Application::createNativeMenuBar() {
    native_main_menu_bar_ = std::make_shared<win32::WinMenuBar>(window_);
    return native_main_menu_bar_;
}

std::shared_ptr<win32::WinMenu> Application::createNativeMenu(const std::string &text) {
    return std::make_shared<win32::WinMenu>(window_, text);
}

LRESULT CALLBACK SplitterWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    static bool isDragging = false;
    static int dragStartX = 0;     // Mouse x when drag started
    static int origSplitterX = 0;  // Splitter's original x position

    switch(msg)
    {

        case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcs->lpCreateParams);
            return 0;
        }

        // Handle messages here...
        case WM_SIZE: {
            // lParam contains the new width and height:
            int newWidth  = LOWORD(lParam);
            int newHeight = HIWORD(lParam);
            // You can store these values or use them to update an internal model.
            // For a simple splitter which only paints a solid background,
            // nothing extra is needed if the size change doesn't affect its behavior.
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            isDragging = true;
            dragStartX = GET_X_LPARAM(lParam);

            RECT rect;
            GetWindowRect(hwnd, &rect);
            POINT pt = { rect.left, rect.top };
            ScreenToClient(GetParent(hwnd), &pt);
            origSplitterX = pt.x;

            SetCapture(hwnd);

            return 0;
        }
        case WM_MOUSEMOVE:
        {
            if (isDragging) {
                // Calculate new position delta relative to the drag start
                int currentX = GET_X_LPARAM(lParam);
                int deltaX = currentX - dragStartX;
                int newSplitterX = origSplitterX + deltaX; // New x position

                // Optionally, enforce boundaries.
                RECT rcParent;
                GetClientRect(getApplication()->hwnd(), &rcParent);
                const int minPos = 50; // Minimum allowed x coordinate for the splitter
                const int maxPos = rcParent.right - 50; // Maximum allowed x coordinate
                if(newSplitterX < minPos)
                    newSplitterX = minPos;
                if(newSplitterX > maxPos)
                    newSplitterX = maxPos;

                // Reposition the splitter:
                // We use SWP_NOSIZE because we only change its x coordinate
                SetWindowPos(hwnd, NULL, newSplitterX, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

                // Now update the left and right panes accordingly.
                // For example, assuming the splitter has a fixed width (e.g., 5 pixels):
                const int splitterWidth = 5;

                NativeVSplitter* splitterData = (NativeVSplitter*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

                // Resize the left pane:
                MoveWindow(splitterData->left, 0, 0, newSplitterX, rcParent.bottom, TRUE);

                // Resize and reposition the right pane:
                int rightPaneX = newSplitterX + splitterWidth;
                MoveWindow(splitterData->right, rightPaneX, 0,
                           rcParent.right - rightPaneX,
                           rcParent.bottom,
                           TRUE);

                // Optionally, force an update/redraw of the parent window or panes.
            }
            return 0;
        }
        case WM_LBUTTONUP:
        {
            // End dragging; call ReleaseCapture.
            ReleaseCapture();
            return 0;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


LRESULT CALLBACK MyContentWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        // Handle messages here...
        case WM_PAINT:
            // Painting code...
                break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND Application::createNativePanel(glm::vec2 position, glm::vec2 size, const int id, glm::ivec3 bgColor) {
    // Center Pane: Render or main content area

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = MyContentWndProc; // Your custom window procedure.
    wc.hInstance = h_instance_;
    wc.lpszClassName = "MyCustomWindowClass";
    // Create a red brush for the background:
    wc.hbrBackground = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));

    RegisterClassEx(&wc);

    HWND panel = CreateWindowEx(
        0,
        "MyCustomWindowClass",  // custom class for your 3D or editor area
        NULL,
        WS_CHILD | WS_VISIBLE,
        position.x, position.y, size.x, size.y,
        window_,
        (HMENU)id,
        h_instance_,
        NULL
    );

    return panel;
}

HWND Application::createNativeVSplitter(HWND leftPane, HWND rightPane, const int id) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = SplitterWndProc;
    wc.hInstance = h_instance_;
    wc.lpszClassName = "SplitterWindowClass";
    wc.hbrBackground = CreateSolidBrush(RGB(10, 255, 10));

    RegisterClassEx(&wc);

    RECT leftRect;
    GetClientRect(leftPane, &leftRect);
    auto splitterData = new NativeVSplitter();
    splitterData->left = leftPane;
    splitterData->right = rightPane;

    HWND splitter = CreateWindowEx(
        0,
        "SplitterWindowClass",  // custom class for your 3D or editor area
        NULL,
        WS_CHILD | WS_VISIBLE,
        leftRect.right , leftRect.top, 5, leftRect.bottom - leftRect.top,
        window_,
        (HMENU)id,
        h_instance_,
        splitterData
    );

    return splitter;
}

HWND Application::createNativeTreeView(glm::ivec2 pos, glm::ivec2 size, int treeViewerId) {
    // For example, if you have a tree view on the left and a render window on the right with a splitter:
         // Example fixed width for the tree view.
    HWND hTreeView = CreateWindowEx(
    0,
    WC_TREEVIEW,
    NULL,
    WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES,
    pos.x, pos.y,
    size.x, size.y,
    window_,
    (HMENU)treeViewerId,
    h_instance_,
    NULL
    );

    RECT rcClient;
    GetClientRect(window_, &rcClient);

    // Fit under the toolbar if we have one.
    if (hWndToolbar) {
        RECT rcToolBar;
        GetWindowRect(hWndToolbar, &rcToolBar);
        // Convert toolbar rectangle from screen to client coordinates.
        POINT pt = { 0, 0 };
        ScreenToClient(window_, &pt);
        int toolbarHeight = (rcToolBar.bottom - rcToolBar.top);

        // Calculate the available client area below the toolbar.
        int availableHeight = rcClient.bottom - toolbarHeight;
        int availableWidth = rcClient.right;

        // Position the toolbar at the top.
        MoveWindow(hWndToolbar, 0, 0, availableWidth, toolbarHeight, TRUE);
        MoveWindow(hTreeView, pos.x, toolbarHeight, size.x, availableHeight, TRUE);
    }

    return hTreeView;
}

HTREEITEM Application::addTreeItem(HWND treeViewer, const std::string text, HTREEITEM parent, HTREEITEM insertAfter) {
    TVINSERTSTRUCT tvis = {0};
    tvis.hParent = parent;
    tvis.hInsertAfter = insertAfter;
    tvis.item.mask = TVIF_TEXT; // You can add TVIF_IMAGE, TVIF_SELECTEDIMAGE, TVIF_PARAM, etc.
    tvis.item.pszText = (LPSTR)text.c_str();
    // Optionally, set tvis.item.lParam to store application-specific data

    return TreeView_InsertItem(treeViewer, &tvis);
}
