#include <windows.h>

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

static DWORD ParseUnsigned(IN LPCSTR lpszString) {
    CONST DWORD dwLength = StringLength(lpszString);
    DWORD dwResult = 0;
    DWORD i;
    for (i = 0; i < dwLength; ++i) {
        dwResult *= 10;
        dwResult += lpszString[i] - '0';
    }
    return dwResult;
}

static VOID PrintErrorCode(IN LPCSTR lpszNumber) {
    CONST DWORD dwCode = ParseUnsigned(lpszNumber);
    CHAR *aBuffer[1] = { NULL };
    DWORD dwWritten = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        dwCode,
        0,
        (LPSTR)(aBuffer),
        32,
        NULL
    );
    PrintToOutput("Code ");
    PrintToOutput(lpszNumber);
    PrintToOutput(": ");
    PrintToOutput((dwWritten > 0) ? aBuffer[0] : "?\n");
    LocalFree(aBuffer[0]);
}

INT main(IN INT nArgc, IN LPCSTR *lpArgv) {
    INT i;
    --nArgc, ++lpArgv;
    for (i = 0; i < nArgc; ++i) {
        PrintErrorCode(lpArgv[i]);
    }
    return 0;
}
