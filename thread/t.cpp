#include <iostream>
#include <thread>
#include <string>
#include <iostream>
#include <thread>

int counter = 0;           // 共享全局变量

void increment() {
    for (int i = 0; i < 100000; ++i) {
        ++counter;          // 看似一行，实则三步
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();
    std::cout << counter << "\n";  // 期望 200000
    return 0;
}