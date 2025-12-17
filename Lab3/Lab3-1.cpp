#include <iostream>
#include <windows.h>

using namespace std;

// Simulating child process behavior
void childProcess() {
    cout << "\n=== CHILD PROCESS ===" << endl;
    cout << "Child PID: " << GetCurrentProcessId() << endl;
    cout << "Child is executing..." << endl;

    // Simulate some work
    for (int i = 1; i <= 3; i++) {
        cout << "Child: Working... " << i << "/3" << endl;
        Sleep(500);
    }

    cout << "Child: Completed execution" << endl;
    cout << "Child: Exiting with status 0" << endl;
}

// Simulating parent process behavior
void parentProcess(PROCESS_INFORMATION pi) {
    cout << "\n=== PARENT PROCESS ===" << endl;
    cout << "Parent PID: " << GetCurrentProcessId() << endl;
    cout << "Created child with PID: " << pi.dwProcessId << endl;

    // Demonstrate parent can continue executing
    cout << "\nParent: Continuing execution while child runs..." << endl;
    for (int i = 1; i <= 3; i++) {
        cout << "Parent: Doing work... " << i << "/3" << endl;
        Sleep(700);
    }

    // Wait for child process to complete (like wait() in Unix)
    cout << "\nParent: Waiting for child process to finish..." << endl;
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get child's exit status
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    cout << "Parent: Child exited with status " << exitCode << endl;

    // Clean up handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    cout << "Parent: All done!" << endl;
}

int main(int argc, char* argv[]) {
    // Check if this is the child process
    if (argc > 1 && string(argv[1]) == "child") {
        childProcess();
        return 0;  // Child exits here
    }

    // PARENT PROCESS CODE
    cout << "====================================" << endl;
    cout << "OS PROCESS CREATION DEMONSTRATION" << endl;
    cout << "====================================" << endl;
    cout << "\nConcepts Demonstrated:" << endl;
    cout << "1. Process Creation (like fork())" << endl;
    cout << "2. Parent-Child Relationship" << endl;
    cout << "3. Concurrent Execution" << endl;
    cout << "4. Process Synchronization (wait)" << endl;
    cout << "5. Exit Status Communication" << endl;
    cout << "====================================" << endl;

    // Get current executable path
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // Prepare command line for child
    string cmdLine = string(exePath) + " child";
    char* cmdLineChar = new char[cmdLine.length() + 1];
    strcpy_s(cmdLineChar, cmdLine.length() + 1, cmdLine.c_str());

    // Initialize process structures
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    cout << "\nParent: About to create child process..." << endl;
    cout << "Parent PID: " << GetCurrentProcessId() << endl;

    // CREATE NEW PROCESS (analogous to fork() in Unix)
    if (!CreateProcessA(
        NULL,           // No module name
        cmdLineChar,    // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Handle inheritance
        0,              // No creation flags
        NULL,           // Use parent's environment
        NULL,           // Use parent's directory
        &si,            // Startup info
        &pi)            // Process info
        ) {
        cerr << "CreateProcess failed: " << GetLastError() << endl;
        delete[] cmdLineChar;
        return 1;
    }

    cout << "Parent: Child process created successfully!" << endl;

    // Parent continues execution
    parentProcess(pi);

    delete[] cmdLineChar;

    cout << "\n====================================" << endl;
    cout << "DEMONSTRATION COMPLETE" << endl;
    cout << "====================================" << endl;

    return 0;
}
