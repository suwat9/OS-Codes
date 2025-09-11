# Chapter 5: CPU Scheduling - Complete Worksheet
*Based on Operating System Concepts 10th Edition (2018)*  
*Authors: Abraham Silberschatz, Peter Baer Galvin, and Greg Gagne*

---

## 5.1 Basic Concepts

### 5.1.1 Content Overview
CPU scheduling is fundamental to multiprogramming operating systems. The CPU scheduler selects from ready processes in memory and allocates the CPU to one of them. CPU scheduling decisions may take place when a process:
- Switches from running to waiting state
- Switches from running to ready state  
- Switches from waiting to ready state
- Terminates

**Key Terms:**
- **CPU Burst**: Time spent executing on CPU
- **I/O Burst**: Time spent waiting for I/O operations
- **Preemptive Scheduling**: OS can interrupt running processes
- **Non-preemptive Scheduling**: Process runs until completion or voluntary yield
- **Dispatcher**: Module that gives control of CPU to selected process
- **Dispatch Latency**: Time to stop one process and start another

### 5.1.2 C++ Implementation - Basic Process Structure

```cpp
// File: process_basics.cpp
// Compile: g++ -o process_basics process_basics.cpp -std=c++17

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>

struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int priority;
    
    Process(int id, int at, int bt, int pr = 0) 
        : pid(id), arrival_time(at), burst_time(bt), 
          remaining_time(bt), priority(pr) {}
};

class ProcessScheduler {
public:
    std::vector<Process> processes;
    
    void addProcess(int pid, int arrival, int burst, int priority = 0) {
        processes.emplace_back(pid, arrival, burst, priority);
    }
    
    void displayProcesses() {
        std::cout << std::setw(5) << "PID" << std::setw(10) << "Arrival" 
                  << std::setw(10) << "Burst" << std::setw(12) << "Completion" 
                  << std::setw(12) << "Turnaround" << std::setw(10) << "Waiting\n";
        std::cout << std::string(60, '-') << "\n";
        
        for (const auto& p : processes) {
            std::cout << std::setw(5) << p.pid << std::setw(10) << p.arrival_time
                      << std::setw(10) << p.burst_time << std::setw(12) << p.completion_time
                      << std::setw(12) << p.turnaround_time << std::setw(10) << p.waiting_time << "\n";
        }
    }
    
    double calculateAverageWaitingTime() {
        int total = 0;
        for (const auto& p : processes) {
            total += p.waiting_time;
        }
        return static_cast<double>(total) / processes.size();
    }
    
    double calculateAverageTurnaroundTime() {
        int total = 0;
        for (const auto& p : processes) {
            total += p.turnaround_time;
        }
        return static_cast<double>(total) / processes.size();
    }
};

// Demo main function
int main() {
    ProcessScheduler scheduler;
    
    // Example processes
    scheduler.addProcess(1, 0, 7);
    scheduler.addProcess(2, 2, 4);
    scheduler.addProcess(3, 4, 1);
    scheduler.addProcess(4, 5, 4);
    
    std::cout << "Basic Process Structure Demo\n";
    std::cout << "============================\n";
    scheduler.displayProcesses();
    
    return 0;
}
```

### 5.1.3 Multiple Choice Quiz

1. **Which of the following is NOT a CPU scheduling decision point?**
   - A) Process switches from running to waiting
   - B) Process switches from running to ready
   - C) Process allocation of memory
   - D) Process terminates
   - **Answer: C**

2. **What is dispatch latency?**
   - A) Time for process to complete
   - B) Time to stop one process and start another
   - C) Time process waits in ready queue
   - D) Time for I/O operations
   - **Answer: B**

3. **In preemptive scheduling:**
   - A) Process runs until completion
   - B) OS cannot interrupt running processes
   - C) OS can forcibly remove CPU from process
   - D) Only I/O can interrupt process
   - **Answer: C**

4. **CPU burst refers to:**
   - A) Time spent waiting for I/O
   - B) Time spent executing on CPU
   - C) Total process execution time
   - D) Time in ready queue
   - **Answer: B**

5. **The dispatcher is responsible for:**
   - A) Selecting next process to run
   - B) Managing memory allocation
   - C) Giving control of CPU to selected process
   - D) Handling I/O operations
   - **Answer: C**

### 5.1.4 Key Pitfalls & Summary

**Common Pitfalls:**
- Confusing scheduling algorithms with dispatcher functions
- Not understanding difference between preemptive and non-preemptive
- Mixing up CPU burst with I/O burst concepts
- Ignoring context switching overhead in calculations

**Short Summary:** CPU scheduling manages which process gets CPU time. The scheduler selects processes, while the dispatcher performs the actual switch. Understanding burst patterns and preemption is crucial for effective scheduling.

### 5.1.5 Future Implementation Ideas
- Implement a CPU burst time predictor using exponential averaging
- Create visualization of process state transitions
- Develop a simulator showing context switching overhead
- Build a multi-level feedback queue scheduler

