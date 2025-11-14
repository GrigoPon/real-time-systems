#include <thread>
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
#include <cmath>

//std::mutex m;

void Func(std::string name)
{
    long double i = 0;
    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        i += 1e-9L;  // прибавляем 10^(-9)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (elapsed.count() >= 1000) // 1 секунда
            break;
    }

    //m.lock();
    std::cout << name << ": " << i << std::endl;
    //m.unlock();
}

int main()
{
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");

    thread1.join();
    thread2.join();
    thread3.join();

    system("pause");
    return 0;
}