#include <QtGui>
#include "ER_MainWindow.h"
#include "ER_QGLWidget.h"
#include "ER_QWebcamWidget.h"
#include "sift.h"
#include "imgfeatures.h"
#include "utils.h"

ER_MainWindow::ER_MainWindow() 
{
	glWidget = new ER_QGLWidget;
	webcamWidget = new ER_QWebcamWidget;
	isTrajectoryShown = false;
	isCameraHistoryShown = false;
	openDirectory = "";
	countFrame = -1;
	isVideoShowing = false;

	createActions();
	createMenus();
	InitializeWindow();
	
	setWindowTitle(tr("Intel Camera Motion Viewer 1.0"));
}

void ER_MainWindow::InitializeWindow()
{
	//////////////////////////////////////////////////////
	// Playback controller
	playbackGroup = new QGroupBox(tr("Play"));
	QGridLayout *playback_layout = new QGridLayout;
	
	playbackSlider = CreatePlaybackSlider();
	playbackSlider->setValue(0);
	currentFrameLabel = new QLabel(tr("N/A"));
	playbackButton = CreatePlaybackButton();
	playbackButton1 = CreatePlaybackButton1();
	playbackButton3 = CreatePlaybackButton3();
	timer = new QTimer;
	timer->stop();

	//playback_layout->addWidget(playbackSlider, 0, 0, 1, -1);
	playback_layout->addWidget(playbackButton, 0, 1);
	playback_layout->addWidget(playbackButton1, 0, 0);
	playback_layout->addWidget(playbackButton3, 0, 2);
	//playback_layout->addWidget(playbackButton2, 0, 2);
	playback_layout->addWidget(currentFrameLabel, 1, 0);
	playbackGroup->setLayout(playback_layout);

	//connect(timer, SIGNAL(timeout()), this, SLOT(LoadCameraFileMT()));
	//connect(timer, SIGNAL(timeout()), glWidget, SLOT(UpdateFrame()));

	//connect(playbackButton, SIGNAL(toggled(bool)), glWidget, SLOT(SetPlayButton(bool)));
	connect(playbackButton, SIGNAL(released()), this, SLOT(RegisterCameraMT()));
	connect(playbackButton1, SIGNAL(released()), this, SLOT(VideoMT()));
	connect(playbackButton3, SIGNAL(released()), this, SLOT(FlushCameraData()));
	//connect(playbackButton2, SIGNAL(released()), this, SLOT(RegisterCameraMT()));
	//connect(glWidget, SIGNAL(PlayButtonChanged(int)), timer, SLOT(start(int)));
	//connect(glWidget, SIGNAL(StopTimer()), timer, SLOT(stop()));
	connect(glWidget, SIGNAL(UntoggleButton()), playbackButton, SLOT(toggle()));
	//connect(timer, SIGNAL(timeout()), glWidget, SLOT(Play()));
	connect(playbackSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(SetFrameFromPlayback(int)));
	connect(glWidget, SIGNAL(FrameFromPlaybackChanged(int)), playbackSlider, SLOT(setValue(int)));
	connect(glWidget, SIGNAL(FrameFromPlaybackChanged_Label(QString)), currentFrameLabel, SLOT(setText(QString)));

	connect(this, SIGNAL(HideTrajectory()), glWidget, SLOT(HideTrajectory()));
	connect(this, SIGNAL(ShowTrajectory()), glWidget, SLOT(ShowTrajectory()));

	connect(this, SIGNAL(HideCameraHistory()), glWidget, SLOT(HideCameraHistory()));
	connect(this, SIGNAL(ShowCameraHistory()), glWidget, SLOT(ShowCameraHistory()));

	connect(this, SIGNAL(ChangeStaticPointSize(int)), glWidget, SLOT(ChangeStaticPointSize(int)));
	connect(this, SIGNAL(ChangeDynamicPointSize(int)), glWidget, SLOT(ChangeDynamicPointSize(int)));

	connect(this, SIGNAL(ChangeStaticPointColor()), glWidget, SLOT(ChangeStaticPointColor()));
	connect(this, SIGNAL(ChangeDynamicPointColor()), glWidget, SLOT(ChangeDynamicPointColor()));
	connect(this, SIGNAL(CaptureImage()), glWidget, SLOT(CaptureImage()));

	connect(this, SIGNAL(PressPlayButton()), playbackButton, SLOT(toggle()));
	connect(this, SIGNAL(UpdateFrame()), glWidget, SLOT(UpdateFrame()));

	//////////////////////////////////////////////////////
	// Window Design
	QFrame *main_window = new QFrame;
	QVBoxLayout *top_layout = new QVBoxLayout(main_window);
	
	//top_layout->addWidget(webcamWidget, 0, 0);
	top_layout->addWidget(glWidget);
	top_layout->addWidget(playbackGroup);

	setCentralWidget(main_window);
}

void ER_MainWindow::LoadCameraFileMT()
{
	QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::LoadCameraFile);
	//for (iFile_ = 0; iFile_ < vKeyFileName.size(); iFile_++)
	//{
	//	QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::LoadCameraFile);
	//	glWidget->UpdateFrame();
	//}
}

void ER_MainWindow::RegisterCameraMT()
{
	//QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::RegisterCamera);
	QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::RegisterCamera_Shohei);
}

IplImage *ER_MainWindow::GrabFrame()
{
	while (1)
	{
		if (webcamWidget->isCaptured)
		{
			cvShowImage( "Captured Image", webcamWidget->frame );
			cvWaitKey(1);
			return webcamWidget->frame;
		}
		else
			cvWaitKey(1);
	}

}

void ER_MainWindow::VideoMT()
{
	if (!isVideoShowing)
	{
		isVideoShowing = true;
		QFuture<void> future = QtConcurrent::run(webcamWidget, &ER_QWebcamWidget::ShowVideo);		
	}
	//for (iFile_ = 0; iFile_ < vKeyFileName.size(); iFile_++)
	//{
	//	QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::LoadCameraFile);
	//	glWidget->UpdateFrame();
	//}
}

double ER_MainWindow::diffclock(clock_t clock1,clock_t clock2)
{
	double diffticks=clock1-clock2;
	double diffms=(diffticks)/CLOCKS_PER_SEC;
	return diffms;
}

void ER_MainWindow::FlushCameraData()
{
	glWidget->renderer.vCamera[0].vTakenFrame.clear();
	glWidget->renderer.vCamera[0].vC.clear();
	glWidget->renderer.vCamera[0].vR.clear();	
	glWidget->UpdateFrame();
}


