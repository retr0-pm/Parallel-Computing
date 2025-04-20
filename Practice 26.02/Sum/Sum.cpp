#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace chrono;

int main() {
    const int N = 100000000;
    vector<int> arr(N);

    srand(time(nullptr));

    // Заполнение массива случайными числами от 1 до 100
    for (int i = 0; i < N; ++i) {
        arr[i] = rand() % 100 + 1;
    }

    long long sum_seq = 0;
    long long sum_par = 0;

    omp_set_num_threads(omp_get_max_threads());

    // Последовательное суммирование
    auto start_seq = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        sum_seq += arr[i];
    auto end_seq = high_resolution_clock::now();

    // Параллельное суммирование
    auto start_par = high_resolution_clock::now();
#pragma omp parallel for reduction(+:sum_par) schedule(dynamic, 1000000)
    for (int i = 0; i < N; ++i)
        sum_par += arr[i];
    auto end_par = high_resolution_clock::now();

    // Проверка совпадения результатов
    if (sum_seq != sum_par)
        cout << "Error: sums do not match!" << endl;

    // Время выполнения
    auto time_seq = duration_cast<milliseconds>(end_seq - start_seq).count();
    auto time_par = duration_cast<milliseconds>(end_par - start_par).count();

    // Разница во времени
    auto diff_time = time_seq - time_par;
    double speedup = (double)time_seq / time_par;

    // Вывод результатов
    cout << "Sequential sum time: " << time_seq << " ms" << endl;
    cout << "Parallel sum time (dynamic, chunk 1000000): " << time_par << " ms" << endl;
    cout << "Time difference: " << diff_time << " ms" << endl;
    cout << "Speedup: " << speedup << "x" << endl;
    cout << "Threads used: " << omp_get_max_threads() << endl;

    return 0;
}
