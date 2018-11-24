#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include <future>
#include <condition_variable>

class thread_pool
{
private:
    bool working_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
public:
    thread_pool(int threads);
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>;

    ~thread_pool();
};