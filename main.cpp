#include "opencv/highgui.h"
#include "opencv/cv.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Maths methods
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define abs(x) ((x) > 0 ? (x) : -(x))
#define sign(x) ((x) > 0 ? 1 : -1)

// Step mooving for object min & max
#define STEP_MIN 5
#define STEP_MAX 100

#define RATIO 4

class ball
{
    public:
    ball(int gwidth, int gheight)
    {
        y = gwidth/2;
        x = gheight/2;
        width = gwidth;
        height = gheight;
        speed = 30;
        angle = 120;
        r = 8;

    }

    void update()
    {

        //Rebond sur le bord
        int MRayon = r / 2;
        if (x + MRayon >= width)
           bounce();
        if (x - MRayon <= 1)
           bounce();
        if (y + MRayon >= height)
            bounce(false);
        if (y - MRayon <= 1)
            bounce(false);

        //Evolution de la balle
        x += (cos(M_PI * angle / 180) * speed);
        y -= (sin(M_PI * angle / 180) * speed);
    }

    void bounce(bool vertical = true)
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
            angle += rand()%6;
        else
            angle -= rand()%6;
    }

    void draw(IplImage *img)
    {
         cvDrawCircle(img, cvPoint(x,y), r, CV_RGB(255, 0, 0), -1);
         cvDrawCircle(img, cvPoint(x,y), r, CV_RGB(0, 0, 0));
    }

    int getY() const {return y;}
    int getX() const {return x;}
    int getR() const {return r;}

    private:
    int x,y,r;
    double angle;
    int speed;

    int width, height;

};

int vmin = 55;
int vmax = 250;
int tolerance = 32;

class player
{
    public:
    player()
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

    void setTracking(int h, int s, int v)
    {
        _h = h;
        _s = s;
        _v = v;
        isTracking = true;
    }

    void tracking(IplImage *hsv, IplImage *frame = NULL)
    {
        int sommeX = 0, sommeY = 0;
        int nbPixels = 0;

        if(isTracking == false)
            return;

        if(mask == NULL)
            mask = cvCreateImage(cvGetSize(hsv), hsv->depth, 1);


        // We create the mask
        cvInRangeS(hsv, cvScalar(_h - tolerance -1, _s - tolerance, vmin), cvScalar(_h + tolerance -1, _s + tolerance, vmax), mask);

        // Create kernels for the morphological operation
        IplConvKernel *kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);

        // Morphological opening (inverse because we have white pixels on black background)
        cvDilate(mask, mask, kernel, 1);
        cvErode(mask, mask, kernel, 2);

        // We release the memory of kernels
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

    void checkCollide(ball &B)
    {
         //La balle est sur la raquette :
        if (bounding_box(x, y - h/2, w, h, B.getX() - 2, B.getY() - 2, B.getR() + 4, B.getR() + 4))
            B.bounce();
        else
        {
            /*
            //La balle est dans les buts
            if(leftPlayer & bounding_box(0, 0, x, _YGoalPoint2, B.getX(), B.getY(), B.getR(), B.getR())
                score--;
            else(!leftPlayer & bounding_box(_XGoalPoint1, _YGoalPoint1, 0, _YGoalPoint2, B.getX(), B.getY(), B.getR(), B.getR())
                score--;
            */

        }
    }

    void debug(int x, int y, IplImage *frame)
    {
        if(frame != NULL)
            cvDrawCircle(frame, cvPoint(x,y), 5, CV_RGB(255, 255, 255), -1);

        if(leftPlayer)
            cvShowImage("MaskLeft", mask);
        else
            cvShowImage("MaskRight", mask);

    }

