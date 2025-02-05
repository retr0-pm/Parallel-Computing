// Factorial.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.

#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

long long factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;
    return result;
}

int main() {
    int num;
    cout << "Enter a number: ";
    cin >> num;

    auto start_time = high_resolution_clock::now();

    if (num < 0)
        cout << "Factorial is not defined for negative numbers." << endl;
    else
        cout << "Factorial of " << num << " is " << factorial(num) << endl;

    auto end_time = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(end_time - start_time);

    cout << "Execution time: " << duration.count() << " ms" << endl;

    return 0;
}
