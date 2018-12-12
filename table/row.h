#pragma once

#include "schema.h"
#include <vector>

/*
UNKNOWN  0
INT      1
LONG     2
FLOAT    3
DOUBLE   4
CHAR     5
VARCHAR  6

CREATE TABLE students (student_number INT, name VARCHAR(20), city VARCHAR(20) NULLABLE, grade FLOAT);

student_number = 21721242
name           = YIYANGYI
city           = wenzhou
grade          = 4.0

Byte Sequence: 36 bytes

+--+-----------+--+-----------+-----------------------+--+
|09|01 4b 70 9a|16|00 00 00 08|59 49 59 41 4e 47 59 49|1e|
+--+-----------+--+-----------+-----------------------+--+
-----------+--------------------+--+-----------+--+
00 00 00 07|77 65 6e 7a 68 6f 75|23|40 80 00 00|00| <- indicate end
-----------+--------------------+--+-----------+--+

break down:

  1 type(int)            21721242
+-----+---+      +--+  +-----------+
|00001|001|----->|09|  |01 4b 70 9a|
+-----+---+      +--+  +-----------+

  2 type(varchar)         len(8)       Y  I  Y  A  N  G  Y  I
+-----+---+      +--+  +-----------+  +-----------------------+
|00010|110|----->|16|  |00 00 00 08|  |59 49 59 41 4e 47 59 49|
+-----+---+      +--+  +-----------+  +-----------------------+

  3 type(varchar)          len(7)      w  e  n  z  h  o  u
+-----+---+      +--+  +-----------+  +--------------------+
|00011|110|----->|1e|  |00 00 00 07|  |77 65 6e 7a 68 6f 75|
+-----+---+      +--+  +-----------+  +--------------------+

  4 type(float)       ieee format for 4.0
+-----+---+      +--+  +-----------+
|00100|011|----->|23|  |40 80 00 00|
+-----+---+      +--+  +-----------+
*/

class row
{
public:
    static char* encode(std::vector<char *> values, const schema& s);
    static std::vector<void *> decode(char* ptr, const schema& s);
private:
    static char* encode_column(char* data, int type, column_id_t id);
    static std::pair<int, void *> decode_column(char* ptr);
};