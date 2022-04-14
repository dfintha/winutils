#include <Windows.h>
#include <stdio.h>

#define POINTER_TO_ULONG(lpValue) ((ULONG) ((UINT_PTR) lpValue))

BOOL ForceShowWindow(IN LPCSTR szWindowName) {
    HWND hWnd;
    RECT rWnd;

    hWnd = FindWindowA(NULL, szWindowName);
    if (hWnd == NULL) {
        puts("  ERROR: Window not found.");
        return FALSE;
    }
    printf("  Window handle is 0x%X.\n", POINTER_TO_ULONG(hWnd));

    if (!GetWindowRect(hWnd, &rWnd)) {
        puts("  ERROR: Failed to get window position.");
        return FALSE;
    }
    printf("  Position of window is (%d, %d).\n", rWnd.left, rWnd.top);

    puts("  Setting window to be at position (10, 10).");
    if (!SetWindowPos(hWnd, NULL, 10, 10, 0, 0, SWP_NOSIZE)) {
        puts("  ERROR: Failed to set window position.");
        return FALSE;
    }

    puts("  Setting opacity of window to 100%.");
    if (!SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA)) {
        puts("  ERROR: Failed to set opacity.");
        return FALSE;
    }

    puts("  Showing window.");
    if (!ShowWindow(hWnd, SW_SHOW)) {
        puts("  ERROR: Failed to show window.");
        return FALSE;
    }

    return TRUE;
}

INT main(IN INT nArgc, IN LPCSTR *lpArgv) {
    INT i;
    --nArgc, ++lpArgv;
    for (i = 0; i < nArgc; ++i) {
        printf("Window '%s':\n", lpArgv[i]);
        ForceShowWindow(lpArgv[i]);
    }
    return 0;
}