---

## 5.2 Scheduling Criteria

### 5.2.1 Content Overview

**Performance Metrics:**
- **CPU Utilization**: Percentage of time CPU is busy (40-90%)
- **Throughput**: Number of processes completed per time unit
- **Turnaround Time**: Time from submission to completion
- **Waiting Time**: Time spent in ready queue
- **Response Time**: Time from submission to first response

**Optimization Goals:**
- Maximize: CPU utilization, Throughput
- Minimize: Turnaround time, Waiting time, Response time

### 5.2.2 C++ Implementation - Performance Metrics Calculator

```cpp
// File: scheduling_metrics.cpp
// Compile: g++ -o scheduling_metrics scheduling_metrics.cpp -std=c++17

#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

class MetricsCalculator {
private:
    std::vector<Process> processes;
    int total_time;
    int cpu_idle_time;

public:
    void setProcesses(const std::vector<Process>& procs) {
        processes = procs;
        calculateTotalTime();
    }
    
    void calculateTotalTime() {
        if (processes.empty()) return;
        
        int max_completion = 0;
        for (const auto& p : processes) {
            max_completion = std::max(max_completion, p.completion_time);
        }
        total_time = max_completion;
    }
    
    double getCPUUtilization() {
        int cpu_busy_time = total_time - cpu_idle_time;
        return (static_cast<double>(cpu_busy_time) / total_time) * 100.0;
    }
    
    double getThroughput() {
        return static_cast<double>(processes.size()) / total_time;
    }
    
    double getAverageWaitingTime() {
        int total_waiting = 0;
        for (const auto& p : processes) {
            total_waiting += p.waiting_time;
        }
        return static_cast<double>(total_waiting) / processes.size();
    }
    
    double getAverageTurnaroundTime() {
        int total_turnaround = 0;
        for (const auto& p : processes) {
            total_turnaround += p.turnaround_time;
        }
        return static_cast<double>(total_turnaround) / processes.size();
    }
    
    double getAverageResponseTime() {
        // Assuming response time equals waiting time for simplicity
        return getAverageWaitingTime();
    }
    
    void displayMetrics() {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n=== SCHEDULING METRICS ===\n";
        std::cout << "CPU Utilization: " << getCPUUtilization() << "%\n";
        std::cout << "Throughput: " << getThroughput() << " processes/unit time\n";
        std::cout << "Average Waiting Time: " << getAverageWaitingTime() << " units\n";
        std::cout << "Average Turnaround Time: " << getAverageTurnaroundTime() << " units\n";
        std::cout << "Average Response Time: " << getAverageResponseTime() << " units\n";
    }
    
    void setCPUIdleTime(int idle) { cpu_idle_time = idle; }
};

// Demo usage
int main() {
    std::vector<Process> sample_processes = {
        Process(1, 0, 7), Process(2, 2, 4), Process(3, 4, 1)
    };
    
    // Simulate some completion times
    sample_processes[0].completion_time = 7;
    sample_processes[0].turnaround_time = 7;
    sample_processes[0].waiting_time = 0;
    
    sample_processes[1].completion_time = 11;
    sample_processes[1].turnaround_time = 9;
    sample_processes[1].waiting_time = 5;
    
    sample_processes[2].completion_time = 12;
    sample_processes[2].turnaround_time = 8;
    sample_processes[2].waiting_time = 7;
    
    MetricsCalculator calc;
    calc.setProcesses(sample_processes);
    calc.setCPUIdleTime(0);
    calc.displayMetrics();
    
    return 0;
}
```

### 5.2.3 Multiple Choice Quiz

1. **Which metric should be maximized for better performance?**
   - A) Waiting time
   - B) Response time
   - C) CPU utilization
   - D) Turnaround time
   - **Answer: C**

2. **Turnaround time is calculated as:**
   - A) Completion time - Arrival time
   - B) Burst time + Waiting time
   - C) Both A and B are correct
   - D) Response time + Burst time
   - **Answer: C**

3. **In an interactive system, which metric is most important?**
   - A) Throughput
   - B) CPU utilization
   - C) Response time
   - D) Turnaround time
   - **Answer: C**

4. **Throughput is measured as:**
   - A) Time per process
   - B) Processes per time unit
   - C) CPU busy percentage
   - D) Average waiting time
   - **Answer: B**

5. **Waiting time includes:**
   - A) Time executing on CPU
   - B) Time waiting for I/O
   - C) Time in ready queue only
   - D) Total process lifetime
   - **Answer: C**

### 5.2.4 Key Pitfalls & Summary

**Common Pitfalls:**
- Confusing response time with turnaround time
- Not accounting for arrival times in calculations
- Assuming all metrics can be optimized simultaneously
- Forgetting that different system types prioritize different metrics

