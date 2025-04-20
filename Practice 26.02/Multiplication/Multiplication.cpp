#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace chrono;

// Генерация случайной матрицы размером n x n и вектора размером n
void generateRandomData(int n, vector<vector<double>>& matrix, vector<double>& vec) {
    srand(time(nullptr));

    // Генерация случайной матрицы в диапазоне от -100 до 100
    matrix.resize(n, vector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            matrix[i][j] = rand() % 201 - 100;

    // Генерация случайного вектора в диапазоне от -100 до 100
    vec.resize(n);
    for (int i = 0; i < n; ++i)
        vec[i] = rand() % 201 - 100;
}

// Последовательное умножение матрицы на вектор
void matrixVectorMultSeq(const vector<vector<double>>& matrix, const vector<double>& vec, vector<double>& result) {
    int n = matrix.size();
    for (int i = 0; i < n; ++i) {
        result[i] = 0.0;
        for (int j = 0; j < n; ++j) {
            result[i] += matrix[i][j] * vec[j];
        }
    }
}

// Параллельное умножение матрицы на вектор
void matrixVectorMultPar(const vector<vector<double>>& matrix, const vector<double>& vec, vector<double>& result) {
    int n = matrix.size();

    // Параллельный цикл с динамическим распределением задач по потокам
    #pragma omp parallel for schedule(dynamic, 1000) // Используем динамическое распределение
        for (int i = 0; i < n; ++i) {
            double local_sum = 0.0; // Локальная сумма для каждой строки
            for (int j = 0; j < n; ++j) {
                local_sum += matrix[i][j] * vec[j];
            }

        // Используем атомарную операцию для добавления локальной суммы к результату
        #pragma omp atomic
        result[i] += local_sum;
        }
}

int main() {
    int n = 10000; // Увеличиваем размер матрицы для более точной оценки

    // Генерация случайных данных
    vector<vector<double>> matrix;
    vector<double> vec, result_seq(n, 0.0), result_par(n, 0.0);
    generateRandomData(n, matrix, vec);

    // Установка количества потоков
    omp_set_num_threads(8);

    // Замер времени для последовательного умножения
    auto start_seq = high_resolution_clock::now();
    matrixVectorMultSeq(matrix, vec, result_seq);
    auto end_seq = high_resolution_clock::now();
    auto time_seq = duration_cast<milliseconds>(end_seq - start_seq).count();

    // Замер времени для параллельного умножения
    auto start_par = high_resolution_clock::now();
    matrixVectorMultPar(matrix, vec, result_par);
    auto end_par = high_resolution_clock::now();
    auto time_par = duration_cast<milliseconds>(end_par - start_par).count();

    // Проверка корректности результатов
    bool correct = true;
    for (int i = 0; i < n; ++i) {
        if (abs(result_seq[i] - result_par[i]) > 1e-6) {
            correct = false;
            break;
        }
    }

    // Вывод результатов
    cout << "Sequential time: " << time_seq << " ms" << endl;
    cout << "Parallel time: " << time_par << " ms" << endl;

    // Разница по времени
    cout << "Time difference: " << abs(time_seq - time_par) << " ms" << endl;

    // Ускорение
    double speedup = (double)time_seq / time_par;
    cout << "Speedup: " << speedup << "x" << endl;

    // Проверка корректности
    if (correct) {
        cout << "Results are correct!" << endl;
    }
    else {
        cout << "Results are incorrect!" << endl;
    }

    return 0;
}
