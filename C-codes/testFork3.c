#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

void GetProcessNameByPID(DWORD pid, char *name, size_t size) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe;  // wide-char version
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == pid) {
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
    STARTUPINFO si1, si2;
    PROCESS_INFORMATION pi1, pi2;
    DWORD parentPID = GetCurrentProcessId();
    char parentName[MAX_PATH], child1Name[MAX_PATH], child2Name[MAX_PATH];

    ZeroMemory(&si1, sizeof(si1));
    si1.cb = sizeof(si1);
    ZeroMemory(&pi1, sizeof(pi1));

    ZeroMemory(&si2, sizeof(si2));
    si2.cb = sizeof(si2);
    ZeroMemory(&pi2, sizeof(pi2));

    // Launch first child process
    if (!CreateProcess(
        "C:\\Windows\\System32\\notepad.exe",
        NULL, NULL, NULL,
        FALSE, 0, NULL, NULL,
        &si1, &pi1))
    {
        printf("CreateProcess for child 1 failed (%lu).\n", GetLastError());
        return 1;
    }

    // Launch second child process
    if (!CreateProcess(
        "C:\\Windows\\System32\\calc.exe",
        NULL, NULL, NULL,
        FALSE, 0, NULL, NULL,
        &si2, &pi2))
    {
        printf("CreateProcess for child 2 failed (%lu).\n", GetLastError());
        return 1;
    }

    // Get process names
    GetProcessNameByPID(parentPID, parentName, MAX_PATH);
    GetProcessNameByPID(pi1.dwProcessId, child1Name, MAX_PATH);
    GetProcessNameByPID(pi2.dwProcessId, child2Name, MAX_PATH);

    // Show results
    printf("Parent PID: %lu | Process Name: %s\n", parentPID, parentName);
    printf("Child1 PID: %lu | Process Name: %s\n", pi1.dwProcessId, child1Name);
    printf("Child2 PID: %lu | Process Name: %s\n", pi2.dwProcessId, child2Name);

    // Wait for both children to finish (concurrently running)
    WaitForSingleObject(pi1.hProcess, INFINITE);
    WaitForSingleObject(pi2.hProcess, INFINITE);

    // Close handles
    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);

    return 0;
}