**Short Summary:** Scheduling criteria provide measurable ways to evaluate scheduler performance. Different systems prioritize different metrics: batch systems focus on throughput, interactive systems on response time.

### 5.2.5 Future Implementation Ideas
- Create a multi-objective optimization scheduler
- Implement adaptive metric weight adjustment
- Build a real-time performance monitor
- Develop workload-specific metric calculators

---

## 5.3 Scheduling Algorithms

### 5.3.1 Content Overview

**Major Algorithms:**
1. **First-Come, First-Served (FCFS)**: Non-preemptive, simple but can cause convoy effect
2. **Shortest-Job-First (SJF)**: Optimal average waiting time, but requires burst time prediction
3. **Shortest-Remaining-Time-First (SRTF)**: Preemptive version of SJF
4. **Priority Scheduling**: Processes have priorities, can cause starvation
5. **Round Robin (RR)**: Time quantum based, good for interactive systems
6. **Multilevel Queue**: Multiple queues with different priorities
7. **Multilevel Feedback Queue**: Processes can move between queues

### 5.3.2 C++ Implementation - Complete Scheduling Algorithms

```cpp
// File: scheduling_algorithms.cpp
// Compile: g++ -o scheduling_algorithms scheduling_algorithms.cpp -std=c++17

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>

class SchedulingAlgorithms {
public:
    // FCFS Scheduling
    static void FCFS(std::vector<Process>& processes) {
        std::sort(processes.begin(), processes.end(), 
                  [](const Process& a, const Process& b) {
                      return a.arrival_time < b.arrival_time;
                  });
        
        int current_time = 0;
        for (auto& p : processes) {
            if (current_time < p.arrival_time) {
                current_time = p.arrival_time;
            }
            p.completion_time = current_time + p.burst_time;
            p.turnaround_time = p.completion_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
            current_time = p.completion_time;
        }
    }
    
    // SJF Non-preemptive Scheduling
    static void SJF(std::vector<Process>& processes) {
        int n = processes.size();
        std::vector<bool> completed(n, false);
        int current_time = 0;
        int completed_count = 0;
        
        while (completed_count < n) {
            int shortest_job = -1;
            int min_burst = INT_MAX;
            
            for (int i = 0; i < n; i++) {
                if (!completed[i] && processes[i].arrival_time <= current_time) {
                    if (processes[i].burst_time < min_burst) {
                        min_burst = processes[i].burst_time;
                        shortest_job = i;
                    }
                }
            }
            
            if (shortest_job == -1) {
                current_time++;
                continue;
            }
            
            processes[shortest_job].completion_time = current_time + processes[shortest_job].burst_time;
            processes[shortest_job].turnaround_time = processes[shortest_job].completion_time - processes[shortest_job].arrival_time;
            processes[shortest_job].waiting_time = processes[shortest_job].turnaround_time - processes[shortest_job].burst_time;
            
            current_time = processes[shortest_job].completion_time;
            completed[shortest_job] = true;
            completed_count++;
        }
    }
    
    // SRTF (Preemptive SJF) Scheduling
    static void SRTF(std::vector<Process>& processes) {
        int n = processes.size();
        std::vector<int> remaining_time(n);
        
        for (int i = 0; i < n; i++) {
            remaining_time[i] = processes[i].burst_time;
        }
        
        int current_time = 0;
        int completed = 0;
        
        while (completed < n) {
            int shortest = -1;
            int min_remaining = INT_MAX;
            
            for (int i = 0; i < n; i++) {
                if (processes[i].arrival_time <= current_time && 
                    remaining_time[i] < min_remaining && remaining_time[i] > 0) {
                    min_remaining = remaining_time[i];
                    shortest = i;
                }
            }
            
            if (shortest == -1) {
                current_time++;
                continue;
            }
            
            remaining_time[shortest]--;
            current_time++;
            
            if (remaining_time[shortest] == 0) {
                completed++;
                processes[shortest].completion_time = current_time;
                processes[shortest].turnaround_time = processes[shortest].completion_time - processes[shortest].arrival_time;
                processes[shortest].waiting_time = processes[shortest].turnaround_time - processes[shortest].burst_time;
            }
        }
    }
    
    // Round Robin Scheduling
    static void RoundRobin(std::vector<Process>& processes, int quantum) {
        std::queue<int> ready_queue;
        std::vector<int> remaining_time(processes.size());
        
        for (size_t i = 0; i < processes.size(); i++) {
            remaining_time[i] = processes[i].burst_time;
        }
        
        int current_time = 0;
        
        // Add initial processes to queue
        for (size_t i = 0; i < processes.size(); i++) {
            if (processes[i].arrival_time <= current_time) {
                ready_queue.push(i);
            }
        }
        
        while (!ready_queue.empty()) {
            int current_process = ready_queue.front();
            ready_queue.pop();
            
            int exec_time = std::min(quantum, remaining_time[current_process]);
            remaining_time[current_process] -= exec_time;
            current_time += exec_time;
            
            // Add newly arrived processes
            for (size_t i = 0; i < processes.size(); i++) {
                if (processes[i].arrival_time <= current_time && remaining_time[i] > 0) {
                    bool already_in_queue = false;
                    std::queue<int> temp_queue = ready_queue;
                    while (!temp_queue.empty()) {
                        if (temp_queue.front() == (int)i) {
                            already_in_queue = true;
                            break;
                        }
                        temp_queue.pop();
                    }
                    if (!already_in_queue && i != current_process) {
                        ready_queue.push(i);
                    }
                }
            }
            
            if (remaining_time[current_process] == 0) {
                processes[current_process].completion_time = current_time;
                processes[current_process].turnaround_time = processes[current_process].completion_time - processes[current_process].arrival_time;
                processes[current_process].waiting_time = processes[current_process].turnaround_time - processes[current_process].burst_time;
            } else {
                ready_queue.push(current_process);
            }
        }
    }
    
    // Priority Scheduling (Non-preemptive)
    static void PriorityScheduling(std::vector<Process>& processes) {
        int n = processes.size();
        std::vector<bool> completed(n, false);
        int current_time = 0;
        int completed_count = 0;
        
        while (completed_count < n) {
            int highest_priority_job = -1;
            int highest_priority = INT_MAX; // Lower number = higher priority
            
            for (int i = 0; i < n; i++) {
                if (!completed[i] && processes[i].arrival_time <= current_time) {
                    if (processes[i].priority < highest_priority) {
                        highest_priority = processes[i].priority;
                        highest_priority_job = i;
                    }
                }
            }
            
            if (highest_priority_job == -1) {
                current_time++;
                continue;
            }
            
            processes[highest_priority_job].completion_time = current_time + processes[highest_priority_job].burst_time;
            processes[highest_priority_job].turnaround_time = processes[highest_priority_job].completion_time - processes[highest_priority_job].arrival_time;
            processes[highest_priority_job].waiting_time = processes[highest_priority_job].turnaround_time - processes[highest_priority_job].burst_time;
            
            current_time = processes[highest_priority_job].completion_time;
            completed[highest_priority_job] = true;
            completed_count++;
        }
    }
};

// Demo main function
int main() {
    // Test data
    std::vector<Process> processes = {
        Process(1, 0, 7, 2),
        Process(2, 2, 4, 1),
        Process(3, 4, 1, 4),
        Process(4, 5, 4, 3)
    };
    
    std::cout << "=== FCFS Scheduling ===\n";
    auto fcfs_processes = processes;
    SchedulingAlgorithms::FCFS(fcfs_processes);
    ProcessScheduler scheduler;
    scheduler.processes = fcfs_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n\n";
    
    std::cout << "=== SJF Scheduling ===\n";
    auto sjf_processes = processes;
    SchedulingAlgorithms::SJF(sjf_processes);
    scheduler.processes = sjf_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n\n";
    
    std::cout << "=== Round Robin (Quantum=2) Scheduling ===\n";
    auto rr_processes = processes;
    SchedulingAlgorithms::RoundRobin(rr_processes, 2);
    scheduler.processes = rr_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n\n";
    
    return 0;
}
```

