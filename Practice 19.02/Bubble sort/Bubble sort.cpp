#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <omp.h>

using namespace std;

// Проверка массива на отсортированность
bool isSorted(const vector<int>& arr) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        if (arr[i] > arr[i + 1])
            return false;
    }
    return true;
}

// Обычная пузырьковая сортировка
void bubbleSort(vector<int>& arr) {
    size_t n = arr.size();
    for (size_t i = 0; i < n - 1; ++i) {
        for (size_t j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1])
                swap(arr[j], arr[j + 1]);
        }
    }
}

// Параллельная пузырьковая сортировка (алгоритм чет-нечет)
void bubbleSortParallel(vector<int>& arr) {
    size_t n = arr.size();
    bool sorted = false;

    while (!sorted) {
        sorted = true;

        // Четная итерация
        #pragma omp parallel for reduction(|| : sorted)
            for (int j = 0; j < n - 1; j += 2) {
                if (arr[j] > arr[j + 1]) {
                    swap(arr[j], arr[j + 1]);
                    sorted = false;
                }
            }
            #pragma omp barrier

        // Нечетная итерация
        #pragma omp parallel for reduction(|| : sorted)
            for (int j = 1; j < n - 1; j += 2) {
                if (arr[j] > arr[j + 1]) {
                    swap(arr[j], arr[j + 1]);
                    sorted = false;
                }
            }
            #pragma omp barrier
    }
}

int main() {
    const int N = 10000;
    vector<int> arr1(N), arr2(N);

    srand(time(nullptr));

    // Заполнение массива случайными числами от 1 до 10000
    for (int i = 0; i < N; ++i) {
        arr1[i] = rand() % 10000 + 1;
    }
    arr2 = arr1;

    // Установка количества потоков для OpenMP
    omp_set_num_threads(omp_get_max_threads());

    // Замер времени обычной сортировки
    auto start1 = chrono::high_resolution_clock::now();
    bubbleSort(arr1);
    auto end1 = chrono::high_resolution_clock::now();

    // Замер времени параллельной сортировки
    auto start2 = chrono::high_resolution_clock::now();
    bubbleSortParallel(arr2);
    auto end2 = chrono::high_resolution_clock::now();

    // Подсчёт времени выполнения в миллисекундах
    auto time1 = chrono::duration_cast<chrono::milliseconds>(end1 - start1).count();
    auto time2 = chrono::duration_cast<chrono::milliseconds>(end2 - start2).count();

    // Вывод результатов для обычной сортировки
    cout << "Bubble Sort time: " << time1 << " ms" << endl;
    cout << "Bubble Sort is " << (isSorted(arr1) ? "correct" : "incorrect") << endl;

    // Вывод результатов для параллельной сортировки
    cout << "Parallel Bubble Sort time: " << time2 << " ms" << endl;
    cout << "Parallel Bubble Sort is " << (isSorted(arr2) ? "correct" : "incorrect") << endl;

    // Вывод количества доступных потоков OpenMP
    cout << "Threads used: " << omp_get_max_threads() << endl;

    return 0;
}
