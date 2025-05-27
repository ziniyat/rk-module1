#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <string>

//Добавляем русский язык в вывод
bool chcp(unsigned codepage)
{
    auto command = "chcp " + std::to_string(codepage);
    return !std::system(command.c_str());
}

static bool codepage_is_set = chcp(1251);

using namespace std;
using namespace std::chrono;

const int N = 100;            // Определяем константу для размера матрицы
const int threads_count = 4;  // Константа для кол-ва потоков 

// Создаём 3 глобальных матрицы, 2 из котрых будут умножаться, а 3-я будет результирующей 
vector<vector<int>> A(N, vector<int>(N));
vector<vector<int>> B(N, vector<int>(N));
vector<vector<int>> C(N, vector<int>(N));

// Функция для заполнения матрицы случайными числами
void initialize_matrices() {
    random_device rd;          //Обьявляем генератор случайных чисел
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 10);  //Задаём диапазон случайных значений (от 0 до 10)
    //Цикл, который заполняет матрицу А и В случайными числами
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = dis(gen);
            B[i][j] = dis(gen);
        }
    }
}

// Функция, выполняемая в каждом потоке
void thread_function(int id) {
    int rows_per_thread = N / threads_count;                      // Вычисляем кол-во строк, обрабатываемых каждым потоком
    int start_row = id * rows_per_thread;                         // Вычисляем начальную строку для текущего потока
    int end_row = (id == threads_count - 1) ? N : start_row + rows_per_thread;  // Вычисляем конечную строку

    //Цикл для заполнения результирующей матрицы 
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;                          // Обнуляем элемент результирующей матрицы
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];    // Производим умножение и суммирование 
            }
        }
    }
}

// Основная функция
int main() {
    initialize_matrices();      // Заполняем матрицы

    vector<thread> threads;     // Вектор для хранения потоков

    auto start = high_resolution_clock::now();   // Запись времени начала

    // Цикл для создания потоков 
    for (int i = 0; i < threads_count; ++i) {
        threads.emplace_back(thread_function, i);   // Создание нового потока
    }

    for (auto& t : threads) {
        t.join();                // Ожидание завершения всех потоков
    }

    auto end = high_resolution_clock::now(); // Запись времени окончания

    // Вывод времени выполнения программы в миллисекундах
    cout << "Затраченное время на многопоточное умножение: "
        << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    // Однопоточное умножение для сравнения
    start = high_resolution_clock::now(); // Запись времени начала

    //Цикл заполнения результирующей матрицы для одного потока
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0; // Обнуляем элемент результирующей матрицы
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j]; // Умножение и суммирование
            }
        }
    }

    end = high_resolution_clock::now(); // Запись времени окончания

    // Вывод времени выполнения программы в миллисекундах
    cout << "Затраченное время на однопоточное умножение: "
        << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    return 0;
}



Выводы:
Многопоточное умножение быстрее однопоточного, которое занимает 23 миллисекунды.
Время выполнения в зависимости от кол-ва потоков:
- 15 миллисекунд при 2-х потоках
- 7 миллисекунд при 4-х потоках
- 5 миллисекунд при 8-ми потоках
В случае одной и той же матрицы.