### 5.3.3 Multiple Choice Quiz

1. **FCFS scheduling can suffer from:**
   - A) Starvation
   - B) Convoy effect
   - C) High context switching
   - D) Priority inversion
   - **Answer: B**

2. **SJF scheduling is optimal in terms of:**
   - A) CPU utilization
   - B) Throughput
   - C) Average waiting time
   - D) Response time
   - **Answer: C**

3. **Round Robin scheduling is best suited for:**
   - A) Batch processing
   - B) Real-time systems
   - C) Interactive systems
   - D) Background processes
   - **Answer: C**

4. **Priority scheduling can cause:**
   - A) Convoy effect
   - B) Starvation
   - C) High turnaround time
   - D) Low CPU utilization
   - **Answer: B**

5. **SRTF is the preemptive version of:**
   - A) FCFS
   - B) Priority scheduling
   - C) SJF
   - D) Round Robin
   - **Answer: C**

### 5.3.4 Key Pitfalls & Summary

**Common Pitfalls:**
- FCFS convoy effect with long processes
- SJF starvation of long processes
- RR performance degradation with wrong quantum
- Priority scheduling starvation without aging
- Not considering preemption overhead

**Short Summary:** Each algorithm has trade-offs. FCFS is simple but inefficient. SJF is optimal for waiting time but impractical. RR provides fairness. Priority scheduling offers control but risks starvation.

### 5.3.5 Future Implementation Ideas
- Implement aging mechanism for priority scheduling
- Create adaptive quantum size for Round Robin
- Build lottery scheduling algorithm
- Develop fair share scheduling

---

## 5.4 Thread Scheduling

### 5.4.1 Content Overview

