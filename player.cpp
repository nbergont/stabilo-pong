#include "player.hpp"
#include "ball.hpp"
#include "opencv/highgui.h"
#include <stdio.h>
#include <math.h>

player::player()
{
	y = 0;
	x = 20;
	w = 20;
	h = 120;
	score = 10;
	mask = NULL;
	isTracking = false;
	font = cvFont(1);
}

void player::setTracking(int h, int s, int v)
{
	_h = h;
	_s = s;
	_v = v;
	isTracking = true;
}

void player::tracking(IplImage *hsv, IplImage *frame)
{
	int sommeX = 0, sommeY = 0;
	int nbPixels = 0;

	if(isTracking == false)
	    return;

	if(mask == NULL)
	    mask = cvCreateImage(cvGetSize(hsv), hsv->depth, 1);


	// We create the color mask with tolerance
	cvInRangeS(hsv, cvScalar(_h - tolerance -1, _s - tolerance, vmin), cvScalar(_h + tolerance -1, _s + tolerance, vmax), mask);

	// Create kernels
	IplConvKernel *kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);

	// Noise remove ...
	cvDilate(mask, mask, kernel, 1);
	cvErode(mask, mask, kernel, 2);
	cvReleaseStructuringElement(&kernel);

	// We go through the mask to look for the tracked object and get its gravity center
	for(int xt = 0; xt < mask->width; xt++) {
	    for(int yt = 0; yt < mask->height; yt++) {

		// If its a tracked pixel, count it to the center of gravity's calcul
		if(((uchar *)(mask->imageData + yt*mask->widthStep))[xt] == 255) {
		    sommeX += xt;
		    sommeY += yt;
		    nbPixels++;
		}
	    }
	}

	//Calcul des coordonnées reel
	int xr = (sommeX / nbPixels)*RATIO;
	int yr = (sommeY / nbPixels)*RATIO;

	if(nbPixels > 5)
	{
	     debug(xr, yr, frame);

	    if(xr > mask->width/2*RATIO)
		leftPlayer = false;
	    else
		leftPlayer = true;

	    update(yr);
	}
	else
	     update(mask->height/2*RATIO);
}

void player::checkCollide(ball &B)
{
	 //La balle est sur la raquette :
	if (bounding_box(x, y - h/2, w, h, B.getX() - 2, B.getY() - 2, B.getR() + 4, B.getR() + 4))
	    B.bounce();
	else
	{
	    //la balle est dans les "buts"
	    if(leftPlayer)
            if(B.isLeftBallLost() && score)
                score--;

        if(!leftPlayer)
            if(B.isRightBallLost() && score)
                score--;
	}
}

void player::debug(int x, int y, IplImage *frame)
{
	if(frame != NULL)
	    cvDrawCircle(frame, cvPoint(x,y), 5, CV_RGB(255, 255, 255), -1);

	if(leftPlayer)
	    cvShowImage("MaskLeft", mask);
	else
	    cvShowImage("MaskRight", mask);
}

void player::draw(IplImage *img)
{
	if(isTracking == false)
	    return;

	if(leftPlayer)
	    x = 20;
	else
	    x = img->width - 40;

	cvRectangleR(img, cvRect(x, y - h/2, w, h), CV_RGB(_h, _s, _v), -1);
	cvRectangleR(img, cvRect(x, y - h/2, w, h), CV_RGB(0, 0, 0)); //Black contour

	//Print score
	char tscore[4];
	sprintf(tscore,"%d", score);
	cvPutText(img, tscore, cvPoint(x, 20), &font, CV_RGB(255, 0, 0));
}

void player::update(int yp)
{
	//Smooth mvt of player bar
	y = y + ((yp - y)/(1.6));
}

bool player::bounding_box(int b1_x, int b1_y, int b1_w, int b1_h, int b2_x, int b2_y, int b2_w, int b2_h)
{
	if ((b1_x > b2_x + b2_w - 1) ||  // is b1 on the right side of b2?
	(b1_y > b2_y + b2_h - 1) ||  // is b1 under b2?
	(b2_x > b1_x + b1_w - 1) ||  // is b2 on the right side of b1?
	(b2_y > b1_y + b1_h - 1))         // is b2 under b1?
	{
	    // no collision
	    return false;
	}

	// collision
	return true;
}

