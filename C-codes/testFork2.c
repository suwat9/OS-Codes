#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

void GetProcessNameByPID(DWORD pid, char *name, size_t size) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe;  // Use wide-char version
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == pid) {
                // Convert wide string to ANSI
                WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1, name, (int)size, NULL, NULL);
                CloseHandle(hSnapshot);
                return;
            }
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    snprintf(name, size, "<Unknown>");
}

int main() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD parentPID = GetCurrentProcessId();
    char parentName[MAX_PATH], childName[MAX_PATH];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start child process: notepad.exe
    if (!CreateProcess(
        "C:\\Windows\\System32\\notepad.exe",
        NULL, NULL, NULL,
        FALSE, 0, NULL, NULL,
        &si, &pi))
    {
        printf("CreateProcess failed (%lu).\n", GetLastError());
        return 1;
    }

    // Get process names
    GetProcessNameByPID(parentPID, parentName, MAX_PATH);
    GetProcessNameByPID(pi.dwProcessId, childName, MAX_PATH);

    // Show results
    printf("Parent PID: %lu | Process Name: %s\n", parentPID, parentName);
    printf("Child  PID: %lu | Process Name: %s\n", pi.dwProcessId, childName);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