**Key Concepts:**
- **User-level threads**: Scheduled by thread library
- **Kernel-level threads**: Scheduled by operating system
- **Process-Contention Scope (PCS)**: Competition among threads within same process
- **System-Contention Scope (SCS)**: Competition among all threads in system
- **Thread scheduling models**: Many-to-one, one-to-one, many-to-many

### 5.4.2 C++ Implementation - Thread Scheduling Simulation

```cpp
// File: thread_scheduling.cpp
// Compile: g++ -o thread_scheduling thread_scheduling.cpp -std=c++17 -pthread

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

class ThreadInfo {
public:
    int thread_id;
    int priority;
    int burst_time;
    std::chrono::steady_clock::time_point arrival_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point completion_time;
    
    ThreadInfo(int id, int prio, int burst) 
        : thread_id(id), priority(prio), burst_time(burst) {
        arrival_time = std::chrono::steady_clock::now();
    }
};

class ThreadScheduler {
private:
    std::queue<ThreadInfo> ready_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    std::atomic<int> active_threads{0};
    
public:
    void addThread(ThreadInfo thread_info) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        ready_queue.push(thread_info);
        active_threads++;
        cv.notify_one();
    }
    
    void scheduler() {
        while (running || active_threads > 0) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [this] { return !ready_queue.empty() || !running; });
            
            if (!ready_queue.empty()) {
                ThreadInfo current_thread = ready_queue.front();
                ready_queue.pop();
                lock.unlock();
                
                // Simulate thread execution
                current_thread.start_time = std::chrono::steady_clock::now();
                std::cout << "Executing Thread " << current_thread.thread_id 
                          << " (Priority: " << current_thread.priority << ")\n";
                
                std::this_thread::sleep_for(std::chrono::milliseconds(current_thread.burst_time * 100));
                
                current_thread.completion_time = std::chrono::steady_clock::now();
                
                auto turnaround_time = std::chrono::duration_cast<std::chrono::milliseconds>
                    (current_thread.completion_time - current_thread.arrival_time);
                
                std::cout << "Thread " << current_thread.thread_id 
                          << " completed. Turnaround time: " << turnaround_time.count() << "ms\n";
                
                active_threads--;
            }
        }
    }
    
    void stop() {
        running = false;
        cv.notify_all();
    }
};

// Pthread-style thread attributes simulation
class ThreadAttributes {
public:
    enum SchedulingPolicy { SCHED_FIFO, SCHED_RR, SCHED_OTHER };
    enum ContentionScope { PTHREAD_SCOPE_PROCESS, PTHREAD_SCOPE_SYSTEM };
    
    SchedulingPolicy policy = SCHED_OTHER;
    ContentionScope scope = PTHREAD_SCOPE_SYSTEM;
    int priority = 0;
    
    void setSchedulingPolicy(SchedulingPolicy pol) { policy = pol; }
    void setContentionScope(ContentionScope sc) { scope = sc; }
    void setPriority(int prio) { priority = prio; }
    
    void displayAttributes() {
        std::cout << "Thread Attributes:\n";
        std::cout << "  Policy: " << (policy == SCHED_FIFO ? "FIFO" : 
                                    policy == SCHED_RR ? "Round Robin" : "Other") << "\n";
        std::cout << "  Scope: " << (scope == PTHREAD_SCOPE_PROCESS ? "Process" : "System") << "\n";
        std::cout << "  Priority: " << priority << "\n";
    }
};

// Demo worker thread function
void workerThread(int id, int work_time) {
    std::cout << "Worker Thread " << id << " starting work for " << work_time << "ms\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(work_time));
    std::cout << "Worker Thread " << id << " completed work\n";
}

int main() {
    std::cout << "=== THREAD SCHEDULING DEMONSTRATION ===\n\n";
    
    // Demonstrate thread attributes
    ThreadAttributes attr;
    attr.setSchedulingPolicy(ThreadAttributes::SCHED_RR);
    attr.setContentionScope(ThreadAttributes::PTHREAD_SCOPE_SYSTEM);
    attr.setPriority(5);
    attr.displayAttributes();
    
    std::cout << "\n=== THREAD SCHEDULER SIMULATION ===\n";
    
    ThreadScheduler scheduler;
    
    // Start scheduler in separate thread
    std::thread scheduler_thread(&ThreadScheduler::scheduler, &scheduler);
    
    // Create and schedule threads
    scheduler.addThread(ThreadInfo(1, 3, 5));
    scheduler.addThread(ThreadInfo(2, 1, 3));
    scheduler.addThread(ThreadInfo(3, 2, 4));
    scheduler.addThread(ThreadInfo(4, 1, 2));
    
    // Wait for all threads to complete
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    scheduler.stop();
    scheduler_thread.join();
    
    std::cout << "\n=== PTHREAD STYLE THREADS ===\n";
    
    // Create multiple worker threads
    std::vector<std::thread> workers;
    
    for (int i = 1; i <= 4; i++) {
        workers.emplace_back(workerThread, i, i * 200);
    }
    
    // Wait for all workers to complete
    for (auto& worker : workers) {
        worker.join();
    }
    
    std::cout << "\nAll threads completed!\n";
    return 0;
}
```