void ER_MainWindow::RegisterCamera_Shohei()
{
	// init tbb
	//tbb::task_scheduler_init tbb_init;

	int nn = 2;
	flann::Matrix<float> descDatabase(new float[vvDescRD.size()*128], vvDescRD.size(), 128);

	for (int iDesc = 0; iDesc < vvDescRD.size(); iDesc++)
	{
		for (int iDim = 0; iDim < 128; iDim++)
		{
			descDatabase[iDesc][iDim] = (float) vvDescRD[iDesc][iDim];
		}
	}

	flann::Index<flann::L2<float>> index_database(descDatabase, flann::KDTreeIndexParams(8));
	index_database.buildIndex();
	cvNamedWindow( "Captured Image", CV_WINDOW_AUTOSIZE );
	int key = 0;

	IplImage *im;
	IplImage *im_gray = cvCreateImage( cvSize( 640, 480 ), IPL_DEPTH_8U, 1 );
	
	while( key != 'q' ) 
	{
		im = cvLoadImage("temp.bmp");
		//im = GrabFrame();
		cvCvtColor( im, im_gray, CV_RGB2GRAY );
		IplImage * gray32f = cvCreateImage(cvGetSize(im), IPL_DEPTH_32F, 1);
		cvConvertScale(im_gray, gray32f);


		// init the detector (you can reuse this for images in the same size)
		SIFT::detector_t sd;
		sd.init(gray32f->width, gray32f->height);

		// detect
		std::vector<SIFT::keypoint_t> sift_result;
		sd.detect(gray32f, &sift_result);

		vector<SIFT_Descriptor> vSift_desc;
		for (int iSIFT = 0; iSIFT < sift_result.size(); iSIFT++)
		{
			SIFT_Descriptor descriptor;
			descriptor.x = sift_result[iSIFT].y;
			descriptor.y = sift_result[iSIFT].x;

			descriptor.dis_x = sift_result[iSIFT].y;
			descriptor.dis_y = sift_result[iSIFT].x;

			vector<int> desc;
			for (int iDim = 0; iDim < 128; iDim++)
			{
				desc.push_back(sift_result[iSIFT].descriptor[iDim]);
			}
			descriptor.vDesc = desc;
			descriptor.id = iSIFT;
			vSift_desc.push_back(descriptor);
		}

		//cvConvertImage(im, im_gray, CV_RGB2GRAY);

		//system("bmp2pgm.exe image0000001.bmp temp.pgm");
		//cvSaveImage("temp.bmp", im);
		//system("bmp2pgm.exe temp.bmp temp.pgm");
		//system("sift.exe <temp.pgm> key.key");

		//vector<SIFT_Descriptor> vSift_desc;
		//LoadSIFTData("key.key", vSift_desc);
		int n = vSift_desc.size();


		//struct feature* features;
		//int n = _sift_features( im_gray, &features, intvls, sigma, contr_thr, curv_thr,
		//	img_dbl, descr_width, descr_hist_bins );
		cout << "Number of TDs: " << n << endl;

		CvMat *C = cvCreateMat(3,1,CV_32FC1);
		CvMat *R = cvCreateMat(3,3,CV_32FC1);
		//vector<SIFT_Descriptor> vSift_desc = vvSIFT_desc[iFile];

		flann::Matrix<int> result(new int[n*nn], n, nn);
		flann::Matrix<float> dist(new float[n*nn], n, nn);

		flann::Matrix<float> descTarget(new float[n*128], n, 128);

		for (int iDesc = 0; iDesc < n; iDesc++)
		{
			for (int iDim = 0; iDim < 128; iDim++)
			{
				descTarget[iDesc][iDim] = (float) vSift_desc[iDesc].vDesc[iDim];
			}
		}

		index_database.knnSearch(descTarget, result, dist, nn, flann::SearchParams(256));

		vector<Correspondence2D3D> vCorr, vCorr_ransac;
		for (int iDesc = 0; iDesc < n; iDesc++)
		{
			float dist1 = dist[iDesc][0];
			float dist2 = dist[iDesc][1];

			if (dist1/dist2 < 0.7)
			{
				Correspondence2D3D corr;
				corr.u = vSift_desc[iDesc].x;
				corr.v = vSift_desc[iDesc].y;

				corr.x = glWidget->renderer.vStaticStructure[result[iDesc][0]].x;
				corr.y = glWidget->renderer.vStaticStructure[result[iDesc][0]].y;
				corr.z = glWidget->renderer.vStaticStructure[result[iDesc][0]].z;

				vCorr.push_back(corr);
			}
		}
		descTarget.free();
		result.free();
		dist.free();

		glWidget->UpdateFrame();

		if (vCorr.size() < 30)
		{
			cout << "No corresondence" << endl;
			cvReleaseMat(&C);
			cvReleaseMat(&R);
			continue;
		}

		CvMat *cX = cvCreateMat(vCorr.size(), 3, CV_32FC1);
		CvMat *cx = cvCreateMat(vCorr.size(), 2, CV_32FC1);

		for (int iPoint = 0; iPoint < vCorr.size(); iPoint++)
		{
			cvSetReal2D(cX, iPoint, 0, vCorr[iPoint].x);
			cvSetReal2D(cX, iPoint, 1, vCorr[iPoint].y);
			cvSetReal2D(cX, iPoint, 2, vCorr[iPoint].z);

			cvSetReal2D(cx, iPoint, 0, vCorr[iPoint].u);
			cvSetReal2D(cx, iPoint, 1, vCorr[iPoint].v);
		}

		CvMat *P = cvCreateMat(3,4,CV_32FC1);
		vector<int> vInlier;
		cout << "Number of correspondences: " << cX->rows << endl;
		if (EPNP_Ransac(cX, cx, K, P, 3, 5000, vInlier) < 20)
		{
			cout << "No ePNP solution" << endl;
			cvReleaseMat(&cX);
			cvReleaseMat(&cx);
			cvReleaseMat(&P);
			continue;
		}
		//if (EPNP_NoRansac(cX, cx, K, P) < 30)
		//{
		//	cout << "No ePNP solution" << endl;
		//	cvReleaseMat(&cX);
		//	cvReleaseMat(&cx);
		//	cvReleaseMat(&P);
		//	cvReleaseMat(&C);
		//	cvReleaseMat(&R);
		//	continue;
		//}
		//CvMat *K_inv = cvCreateMat(3,3,CV_32FC1);
		cvMatMul(K_inv, P, P);
		cvSetReal2D(R, 0, 0, cvGetReal2D(P, 0, 0));		cvSetReal2D(R, 0, 1, cvGetReal2D(P, 0, 1));		cvSetReal2D(R, 0, 2, cvGetReal2D(P, 0, 2));
		cvSetReal2D(R, 1, 0, cvGetReal2D(P, 1, 0));		cvSetReal2D(R, 1, 1, cvGetReal2D(P, 1, 1));		cvSetReal2D(R, 1, 2, cvGetReal2D(P, 1, 2));
		cvSetReal2D(R, 2, 0, cvGetReal2D(P, 2, 0));		cvSetReal2D(R, 2, 1, cvGetReal2D(P, 2, 1));		cvSetReal2D(R, 2, 2, cvGetReal2D(P, 2, 2));

		cvSetReal2D(C, 0, 0, -cvGetReal2D(P, 0, 3));	cvSetReal2D(C, 1, 0, -cvGetReal2D(P, 1, 3));	cvSetReal2D(C, 2, 0, -cvGetReal2D(P, 2, 3));
		CvMat *R_inv  = cvCreateMat(3,3,CV_32FC1);
		cvTranspose(R, R_inv);
		cvMatMul(R_inv, C, C);

		cvReleaseMat(&R_inv);
		//cvReleaseMat(&P);
		cvReleaseMat(&cX);
		cvReleaseMat(&cx);
		cvReleaseMat(&P);

		glWidget->renderer.vCamera[0].vTakenFrame.push_back(0);
		glWidget->renderer.vCamera[0].vC.push_back(C);
		glWidget->renderer.vCamera[0].vR.push_back(R);	

		glWidget->UpdateFrame();
	}
	descDatabase.free();
	cvReleaseImage(&im);
}

