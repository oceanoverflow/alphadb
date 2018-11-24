#pragma once

#include "codec.h"

#include <functional>


class rpc
{
private:
    codec codec_;
    std::function<void()> service_;
public:
    rpc();
    ~rpc();
 
    template<typename T, typename R1, typename R2>
    void set(const char* str, T* obj, std::function<void(T*, const R1&, R2&)> func);

    void set_codec(const codec& codec);


    void operator()();
    
    int hash(const char* str) const;
};
