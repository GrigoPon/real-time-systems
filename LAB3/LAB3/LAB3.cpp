#include <iostream>
#include <thread>
#include <mutex>
#include <string>


int coins = 100;
int all = coins;
int Bob_coins = 0;
int Tom_coins = 0;

std::mutex mtx;

void coins_sharing(std::string name, int& thief_coins, int& companion_coins) {
    while (true) {
        bool took_coin = false;
        bool finished = false;

        {
            std::lock_guard<std::mutex> lock(mtx);
            //монеты закончились
            if (coins <= 0) {
                finished = true;
            }
            //отдать покойнику (покойник получает только при нечетном количестве монет)
            else if (all % 2 != 0 && coins == 1) {
                std::cout << "Last coin goes to the grave. Coins left: " << coins << std::endl;
                coins = 0;
                finished = true;
            }
            //дележка
            else if (thief_coins <= companion_coins) {
                coins--;
                thief_coins++;
                std::cout << name << ": " << thief_coins << " vs " << companion_coins
                    << " | coins left: " << coins << std::endl;
                took_coin = true;
            }
        }
        //проверка закончились ли монеты (грубо говоря)
        if (finished) {
            break;
        }

        if (!took_coin) {

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

int main() {
    std::thread bob(coins_sharing, "Bob", std::ref(Bob_coins), std::ref(Tom_coins));
    std::thread tom(coins_sharing, "Tom", std::ref(Tom_coins), std::ref(Bob_coins));

    bob.join();
    tom.join();

    std::cout << "\nFinal result:\n";
    std::cout << "Bob: " << Bob_coins << std::endl;
    std::cout << "Tom: " << Tom_coins << std::endl;
    std::cout << "Grave: " << (all - Bob_coins - Tom_coins) << std::endl;

    return 0;
}