void ER_MainWindow::RegisterCamera()
{
	
	int nn = 2;
	flann::Matrix<float> descDatabase(new float[vvDescRD.size()*128], vvDescRD.size(), 128);

	for (int iDesc = 0; iDesc < vvDescRD.size(); iDesc++)
	{
		for (int iDim = 0; iDim < 128; iDim++)
		{
			descDatabase[iDesc][iDim] = (float) vvDescRD[iDesc][iDim];
		}
	}

	flann::Index<flann::L2<float>> index_database(descDatabase, flann::KDTreeIndexParams(8));
	index_database.buildIndex();
	cvNamedWindow( "Captured Image", CV_WINDOW_AUTOSIZE );
	int key = 0;

	//int intvls = SIFT_INTVLS;
	//double sigma = SIFT_SIGMA;
	//double contr_thr = SIFT_CONTR_THR;
	//int curv_thr = SIFT_CURV_THR;
	//int img_dbl = SIFT_IMG_DBL;
	//int descr_width = SIFT_DESCR_WIDTH;
	//int descr_hist_bins = SIFT_DESCR_HIST_BINS;


	IplImage *im;
	IplImage *im_gray = cvCreateImage( cvSize( 640, 480 ), IPL_DEPTH_8U, 1 );;
	while( key != 'q' ) 
	{
		im = GrabFrame();
		//cvConvertImage(im, im_gray, CV_RGB2GRAY);

		//system("bmp2pgm.exe image0000001.bmp temp.pgm");
		cvSaveImage("temp.bmp", im);
		system("bmp2pgm.exe temp.bmp temp.pgm");
		system("sift.exe <temp.pgm> key.key");

		vector<SIFT_Descriptor> vSift_desc;
		LoadSIFTData("key.key", vSift_desc);
		int n = vSift_desc.size();


		//struct feature* features;
		//int n = _sift_features( im_gray, &features, intvls, sigma, contr_thr, curv_thr,
		//	img_dbl, descr_width, descr_hist_bins );
		cout << "Number of TDs: " << n << endl;

		CvMat *C = cvCreateMat(3,1,CV_32FC1);
		CvMat *R = cvCreateMat(3,3,CV_32FC1);
		//vector<SIFT_Descriptor> vSift_desc = vvSIFT_desc[iFile];
		
		flann::Matrix<int> result(new int[n*nn], n, nn);
		flann::Matrix<float> dist(new float[n*nn], n, nn);

		flann::Matrix<float> descTarget(new float[n*128], n, 128);

		for (int iDesc = 0; iDesc < n; iDesc++)
		{
			for (int iDim = 0; iDim < 128; iDim++)
			{
				descTarget[iDesc][iDim] = (float) vSift_desc[iDesc].vDesc[iDim];
			}
		}

		index_database.knnSearch(descTarget, result, dist, nn, flann::SearchParams(256));

		vector<Correspondence2D3D> vCorr, vCorr_ransac;
		for (int iDesc = 0; iDesc < n; iDesc++)
		{
			float dist1 = dist[iDesc][0];
			float dist2 = dist[iDesc][1];

			if (dist1/dist2 < 0.7)
			{
				Correspondence2D3D corr;
				corr.u = vSift_desc[iDesc].x;
				corr.v = vSift_desc[iDesc].y;

				corr.x = glWidget->renderer.vStaticStructure[result[iDesc][0]].x;
				corr.y = glWidget->renderer.vStaticStructure[result[iDesc][0]].y;
				corr.z = glWidget->renderer.vStaticStructure[result[iDesc][0]].z;

				vCorr.push_back(corr);
			}
		}
		descTarget.free();
		result.free();
		dist.free();

		if (vCorr.size() < 30)
		{
			cout << "No corresondence" << endl;
			cvReleaseMat(&C);
			cvReleaseMat(&R);
			continue;
		}

		CvMat *cX = cvCreateMat(vCorr.size(), 3, CV_32FC1);
		CvMat *cx = cvCreateMat(vCorr.size(), 2, CV_32FC1);

		for (int iPoint = 0; iPoint < vCorr.size(); iPoint++)
		{
			cvSetReal2D(cX, iPoint, 0, vCorr[iPoint].x);
			cvSetReal2D(cX, iPoint, 1, vCorr[iPoint].y);
			cvSetReal2D(cX, iPoint, 2, vCorr[iPoint].z);

			cvSetReal2D(cx, iPoint, 0, vCorr[iPoint].u);
			cvSetReal2D(cx, iPoint, 1, vCorr[iPoint].v);
		}

		CvMat *P = cvCreateMat(3,4,CV_32FC1);
		vector<int> vInlier;
		cout << "Number of correspondences: " << cX->rows << endl;
		if (EPNP_Ransac(cX, cx, K, P, 3, 5000, vInlier) < 20)
		{
			cout << "No ePNP solution" << endl;
			cvReleaseMat(&cX);
			cvReleaseMat(&cx);
			cvReleaseMat(&P);
			continue;
		}
		//if (EPNP_NoRansac(cX, cx, K, P) < 30)
		//{
		//	cout << "No ePNP solution" << endl;
		//	cvReleaseMat(&cX);
		//	cvReleaseMat(&cx);
		//	cvReleaseMat(&P);
		//	cvReleaseMat(&C);
		//	cvReleaseMat(&R);
		//	continue;
		//}
		//CvMat *K_inv = cvCreateMat(3,3,CV_32FC1);
		cvMatMul(K_inv, P, P);
		cvSetReal2D(R, 0, 0, cvGetReal2D(P, 0, 0));		cvSetReal2D(R, 0, 1, cvGetReal2D(P, 0, 1));		cvSetReal2D(R, 0, 2, cvGetReal2D(P, 0, 2));
		cvSetReal2D(R, 1, 0, cvGetReal2D(P, 1, 0));		cvSetReal2D(R, 1, 1, cvGetReal2D(P, 1, 1));		cvSetReal2D(R, 1, 2, cvGetReal2D(P, 1, 2));
		cvSetReal2D(R, 2, 0, cvGetReal2D(P, 2, 0));		cvSetReal2D(R, 2, 1, cvGetReal2D(P, 2, 1));		cvSetReal2D(R, 2, 2, cvGetReal2D(P, 2, 2));

		cvSetReal2D(C, 0, 0, -cvGetReal2D(P, 0, 3));	cvSetReal2D(C, 1, 0, -cvGetReal2D(P, 1, 3));	cvSetReal2D(C, 2, 0, -cvGetReal2D(P, 2, 3));
		CvMat *R_inv  = cvCreateMat(3,3,CV_32FC1);
		cvTranspose(R, R_inv);
		cvMatMul(R_inv, C, C);

		cvReleaseMat(&R_inv);
		//cvReleaseMat(&P);
		cvReleaseMat(&cX);
		cvReleaseMat(&cx);
		cvReleaseMat(&P);

		glWidget->renderer.vCamera[0].vTakenFrame.push_back(0);
		glWidget->renderer.vCamera[0].vC.push_back(C);
		glWidget->renderer.vCamera[0].vR.push_back(R);	
		
		glWidget->UpdateFrame();
	}
	descDatabase.free();
	cvReleaseImage(&im);
}

