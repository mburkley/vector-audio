#include <iostream>
#include <fstream>
#include <cmath>

#include "vectorlist.h"
#include "wav.h"

using namespace std;

WavFile wav;

void line (Point c1, Point c2)
{
    #if 0
    // Ensure x1 < x2
    if (c1.x() > c2.x())
    {
        Point tmp = c1;
        c1 = c2;
        c2 = tmp;
    }
    #endif

        int x1 = c1.x() * 200 - 100;
        int y1 = c1.y() * 200 - 100;
        int x2 = c2.x() * 200 - 100;
        int y2 = c2.y() * 200 - 100;

    cout << "line "<<x1<<","<<y1<<","<<x2<<","<<y2<<endl;
    int xd = (x2 < x1) ? -1 : 1;
    int yd = (y2 < y1) ? -1 : 1;

    int err = 0;

    if (y1 == y2)
        err = -1;

    int x = x1;
    int y = y1;

    while (x != x2 || y != y2)
    {
        if (err < 0)
        {
            x += xd;
            err += (y2 - y1) * yd;
        }
        else
        {
            y += yd;
            err -= (x2 - x1) * xd;
        }

        cout << x << "," << y << endl ;
        wav.writeSample (x*320, y*320);
    }
}

Point linePoint (Point c0, Point c1, double t)
{
    int x = (c1.x() - c0.x()) * t + c0.x();
    int y = (c1.y() - c0.y()) * t + c0.y();
    return Point (x,y);
}

void cubicBezier (VectorList& list, Point c0, Point c1, Point c2, Point c3)
{
    Point cursor = c0;

    for (double t = 0; t < 1.0; t += 0.1)
    {
        Point p0 = linePoint (c0, c1, t);
        Point p1 = linePoint (c1, c2, t);
        Point p2 = linePoint (c2, c3, t);

        Point q0 = linePoint (p0, p1, t);
        Point q1 = linePoint (p1, p2, t);

        Point draw = linePoint (q0, q1, t);
        // line (cursor, draw);
        // cursor = draw;
        list.add (cursor, draw);
        cursor = draw;
    }
}

void draw(VectorList& list, double left, double top, double right, double bottom)
{
    for (int i = 0; i < list.count() - 1; i++)
    {
        Vector v = list.get(i);
        Point p1 (v.begin());
        Point p2 (v.end());
        cout<<"xrange "<<left<<","<<right<<endl;
        cout<<"p1x "<<p1.x() << "->" ;
        p1.scalex (left, right);
        cout << p1.x() << endl;
        cout<<"p1y "<<p1.y() << "->" ;
        p1.scaley (top, bottom);
        cout << p1.y() << endl;
        p2.scalex (left, right);
        p2.scaley (top, bottom);
        line (p1, p2);
    }

    //    line (list.get(i), list.get(i+1));

    //line (list.get(list.count() - 1), list.get(0));
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "Usage: "<<argv[0]<< " <in-file> <out-file>"<<endl;
        return 1;
    }

    wav.openWrite (argv[2], 16, 2);
    ifstream in (argv[1], ifstream::in);

    string line;
    Point cursor;
    VectorList list;
    double left, right, top, bottom;

    while (getline (in, line))
    {
        double x1,y1,x2,y2,x3,y3;
        switch (line[0])
        {
        case 'S':
            sscanf (line.c_str(), "S %lf,%lf %lf,%lf", &left, &top, &right, &bottom);
            cout << "scale "<< left<<","<<top<< " "<<right<<","<<bottom<<endl;
            break;
        case 'M':
            sscanf (line.c_str(), "M %lf,%lf", &x1, &y1);
            cout << "move "<< x1<<","<<y1<<endl;
            cursor = Point (x1, y1);
            break;
        case 'C':
            sscanf (line.c_str(), "C %lf,%lf %lf,%lf %lf,%lf", &x1, &y1, &x2,
                                &y2, &x3, &y3);
            cout << "curve "<< x1<<","<<y1<< " "<<x2<<","<<y2<<" "<< x3<<","<<y3<<endl;
            cubicBezier (list, cursor, Point (x1, y1), Point (x2, y2), Point (x3, y3));
            cursor = Point (x3, y3);
            break;
        case 'L':
            sscanf (line.c_str(), "L %lf,%lf", &x1, &y1);
            cout << "line "<< x1<<","<<y1<< endl;
            list.add (Vector (cursor, Point (x1, y1)));
            cursor = Point (x1, y1);
            break;
        default: cout << "huh"<<endl; break;
        }
    }

    for (int i = 0; i < 1000; i++)
    draw(list, left, top, right, bottom);
    #if 0
    for (double angle = 0; angle < 2 * M_PI; angle += 0.01)
    {
        int scale = 100;
        points.clear();
        points.add (scale * cos(angle), scale * sin(angle));
        points.add (scale * cos(angle+M_PI/2), scale * sin(angle+M_PI/2));
        points.add (scale * cos(angle+M_PI), scale * sin(angle+M_PI));
        points.add (scale * cos(angle+3*M_PI/2), scale * sin(angle+3*M_PI/2));
        draw();
    }
    #endif

    wav.close();
    return 0;
}

