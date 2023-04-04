#include <windows.h>
#include <commctrl.h>
#define IDI_APPICON 101

static HWND hWndFrame = NULL;
static HWND hWndClient = NULL;
static HWND hWndManager = NULL;
static HWND hCaptureButton = NULL;
static HWND hReleaseButton = NULL;
static HWND hRefreshButton = NULL;
static HWND hWindowList = NULL;
static HWND hChildList = NULL;

static VOID AddTextItemToListView(IN HWND hList, IN LPCSTR lpText) {
    CONST INT iItems = ListView_GetItemCount(hList);
    LVITEM lvItem;
    lvItem.pszText = (LPSTR) lpText;
    lvItem.cchTextMax = 256;
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvItem.stateMask = 0;
    lvItem.iItem = iItems + 1;
    lvItem.iSubItem = 0;
    lvItem.state = 0;
    ListView_InsertItem(hList, &lvItem);
}

static VOID CaptureChild(IN HWND hWndChild) {
    SetParent(hWndChild, hWndClient);
    SendMessage(hWndChild, WM_SYSCOMMAND, SC_RESTORE, 0);
    SetWindowPos(hWndChild, NULL, 10, 10, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

static VOID ReleaseChild(IN HWND hWndChild) {
    SetParent(hWndChild, GetDesktopWindow());
    SendMessage(hWndChild, WM_SYSCOMMAND, SC_RESTORE, 0);
    SetWindowPos(hWndChild, NULL, 10, 10, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

static BOOL CALLBACK WindowEnumCallback(IN HWND hWnd, IN LPARAM lParam) {
    LPSTR lpBuffer;
    CONST INT iLength = GetWindowTextLength(hWnd);

    if (!IsWindowVisible(hWnd) || iLength == 0 || hWnd == hWndFrame ||
        hWnd == hWndClient || hWnd == hWndManager || hWnd == hCaptureButton ||
        hWnd == hReleaseButton || hWnd == hRefreshButton ||
        hWnd == hWindowList || hWnd == hChildList)
        return TRUE;

    lpBuffer = HeapAlloc(GetProcessHeap(), 0, iLength + 1);
    GetWindowText(hWnd, lpBuffer, iLength + 1);
    AddTextItemToListView((HWND) lParam, lpBuffer);
    HeapFree(GetProcessHeap(), 0, lpBuffer);
    return TRUE;
}

static VOID CALLBACK RefreshCallback(VOID) {
    ListView_DeleteAllItems(hWindowList);
    ListView_DeleteAllItems(hChildList);
    EnumWindows(WindowEnumCallback, (LPARAM) hWindowList);
    EnumChildWindows(hWndClient, WindowEnumCallback, (LPARAM) hChildList);
}

static LRESULT CALLBACK FrameWindowProc(
    IN HWND hWnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
) {
    CLIENTCREATESTRUCT ccs;

    switch (uMsg) {
        case WM_CREATE:
            ZeroMemory(&ccs, sizeof(CLIENTCREATESTRUCT));
            ccs.idFirstChild = 1;
            hWndClient = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                "MDICLIENT",
                NULL,
                WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                hWnd,
                NULL,
                GetModuleHandle(NULL),
                &ccs
            );
            break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefFrameProc(hWnd, hWndClient, uMsg, wParam, lParam);
}

static HWND CreateFrameWindow(IN HINSTANCE hInstance) {
    WNDCLASSA wcFrame;
    HICON hIcon = LoadImage(
        hInstance,
        MAKEINTRESOURCE(IDI_APPICON),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED
    );

    wcFrame.style = CS_HREDRAW | CS_VREDRAW;
    wcFrame.lpfnWndProc = FrameWindowProc;
    wcFrame.cbClsExtra = 0;
    wcFrame.cbWndExtra = 0;
    wcFrame.hInstance = hInstance;
    wcFrame.hIcon = hIcon; /*LoadIcon(NULL, IDI_APPLICATION);*/
    wcFrame.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcFrame.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcFrame.lpszMenuName = NULL;
    wcFrame.lpszClassName = "MDIFRAME";
    if (!RegisterClassA(&wcFrame))
        return NULL;
    return CreateWindowExA(
        0,
        "MDIFRAME",
        "MDI Capture",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );
}

static LRESULT CALLBACK ManagerWindowProc(
    IN HWND hWnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
) {
    INT iSelect;
    CHAR aBuffer[1024];

    switch (uMsg) {
        case WM_COMMAND:
            if ((HWND) lParam == hRefreshButton) {
                RefreshCallback();
            }
            if ((HWND) lParam == hCaptureButton) {
                iSelect = ListView_GetNextItem(hWindowList, -1, LVNI_SELECTED);
                ListView_GetItemText(hWindowList, iSelect, 0, aBuffer, 1024);
                CaptureChild(FindWindowA(NULL, aBuffer));
                RefreshCallback();
            }
            if ((HWND) lParam == hReleaseButton) {
                iSelect = ListView_GetNextItem(hChildList, -1, LVNI_SELECTED);
                ListView_GetItemText(hChildList, iSelect, 0, aBuffer, 1024);
                ReleaseChild(FindWindowExA(hWndClient, NULL, NULL, aBuffer));
                RefreshCallback();
            }
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static HWND CreateManagerWindow(IN HINSTANCE hInstance) {
    HWND hWnd;
    WNDCLASSA wcManager;
    LV_COLUMN lcColumn;

    CONST INT iButtonWidth = 100;
    CONST INT iButtonHeight = 30;
    CONST INT iPadding = 10;
    CONST INT iListHeight = 200;
    CONST INT iWindowWidth = iPadding * 4 + iButtonWidth * 3 + 200;
    CONST INT iWindowHeight = iPadding * 8 + iButtonHeight + iListHeight * 2;
    CONST INT iListWidth = iWindowWidth - iPadding * 2;

    wcManager.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
    wcManager.lpfnWndProc = ManagerWindowProc;
    wcManager.cbClsExtra = 0;
    wcManager.cbWndExtra = 0;
    wcManager.hInstance = hInstance;
    wcManager.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcManager.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcManager.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcManager.lpszMenuName = NULL;
    wcManager.lpszClassName = "MDIMANAGER";
    if (!RegisterClassA(&wcManager))
        return NULL;

    hWnd = CreateWindowExA(
        0,
        "MDIMANAGER",
        "Capture Manager",
        WS_OVERLAPPED | WS_CAPTION,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        iWindowWidth + 5,
        iWindowHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    hCaptureButton = CreateWindow(
        WC_BUTTON,
        "Capture",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        iPadding,
        iPadding,
        iButtonWidth,
        iButtonHeight,
        hWnd,
        NULL,
        (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL
    );
    SendMessage(
        hCaptureButton,
        WM_SETFONT,
        (LPARAM) GetStockObject(DEFAULT_GUI_FONT),
        TRUE
    );

    hReleaseButton = CreateWindow(
        WC_BUTTON,
        "Release",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        iPadding * 2 + iButtonWidth,
        iPadding,
        iButtonWidth,
        iButtonHeight,
        hWnd,
        NULL,
        (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL
    );
    SendMessage(
        hReleaseButton,
        WM_SETFONT,
        (LPARAM) GetStockObject(DEFAULT_GUI_FONT),
        TRUE
    );

    hRefreshButton = CreateWindow(
        WC_BUTTON,
        "Refresh",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        iPadding * 3 + iButtonWidth * 2,
        iPadding,
        iButtonWidth,
        iButtonHeight,
        hWnd,
        NULL,
        (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL
    );
    SendMessage(
        hRefreshButton,
        WM_SETFONT,
        (LPARAM) GetStockObject(DEFAULT_GUI_FONT),
        TRUE
    );

    hWindowList = CreateWindow(
        WC_LISTVIEW,
        "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        iPadding,
        iPadding * 2 + iButtonHeight,
        iListWidth,
        iListHeight,
        hWnd,
        NULL,
        (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL
    );
    ListView_SetExtendedListViewStyle(
        hWindowList,
        LVS_EX_FLATSB | LVS_EX_GRIDLINES
    );

    ZeroMemory(&lcColumn, sizeof(lcColumn));
    lcColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lcColumn.cx = 40;
    lcColumn.pszText = "External Window Title";
    lcColumn.cx = iListWidth - 1;
    ListView_InsertColumn(hWindowList, 0, &lcColumn);

    hChildList = CreateWindow(
        WC_LISTVIEW,
        "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        iPadding,
        iPadding * 3 + iButtonHeight + iListHeight,
        iListWidth,
        iListHeight,
        hWnd,
        NULL,
        (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL
    );
    ListView_SetExtendedListViewStyle(
        hChildList,
        LVS_EX_FLATSB | LVS_EX_GRIDLINES
    );

    ZeroMemory(&lcColumn, sizeof(lcColumn));
    lcColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lcColumn.cx = 40;
    lcColumn.pszText = "Captured Window Title";
    lcColumn.cx = iListWidth - 1;
    ListView_InsertColumn(hChildList, 0, &lcColumn);

    return hWnd;
}

INT WinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE hPrevInstance,
    IN LPSTR lpCmdLine,
    IN INT nShowCmd
) {
    MSG msg;
    INITCOMMONCONTROLSEX icc;

    (VOID)(hPrevInstance);
    (VOID)(lpCmdLine);

    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    hWndFrame = CreateFrameWindow(hInstance);
    ShowWindow(hWndFrame, nShowCmd);
    hWndManager = CreateManagerWindow(hInstance);
    ShowWindow(hWndManager, SW_NORMAL);
    CaptureChild(hWndManager);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (!TranslateMDISysAccel(hWndClient, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}