void ER_MainWindow::LoadCameraFile()
{
	//QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::LoadCameraFile);
	//glWidget->UpdateFrame();
	vvSIFT_desc.clear();
	vector<CvMat *> vP_temp;
	for (int iFile = 0; iFile < 4/*vKeyFileName.size()*/; iFile++)
	{
		string openDirectory_s = openDirectory.toStdString();
		string keyFile = openDirectory_s + "/key/" + vKeyFileName[iFile];

		vector<SIFT_Descriptor> vSift_desc;
		LoadSIFTData(keyFile, vSift_desc);

		vector<double> vx1, vy1;
		for (int isift = 0; isift < vSift_desc.size(); isift++)
		{
			vx1.push_back(vSift_desc[isift].x);
			vy1.push_back(vSift_desc[isift].y);
		}
		Undistortion(K, K_inv, omega, vx1, vy1);
		for (int isift = 0; isift < vSift_desc.size(); isift++)
		{
			vSift_desc[isift].x = vx1[isift];
			vSift_desc[isift].y = vy1[isift];
		}

		vvSIFT_desc.push_back(vSift_desc);
	}

	int nn = 2;
	// Construct 3D KD tree

	flann::Matrix<float> descDatabase(new float[vvDescRD.size()*128], vvDescRD.size(), 128);

	for (int iDesc = 0; iDesc < vvDescRD.size(); iDesc++)
	{
		for (int iDim = 0; iDim < 128; iDim++)
		{
			descDatabase[iDesc][iDim] = (float) vvDescRD[iDesc][iDim];
		}
	}

	clock_t begin=clock();
	flann::Index<flann::L2<float>> index_database(descDatabase, flann::KDTreeIndexParams(8));
	index_database.buildIndex();
	clock_t end =clock();
	cout << "KD tree building: " << double(diffclock(end,begin)) << endl;

	for (int iFile = 0; iFile < vvSIFT_desc.size(); iFile++)
	{
		CvMat *C = cvCreateMat(3,1,CV_32FC1);
		CvMat *R = cvCreateMat(3,3,CV_32FC1);
		vector<SIFT_Descriptor> vSift_desc = vvSIFT_desc[iFile];
		flann::Matrix<int> result(new int[vSift_desc.size()*nn], vSift_desc.size(), nn);
		flann::Matrix<float> dist(new float[vSift_desc.size()*nn], vSift_desc.size(), nn);

		flann::Matrix<float> descTarget(new float[vSift_desc.size()*128], vSift_desc.size(), 128);

		for (int iDesc = 0; iDesc < vSift_desc.size(); iDesc++)
		{
			for (int iDim = 0; iDim < 128; iDim++)
			{
				descTarget[iDesc][iDim] = (float) vSift_desc[iDesc].vDesc[iDim];
			}
		}

		begin = clock();
		index_database.knnSearch(descTarget, result, dist, nn, flann::SearchParams(256));
		end = clock();
		cout << "KD tree search: " << double(diffclock(end,begin))  << " " <<  vSift_desc.size() << endl;
		
		
		vector<Correspondence2D3D> vCorr, vCorr_ransac;
		for (int iDesc = 0; iDesc < vSift_desc.size(); iDesc++)
		{
			float dist1 = dist[iDesc][0];
			float dist2 = dist[iDesc][1];

			if (dist1/dist2 < 0.7)
			{
				Correspondence2D3D corr;
				corr.u = vSift_desc[iDesc].x;
				corr.v = vSift_desc[iDesc].y;

				corr.x = glWidget->renderer.vStaticStructure[result[iDesc][0]].x;
				corr.y = glWidget->renderer.vStaticStructure[result[iDesc][0]].y;
				corr.z = glWidget->renderer.vStaticStructure[result[iDesc][0]].z;

				vCorr.push_back(corr);
			}
		}
		descTarget.free();
		result.free();
		dist.free();

		if (vCorr.size() < 30)
		{
			cout << "No corresondence" << endl;
			cvReleaseMat(&C);
			cvReleaseMat(&R);
			continue;
		}

		CvMat *cX = cvCreateMat(vCorr.size(), 3, CV_32FC1);
		CvMat *cx = cvCreateMat(vCorr.size(), 2, CV_32FC1);

		for (int iPoint = 0; iPoint < vCorr.size(); iPoint++)
		{
			cvSetReal2D(cX, iPoint, 0, vCorr[iPoint].x);
			cvSetReal2D(cX, iPoint, 1, vCorr[iPoint].y);
			cvSetReal2D(cX, iPoint, 2, vCorr[iPoint].z);

			cvSetReal2D(cx, iPoint, 0, vCorr[iPoint].u);
			cvSetReal2D(cx, iPoint, 1, vCorr[iPoint].v);
		}

		CvMat *P = cvCreateMat(3,4,CV_32FC1);
		vector<int> vInlier;
		cout << "Number of correspondences: " << cX->rows << endl;
		//if (EPNP_Ransac(cX, cx, K, P, 1e+1, 100, vInlier) < 30)
		//{
		//	cout << "No ePNP solution" << endl;
		//	cvReleaseMat(&cX);
		//	cvReleaseMat(&cx);
		//	cvReleaseMat(&P);
		//	return false;
		//}
		if (EPNP_NoRansac(cX, cx, K, P) < 30)
		{
			cout << "No ePNP solution" << endl;
			cvReleaseMat(&cX);
			cvReleaseMat(&cx);
			cvReleaseMat(&P);
			cvReleaseMat(&C);
			cvReleaseMat(&R);
			continue;
		}
		//CvMat *K_inv = cvCreateMat(3,3,CV_32FC1);
		cvMatMul(K_inv, P, P);
		cvSetReal2D(R, 0, 0, cvGetReal2D(P, 0, 0));		cvSetReal2D(R, 0, 1, cvGetReal2D(P, 0, 1));		cvSetReal2D(R, 0, 2, cvGetReal2D(P, 0, 2));
		cvSetReal2D(R, 1, 0, cvGetReal2D(P, 1, 0));		cvSetReal2D(R, 1, 1, cvGetReal2D(P, 1, 1));		cvSetReal2D(R, 1, 2, cvGetReal2D(P, 1, 2));
		cvSetReal2D(R, 2, 0, cvGetReal2D(P, 2, 0));		cvSetReal2D(R, 2, 1, cvGetReal2D(P, 2, 1));		cvSetReal2D(R, 2, 2, cvGetReal2D(P, 2, 2));

		cvSetReal2D(C, 0, 0, -cvGetReal2D(P, 0, 3));	cvSetReal2D(C, 1, 0, -cvGetReal2D(P, 1, 3));	cvSetReal2D(C, 2, 0, -cvGetReal2D(P, 2, 3));
		CvMat *R_inv  = cvCreateMat(3,3,CV_32FC1);
		cvTranspose(R, R_inv);
		cvMatMul(R_inv, C, C);

		vP_temp.push_back(P);
		cvReleaseMat(&R_inv);
		//cvReleaseMat(&P);
		cvReleaseMat(&cX);
		cvReleaseMat(&cx);

		glWidget->renderer.vCamera[0].vTakenFrame.push_back(iFile);
		glWidget->renderer.vCamera[0].vC.push_back(C);
		glWidget->renderer.vCamera[0].vR.push_back(R);	
		CvMat *K_temp = cvCreateMat(3,3,CV_32FC1);
		cvSetIdentity(K_temp);

		SaveAbsoluteCameraData("camera.txt", vP_temp, glWidget->renderer.vCamera[0].vTakenFrame, glWidget->renderer.vCamera[0].vTakenFrame.size(), K_temp);
		//glWidget->UpdateFrame();
		emit UpdateFrame();
	}
	descDatabase.free();
	glWidget->UpdateFrame();
}

