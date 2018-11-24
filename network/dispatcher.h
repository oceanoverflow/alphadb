#pragma once

#include "poller.h"
#include "thread_pool.h"

#include <map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <utility>
#include <functional>

enum class timeunit {
    unit,
    milli,
    micro,
    nano
};

static int64_t get_current_time(timeunit unit)
{
    int64_t val;
    std::chrono::time_point<std::chrono::system_clock> p = std::chrono::system_clock::now();
    switch (unit)
    {
        case timeunit::unit:
            val = std::chrono::duration_cast<std::chrono::seconds>(p.time_since_epoch()).count();
            break;
        case timeunit::milli:
            val = std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch()).count();
            break;
        case timeunit::micro:
            val = std::chrono::duration_cast<std::chrono::microseconds>(p.time_since_epoch()).count();
            break;
        case timeunit::nano:
            val = std::chrono::duration_cast<std::chrono::nanoseconds>(p.time_since_epoch()).count();
            break;
        default:
            break;
    }
    return val;
}

/********************************************************************************************************
                        
                        +-------------+----------+--------+-----+-----+-----+-----+-----+-----+-----+
                        v             |          |        |     |     |     |     |     |     |     |
                       now     next_timeout:     v        v     v     v     v     v     v     v     v
timeline: (unit:ms)-----x-------------x----------x--------x-----x-----x-----x-----x-----x-----x-----x---> 
                                      ^          ^        ^     ^     ^     ^     ^     ^     ^     ^    
                                      |          |        |     |     |     |     |     |     |     |
                                      |          |        |     |     |     |     |     |     |     | 
                                      |          |        |     |     |     |     |     |     |     | 
                                      |          |        |     |     |     |     |     |     |     |
+^^^^^^^^^^+        (run_after^)      |          |        |     |     |     |     |     |     |     |  
|dispatcher|--------------------------+          |        |     |     |     |     |     |     |     |
+----------+                                     |        |     |     |     |     |     |     |     |  
      |  |                                       |        |     |     |     |     |     |     |     | 
      |  |           (run_at*)                   |        |     |     |     |     |     |     |     |  
      |  +---------------------------------------+        |     |     |     |     |     |     |     |  
      |             (run_every^)                          |     |     |     |     |     |     |     |  
      +---------------------------------------------------+-----+-----+-----+-----+-----+-----+-----+

      basic constructure: 
        run_at*
        now = get_current_time(timeunit::milli)
      run_after^ = run_at(now + duration)
      run_every^ = run_at(now + duration, interval)

**********************************************************************************************************/

using task = std::function<void()>;
// if first is negative, means it is a repeatable timer
// first is milli time point, second is time_seq which is guaranted to be unique
typedef std::pair<int64_t, int64_t> timer_id_t;

struct repeatable_task {
    int64_t    at;
    int64_t    interval;
    timer_id_t timer_id;
    task       callback;
};

const int MAX_TIMEOUTS{1 << 30};
const int DEFAULT_TIMEOUTS{10000};

class dispatcher
{
private:
    std::atomic<bool>    working_;
    int                  wake_up_[2];
    poller*              poller_;
    channel*             channel_;
    thread_pool*         thread_pool_;

    std::atomic<int64_t> time_seq_;
    int                  next_timeout_;
    
    std::mutex                            mutex_;
    std::map<timer_id_t, task>            timers_;
    std::map<timer_id_t, repeatable_task> repeatable_timers_;
private:
    void wakeup();
    void handle_timeout();
    void update_next_timeout();
    void repeat(repeatable_task* rt);
public:
    dispatcher(int threads);
    ~dispatcher();

    poller* poller() const;
    
    void loop();
    void loop_once(int ms);
    
    // absolute time: timepoint = get_current_time()
    timer_id_t run_at(int64_t tp, task&& t, int64_t interval = 0);
    timer_id_t run_at(int64_t tp, task& t, int64_t interval = 0);

    // relative time: time duration = ms
    timer_id_t run_after(int64_t ms, task&& t);
    timer_id_t run_after(int64_t ms, task& t);

    // relative time: time duration = ms
    timer_id_t run_every(int64_t ms, int64_t interval, task&& t);
    timer_id_t run_every(int64_t ms, int64_t interval, task& t);

    // TODO
    void reschedule(timer_id_t& tid, int64_t ms, task&& t);

    // cancel the timer or repeatable timer
    bool cancel(const timer_id_t& tid);
    void exit();
};