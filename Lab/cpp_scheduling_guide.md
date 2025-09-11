# C++ Thread and Multiprocessor Scheduling Guide

## Overview

This guide demonstrates advanced thread scheduling and multiprocessor scheduling concepts in C++ using modern threading libraries. The examples include priority-based thread scheduling, work stealing, load balancing, and NUMA-aware scheduling.

## Prerequisites

- C++17 or later compiler
- pthread library support
- WSL (Windows Subsystem for Linux) for Windows development

## Setting Up WSL for C++ Development

### Installing WSL

```powershell
# Run in PowerShell as Administrator
wsl --install
```

### Installing Build Tools

```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install build-essential

# Verify installation
g++ --version
```

### Compiling C++ Code in WSL

```bash
# Navigate to your code directory
cd /mnt/c/path/to/your/code

# Or copy to WSL filesystem for better performance
cp /mnt/c/path/to/code/file.cpp ~/

# Compile with threading support
g++ -o program program.cpp -std=c++17 -pthread

# Run the program
./program
```

## Thread Scheduling Implementation

### Key Concepts Demonstrated

1. **Priority-based Scheduling**: Higher priority threads execute first
2. **Thread Attributes**: Scheduling policies and contention scope
3. **Synchronization**: Mutexes, condition variables, and atomic operations
4. **Thread Safety**: Proper coordination between threads

### Common Issues and Solutions

#### Issue 1: Enum Name Conflicts

**Problem**: System headers define constants like `SCHED_FIFO`, `PTHREAD_SCOPE_SYSTEM`

**Error**:
```
error: expected identifier before numeric constant
```

**Solution**: Use custom enum names to avoid conflicts

```cpp
// Instead of conflicting names
enum SchedulingPolicy { SCHED_FIFO, SCHED_RR, SCHED_OTHER };

// Use custom names
enum SchedulingPolicy { POLICY_FIFO, POLICY_RR, POLICY_OTHER };
```

#### Issue 2: Missing Headers

**Problem**: Missing `#include <climits>` for `INT_MAX`

**Solution**: Add required headers

```cpp
#include <climits>  // For INT_MAX
#include <functional>  // For std::function
#include <memory>  // For std::unique_ptr
```

### Thread Scheduling Features

- **Priority Queue**: Implements priority-based scheduling
- **Thread Attributes**: Simulates pthread-style attributes
- **Condition Variables**: Efficient thread coordination
- **Atomic Operations**: Thread-safe counters and flags

## Multiprocessor Scheduling Implementation

### Key Concepts Demonstrated

1. **CPU Affinity**: Tasks can prefer specific cores
2. **Work Stealing**: Idle cores steal work from busy cores
3. **Load Balancing**: Automatic task migration between cores
4. **NUMA Awareness**: Memory locality optimization

### Architecture Overview

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   Core 0    │    │   Core 1    │    │   Core 2    │
│ Local Queue │    │ Local Queue │    │ Local Queue │
└─────────────┘    └─────────────┘    └─────────────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                           │
                  ┌─────────────┐
                  │ Global Queue│
                  │Load Balancer│
                  └─────────────┘
```

### Critical Issue: Mutex in std::vector

**Problem**: `CPUCore` contains `std::mutex` which is non-copyable/movable

**Error**:
```
error: static assertion failed: result type must be constructible from input type
```

**Root Cause**: `std::vector` needs to move/copy objects during resize operations

**Solution**: Use `std::unique_ptr` to manage `CPUCore` objects

```cpp
// Problem code
std::vector<CPUCore> cores;
cores.emplace_back(i);  // Fails - can't move mutex

// Solution
std::vector<std::unique_ptr<CPUCore>> cores;
cores.push_back(std::make_unique<CPUCore>(i));  // Works
```

### Load Balancing Algorithm

```cpp
void loadBalancer() {
    while (running.load()) {
        // Find most and least loaded cores
        int min_load = INT_MAX, max_load = 0;
        int min_core = -1, max_core = -1;
        
        for (int i = 0; i < num_cores; i++) {
            int load = cores[i]->getQueueSize();
            if (load < min_load) {
                min_load = load;
                min_core = i;
            }
            if (load > max_load) {
                max_load = load;
                max_core = i;
            }
        }
        
        // Migrate if imbalance exceeds threshold
        if (max_load - min_load > THRESHOLD) {
            migrateTask(max_core, min_core);
        }
    }
}
```

### Work Stealing Implementation

```cpp
bool workStealing(int core_id, Task& stolen_task) {
    int max_load = 0;
    int victim_core = -1;
    
    // Find most loaded core
    for (int i = 0; i < num_cores; i++) {
        if (i != core_id && 
            cores[i]->getQueueSize() > max_load + THRESHOLD) {
            max_load = cores[i]->getQueueSize();
            victim_core = i;
        }
    }
    
    // Steal task from victim
    if (victim_core != -1 && cores[victim_core]->getTask(stolen_task)) {
        return true;
    }
    return false;
}
```

## NUMA-Aware Scheduling

### NUMA Topology Simulation

```cpp
class NUMAScheduler {
private:
    struct NUMANode {
        int node_id;
        std::vector<int> cpu_cores;
        int memory_latency;  // Access latency in nanoseconds
    };
    
