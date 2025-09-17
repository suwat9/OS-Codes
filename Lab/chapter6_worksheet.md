# Chapter 6: Synchronization Tools - Student Study Worksheet

## Overview
This worksheet contains the complete C++ implementation of Chapter 6 concepts plus exercises to help you master process synchronization through hands-on practice, analysis problems, and programming challenges.

---

## Complete C++ Source Code: synchronization_tools.cpp

Save this code as `synchronization_tools.cpp` and compile with: `g++ -std=c++17 -pthread -o sync_tools synchronization_tools.cpp`

```cpp
/*
 * Chapter 6: Synchronization Tools - C++17 Compatible Version
 * Operating Systems Concepts - Student Study Guide
 */

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std;
using namespace std::chrono;

//=============================================================================
// CUSTOM SEMAPHORE IMPLEMENTATION FOR C++17
//=============================================================================

class Semaphore {
private:
    mutex mtx;
    condition_variable cv;
    int count;

public:
    explicit Semaphore(int initial_count) : count(initial_count) {}

    void acquire() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this] { return count > 0; });
        --count;
    }

    void release() {
        unique_lock<mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }
    
    bool try_acquire() {
        lock_guard<mutex> lock(mtx);
        if (count > 0) {
            --count;
            return true;
        }
        return false;
    }
};

//=============================================================================
// 1. DEMONSTRATING RACE CONDITIONS (Section 6.1)
//=============================================================================

class RaceConditionDemo {
private:
    static int shared_counter;
    static const int ITERATIONS = 100000;
    
public:
    // Function that increments counter (unsafe - creates race condition)
    static void unsafe_increment() {
        for (int i = 0; i < ITERATIONS; ++i) {
            shared_counter++;  // Race condition here!
        }
    }
    
    static void demonstrate_race_condition() {
        cout << "\n=== RACE CONDITION DEMONSTRATION ===" << endl;
        shared_counter = 0;
        
        // Create two threads that increment the same variable
        thread t1(unsafe_increment);
        thread t2(unsafe_increment);
        
        t1.join();
        t2.join();
        
        cout << "Expected result: " << (2 * ITERATIONS) << endl;
        cout << "Actual result: " << shared_counter << endl;
        cout << "Difference: " << (2 * ITERATIONS - shared_counter) << endl;
        
        if (shared_counter != 2 * ITERATIONS) {
            cout << "RACE CONDITION DETECTED!" << endl;
        }
    }
};

int RaceConditionDemo::shared_counter = 0;

//=============================================================================
// 2. PETERSON'S SOLUTION (Section 6.3)
//=============================================================================

class PetersonSolution {
private:
    static bool flag[2];
    static int turn;
    static int shared_data;
    static const int ITERATIONS = 1000;
    
    static void process(int process_id) {
        int other = 1 - process_id;
        
        for (int i = 0; i < ITERATIONS; ++i) {
            // Entry section
            flag[process_id] = true;
            turn = other;
            while (flag[other] && turn == other) {
                // Busy wait
            }
            
            // Critical section
            int temp = shared_data;
            this_thread::sleep_for(microseconds(1)); // Simulate work
            shared_data = temp + 1;
            
            // Exit section
            flag[process_id] = false;
            
            // Remainder section
            this_thread::sleep_for(microseconds(1));
        }
    }
    
public:
    static void demonstrate_peterson() {
        cout << "\n=== PETERSON'S SOLUTION DEMONSTRATION ===" << endl;
        flag[0] = flag[1] = false;
        turn = 0;
        shared_data = 0;
        
        thread t1(process, 0);
        thread t2(process, 1);
        
        t1.join();
        t2.join();
        
        cout << "Expected result: " << (2 * ITERATIONS) << endl;
        cout << "Peterson's solution result: " << shared_data << endl;
        cout << "Peterson's solution: " << (shared_data == 2 * ITERATIONS ? "SUCCESS" : "FAILED") << endl;
    }
};

bool PetersonSolution::flag[2] = {false, false};
int PetersonSolution::turn = 0;
int PetersonSolution::shared_data = 0;

//=============================================================================
// 3. HARDWARE INSTRUCTIONS (Section 6.4)
//=============================================================================

class HardwareInstructions {
private:
    static atomic<bool> lock_var;
    static int shared_counter;
    static const int ITERATIONS = 100000;
    
    // Simulate test_and_set instruction
    static bool test_and_set(atomic<bool>& target) {
        return target.exchange(true);
    }
    
    // Simulate compare_and_swap instruction
    static bool compare_and_swap(atomic<int>& value, int expected, int new_value) {
        return value.compare_exchange_strong(expected, new_value);
    }
    
    static void safe_increment_tas() {
        for (int i = 0; i < ITERATIONS; ++i) {
            // Acquire lock using test_and_set
            while (test_and_set(lock_var)) {
                // Busy wait
            }
            
            // Critical section
            shared_counter++;
            
            // Release lock
            lock_var.store(false);
        }
    }
    
public:
    static void demonstrate_test_and_set() {
        cout << "\n=== TEST AND SET DEMONSTRATION ===" << endl;
        lock_var.store(false);
        shared_counter = 0;
        
        thread t1(safe_increment_tas);
        thread t2(safe_increment_tas);
        
        t1.join();
        t2.join();
        
        cout << "Expected result: " << (2 * ITERATIONS) << endl;
        cout << "Test-and-Set result: " << shared_counter << endl;
        cout << "Test-and-Set: " << (shared_counter == 2 * ITERATIONS ? "SUCCESS" : "FAILED") << endl;
    }
    
    static void demonstrate_compare_and_swap() {
        cout << "\n=== COMPARE AND SWAP DEMONSTRATION ===" << endl;
        atomic<int> cas_counter{0};
        
        auto cas_increment = [&cas_counter]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                int old_val, new_val;
                do {
                    old_val = cas_counter.load();
                    new_val = old_val + 1;
                } while (!cas_counter.compare_exchange_weak(old_val, new_val));
            }
        };
        
        thread t1(cas_increment);
        thread t2(cas_increment);
        
        t1.join();
        t2.join();
        
        cout << "Expected result: " << (2 * ITERATIONS) << endl;
        cout << "Compare-and-Swap result: " << cas_counter.load() << endl;
        cout << "Compare-and-Swap: " << (cas_counter.load() == 2 * ITERATIONS ? "SUCCESS" : "FAILED") << endl;
    }
};

atomic<bool> HardwareInstructions::lock_var{false};
int HardwareInstructions::shared_counter = 0;

//=============================================================================
// 4. MUTEX LOCKS (Section 6.5)
//=============================================================================

class MutexDemo {
private:
    static mutex mtx;
    static int shared_counter;
    static const int ITERATIONS = 100000;
    
    static void safe_increment() {
        for (int i = 0; i < ITERATIONS; ++i) {
            mtx.lock();      // Acquire lock
            shared_counter++;  // Critical section
            mtx.unlock();    // Release lock
        }
    }
    
public:
    static void demonstrate_mutex() {
        cout << "\n=== MUTEX LOCK DEMONSTRATION ===" << endl;
        shared_counter = 0;
        
        thread t1(safe_increment);
        thread t2(safe_increment);
        
        t1.join();
        t2.join();
        
        cout << "Expected result: " << (2 * ITERATIONS) << endl;
        cout << "Mutex result: " << shared_counter << endl;
        cout << "Mutex: " << (shared_counter == 2 * ITERATIONS ? "SUCCESS" : "FAILED") << endl;
    }
};

mutex MutexDemo::mtx;
int MutexDemo::shared_counter = 0;

//=============================================================================
// 5. SEMAPHORE IMPLEMENTATION (Section 6.6) - Using Custom Semaphore
//=============================================================================

class SemaphoreDemo {
private:
    static Semaphore resource_semaphore;
    
    static void process_task(int process_id) {
        cout << "Process " << process_id << " trying to acquire resource..." << endl;
        
        resource_semaphore.acquire();  // P() operation
        cout << "Process " << process_id << " acquired resource!" << endl;
        
        // Simulate work
        this_thread::sleep_for(seconds(2));
        
        cout << "Process " << process_id << " releasing resource..." << endl;
        resource_semaphore.release();  // V() operation
    }
    
public:
    static void demonstrate_semaphore() {
        cout << "\n=== SEMAPHORE DEMONSTRATION ===" << endl;
        cout << "Managing 3 resources with 5 processes" << endl;
        
        vector<thread> processes;
        
        for (int i = 0; i < 5; ++i) {
            processes.emplace_back(process_task, i);
        }
        
        for (auto& t : processes) {
            t.join();
        }
        
        cout << "All processes completed!" << endl;
    }
};

Semaphore SemaphoreDemo::resource_semaphore{3}; // 3 resources available

//=============================================================================
// 6. PRODUCER-CONSUMER PROBLEM (Sections 6.1, 6.6)
//=============================================================================

class ProducerConsumer {
private:
    static const int BUFFER_SIZE = 10;
    static int buffer[BUFFER_SIZE];
    static int in, out, count;
    static mutex buffer_mutex;
    static condition_variable not_empty, not_full;
    static bool done;
    
public:
    static void producer(int producer_id) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 100);
        
        for (int i = 0; i < 5; ++i) {
            int item = dis(gen);
            
            unique_lock<mutex> lock(buffer_mutex);
            not_full.wait(lock, []{ return count < BUFFER_SIZE; });
            
            // Critical section
            buffer[in] = item;
            in = (in + 1) % BUFFER_SIZE;
            count++;
            
            cout << "Producer " << producer_id << " produced: " << item << endl;
            
            not_empty.notify_one();
            lock.unlock();
            
            this_thread::sleep_for(milliseconds(100));
        }
    }
    
    static void consumer(int consumer_id) {
        for (int i = 0; i < 5; ++i) {
            unique_lock<mutex> lock(buffer_mutex);
            not_empty.wait(lock, []{ return count > 0 || done; });
            
            if (count > 0) {
                // Critical section
                int item = buffer[out];
                out = (out + 1) % BUFFER_SIZE;
                count--;
                
                cout << "Consumer " << consumer_id << " consumed: " << item << endl;
                
                not_full.notify_one();
            }
            
            lock.unlock();
            
            this_thread::sleep_for(milliseconds(150));
        }
    }
    
    static void demonstrate_producer_consumer() {
        cout << "\n=== PRODUCER-CONSUMER DEMONSTRATION ===" << endl;
        
        in = out = count = 0;
        done = false;
        
        vector<thread> threads;
        
        // Create 2 producers and 2 consumers
        threads.emplace_back(producer, 1);
        threads.emplace_back(producer, 2);
        threads.emplace_back(consumer, 1);
        threads.emplace_back(consumer, 2);
        
        for (auto& t : threads) {
            t.join();
        }
        
        done = true;
        not_empty.notify_all();
        
        cout << "Producer-Consumer demonstration completed!" << endl;
    }
};

int ProducerConsumer::buffer[ProducerConsumer::BUFFER_SIZE];
int ProducerConsumer::in = 0;
int ProducerConsumer::out = 0;
int ProducerConsumer::count = 0;
mutex ProducerConsumer::buffer_mutex;
condition_variable ProducerConsumer::not_empty;
condition_variable ProducerConsumer::not_full;
bool ProducerConsumer::done = false;

//=============================================================================
// 7. MONITOR IMPLEMENTATION (Section 6.7)
//=============================================================================

class Monitor {
private:
    mutable mutex monitor_mutex;
    condition_variable condition_x;
    int x_count = 0;
    
public:
    void wait_x() {
        unique_lock<mutex> lock(monitor_mutex);
        x_count++;
        condition_x.wait(lock);
        x_count--;
    }
    
    void signal_x() {
        unique_lock<mutex> lock(monitor_mutex);
        if (x_count > 0) {
            condition_x.notify_one();
        }
    }
    
    template<typename Func>
    auto execute(Func&& func) -> decltype(func()) {
        unique_lock<mutex> lock(monitor_mutex);
        return func();
    }
};

class ResourceAllocator {
private:
    Monitor monitor;
    bool busy = false;
    condition_variable resource_available;
    
public:
    void acquire(int time) {
        monitor.execute([&]() {
            while (busy) {
                monitor.wait_x();
            }
            busy = true;
            cout << "Resource acquired for " << time << " seconds" << endl;
        });
    }
    
    void release() {
        monitor.execute([&]() {
            busy = false;
            monitor.signal_x();
            cout << "Resource released" << endl;
        });
    }
    
    static void demonstrate_monitor() {
        cout << "\n=== MONITOR DEMONSTRATION ===" << endl;
        
        ResourceAllocator allocator;
        
        auto process = [&allocator](int id, int duration) {
            cout << "Process " << id << " requesting resource..." << endl;
            allocator.acquire(duration);
            
            this_thread::sleep_for(seconds(duration));
            
            allocator.release();
            cout << "Process " << id << " finished" << endl;
        };
        
        vector<thread> processes;
        processes.emplace_back(process, 1, 2);
        processes.emplace_back(process, 2, 1);
        processes.emplace_back(process, 3, 3);
        
        for (auto& t : processes) {
            t.join();
        }
        
        cout << "Monitor demonstration completed!" << endl;
    }
};

//=============================================================================
// 8. DINING PHILOSOPHERS PROBLEM (Classic Synchronization Problem)
//=============================================================================

class DiningPhilosophers {
private:
    static const int NUM_PHILOSOPHERS = 5;
    static mutex chopsticks[NUM_PHILOSOPHERS];
    
    static void philosopher(int id) {
        for (int i = 0; i < 3; ++i) { // Each philosopher eats 3 times
            // Think
            cout << "Philosopher " << id << " is thinking..." << endl;
            this_thread::sleep_for(milliseconds(1000 + (id * 100)));
            
            // Pick up chopsticks (avoid deadlock by ordering)
            int left = id;
            int right = (id + 1) % NUM_PHILOSOPHERS;
            
            // Always pick up lower numbered chopstick first
            if (left > right) swap(left, right);
            
            chopsticks[left].lock();
            cout << "Philosopher " << id << " picked up left chopstick" << endl;
            
            chopsticks[right].lock();
            cout << "Philosopher " << id << " picked up right chopstick" << endl;
            
            // Eat
            cout << "Philosopher " << id << " is EATING" << endl;
            this_thread::sleep_for(milliseconds(500));
            
            // Put down chopsticks
            chopsticks[right].unlock();
            chopsticks[left].unlock();
            
            cout << "Philosopher " << id << " finished eating" << endl;
        }
    }
    
public:
    static void demonstrate_dining_philosophers() {
        cout << "\n=== DINING PHILOSOPHERS DEMONSTRATION ===" << endl;
        
        vector<thread> philosophers;
        
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            philosophers.emplace_back(philosopher, i);
        }
        
        for (auto& t : philosophers) {
            t.join();
        }
        
        cout << "All philosophers finished dining!" << endl;
    }
};

mutex DiningPhilosophers::chopsticks[DiningPhilosophers::NUM_PHILOSOPHERS];

//=============================================================================
// MAIN FUNCTION - RUN ALL DEMONSTRATIONS
//=============================================================================

int main() {
    cout << "CHAPTER 6: SYNCHRONIZATION TOOLS - C++17 IMPLEMENTATION" << endl;
    cout << "========================================================" << endl;
    
    try {
        // 1. Race Condition Demonstration
        RaceConditionDemo::demonstrate_race_condition();
        
        // 2. Peterson's Solution
        PetersonSolution::demonstrate_peterson();
        
        // 3. Hardware Instructions
        HardwareInstructions::demonstrate_test_and_set();
        HardwareInstructions::demonstrate_compare_and_swap();
        
        // 4. Mutex Locks
        MutexDemo::demonstrate_mutex();
        
        // 5. Semaphores
        SemaphoreDemo::demonstrate_semaphore();
        
        // 6. Producer-Consumer Problem
        ProducerConsumer::demonstrate_producer_consumer();
        
        // 7. Monitor
        ResourceAllocator::demonstrate_monitor();
        
        // 8. Dining Philosophers
        DiningPhilosophers::demonstrate_dining_philosophers();
        
        cout << "\n=== ALL DEMONSTRATIONS COMPLETED ===" << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}

/*
 * COMPILATION INSTRUCTIONS:
 * 
 * For C++17 (this version):
 * g++ -std=c++17 -pthread synchronization_tools.cpp -o synchronization_tools
 * 
 * For C++20 (if available):
 * g++ -std=c++20 -pthread synchronization_tools.cpp -o synchronization_tools
 * 
 * LEARNING OBJECTIVES:
 * After studying this code, students should understand:
 * 1. How race conditions occur and their consequences
 * 2. Peterson's algorithm for mutual exclusion
 * 3. Hardware-based synchronization primitives
 * 4. Mutex locks and their proper usage
 * 5. Semaphore operations and resource management (with custom implementation)
 * 6. Monitor concept and implementation
 * 7. Classic synchronization problems and solutions
 */
```
```

---

## Part 1: Conceptual Understanding

### 1.1 Critical Section Problem
**Study the race condition demonstration in the code, then answer:**

**Q1:** What are the three requirements any solution to the critical-section problem must satisfy?

**Answer:** 
1. **Mutual Exclusion** - Only one process can execute in critical section at a time
2. **Progress** - Selection of next process cannot be postponed indefinitely  
3. **Bounded Waiting** - Limit on number of times other processes can enter critical section

**Q2:** Explain why the race condition occurs in `RaceConditionDemo::unsafe_increment()`.

**Answer:** The increment operation `shared_counter++` is not atomic. It consists of three steps:
1. Load value from memory to register
2. Increment the value in register  
3. Store value back to memory

When two threads execute concurrently, their operations can interleave, causing lost updates.

**Q3:** Run the race condition demo multiple times. Do you always get the same result? Why or why not?

**Your Answer:** ________________________________

---

### 1.2 Peterson's Solution Analysis

**Study `PetersonSolution` class and answer:**

**Q4:** Peterson's solution uses two shared variables: `flag[]` and `turn`. Explain the purpose of each.

**Answer:**
- `flag[i]` - Indicates process i wants to enter critical section
- `turn` - Indicates whose turn it is when both processes want to enter

**Q5:** Trace through Peterson's algorithm when both processes want to enter critical section simultaneously:

```
Process 0:           Process 1:
flag[0] = true      flag[1] = true  
turn = 1            turn = 0
while(flag[1] && turn==1)  while(flag[0] && turn==0)
```

**Who enters first and why?**

**Your Answer:** ________________________________

**Programming Exercise 1:**
Modify Peterson's solution to work with 3 processes. What additional variables do you need?

```cpp
class ThreeProcessPeterson {
    // Your implementation here
};
```

---

### 1.3 Hardware Instructions

**Q6:** What advantage do hardware instructions like `test_and_set()` have over software solutions like Peterson's algorithm?

**Your Answer:** ________________________________

**Q7:** Compare `test_and_set()` vs `compare_and_swap()`. Which is more flexible and why?

**Answer:** `compare_and_swap()` is more flexible because:
- It can implement lock-free data structures
- Allows conditional updates based on expected values
- Can implement counters, stacks, queues without traditional locks
- `test_and_set()` only works with boolean values

**Programming Exercise 2:**
Implement a simple spinlock using `compare_and_swap()`:

```cpp
class SpinLock {
private:
    atomic<int> lock_value{0};
    
public:
    void acquire() {
        // Your implementation
        while(true) {
            int expected = 0;
            if(lock_value.compare_exchange_weak(expected, 1)) {
                break; // Successfully acquired lock
            }
            // Spin (busy wait)
        }
    }
    
