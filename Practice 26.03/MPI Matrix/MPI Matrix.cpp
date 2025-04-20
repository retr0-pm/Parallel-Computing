#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <windows.h>
#include <locale>

#define MATRIX_SIZE 500
#define MIN_RAND_VALUE 1
#define MAX_RAND_VALUE 10

// Функция для заполнения матрицы случайными числами
void fill_matrix(int* matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = MIN_RAND_VALUE + rand() % (MAX_RAND_VALUE - MIN_RAND_VALUE + 1);
        }
    }
}

// Функция для вывода среза матрицы
void print_matrix_slice(int* matrix, int rows, int cols, int slice_size) {
    for (int i = 0; i < slice_size && i < rows; i++) {
        for (int j = 0; j < slice_size && j < cols; j++) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "C");

    int rank, size;
    double start_time, end_time;

    // Инициализация MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получение номера текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получение общего числа процессов

    // Проверка, что размер матрицы делится на количество процессов без остатка
    if (MATRIX_SIZE % size != 0) {
        if (rank == 0) {
            printf("Error: matrix size (%d) must be divisible by the number of processes (%d).\n", MATRIX_SIZE, size);
        }
        MPI_Finalize();
        return 1;
    }

    int rows_per_process = MATRIX_SIZE / size;

    // Выделение памяти под матрицы
    int* A = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* B = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* C = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));

    // Локальные части матрицы A и результирующей матрицы C для каждого процесса
    int* local_A = (int*)malloc(rows_per_process * MATRIX_SIZE * sizeof(int));
    int* local_C = (int*)malloc(rows_per_process * MATRIX_SIZE * sizeof(int));

    // Только корневой процесс (rank 0) заполняет матрицы A и B
    if (rank == 0) {
        srand(time(NULL));
        fill_matrix(A, MATRIX_SIZE, MATRIX_SIZE);
        fill_matrix(B, MATRIX_SIZE, MATRIX_SIZE);
        start_time = MPI_Wtime(); // Засекаем время начала вычислений
    }

    // Распределение строк матрицы A между процессами
    MPI_Scatter(A, rows_per_process * MATRIX_SIZE, MPI_INT, local_A, rows_per_process * MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // Передача всей матрицы B всем процессам
    MPI_Bcast(B, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // Параллельное перемножение матриц
    for (int i = 0; i < rows_per_process; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            local_C[i * MATRIX_SIZE + j] = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                local_C[i * MATRIX_SIZE + j] += local_A[i * MATRIX_SIZE + k] * B[k * MATRIX_SIZE + j];
            }
        }
    }

    // Сборка результирующей матрицы C из локальных частей
    MPI_Gather(local_C, rows_per_process * MATRIX_SIZE, MPI_INT, C, rows_per_process * MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // Вывод результатов и времени выполнения только в корневом процессе
    if (rank == 0) {
        end_time = MPI_Wtime();

        printf("\n=== Matrix Multiplication Results ===\n");
        printf("\nFirst 5x5 elements of the resulting matrix:\n");
        print_matrix_slice(C, MATRIX_SIZE, MATRIX_SIZE, 5);

        printf("Matrix size: %dx%d\n", MATRIX_SIZE, MATRIX_SIZE);
        printf("Number of processes: %d\n", size);
        printf("Execution time: %.3f seconds\n", end_time - start_time);

        // Освобождение памяти у корневого процесса
        free(A);
        free(B);
        free(C);
    }

    // Освобождение локальной памяти у всех процессов
    free(local_A);
    free(local_C);

    // Завершение работы MPI
    MPI_Finalize();
    return 0;
}
