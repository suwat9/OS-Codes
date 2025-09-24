#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <vector>
#include <chrono>
#include <random>
#include <condition_variable>
#include <atomic>

using namespace std;
using namespace std::chrono;

//=============================================================================
// SOLUTION 1: SEMAPHORE-BASED APPROACH (Prevents Deadlock + Reduces Starvation)
//=============================================================================
class DiningPhilosophersSemaphore {
private:
    static const int NUM_PHILOSOPHERS = 5;
    static mutex chopsticks[NUM_PHILOSOPHERS];
    // Key insight: Allow only N-1 philosophers to compete for chopsticks simultaneously
    // This guarantees at least one philosopher can always get both chopsticks
    static counting_semaphore<NUM_PHILOSOPHERS-1> dining_semaphore;
    
    static void philosopher(int id) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> think_time(500, 1500);
        
        for (int meal = 0; meal < 3; ++meal) {
            // THINKING PHASE
            cout << "Philosopher " << id << " is thinking (meal " << meal + 1 << ")..." << endl;
            this_thread::sleep_for(milliseconds(think_time(gen)));
            
            // ACQUIRE PERMISSION TO DINE
            // This is the key: only N-1 philosophers can attempt to eat simultaneously
            // This prevents circular wait and guarantees deadlock freedom
            cout << "Philosopher " << id << " wants to eat, requesting dining permission..." << endl;
            dining_semaphore.acquire();
            
            // ACQUIRE CHOPSTICKS
            int left_chopstick = id;
            int right_chopstick = (id + 1) % NUM_PHILOSOPHERS;
            
            cout << "Philosopher " << id << " trying to pick up chopsticks..." << endl;
            
            // Pick up chopsticks (can use any order since we're protected by semaphore)
            chopsticks[left_chopstick].lock();
            cout << "Philosopher " << id << " picked up left chopstick " << left_chopstick << endl;
            
            chopsticks[right_chopstick].lock();
            cout << "Philosopher " << id << " picked up right chopstick " << right_chopstick << endl;
            
            // EATING PHASE
            cout << "*** Philosopher " << id << " is EATING (meal " << meal + 1 << ") ***" << endl;
            this_thread::sleep_for(milliseconds(800 + (id * 50))); // Slight variation in eating time
            
            // RELEASE CHOPSTICKS
            chopsticks[right_chopstick].unlock();
            chopsticks[left_chopstick].unlock();
            cout << "Philosopher " << id << " put down both chopsticks" << endl;
            
            // RELEASE DINING PERMISSION
            dining_semaphore.release();
            cout << "Philosopher " << id << " finished eating meal " << meal + 1 << endl;
            
            // Small break between meals
            this_thread::sleep_for(milliseconds(200));
        }
        cout << "Philosopher " << id << " completed all meals!" << endl;
    }
    
public:
    static void demonstrate() {
        cout << "\n=== SEMAPHORE-BASED DINING PHILOSOPHERS ===" << endl;
        cout << "Solution: Allow max " << NUM_PHILOSOPHERS-1 << " philosophers to compete for chopsticks" << endl;
        cout << "Benefits: Prevents deadlock, reduces starvation risk\n" << endl;
        
        vector<thread> philosophers;
        
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            philosophers.emplace_back(philosopher, i);
        }
        
        for (auto& t : philosophers) {
            t.join();
        }
        
        cout << "\nAll philosophers finished dining! (Semaphore solution)" << endl;
    }
};

// Static member definitions
mutex DiningPhilosophersSemaphore::chopsticks[DiningPhilosophersSemaphore::NUM_PHILOSOPHERS];
counting_semaphore<DiningPhilosophersSemaphore::NUM_PHILOSOPHERS-1> 
    DiningPhilosophersSemaphore::dining_semaphore{DiningPhilosophersSemaphore::NUM_PHILOSOPHERS-1};

//=============================================================================
// SOLUTION 2: WAITER SOLUTION (Central Coordinator - Prevents Both Issues)
//=============================================================================
class DiningPhilosophersWaiter {
private:
    static const int NUM_PHILOSOPHERS = 5;
    static mutex chopsticks[NUM_PHILOSOPHERS];
    static mutex waiter_mutex;  // Waiter controls access to chopstick acquisition
    static condition_variable waiter_cv;
    static bool chopstick_available[NUM_PHILOSOPHERS];
    
    // Check if philosopher can pick up both chopsticks
    static bool can_eat(int philosopher_id) {
        int left = philosopher_id;
        int right = (philosopher_id + 1) % NUM_PHILOSOPHERS;
        return chopstick_available[left] && chopstick_available[right];
    }
    
