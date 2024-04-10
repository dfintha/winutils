#include <windows.h>
#include <strsafe.h>

static DWORD StringLength(IN LPCSTR lpszString) {
    DWORD dwLength = 0;
    while (*lpszString++ != '\0')
        ++dwLength;
    return dwLength;
}

static VOID PrintToOutput(IN LPCSTR lpMessage) {
    HANDLE hStdErr = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(hStdErr, lpMessage, StringLength(lpMessage), NULL, NULL);
}

static BOOL CALLBACK WindowEnumCallback(IN HWND hWnd, IN LPARAM lUnused) {
    DWORD dwProcessId;
    CHAR szProcessId[9];
    DWORD dwProcessIdLength;
    static CHAR szBuffer[1024];

    (VOID)(lUnused);

    if (!IsWindowVisible(hWnd) || GetWindowTextLength(hWnd) == 0)
        return TRUE;

    GetWindowThreadProcessId(hWnd, &dwProcessId);
    _ltoa(dwProcessId, szProcessId, 10);
    dwProcessIdLength = StringLength(szProcessId);

    CopyMemory(szBuffer, "[        ] ", 12);
    CopyMemory(
        szBuffer + (9 - dwProcessIdLength),
        szProcessId,
        dwProcessIdLength
    );
    GetWindowTextA(hWnd, szBuffer + 11, 1000);
    StringCchCatA(szBuffer, 1024, "\n");

    PrintToOutput(szBuffer);
    return TRUE;
}

INT main(VOID) {
    return EnumWindows(WindowEnumCallback, 0) ? 0 : 1;
}
