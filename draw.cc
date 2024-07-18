#include <iostream>
#include <fstream>
#include <cmath>
#include <pulse/simple.h>

#include "vectorlist.h"

/*  Define the "resolution" of our vector display.  This is the number of points
 *  that will be used by the line draw method */
#define XSIZE 200
#define YSIZE 200

#define AUDIO_FREQUENCY 96000
#define AMPLITUDE 32767

using namespace std;

class VectorScreen
{
private:
    pa_simple *_pulseAudioHandle;
    VectorList _list;
    double _left, _right, _top, _bottom;
    Point _cursor;

    /*  Draw a line using the DDA algorithm to ensure we output steps along the
     *  line */
    void line (Point c1, Point c2)
    {
        /*  Scale line endpoint co-ordinates to XSIZE / YSIZE */
        int x1 = c1.x() * XSIZE - XSIZE / 2;
        int y1 = c1.y() * YSIZE - YSIZE / 2;
        int x2 = c2.x() * XSIZE - XSIZE / 2;
        int y2 = c2.y() * YSIZE - YSIZE / 2;

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

            /* Create a stereo sample where left channel is X and right channel
             * is Y.  Scale up the value to the amplitude.  Write 4 bytes to
             * pulse */
            int16_t sample[2];
            sample[0] = x * AMPLITUDE / XSIZE;
            sample[1] = y * AMPLITUDE / YSIZE;
            pa_simple_write (_pulseAudioHandle, (uint8_t*)sample, 4, NULL);
        }
    }

    /*  Find a point on a line from c0 to c1 where t is from 0.0 to 1.0 */
    Point linePoint (Point c0, Point c1, double t)
    {
        int x = (c1.x() - c0.x()) * t + c0.x();
        int y = (c1.y() - c0.y()) * t + c0.y();
        return Point (x,y);
    }

    /*  Curves are assumed to be cubic Bezier only, see
     *  https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Higher-order_curves */
    void cubicBezier (Point c0, Point c1, Point c2, Point c3)
    {
        for (double t = 0; t < 1.0; t += 0.1)
        {
            Point p0 = linePoint (c0, c1, t);
            Point p1 = linePoint (c1, c2, t);
            Point p2 = linePoint (c2, c3, t);

            Point q0 = linePoint (p0, p1, t);
            Point q1 = linePoint (p1, p2, t);

            Point p = linePoint (q0, q1, t);
            _list.add (_cursor, p);
            _cursor = p;
        }
    }

public:
    void draw ()
    {
        for (int i = 0; i < _list.count() - 1; i++)
        {
            Vector v = _list.get(i);
            Point p1 (v.begin());
            Point p2 (v.end());
            p1.scalex (_left, _right);
            p1.scaley (_top, _bottom);
            p2.scalex (_left, _right);
            p2.scaley (_top, _bottom);
            line (p1, p2);
        }
    }

    void limits (string& line)
    {
        sscanf (line.c_str(), "S %lf,%lf %lf,%lf", &_left, &_top, &_right, &_bottom);
        cout << "scale "<< _left<<","<<_top<< " "<<_right<<","<<_bottom<<endl;
    }

    void move (string& line)
    {
        double x1,y1;
        sscanf (line.c_str(), "M %lf,%lf", &x1, &y1);
        cout << "move "<< x1<<","<<y1<<endl;
        _cursor = Point (x1, y1);
    }

    void curve (string& line)
    {
        double x1,y1,x2,y2,x3,y3;
        sscanf (line.c_str(), "C %lf,%lf %lf,%lf %lf,%lf", &x1, &y1, &x2,
                            &y2, &x3, &y3);
        cout << "curve "<< x1<<","<<y1<< " "<<x2<<","<<y2<<" "<< x3<<","<<y3<<endl;
        cubicBezier (_cursor, Point (x1, y1), Point (x2, y2), Point (x3, y3));
        _cursor = Point (x3, y3);
    }

    void line (string& line)
    {
        double x1,y1;
        sscanf (line.c_str(), "L %lf,%lf", &x1, &y1);
        cout << "line "<< x1<<","<<y1<< endl;
        _list.add (Vector (_cursor, Point (x1, y1)));
        _cursor = Point (x1, y1);
    }

    bool pulseOpen ()
    {
        static pa_sample_spec pulseAudioSpec;

        pulseAudioSpec.format = PA_SAMPLE_S16NE;
        pulseAudioSpec.channels = 2;
        pulseAudioSpec.rate = AUDIO_FREQUENCY;

        _pulseAudioHandle = pa_simple_new(NULL,               // Use the default server.
                          "Vector audio",           // Our application's name.
                          PA_STREAM_PLAYBACK,
                          NULL,               // Use the default device.
                          "Games",            // Description of our stream.
                          &pulseAudioSpec,                // Our sample format.
                          NULL,               // Use default channel map
                          NULL,               // Use default buffering attributes.
                          NULL               // Ignore error code.
                          );

        return _pulseAudioHandle != nullptr;
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: "<<argv[0]<< " <in-file>"<<endl;
        return 1;
    }

    VectorScreen screen;

    if (!screen.pulseOpen())
    {
        cerr << "Pulse audio init problem" << endl;
        return 1;
    }

    ifstream in (argv[1], ifstream::in);

    string line;

    while (getline (in, line))
    {
        switch (line[0])
        {
        case 'S': screen.limits (line); break;
        case 'M': screen.move (line); break;
        case 'C': screen.curve (line); break;
        case 'L': screen.line (line); break;
        default: cout << "huh"<<endl; break;
        }
    }

    while (1)
        screen.draw ();

    return 0;
}

