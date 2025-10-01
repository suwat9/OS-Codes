# C++ Deadlock Management Worksheet
## Operating Systems Concepts - Deadlock Prevention, Avoidance, and Detection

---

## **Introduction to Deadlocks**

A **deadlock** occurs when two or more processes are unable to proceed because each is waiting for the other to release resources.

**Four Necessary Conditions for Deadlock (Coffman Conditions)**:
1. **Mutual Exclusion**: Resources cannot be shared
2. **Hold and Wait**: Process holds resources while waiting for others
3. **No Preemption**: Resources cannot be forcibly taken
4. **Circular Wait**: Circular chain of processes waiting for resources

---

## **Part 1: Deadlock Prevention**

### Concept:
Prevent deadlock by ensuring at least ONE of the four Coffman conditions cannot hold.

### Exercise 1.1: Deadlock Example (Problem)

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1, mutex2;

// This code WILL create a deadlock!
void thread1() {
    mutex1.lock();
    std::cout << "Thread 1: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Thread 1: Waiting for mutex2...\n";
    mutex2.lock();  // Will wait forever if thread2 has mutex2
    
    std::cout << "Thread 1: Locked mutex2\n";
    
    mutex2.unlock();
    mutex1.unlock();
}

void thread2() {
    mutex2.lock();
    std::cout << "Thread 2: Locked mutex2\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Thread 2: Waiting for mutex1...\n";
    mutex1.lock();  // Will wait forever if thread1 has mutex1
    
    std::cout << "Thread 2: Locked mutex1\n";
    
    mutex1.unlock();
    mutex2.unlock();
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Question**: Why does this code create a deadlock?

**Your Answer**: 
```
_______________________________________________________________
_______________________________________________________________
_______________________________________________________________
```

---

### Exercise 1.2: Solution - Lock Ordering (Prevention)

**Strategy**: Prevent Circular Wait by imposing a total ordering on resources.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1, mutex2;

// Solution: Always lock mutexes in the same order
void thread1_fixed() {
    // Lock in order: mutex1 first, then mutex2
    mutex1.lock();
    std::cout << "Thread 1: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    mutex2.lock();
    std::cout << "Thread 1: Locked mutex2\n";
    
    // Perform operations
    std::cout << "Thread 1: Critical section\n";
    
    // Unlock in reverse order
    mutex2.unlock();
    mutex1.unlock();
    std::cout << "Thread 1: Released all locks\n";
}

void thread2_fixed() {
    // Lock in SAME order: mutex1 first, then mutex2
    mutex1.lock();
    std::cout << "Thread 2: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    mutex2.lock();
    std::cout << "Thread 2: Locked mutex2\n";
    
    // Perform operations
    std::cout << "Thread 2: Critical section\n";
    
    // Unlock in reverse order
    mutex2.unlock();
    mutex1.unlock();
    std::cout << "Thread 2: Released all locks\n";
}

int main() {
    std::thread t1(thread1_fixed);
    std::thread t2(thread2_fixed);
    
    t1.join();
    t2.join();
    
    std::cout << "Program completed successfully!\n";
    
    return 0;
}
```

**Question**: Which Coffman condition did we eliminate?

**Answer**: Circular Wait - by ensuring all threads acquire locks in the same order.

---

### Exercise 1.3: Prevention Using `std::lock()`

**Modern C++ Solution**: Use `std::lock()` to lock multiple mutexes atomically.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1, mutex2;

void safe_thread1() {
    // std::lock locks multiple mutexes without deadlock
    std::lock(mutex1, mutex2);
    
    // Adopt the locks into lock_guards for RAII
    std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);
    
    std::cout << "Thread 1: Locked both mutexes safely\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Thread 1: Doing work\n";
    
    // Locks automatically released when lock_guards go out of scope
}

void safe_thread2() {
    // Can lock in any order with std::lock
    std::lock(mutex2, mutex1);
    
    std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);
    
    std::cout << "Thread 2: Locked both mutexes safely\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Thread 2: Doing work\n";
}

int main() {
    std::thread t1(safe_thread1);
    std::thread t2(safe_thread2);
    
    t1.join();
    t2.join();
    
    std::cout << "No deadlock occurred!\n";
    
    return 0;
}
```

---

### Exercise 1.4: Your Turn - Fix This Deadlock

**Problem Code**:
```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex resourceA, resourceB, resourceC;

void process1() {
    resourceA.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    resourceB.lock();
    // Work...
    resourceB.unlock();
    resourceA.unlock();
}

void process2() {
    resourceB.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    resourceC.lock();
    // Work...
    resourceC.unlock();
    resourceB.unlock();
}

void process3() {
    resourceC.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    resourceA.lock();
    // Work...
    resourceA.unlock();
    resourceC.unlock();
}
```

**Task**: Rewrite the three functions to prevent deadlock using lock ordering.

**Your Solution**:
```cpp
void process1() {
    // Your code here
    
    
    
}

void process2() {
    // Your code here
    
    
    
}

void process3() {
    // Your code here
    
    
    
}
```

---

## **Part 2: Deadlock Avoidance**

### Concept:
Make decisions at runtime to avoid entering unsafe states. The most famous algorithm is the **Banker's Algorithm**.

### Exercise 2.1: Banker's Algorithm Implementation

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

class BankersAlgorithm {
private:
    int numProcesses;
    int numResources;
    
    std::vector<std::vector<int>> allocation;   // Currently allocated
    std::vector<std::vector<int>> maximum;      // Maximum demand
    std::vector<int> available;                  // Available resources
    
public:
    BankersAlgorithm(int processes, int resources) 
        : numProcesses(processes), numResources(resources) {
        allocation.resize(processes, std::vector<int>(resources, 0));
        maximum.resize(processes, std::vector<int>(resources, 0));
        available.resize(resources, 0);
    }
    
    void setAvailable(const std::vector<int>& avail) {
        available = avail;
    }
    
    void setMaximum(int process, const std::vector<int>& max) {
        maximum[process] = max;
    }
    
    void setAllocation(int process, const std::vector<int>& alloc) {
        allocation[process] = alloc;
    }
    
    // Calculate Need matrix (Maximum - Allocation)
    std::vector<std::vector<int>> calculateNeed() {
        std::vector<std::vector<int>> need(numProcesses, 
                                           std::vector<int>(numResources));
        for (int i = 0; i < numProcesses; i++) {
            for (int j = 0; j < numResources; j++) {
                need[i][j] = maximum[i][j] - allocation[i][j];
            }
        }
        return need;
    }
    
    // Check if system is in safe state
    bool isSafeState(std::vector<int>& safeSequence) {
        std::vector<int> work = available;
        std::vector<bool> finish(numProcesses, false);
        std::vector<std::vector<int>> need = calculateNeed();
        
        safeSequence.clear();
        
        // Try to find safe sequence
        for (int count = 0; count < numProcesses; count++) {
            bool found = false;
            
            for (int i = 0; i < numProcesses; i++) {
                if (!finish[i]) {
                    // Check if need[i] <= work
                    bool canAllocate = true;
                    for (int j = 0; j < numResources; j++) {
                        if (need[i][j] > work[j]) {
                            canAllocate = false;
                            break;
                        }
                    }
                    
                    if (canAllocate) {
                        // Allocate resources
                        for (int j = 0; j < numResources; j++) {
                            work[j] += allocation[i][j];
                        }
                        
                        safeSequence.push_back(i);
                        finish[i] = true;
                        found = true;
                    }
                }
            }
            
            if (!found) {
                return false; // No safe sequence exists
            }
        }
        
        return true; // Safe sequence found
    }
    
    // Request resources for a process
    bool requestResources(int process, const std::vector<int>& request) {
        std::vector<std::vector<int>> need = calculateNeed();
        
        // Check if request <= need
        for (int i = 0; i < numResources; i++) {
            if (request[i] > need[process][i]) {
                std::cout << "Error: Process exceeded maximum claim\n";
                return false;
            }
        }
        
        // Check if request <= available
        for (int i = 0; i < numResources; i++) {
            if (request[i] > available[i]) {
                std::cout << "Process must wait - insufficient resources\n";
                return false;
            }
        }
        
        // Pretend to allocate
        for (int i = 0; i < numResources; i++) {
            available[i] -= request[i];
            allocation[process][i] += request[i];
        }
        
        // Check if safe
        std::vector<int> safeSeq;
        if (isSafeState(safeSeq)) {
            std::cout << "Request granted! Safe sequence: ";
            for (int p : safeSeq) {
                std::cout << "P" << p << " ";
            }
            std::cout << "\n";
            return true;
        } else {
            // Rollback
            for (int i = 0; i < numResources; i++) {
                available[i] += request[i];
                allocation[process][i] -= request[i];
            }
            std::cout << "Request denied - would lead to unsafe state\n";
            return false;
        }
    }
    
    void printState() {
        std::cout << "\n=== Current State ===\n";
        
        std::cout << "Available: ";
        for (int val : available) {
            std::cout << val << " ";
        }
        std::cout << "\n\nAllocation Matrix:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int val : allocation[i]) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        
        std::cout << "\nMaximum Matrix:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int val : maximum[i]) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        
        auto need = calculateNeed();
        std::cout << "\nNeed Matrix:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int val : need[i]) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    // Example: 5 processes, 3 resource types (A, B, C)
    BankersAlgorithm banker(5, 3);
    
    // Available resources: A=3, B=3, C=2
    banker.setAvailable({3, 3, 2});
    
    // Set Maximum matrix
    banker.setMaximum(0, {7, 5, 3});
    banker.setMaximum(1, {3, 2, 2});
    banker.setMaximum(2, {9, 0, 2});
    banker.setMaximum(3, {2, 2, 2});
    banker.setMaximum(4, {4, 3, 3});
    
    // Set Allocation matrix
    banker.setAllocation(0, {0, 1, 0});
    banker.setAllocation(1, {2, 0, 0});
    banker.setAllocation(2, {3, 0, 2});
    banker.setAllocation(3, {2, 1, 1});
    banker.setAllocation(4, {0, 0, 2});
    
    banker.printState();
    
    // Check if initial state is safe
    std::vector<int> safeSeq;
    if (banker.isSafeState(safeSeq)) {
        std::cout << "\nSystem is in SAFE state\n";
        std::cout << "Safe sequence: ";
        for (int p : safeSeq) {
            std::cout << "P" << p << " ";
        }
        std::cout << "\n";
    }
    
    // Process 1 requests (1, 0, 2)
    std::cout << "\n--- P1 requests (1, 0, 2) ---\n";
    banker.requestResources(1, {1, 0, 2});
    
    // Process 4 requests (3, 3, 0)
    std::cout << "\n--- P4 requests (3, 3, 0) ---\n";
    banker.requestResources(4, {3, 3, 0});
    
    // Process 0 requests (0, 2, 0)
    std::cout << "\n--- P0 requests (0, 2, 0) ---\n";
    banker.requestResources(0, {0, 2, 0});
    
    return 0;
}
```

---

### Exercise 2.2: Your Turn - Test Banker's Algorithm

**Scenario**: 
- 4 processes (P0, P1, P2, P3)
- 3 resource types (A, B, C)
- Available: A=2, B=3, C=2

**Allocation Matrix**:
```
     A  B  C
P0:  1  0  1
P1:  2  1  1
P2:  0  1  1
P3:  1  1  0
```

**Maximum Matrix**:
```
     A  B  C
P0:  3  2  2
P1:  4  2  2
P2:  2  3  3
P3:  3  3  2
```

**Questions**:
1. Calculate the Need matrix:
```
     A  B  C
P0:  _  _  _
P1:  _  _  _
P2:  _  _  _
P3:  _  _  _
```

2. Is the system in a safe state? If yes, provide a safe sequence:
```
Your Answer: _______________________________________________
```

3. If P2 requests (0, 1, 1), should the request be granted?
```
Your Answer: _______________________________________________
```

---

## **Part 3: Deadlock Detection**

### Concept:
Allow deadlocks to occur, but detect and recover from them.

### Exercise 3.1: Wait-For Graph Detection

```cpp
#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

class DeadlockDetector {
private:
    int numProcesses;
    std::vector<std::vector<int>> waitForGraph; // Adjacency list
    
    // DFS helper for cycle detection
    bool hasCycleDFS(int node, std::vector<bool>& visited, 
                     std::vector<bool>& recStack, 
                     std::vector<int>& cycle) {
        visited[node] = true;
        recStack[node] = true;
        cycle.push_back(node);
        
        for (int neighbor : waitForGraph[node]) {
            if (!visited[neighbor]) {
                if (hasCycleDFS(neighbor, visited, recStack, cycle)) {
                    return true;
                }
            } else if (recStack[neighbor]) {
                // Cycle found - trim cycle to actual loop
                auto it = std::find(cycle.begin(), cycle.end(), neighbor);
                cycle.erase(cycle.begin(), it);
                return true;
            }
        }
        
        recStack[node] = false;
        cycle.pop_back();
        return false;
    }
    
public:
    DeadlockDetector(int processes) : numProcesses(processes) {
        waitForGraph.resize(processes);
    }
    
    // Add edge: process1 waits for process2
    void addWaitEdge(int process1, int process2) {
        waitForGraph[process1].push_back(process2);
    }
    
    // Remove edge
    void removeWaitEdge(int process1, int process2) {
        auto& edges = waitForGraph[process1];
        edges.erase(std::remove(edges.begin(), edges.end(), process2), 
                   edges.end());
    }
    
    // Detect cycle (deadlock)
    bool detectDeadlock(std::vector<int>& deadlockedProcesses) {
        std::vector<bool> visited(numProcesses, false);
        std::vector<bool> recStack(numProcesses, false);
        
        for (int i = 0; i < numProcesses; i++) {
            if (!visited[i]) {
                std::vector<int> cycle;
                if (hasCycleDFS(i, visited, recStack, cycle)) {
                    deadlockedProcesses = cycle;
                    return true;
                }
            }
        }
        
        return false;
    }
    
    void printGraph() {
        std::cout << "\n=== Wait-For Graph ===\n";
        for (int i = 0; i < numProcesses; i++) {
            if (!waitForGraph[i].empty()) {
                std::cout << "P" << i << " waits for: ";
                for (int p : waitForGraph[i]) {
                    std::cout << "P" << p << " ";
                }
                std::cout << "\n";
            }
        }
    }
};

int main() {
    // Create detector for 5 processes
    DeadlockDetector detector(5);
    
    // Build wait-for graph
    // P0 waits for P1
    detector.addWaitEdge(0, 1);
    // P1 waits for P2
    detector.addWaitEdge(1, 2);
    // P2 waits for P3
    detector.addWaitEdge(2, 3);
    // P3 waits for P4
    detector.addWaitEdge(3, 4);
    // P4 waits for P1 (creates cycle!)
    detector.addWaitEdge(4, 1);
    
    detector.printGraph();
    
    // Detect deadlock
    std::vector<int> deadlocked;
    if (detector.detectDeadlock(deadlocked)) {
        std::cout << "\n🚨 DEADLOCK DETECTED!\n";
        std::cout << "Deadlocked processes: ";
        for (int p : deadlocked) {
            std::cout << "P" << p << " ";
        }
        std::cout << "\n";
        
        // Recovery: Kill one process
        std::cout << "\nRecovery: Terminating P" << deadlocked[0] << "\n";
        // Remove all edges from/to this process
        for (int i = 0; i < 5; i++) {
            detector.removeWaitEdge(deadlocked[0], i);
            detector.removeWaitEdge(i, deadlocked[0]);
        }
        
        // Check again
        deadlocked.clear();
        if (!detector.detectDeadlock(deadlocked)) {
            std::cout << "✓ Deadlock resolved!\n";
        }
    } else {
        std::cout << "\n✓ No deadlock detected\n";
    }
    
    return 0;
}
```

---

### Exercise 3.2: Resource Allocation Graph Detection

```cpp
#include <iostream>
#include <vector>
#include <queue>

class RAGDetector {
private:
    int numProcesses;
    int numResources;
    
    // Allocation[i][j] = process i holds j instances of resource
    std::vector<std::vector<int>> allocation;
    // Request[i][j] = process i requests j instances of resource
    std::vector<std::vector<int>> request;
    // Available[j] = available instances of resource j
    std::vector<int> available;
    
public:
    RAGDetector(int processes, int resources) 
        : numProcesses(processes), numResources(resources) {
        allocation.resize(processes, std::vector<int>(resources, 0));
        request.resize(processes, std::vector<int>(resources, 0));
        available.resize(resources, 0);
    }
    
    void setAllocation(int process, int resource, int count) {
        allocation[process][resource] = count;
    }
    
    void setRequest(int process, int resource, int count) {
        request[process][resource] = count;
    }
    
    void setAvailable(int resource, int count) {
        available[resource] = count;
    }
    
    // Detect deadlock using resource allocation
    bool detectDeadlock(std::vector<int>& deadlockedProcesses) {
        std::vector<int> work = available;
        std::vector<bool> finish(numProcesses, false);
        
        // Mark processes with no requests as finished
        for (int i = 0; i < numProcesses; i++) {
            bool hasRequest = false;
            for (int j = 0; j < numResources; j++) {
                if (request[i][j] > 0) {
                    hasRequest = true;
                    break;
                }
            }
            if (!hasRequest) {
                finish[i] = true;
            }
        }
        
        // Try to satisfy requests
        bool progress = true;
        while (progress) {
            progress = false;
            
            for (int i = 0; i < numProcesses; i++) {
                if (!finish[i]) {
                    // Check if request can be satisfied
                    bool canSatisfy = true;
                    for (int j = 0; j < numResources; j++) {
                        if (request[i][j] > work[j]) {
                            canSatisfy = false;
                            break;
                        }
                    }
                    
                    if (canSatisfy) {
                        // Grant resources
                        for (int j = 0; j < numResources; j++) {
                            work[j] += allocation[i][j];
                        }
                        finish[i] = true;
                        progress = true;
                    }
                }
            }
        }
        
        // Collect deadlocked processes
        for (int i = 0; i < numProcesses; i++) {
            if (!finish[i]) {
                deadlockedProcesses.push_back(i);
            }
        }
        
        return !deadlockedProcesses.empty();
    }
    
    void printState() {
        std::cout << "\n=== Resource Allocation State ===\n";
        
        std::cout << "Available: ";
        for (int i = 0; i < numResources; i++) {
            std::cout << "R" << i << "=" << available[i] << " ";
        }
        std::cout << "\n\nAllocation:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                std::cout << allocation[i][j] << " ";
            }
            std::cout << "\n";
        }
        
        std::cout << "\nRequest:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                std::cout << request[i][j] << " ";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    // 5 processes, 3 resource types
    RAGDetector detector(5, 3);
    
    // Set available resources
    detector.setAvailable(0, 0); // R0: 0 available
    detector.setAvailable(1, 0); // R1: 0 available
    detector.setAvailable(2, 0); // R2: 0 available
    
    // Set allocations
    detector.setAllocation(0, 0, 1);
    detector.setAllocation(1, 1, 1);
    detector.setAllocation(2, 2, 1);
    detector.setAllocation(3, 0, 1);
    detector.setAllocation(4, 1, 1);
    
    // Set requests (creating deadlock)
    detector.setRequest(0, 1, 1); // P0 wants R1
    detector.setRequest(1, 2, 1); // P1 wants R2
    detector.setRequest(2, 0, 1); // P2 wants R0 (cycle!)
    detector.setRequest(3, 1, 1); // P3 wants R1
    detector.setRequest(4, 2, 1); // P4 wants R2
    
    detector.printState();
    
    // Detect deadlock
    std::vector<int> deadlocked;
    if (detector.detectDeadlock(deadlocked)) {
        std::cout << "\n🚨 DEADLOCK DETECTED!\n";
        std::cout << "Deadlocked processes: ";
        for (int p : deadlocked) {
            std::cout << "P" << p << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "\n✓ No deadlock detected\n";
    }
    
    return 0;
}
```

---

### Exercise 3.3: Your Turn - Detect Deadlock

**Scenario**:
- 4 processes (P0, P1, P2, P3)
- 2 resource types (R0, R1)
- Available: R0=0, R1=0

**Allocation**:
```
     R0  R1
P0:   1   0
P1:   0   1
P2:   1   0
P3:   0   1
```

**Request**:
```
     R0  R1
P0:   0   1
P1:   1   0
P2:   0   1
P3:   1   0
```

**Questions**:
1. Draw the wait-for graph:
```
Your drawing/description here:




```

2. Is there a deadlock? If yes, which processes are deadlocked?
```
Your Answer: _______________________________________________
```

3. Suggest a recovery strategy:
```
Your Answer: _______________________________________________
_______________________________________________
```

---

## **Part 4: Comprehensive Exercise**

### Scenario: Bank Account Transfers

Implement a system that prevents deadlock when multiple threads transfer money between accounts.

**Requirements**:
- 5 bank accounts
- Multiple threads performing transfers
- Use deadlock prevention technique
- Must not lose or create money

**Your Implementation**:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>

class BankAccount {
private:
    double balance;
    std::mutex mtx;
    int accountId;
    
public:
    BankAccount(int id, double initial) 
        : accountId(id), balance(initial) {}
    
    // TODO: Implement safe transfer method
    // Use lock ordering or std::lock to prevent deadlock
    
    static bool transfer(BankAccount& from, BankAccount& to, double amount) {
        // Your code here
        
        
        
        
        return true;
    }
    
    double getBalance() {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }
    
    int getId() const { return accountId; }
};

int main() {
    // Create accounts
    std::vector<BankAccount> accounts;
    for (int i = 0; i < 5; i++) {
        accounts.emplace_back(i, 1000.0);
    }
    
    // Create threads that perform random transfers
    // TODO: Implement this
    
    return 0;
}
```

---

## **Answer Key**

### Exercise 1.1 Answer:
**Why does this create a deadlock?**
- Thread 1 locks mutex1 and waits for mutex2
- Thread 2 locks mutex2 and waits for mutex1
- This creates a circular wait condition
- All four Coffman conditions are satisfied

### Exercise 1.4 Solution:
```cpp
// Solution: Lock all resources in order A -> B -> C

void process1() {
    resourceA.lock();
    resourceB.lock();
    // Work...
    resourceB.unlock();
    resourceA.unlock();
}

void process2() {
    resourceB.lock();
    resourceC.lock();
    // Work...
    resourceC.unlock();
    resourceB.unlock();
}

void process3() {
    resourceA.lock();  // Changed from resourceC
    resourceC.lock();
    // Work...
    resourceC.unlock();
    resourceA.unlock();
}
```

### Exercise 2.2 Answers:

**1. Need Matrix**:
```
     A  B  C
P0:  2  2  1
P1:  2  1  1
P2:  2  2  2
P3:  2  2  2
```

**2. Safe State**: Yes, safe sequence exists: P1 → P3 → P0 → P2

**3. P2 request (0,1,1)**: 
- Check: (0,1,1) ≤ (2,2,2) ✓
- Check: (0,1,1) ≤ (2,3,2) ✓
- After allocation: Available = (2,2,1)
- Safe sequence still exists: P1 → P3 → P2 → P0
- **Request should be GRANTED**

### Exercise 3.3 Answers:

**1. Wait-For Graph**:
- P0 → P1 (P0 waits for R1 held by P1)
- P1 → P0 (P1 waits for R0 held by P0)
- P2 → P1 (P2 waits for R1 held by P1)
- P3 → P0 (P3 waits for R0 held by P0)

**2. Deadlock**: YES. P0 and P1 are in a circular wait.

**3. Recovery**: 
- Terminate P0 or P1 (victim selection)
- Rollback P0 or P1 to previous state
- Preempt R0 from P0 or R1 from P1

### Exercise 4 Solution:
```cpp
static bool transfer(BankAccount& from, BankAccount& to, double amount) {
    if (&from == &to) return false;
    
    // Use address comparison for consistent ordering
    BankAccount* first = (&from < &to) ? &from : &to;
    BankAccount* second = (&from < &to) ? &to : &from;
    
    std::lock(first->mtx, second->mtx);
    std::lock_guard<std::mutex> lock1(first->mtx, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(second->mtx, std::adopt_lock);
    
    if (from.balance >= amount) {
        from.balance -= amount;
        to.balance += amount;
        std::cout << "Transfer: $" << amount 
                  << " from Account " << from.accountId 
                  << " to Account " << to.accountId << "\n";
        return true;
    }
    return false;
}
```

---

## **Compilation Instructions**

```bash
# For basic examples
g++ -std=c++11 -pthread filename.cpp -o program

# With optimization
g++ -std=c++11 -pthread -O2 filename.cpp -o program

# Run
./program
```

---

## **Summary Table**

| Technique | Approach | Pros | Cons |
|-----------|----------|------|------|
| **Prevention** | Break one Coffman condition | No deadlocks occur | Resource underutilization |
| **Avoidance** | Check safe state before allocation | Optimal resource use | Requires future knowledge |
| **Detection** | Detect and recover | Maximum resource use | Overhead of detection |

---

## **Key Takeaways**

1. ✅ **Always lock mutexes in the same order** to prevent circular wait
2. ✅ Use `std::lock()` for locking multiple mutexes atomically
3. ✅ Use `std::lock_guard` or `std::unique_lock` for RAII
4. ✅ Banker's Algorithm ensures safety but needs advance information
5. ✅ Detection allows maximum concurrency but adds overhead

**End of Worksheet** 🎓