    void release() {
        // Your implementation
        lock_value.store(0);
    }
};
```

---

## Part 2: Mutex and Semaphore Programming

### 2.1 Mutex Lock Exercises

**Q8:** What is the main disadvantage of the mutex implementation shown in class?

**Answer:** Busy waiting (spinning) - wastes CPU cycles while waiting for lock

**Programming Exercise 3:**
Fix the race condition in this banking system:

```cpp
class BankAccount {
private:
    double balance;
    // Add synchronization here
    
public:
    BankAccount(double initial) : balance(initial) {}
    
    void withdraw(double amount) {
        // Critical section - fix race condition
        if (balance >= amount) {
            // Simulate processing delay
            this_thread::sleep_for(milliseconds(1));
            balance -= amount;
            cout << "Withdrew: " << amount << ", Balance: " << balance << endl;
        }
    }
    
    void deposit(double amount) {
        // Critical section - fix race condition  
        balance += amount;
        cout << "Deposited: " << amount << ", Balance: " << balance << endl;
    }
    
    double get_balance() const {
        // Need synchronization here too?
        return balance;
    }
};
```

**Your Solution:**
```cpp
// Add your fixed version here
class SafeBankAccount {
private:
    double balance;
    mutable mutex balance_mutex;  // mutable for const methods
    
public:
    SafeBankAccount(double initial) : balance(initial) {}
    