    std::vector<NUMANode> numa_nodes;
    
public:
    NUMAScheduler() {
        // Simulate 2 NUMA nodes
        numa_nodes.emplace_back(0, {0, 1}, 100);  // Local access
        numa_nodes.emplace_back(1, {2, 3}, 300);  // Remote access
    }
};
```

### Core Selection Algorithm

```cpp
int selectOptimalCore(int preferred_node = -1) {
    if (preferred_node >= 0) {
        // Use preferred NUMA node
        return numa_nodes[preferred_node].cpu_cores[0];
    }
    
    // Select node with lowest memory latency
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
```

## Compilation and Execution

### Build Commands

```bash
# Thread scheduling
g++ -o thread_scheduling thread_scheduling.cpp -std=c++17 -pthread

# Multiprocessor scheduling  
g++ -o multiprocessor_scheduling multiprocessor_scheduling.cpp -std=c++17 -pthread
```

### Expected Output

#### Thread Scheduling
```
=== THREAD SCHEDULING DEMONSTRATION ===

Thread Attributes:
  Policy: Round Robin
  Scope: System
  Priority: 5

=== THREAD SCHEDULER SIMULATION ===
Executing Thread 3 (Priority: 5)
Executing Thread 1 (Priority: 3)
Thread 3 completed. Turnaround time: 425ms
Executing Thread 2 (Priority: 1)
...
```

#### Multiprocessor Scheduling
```
=== MULTI-PROCESSOR SCHEDULING DEMO ===

CPU Core 0 scheduler started
CPU Core 1 scheduler started
...
Added Task 1 without CPU affinity
Added Task 3 with CPU affinity to Core 2
Core 0 executing Task 1 (Burst: 150ms)
Core 2 executing Task 3 (Burst: 75ms)
Core 1 stole task 2 from Core 0
Load Balancer: Migrated Task 4 from Core 2 to Core 1
...
```

## Performance Considerations

### Thread Scheduling
- **Priority Queues**: O(log n) insertion/removal
- **Condition Variables**: Efficient blocking/waking
- **Atomic Operations**: Lock-free counters

### Multiprocessor Scheduling
- **Work Stealing**: Reduces idle time
- **Load Balancing**: Prevents hotspots
- **NUMA Awareness**: Optimizes memory access

## Best Practices

1. **Use atomic operations** for shared counters
2. **Minimize lock contention** with fine-grained locking
3. **Avoid busy waiting** - use condition variables
4. **Handle thread-safe destruction** properly
5. **Test with different core counts** and workloads

## Troubleshooting

### Common Compilation Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `INT_MAX not declared` | Missing header | Add `#include <climits>` |
| `SCHED_FIFO conflicts` | System constant conflict | Use custom enum names |
| `non-copyable type` | Mutex in vector | Use `std::unique_ptr` |
| `pthread undefined` | Missing library | Add `-pthread` flag |

### Runtime Issues

- **Deadlocks**: Use lock ordering, timeouts
- **Race conditions**: Verify atomic operations
- **Memory leaks**: Use RAII, smart pointers
- **Poor performance**: Profile lock contention

## Extensions and Improvements

1. **Real-time scheduling**: Implement deadline-aware algorithms
2. **Energy-aware scheduling**: Consider power consumption
3. **Heterogeneous systems**: Support different core types
4. **Dynamic adaptation**: Adjust parameters based on workload
5. **Performance monitoring**: Add detailed statistics and profiling

## Conclusion

This implementation demonstrates key concepts in modern multiprocessor scheduling:

- **Scalability**: Handles multiple cores efficiently
- **Flexibility**: Supports various scheduling policies
- **Performance**: Minimizes idle time and load imbalance
- **Correctness**: Thread-safe with proper synchronization

The code serves as a foundation for understanding and implementing advanced scheduling algorithms in high-performance computing systems.