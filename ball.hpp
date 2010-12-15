#ifndef BALL_HPP
#define BALL_HPP

#include "opencv/cv.h"

class ball
{
    public:
    ball(int gwidth, int gheight);
    void update();
    void bounce(bool vertical = true);
    void draw(IplImage *img);

    int getY() const ;
    int getX() const ;
    int getR() const ;

    private:
    int x,y,r;
    double angle;
    int speed;

    int width, height;

};

#endif