    void withdraw(double amount) {
        lock_guard<mutex> lock(balance_mutex);
        if (balance >= amount) {
            this_thread::sleep_for(milliseconds(1));
            balance -= amount;
            cout << "Withdrew: " << amount << ", Balance: " << balance << endl;
        }
    }
    
    void deposit(double amount) {
        lock_guard<mutex> lock(balance_mutex);
        balance += amount;
        cout << "Deposited: " << amount << ", Balance: " << balance << endl;
    }
    
    double get_balance() const {
        lock_guard<mutex> lock(balance_mutex);
        return balance;
    }
};
```

### 2.2 Semaphore Applications

**Q9:** What's the difference between a binary semaphore and a counting semaphore?

**Answer:** 
- **Binary semaphore:** Can only have values 0 or 1 (like a mutex)
- **Counting semaphore:** Can have any non-negative integer value (manages multiple resources)

**Programming Exercise 4:**
Implement a thread-safe bounded queue using our custom semaphore:

```cpp
template<typename T, size_t SIZE>
class BoundedQueue {
private:
    T buffer[SIZE];
    size_t head, tail;
    Semaphore empty_slots{SIZE};  // Initially all empty
    Semaphore filled_slots{0};    // Initially none filled  
    mutex queue_mutex;
    
public:
    void enqueue(const T& item) {
        empty_slots.acquire();          // Wait for empty slot
        
        lock_guard<mutex> lock(queue_mutex);
        buffer[tail] = item;
        tail = (tail + 1) % SIZE;
        
        filled_slots.release();         // Signal new item available
    }
    
