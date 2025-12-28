#include <cursor.h>

Cursor::Cursor(int _x, int _y){
    x = _x, y = _y;
}

bool operator<=(const Cursor &a, const Cursor &b){
    return a.x == b.x ? a.y <= b.y : a.x < b.x;
}