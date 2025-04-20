#include <iostream>
#include <opencv2/opencv.hpp>
#include <omp.h>

using namespace std;
using namespace cv;

// Размер изображения
const int SIZE = 729;

// Цвета
const Scalar BACKGROUND_COLOR = Scalar(255, 255, 255); // белый
const Scalar CARPET_COLOR = Scalar(255, 0, 0);         // синий

// Рекурсивно заполняет изображение квадратами
void drawCarpet(Mat& image, int x, int y, int size, int depth) {
    if (depth == 0) {
        rectangle(image, Point(x, y), Point(x + size, y + size), CARPET_COLOR, FILLED);
        return;
    }

    int newSize = size / 3;

    // Параллельная обработка всех подквадратов
#pragma omp parallel for collapse(2)
    for (int dx = 0; dx < 3; ++dx) {
        for (int dy = 0; dy < 3; ++dy) {
            if (dx == 1 && dy == 1) {
                // Центр оставляем белым
                rectangle(image, Point(x + newSize, y + newSize),
                    Point(x + 2 * newSize, y + 2 * newSize), BACKGROUND_COLOR, FILLED);
            }
            else {
                drawCarpet(image, x + dx * newSize, y + dy * newSize, newSize, depth - 1);
            }
        }
    }
}

int main() {
    int depth;
    cout << "Enter recursion depth (1-7): "; //Порядок рекурсии
    cin >> depth;

    if (depth < 1 || depth > 7) {
        cout << "Invalid depth value!" << endl;
        return -1;
    }

    // Создаем изображение белого цвета
    Mat image(SIZE, SIZE, CV_8UC3, BACKGROUND_COLOR);

    // Старт таймера
    double start = omp_get_wtime();

    // Рисуем ковер
    drawCarpet(image, 0, 0, SIZE, depth);

    // Конец таймера
    double end = omp_get_wtime();
    cout << "Rendering time: " << (end - start) * 1000 << " ms" << endl;

    // Показываем изображение
    imshow("Sierpinski Carpet", image);
    waitKey(0);

    return 0;
}
