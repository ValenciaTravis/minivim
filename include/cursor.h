#ifndef __MY_CURSOR__
#define __MY_CURSOR__

/**
 * Type: Cursor
 * ------------------------------------------------------------
 * A type representing a postion in the window or in the file.
 */

class Cursor{
public:
    int x, y;
    Cursor(int _x=0, int _y=0);
};

bool operator<=(const Cursor &, const Cursor &);

#endif

