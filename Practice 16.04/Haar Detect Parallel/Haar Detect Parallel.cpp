#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <iostream>
#include <omp.h>

using namespace std;
using namespace cv;

int main() {
    // Открытие видеофайла
    VideoCapture cap("vid.mp4");
    if (!cap.isOpened()) {
        cerr << "Error: Failed to open video file!" << endl;
        return -1;
    }

    // Загрузка каскадов Хаара
    CascadeClassifier face_cascade;
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        cerr << "Error: Failed to load face classifier!" << endl;
        return -1;
    }

    CascadeClassifier eye_cascade;
    if (!eye_cascade.load("haarcascade_eye.xml")) {
        cerr << "Error: Failed to load eye classifier!" << endl;
        return -1;
    }

    CascadeClassifier smile_cascade;
    if (!smile_cascade.load("haarcascade_smile.xml")) {
        cerr << "Error: Failed to load smile classifier!" << endl;
        return -1;
    }

    Mat img;

    // Получение размеров видео
    int frame_width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    Size frameSize(frame_width, frame_height);

    // Создание видеофайла для записи результата
    VideoWriter writer("result.mp4", VideoWriter::fourcc('a', 'v', 'c', '1'), 30, frameSize, true);

    while (true) {
        cap >> img;
        if (img.empty()) {
            break;
        }

        // Преобразование в оттенки серого и улучшение контраста
        Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);

        // Векторы для хранения обнаруженных объектов
        vector<Rect> faces, eyes, smiles;

        // Параллельное обнаружение лиц, глаз и улыбок
#pragma omp parallel sections
        {
#pragma omp section
            {
                face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0, Size(50, 50));
            }

#pragma omp section
            {
                eye_cascade.detectMultiScale(gray, eyes, 1.2, 10, 0, Size(50, 50));
            }

#pragma omp section
            {
                smile_cascade.detectMultiScale(gray, smiles, 1.2, 50, 0, Size(40, 20));
            }
        }

        // Отображение прямоугольников вокруг лиц
        for (size_t i = 0; i < faces.size(); i++) {
            rectangle(img, faces[i], Scalar(255, 0, 0), 2);
        }

        // Отображение прямоугольников вокруг глаз
        for (size_t i = 0; i < eyes.size(); i++) {
            bool bad = true;
            Point xy(eyes[i].x, eyes[i].y);
            for (auto face : faces) {
                if (face.contains(xy)) {
                    bad = false;
                    break;
                }
            }
            if (!bad) {
                rectangle(img, eyes[i], Scalar(0, 0, 255), 2);
            }
        }

        // Отображение прямоугольников вокруг улыбок
        for (size_t i = 0; i < smiles.size(); i++) {
            bool bad = true;
            Point xy(smiles[i].x, smiles[i].y);
            for (auto face : faces) {
                if (face.contains(xy)) {
                    bad = false;
                    break;
                }
            }
            if (!bad) {
                rectangle(img, smiles[i], Scalar(0, 255, 0), 2);
            }
        }

        // Показать и записать кадр
        imshow("Detected", img);
        writer.write(img);

        // Выход при нажатии клавиши ESC
        if ((waitKey(30) & 0xEFFFFF) == 27) {
            break;
        }
    }

    // Освобождение ресурсов
    writer.release();
    cap.release();
    destroyAllWindows();

    return 0;
}
