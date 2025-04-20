#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace chrono;

const int SIZE = 500;

// Функция для заполнения матрицы случайными числами
void fill_matrix(vector<vector<int>>& matrix) {
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            matrix[i][j] = rand() % 100;
}

// Последовательное умножение матриц
void multiply_sequential(const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C) {
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < SIZE; ++k)
                C[i][j] += A[i][k] * B[k][j];
        }
}

// Параллельное умножение матриц
void multiply_parallel(const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C) {
#pragma omp parallel for
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < SIZE; ++k)
                C[i][j] += A[i][k] * B[k][j];
        }
}

// Проверка на совпадение двух матриц
bool compare_matrices(const vector<vector<int>>& M1, const vector<vector<int>>& M2) {
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            if (M1[i][j] != M2[i][j])
                return false;
    return true;
}

int main() {
    srand(time(0));

    vector<vector<int>> A(SIZE, vector<int>(SIZE));
    vector<vector<int>> B(SIZE, vector<int>(SIZE));
    vector<vector<int>> C_seq(SIZE, vector<int>(SIZE));
    vector<vector<int>> C_par(SIZE, vector<int>(SIZE));

    fill_matrix(A);
    fill_matrix(B);

    // Последовательное умножение
    auto start_seq = high_resolution_clock::now();
    multiply_sequential(A, B, C_seq);
    auto end_seq = high_resolution_clock::now();
    auto duration_seq = duration_cast<milliseconds>(end_seq - start_seq);

    // Параллельное умножение
    auto start_par = high_resolution_clock::now();
    multiply_parallel(A, B, C_par);
    auto end_par = high_resolution_clock::now();
    auto duration_par = duration_cast<milliseconds>(end_par - start_par);

    // Проверка корректности
    bool equal = compare_matrices(C_seq, C_par);

    // Вывод результатов
    cout << "Sequential time: " << duration_seq.count() << " ms" << endl;
    cout << "Parallel time:   " << duration_par.count() << " ms" << endl;
    cout << "Results are " << (equal ? "correct (equal)" : "different (error)") << endl;

    return 0;
}
