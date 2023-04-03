#include <Windows.h>

static DWORD StringLength(IN LPCSTR lpString) {
    DWORD dwLength = 0;
    while (*lpString++ != '\0')
        ++dwLength;
    return dwLength;
}

static VOID PrintToOutput(IN LPCSTR lpMessage) {
    HANDLE hStdErr = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(hStdErr, lpMessage, StringLength(lpMessage), NULL, NULL);
}

static BOOL ForceShowWindow(IN LPCSTR szWindowName) {
    HWND hWnd;
    RECT rWnd;

    hWnd = FindWindowA(NULL, szWindowName);
    if (hWnd == NULL) {
        PrintToOutput("  ERROR: Window not found.\n");
        return FALSE;
    }
    PrintToOutput("  Window handle found.\n");

    if (!GetWindowRect(hWnd, &rWnd)) {
        PrintToOutput("  ERROR: Failed to get window position.");
        return FALSE;
    }
    PrintToOutput("  Window position found.\n");

    PrintToOutput("  Setting window to be at position (10, 10).\n");
    if (!SetWindowPos(hWnd, NULL, 10, 10, 0, 0, SWP_NOSIZE)) {
        PrintToOutput("  ERROR: Failed to set window position.\n");
        return FALSE;
    }

    PrintToOutput("  Setting opacity of window to 100%.\n");
    if (!SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA)) {
        PrintToOutput("  ERROR: Failed to set opacity.\n");
        return FALSE;
    }

    PrintToOutput("  Showing window.\n");
    if (!ShowWindow(hWnd, SW_SHOW)) {
        PrintToOutput("  ERROR: Failed to show window.\n");
        return FALSE;
    }

    return TRUE;
}

INT main(IN INT nArgc, IN LPCSTR *lpArgv) {
    INT i;
    --nArgc, ++lpArgv;
    for (i = 0; i < nArgc; ++i) {
        ForceShowWindow(lpArgv[i]);
    }
    return 0;
}
