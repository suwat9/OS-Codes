// ID : 654244001 Name : Suwat Ta
#include <windows.h>
#include <stdio.h>

int main() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD parentPID = GetCurrentProcessId(); // Parent PID

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // --------- child process: Child1.exe
    if (!CreateProcess(
        "E:\\Suwat\\C-codes\\Child1.exe", // Application name

        NULL,      // Command line arguments
        NULL,      // Process handle not inheritable
        NULL,      // Thread handle not inheritable
        FALSE,     // Set handle inheritance to FALSE
        0,         // No creation flags
        NULL,      // Use parent's environment block
        NULL,      // Use parent's starting directory 
        &si,       // Pointer to STARTUPINFO structure
        &pi)       // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%lu).\n", GetLastError());
        return 1;
    }

    printf("Parent PID: %lu\n", parentPID);
    printf("Child PID:  %lu\n", pi.dwProcessId);

    // --------- child process ---------- (optional)
    WaitForSingleObject(pi.hProcess, INFINITE);

    // --------- handle
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}


