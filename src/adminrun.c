#include <Windows.h>

DWORD StringLength(IN LPCSTR lpString) {
    DWORD dwLength = 0;
    while (*lpString++ != '\0')
        ++dwLength;
    return dwLength;
}

LPSTR StringCopy(IN LPSTR lpDestination, IN LPCSTR lpSource) {
    LPSTR lpCopy = lpDestination;
    while ((*lpCopy++ = *lpSource++) != '\0');
    return lpDestination;
}

LPSTR StringConcat(IN LPSTR lpDestination, IN LPCSTR lpSource) {
    LPSTR lpCopy = lpDestination;
    lpCopy += StringLength(lpDestination);
    StringCopy(lpCopy, lpSource);
    return lpDestination;
}

VOID PrintToError(IN LPCSTR lpMessage) {
    HANDLE hStdErr = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(hStdErr, lpMessage, StringLength(lpMessage), NULL, NULL);
}

LPSTR ConcatenateArguments(IN INT nArgc, IN LPCSTR *lpArgv) {
    DWORD dwLength = 0;
    LPSTR lpResult = NULL;
    INT i;

    for (i = 0; i < nArgc; ++i)
        dwLength += StringLength(lpArgv[i] + 1);

    lpResult = LocalAlloc(LPTR, dwLength);
    if (lpResult != NULL) {
        for (i = 0; i < nArgc; ++i) {
            StringConcat(lpResult, lpArgv[i]);
            StringConcat(lpResult, " ");
        }
    }
    return lpResult;
}

INT RunAsAdministrator(IN INT nArgc, IN LPCSTR *lpArgv) {
    LPSTR lpArguments = ConcatenateArguments(nArgc - 1, lpArgv + 1);
    LPCSTR lpFailMessage =
        "Failed to execute command with administrator privileges.\n";

    HINSTANCE hInstance = ShellExecuteA(
        NULL,
        "runas",
        lpArgv[0],
        lpArguments,
        NULL,
        SW_SHOWNORMAL
    );
    LocalFree(lpArguments);

    if (hInstance < (HINSTANCE) 32) {
        PrintToError(lpFailMessage);
        return 1;
    }
    return 0;
}

INT main(IN INT nArgc, IN LPCSTR *lpArgv) {
    if (nArgc < 2) {
        PrintToError("No command specified.\n");
        return 1;
    }
    return RunAsAdministrator(--nArgc, ++lpArgv);
}
