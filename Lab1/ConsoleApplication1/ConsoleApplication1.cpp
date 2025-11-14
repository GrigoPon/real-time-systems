#include <iostream>
#include <thread>
#include <string>
#include <time.h>

// Функция для вычисления факториала
unsigned long long factorial(int n) {
    unsigned long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Функция, которая будет выполняться в потоке
void Func(std::string name) {
    const int iterations = 10000000;
    const int n = 10;

    for (int i = 0; i < iterations; i++) {
        volatile unsigned long long fact = factorial(n); // volatile предотвращает чрезмерную оптимизацию
    }

    std::cout << "Поток " << name << " завершён." << std::endl;
}

int main() {
    setlocale(LC_ALL, "RU");

    const int total_iterations = 10000000;
    const int n = 10;


    std::cout << "Введите номер задачи: " << std::endl;
    std::cout << "1) Упражнение 1 \n 2) Задание 1 \n 3) Задание 2 \n ";
    int k = 0;
    std::cin >> k;
    switch (k) {
    case 1: {
        //РАЗМИНКА

        std::string s = "01234";
        for (/*чтобы запустилась программа в конструкции свитч кейс*/ /*unsigned*/ int i = s.size() - 1; i >= 0; i--)
        {
            std::cout << s[i] << std::endl;
        }
        break;
    }
    case 2: {
        clock_t start = clock();

        for (int i = 0; i < total_iterations; i++) {
            unsigned long long fact = factorial(n);
        }

        clock_t end = clock();
        double seconds = (double)(end - start) / CLOCKS_PER_SEC;

        std::cout << "10! = " << factorial(10) << std::endl;
        std::cout << "Time taken: " << seconds << " seconds" << std::endl;
        break;
    }
    case 3: {
        // ЗАМЕР ВРЕМЕНИ ДЛЯ ПОСЛЕДОВАТЕЛЬНОГО ВЫПОЛНЕНИЯ
        std::cout << "Последовательное выполнение..." << std::endl;
        clock_t start_seq = clock();

        // Первый вызов
        for (int i = 0; i < total_iterations; i++) {
            volatile unsigned long long fact1 = factorial(n);
        }

        // Второй вызов  
        for (int i = 0; i < total_iterations; i++) {
            volatile unsigned long long fact2 = factorial(n);
        }

        clock_t end_seq = clock();
        double time_seq = (double)(end_seq - start_seq) / CLOCKS_PER_SEC;
        std::cout << "Время последовательного выполнения: " << time_seq << " секунд" << std::endl;

        std::cout << "\nПараллельное выполнение..." << std::endl;

        // ЗАМЕР ВРЕМЕНИ ДЛЯ ПАРАЛЛЕЛЬНОГО ВЫПОЛНЕНИЯ
        clock_t start_par = clock();

        std::thread thread1(Func, "t1");
        std::thread thread2(Func, "t2");

        thread1.join();
        thread2.join();

        clock_t end_par = clock();
        double time_par = (double)(end_par - start_par) / CLOCKS_PER_SEC;
        std::cout << "Время параллельного выполнения: " << time_par << " секунд" << std::endl;

        // СРАВНЕНИЕ
        std::cout << "\nСравнение:" << std::endl;
        std::cout << "Ускорение: " << time_seq / time_par << "x" << std::endl;
        std::cout << "Эффективность: " << (time_seq / time_par) / 2.0 * 100 << "%" << std::endl;

        system("pause");
        break;
    }
    default:
        std::cout << "Некорректный номер!" << std::endl;
        break;
    }

    return 0;
}