    void draw(IplImage *img)
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
        cvPutText(img, tscore, cvPoint(x, 20), &font, CV_RGB(255, 255, 255));
    }

    private:

    void update(int yp)
    {
        //Smooth mvt
        y = y + ((yp - y)/(1.6));

        /*
        int objectNextStepY;
        // Move step by step the object position to the desired position
        if (abs(y - yp) > STEP_MIN) {
            objectNextStepY = max(STEP_MIN, min(STEP_MAX, abs(y - yp) / 2));
            y += (-1) * sign(y - yp) * objectNextStepY;
        }
        */

    }

    bool bounding_box(int b1_x, int b1_y, int b1_w, int b1_h, int b2_x, int b2_y, int b2_w, int b2_h)
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


    int score;
    int y,x,h,w;

    bool leftPlayer;

    bool isTracking;
    int _h,_s,_v;
    IplImage *mask;
    CvFont font;

};


IplImage *frame;
IplImage* sframe;
player p1,p2;

/*
 * Get the color of the pixel where the mouse has clicked
 * We put this color as model color (the color we want to tracked)
 */
void getObjectColor(int event, int x, int y, int flags, void *param = NULL) {

	// Vars
	CvScalar pixel;
	IplImage *hsv;

	if(event == CV_EVENT_LBUTTONUP)	{

		// Get the hsv image
		hsv = cvCloneImage(sframe);
		cvCvtColor(sframe, hsv, CV_BGR2HSV);

		// Get the selected pixel
		pixel = cvGet2D(hsv, y, x);

		// Change the value of the tracked color with the color of the selected pixel
		int h = (int)pixel.val[0];
		int s = (int)pixel.val[1];
		int v = (int)pixel.val[2];

		p1.setTracking(h,s,v);

		// Release the memory of the hsv image
        cvReleaseImage(&hsv);

	}
	else if(event == CV_EVENT_RBUTTONUP)
	{
        // Get the hsv image
		hsv = cvCloneImage(sframe);
		cvCvtColor(sframe, hsv, CV_BGR2HSV);

		// Get the selected pixel
		pixel = cvGet2D(hsv, y, x);

		// Change the value of the tracked color with the color of the selected pixel
		int h = (int)pixel.val[0];
		int s = (int)pixel.val[1];
		int v = (int)pixel.val[2];

		p2.setTracking(h,s,v);

		// Release the memory of the hsv image
        cvReleaseImage(&hsv);
	}

}

int main() {

    srand ( time(NULL) );


	// Video Capture
	CvCapture *capture;

	// Initialize the video Capture
 	capture = cvCreateCameraCapture(0);

	// Check if the capture is ok
    if (!capture)
    {
		printf("Can't initialize the video capture.\n");
        return -1;
 	}

    //First frame for informations
 	sframe = cvQueryFrame(capture);

 	frame = cvCreateImage(cvSize(cvGetSize(sframe).width/RATIO, cvGetSize(sframe).height/RATIO), IPL_DEPTH_8U, 3);

 	IplImage* hsv_frame = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);

    ball balle(sframe->width, sframe->height);

    cvNamedWindow("Pong", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Reglage", CV_WINDOW_AUTOSIZE);
    cvCreateTrackbar("vmin", "Reglage", &vmin, 255, NULL );
    cvCreateTrackbar("vmax", "Reglage", &vmax, 255, NULL );
    cvCreateTrackbar("tolerance", "Reglage", &tolerance, 50, NULL );

	// Mouse event to select the tracked color on the original image
	cvSetMouseCallback("Pong", getObjectColor);

	// While we don't want to quit
	char key = 0;
	while(key != 'Q' && key != 'q') {

		// We get next frame
		sframe = cvQueryFrame(capture);
		if(!frame)
			continue;

        //Effet mirroir
        cvFlip(sframe, sframe, 1);

        cvResize(sframe, frame);

        // Convert color space to HSV as it is much easier to filter colors in the HSV color-space.
        cvCvtColor(frame, hsv_frame, CV_BGR2HSV);

        p1.tracking(hsv_frame, sframe);
        p1.draw(sframe);
        p1.checkCollide(balle);

        p2.tracking(hsv_frame, sframe);
        p2.draw(sframe);
        p2.checkCollide(balle);

        balle.update();
        balle.draw(sframe);


        cvShowImage("Pong", sframe );

		key = cvWaitKey(10);
	}

	// Destroy objects
	cvDestroyWindow("Pong");
	cvReleaseCapture(&capture);

	return 0;

}