void ER_MainWindow::Undistortion(CvMat *K, CvMat *invK, double omega, vector<double> &vx,  vector<double> &vy)
{
	for (int iPoint = 0; iPoint < vx.size(); iPoint++)
	{
		CvMat *x_homo = cvCreateMat(3,1,CV_32FC1);
		cvSetReal2D(x_homo, 0, 0, vx[iPoint]);
		cvSetReal2D(x_homo, 1, 0, vy[iPoint]);
		cvSetReal2D(x_homo, 2, 0, 1);
		CvMat *x_homo_n = cvCreateMat(3,1,CV_32FC1);
		cvMatMul(invK, x_homo, x_homo_n);
		double x_n, y_n;
		x_n = cvGetReal2D(x_homo_n, 0, 0);
		y_n = cvGetReal2D(x_homo_n, 1, 0);
		double r_d = sqrt(x_n*x_n+y_n*y_n);
		double r_u = tan(r_d*omega)/2/tan(omega/2); 
		double x_u = r_u/r_d*x_n;
		double y_u = r_u/r_d*y_n;
		CvMat *x_undist_n = cvCreateMat(3,1,CV_32FC1);
		cvSetReal2D(x_undist_n, 0, 0, x_u);
		cvSetReal2D(x_undist_n, 1, 0, y_u);
		cvSetReal2D(x_undist_n, 2, 0, 1);
		CvMat *x_undist = cvCreateMat(3,1,CV_32FC1);
		cvMatMul(K, x_undist_n, x_undist);
		vx[iPoint] = cvGetReal2D(x_undist,0,0);
		vy[iPoint] = cvGetReal2D(x_undist,1,0);

		cvReleaseMat(&x_homo);
		cvReleaseMat(&x_homo_n);
		cvReleaseMat(&x_undist_n);
		cvReleaseMat(&x_undist);
	}
}

int ER_MainWindow::EPNP_ExtrinsicCameraParamEstimation(CvMat *X, CvMat *x, CvMat *K, CvMat *P)
{
	epnp PnP;

	PnP.set_internal_parameters(cvGetReal2D(K, 0, 2), cvGetReal2D(K, 1, 2), cvGetReal2D(K, 0, 0), cvGetReal2D(K, 1, 1));
	PnP.set_maximum_number_of_correspondences(X->rows);
	PnP.reset_correspondences();
	for(int i = 0; i < X->rows; i++) {
		PnP.add_correspondence(cvGetReal2D(X, i, 0), cvGetReal2D(X, i, 1), cvGetReal2D(X, i, 2), cvGetReal2D(x, i, 0), cvGetReal2D(x, i, 1));
	}

	double R_est[3][3], t_est[3];
	double err2 = PnP.compute_pose(R_est, t_est);

	cvSetReal2D(P, 0, 3, t_est[0]);
	cvSetReal2D(P, 1, 3, t_est[1]);
	cvSetReal2D(P, 2, 3, t_est[2]);

	cvSetReal2D(P, 0, 0, R_est[0][0]);		cvSetReal2D(P, 0, 1, R_est[0][1]);		cvSetReal2D(P, 0, 2, R_est[0][2]);
	cvSetReal2D(P, 1, 0, R_est[1][0]);		cvSetReal2D(P, 1, 1, R_est[1][1]);		cvSetReal2D(P, 1, 2, R_est[1][2]);
	cvSetReal2D(P, 2, 0, R_est[2][0]);		cvSetReal2D(P, 2, 1, R_est[2][1]);		cvSetReal2D(P, 2, 2, R_est[2][2]);
	cvMatMul(K, P, P);

	return 1;
}