### 5.4.3 Multiple Choice Quiz

1. **Process-Contention Scope (PCS) means:**
   - A) Threads compete with all system threads
   - B) Threads compete within the same process
   - C) Threads compete for system resources
   - D) Threads compete with kernel threads
   - **Answer: B**

2. **User-level threads are scheduled by:**
   - A) Operating system kernel
   - B) Hardware scheduler
   - C) Thread library
   - D) Process manager
   - **Answer: C**

3. **In the many-to-one model:**
   - A) Multiple kernel threads map to one user thread
   - B) Multiple user threads map to one kernel thread
   - C) Each user thread maps to one kernel thread
   - D) Threads are scheduled by hardware
   - **Answer: B**

4. **System-Contention Scope (SCS) involves:**
   - A) Competition among threads in same process
   - B) Competition among all threads in system
   - C) Competition for process resources
   - D) Competition for user-level scheduling
   - **Answer: B**

5. **Kernel-level threads have the advantage of:**
   - A) Lower overhead
   - B) Faster context switching
   - C) True parallelism on multiprocessors
   - D) No system calls required
   - **Answer: C**

### 5.4.4 Key Pitfalls & Summary

**Common Pitfalls:**
- Confusing user-level and kernel-level thread scheduling
- Not understanding contention scope implications
- Assuming all threads have equal scheduling opportunities
- Ignoring thread library implementation details

**Short Summary:** Thread scheduling operates at two levels: user-level (by thread library) and kernel-level (by OS). The choice between PCS and SCS affects performance and resource competition.

### 5.4.5 Future Implementation Ideas
- Implement a user-level thread scheduler
- Create thread pool with work-stealing algorithm
- Build priority inheritance mechanism
- Develop thread affinity scheduler for NUMA systems

---

## 5.5 Multi-Processor Scheduling

### 5.5.1 Content Overview

**Key Concepts:**
- **Asymmetric Multiprocessing**: One processor handles scheduling decisions
- **Symmetric Multiprocessing (SMP)**: Each processor schedules itself
- **Processor Affinity**: Process prefers to run on same processor
- **Load Balancing**: Distributing work evenly across processors
- **NUMA (Non-Uniform Memory Access)**: Memory access times vary by location

**Approaches:**
- **Push Migration**: Periodic task to redistribute load
- **Pull Migration**: Idle processor pulls tasks from busy processor
- **Soft Affinity**: Attempt to keep process on same processor
- **Hard Affinity**: Process bound to specific processor set

### 5.5.2 C++ Implementation - Multi-Processor Scheduling Simulation

