#include "ball.hpp"

ball::ball(int gwidth, int gheight)
{
	y = gwidth/2;
	x = gheight/2;
	width = gwidth;
	height = gheight;
	speed = 30;
	angle = 120;
	r = 8;

	LeftBallLost = false;
	RightBallLost = false;
}

void ball::update()
{

	//Rebond sur le bord
	int MRayon = r / 2;
	if (x + MRayon >= width)
	{
        RightBallLost = true;
	    bounce();
	}
	if (x - MRayon <= 1)
	{
	    LeftBallLost = true;
	    bounce();
	}
	if (y + MRayon >= height)
	    bounce(false);
	if (y - MRayon <= 1)
	    bounce(false);

	//Evolution de la balle
	x += (cos(M_PI * angle / 180) * speed);
	y -= (sin(M_PI * angle / 180) * speed);
}

void ball::bounce(bool vertical)
{
	if (vertical)
	    if (angle > 0)
		angle = 180 - angle;
	    else
		angle = -(angle + 180);
	else //Horizontal
	    angle = -angle;

	//Rajoute peu de fun ...
	if(rand()%2)
	    angle += rand()%3;
	else
	    angle -= rand()%3;
}

void ball::draw(IplImage *img)
{
	cvDrawCircle(img, cvPoint(x,y), r, CV_RGB(255, 0, 0), -1);
	cvDrawCircle(img, cvPoint(x,y), r, CV_RGB(0, 0, 0));
}

bool ball::isLeftBallLost()
{
    bool temp = LeftBallLost;
    LeftBallLost = false;
    return temp;
}
bool ball::isRightBallLost()
{
    bool temp = RightBallLost;
    RightBallLost = false;
    return temp;
}

int ball::getY() const {return y;}
int ball::getX() const {return x;}
int ball::getR() const {return r;}