    T dequeue() {
        filled_slots.acquire();         // Wait for item
        
        lock_guard<mutex> lock(queue_mutex);
        T item = buffer[head];
        head = (head + 1) % SIZE;
        
        empty_slots.release();          // Signal empty slot available
        return item;
    }
};
```

---

## Part 3: Producer-Consumer Problem

### 3.1 Analysis Questions

**Q10:** In the Producer-Consumer implementation, why do we need both a mutex AND condition variables?

**Answer:** 
- **Mutex:** Protects shared buffer data structure from race conditions
- **Condition Variables:** Allow threads to efficiently wait for conditions (buffer not full/empty) without busy waiting

**Q11:** What happens if we remove the `not_full.wait()` call from the producer?

**Your Answer:** ________________________________

**Programming Exercise 5:**
Implement Producer-Consumer using our custom semaphore instead of condition variables:

```cpp
class ProducerConsumerSemaphore {
private:
    static const int BUFFER_SIZE = 5;
    int buffer[BUFFER_SIZE];
    int in, out;
    
    // Add semaphores here
    Semaphore empty{BUFFER_SIZE};  // Custom semaphore class
    Semaphore full{0};
    mutex buffer_mutex;
    
public:
    void producer(int id) {
        for(int i = 0; i < 3; ++i) {
            int item = id * 10 + i;
            
            // Your implementation
            empty.acquire();
            {
                lock_guard<mutex> lock(buffer_mutex);
                buffer[in] = item;
                in = (in + 1) % BUFFER_SIZE;
                cout << "Producer " << id << " produced: " << item << endl;
            }
            full.release();
        }
    }
    
