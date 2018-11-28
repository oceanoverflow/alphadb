#include "comparator.h"
#include <cstring>
#include <algorithm>

comparator::comparator(data_type type): data_type_{type} {}

comparator::~comparator() {}

int comparator::compare(tuple_t t1, tuple_t t2) const
{ 
    if (data_type_ == data_type::INT) {
        if (*reinterpret_cast<int *>(t1.first) < *reinterpret_cast<int *>(t2.first)) return -1;
        if (*reinterpret_cast<int *>(t1.first) == *reinterpret_cast<int *>(t2.first)) return 0;
        if (*reinterpret_cast<int *>(t1.first) > *reinterpret_cast<int *>(t2.first)) return 1;
    }

    if (data_type_ == data_type::LONG) {
        if (*reinterpret_cast<long *>(t1.first) < *reinterpret_cast<long *>(t2.first)) return -1;
        if (*reinterpret_cast<long *>(t1.first) == *reinterpret_cast<long *>(t2.first)) return 0;
        if (*reinterpret_cast<long *>(t1.first) > *reinterpret_cast<long *>(t2.first)) return 1;
    }

    if (data_type_ == data_type::FLOAT) {
        if (*reinterpret_cast<float *>(t1.first) < *reinterpret_cast<float *>(t2.first)) return -1;
        if (*reinterpret_cast<float *>(t1.first) == *reinterpret_cast<float *>(t2.first)) return 0;
        if (*reinterpret_cast<float *>(t1.first) > *reinterpret_cast<float *>(t2.first)) return 1;
    }

    if (data_type_ == data_type::DOUBLE) {
        if (*reinterpret_cast<double *>(t1.first) < *reinterpret_cast<double *>(t2.first)) return -1;
        if (*reinterpret_cast<double *>(t1.first) == *reinterpret_cast<double *>(t2.first)) return 0;
        if (*reinterpret_cast<double *>(t1.first) > *reinterpret_cast<double *>(t2.first)) return 1;
    }
  
    if (data_type_ == data_type::CHAR) {
        if (reinterpret_cast<char>(t1.first) < reinterpret_cast<char>(t2.first)) return -1;
        if (reinterpret_cast<char>(t1.first) == reinterpret_cast<char>(t2.first)) return 0;
        if (reinterpret_cast<char>(t1.first) > reinterpret_cast<char>(t2.first)) return 1;
    }
    
    if (data_type_ == data_type::VARCHAR) {
        return strncmp(static_cast<char*>(t1.first), static_cast<char*>(t2.first), std::min(t1.second, t2.second));
    }

    return 0;
}