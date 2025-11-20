#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

const int NUM_DAYS = 5;
const int MAX_NUGGETS_PER_FATMAN = 10000;

std::mutex mtx;
std::vector<int> plates = { 3000, 3000, 3000 };
std::atomic<bool> cook_fired(false);
std::atomic<int> fatmen_eaten[3] = { 0, 0, 0 };
std::atomic<bool> has_exploded[3] = { false, false, false };
std::atomic<int> exploded_count(0);
std::atomic<bool> time_up(false);
std::atomic<bool> time_is_up(false);

void fatman(int id, int gluttony) {
    while (!cook_fired.load() && !time_is_up.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(mtx);

        if (cook_fired.load() || time_is_up.load())
            return;

        if (plates[id] <= 0) {
            cook_fired = true;
            return;
        }

        int to_eat = std::min(gluttony, plates[id]);
        plates[id] -= to_eat;
        fatmen_eaten[id] += to_eat;

       
        if (!has_exploded[id] && fatmen_eaten[id] >= MAX_NUGGETS_PER_FATMAN) {
            has_exploded[id] = true;
            exploded_count++;
        }
    }
}

void cook(int efficiency_factor) {
    auto start = std::chrono::steady_clock::now();
    const auto duration = std::chrono::seconds(NUM_DAYS);

    while (!cook_fired.load()) {
        auto now = std::chrono::steady_clock::now();
        if (now - start >= duration) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (cook_fired.load()) break;

        std::lock_guard<std::mutex> lock(mtx);
        if (cook_fired.load()) break;

        for (int i = 0; i < 3; ++i) {
            plates[i] += efficiency_factor;
        }
    }
    time_is_up = true; // даём сигнал толстякам остановиться
}

void run_simulation(int gluttony, int efficiency_factor, const std::string& name) {
    //сброс параметров
    plates = { 3000, 3000, 3000 };
    cook_fired = false;
    time_is_up = false;
    exploded_count = 0;
    for (int i = 0; i < 3; ++i) has_exploded[i] = false;
    for (int i = 0; i < 3; ++i) fatmen_eaten[i] = 0;
    

    std::cout << "\n=== " << name << " ===\n";
    std::cout << "gluttony = " << gluttony << ", efficiency_factor = " << efficiency_factor << "\n";

    std::thread t_cook(cook, efficiency_factor);
    std::thread t1(fatman, 0, gluttony);
    std::thread t2(fatman, 1, gluttony);
    std::thread t3(fatman, 2, gluttony);

    t_cook.join();
    t1.join();
    t2.join();
    t3.join();

    // Определяем результат
    if (cook_fired) {
        std::cout << "Результат: Кука уволили\n";
    }
    else if (exploded_count == 3) {
        std::cout << "Результат: Кук не получил зарплату\n";
    }
    else {
        std::cout << "Результат: Кук уволился сам\n";
    }

    std::cout << "На тарелках: ";
    for (int p : plates) std::cout << p << " ";
    std::cout << "\nСъедено: ";
    for (int i = 0; i < 3; i++) std::cout << fatmen_eaten[i] << " ";
    std::cout << "\nВзорвалось: " << exploded_count << "\n";
}

int main() {
    setlocale(LC_ALL, "RU");

    // Условие 1: Увольнение - толстяки едят быстрее чем повар успевает пополнять
    run_simulation(500, 50, "Условие 1: Увольнение");

    // Условие 2: Нет зарплаты - некому платить, потому что толстяки лопнули))
    run_simulation(300, 800, "Условие 2: Кук не получил зарплату");

    // Условие 3: Увольняется сам, т.к. за 5 дней (5 сек компа) никто не лопнул и тарелки не опустели
    run_simulation(200, 500, "Условие 3: Увольняется сам");

    return 0;
}