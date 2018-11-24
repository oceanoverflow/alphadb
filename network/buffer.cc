#include "buffer.h"

buffer::buffer(size_t size): full_{false}, head_{0}, tail_{0}, max_size_{size}
{
    buf_ = new char[max_size_];
    memset(&buf_, 0, max_size_);
}

buffer::~buffer() 
{
    delete buf_;
    buf_ = nullptr;
}

void buffer::advance_get(size_t s)
{
    std::lock_guard<std::mutex> lock(mutex_);
    tail_ = (tail_ + s) % max_size_;
}

void buffer::advance_put(size_t s)
{
    std::lock_guard<std::mutex> lock(mutex_);
    head_ = (head_ + s) % max_size_;
}

char* buffer::put_begin() const
{  
    return buf_ + head_;
}

char* buffer::get_begin() const
{  
    return buf_ + tail_;
}

void buffer::put(char c)
{
    std::lock_guard<std::mutex> lock(mutex_);

    buf_[head_] = c;

    if (full_)
        tail_ = (tail_ + 1) % max_size_;

    head_ = (head_ + 1) % max_size_;

    full_ = head_ == tail_;
}

void buffer::put(const char* s, size_t len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (max_size_ - size() < len) {
        // resize
    }

    memcpy(buf_ + head_, s, len);
    head_ = (head_ + len) % max_size_;
}

char buffer::get()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (empty())
        return char();

    char val = buf_[tail_];
    full_ = false;
    tail_ = (tail_ + 1) % max_size_;

    return val;
}

void buffer::get(char* s, size_t len)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (size() < len)
        return;
    full_ = false;    
    memcpy(s, buf_ + tail_, len);
    tail_ = (tail_ + len) % max_size_;
}

void buffer::unget(size_t s)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (full() || size() < s)
        return;

    tail_ = (tail_ - s) % max_size_;
}

void buffer::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    head_ = tail_;
    full_ = false;
}

bool buffer::empty() const
{
    // if head and tail are equal, we are empty
    return !full_ && head_ == tail_;
}

bool buffer::full() const
{
    // if tail is ahead the head by 1, we are full
    return full_;
}

size_t buffer::capacity() const
{
    return max_size_;
}

size_t buffer::size() const
{
    size_t size = max_size_;
    
    if (!full_) {
        if (head_ >= tail_)
            size = head_ - tail_;
        else
            size = max_size_ - (tail_ - head_);
    }
    
    return size;
}

void myprint(std::ostream& os, char c, size_t size)
{
    for(size_t i = 0; i < size; i++)
        os << c;
}

std::ostream& operator<<(std::ostream& os, const buffer& buf)
{   
    int cap = buf.max_size_ / 100;
    int head = buf.head_ / 100;
    int tail = buf.tail_ / 100;

    os << std::endl;
    os << "cap:" << buf.max_size_ << " " << "size:" << buf.size() << " " << "head:" << buf.head_ << " " << "tail:" << buf.tail_ << std::endl;
    os << '|'; myprint(os, '-', cap); os << "|" << std::endl;
    
    if (buf.empty() || head == tail) {
        myprint(os, ' ', tail); os << '^'; os << std::endl;
        myprint(os, ' ', tail); os << '|'; os << std::endl;
        myprint(os, ' ', tail); os << '+'; myprint(os, '-', 3); os << "h=" << buf.head_ << std::endl;
        myprint(os, ' ', tail); os << '|'; os << std::endl;
        myprint(os, ' ', tail); os << '+'; myprint(os, '-', 3); os << "t=" << buf.tail_ << std::endl;
        return os;
    }
    
    if (head > tail) {
        myprint(os, ' ', tail); os << '^'; myprint(os, ' ', head-tail-1); os << '^' << std::endl;
        myprint(os, ' ', tail); os << '|'; myprint(os, ' ', head-tail-1); os << '|' << std::endl;
        myprint(os, ' ', tail); os << 'v'; myprint(os, ' ', head-tail-1); os << '^' << std::endl;
        myprint(os, ' ', tail); os << 't'; myprint(os, '-', head-tail-1); os << 'h' << std::endl;
    }
    else {
        os << '|'; myprint(os, ' ', head); os << '^'; myprint(os, ' ', tail-head-2); os << '^'; myprint(os, ' ', cap - tail); os << '|' << std::endl;
        os << '|'; myprint(os, ' ', head); os << '|'; myprint(os, ' ', tail-head-2); os << '|'; myprint(os, ' ', cap - tail); os << '|' << std::endl;
        os << '|'; myprint(os, ' ', head); os << '^'; myprint(os, ' ', tail-head-2); os << 'v'; myprint(os, ' ', cap - tail); os << '|' << std::endl;
        os << '+'; myprint(os, '-', head); os << 'h'; myprint(os, ' ', tail-head-2); os << 't'; myprint(os, '-', cap - tail); os << '+' << std::endl;
    }
    os << std::endl;
    return os;
}