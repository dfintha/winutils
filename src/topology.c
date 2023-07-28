#include <windows.h>

enum {
    SUCCESS = 0,
    GETDISPLAYCONFIGBUFFERSIZES_FAILED = 1,
    GETPROCESSHEAP_FAILED = 2,
    HEAPALLOC_FAILED = 3,
    QUERYDISPLAYCONFIG_FAILED = 4,
    SETDISPLAYCONFIG_FAILED = 5
};

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

static VOID PrintToError(IN LPCSTR lpMessage) {
    HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    WriteConsoleA(hStdErr, lpMessage, StringLength(lpMessage), NULL, NULL);
}

static VOID PrintErrorMessage(IN DWORD dwCode) {
    char *lpszBuffer[1] = { NULL };
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        dwCode,
        0,
        (LPSTR) lpszBuffer,
        32,
        NULL
    );
    PrintToError(lpszBuffer[0]);
    LocalFree(lpszBuffer[0]);
}

static VOID PrintLastErrorMessage(VOID) {
    PrintErrorMessage(GetLastError());
}

INT main(VOID) {
    UINT32 numPaths;
    UINT32 numModes;
    DISPLAYCONFIG_PATH_INFO *lpPaths = NULL;
    DISPLAYCONFIG_MODE_INFO *lpModes = NULL;
    DISPLAYCONFIG_TOPOLOGY_ID idCurrentTopology;
    HANDLE hHeap = NULL;
    BOOL bCloned;
    LONG lResult;
    INT iStatus = SUCCESS;

    lResult = GetDisplayConfigBufferSizes(
        QDC_DATABASE_CURRENT,
        &numPaths,
        &numModes
    );

    if (lResult != ERROR_SUCCESS) {
        PrintToError("GetDisplayConfigBufferSize() failed: ");
        PrintErrorMessage(lResult);
        iStatus = GETDISPLAYCONFIGBUFFERSIZES_FAILED;
        goto end;
    }

    hHeap = GetProcessHeap();
    if (hHeap == NULL) {
        PrintToError("GetProcessHeap() failed: ");
        PrintLastErrorMessage();
        iStatus = GETPROCESSHEAP_FAILED;
        goto end;
    }

    lpPaths = HeapAlloc(hHeap, 0, numPaths * sizeof(DISPLAYCONFIG_PATH_INFO));
    lpModes = HeapAlloc(hHeap, 0, numModes * sizeof(DISPLAYCONFIG_MODE_INFO));
    if (lpPaths == NULL || lpModes == NULL) {
        PrintToError("HeapAlloc() failed: ");
        PrintLastErrorMessage();
        iStatus = HEAPALLOC_FAILED;
        goto end;
    }

    lResult = QueryDisplayConfig(
        QDC_DATABASE_CURRENT,
        &numPaths,
        lpPaths,
        &numModes,
        lpModes,
        &idCurrentTopology
    );

    if (lResult != ERROR_SUCCESS) {
        PrintToError("QueryDisplayConfig() failed: ");
        PrintErrorMessage(lResult);
        iStatus = QUERYDISPLAYCONFIG_FAILED;
        goto end;
    }

    bCloned = (idCurrentTopology & DISPLAYCONFIG_TOPOLOGY_CLONE) != 0;
    PrintToOutput("Current topology mode is ");
    PrintToOutput(bCloned ? "CLONED" : "MIRRORED");
    PrintToOutput(", attempting to switch to ");
    PrintToOutput(bCloned ? "MIRRORED" : "CLONED");
    PrintToOutput(" mode.\n");

    lResult = SetDisplayConfig(
        0,
        NULL,
        0,
        NULL,
        SDC_APPLY | (bCloned ? SDC_TOPOLOGY_EXTEND : SDC_TOPOLOGY_CLONE)
    );

    if (lResult != ERROR_SUCCESS) {
        PrintToError("SetDisplayConfig() failed: ");
        PrintErrorMessage(lResult);
        iStatus = SETDISPLAYCONFIG_FAILED;
    }

end:
    HeapFree(hHeap, 0, lpPaths);
    HeapFree(hHeap, 0, lpModes);
    CloseHandle(hHeap);
    return iStatus;
}