int ER_MainWindow::EPNP_Ransac(CvMat *X, CvMat *x, CvMat *K, CvMat *P, double ransacThreshold, int ransacMaxIter, vector<int> &vInlierIndex)
{
	srand ( time(NULL) );
	int min_set = 4;
	if (X->rows < min_set)
		return 0;

	/////////////////////////////////////////////////////////////////
	// Ransac
	vector<int> vOutlierIndex;
	vInlierIndex.clear();
	vOutlierIndex.clear();

	vector<int> vInlier, vOutlier;
	int maxInlier = 0;

	CvMat *X_homoT = cvCreateMat(4, X->rows, CV_32FC1);
	CvMat *X_homo = cvCreateMat(X->rows, 4, CV_32FC1);
	CvMat *x_homoT = cvCreateMat(3, x->rows, CV_32FC1);
	CvMat *x_homo = cvCreateMat(x->rows, 3, CV_32FC1);
	Inhomo2Homo(X, X_homo);
	cvTranspose(X_homo, X_homoT);
	Inhomo2Homo(x, x_homo);
	cvTranspose(x_homo, x_homoT);

	CvMat *randx = cvCreateMat(min_set, 2, CV_32FC1);
	CvMat *randX = cvCreateMat(min_set, 3, CV_32FC1);
	CvMat *randP = cvCreateMat(3,4,CV_32FC1);
	int *randIdx = (int *) malloc(min_set * sizeof(int));

	CvMat *reproj = cvCreateMat(3,1,CV_32FC1);
	CvMat *homo_X = cvCreateMat(4,1,CV_32FC1);
	for (int iRansacIter = 0; iRansacIter < ransacMaxIter; iRansacIter++)
	{		
		for (int iIdx = 0; iIdx < min_set; iIdx++)
			randIdx[iIdx] = rand()%X->rows;

		for (int iIdx = 0; iIdx < min_set; iIdx++)
		{
			cvSetReal2D(randx, iIdx, 0, cvGetReal2D(x, randIdx[iIdx], 0));
			cvSetReal2D(randx, iIdx, 1, cvGetReal2D(x, randIdx[iIdx], 1));
			cvSetReal2D(randX, iIdx, 0, cvGetReal2D(X, randIdx[iIdx], 0));
			cvSetReal2D(randX, iIdx, 1, cvGetReal2D(X, randIdx[iIdx], 1));
			cvSetReal2D(randX, iIdx, 2, cvGetReal2D(X, randIdx[iIdx], 2));
		}
		EPNP_ExtrinsicCameraParamEstimation(randX, randx, K, randP);

		vInlier.clear();
		vOutlier.clear();
		for (int ip = 0; ip < X->rows; ip++)
		{
			cvSetReal2D(homo_X, 0, 0, cvGetReal2D(X, ip, 0));
			cvSetReal2D(homo_X, 1, 0, cvGetReal2D(X, ip, 1));
			cvSetReal2D(homo_X, 2, 0, cvGetReal2D(X, ip, 2));
			cvSetReal2D(homo_X, 3, 0, 1);

			cvMatMul(randP, homo_X, reproj);
			double u = cvGetReal2D(reproj, 0, 0)/cvGetReal2D(reproj, 2, 0);
			double v = cvGetReal2D(reproj, 1, 0)/cvGetReal2D(reproj, 2, 0);
			double dist = sqrt((u-cvGetReal2D(x, ip, 0))*(u-cvGetReal2D(x, ip, 0))+(v-cvGetReal2D(x, ip, 1))*(v-cvGetReal2D(x, ip, 1)));
			if (dist < ransacThreshold)
			{
				vInlier.push_back(ip);
			}
			else
			{
				vOutlier.push_back(ip);
			}

		}


		if (vInlier.size() > maxInlier)
		{
			maxInlier = vInlier.size();
			SetSubMat(P, 0, 0, randP);
			vInlierIndex = vInlier;
			vOutlierIndex = vOutlier;
		}

		if (vInlier.size() > X->rows * 0.8)
		{
			break;
		}
	}

	//CvMat *Xin = cvCreateMat(vInlierIndex.size(), 3, CV_32FC1);
	//CvMat *xin = cvCreateMat(vInlierIndex.size(), 2, CV_32FC1);
	//for (int iInlier = 0; iInlier < vInlierIndex.size(); iInlier++)
	//{
	//	cvSetReal2D(Xin, iInlier, 0, cvGetReal2D(X, vInlierIndex[iInlier], 0));
	//	cvSetReal2D(Xin, iInlier, 1, cvGetReal2D(X, vInlierIndex[iInlier], 1));
	//	cvSetReal2D(Xin, iInlier, 2, cvGetReal2D(X, vInlierIndex[iInlier], 2));

	//	cvSetReal2D(xin, iInlier, 0, cvGetReal2D(x, vInlierIndex[iInlier], 0));
	//	cvSetReal2D(xin, iInlier, 1, cvGetReal2D(x, vInlierIndex[iInlier], 1));
	//}
	//EPNP_ExtrinsicCameraParamEstimation(Xin, xin, K, P);

	//cvReleaseMat(&Xin);
	//cvReleaseMat(&xin);
	cvReleaseMat(&reproj);
	cvReleaseMat(&homo_X);
	free(randIdx);
	cvReleaseMat(&randx);
	cvReleaseMat(&randX);
	cvReleaseMat(&randP);

	cvReleaseMat(&X_homoT);
	cvReleaseMat(&x_homo);
	cvReleaseMat(&x_homoT);
	cvReleaseMat(&X_homo);
	//if (vInlierIndex.size() < 20)
	//	return 0;
	cout << "Number of features to do ePNP camera pose estimation: " << vInlierIndex.size() << endl;
	return vInlierIndex.size();
}

int ER_MainWindow::EPNP_NoRansac(CvMat *X, CvMat *x, CvMat *K, CvMat *P)
{
	if (X->rows < 4)
		return 0;

	EPNP_ExtrinsicCameraParamEstimation(X, x, K, P);
	return X->rows;
}

