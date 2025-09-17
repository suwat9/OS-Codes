# Chapter 6: Synchronization Tools - Student Study Worksheet

## Overview
This worksheet contains the complete C++ implementation of Chapter 6 concepts plus exercises to help you master process synchronization through hands-on practice, analysis problems, and programming challenges.

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
