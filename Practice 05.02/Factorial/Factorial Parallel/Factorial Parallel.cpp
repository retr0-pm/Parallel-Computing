#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

using namespace std;
using namespace chrono;

mutex mtx;
long long global_result = 1;

void partial_factorial(long long start, long long end) {
    long long local_result = 1;
    for (long long i = start; i <= end; ++i) {
        local_result *= i;
    }

    lock_guard<mutex> lock(mtx);
    global_result *= local_result;
}

long long parallel_factorial(int n, int num_threads) {
    if (n < 0) {
        throw invalid_argument("Factorial is not defined for negative numbers.");
    }
    if (n == 0 || n == 1) {
        return 1;
    }
    if (num_threads < 1) {
        throw invalid_argument("Number of threads must be at least 1.");
    }

    global_result = 1;
    vector<thread> threads;

    int chunk_size = n / num_threads;
    int remainder = n % num_threads;

    int start = 1;
    for (int i = 0; i < num_threads; ++i) {
        int end = start + chunk_size - 1;
        if (i < remainder) {
            end++;
        }

        threads.emplace_back(partial_factorial, start, end);
        start = end + 1;
    }

    for (auto& t : threads) {
        t.join();
    }

    return global_result;
}

int main() {
    int num, num_threads;
    cout << "Enter a number: ";
    cin >> num;
    cout << "Enter number of threads: ";
    cin >> num_threads;

    try {
        auto start_time = high_resolution_clock::now();

        long long result = parallel_factorial(num, num_threads);

        auto end_time = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(end_time - start_time);

        cout << "Factorial of " << num << " is " << result << endl;
        cout << "Execution time: " << duration.count() << " ms" << endl;
    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}