bool ER_MainWindow::AbsoluteRegistration(string filename, CvMat *C, CvMat *R)
{

	vector<SIFT_Descriptor> vSift_desc;
	LoadSIFTData(filename, vSift_desc);

	vector<double> vx1, vy1;
	for (int isift = 0; isift < vSift_desc.size(); isift++)
	{
		vx1.push_back(vSift_desc[isift].x);
		vy1.push_back(vSift_desc[isift].y);
	}
	Undistortion(K, K_inv, omega, vx1, vy1);
	for (int isift = 0; isift < vSift_desc.size(); isift++)
	{
		vSift_desc[isift].x = vx1[isift];
		vSift_desc[isift].y = vy1[isift];
	}

	flann::Matrix<float> descTarget(new float[vSift_desc.size()*128], vSift_desc.size(), 128);

	for (int iDesc = 0; iDesc < vSift_desc.size(); iDesc++)
	{
		for (int iDim = 0; iDim < 128; iDim++)
		{
			descTarget[iDesc][iDim] = (float) vSift_desc[iDesc].vDesc[iDim];
		}
	}
	int nn = 2;
	flann::Matrix<int> result(new int[vSift_desc.size()*nn], vSift_desc.size(), nn);
	flann::Matrix<float> dist(new float[vSift_desc.size()*nn], vSift_desc.size(), nn);
	// Construct 3D KD tree
	flann::Matrix<float> descDatabase(new float[vvDescRD.size()*128], vvDescRD.size(), 128);

	for (int iDesc = 0; iDesc < vvDescRD.size(); iDesc++)
	{
		for (int iDim = 0; iDim < 128; iDim++)
		{
			descDatabase[iDesc][iDim] = (float) vvDescRD[iDesc][iDim];
		}
	}

	flann::Index<flann::L2<float>> index_database(descDatabase, flann::KDTreeIndexParams(8));
	index_database.buildIndex();
	index_database.knnSearch(descTarget, result, dist, nn, flann::SearchParams(256));

	//index_database.nnIndex->free();
	//index_database.~Index();


	vector<Correspondence2D3D> vCorr, vCorr_ransac;
	for (int iDesc = 0; iDesc < vSift_desc.size(); iDesc++)
	{
		float dist1 = dist[iDesc][0];
		float dist2 = dist[iDesc][1];

		if (dist1/dist2 < 0.7)
		{
			Correspondence2D3D corr;
			corr.u = vSift_desc[iDesc].x;
			corr.v = vSift_desc[iDesc].y;

			corr.x = glWidget->renderer.vStaticStructure[result[iDesc][0]].x;
			corr.y = glWidget->renderer.vStaticStructure[result[iDesc][0]].y;
			corr.z = glWidget->renderer.vStaticStructure[result[iDesc][0]].z;

			//corr.id_2D = vSift_desc[iDesc].id;
			//corr.id_3D = vID[result[iDesc][0]];

			vCorr.push_back(corr);
		}
	}

	descTarget.free();
	descDatabase.free();
	result.free();
	dist.free();

	if (vCorr.size() < 30)
	{
		cout << "No corresondence" << endl;
		return false;
	}

	CvMat *cX = cvCreateMat(vCorr.size(), 3, CV_32FC1);
	CvMat *cx = cvCreateMat(vCorr.size(), 2, CV_32FC1);

	for (int iPoint = 0; iPoint < vCorr.size(); iPoint++)
	{
		cvSetReal2D(cX, iPoint, 0, vCorr[iPoint].x);
		cvSetReal2D(cX, iPoint, 1, vCorr[iPoint].y);
		cvSetReal2D(cX, iPoint, 2, vCorr[iPoint].z);

		cvSetReal2D(cx, iPoint, 0, vCorr[iPoint].u);
		cvSetReal2D(cx, iPoint, 1, vCorr[iPoint].v);
	}

	CvMat *P = cvCreateMat(3,4,CV_32FC1);
	vector<int> vInlier;
	cout << "Number of correspondences: " << cX->rows << endl;
	//if (EPNP_Ransac(cX, cx, K, P, 1e+1, 100, vInlier) < 30)
	//{
	//	cout << "No ePNP solution" << endl;
	//	cvReleaseMat(&cX);
	//	cvReleaseMat(&cx);
	//	cvReleaseMat(&P);
	//	return false;
	//}
	if (EPNP_NoRansac(cX, cx, K, P) < 30)
	{
		cout << "No ePNP solution" << endl;
		cvReleaseMat(&cX);
		cvReleaseMat(&cx);
		cvReleaseMat(&P);
		return false;
	}
	//CvMat *K_inv = cvCreateMat(3,3,CV_32FC1);
	cvMatMul(K_inv, P, P);
	cvSetReal2D(R, 0, 0, cvGetReal2D(P, 0, 0));		cvSetReal2D(R, 0, 1, cvGetReal2D(P, 0, 1));		cvSetReal2D(R, 0, 2, cvGetReal2D(P, 0, 2));
	cvSetReal2D(R, 1, 0, cvGetReal2D(P, 1, 0));		cvSetReal2D(R, 1, 1, cvGetReal2D(P, 1, 1));		cvSetReal2D(R, 1, 2, cvGetReal2D(P, 1, 2));
	cvSetReal2D(R, 2, 0, cvGetReal2D(P, 2, 0));		cvSetReal2D(R, 2, 1, cvGetReal2D(P, 2, 1));		cvSetReal2D(R, 2, 2, cvGetReal2D(P, 2, 2));

	cvSetReal2D(C, 0, 0, -cvGetReal2D(P, 0, 3));	cvSetReal2D(C, 1, 0, -cvGetReal2D(P, 1, 3));	cvSetReal2D(C, 2, 0, -cvGetReal2D(P, 2, 3));
	CvMat *R_inv  = cvCreateMat(3,3,CV_32FC1);
	cvTranspose(R, R_inv);
	cvMatMul(R_inv, C, C);

	cvReleaseMat(&R_inv);
	cvReleaseMat(&P);
	cvReleaseMat(&cX);
	cvReleaseMat(&cx);

	return true;
}

QSlider *ER_MainWindow::CreatePlaybackSlider()
{
	QSlider *slider = new QSlider(Qt::Horizontal);
	slider->setRange(0, (glWidget->renderer.max_nFrames-1)*glWidget->renderer.renderingFrequency);
	slider->setSingleStep(1);
	slider->setPageStep(glWidget->renderer.renderingFrequency);
	slider->setTickInterval(glWidget->renderer.renderingFrequency);
	slider->setTickPosition(QSlider::TicksBelow);
	return slider;
}

QPushButton *ER_MainWindow::CreatePlaybackButton()
{
	QPushButton *toggleButton = new QPushButton(tr("&Register Camera"));
	//toggleButton->setCheckable(true);
	//toggleButton->setChecked(false);
	return toggleButton;
}

QPushButton *ER_MainWindow::CreatePlaybackButton1()
{
	QPushButton *toggleButton = new QPushButton(tr("&Start Video"));
	//togglebutton->setcheckable(true);
	//togglebutton->setchecked(false);
	return toggleButton;
}

QPushButton *ER_MainWindow::CreatePlaybackButton3()
{
	QPushButton *toggleButton = new QPushButton(tr("&Flush Camera Data"));
	//toggleButton->setCheckable(true);
	//toggleButton->setChecked(false);
	return toggleButton;
}

void ER_MainWindow::createActions()
{
	openAct = new QAction(tr("&Open Dir..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open a directory"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(setExistingDirectory()));

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	captureCurrentFrameAct = new QAction(tr("Capture Current Frame"), this);
	connect(captureCurrentFrameAct, SIGNAL(triggered()), this, SLOT(CaptureCurrentFrameAct()));

	captureSequentialFrameStartAct = new QAction(tr("Capture Start"), this);
	connect(captureSequentialFrameStartAct, SIGNAL(triggered()), this, SLOT(CaptureFrameStartAct()));

	captureSequentialFrameEndAct = new QAction(tr("Capture Finish"), this);
	connect(captureSequentialFrameEndAct, SIGNAL(triggered()), this, SLOT(CaptureFrameEndAct()));

	captureFileIndexResetAct = new QAction(tr("Index Reset"), this);
	connect(captureFileIndexResetAct, SIGNAL(triggered()), this, SLOT(CaptureFileIndexReset()));

}

void ER_MainWindow::CaptureCurrentFrameAct()
{
	emit CaptureImage();
}

void ER_MainWindow::CaptureFrameStartAct()
{
	glWidget->isCapturing = true;
	glWidget->UpdateFrame();
}

void ER_MainWindow::CaptureFrameEndAct()
{
	glWidget->isCapturing = false;
	glWidget->UpdateFrame();
}

void ER_MainWindow::CaptureFileIndexReset()
{
	glWidget->renderer.capturedFileIndex = 0;
}

void ER_MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	fileMenu = menuBar()->addMenu(tr("&Capture"));
	fileMenu->addAction(captureCurrentFrameAct);
	fileMenu->addAction(captureSequentialFrameStartAct);
	fileMenu->addAction(captureSequentialFrameEndAct);
	fileMenu->addSeparator();
	fileMenu->addAction(captureFileIndexResetAct);
}

