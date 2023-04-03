#include <windows.h>
#include <psapi.h>

static DWORD StringLength(IN LPCSTR lpString) {
    DWORD dwLength = 0;
    while (*lpString++ != '\0')
        ++dwLength;
    return dwLength;
}

static BOOL StringEqualN(IN LPCSTR lpLhs, IN LPCSTR lpRhs, IN DWORD dwLength) {
    DWORD i;
    for (i = 0; i < dwLength; ++i)
        if (lpLhs[i] != lpRhs[i])
            return FALSE;
    return TRUE;
}

static BOOL StringEqual(IN LPCSTR lpLhs, IN LPCSTR lpRhs) {
    CONST DWORD dwLength = StringLength(lpLhs);
    if (dwLength != StringLength(lpRhs))
        return FALSE;
    return StringEqualN(lpLhs, lpRhs, dwLength);
}


static VOID PrintToOutput(IN LPCSTR lpMessage) {
    HANDLE hStdErr = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(hStdErr, lpMessage, StringLength(lpMessage), NULL, NULL);
}

static LPSTR GetProcessIdByName(
    IN DWORD dwProcessId,
    IN LPSTR lpBuffer,
    IN SIZE_T cbBufferSize
) {
    CONST DWORD flags = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
    HANDLE hProcess = OpenProcess(flags, FALSE, dwProcessId);
    HMODULE hModule;
    DWORD dwLength;

    if (hProcess == NULL) {
        lpBuffer[0]= '\0';
    } else {
        if (EnumProcessModules(hProcess, &hModule, sizeof(HMODULE), &dwLength))
            GetModuleBaseNameA(hProcess, hModule, lpBuffer, cbBufferSize);
        CloseHandle(hProcess);
    }

    return lpBuffer;
}

static VOID KillProcessById(IN DWORD dwProcessId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    if (hProcess != NULL)
        TerminateProcess(hProcess, 42);
    CloseHandle(hProcess);
}

INT main(IN INT nArgc, IN LPSTR *lpArgv) {
    CHAR szBuffer[2048];
    DWORD dwProcessIds[2048];
    DWORD dwLength;
    UINT i;
    UINT nCount = 0;

    if (nArgc < 2) {
        PrintToOutput("Usage: killall <NAME>\n");
        return 0;
    }

    if (!EnumProcesses(dwProcessIds, sizeof(dwProcessIds), &dwLength))
        return 1;

    for (i = 0; i < dwLength / sizeof(DWORD); ++i) {
        GetProcessIdByName(dwProcessIds[i], szBuffer, sizeof(szBuffer));
        if (StringEqual(lpArgv[1], szBuffer)) {
            KillProcessById(dwProcessIds[i]);
            ++nCount;
        } else if (StringEqualN(lpArgv[1], szBuffer, StringLength(lpArgv[1]))) {
            if (StringEqual(".exe", szBuffer + StringLength(lpArgv[1]))) {
                KillProcessById(dwProcessIds[i]);
                ++nCount;
            }
        }
    }

    if (nCount == 0)
        PrintToOutput("No processes killed.\n");
    return 0;
}