```cpp
// File: multiprocessor_scheduling.cpp
// Compile: g++ -o multiprocessor_scheduling multiprocessor_scheduling.cpp -std=c++17 -pthread

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm>

class Task {
public:
    int task_id;
    int burst_time;
    int preferred_cpu;
    std::chrono::steady_clock::time_point arrival_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point completion_time;
    
    Task(int id, int burst, int cpu = -1) 
        : task_id(id), burst_time(burst), preferred_cpu(cpu) {
        arrival_time = std::chrono::steady_clock::now();
    }
};

class CPUCore {
public:
    int core_id;
    std::queue<Task> local_queue;
    std::mutex queue_mutex;
    std::atomic<bool> is_busy{false};
    std::atomic<int> load{0};
    
    CPUCore(int id) : core_id(id) {}
    
    void addTask(const Task& task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        local_queue.push(task);
        load++;
    }
    
    bool getTask(Task& task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (!local_queue.empty()) {
            task = local_queue.front();
            local_queue.pop();
            load--;
            return true;
        }
        return false;
    }
    
    int getQueueSize() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return local_queue.size();
    }
    
    bool isEmpty() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return local_queue.empty();
    }
};

class MultiProcessorScheduler {
private:
    std::vector<CPUCore> cores;
    std::queue<Task> global_queue;
    std::mutex global_mutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    std::atomic<int> active_tasks{0};
    int num_cores;
    
    // Load balancing parameters
    static constexpr int LOAD_BALANCE_THRESHOLD = 2;
    static constexpr int MIGRATION_COST = 5; // milliseconds
    
public:
    MultiProcessorScheduler(int cores_count) : num_cores(cores_count) {
        for (int i = 0; i < cores_count; i++) {
            cores.emplace_back(i);
        }
    }
    
    void addTask(const Task& task) {
        if (task.preferred_cpu >= 0 && task.preferred_cpu < num_cores) {
            // Processor affinity - try preferred CPU first
            cores[task.preferred_cpu].addTask(task);
        } else {
            // Global queue for load balancing
            std::lock_guard<std::mutex> lock(global_mutex);
            global_queue.push(task);
        }
        active_tasks++;
        cv.notify_all();
    }
    
    void cpuScheduler(int core_id) {
        std::cout << "CPU Core " << core_id << " scheduler started\n";
        
        while (running || active_tasks > 0) {
            Task current_task(0, 0);
            bool has_task = false;
            
            // Try to get task from local queue first (processor affinity)
            if (cores[core_id].getTask(current_task)) {
                has_task = true;
            }
            // Try global queue
            else {
                std::lock_guard<std::mutex> lock(global_mutex);
                if (!global_queue.empty()) {
                    current_task = global_queue.front();
                    global_queue.pop();
                    has_task = true;
                }
            }
            
            // Work stealing - try to steal from other cores
            if (!has_task) {
                has_task = workStealing(core_id, current_task);
            }
            
            if (has_task) {
                executeTask(core_id, current_task);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        
        std::cout << "CPU Core " << core_id << " scheduler stopped\n";
    }
    
    bool workStealing(int core_id, Task& stolen_task) {
        // Find the most loaded core
        int max_load = 0;
        int victim_core = -1;
        
        for (int i = 0; i < num_cores; i++) {
            if (i != core_id && cores[i].getQueueSize() > max_load + LOAD_BALANCE_THRESHOLD) {
                max_load = cores[i].getQueueSize();
                victim_core = i;
            }
        }
        
        if (victim_core != -1 && cores[victim_core].getTask(stolen_task)) {
            std::cout << "Core " << core_id << " stole task " << stolen_task.task_id 
                      << " from Core " << victim_core << "\n";
            return true;
        }
        
        return false;
    }
    
    void executeTask(int core_id, Task& task) {
        cores[core_id].is_busy = true;
        task.start_time = std::chrono::steady_clock::now();
        
        std::cout << "Core " << core_id << " executing Task " << task.task_id 
                  << " (Burst: " << task.burst_time << "ms)\n";
        
        // Simulate task execution
        std::this_thread::sleep_for(std::chrono::milliseconds(task.burst_time));
        
        task.completion_time = std::chrono::steady_clock::now();
        
        auto turnaround_time = std::chrono::duration_cast<std::chrono::milliseconds>
            (task.completion_time - task.arrival_time);
        
        std::cout << "Core " << core_id << " completed Task " << task.task_id 
                  << " (Turnaround: " << turnaround_time.count() << "ms)\n";
        
        cores[core_id].is_busy = false;
        active_tasks--;
    }
    
    void loadBalancer() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Check load imbalance
            int min_load = INT_MAX, max_load = 0;
            int min_core = -1, max_core = -1;
            
            for (int i = 0; i < num_cores; i++) {
                int load = cores[i].getQueueSize();
                if (load < min_load) {
                    min_load = load;
                    min_core = i;
                }
                if (load > max_load) {
                    max_load = load;
                    max_core = i;
                }
            }
            
            // Migrate tasks if imbalance is significant
            if (max_load - min_load > LOAD_BALANCE_THRESHOLD && max_core != -1 && min_core != -1) {
                Task migrated_task(0, 0);
                if (cores[max_core].getTask(migrated_task)) {
                    cores[min_core].addTask(migrated_task);
                    std::cout << "Load Balancer: Migrated Task " << migrated_task.task_id 
                              << " from Core " << max_core << " to Core " << min_core << "\n";
                }
            }
        }
    }
    
    void displayStats() {
        std::cout << "\n=== CPU CORE STATISTICS ===\n";
        for (int i = 0; i < num_cores; i++) {
            std::cout << "Core " << i << ": Queue Size = " << cores[i].getQueueSize()
                      << ", Busy = " << (cores[i].is_busy ? "Yes" : "No") << "\n";
        }
        std::cout << "Active Tasks: " << active_tasks.load() << "\n";
    }
    
    void stop() {
        running = false;
        cv.notify_all();
    }
};

// NUMA-aware scheduler simulation
class NUMAScheduler {
private:
    struct NUMANode {
        int node_id;
        std::vector<int> cpu_cores;
        int memory_latency; // Access latency in nanoseconds
        
        NUMANode(int id, std::vector<int> cores, int latency) 
            : node_id(id), cpu_cores(cores), memory_latency(latency) {}
    };
    
    std::vector<NUMANode> numa_nodes;
    
public:
    NUMAScheduler() {
        // Simulate 2 NUMA nodes
        numa_nodes.emplace_back(0, std::vector<int>{0, 1}, 100); // Local access
        numa_nodes.emplace_back(1, std::vector<int>{2, 3}, 300); // Remote access
    }
    
    int selectOptimalCore(int preferred_node = -1) {
        if (preferred_node >= 0 && preferred_node < numa_nodes.size()) {
            // Return first available core from preferred NUMA node
            const auto& node = numa_nodes[preferred_node];
            if (!node.cpu_cores.empty()) {
                return node.cpu_cores[0];
            }
        }
        
        // Find node with lowest memory latency that has available cores
        int best_node = 0;
        int min_latency = numa_nodes[0].memory_latency;
        
        for (size_t i = 1; i < numa_nodes.size(); i++) {
            if (numa_nodes[i].memory_latency < min_latency) {
                min_latency = numa_nodes[i].memory_latency;
                best_node = i;
            }
        }
        
        return numa_nodes[best_node].cpu_cores[0];
    }
    
    void displayNUMATopology() {
        std::cout << "\n=== NUMA TOPOLOGY ===\n";
        for (const auto& node : numa_nodes) {
            std::cout << "NUMA Node " << node.node_id 
                      << ": CPUs [";
            for (size_t i = 0; i < node.cpu_cores.size(); i++) {
                std::cout << node.cpu_cores[i];
                if (i < node.cpu_cores.size() - 1) std::cout << ", ";
            }
            std::cout << "], Memory Latency: " << node.memory_latency << "ns\n";
        }
    }
};

int main() {
    std::cout << "=== MULTI-PROCESSOR SCHEDULING DEMO ===\n\n";
    
    const int NUM_CORES = 4;
    MultiProcessorScheduler scheduler(NUM_CORES);
    
    // Start CPU schedulers
    std::vector<std::thread> cpu_threads;
    for (int i = 0; i < NUM_CORES; i++) {
        cpu_threads.emplace_back(&MultiProcessorScheduler::cpuScheduler, &scheduler, i);
    }
    
    // Start load balancer
    std::thread load_balancer_thread(&MultiProcessorScheduler::loadBalancer, &scheduler);
    
    // Generate tasks with different affinities
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> burst_dist(50, 200);
    std::uniform_int_distribution<> affinity_dist(0, NUM_CORES - 1);
    
    std::cout << "Generating tasks...\n";
    for (int i = 1; i <= 12; i++) {
        int burst_time = burst_dist(gen);
        int preferred_cpu = (i % 3 == 0) ? affinity_dist(gen) : -1; // Some tasks have affinity
        
        Task task(i, burst_time, preferred_cpu);
        scheduler.addTask(task);
        
        if (preferred_cpu >= 0) {
            std::cout << "Added Task " << i << " with CPU affinity to Core " << preferred_cpu << "\n";
        } else {
            std::cout << "Added Task " << i << " without CPU affinity\n";
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Let tasks execute
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    scheduler.displayStats();
    
    // Demonstrate NUMA awareness
    NUMAScheduler numa_scheduler;
    numa_scheduler.displayNUMATopology();
    
    std::cout << "\nOptimal core for NUMA node 0: " << numa_scheduler.selectOptimalCore(0) << "\n";
    std::cout << "Optimal core for NUMA node 1: " << numa_scheduler.selectOptimalCore(1) << "\n";
    
    // Stop scheduler
    scheduler.stop();
    load_balancer_thread.join();
    
    for (auto& thread : cpu_threads) {
        thread.join();
    }
    
    std::cout << "\nMulti-processor scheduling demo completed!\n";
    return 0;
}
```

