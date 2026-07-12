#include <iostream>
#include <thread>
#include <string>
#include <thread>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mu;
std::condition_variable cv;
std::deque<int> buffer;
const unsigned int maxBufferSize = 20;

void producer(int num)
{
    while (num)
    {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, []()
                { return buffer.size() < maxBufferSize; });
        buffer.push_back(num);
        std::cout << "produced: " << num << std::endl;
        num--;
        lock.unlock();//让被唤醒的线程直接拿到锁
        cv.notify_one();
    }
}

void consumer()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, []()
                { return !buffer.empty(); });
        int num = buffer.front();
        buffer.pop_front();
        std::cout << "consumed: " << num << std::endl;
        lock.unlock();
        cv.notify_one();
    }
}

int main()
{
    std::thread prod(producer,30);
    std::thread cons(consumer);
    prod.join();
    cons.join();
    return 0;
}

//sleep和wait有什么区别