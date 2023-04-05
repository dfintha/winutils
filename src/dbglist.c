#include <windows.h>
#include <commctrl.h>
#define IDI_APPICON 101

static HWND hMainWindow = NULL;
static HWND hOutputList = NULL;
static HANDLE hBufferReadyEvent = NULL;
static HANDLE hDataReadyEvent = NULL;
static HANDLE hMapping = NULL;
static BOOL bExiting = FALSE;
static HFONT hFixedFont = NULL;

static CONST INT iProcessIdColLength = 75;
static CONST INT iTimestampColLength = 225;

static HFONT LoadFont(IN LONG lSize, IN LPCSTR lpszName) {
    return CreateFont(
        lSize, 0,
        0, 0,
        FW_LIGHT,
        FALSE, FALSE, FALSE,
        0, 0, 0, 0, 0,
        lpszName
    );
}

static VOID LoadFixedFont(VOID) {
    HANDLE hDC = GetDC(NULL);
    LONG lSize = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(NULL, hDC);

    hFixedFont = LoadFont(lSize, "Cascadia Mono");
    if (hFixedFont != NULL)
        return;
    hFixedFont = LoadFont(lSize, "Consolas");
    if (hFixedFont != NULL)
        return;
    hFixedFont = LoadFont(lSize, "Courier New");
    if (hFixedFont != NULL)
        return;
}

static LPSTR ProcessIdToString(IN DWORD dwNumber) {
    static CHAR aBuffer[16];
    DWORD dwIndex = 0;
    CHAR cTemp;
    DWORD i;

    if (dwNumber == 0) {
        aBuffer[dwIndex++] = '0';
        aBuffer[dwIndex] = '\0';
        return aBuffer;
    }

    while (dwNumber != 0) {
        aBuffer[dwIndex++] = '0' + (dwNumber % 10);
        dwNumber /= 10;
    }

    for (i = 0; i < dwIndex / 2; ++i) {
        cTemp = aBuffer[i];
        aBuffer[i] = aBuffer[dwIndex - i - 1];
        aBuffer[dwIndex - i - 1] = cTemp;
    }

    aBuffer[dwIndex] = '\0';
    return aBuffer;
}

static LPSTR TimestampToString(IN LPSYSTEMTIME lpTimestamp) {
    static CHAR aBuffer[32];
    DWORD dwIndex = 0;
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wYear / 1000);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wYear % 1000 / 100);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wYear % 100 / 10);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wYear % 10);
    aBuffer[dwIndex++] = '-';
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wMonth / 10);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wMonth % 10);
    aBuffer[dwIndex++] = '-';
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wDay / 10);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wDay % 10);
    aBuffer[dwIndex++] = ' ';
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wHour / 10);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wHour % 10);
    aBuffer[dwIndex++] = ':';
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wMinute / 10);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wMinute % 10);
    aBuffer[dwIndex++] = ':';
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wSecond / 10);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wSecond % 10);
    aBuffer[dwIndex++] = '.';
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wMilliseconds / 100);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wMilliseconds % 100 / 10);
    aBuffer[dwIndex++] = '0' + (lpTimestamp->wMilliseconds % 10);
    aBuffer[dwIndex] = '\0';
    return aBuffer;
}

static VOID AddEntryToListView(
    IN HWND hList,
    IN LPVOID lpTimestamp,
    IN DWORD dwProcessId,
    IN LPCSTR lpText
) {
    CONST INT iItems = ListView_GetItemCount(hList);
    LVITEM lvItem;
    ZeroMemory(&lvItem, sizeof(LVITEM));

    lvItem.iItem = iItems;
    ListView_InsertItem(hList, &lvItem);

    lvItem.pszText = TimestampToString(lpTimestamp);
    lvItem.cchTextMax = 32;
    lvItem.mask = LVIF_TEXT | LVIF_STATE;
    lvItem.iSubItem = 0;
    ListView_SetItem(hList, &lvItem);

    lvItem.pszText = ProcessIdToString(dwProcessId);
    lvItem.cchTextMax = 16;
    lvItem.mask = LVIF_TEXT | LVIF_STATE;
    lvItem.iSubItem = 1;
    ListView_SetItem(hList, &lvItem);

    lvItem.pszText = (LPSTR) lpText;
    lvItem.cchTextMax = 1024;
    lvItem.mask = LVIF_TEXT | LVIF_STATE;
    lvItem.iSubItem = 2;
    ListView_SetItem(hList, &lvItem);
}

static DWORD WINAPI ListenerThreadProc(IN LPVOID lpParameter) {
    LPVOID hView;
    DWORD dwProcessId;
    LPCSTR lpszMessage;
    SYSTEMTIME stTime;

    (VOID)(lpParameter);

    while (!bExiting) {
        SetEvent(hBufferReadyEvent);
        WaitForSingleObject(hDataReadyEvent, INFINITE);
        GetSystemTime(&stTime);
        hView = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 1024);
        if (hView == NULL)
            continue;

        dwProcessId = *((DWORD *) hView);
        lpszMessage = ((LPCSTR) hView) + 4;
        AddEntryToListView(hOutputList, &stTime, dwProcessId, lpszMessage);
    }

    ExitThread(0);
}