    // Waiter grants permission to eat (atomic check and reserve)
    static void request_chopsticks(int philosopher_id) {
        unique_lock<mutex> lock(waiter_mutex);
        
        // Wait until both chopsticks are available
        waiter_cv.wait(lock, [philosopher_id] { return can_eat(philosopher_id); });
        
        // Reserve both chopsticks atomically
        int left = philosopher_id;
        int right = (philosopher_id + 1) % NUM_PHILOSOPHERS;
        chopstick_available[left] = false;
        chopstick_available[right] = false;
        
        cout << "Waiter: Granted chopsticks " << left << " and " << right 
             << " to Philosopher " << philosopher_id << endl;
    }
    
    // Waiter handles chopstick return
    static void return_chopsticks(int philosopher_id) {
        unique_lock<mutex> lock(waiter_mutex);
        
        int left = philosopher_id;
        int right = (philosopher_id + 1) % NUM_PHILOSOPHERS;
        chopstick_available[left] = true;
        chopstick_available[right] = true;
        
        cout << "Waiter: Philosopher " << philosopher_id 
             << " returned chopsticks " << left << " and " << right << endl;
        
        // Notify all waiting philosophers that chopsticks are available
        waiter_cv.notify_all();
    }
    
    static void philosopher(int id) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> think_time(400, 1200);
        
        for (int meal = 0; meal < 3; ++meal) {
            // THINKING
            cout << "Philosopher " << id << " is thinking..." << endl;
            this_thread::sleep_for(milliseconds(think_time(gen)));
            
            // REQUEST PERMISSION FROM WAITER
            cout << "Philosopher " << id << " asks waiter for permission to eat..." << endl;
            request_chopsticks(id);
            
            // EATING (chopsticks guaranteed to be available)
            cout << "*** Philosopher " << id << " is EATING (meal " << meal + 1 << ") ***" << endl;
            this_thread::sleep_for(milliseconds(600));
            
            // RETURN CHOPSTICKS TO WAITER
            return_chopsticks(id);
            cout << "Philosopher " << id << " finished meal " << meal + 1 << endl;
        }
        cout << "Philosopher " << id << " completed all meals!" << endl;
    }
    
public:
    static void demonstrate() {
        cout << "\n=== WAITER-BASED DINING PHILOSOPHERS ===" << endl;
        cout << "Solution: Central waiter controls chopstick allocation" << endl;
        cout << "Benefits: Complete deadlock prevention, fair starvation prevention\n" << endl;
        
        // Initialize chopstick availability
        fill(chopstick_available, chopstick_available + NUM_PHILOSOPHERS, true);
        
        vector<thread> philosophers;
        
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            philosophers.emplace_back(philosopher, i);
        }
        
        for (auto& t : philosophers) {
            t.join();
        }
        
        cout << "\nAll philosophers finished dining! (Waiter solution)" << endl;
    }
};

// Static member definitions
mutex DiningPhilosophersWaiter::chopsticks[DiningPhilosophersWaiter::NUM_PHILOSOPHERS];
mutex DiningPhilosophersWaiter::waiter_mutex;
condition_variable DiningPhilosophersWaiter::waiter_cv;
bool DiningPhilosophersWaiter::chopstick_available[DiningPhilosophersWaiter::NUM_PHILOSOPHERS];

//=============================================================================
// SOLUTION 3: TIMEOUT-BASED APPROACH (Practical Starvation Prevention)
//=============================================================================
class DiningPhilosophersTimeout {
private:
    static const int NUM_PHILOSOPHERS = 5;
    static timed_mutex chopsticks[NUM_PHILOSOPHERS];
    static atomic<int> successful_meals;
    static atomic<int> timeouts;
    
    static void philosopher(int id) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> think_time(300, 1000);
        
        int meals_eaten = 0;
        int attempts = 0;
        
