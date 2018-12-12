#include "row.h"

#include <string>
#include <utility>

char* row::encode(std::vector<char *> values, const schema& s)
{ 
    char* ret;
    for(int i = 0; i < values.size(); i++)
    {
        if (values[i] == nullptr) continue;
        char* cs = encode_column(values[i], static_cast<std::underlying_type<column_type>::type>(s.col_defs_[i].type), i);
        ret = strcat(ret, cs);
    }
    return ret;
}

char* row::encode_column(char* data, int type, column_id_t id)
{
    int len = 1;
    switch (type)
    {
        case 1:
            len += sizeof(int);
            break;
        case 2:
            len += sizeof(long);
            break;
        case 3:
            len += sizeof(float);
            break;
        case 4:
            len += sizeof(double);
            break;
        case 5:
            len += sizeof(char);
            break;
        case 6:
            len += sizeof(int) + strlen(data);
            break;
        default:
            break;
    }
    char* buf = new char[len];

    char* dst = buf;
    char b = static_cast<char>(id >> 3 | type);
    memcpy(dst++, &b, 1);
    switch (type)
    {
        case 1:
            memcpy(dst, data, sizeof(int));
            break;
        case 2:
            memcpy(dst, data, sizeof(long));
            break;
        case 3:
            memcpy(dst, data, sizeof(float));
            break;
        case 4:
            memcpy(dst, data, sizeof(double));
            break;
        case 5:
            memcpy(dst, data, sizeof(char));
            break;
        case 6:
            int len = strlen(data);
            memcpy(dst, &len, sizeof(int));
            dst += sizeof(int);
            memcpy(dst, data, len);
            break;
        default:
            break;
    }
    return buf;
}

std::vector<void *> row::decode(char* ptr, const schema& s)
{
    std::vector<void *> vs;
    vs.reserve(s.col_defs_.size());
    while(*ptr != 0x00){
        auto pair = decode_column(ptr);
        vs[pair.first] = pair.second; 
    }
    return vs;
}

std::pair<int, void *> row::decode_column(char* ptr)
{
    char b = *ptr++;
    int column_id = b >> 3;
    int type = b | 0x07;

    switch (type)
    {
        case 1:
            int val = *reinterpret_cast<int *>(ptr);
            ptr += sizeof(int);
            return {column_id, (void *)val};
        case 2:
            long val = *reinterpret_cast<long *>(ptr);
            ptr += sizeof(long);
            return {column_id, (void *)val};
        case 3:
            float val = *reinterpret_cast<float *>(ptr);
            ptr += sizeof(float);
            return {column_id, (void *)val};
        case 4:
            double val = *reinterpret_cast<double *>(ptr);
            ptr += sizeof(double);
            return {column_id, (void *)val};
        case 5:
            char val = *ptr++;
            return {column_id, (void *)val};
        case 6:
            int len = *reinterpret_cast<int *>(ptr);
            ptr += sizeof(int);
            char* str = new char[len];
            for(size_t i = 0; i < len; i++)
                 str[i] = *ptr++;
            
            return {column_id, (void *)(str)};
        default:
            break;
    }
}