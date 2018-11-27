#include "iterator.h"

#include <string.h>

criterion::criterion(void* val, column_definition coldef, criterion_type type)
: val{val}, coldef{coldef}, type{type} {}

criterion::~criterion() {}

bool criterion::eval(char* buf) const
{
    return internal_eval(buf, type);
}

bool criterion::internal_eval(char* buf, criterion_type critype) const
{
    data_type datatype = coldef.type.type;
    if (critype == criterion_type::LESS_THAN) {
        if (datatype == data_type::INT) {
            return *reinterpret_cast<int *>(buf) < *reinterpret_cast<int *>(val);
        }
        
        if (datatype == data_type::LONG) {
            return *reinterpret_cast<long *>(buf) < *reinterpret_cast<long *>(val);
        }
        
        if (datatype == data_type::FLOAT) {
            return *reinterpret_cast<float *>(buf) < *reinterpret_cast<float *>(val);
        }
        
        if (datatype == data_type::DOUBLE) {
            return *reinterpret_cast<double *>(buf) < *reinterpret_cast<double *>(val);
        }

        if (datatype == data_type::CHAR) {
            return reinterpret_cast<char>(buf) < reinterpret_cast<char>(val);
        }
 
        if (datatype == data_type::VARCHAR) {
            return strncmp(buf, static_cast<char *>(val), coldef.type.length) < 0;
        }
    }
    
    if (critype == criterion_type::GREATER_THAN) {
        if (datatype == data_type::INT) {
            return *reinterpret_cast<int *>(buf) > *reinterpret_cast<int *>(val);
        }
        
        if (datatype == data_type::LONG) {
            return *reinterpret_cast<long *>(buf) > *reinterpret_cast<long *>(val);
        }
        
        if (datatype == data_type::FLOAT) {
            return *reinterpret_cast<float *>(buf) > *reinterpret_cast<float *>(val);
        }
        
        if (datatype == data_type::DOUBLE) {
            return *reinterpret_cast<double *>(buf) > *reinterpret_cast<double *>(val);
        }

        if (datatype == data_type::CHAR) {
            return reinterpret_cast<char>(buf) > reinterpret_cast<char>(val);
        }
 
        if (datatype == data_type::VARCHAR) {
            return strncmp(buf, static_cast<char *>(val), coldef.type.length) > 0;
        }
    }

    if (critype == criterion_type::EQUAL) {
        if (datatype == data_type::INT) {
            return *reinterpret_cast<int *>(buf) == *reinterpret_cast<int *>(val);
        }
        
        if (datatype == data_type::LONG) {
            return *reinterpret_cast<long *>(buf) == *reinterpret_cast<long *>(val);
        }
        
        if (datatype == data_type::FLOAT) {
            return *reinterpret_cast<float *>(buf) == *reinterpret_cast<float *>(val);
        }
        
        if (datatype == data_type::DOUBLE) {
            return *reinterpret_cast<double *>(buf) == *reinterpret_cast<double *>(val);
        }

        if (datatype == data_type::CHAR) {
            return reinterpret_cast<char>(buf) == reinterpret_cast<char>(val);
        }
 
        if (datatype == data_type::VARCHAR) {
            return strncmp(buf, static_cast<char *>(val), coldef.type.length) == 0;
        }
    }
    
    if (critype == criterion_type::GREATER_EQUAL) {
        return this->internal_eval(buf, criterion_type::GREATER_THAN) || this->internal_eval(buf, criterion_type::EQUAL);
    }

    if (critype == criterion_type::LESS_EQUAL) {
        return this->internal_eval(buf, criterion_type::LESS_THAN) || this->internal_eval(buf, criterion_type::EQUAL);
    }
    
} 