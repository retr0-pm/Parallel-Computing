#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <limits.h>

using namespace std;

// Размер массива и диапазон значений
const int ARRAY_SIZE = 1000000;
const int MAX_VALUE = 100;

int main(int argc, char** argv) {
    int rank, size;

    // Инициализация MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // номер текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // общее количество процессов

    int elements_per_proc = ARRAY_SIZE / size;
    int remainder = ARRAY_SIZE % size;

    // Учитываем остаток — распределяем по первым процессам
    int local_size = elements_per_proc + (rank < remainder ? 1 : 0);

    vector<int> local_array(local_size);

    // Инициализация генератора случайных чисел для каждого процесса
    srand(time(0) + rank * 1000);

    // Генерация случайного локального массива
    for (int i = 0; i < local_size; ++i) {
        local_array[i] = rand() % MAX_VALUE;
    }

    // Последовательная версия
    double start_seq, end_seq;
    long long seq_sum = 0;
    if (rank == 0) {
        vector<int> full_array(ARRAY_SIZE);
        srand(time(0));

        for (int i = 0; i < ARRAY_SIZE; ++i) {
            full_array[i] = rand() % MAX_VALUE;
        }

        start_seq = MPI_Wtime();
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            seq_sum += full_array[i];
        }
        end_seq = MPI_Wtime();

        cout << "Sequential sum: " << seq_sum << endl;
        cout << "Sequential time: " << (end_seq - start_seq) * 1000 << " ms" << endl;
    }

    // Параллельная версия
    MPI_Barrier(MPI_COMM_WORLD);
    double start_par = MPI_Wtime();

    long long local_sum = 0;
    for (int i = 0; i < local_size; ++i) {
        local_sum += local_array[i];
    }

    long long total_sum = 0;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_par = MPI_Wtime();

    if (rank == 0) {
        cout << "Parallel sum: " << total_sum << endl;
        cout << "Parallel time: " << (end_par - start_par) * 1000 << " ms" << endl;

        cout << "Time difference: " << ((end_seq - start_seq) - (end_par - start_par)) * 1000 << " ms" << endl;
        double speedup = (end_seq - start_seq) / (end_par - start_par);
        cout << "Speedup: " << speedup << "x" << endl;
    }

    MPI_Finalize();
    return 0;
}