### 5.5.3 Multiple Choice Quiz

1. **In asymmetric multiprocessing:**
   - A) All processors schedule themselves
   - B) One processor handles all scheduling decisions
   - C) Processors share scheduling responsibilities
   - D) Each processor has its own scheduler
   - **Answer: B**

2. **Processor affinity means:**
   - A) Process prefers specific processor
   - B) Processor prefers specific process
   - C) All processors are equivalent
   - D) Load balancing is disabled
   - **Answer: A**

3. **Pull migration occurs when:**
   - A) System redistributes load periodically
   - B) Idle processor pulls tasks from busy processor
   - C) Busy processor pushes tasks to idle processor
   - D) Load balancer migrates all tasks
   - **Answer: B**

4. **NUMA stands for:**
   - A) Non-Uniform Memory Architecture
   - B) Non-Uniform Memory Access
   - C) New Unified Memory Access
   - D) Network Unified Memory Architecture
   - **Answer: B**

5. **Hard affinity means:**
   - A) Process can run on any processor
   - B) Process prefers but can migrate
   - C) Process is bound to specific processor set
   - D) System ignores affinity settings
   - **Answer: C**

### 5.5.4 Key Pitfalls & Summary

**Common Pitfalls:**
- Ignoring cache effects when migrating processes
- Over-aggressive load balancing causing thrashing
- Not considering NUMA topology in scheduling
- Assuming all processors are identical
- Neglecting processor affinity benefits

**Short Summary:** Multi-processor scheduling balances load distribution with processor affinity. NUMA systems require topology-aware scheduling. Work stealing and migration help balance loads but have costs.

### 5.5.5 Future Implementation Ideas
- Implement gang scheduling for parallel applications
- Create NUMA-aware memory allocator integration
- Build cache-aware task migration policies
- Develop heterogeneous processor scheduling

---

