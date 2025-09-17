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