    void consumer(int id) {
        for(int i = 0; i < 3; ++i) {
            // Your implementation  
            full.acquire();
            int item;
            {
                lock_guard<mutex> lock(buffer_mutex);
                item = buffer[out];
                out = (out + 1) % BUFFER_SIZE;
            }
            empty.release();
            
            cout << "Consumer " << id << " consumed: " << item << endl;
        }
    }
};
```

**Note:** Our custom `Semaphore` class provides the same interface as C++20's `counting_semaphore` but works with C++17. The key methods are:
- `acquire()` - Decrements counter, blocks if zero (P operation)
- `release()` - Increments counter, wakes waiting thread (V operation)
- `try_acquire()` - Non-blocking version, returns true if successful

---

## Part 4: Advanced Synchronization

### 4.1 Dining Philosophers Analysis

**Q12:** What causes deadlock in the naive dining philosophers solution?

**Answer:** Circular waiting - if all philosophers pick up their left chopstick simultaneously, they'll wait forever for the right chopstick.

**Q13:** The provided solution uses "ordered resource allocation." Explain how this prevents deadlock.

**Your Answer:** ________________________________

**Programming Exercise 6:**
Implement an alternative solution using a semaphore to limit philosophers:

```cpp
class DiningPhilosophersWithSemaphore {
private:
    static const int NUM_PHILOSOPHERS = 5;
    mutex chopsticks[NUM_PHILOSOPHERS];
    counting_semaphore<4> dining_room{4}; // Only 4 can try to eat at once
    
