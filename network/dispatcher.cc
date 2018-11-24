#include "net.h"
#include "channel.h"
#include "dispatcher.h"

#include <unistd.h>

#include <algorithm>

/*
The self-pipe trick (from the linux programming interface 63.5.2)

Since pselect() is not widely implemented, portable applications must employ other
strategies to avoid race conditions when simultaneously waiting for signal and call-
ing select() on a set of file descriptors. One common solution is the following.

1. Create a pipe, and mark its read and write ends as nonblocking.
2. As well as monitoring all of the other file descriptors that are of interest,
   include the read end of the pipe in the readfds set given to select().
3. Install a handler for the signal that is of interest. When this signal handler is
   called, it writes a byte of data to the pipe. Note the following points about the
   signal handler:
   - The write end of the pipe was marked as nonblocking in the first step to
     prevent the possibility that signals arrive so rapidly that repeated invocations
     of the signal handler fill the pipe, with the result that the signal handler's
     write() (and thus the process itself) si blocked. (It doesn't matter if a write
     to a full pipe fails, since the previous writes will already have indicated the
     delivery of the signal.)
   - The signal handler is installed after creating the pipe, in order to prevent
     the race condition that would occur if a signal was delivered before the
     pipe was created.
   - It is safe to use write() inside the signal handler, because it is one of the
     async-signal-safe functions listed...
4. Place the select() call in a loop, so that it is restarted if interrupted by a signal
   handler. (Restarting in this fashion is not strictly necessary; it merely means
   that we can check for the arrival of a signal by inspecting readfds, rather than
   checking for an EINTR error return.)
5. On successful completion of the select() call, we can determine whether a signal
   arrived by checking if the file descriptors for the read end of the pipe is set in
   readfds.
6. Whenever a signal has arrived, read all bytes that are in the pipe. Since multiple
   signals may arrive, employ a loop that reads bytes until the (nonblocking) read()
   fails with the error EAGAIN. After draining the pipe, perform whatever actions
   must be taken in response to delivery of the signal.

This technique is commonly known as the self-pipe trick, variations on this technique 
can equally be employed with poll() and epoll_wait().
*/

dispatcher::dispatcher(int threads)
: working_(true), time_seq_(0), next_timeout_(MAX_TIMEOUTS)
{
#ifdef __APPLE__
    poller_ = new kqueue_poller();
#elif defined(__linux__)
    poller_ = new epoll_poller();
#endif

    pipe(wake_up_);
    set_non_blocking(wake_up_[0], true);
    channel_ = new channel(wake_up_[0], poller_);
    thread_pool_ = new thread_pool(threads);
    channel_->on_read([this]() {
        char buf;
        ssize_t r = read(channel_->fd(), &buf, sizeof(buf));
        if (r != sizeof(buf)) assert(false);
    });
}

dispatcher::~dispatcher()
{
    close(wake_up_[1]);
    delete poller_;
    delete channel_;
    delete thread_pool_;
}

poller* dispatcher::poller() const
{
    return poller_;
}

void dispatcher::loop()
{
    while(working_)
        this->loop_once(DEFAULT_TIMEOUTS);
    timers_.clear();
    repeatable_timers_.clear();
    this->loop_once(0);
}

void dispatcher::loop_once(int ms)
{
    int timeout = std::min(ms, next_timeout_);
    poller_->loop_once(timeout);
    handle_timeout();
}

void dispatcher::repeat(repeatable_task* task)
{
    task->at += task->interval;
    task->timer_id = timer_id_t{task->at, ++time_seq_};
    timers_[task->timer_id] = [this, task]() { repeat(task); };
    update_next_timeout();
    task->callback();
}

timer_id_t dispatcher::run_at(int64_t tp, task&& t, int64_t interval)
{
    if (!working_)
        return timer_id_t{0, 0};
    
    if (interval != 0) {
        timer_id_t tid{-tp, ++time_seq_};
        repeatable_task& rtask = repeatable_timers_[tid];
        rtask = repeatable_task{tp, interval, {tp, ++time_seq_}, std::move(t)};
        repeatable_task* rtaskptr = &rtask;
        timers_[rtaskptr->timer_id] = [this, rtaskptr]{ repeat(rtaskptr); };
        update_next_timeout();
        return tid;
    }
    else {
        timer_id_t tid{tp, ++time_seq_};
        timers_.insert({tid, std::move(t)});
        update_next_timeout();
        return tid;
    }
}

bool dispatcher::cancel(const timer_id_t& tid)
{    
    if (tid.first < 0) {
        auto p = repeatable_timers_.find(tid);
        auto pp = timers_.find(p->second.timer_id);
        
        if (pp != timers_.end())
            timers_.erase(pp);
        
        repeatable_timers_.erase(p);
        return true;
    }
    else {
        auto p = timers_.find(tid);
        if (p != timers_.end()) {
            timers_.erase(p);
            return true;
        }
        return false;
    }  
}

void dispatcher::exit()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        working_ = false;
    }
    this->wakeup();
}

void dispatcher::wakeup()
{
    // see the comments above
    write(wake_up_[1], "", 1);
}

void dispatcher::handle_timeout()
{
    int64_t now = get_current_time(timeunit::milli);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        while(!timers_.empty() && timers_.begin()->first < timer_id_t{now, MAX_TIMEOUTS}){
            thread_pool_->enqueue(std::move(timers_.begin()->second));
            timers_.erase(timers_.begin());
        }
    }
    update_next_timeout();
}

void dispatcher::update_next_timeout()
{
    if (timers_.empty())
        next_timeout_ = MAX_TIMEOUTS;
    else {
        timer_id_t tid = timers_.begin()->first;
        // get the current time and compare
        int next_timeout = tid.first - get_current_time(timeunit::milli);
        next_timeout_ = next_timeout <= 0 ? 0 : next_timeout;
    }
}

timer_id_t dispatcher::run_at(int64_t tp, task& t, int64_t interval)
{
    return run_at(tp, std::forward<task&&>(t), interval);
}

timer_id_t dispatcher::run_after(int64_t ms, task& t)
{
    return run_at(get_current_time(timeunit::milli) + ms, std::forward<task&&>(t), 0);
}

timer_id_t dispatcher::run_after(int64_t ms, task&& t)
{
    return run_at(get_current_time(timeunit::milli) + ms, t, 0);
}

timer_id_t dispatcher::run_every(int64_t ms, int64_t interval, task&& t)
{
    return run_at(get_current_time(timeunit::milli) + ms, std::forward<task&&>(t), interval);
}

timer_id_t dispatcher::run_every(int64_t ms, int64_t interval, task&& t)
{
    return run_at(get_current_time(timeunit::milli) + ms, t, interval);
}