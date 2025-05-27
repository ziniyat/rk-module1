#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <mutex>
#include <atomic>
#include <string>

// Добавляем русский язык в вывод
bool chcp(unsigned codepage) {
    auto command = "chcp " + std::to_string(codepage);
    return !std::system(command.c_str());
}

static bool codepage_is_set = chcp(1251);

using namespace std;
using namespace std::chrono;

const int client_count = 10;            // Константа для кол-ва клиентов
const int transaction_count = 100;      // Кол-во транзакций
const int initial_balance = 1000;       // Начальный баланс для каждого клиента

//Создадим глобальные переменные для хранения балансов:
int balances[client_count];                   // Балансы клиентов
atomic<int> atomic_balances[client_count];    // Атомарные балансы клиентов
mutex mtx;                                    // Мьютекс для синхронизации доступа к обычным балансам

// Функция для выполнения транзакций без синхронизации
void unsync_transactions(int client_id) {
    for (int i = 0; i < transaction_count; ++i) {
        int amount = rand() % 200 - 100;         // Генерируем случаную сумму транзакции, так, чтобы она была в диапазоне от -100 до 99
        atomic_balances[client_id] += amount;    // Добавляем изменение баланса
    }
}

// Функция для выполнения транзакций с использованием std::atomic
void atomic_transactions(int client_id) {
    for (int i = 0; i < transaction_count; ++i) {
        int amount = rand() % 200 - 100;         // Делаем то же самое, что и функции без синхронизации, только с использованием атомарных переменных
        atomic_balances[client_id] += amount;    
    }
}

// Функция для выполнения транзакций с использованием std::mutex
void mutex_transactions(int client_id) {
    for (int i = 0; i < transaction_count; ++i) {
        int amount = rand() % 200 - 100;         // Случайная сумма для транзакции
        lock_guard<mutex> lock(mtx);             // Обеспечиваем защиту доступа к балансу
        balances[client_id] += amount;           // Измененяем баланс
    }
}

// Основная функция
int main() {
    srand(time(0)); // Инициализация генератора случайных чисел

    // Создание балансов для каждого клиента 
    for (int i = 0; i < client_count; ++i) {
        balances[i] = initial_balance;
        atomic_balances[i] = initial_balance;
    }

    // Блок для работы потоков без синхронизации
    auto start_unsync = high_resolution_clock::now();  //Считываем начальное время
    vector<thread> threads_unsync;                     //Вектор для хранения потоков
    //Цикл для создания потоков
    for (int i = 0; i < client_count; ++i) {
        threads_unsync.emplace_back(unsync_transactions, i);
    }
    for (auto& t : threads_unsync) {
        t.join();                    //Ожидание завершения потоков
    }
    auto end_unsync = high_resolution_clock::now(); //Считываем конечное время

    // Вывод результата
    int total_unsync = 0;
    //Цикл для суммирования всех балансов
    for (int i = 0; i < client_count; ++i) {
        total_unsync += balances[i];
    }
    //Вывод результата
    cout << "Общий баланс (без синхронизации): " << total_unsync << endl;
    cout << "Затраченное время (без синхронизации): "
        << duration_cast<nanoseconds>(end_unsync - start_unsync).count() << " ns" << endl;

    // Сброс счетов для следующего метода
    for (int i = 0; i < client_count; ++i) {
        balances[i] = initial_balance;
    }

    // Блок для работы потоков с использованием std::atomic
    //Здесь почти то же самое, что и в функции без синхронизации, только с испольхованием атомарных переменных
    auto start_atomic = high_resolution_clock::now(); 
    vector<thread> threads_atomic;                     
    for (int i = 0; i < client_count; ++i) {
        threads_atomic.emplace_back(atomic_transactions, i);
    }
    for (auto& t : threads_atomic) {
        t.join();
    }
    auto end_atomic = high_resolution_clock::now();   

    int total_atomic = 0;
    for (int i = 0; i < client_count; ++i) {
        total_atomic += atomic_balances[i].load(); //Суммирование всех балансов
    }
    //Вывод результата 
    cout << "Общий баланс (с использованием std::atomic): " << total_atomic << endl;
    cout << "Затраченное время (с использованием std::atomic): "
        << duration_cast<nanoseconds>(end_atomic - start_atomic).count() << " ns" << endl;

    // Сброс счетов для следующего метода
    for (int i = 0; i < client_count; ++i) {
        balances[i] = initial_balance;
    }

    // Блок для работы потоков с использованием std::mutex
    // То же самое, только теперь работаем с мьютексами
    auto start_mutex = high_resolution_clock::now();
    vector<thread> threads_mutex;
    for (int i = 0; i < client_count; ++i) {
        threads_mutex.emplace_back(mutex_transactions, i);
    }
    for (auto& t : threads_mutex) {
        t.join();
    }
    auto end_mutex = high_resolution_clock::now();

    //Получение суммы всех балансов 
    int total_mutex = 0;
    for (int i = 0; i < client_count; ++i) {
        total_mutex += balances[i];
    }
    //Вывод результата
    cout << "Общий баланс (с использованием std::mutex): " << total_mutex << endl;
    cout << "Затраченное время (с использованием std::mutex): "
        << duration_cast<nanoseconds>(end_mutex - start_mutex).count() << " ns" << endl;

    return 0;
}

Выводы:
Самым быстрым методом в этом задании оказался метод с использованием мьютексов
Дальше идёт метод с использованием атомарных переменных
И самый долгий метод - метод без синхронизации
Вдобавок, метод без синхронизации оказался довольно некорректным, так как с ним происходит гонка данных, когда несколько потоков одновременно изменяют один и тот же ресурс (в данном случае, это баланс клиента). 