    static void philosopher(int id) {
        for(int meal = 0; meal < 3; ++meal) {
            think(id);
            
            dining_room.acquire(); // Enter dining room
            
            // Pick up chopsticks
            chopsticks[id].lock();
            chopsticks[(id + 1) % NUM_PHILOSOPHERS].lock();
            
            eat(id);
            
            // Put down chopsticks
            chopsticks[(id + 1) % NUM_PHILOSOPHERS].unlock();
            chopsticks[id].unlock();
            
            dining_room.release(); // Leave dining room
        }
    }
    
    static void think(int id) {
        cout << "Philosopher " << id << " thinking" << endl;
        this_thread::sleep_for(milliseconds(500));
    }
    
    static void eat(int id) {
        cout << "Philosopher " << id << " eating" << endl;
        this_thread::sleep_for(milliseconds(300));
    }
};
```

---

## Part 5: Monitor Implementation

### 5.1 Monitor Exercises

**Q14:** What are the key differences between monitors and semaphores?

**Answer:**
- **Monitors:** Higher-level abstraction, automatic mutual exclusion, condition variables for signaling
- **Semaphores:** Lower-level, manual synchronization, counting mechanism

**Programming Exercise 7:**
Implement a Reader-Writer lock using monitors:

```cpp
class ReaderWriterMonitor {
private:
    mutable mutex monitor_lock;
    condition_variable readers_ok, writers_ok;
    int readers = 0;
    int writers = 0;
    int waiting_writers = 0;
    
public:
    void start_read() {
        unique_lock<mutex> lock(monitor_lock);
        while(writers > 0 || waiting_writers > 0) {
            readers_ok.wait(lock);
        }
        readers++;
    }
    
