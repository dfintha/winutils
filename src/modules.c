#include <windows.h>
#include <psapi.h>

static DWORD StringLength(IN LPCSTR lpszString) {
    DWORD dwLength = 0;
    while (*lpszString++ != '\0')
        ++dwLength;
    return dwLength;
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

static VOID PrintToOutput(IN LPCSTR lpMessage) {
    HANDLE hStdErr = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(hStdErr, lpMessage, StringLength(lpMessage), NULL, NULL);
}

static VOID PrintToError(IN LPCSTR lpMessage) {
    HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    WriteConsoleA(hStdErr, lpMessage, StringLength(lpMessage), NULL, NULL);
}

static LPCSTR FileName(IN LPCSTR lpszPath) {
    CONST DWORD dwLength = StringLength(lpszPath);
    DWORD i;
    for (i = dwLength - 1; i > 0; --i) {
        if (lpszPath[i] == '\\')
            return lpszPath + i + 1;
    }
    return lpszPath;
}

static VOID PrintLastErrorString(VOID) {
    const DWORD dwCode = GetLastError();
    CHAR *aBuffer[1] = { NULL };
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        dwCode,
        0,
        (LPSTR)(aBuffer),
        32,
        NULL
    );
    PrintToError(aBuffer[0]);
    LocalFree(aBuffer[0]);
}

INT main(IN INT nArgc, IN LPCSTR* lpArgv) {
    DWORD dwProcessId;
    DWORD dwBytes;
    DWORD dwCount;
    HANDLE hProcess;
    HMODULE *lphModules;
    CHAR szModuleName[MAX_PATH];
    DWORD i;

    if (nArgc < 2) {
        PrintToError("No process ID specified.\n");
        return 1;
    }

    dwProcessId = ParseUnsigned(lpArgv[1]);
    hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        dwProcessId
    );
    if (hProcess == NULL) {
        PrintToError("Failed to open process: ");
        PrintLastErrorString();
        return 1;
    }

    if (!EnumProcessModules(hProcess, NULL, 0, &dwBytes)) {
        PrintToError("Failed to query required buffer size: ");
        PrintLastErrorString();
        CloseHandle(hProcess);
        return 1;
    }

    lphModules = (HMODULE *) LocalAlloc(LPTR, dwBytes);
    if (!EnumProcessModules(hProcess, lphModules, dwBytes, &dwBytes)) {
        PrintToError("Failed to query process modules: ");
        PrintLastErrorString();
        CloseHandle(hProcess);
        LocalFree(lphModules);
        return 1;
    }

    dwCount = dwBytes / sizeof(HMODULE);
    for (i = 0; i < dwCount; ++i) {
        GetModuleFileNameExA(hProcess, lphModules[i], szModuleName, MAX_PATH);
        PrintToOutput(FileName(szModuleName));
        PrintToOutput(" (");
        PrintToOutput(szModuleName);
        PrintToOutput(")\n");
    }

    CloseHandle(hProcess);
    LocalFree(lphModules);
    return 0;
}
