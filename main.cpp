#include "opencv/highgui.h"
#include "opencv/cv.h"
#include <stdio.h>
#include "ball.hpp"
#include "player.hpp"

IplImage *frame;
IplImage* sframe;
player p1,p2;

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
