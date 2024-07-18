class Point
{
public:
    Point() { _x = 0; _y = 0; }
    Point(double x, double y) { _x = x; _y = y; }
    double x() { return _x; }
    double y() { return _y; }
    void scalex (double min, double max) { _x = (_x - min) / (max - min); }
    void scaley (double min, double max) { _y = (_y - min) / (max - min); }
private:
    double _x;
    double _y;
};
