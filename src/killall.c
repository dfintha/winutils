#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <psapi.h>

LPSTR GetProcessIdByName(
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

VOID KillProcessById(IN DWORD dwProcessId) {
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
        printf("usage: killall <name>\n");
        return 0;
    }

    if (!EnumProcesses(dwProcessIds, sizeof(dwProcessIds), &dwLength))
        return 1;

    for (i = 0; i < dwLength / sizeof(DWORD); ++i) {
        GetProcessIdByName(dwProcessIds[i], szBuffer, sizeof(szBuffer));
        if (strcmp(lpArgv[1], szBuffer) == 0) {
            KillProcessById(dwProcessIds[i]);
            ++nCount;
        } else if (strncmp(lpArgv[1], szBuffer, strlen(lpArgv[1])) == 0) {
            if (strcmp(".exe", szBuffer + strlen(lpArgv[1])) == 0) {
                KillProcessById(dwProcessIds[i]);
                ++nCount;
            }
        }
    }

    printf("Killed %d process%s.\n", nCount, nCount != 1 ? "es" : "");
    return 0;
}
