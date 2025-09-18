Sample code to test race condition in multiple threads:
```cpp
#include <iostream>
#include <thread>

int counter = 0;

void increment(const char* thread_name) {
    for (int i = 0; i < 100000; ++i) {
            ++counter; // No synchronization: race condition!
                }
                    std::cout << thread_name << " finished\n";
                    }

                    int main() {
                        std::thread t1(increment, "Thread 1");
                            std::thread t2(increment, "Thread 2");

                                t1.join();
                                    t2.join();

                                        std::cout << "Final counter value: " << counter << std::endl;
                                            return 0;
                                            }
```