void ER_MainWindow::setExistingDirectory()
{
	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QString openDirectoryTemp = QFileDialog::getExistingDirectory(this, tr("Specify the directory where files are located"), "a", options);
	if (!openDirectoryTemp.isEmpty())
	{
		openDirectory = openDirectoryTemp;
		LoadReconstructionFiles();
	}
}

void ER_MainWindow::LoadReconstructionFiles()
{
	countFrame = 1;
	//timer->start(30);
	glWidget = new ER_QGLWidget;
	Renderer renderer;
	string openDirectory_s = openDirectory.toStdString();
	renderer.filePath = openDirectory_s + "/";
	string structureFile = openDirectory_s + "/structure.txt";
	//string cameraFile = openDirectory_s + "/camera.txt";
	string descriptorFile = openDirectory_s + "/descriptors.txt";
	string calibFile = openDirectory_s + "/calib.txt";
	//string listFile = openDirectory_s + "/key/filelist.list";

	//string thetaFile = openDirectory.toStdString() + "/theta0.txt";
	double center_x, center_y, center_z;
	vector<Camera> vCamera;
	vector<StaticStructure> vStaticStructure; 
	vector<Theta> vTheta;
	int nFeatures_dyn;
	int max_frames;
	int nCameras;
	int nBase;

	vector<double> vX, vY, vZ;
	vector<int> vID;
	//vKeyFileName.clear();
	vvDescRD.clear();
	
	LoadStructureData(structureFile, vStaticStructure);
	//LoadFileListData(listFile, vKeyFileName);
	LoadDescriptorData(descriptorFile, vvDescRD);

	vector<vector<int>> vvDescRD_temp;
	for (int iStr = 0; iStr < vStaticStructure.size(); iStr++)
	{
		vvDescRD_temp.push_back(vvDescRD[vStaticStructure[iStr].id]);
	}
	vvDescRD = vvDescRD_temp;

	// Load calibration data
	cout << "Load Calibration" << endl;
	ifstream fin_cal;
	fin_cal.open(calibFile.c_str(), ifstream::in);
	string dummy;
	int im_width, im_height;
	double focal_x, focal_y, princ_x, princ_y;
	fin_cal >> dummy >> im_width;
	fin_cal >> dummy >> im_height;
	fin_cal >> dummy >> focal_x;
	fin_cal >> dummy >> focal_y;
	fin_cal >> dummy >> princ_x;
	fin_cal >> dummy >> princ_y;
	//fin_cal >> dummy >> omega;

	K = cvCreateMat(3,3,CV_32FC1);
	cvSetIdentity(K);
	cvSetReal2D(K, 0, 0, focal_x);
	cvSetReal2D(K, 0, 2, princ_x);
	cvSetReal2D(K, 1, 1, focal_y);
	cvSetReal2D(K, 1, 2, princ_y);
	K_inv = cvCreateMat(3,3,CV_32FC1);
	cvInvert(K, K_inv);
	fin_cal.close();

	renderer.vStaticStructure = vStaticStructure;

	Camera cam;
	vCamera.push_back(cam);
	//renderer.SetStaticStrucuture(vStaticStructure);
	//renderer.SetCamera(vCamera);
	renderer.vCamera = vCamera;
	//renderer.SetTheta(vTheta, 30, max_frames);
	glWidget->renderer = renderer;
	//glWidget->renderer.vCamera.push_back(cam);

	//LoadCameraFile();

	//QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::LoadCameraFile);
	InitializeWindow();
}

void ER_MainWindow::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
		close();
	else if (e->key() == Qt::Key_T)
	{
		if (isTrajectoryShown)
		{
			isTrajectoryShown = false;
			emit HideTrajectory();
		}
		else
		{
			isTrajectoryShown = true;
			emit ShowTrajectory();
		}
	}
	else if (e->key() == Qt::Key_C)
	{
		if (isCameraHistoryShown)
		{
			isCameraHistoryShown = false;
			emit HideCameraHistory();
		}
		else
		{
			isCameraHistoryShown = true;
			emit ShowCameraHistory();
		}
	}
	else if (e->key() == Qt::Key_A)
	{
		//QFuture<void> future = QtConcurrent::run(this, &ER_MainWindow::LoadCameraFile);
		emit PressPlayButton();
	}
	else if(e->key() == Qt::Key_1)
		emit ChangeStaticPointSize(-1);
	else if(e->key() == Qt::Key_2)
		emit ChangeStaticPointSize(1);
	else if(e->key() == Qt::Key_4)
		emit ChangeDynamicPointSize(-1);
	else if(e->key() == Qt::Key_5)
		emit ChangeDynamicPointSize(1);
	else if(e->key() == Qt::Key_3)
		emit ChangeStaticPointColor();
	else if(e->key() == Qt::Key_6)
		emit ChangeDynamicPointColor();
	else if(e->key() == Qt::Key_7)
		ChangeCameraSize(-0.01);
	else if(e->key() == Qt::Key_8)
		ChangeCameraSize(0.01);
	else if(e->key() == Qt::Key_9)
		ChangeBackGroundColor();
	else if(e->key() == Qt::Key_Q)
		emit CaptureImage();
	else if (e->key() == Qt::Key_Z)
	{
		glWidget->viewZ_ += ( 1000 ) * 5 / (double)(glWidget->height());
		glWidget->UpdateFrame();
	}
	else if (e->key() == Qt::Key_X)
	{
		glWidget->viewZ_ -= ( 1000 ) * 5 / (double)(glWidget->height());
		glWidget->UpdateFrame();
	}
	else
		QWidget::keyPressEvent(e);
}

void ER_MainWindow::ChangeCameraSize(float plus)
{
	if (glWidget->renderer.cameraSize+plus > 0 )
		glWidget->renderer.cameraSize += plus;
	
	glWidget->UpdateFrame();
}

void ER_MainWindow::ChangeBackGroundColor()
{
	if (glWidget->renderer.isBackGroundBlack == true)
		glWidget->renderer.isBackGroundBlack = false;
	else
		glWidget->renderer.isBackGroundBlack = true;
	glWidget->renderer.UpdateColor();
	glWidget->UpdateFrame();
}

ER_MainWindow::~ER_MainWindow()
{
}
