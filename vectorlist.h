#include <vector>

#include "vector.h"

using namespace std;

class VectorList
{
public:
    void add (Vector v) { _list.push_back (v); }
    void add (Point p0, Point p1) { Vector v (p0, p1); _list.push_back(v); }
    int count() { return _list.size(); }
    void clear() { _list.clear(); }
    Vector get(int i) { return _list[i]; }
private:
    vector<Vector> _list;
};

