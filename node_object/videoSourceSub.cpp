/* 
 * Capture live videos or video clips
 * Project: Multimodal First Person Sensing (MFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 */

// System Library
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
// OpenCV Library
#include <cv.h>

#include <cxcore.h>
#include <highgui.h>
// System Param Library
#include "systemParams.h"

using namespace std;

// parameters
bool saveVideo = TRUE;
bool stopVideo = FALSE;

void saveFrame (string, IplImage*);
void setSaveStatus (bool);
void setPauseStatus (bool);

int main (int argc, char *argv[]) {

	// parse directory path
	string dataDir = DIR_PATH;
	dataDir.replace(dataDir.find("~"), 1, getenv("HOME"));

	// opencv variables
	CvCapture *capture;
	IplImage *frame;

	cvNamedWindow( "videoSource", CV_WINDOW_AUTOSIZE );

	// open camera or video, based on settings
	// use live camera
	if (VID_SOURCE == 0) {
		capture = cvCaptureFromCAM (CV_CAP_ANY);
		if ( !capture ) {
			cout << "<Video> Cannot initialize webcam !" << endl;
			return 1;
		}
	// use video files
	} else if (VID_SOURCE == 1) {
		capture = cvCaptureFromAVI (VID_PATH);
		if ( !capture ) {
			cout << "<Video> Cannot load video at: " << VID_PATH << endl;
			return 1;
		}
	} else {
		cout << "<Video> Camera or video disabled" << endl;
		return 1;
	}

	while (TRUE) {

		if (!stopVideo) {
			frame = cvQueryFrame (capture);
			if ( !frame ) {
				cout << "<Video> Unable to capture frame !" << endl;
				getchar();
				break;
			}
		} 

		cvShowImage("videoSource", frame);

		if ( (cvWaitKey(10) & 255) == 27 ) {
			break;
		}
		// publish
		
	}

	cvReleaseCapture( &capture );
	cvDestroyWindow( "videoSource" );
	return 0;
}

void setSaveStatus (bool s) {
	saveVideo = s;
}

void setPauseStatus (bool s) {
	stopVideo = s;
}