static LRESULT CALLBACK MainWindowProc(
    IN HWND hWnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
) {
    RECT rWindow;
    static CONST INT iFixColLength = iProcessIdColLength - iTimestampColLength;

    switch (uMsg) {
        case WM_SIZING:
            rWindow = *((LPRECT) lParam);
            /* fallthrough */
        case WM_SIZE:
            GetWindowRect(hMainWindow, &rWindow);
            SetWindowPos(
                hOutputList,
                NULL,
                0,
                0,
                rWindow.right - rWindow.left,
                rWindow.bottom - rWindow.top,
                SWP_NOZORDER
            );
            ListView_SetColumnWidth(
                hOutputList,
                2,
                rWindow.right - rWindow.left - iFixColLength
            );
            break;

        case WM_CLOSE:
            bExiting = TRUE;
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static HWND CreateMainWindow(HINSTANCE hInstance) {
    HWND hWnd;
    WNDCLASSA wcMain;
    LVCOLUMNA lcColumn;
    HICON hIcon = LoadImage(
        hInstance,
        MAKEINTRESOURCE(IDI_APPICON),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED
    );
    CONST INT iWindowWidth = 800;
    CONST INT iWindowHeight = 480;

    wcMain.style = CS_HREDRAW | CS_VREDRAW;
    wcMain.lpfnWndProc = MainWindowProc;
    wcMain.cbClsExtra = 0;
    wcMain.cbWndExtra = 0;
    wcMain.hInstance = hInstance;
    wcMain.hIcon = hIcon;
    wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcMain.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcMain.lpszMenuName = NULL;
    wcMain.lpszClassName = "DBGLISTMAINWINDOW";
    if (!RegisterClassA(&wcMain))
        return NULL;

    hWnd = CreateWindowExA(
        0,
        "DBGLISTMAINWINDOW",
        "DbgList",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        iWindowWidth,
        iWindowHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    hOutputList = CreateWindow(
        WC_LISTVIEW,
        "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        0,
        0,
        iWindowWidth,
        iWindowHeight,
        hWnd,
        NULL,
        (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL
    );
    ListView_SetExtendedListViewStyle(
        hOutputList,
        LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT
    );

    ZeroMemory(&lcColumn, sizeof(LVCOLUMNA));
    lcColumn.mask = 0;
    lcColumn.pszText = "";
    lcColumn.cx = 1;
    ListView_InsertColumn(hOutputList, 0, &lcColumn);

    lcColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
    lcColumn.pszText = "Timestamp";
    lcColumn.cx = iTimestampColLength;
    lcColumn.fmt = LVCFMT_CENTER;
    lcColumn.iSubItem = 1;
    ListView_InsertColumn(hOutputList, 1, &lcColumn);

    lcColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
    lcColumn.pszText = "PID";
    lcColumn.cx = iProcessIdColLength;
    lcColumn.fmt = LVCFMT_CENTER;
    lcColumn.iSubItem = 2;
    ListView_InsertColumn(hOutputList, 2, &lcColumn);

    lcColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
    lcColumn.pszText = "Message";
    lcColumn.cx = iWindowWidth - iProcessIdColLength - iTimestampColLength;
    lcColumn.fmt = LVCFMT_LEFT;
    lcColumn.iSubItem = 3;
    ListView_InsertColumn(hOutputList, 3, &lcColumn);

    ListView_DeleteColumn(hOutputList, 0);

    if (hFixedFont != NULL)
        SendMessage(hOutputList, WM_SETFONT, (LPARAM) hFixedFont, TRUE);

    SendMessage(
        ListView_GetHeader(hOutputList),
        WM_SETFONT,
        (LPARAM) GetStockObject(DEFAULT_GUI_FONT),
        TRUE
    );

    return hWnd;
}

INT WinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE hPrevInstance,
    IN LPSTR lpCmdLine,
    IN INT nShowCmd
) {
    MSG msg;

    (VOID)(hPrevInstance);
    (VOID)(lpCmdLine);
    (VOID)(nShowCmd);

    hBufferReadyEvent = CreateEventA(NULL, FALSE, FALSE, "DBWIN_BUFFER_READY");
    if (hBufferReadyEvent == NULL) {
        MessageBoxA(
            NULL,
            "Failed to open the 'buffer ready' event.",
            "dbglist",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }

    hDataReadyEvent = CreateEventA(NULL, FALSE, FALSE, "DBWIN_DATA_READY");
    if (hDataReadyEvent == NULL) {
        CloseHandle(hBufferReadyEvent);
        MessageBoxA(
            NULL,
            "Failed to open the 'data ready' event.",
            "dbglist",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }

    hMapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        4096,
        "DBWIN_BUFFER"
    );
    if (hMapping == NULL) {
        CloseHandle(hBufferReadyEvent);
        CloseHandle(hDataReadyEvent);
        MessageBoxA(
            NULL,
            "Failed to open the buffer mapping.",
            "dbglist",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }

    LoadFixedFont();
    hMainWindow = CreateMainWindow(hInstance);
    CreateThread(
        NULL,
        0,
        ListenerThreadProc,
        NULL,
        0,
        NULL
    );

    ShowWindow(hMainWindow, nShowCmd);
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(hBufferReadyEvent);
    CloseHandle(hDataReadyEvent);
    CloseHandle(hMapping);
    DeleteObject(hFixedFont);

    return msg.wParam;
}
