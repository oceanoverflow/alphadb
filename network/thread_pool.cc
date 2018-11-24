#include "thread_pool.h"

thread_pool::thread_pool(int threads): working_{true}
{
    
    for(int i = 0; i < threads; i++)
    {
        workers_.push_back(std::thread([this] {
            for(;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this]{ return !working_ || !tasks_.empty(); });
                    if(!working_ && tasks_.empty())
                        return;
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        }));
    }
   
}

template<class F, class... Args>
auto thread_pool::enqueue(F&& f, Args&&... args) 
-> std::future<typename std::result_of<F(Args...)>::type> 
{
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(!working_)
            throw std::runtime_error("enqueue on stopped thread pool");
        tasks_.emplace([task]{ (*task)(); });
    }
    condition_.notify_one();
    return res;
}

thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        working_ = false;
    }
    condition_.notify_all();
    
    for(auto& worker : workers_)
        worker.join();
}
