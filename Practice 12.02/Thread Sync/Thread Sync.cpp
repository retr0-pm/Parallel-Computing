#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>

using namespace std;
using namespace chrono;

mutex mtx;

int safe_sum = 0;
int unsafe_sum = 0;
atomic<int> atomic_sum(0);

void safe_add(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        lock_guard<mutex> lock(mtx);
        safe_sum += id;
    }
}

void unsafe_add(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        unsafe_sum += id;
    }
}

void atomic_add(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        atomic_sum += id;
    }
}

int main() {
    const int num_threads = 5;
    const int num_iterations = 100000;
    const int expected_result = (num_threads * (num_threads + 1) / 2) * num_iterations;

    // UNSAFE
    vector<thread> threads;
    auto start_unsafe = high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(unsafe_add, i + 1, num_iterations);
    for (auto& th : threads)
        th.join();

    auto end_unsafe = high_resolution_clock::now();
    auto duration_unsafe = duration_cast<milliseconds>(end_unsafe - start_unsafe);

    // SAFE (with mutex)
    threads.clear();
    auto start_safe = high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(safe_add, i + 1, num_iterations);
    for (auto& th : threads)
        th.join();

    auto end_safe = high_resolution_clock::now();
    auto duration_safe = duration_cast<milliseconds>(end_safe - start_safe);

    // ATOMIC
    threads.clear();
    auto start_atomic = high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(atomic_add, i + 1, num_iterations);
    for (auto& th : threads)
        th.join();

    auto end_atomic = high_resolution_clock::now();
    auto duration_atomic = duration_cast<milliseconds>(end_atomic - start_atomic);

    // RESULTS
    cout << "Expected result: " << expected_result << endl;
    cout << "Final UNSAFE sum: " << unsafe_sum << " (may be incorrect)\n";
    cout << "Final SAFE (mutex) sum: " << safe_sum << " (must be correct)\n";
    cout << "Final ATOMIC sum: " << atomic_sum.load() << " (must be correct)\n\n";

    cout << "Execution times:\n";
    cout << "UNSAFE: " << duration_unsafe.count() << " ms\n";
    cout << "SAFE (mutex): " << duration_safe.count() << " ms\n";
    cout << "ATOMIC: " << duration_atomic.count() << " ms\n";

    return 0;
}
