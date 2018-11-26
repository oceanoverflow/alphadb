#include "schema.h"
#include <string.h>

schema::schema() {}

schema::~schema() {
    for(auto&& i : col_defs_)
    {
        free(i.name);
    }
}

void schema::add_column(char* name, column_type type, bool nullable)
{
    column_definition col_def(name, type, nullable);
    int id = col_defs_.size();
    col_defs_.push_back(col_def);
    col_name_to_id_[name] = id; 
}