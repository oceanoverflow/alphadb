#include "iterator.h"

#include <string.h>

criterion::criterion(void* val, column_type coltype, criterion_type type)
: val{val}, coltype{coltype}, type{type} {}

criterion::~criterion() {}

bool criterion::eval(char* buf) const
{
    return internal_eval(buf, type);
}

bool criterion::internal_eval(char* buf, criterion_type critype) const
{
    if (critype == criterion_type::LESS_THAN) {
        if (coltype.type == data_type::INT) {
            return *reinterpret_cast<int *>(buf) < *reinterpret_cast<int *>(val);
        }
        
        if (coltype.type == data_type::LONG) {
            return *reinterpret_cast<long *>(buf) < *reinterpret_cast<long *>(val);
        }
        
        if (coltype.type == data_type::FLOAT) {
            return *reinterpret_cast<float *>(buf) < *reinterpret_cast<float *>(val);
        }
        
        if (coltype.type == data_type::DOUBLE) {
            return *reinterpret_cast<double *>(buf) < *reinterpret_cast<double *>(val);
        }

        if (coltype.type == data_type::CHAR) {
            return reinterpret_cast<char>(buf) < reinterpret_cast<char>(val);
        }
 
        if (coltype.type == data_type::VARCHAR) {
            return strncmp(buf, static_cast<char *>(val), coltype.length) < 0;
        }
    }
    
    if (critype == criterion_type::GREATER_THAN) {
        if (coltype.type == data_type::INT) {
            return *reinterpret_cast<int *>(buf) > *reinterpret_cast<int *>(val);
        }
        
        if (coltype.type == data_type::LONG) {
            return *reinterpret_cast<long *>(buf) > *reinterpret_cast<long *>(val);
        }
        
        if (coltype.type == data_type::FLOAT) {
            return *reinterpret_cast<float *>(buf) > *reinterpret_cast<float *>(val);
        }
        
        if (coltype.type == data_type::DOUBLE) {
            return *reinterpret_cast<double *>(buf) > *reinterpret_cast<double *>(val);
        }

        if (coltype.type == data_type::CHAR) {
            return reinterpret_cast<char>(buf) > reinterpret_cast<char>(val);
        }
 
        if (coltype.type == data_type::VARCHAR) {
            return strncmp(buf, static_cast<char *>(val), coltype.length) > 0;
        }
    }

    if (critype == criterion_type::EQUAL) {
        if (coltype.type == data_type::INT) {
            return *reinterpret_cast<int *>(buf) == *reinterpret_cast<int *>(val);
        }
        
        if (coltype.type == data_type::LONG) {
            return *reinterpret_cast<long *>(buf) == *reinterpret_cast<long *>(val);
        }
        
        if (coltype.type == data_type::FLOAT) {
            return *reinterpret_cast<float *>(buf) == *reinterpret_cast<float *>(val);
        }
        
        if (coltype.type == data_type::DOUBLE) {
            return *reinterpret_cast<double *>(buf) == *reinterpret_cast<double *>(val);
        }

        if (coltype.type == data_type::CHAR) {
            return reinterpret_cast<char>(buf) == reinterpret_cast<char>(val);
        }
 
        if (coltype.type == data_type::VARCHAR) {
            return strncmp(buf, static_cast<char *>(val), coltype.length) == 0;
        }
    }
    
    if (critype == criterion_type::GREATER_EQUAL) {
        return this->internal_eval(buf, criterion_type::GREATER_THAN) || this->internal_eval(buf, criterion_type::EQUAL);
    }

    if (critype == criterion_type::LESS_EQUAL) {
        return this->internal_eval(buf, criterion_type::LESS_THAN) || this->internal_eval(buf, criterion_type::EQUAL);
    }
    
} 