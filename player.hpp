#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "opencv/cv.h"

#define RATIO 4

static int vmin = 55;
static int vmax = 250;
static int tolerance = 32;

class ball;

class player
{
    public:
    player();
    void setTracking(int h, int s, int v);
    void tracking(IplImage *hsv, IplImage *frame = NULL);
    void checkCollide(ball &B);
    void debug(int x, int y, IplImage *frame);
    void draw(IplImage *img);

    private:

    void update(int yp);

    bool bounding_box(int b1_x, int b1_y, int b1_w, int b1_h, int b2_x, int b2_y, int b2_w, int b2_h);

    int score;
    int y,x,h,w;

    bool leftPlayer;

    bool isTracking;
    int _h,_s,_v;
    IplImage *mask;
    CvFont font;

};

#endif