    void end_read() {
        unique_lock<mutex> lock(monitor_lock);
        readers--;
        if(readers == 0) {
            writers_ok.notify_one();
        }
    }
    
    void start_write() {
        unique_lock<mutex> lock(monitor_lock);
        waiting_writers++;
        while(readers > 0 || writers > 0) {
            writers_ok.wait(lock);
        }
        waiting_writers--;
        writers++;
    }
    
    void end_write() {
        unique_lock<mutex> lock(monitor_lock);
        writers--;
        readers_ok.notify_all();
        writers_ok.notify_one();
    }
};
```

---

## Part 6: Practice Problems

### Problem 1: Parking Lot Simulation
Design a parking lot system with the following requirements:
- 10 parking spaces
- Cars arrive and park for random duration
- Display which spaces are occupied
- Handle concurrent access safely

```cpp
class ParkingLot {
    // Your implementation here
};
```

### Problem 2: Barber Shop Problem
Implement the classical barber shop problem:
- One barber, multiple chairs in waiting room
- Customers arrive randomly
- If no chairs available, customer leaves
- Barber sleeps when no customers

```cpp
class BarberShop {
    // Your implementation here  
};
```

### Problem 3: Bridge Crossing Problem
One-lane bridge that cars can cross in either direction:
- Only cars going same direction can be on bridge
- Prevent starvation of cars going opposite direction
- Maximum 3 cars on bridge at once

```cpp
class Bridge {
    // Your implementation here
};
```

---

## Part 7: Performance Analysis

### 7.1 Benchmarking Exercise

**Programming Exercise 8:**
Create a program that compares performance of different synchronization methods:

```cpp
class SynchronizationBenchmark {
public:
    static void benchmark_mutex() {
        // Measure time for 1000000 lock/unlock operations
        auto start = high_resolution_clock::now();
        // Your implementation
        auto end = high_resolution_clock::now();
        cout << "Mutex time: " << duration_cast<microseconds>(end-start).count() << "μs" << endl;
    }
    
