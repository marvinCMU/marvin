#include <QtGui>

#include "ER_QWebcamWidget.h"

ER_QWebcamWidget::ER_QWebcamWidget()
{

}

ER_QWebcamWidget::~ER_QWebcamWidget()
{
}

void ER_QWebcamWidget::ShowVideo()
{
	capture = 0;
	frame = 0;

	/* initialize camera */
	capture = cvCaptureFromCAM( 0 );

	/* always check */
	if ( !capture ) {
		fprintf( stderr, "Cannot open initialize webcam!\n" );
	}

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 639);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 479);

	cvNamedWindow( "Video Input", CV_WINDOW_AUTOSIZE );
	while(1) {
		/* get a frame */
		isCaptured = false;
		frame = cvQueryFrame( capture );

		isCaptured = true;
		/* always check */
		if( !frame ) break;

		/* display current frame */
		cvShowImage( "Video Input", frame );
		//cvSaveImage("a.bmp", frame);
		cvWaitKey(1);
	}
}