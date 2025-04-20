#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace chrono;

// Функция для вычисления значения f(x) = sinh(x)       (гиперболический синус)
double f(double x) {
    return sinh(x);
}

// Последовательное численное интегрирование методом средних прямоугольников
double integral_seq(double a, double b, int n) {
    double sum = 0.0;
    double dx = (b - a) / n;

    for (int i = 0; i < n; ++i) {
        double x = a + i * dx + dx / 2;
        sum += f(x);
    }

    return sum * dx;
}

// Параллельное численное интегрирование методом средних прямоугольников
double integral_par(double a, double b, int n) {
    double sum = 0.0;
    double dx = (b - a) / n;

    // Используем локальные суммы для каждого потока
#pragma omp parallel
    {
        double local_sum = 0.0;

        // Динамическое распределение работы между потоками
#pragma omp for schedule(dynamic, 100000)
        for (int i = 0; i < n; ++i) {
            double x = a + i * dx + dx / 2;
            local_sum += f(x);
        }

        // Добавляем локальные результаты в общий результат
#pragma omp atomic
        sum += local_sum;
    }

    return sum * dx;
}

int main() {
    double a = 0.0, b = 4.0; // Интервал интегрирования от 0 до 4
    int n = 10000000; // Количество разбиений

    // Вычисление аналитического решения
    double analytic_result = cosh(b) - cosh(a);

    // Замер времени последовательного метода
    auto start_seq = high_resolution_clock::now();
    double seq_result = integral_seq(a, b, n);
    auto end_seq = high_resolution_clock::now();
    auto time_seq = duration_cast<milliseconds>(end_seq - start_seq).count();

    // Замер времени параллельного метода
    auto start_par = high_resolution_clock::now();
    double par_result = integral_par(a, b, n);
    auto end_par = high_resolution_clock::now();
    auto time_par = duration_cast<milliseconds>(end_par - start_par).count();

    // Вывод результатов и ошибки
    cout << "Analytic result: " << analytic_result << endl;
    cout << "Sequential result: " << seq_result << endl;
    cout << "Parallel result: " << par_result << endl;

    cout << "Error (sequential): " << abs(analytic_result - seq_result) << endl;
    cout << "Error (parallel): " << abs(analytic_result - par_result) << endl;

    // Вывод времени
    double speedup = (double)time_seq / time_par;
    cout << "Sequential time: " << time_seq << " ms" << endl;
    cout << "Parallel time: " << time_par << " ms" << endl;

    // Разница по времени
    cout << "Time difference: " << abs(time_seq - time_par) << " ms" << endl;
    cout << "Speedup: " << speedup << "x" << endl;

    return 0;
}
