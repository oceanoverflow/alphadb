#pragma once

#include <mutex>
#include <iostream>
#include <algorithm>

/*
                   circular buffer
buffer is a generic term that refers to a block of memory
that servers as a temporary placeholder. You might encounter
the term in your computer, which uses RAM as a buffer, or in
video streaming where a section of the movie you are streaming
downloads to your device to stay ahead of your viewing.

In computer programming, data can be placed in a software buffer
before it is processed. Because writing data to a buffer is much 
faster than a direct operation, using a buffer while programming
in C and C++ makes a lot of sense and speeds up the calculation
process. Buffers come in handy when a difference exists between
the rate data is received and the rate it is processed.

Buffer v.s. Cache
A buffer is temporary storage of data that is on its way to other
media or storage of data that can be modified non-sequentially 
before it is read sequentially. It attempts to reduce the difference
between input speed and output speed, A cacge also acts as a buffer,
but it stores data that is expected to be read several times to
reduce the need to access slower storage.

due to the resource constrained nature of embedded system, circular
buffer data structures can be found in most projects.

circular buffers (also known as ring buffers) are fixed-size buffers
that work as if the memory is contiguous & circular in nature. As memory
is generated and consumed, data does not need to be reshuffled- rather,
the head/tail pointers are adjusted. when data is added, the head pointer
advances. when data is consumed, the tail pointer advances. If you reach 
the end of the buffer, the pointers simply wrap around to the beginning.

circular buffers are often used as fixed-sized queues. The fixed size is
beneficial for embedded systems, as developers often try to use static
data storage methods rather than dynamic allocations.

circular buffers are also useful structures for situations where data
production and consumption happen at different rates: the most rent data
is always available. If the consumer cannot keep up with production, the
stale data will be overwritten with more recent data. By using a circular
buffer, we can ensure that we are always consuming the most recent data.

  |-----------------------------------------|  max_size_ = 4096
  ^
  |
  +--- head_ = 0
  | 
  +--- tail_ = 0 

         write at head, read at tail
    
  |-----------------------------------------|  max_size_ = 4096
      ^                        ^
      |                        |
      +-tail_            head_-+
      v                        ^
      +----------size----------+

  |-----------------------------------------|  max_size_ = 4096
  |         ^                  ^            |
  |         |                  |            |
  |   head_-+                  +-tail_      |
  |         ^                  v            |
  +--size1--+                  +----size0---+
*/

const int DEFAULT_BUFFER_SIZE{0xFFFF};

class buffer
{
private:
    std::mutex mutex_;
    char* buf_;
    bool full_;
    size_t head_;
    size_t tail_;
    size_t max_size_;
public:
    buffer(size_t size = DEFAULT_BUFFER_SIZE);
    ~buffer();

    friend std::ostream& operator<<(std::ostream& os, const buffer& buf);

    void advance_put(size_t s);
    void advance_get(size_t s);

    char* put_begin() const;
    char* get_begin() const;
    // adding data
    void put(char c);
    void put(const char* s, size_t len);
    // removing data
    char get();
    void get(char* s, size_t len);
    // char* get(size_t s);
    void unget(size_t s = 1);
    // resetting the buffer to empty
    void reset();
    // check empty state
    bool empty() const;
    // check full state
    bool full() const;
    // checking the total capacity of the buffer
    size_t capacity() const;
    // checking the current number of elements in the buffer
    size_t size() const;
};