    static void benchmark_atomic() {
        // Compare with atomic operations
        // Your implementation
    }
    
    static void benchmark_spinlock() {
        // Compare with spinlock
        // Your implementation  
    }
};
```

### 7.2 Analysis Questions

**Q15:** Under what conditions would you choose:
- Spinlock vs Mutex?
- Semaphore vs Condition Variable?
- Monitor vs Manual synchronization?

**Your Analysis:** ________________________________

---

## Part 8: Debugging Exercises

### 8.1 Find the Bugs

**Exercise 1:** What's wrong with this code?
```cpp
void buggy_function() {
    static mutex mtx;
    mtx.lock();
    if (some_condition()) {
        return; // Bug!
    }
    // do work
    mtx.unlock();
}
```

**Answer:** The mutex is not unlocked if `some_condition()` is true, causing deadlock. Use RAII with `lock_guard`.

**Exercise 2:** Identify the race condition:
```cpp
class Counter {
    atomic<int> count{0};
public:
    void increment() {
        int old = count.load();
        count.store(old + 1); // Bug!
    }
};
```

**Answer:** Between load and store, another thread could modify count. Use `count++` or `compare_exchange`.

---

## Self-Assessment Checklist

After completing this worksheet, you should be able to:

- [ ] Explain the critical section problem and its requirements
- [ ] Implement Peterson's solution and understand its limitations  
- [ ] Use hardware instructions for synchronization
- [ ] Apply mutex locks correctly with RAII
- [ ] Design solutions using semaphores
- [ ] Solve producer-consumer problems
- [ ] Implement monitors and condition variables
- [ ] Analyze and prevent deadlock conditions
- [ ] Debug common synchronization errors
- [ ] Choose appropriate synchronization primitives for different scenarios

## Additional Resources

1. **Practice Compilation:**
   ```bash
   g++ -std=c++17 -pthread -Wall -Wextra your_solution.cpp -o your_program
   ```

2. **Debugging Tips:**
   - Use Thread Sanitizer: `g++ -std=c++17 -pthread -fsanitize=thread your_solution.cpp`
   - Use Helgrind (Valgrind): `valgrind --tool=helgrind ./program`
   - Add logging to trace execution order

3. **Key Differences from C++20 Version:**
   - **Custom Semaphore**: We implement our own semaphore using `mutex` and `condition_variable`
   - **C++17 Compatible**: No dependency on `std::counting_semaphore` or other C++20 features
   - **Same Functionality**: All synchronization concepts work identically

---

## Answer Key (Selected Answers)

**Q3:** Results vary due to non-deterministic thread scheduling and race conditions.

**Q11:** Producer will write beyond buffer bounds, causing memory corruption when buffer is full.

**Q13:** By ordering chopstick acquisition (lower number first), we eliminate circular waiting in the resource allocation graph.

Remember: The key to mastering synchronization is practice and understanding the underlying principles, not just memorizing solutions!