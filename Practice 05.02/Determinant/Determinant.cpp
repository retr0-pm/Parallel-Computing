// Determinant.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;
using namespace chrono;

mutex mtx;

double determinant(vector<vector<double>> matrix, int N);

double compute_minor(vector<vector<double>> matrix, int N, int col, double& result) {
    vector<vector<double>> minor(N - 1, vector<double>(N - 1));
    for (int i = 1; i < N; ++i) {
        int minor_col = 0;
        for (int j = 0; j < N; ++j) {
            if (j == col) continue;
            minor[i - 1][minor_col] = matrix[i][j];
            minor_col++;
        }
    }
    result = determinant(minor, N - 1);

    return result;
}

double determinant(vector<vector<double>> matrix, int N) {
    if (N == 1) return matrix[0][0];
    if (N == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    double det = 0.0;
    vector<thread> threads;
    vector<double> minors(N, 0.0);

    for (int i = 0; i < N; ++i) {
        threads.emplace_back(compute_minor, matrix, N, i, ref(minors[i]));
    }

    for (auto& th : threads) {
        th.join();
    }

    for (int i = 0; i < N; ++i) {
        det += (i % 2 == 0 ? 1 : -1) * matrix[0][i] * minors[i];
    }

    return det;
}

int main() {
    int N;
    cout << "Enter dimension N of a square matrix: ";
    cin >> N;

    vector<vector<double>> matrix(N, vector<double>(N));
    cout << "Enter the matrix (" << N << "x" << N << " elements) : \n";
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            cin >> matrix[i][j];
        }
    }

    auto start = high_resolution_clock::now();
    double det = determinant(matrix, N);
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(end - start);

    cout << "Determinant is: " << det << endl;
    cout << "Execution time: " << duration.count() << " microseconds" << endl;

    return 0;
}