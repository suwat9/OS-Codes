#include <iostream>
#include <unistd.h>     // fork(), getpid(), getppid()
#include <sys/wait.h>   // wait()
#include <sys/types.h>  // pid_t

using namespace std;

// Global variable to demonstrate process memory space
int globalCounter = 0;

void demonstrateProcessConcepts() {
    cout << "==================================================" << endl;
    cout << "  OPERATING SYSTEM PROCESS CREATION CONCEPTS" << endl;
    cout << "==================================================" << endl;
    cout << "\nKey Concepts Demonstrated:" << endl;
    cout << "1. Process Creation using fork()" << endl;
    cout << "2. Process ID (PID) assignment" << endl;
    cout << "3. Parent-Child relationship" << endl;
    cout << "4. Copy-on-Write memory (separate address spaces)" << endl;
    cout << "5. Process synchronization (wait)" << endl;
    cout << "6. Exit status collection" << endl;
    cout << "==================================================" << endl;
}

int main() {
    demonstrateProcessConcepts();

    cout << "\n--- Before fork() ---" << endl;
    cout << "Original Process PID: " << getpid() << endl;
    cout << "Initial globalCounter: " << globalCounter << endl;

    // CREATE A NEW PROCESS using fork()
    cout << "\n--- Calling fork() ---" << endl;
    pid_t pid = fork();

    // Error handling
    if (pid < 0) {
        cerr << "Fork failed!" << endl;
        return 1;
    }

    // CHILD PROCESS (pid == 0)
    if (pid == 0) {
        cout << "\n=== CHILD PROCESS ===" << endl;
        cout << "Child PID: " << getpid() << endl;
        cout << "Parent PID: " << getppid() << endl;
        cout << "fork() returned: " << pid << " (0 means I'm the child)" << endl;

        // Modify global variable (Copy-on-Write happens here)
        globalCounter = 100;
        cout << "Child's globalCounter: " << globalCounter << endl;

        // Simulate some work
        cout << "Child executing task..." << endl;
        sleep(2);

        cout << "Child process terminating with exit code 42" << endl;
        return 42;  // Exit with custom code
    }

    // PARENT PROCESS (pid > 0, contains child's PID)
    else {
        cout << "\n=== PARENT PROCESS ===" << endl;
        cout << "Parent PID: " << getpid() << endl;
        cout << "Created Child PID: " << pid << endl;
        cout << "fork() returned: " << pid << " (child's PID)" << endl;

        // Parent's memory space is separate from child
        globalCounter = 50;
        cout << "Parent's globalCounter: " << globalCounter << endl;

        cout << "\nParent waiting for child to complete..." << endl;

        // Wait for child process to finish
        int status;
        pid_t terminated_pid = wait(&status);

        cout << "\n--- Child Process Terminated ---" << endl;
        cout << "Terminated Child PID: " << terminated_pid << endl;

        // Check how child terminated
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            cout << "Child exited normally with code: " << exit_code << endl;
        }
        else if (WIFSIGNALED(status)) {
            cout << "Child terminated by signal: " << WTERMSIG(status) << endl;
        }

        cout << "\n=== FINAL STATE ===" << endl;
        cout << "Parent's final globalCounter: " << globalCounter << endl;
        cout << "(Notice: Child's modification didn't affect parent)" << endl;
        cout << "\nAll processes completed successfully!" << endl;
    }

    return 0;
}