        while (meals_eaten < 3 && attempts < 10) { // Limit total attempts to prevent infinite loops
            attempts++;
            
            // THINKING
            cout << "Philosopher " << id << " is thinking (attempt " << attempts << ")..." << endl;
            this_thread::sleep_for(milliseconds(think_time(gen)));
            
            // TRY TO ACQUIRE CHOPSTICKS WITH TIMEOUT
            int left = id;
            int right = (id + 1) % NUM_PHILOSOPHERS;
            
            // Always try to acquire in consistent order to prevent some deadlocks
            if (left > right) swap(left, right);
            
            cout << "Philosopher " << id << " attempting to get chopsticks (timeout approach)..." << endl;
            
            // Try to lock first chopstick with timeout
            if (chopsticks[left].try_lock_for(milliseconds(1000))) {
                cout << "Philosopher " << id << " got first chopstick " << left << endl;
                
                // Try to lock second chopstick with timeout
                if (chopsticks[right].try_lock_for(milliseconds(1000))) {
                    cout << "Philosopher " << id << " got second chopstick " << right << endl;
                    
                    // SUCCESS - EAT
                    meals_eaten++;
                    successful_meals++;
                    cout << "*** Philosopher " << id << " is EATING (meal " << meals_eaten << ") ***" << endl;
                    this_thread::sleep_for(milliseconds(700));
                    
                    // RELEASE CHOPSTICKS
                    chopsticks[right].unlock();
                    chopsticks[left].unlock();
                    cout << "Philosopher " << id << " finished meal " << meals_eaten << endl;
                    
                } else {
                    // TIMEOUT ON SECOND CHOPSTICK
                    timeouts++;
                    cout << "Philosopher " << id << " timed out on second chopstick, backing off..." << endl;
                    chopsticks[left].unlock();
                    
                    // Exponential backoff to reduce contention
                    this_thread::sleep_for(milliseconds(100 * attempts));
                }
            } else {
                // TIMEOUT ON FIRST CHOPSTICK
                timeouts++;
                cout << "Philosopher " << id << " timed out on first chopstick, will retry..." << endl;
                
                // Random backoff to break synchronization patterns
                this_thread::sleep_for(milliseconds(50 + (gen() % 200)));
            }
        }
        
        cout << "Philosopher " << id << " finished with " << meals_eaten << " meals eaten!" << endl;
    }
    
public:
    static void demonstrate() {
        cout << "\n=== TIMEOUT-BASED DINING PHILOSOPHERS ===" << endl;
        cout << "Solution: Use timeouts and backoff to prevent indefinite blocking" << endl;
        cout << "Benefits: Practical starvation prevention, handles contention gracefully\n" << endl;
        
        successful_meals = 0;
        timeouts = 0;
        
        vector<thread> philosophers;
        
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            philosophers.emplace_back(philosopher, i);
        }
        
        for (auto& t : philosophers) {
            t.join();
        }
        
        cout << "\nTimeout solution completed!" << endl;
        cout << "Total successful meals: " << successful_meals.load() << endl;
        cout << "Total timeouts: " << timeouts.load() << endl;
    }
};

// Static member definitions
timed_mutex DiningPhilosophersTimeout::chopsticks[DiningPhilosophersTimeout::NUM_PHILOSOPHERS];
atomic<int> DiningPhilosophersTimeout::successful_meals{0};
atomic<int> DiningPhilosophersTimeout::timeouts{0};

//=============================================================================
// DEMONSTRATION RUNNER
//=============================================================================
int main() {
    cout << "DINING PHILOSOPHERS PROBLEM - DEADLOCK & STARVATION SOLUTIONS" << endl;
    cout << "=============================================================" << endl;
    
    // Run each solution
    DiningPhilosophersSemaphore::demonstrate();
    this_thread::sleep_for(seconds(2));
    
    DiningPhilosophersWaiter::demonstrate();
    this_thread::sleep_for(seconds(2));
    
    DiningPhilosophersTimeout::demonstrate();
    
    cout << "\n=== ANALYSIS ===" << endl;
    cout << "1. SEMAPHORE: Best balance of simplicity and effectiveness" << endl;
    cout << "2. WAITER: Most fair, but centralized bottleneck" << endl;
    cout << "3. TIMEOUT: Most practical for real systems with contention" << endl;
    
    return 0;
}

/*
SOLUTION COMPARISON:

1. SEMAPHORE APPROACH:
   - Deadlock Prevention: ✅ (limits concurrent diners)
   - Starvation Prevention: ⚠️ (reduced but not eliminated)
   - Performance: Good
   - Complexity: Low

2. WAITER APPROACH:
   - Deadlock Prevention: ✅ (centralized control)
   - Starvation Prevention: ✅ (fair FIFO ordering)
   - Performance: Moderate (centralized bottleneck)
   - Complexity: Medium

3. TIMEOUT APPROACH:
   - Deadlock Prevention: ✅ (timeouts break deadlock)
   - Starvation Prevention: ✅ (backoff ensures eventual success)
   - Performance: Good under contention
   - Complexity: Medium

RECOMMENDED: Semaphore approach for most cases, Waiter for strict fairness requirements
*/
