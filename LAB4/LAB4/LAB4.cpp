#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

// Глобальные переменные
std::mutex table_mutex;
std::atomic<bool> stop_simulation{ false };
std::atomic<int> total_nuggets_eaten[3]{ 0 }; // Сколько съел каждый толстяк
int dish[3] = { 3000, 3000, 3000 }; // начальные тарелки
std::atomic<int> fatmen_destroyed{ 0 };

// Константы, задаваемые перед запуском
int gluttony = 3;
int efficiency_factor = 3;

// Флаги состояния
std::atomic<int> eaters_done_count{ 0 };
std::atomic<bool> cook_just_added{ false };
std::atomic<bool> eaters_finished{ false };

void cook_thread() {
    auto start_time = std::chrono::steady_clock::now();
    while (!stop_simulation) {
        {
            std::lock_guard<std::mutex> lock(table_mutex);
            int per_plate = efficiency_factor / 3;
            int remainder = efficiency_factor % 3;
            for (int i = 0; i < 3; ++i) {
                dish[i] += per_plate + (i < remainder ? 1 : 0);
            }
            cook_just_added = true;
            eaters_finished = false;
            eaters_done_count = 0; // сброс перед новым раундом
        }

        // Ждём, пока активные толстяки поедят (ограничено по времени)
        auto wait_start = std::chrono::steady_clock::now();
        while (!eaters_finished && !stop_simulation) {
            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - wait_start).count() > 5) {
                // Защита от зависания
                break;
            }
            std::this_thread::yield();
        }
        cook_just_added = false;

        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time).count() >= 5) {
            // Не ставим stop_simulation сразу — дадим завершиться текущему раунду
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // После выхода из цикла — сигнал к завершению
    stop_simulation = true;
}

void fatman_thread(int id) {
    while (!stop_simulation) {
        // Ждём, пока Кук выложит (если повар ещё работает)
        while (!cook_just_added && !stop_simulation) {
            std::this_thread::yield();
        }
        if (stop_simulation) break;

        bool destroyed_or_starved = false;
        {
            std::lock_guard<std::mutex> lock(table_mutex);
            if (dish[id] >= gluttony) {
                dish[id] -= gluttony;
                total_nuggets_eaten[id] += gluttony;
                if (total_nuggets_eaten[id] >= 10000) {
                    std::cout << "Толстяк " << id + 1 << " самоуничтожился!\n";
                    fatmen_destroyed++;
                    destroyed_or_starved = true;
                }
            }
            else {
                std::cout << "У толстяка " << id + 1 << " закончились наггетсы! Кука уволили.\n";
                destroyed_or_starved = true;
            }
        }

        if (destroyed_or_starved) {
            // Этот толстяк больше не участвует
            return;
        }

        // Учёт завершения еды этим толстяком
        int prev = eaters_done_count.fetch_add(1);
        if (prev == 2) {
            eaters_done_count = 0;
            eaters_finished = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void run_simulation(int g, int e, const std::string& scenario_name) {
    // Сброс глобальных переменных
    stop_simulation = false;
    fatmen_destroyed = 0;
    for (int i = 0; i < 3; ++i) {
        total_nuggets_eaten[i] = 0;
        dish[i] = 3000;
    }
    cook_just_added = false;
    eaters_finished = false;

    gluttony = g;
    efficiency_factor = e;

    std::cout << "\n=== " << scenario_name << " ===\n";
    std::cout << "gluttony = " << gluttony << ", efficiency_factor = " << efficiency_factor << "\n";

    std::thread cook(cook_thread);
    std::thread fatmen[3] = {
        std::thread(fatman_thread, 0),
        std::thread(fatman_thread, 1),
        std::thread(fatman_thread, 2)
    };

    // Ждём завершения симуляции (по любому условию)
    while (!stop_simulation) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Завершаем потоки
    cook.join();
    for (int i = 0; i < 3; ++i) {
        fatmen[i].join();
    }

    // Теперь определяем результат ПОСЛЕ полного завершения
    bool someone_starved = false;
    for (int i = 0; i < 3; ++i) {
        if (total_nuggets_eaten[i] < 10000 && dish[i] < gluttony) {
            someone_starved = true;
        }
    }

    if (fatmen_destroyed == 3) {
        std::cout << "РЕЗУЛЬТАТ: Кук не получил зарплату (все толстяки самоуничтожились).\n";
    }
    else if (someone_starved) {
        std::cout << "РЕЗУЛЬТАТ: Кука уволили (закончились наггетсы у кого-то).\n";
    }
    else {
        std::cout << "РЕЗУЛЬТАТ: Кук уволился сам (прошло 5 дней).\n";
    }

    // Вывод итогов по каждому толстяку
    for (int i = 0; i < 3; ++i) {
        std::cout << "Толстяк " << i + 1 << " съел: " << total_nuggets_eaten[i] << " наггетсов\n";
    }
}

int main() {
    setlocale(LC_ALL, "RU");
    run_simulation(100, 10, "Сценарий 1: Кука уволили");
    run_simulation(100, 1000, "Сценарий 2: Кук не получил зарплату");
    run_simulation(20, 40, "Сценарий 3: Кук уволился сам");

    return 0;
}