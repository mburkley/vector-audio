#include <vector>

#include "point.h"

using namespace std;

class Vector
{
public:
    Vector (Point begin, Point end) { _begin = begin; _end = end; }
    Point begin () { return _begin; }
    Point end () { return _end; }
private:
    Point _begin;
    Point _end;
};


