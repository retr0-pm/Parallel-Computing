#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <omp.h>
#include <thread>

using namespace std;
using namespace chrono;

// Определение размеров поля
const int WIDTH = 70;
const int HEIGHT = 20;
const int MAX_STEPS = 1000;

// Символы для вывода
#define DEAD '.'
#define ALIVE '#'

#define RESET_COLOR "\033[0m"
#define ALIVE_COLOR "\033[1;32m"  // Зеленый цвет для живых клеток
#define DEAD_COLOR "\033[1;31m"   // Красный цвет для мертвых клеток

// Функция для генерации случайной инициализации
void random_initialize(vector<vector<bool>>& grid) {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            grid[i][j] = rand() % 2;
        }
    }
}

// Функция для инициализации глайдера (Glider)
void glider_initialize(vector<vector<bool>>& grid) {
    grid[1][2] = true;
    grid[2][3] = true;
    grid[3][1] = true;
    grid[3][2] = true;
    grid[3][3] = true;
}

// Функция для подсчета соседей
int count_neighbors(const vector<vector<bool>>& grid, int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;  // Пропускаем саму клетку
            int nx = x + i, ny = y + j;
            if (nx >= 0 && nx < HEIGHT && ny >= 0 && ny < WIDTH && grid[nx][ny]) {
                ++count;
            }
        }
    }
    return count;
}

// Функция для обновления состояния клеток (по классическим правилам)
void update_grid(const vector<vector<bool>>& old_grid, vector<vector<bool>>& new_grid,
    int& born, int& died) {
#pragma omp parallel for collapse(2) // Параллельная обработка клеток
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            int neighbors = count_neighbors(old_grid, i, j);
            if (old_grid[i][j]) {
                new_grid[i][j] = (neighbors == 2 || neighbors == 3); // Живая клетка
                if (!new_grid[i][j]) {
#pragma omp atomic
                    died++;  // Клетка умерла
                }
            }
            else {
                new_grid[i][j] = (neighbors == 3); // Мёртвая клетка
                if (new_grid[i][j]) {
#pragma omp atomic
                    born++;  // Клетка родилась
                }
            }
        }
    }
}

// Функция для вывода текущего состояния поля
void print_grid(const vector<vector<bool>>& grid, int iteration) {
    int alive = 0;

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (grid[i][j]) {
                cout << ALIVE_COLOR << ALIVE << RESET_COLOR;  // Живая клетка (зеленая)
                alive++;
            }
            else {
                cout << DEAD_COLOR << DEAD << RESET_COLOR;  // Мёртвая клетка (красная)
            }
        }
        cout << endl;
    }

    cout << "\nIteration: " << iteration + 1 << " | Alive cells: " << alive << endl;
}

// Функция для очистки экрана
void clear_screen() {
    cout << "\033[2J\033[H";
}

// Основная функция
int main() {
    srand(time(0));

    vector<vector<bool>> grid(HEIGHT, vector<bool>(WIDTH, false));
    vector<vector<bool>> new_grid(HEIGHT, vector<bool>(WIDTH, false));

    int choice;
    cout << "Choose initialization type (1 - Random, 2 - Glider): ";
    cin >> choice;

    if (choice == 1) {
        random_initialize(grid);
    }
    else if (choice == 2) {
        glider_initialize(grid);
    }
    else {
        cout << "Invalid choice!" << endl;
        return 1;
    }

    int steps;
    cout << "Enter the number of steps: ";
    cin >> steps;

    int total_born = 0;  // Всего родившихся клеток
    int total_died = 0;  // Всего умерших клеток

    for (int step = 0; step < steps; ++step) {
        auto start = high_resolution_clock::now();

        int born = 0, died = 0;
        clear_screen();

        update_grid(grid, new_grid, born, died);
        grid.swap(new_grid);

        // Выводим состояние поля
        print_grid(grid, step);

        total_born += born;
        total_died += died;

        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();

        // Вывод информации о времени выполнения и клетках
        cout << "Iteration time: " << time << " ms" << endl;
        cout << "Born cells: " << born << " | Died cells: " << died << endl;
        cout << "Total born cells: " << total_born << " | Total died cells: " << total_died << endl;
        cout << endl;

        // Задержка после вывода
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}
