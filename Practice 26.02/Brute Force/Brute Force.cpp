#include <iostream>
#include <fstream>
#include <string>
#include <openssl/sha.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sys/stat.h>
#include <chrono>
#include <omp.h>
#include <iomanip>

using namespace std;
using namespace chrono;

// Функция для вычисления SHA-256 от строки
void compute_hash(const string& input, unsigned char* output) {
    SHA256((const unsigned char*)input.c_str(), input.length(), output);
}

// Функция для вывода хэша в консоль в шестнадцатеричном виде
void print_hash(const unsigned char* hash) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        printf("%02x", hash[i]);
    }
    cout << endl;
}

// Проверка существования файла
bool file_exists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Получение размера файла
long get_file_size(const string& filename) {
    ifstream file(filename, ios::binary | ios::ate);
    return file.tellg();
}

// Функция генерации случайной строки длиной len
string random_string(int len) {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    string result;
    result.resize(len);

    for (int i = 0; i < len; ++i)
        result[i] = charset[rand() % (sizeof(charset) - 1)];

    return result;
}

int main() {
    srand(time(nullptr));

    const string filename = "pswd.txt";

    // Если файл не существует или он пустой, генерируем пароли
    if (!file_exists(filename) || get_file_size(filename) == 0) {
        cout << "File not found or empty. Generating passwords..." << endl;

        ofstream file(filename);

        const int num_passwords = 20000000;  // размер словаря
        const int pass_length = 8;           // длина пароля

        for (int i = 0; i < num_passwords; ++i) {
            file << random_string(pass_length) << endl;
        }

        file.close();
        cout << "Passwords generated and saved to " << filename << endl;
    }

    // Открытие файла и чтение всех паролей
    ifstream fin(filename);
    vector<string> passwords;
    string line;

    while (getline(fin, line)) {
        passwords.push_back(line);
    }

    // Выбираем случайный индекс для целевого пароля
    int random_index = rand() % passwords.size();
    string target_password = passwords[random_index];

    // Выводим целевой пароль
    cout << "Target password (from file): " << target_password << endl;

    // Вычисление хэша целевого пароля
    unsigned char target_hash[SHA256_DIGEST_LENGTH];
    compute_hash(target_password, target_hash);

    // Выводим хэш пароля, который будем искать
    cout << "Searching for the hash of password: " << target_password << endl;
    cout << "Target hash: ";
    print_hash(target_hash);

    // Открываем файл с паролями для чтения
    fin.clear();
    fin.seekg(0);
    vector<string> passwords_for_search;
    while (getline(fin, line)) {
        passwords_for_search.push_back(line);
    }

    cout << "Total passwords read from file: " << passwords_for_search.size() << endl;

    // Установка числа потоков
    omp_set_num_threads(8);

    string found_seq, found_par;
    bool found_flag_seq = false, found_flag_par = false;

    // Последовательный поиск
    auto start_seq = high_resolution_clock::now();
    for (int i = 0; i < passwords_for_search.size(); ++i) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        compute_hash(passwords_for_search[i], hash);

        if (memcmp(hash, target_hash, SHA256_DIGEST_LENGTH) == 0) {
            found_seq = passwords_for_search[i];
            found_flag_seq = true;
            break;
        }
    }
    auto end_seq = high_resolution_clock::now();
    auto time_seq = duration_cast<microseconds>(end_seq - start_seq).count();

    if (found_flag_seq) {
        cout << "Sequential found password: " << found_seq << endl;
    }
    else {
        cout << "Sequential search did not find the password." << endl;
    }

    // Параллельный поиск
    auto start_par = high_resolution_clock::now();
#pragma omp parallel
    {
        string found_par_local;
        bool found_flag_local = false;

#pragma omp for schedule(static, 1000)
        for (int i = 0; i < passwords_for_search.size(); ++i) {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            compute_hash(passwords_for_search[i], hash);

            if (memcmp(hash, target_hash, SHA256_DIGEST_LENGTH) == 0) {
                found_par_local = passwords_for_search[i];
                found_flag_local = true;
            }

            if (found_flag_local) {
#pragma omp flush(found_flag_par, found_par)
                if (!found_flag_par) {
                    found_par = found_par_local;
                    found_flag_par = true;
                }
                break;
            }
        }
    }
    auto end_par = high_resolution_clock::now();
    auto time_par = duration_cast<microseconds>(end_par - start_par).count();

    if (found_flag_par) {
        cout << "Parallel found password: " << found_par << endl;
    }
    else {
        cout << "Parallel search did not find the password." << endl;
    }

    // Вывод результатов
    cout << "\nResults:" << endl;
    cout << "Sequential found: " << found_seq << endl;
    cout << "Parallel   found: " << found_par << endl;

    cout << "\nSequential time: " << time_seq << " microseconds" << endl;
    cout << "Parallel time:   " << time_par << " microseconds" << endl;

    double speedup = (double)time_seq / time_par;
    auto diff = time_seq - time_par;

    cout << "Time difference: " << diff << " microseconds" << endl;
    cout << "Speedup: " << speedup << "x" << endl;

    return 0;
}
