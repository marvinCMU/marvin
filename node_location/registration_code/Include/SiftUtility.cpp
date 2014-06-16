#include "SiftUtility.h"

void GetCorrespondences(ObjRecDatabase *database, ObjRecKeypointList* features, vector<Point> &vPoint1, vector<Point> &vPoint2)
{
	ObjRecQuery *reco = new ObjRecQuery();
	reco->set_flag("detection-threshold", SIFT_DETECTION_THRESHOLD);
	reco->set_flag("min-keypoints", SIFT_MIN_KEYPOINTS);
	Result ret = reco->recognize(database, features);

	FeaturePointList point1, point2;
	int n = reco->get_num_matches();
	for (int iPoint = 0; iPoint < reco->get_num_matches(); iPoint++)
	{	
		FeaturePointList tPoint1, tPoint2;
		Result ret1 = reco->get_match_feature_points(iPoint, &tPoint2, &tPoint1);
		point1.insert(point1.end(), tPoint1.begin(), tPoint1.end());
		point2.insert(point2.end(), tPoint2.begin(), tPoint2.end());
	}

	//cout << reco->get_num_matches() << " " << point1.size() << endl;
	vPoint1.clear();
	vPoint2.clear();
	for (int iPoint = 0; iPoint < point1.size(); iPoint++)
	{
		Point p1, p2;
		p1.x = point1[iPoint].x;
		p1.y = point1[iPoint].y;
		p2.x = point2[iPoint].x;
		p2.y = point2[iPoint].y;
		vPoint1.push_back(p1);
		vPoint2.push_back(p2);
	}

	// Unique Matching
	vector<Point> vTempPoint1, vTempPoint2;
	for (vector<Point>::iterator iit = vPoint1.begin(); iit < vPoint1.end(); iit++)
	{
		vector<Point>::const_iterator it = search(vTempPoint1.begin(),vTempPoint1.end(), iit, iit+1, IsSamePoint);
		if (it == vTempPoint1.end())
		{
			int iPoint = int(iit-vPoint1.begin());
			vTempPoint1.push_back(vPoint1[iPoint]);
			vTempPoint2.push_back(vPoint2[iPoint]);
		}
	}
	vPoint1 = vTempPoint1;
	vPoint2 = vTempPoint2;

	reco->remove_ref();
}

void GetAppointingCorrespondences(vector<Point> vPoint11, vector<Point> vPoint12, vector<Point> vPoint21, vector<Point> vPoint22, vector<Point> &vPoint1, vector<Point> &vPoint2)
{
	vPoint1.clear();
	vPoint2.clear();
	for (vector<Point>::iterator iit = vPoint11.begin(); iit < vPoint11.end(); iit++)
	{
		vector<Point>::const_iterator it = search(vPoint21.begin(), vPoint21.end(), iit, iit+1, IsSamePoint);
		if (it != vPoint21.end())
		{
			int idx = int(it-vPoint21.begin());
			int iPoint1 = int(iit-vPoint11.begin());
			if (IsSamePoint(vPoint22[idx], vPoint12[iPoint1]))
			{
				vPoint1.push_back(vPoint11[iPoint1]);
				vPoint2.push_back(vPoint12[iPoint1]);
			}				
		}
	}
}

int GetStaticCorrespondences(vector<Point> x1, vector<Point> x2, vector<Point> &newx1, vector<Point> &newx2)
{
	vector<int> vInlierID;
	CvMat *cx1 = cvCreateMat(x1.size(), 2, CV_32FC1);
	CvMat *cx2 = cvCreateMat(x1.size(), 2, CV_32FC1);
	for (int ix = 0; ix < x1.size(); ix++)
	{
		cvSetReal2D(cx1, ix, 0, x1[ix].x);
		cvSetReal2D(cx1, ix, 1, x1[ix].y);
		cvSetReal2D(cx2, ix, 0, x2[ix].x);
		cvSetReal2D(cx2, ix, 1, x2[ix].y);
	}
	CvMat *status = cvCreateMat(1,cx1->rows,CV_8UC1);
	CvMat *F = cvCreateMat(3,3,CV_32FC1);
	int n = cvFindFundamentalMat(cx1, cx2, F, CV_FM_RANSAC, 1, 0.99, status);
	newx1.clear();
	newx2.clear();
	for (int i = 0; i < cx1->rows; i++)
	{
		if (cvGetReal2D(status, 0, i) == 1)
		{
			newx1.push_back(x1[i]);
			newx2.push_back(x2[i]);
		}
	}

	//Classifier classifier;
	//vector<int> visibleFeatureID;
	//for (int i = 0; i < cx1->rows; i++)
	//{
	//	visibleFeatureID.push_back(i);
	//}
	//classifier.SetRansacParam(1e-5, 1e+3);
	//classifier.SetCorrespondance(cx1, cx2, visibleFeatureID);
	//classifier.Classify();
	//vector<int> vOutlierID;
	//classifier.GetClassificationResultByFeatureID(vInlierID, vOutlierID);
	//cx1 = cvCreateMat(classifier.inlier1->rows, classifier.inlier1->cols, CV_32FC1);
	//cx2 = cvCreateMat(classifier.inlier2->rows, classifier.inlier2->cols, CV_32FC1);
	//cx1 = cvCloneMat(classifier.inlier1);
	//cx2 = cvCloneMat(classifier.inlier2);
	//newx1.clear();
	//newx2.clear();
	//for (int i = 0; i < cx1->rows; i++)
	//{
	//	newx1.push_back(x1[i]);
	//	newx2.push_back(x2[i]);
	//}

	cvReleaseMat(&status);
	cvReleaseMat(&F);
	cvReleaseMat(&cx1);
	cvReleaseMat(&cx2);
	return 1;
}

int GetStaticCorrespondences(vector<Point> x1, vector<Point> x2, vector<bool> &vIsInlier)
{
	vector<int> vInlierID;
	CvMat *cx1 = cvCreateMat(x1.size(), 2, CV_32FC1);
	CvMat *cx2 = cvCreateMat(x1.size(), 2, CV_32FC1);
	for (int ix = 0; ix < x1.size(); ix++)
	{
		cvSetReal2D(cx1, ix, 0, x1[ix].x);
		cvSetReal2D(cx1, ix, 1, x1[ix].y);
		cvSetReal2D(cx2, ix, 0, x2[ix].x);
		cvSetReal2D(cx2, ix, 1, x2[ix].y);
	}
	CvMat *status = cvCreateMat(1,cx1->rows,CV_8UC1);
	CvMat *F = cvCreateMat(3,3,CV_32FC1);
	int n = cvFindFundamentalMat(cx1, cx2, F, CV_FM_RANSAC, 1, 0.99, status);
	for (int i = 0; i < cx1->rows; i++)
	{
		if (cvGetReal2D(status, 0, i) == 1)
		{
			vIsInlier.push_back(true);
		}
		else
		{
			vIsInlier.push_back(false);
		}
	}

	//Classifier classifier;
	//vector<int> visibleFeatureID;
	//for (int i = 0; i < cx1->rows; i++)
	//{
	//	visibleFeatureID.push_back(i);
	//}
	//classifier.SetRansacParam(1e-5, 1e+3);
	//classifier.SetCorrespondance(cx1, cx2, visibleFeatureID);
	//classifier.Classify();
	//vector<int> vOutlierID;
	//classifier.GetClassificationResultByFeatureID(vInlierID, vOutlierID);
	//cx1 = cvCreateMat(classifier.inlier1->rows, classifier.inlier1->cols, CV_32FC1);
	//cx2 = cvCreateMat(classifier.inlier2->rows, classifier.inlier2->cols, CV_32FC1);
	//cx1 = cvCloneMat(classifier.inlier1);
	//cx2 = cvCloneMat(classifier.inlier2);
	//newx1.clear();
	//newx2.clear();
	//for (int i = 0; i < cx1->rows; i++)
	//{
	//	newx1.push_back(x1[i]);
	//	newx2.push_back(x2[i]);
	//}

	cvReleaseMat(&status);
	cvReleaseMat(&F);
	cvReleaseMat(&cx1);
	cvReleaseMat(&cx2);
	return 1;
}

//int GetStaticCorrespondences(vector<Point> x1, vector<Point> x2, vector<Point> &newx1, vector<Point> &newx2)
//{
//	vector<int> vInlierID;
//	CvMat *cx1 = cvCreateMat(x1.size(), 2, CV_32FC1);
//	CvMat *cx2 = cvCreateMat(x1.size(), 2, CV_32FC1);
//	for (int ix = 0; ix < x1.size(); ix++)
//	{
//		cvSetReal2D(cx1, ix, 0, x1[ix].x);
//		cvSetReal2D(cx1, ix, 1, x1[ix].y);
//		cvSetReal2D(cx2, ix, 0, x2[ix].x);
//		cvSetReal2D(cx2, ix, 1, x2[ix].y);
//		vInlierID.push_back(ix);
//	}
//	//CvMat *status = cvCreateMat(1,cx1->rows,CV_8UC1);
//	//CvMat *F = cvCreateMat(3,3,CV_32FC1);
//	//int n = cvFindFundamentalMat(cx1, cx2, F, CV_FM_RANSAC , 1, 0.99, status);
//
//	Classifier classifier;
//	double ransacThreshold = 1e-3;
//	int ransacMaxIter = 1e+3;
//	classifier.SetRansacParam(ransacThreshold, ransacMaxIter);
//	classifier.SetCorrespondance(cx1, cx2, vInlierID);
//	classifier.Classify();
//
//
//	//newx1.clear();
//	//newx2.clear();
//	//for (int i = 0; i < cx1->rows; i++)
//	//{
//	//	if (cvGetReal2D(status, 0, i) == 1)
//	//	{
//	//		newx1.push_back(x1[i]);
//	//		newx2.push_back(x2[i]);
//	//	}
//	//}
//
//	newx1.clear();
//	newx2.clear();
//	for (int i = 0; i < classifier.inlier1->rows; i++)
//	{
//		Point p1, p2;
//		p1.x = cvGetReal2D(classifier.inlier1, i, 0);
//		p1.y = cvGetReal2D(classifier.inlier1, i, 1);
//		p2.x = cvGetReal2D(classifier.inlier2, i, 0);
//		p2.y = cvGetReal2D(classifier.inlier2, i, 1);
//		newx1.push_back(p1);
//		newx2.push_back(p2);
//	}
//	return 1;
//}

//int GetStaticCorrespondences(vector<Point> x1, vector<Point> x2, vector<int> &isInlier, int &nInlier)
//{
//	vector<int> vInlierID;
//	CvMat *cx1 = cvCreateMat(x1.size(), 2, CV_32FC1);
//	CvMat *cx2 = cvCreateMat(x1.size(), 2, CV_32FC1);
//	for (int ix = 0; ix < x1.size(); ix++)
//	{
//		cvSetReal2D(cx1, ix, 0, x1[ix].x);
//		cvSetReal2D(cx1, ix, 1, x1[ix].y);
//		cvSetReal2D(cx2, ix, 0, x2[ix].x);
//		cvSetReal2D(cx2, ix, 1, x2[ix].y);
//	}
//	CvMat *status = cvCreateMat(1,cx1->rows,CV_8UC1);
//	CvMat *F = cvCreateMat(3,3,CV_32FC1);
//	int n = cvFindFundamentalMat(cx1, cx2, F, CV_FM_RANSAC , 1, 0.99, status);
//	nInlier = 0;
//	for (int i = 0; i < cx1->rows; i++)
//	{
//		if (cvGetReal2D(status, 0, i) == 1)
//		{	
//			isInlier.push_back(FEATURE_PROPERTY_STATIC);
//			nInlier++;
//		}
//		else
//			isInlier.push_back(FEATURE_PROPERTY_DYNAMIC);
//	}
//	return n;
//}

int DoSIFT(string modelname1, ObjRecKeypointList* features1, string modelname2, ObjRecKeypointList* features2, vector<Point> &vPoint1, vector<Point> &vPoint2)
{
	vector<Point> vPoint11, vPoint12, vPoint21, vPoint22;
	ObjRecDatabase *database1 = new ObjRecDatabase();
	database1->add_model(modelname1.c_str(), features1);
	database1->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
	//int ret = database1->set_flag("feature-type", "small");
	GetCorrespondences(database1, features2, vPoint11, vPoint12);

	ObjRecDatabase *database2 = new ObjRecDatabase();
	database2->add_model(modelname2.c_str(), features2);
	database2->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
	//database2->set_flag("feature-type", "small");
	GetCorrespondences(database2, features1, vPoint22, vPoint21);
	database1->remove_ref();
	database2->remove_ref();
	features2->remove_ref();

	vPoint1.clear();	vPoint2.clear();
	GetAppointingCorrespondences(vPoint11, vPoint12, vPoint21, vPoint22, vPoint1, vPoint2);
	vPoint1 = vPoint11;
	vPoint2 = vPoint12;

	if (vPoint1.size() < 20)
		return 0;
	if (GetStaticCorrespondences(vPoint1, vPoint2, vPoint1, vPoint2) == 0)
		return 0;
	if (vPoint1.size() < 20)
		return 0;

	return 1;
}

int DoSIFT_DYNAMIC(string modelname1, ObjRecKeypointList* features1, string modelname2, ObjRecKeypointList* features2, vector<Point> &vPoint1, vector<Point> &vPoint2)
{
	vector<Point> vPoint11, vPoint12, vPoint21, vPoint22;
	ObjRecDatabase *database1 = new ObjRecDatabase();
	database1->add_model(modelname1.c_str(), features1);
	GetCorrespondences(database1, features2, vPoint11, vPoint12);

	ObjRecDatabase *database2 = new ObjRecDatabase();
	database2->add_model(modelname2.c_str(), features2);
	GetCorrespondences(database2, features1, vPoint22, vPoint21);
	database1->remove_ref();
	database2->remove_ref();
	features2->remove_ref();

	vPoint1.clear();	vPoint2.clear();
	GetAppointingCorrespondences(vPoint11, vPoint12, vPoint21, vPoint22, vPoint1, vPoint2);
	//vPoint1 = vPoint11;
	//vPoint2 = vPoint12;

	//if (vPoint1.size() < 7)
	//	return 0;
	//if (GetStaticCorrespondences(vPoint1, vPoint2, vPoint1, vPoint2) == 0)
	//	return 0;
	//if (vPoint1.size() < 20)
	//	return 0;

	return 1;
}

void Iterate_SIFT_STATIC(FrameCamera cFC, vector<FrameCamera> vCurrentFC, vector<Point> &featureSequence, vector<Feature> &feature, bool display)
{
	Image img1, img2;
	img1.read_file(cFC.imageFileName.c_str());
	IplImage *iplImg1 = cvLoadImage(cFC.imageFileName.c_str());
	//img1.read_file("temp1.jpg");
	ObjRecKeypointList* features1 = new ObjRecKeypointList ();
	features1->set_flag("max-num-features", SIFT_MAX_NUM_FEATURES_STATIC);		features1->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
	features1->set_flag("feature-strength", SIFT_FEATURE_STRENGTH_STATIC);		features1->set_flag("use-upsampling-method", SIFT_UPSAMPLING_MODE_STATIC);
	//features1->set_flag("feature-type", "small");	
	features1->extract(&img1);

	for (int iSecondFrame = 0; iSecondFrame < vCurrentFC.size(); iSecondFrame++)
	{
		img2.read_file(vCurrentFC[iSecondFrame].imageFileName.c_str());
		//img2.read_file("temp2.jpg");
		ObjRecKeypointList* features2 = new ObjRecKeypointList ();
		features2->set_flag("max-num-features", SIFT_MAX_NUM_FEATURES_STATIC);			features2->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
		features2->set_flag("feature-strength", SIFT_FEATURE_STRENGTH_STATIC);			features2->set_flag("use-upsampling-method", SIFT_UPSAMPLING_MODE_STATIC);
		//features2->set_flag("feature-type", "small");	
		features2->extract(&img2);

		vector<Point> vPoint1, vPoint2;
		if(!DoSIFT(cFC.imageFileName, features1, vCurrentFC[iSecondFrame].imageFileName, features2, vPoint1, vPoint2))
		{
			//features2->remove_ref();		
			continue;
		}
		//features2->remove_ref();

		// Registration
		for (vector<Point>::iterator iit = vPoint1.begin(); iit < vPoint1.end(); iit++)
		{
			vector<Point>::const_iterator it = search(featureSequence.begin(),featureSequence.end(),iit, iit+1, IsSamePoint);
			int iPoint = int(iit-vPoint1.begin());
			if (it != featureSequence.end())
			{
				int idx = int(it-featureSequence.begin());

				//if (((int)vPoint2[iPoint].y > 220) && ((int)vPoint2[iPoint].x < 60))
				//	continue;
				feature[idx].vx.push_back(vPoint2[iPoint].x);
				feature[idx].vy.push_back(vPoint2[iPoint].y);
				feature[idx].vCamera.push_back(vCurrentFC[iSecondFrame].cameraID);
				feature[idx].vFrame.push_back(vCurrentFC[iSecondFrame].frameIdx);
			}
			else
			{
				Feature fs;
				fs.vCamera.push_back(cFC.cameraID);
				fs.vCamera.push_back(vCurrentFC[iSecondFrame].cameraID);
				fs.vFrame.push_back(cFC.frameIdx);
				fs.vFrame.push_back(vCurrentFC[iSecondFrame].frameIdx);
				fs.vx.push_back(vPoint1[iPoint].x);
				fs.vx.push_back(vPoint2[iPoint].x);
				fs.vy.push_back(vPoint1[iPoint].y);
				fs.vy.push_back(vPoint2[iPoint].y);
				CvScalar s;
				s=cvGet2D(iplImg1,((int)vPoint1[iPoint].y),((int)vPoint1[iPoint].x));
				fs.b = s.val[0];
				fs.g = s.val[1];
				fs.r = s.val[2];
				//if (((int)vPoint1[iPoint].y > 220) && ((int)vPoint1[iPoint].x < 60))
				//	continue;
				feature.push_back(fs);
				featureSequence.push_back(vPoint1[iPoint]);
			}
		}

		if (display)
		{
			IplImage *iplImg = cvLoadImage(vCurrentFC[iSecondFrame].imageFileName.c_str());
			//IplImage *iplImg = cvLoadImage("temp2.jpg");
			//cvCircle(iplImg, cvPoint(56,227), 8, cvScalar(255,255,0), 10);
			for (int i = 0; i < vPoint1.size(); i++)
			{
				//if (((int)vPoint2[i].y > 220) && ((int)vPoint2[i].x < 60))
				//	continue;
				//cvCircle(iplImg, cvPoint(vPoint1[i].x,(int)vPoint1[i].y), 8, cvScalar(255,0,0), 1);
				cvCircle(iplImg, cvPoint((int)vPoint2[i].x,(int)vPoint2[i].y), 8, cvScalar(255,0,0), 1);
				cvLine(iplImg, cvPoint((int)vPoint1[i].x,(int)vPoint1[i].y), cvPoint((int)vPoint2[i].x,(int)vPoint2[i].y), cvScalar(0,0,0), 3);
			}
			if (iplImg->width < 1024)
			{
				cvShowImage("SIFT", iplImg);
				
				cvWaitKey(50);
				cvReleaseImage(&iplImg);
			}
			else
			{
				double scale = (double)(iplImg->width)/1024;
				CvSize size = cvSize((int)(iplImg->width/scale),(int)(iplImg->height/scale)); 
				IplImage* tmpsize=cvCreateImage(size,IPL_DEPTH_8U,3);
				cvResize(iplImg,tmpsize,CV_INTER_LINEAR); 
				cvShowImage("SIFT", tmpsize);
				//cout << "Frame 1 : " << cFC.cameraID << " " << cFC.frameIdx << "  Frame 2 : " << vCurrentFC[iSecondFrame].cameraID << " " << vCurrentFC[iSecondFrame].frameIdx << " /" << vPoint1.size() << endl;
				cvWaitKey(50);
				cvReleaseImage(&tmpsize);
				cvReleaseImage(&iplImg);
			}	
		}
		cout << "Frame 1 : " << cFC.cameraID << " " << cFC.frameIdx << "  Frame 2 : " << vCurrentFC[iSecondFrame].cameraID << " " << vCurrentFC[iSecondFrame].frameIdx << " /" << vPoint1.size() << endl;
	}
	features1->remove_ref();
}

void Iterate_SIFT_STATIC_SUB(FrameCamera cFC, vector<FrameCamera> vCurrentFC, vector<Point> &featureSequence, vector<Feature> &feature, int nSubx, int nSuby, bool display)
{
	Image img1, img2;
	IplImage *originalImage1, *originalImage2;
	originalImage1 = cvLoadImage(cFC.imageFileName.c_str());	
	int width = (int) (double)originalImage1->width/nSubx;
	int height = (int) (double)originalImage1->height/nSuby;

	for (int iSecondFrame = 0; iSecondFrame < vCurrentFC.size(); iSecondFrame++)
	{
		originalImage2 = cvLoadImage(vCurrentFC[iSecondFrame].imageFileName.c_str());
		vector<Point> vPoint1, vPoint2;
		for (int ix1 = 0; ix1 < nSubx; ix1++)
		{
			for (int iy1 = 0; iy1 < nSuby; iy1++)
			{
				// Second file
				for (int ix2 = 0; ix2 < nSubx; ix2++)
				{
					for (int iy2 = 0; iy2 < nSuby; iy2++)
					{
						int x01 = ix1*width;		int y01 = iy1*height;
						int x02 = ix2*width;		int y02 = iy2*height;
						IplImage *croppedImage1 = cvCreateImage(cvSize(width, height), 8, 3);
						cvSetImageROI(originalImage1, cvRect(x01, y01, width, height));
						cvCopy(originalImage1, croppedImage1);
						cvResetImageROI(originalImage1);
						cvSaveImage("temp1.jpg", croppedImage1);
						cvReleaseImage(&croppedImage1);
						img1.read_file("temp1.jpg");
						IplImage *croppedImage2 = cvCreateImage(cvSize(width, height), 8, 3);
						cvSetImageROI(originalImage2, cvRect(x02, y02, width, height));
						cvCopy(originalImage2, croppedImage2);
						cvResetImageROI(originalImage2);
						cvSaveImage("temp2.jpg", croppedImage2);
						cvReleaseImage(&croppedImage2);
						img2.read_file("temp2.jpg");

						ObjRecKeypointList* features1 = new ObjRecKeypointList ();
						features1->set_flag("max-num-features", SIFT_MAX_NUM_FEATURES_STATIC);		features1->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
						features1->set_flag("feature-strength", SIFT_FEATURE_STRENGTH_STATIC);		features1->set_flag("use-upsampling-method", SIFT_UPSAMPLING_MODE_STATIC);
						//features1->set_flag("feature-type", "small");	
						features1->extract(&img1);
						ObjRecKeypointList* features2 = new ObjRecKeypointList ();
						features2->set_flag("max-num-features", SIFT_MAX_NUM_FEATURES_STATIC);			features2->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
						features2->set_flag("feature-strength", SIFT_FEATURE_STRENGTH_STATIC);			features2->set_flag("use-upsampling-method", SIFT_UPSAMPLING_MODE_STATIC);
						//features2->set_flag("feature-type", "small");	
						features2->extract(&img2);

						vector<Point> vpt1, vpt2;
						//////////////////////////////////////////////////////////////////////
						// SIFT
						vector<Point> vPoint11, vPoint12, vPoint21, vPoint22;
						ObjRecDatabase *database1 = new ObjRecDatabase();
						database1->add_model("temp1", features1);
						database1->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
						//int ret = database1->set_flag("feature-type", "small");
						GetCorrespondences(database1, features2, vPoint11, vPoint12);

						ObjRecDatabase *database2 = new ObjRecDatabase();
						database2->add_model("temp2", features2);
						database2->set_flag("feature-quality", SIFT_FEATURE_QUALITY_STATIC);
						//database2->set_flag("feature-type", "small");
						GetCorrespondences(database2, features1, vPoint22, vPoint21);
						database1->remove_ref();
						database2->remove_ref();
						features2->remove_ref();
						GetAppointingCorrespondences(vPoint11, vPoint12, vPoint21, vPoint22, vpt1, vpt2);
						cout << "(" << ix1 << "," << iy1 << ") " << "(" << ix2 << "," << iy2 << ") " << vpt1.size() << " " << vpt2.size() << endl;
						//vPoint1 = vPoint11;
						//vPoint2 = vPoint12;

						for (int ipoint = 0; ipoint < vpt1.size(); ipoint++)
						{
							Point p1, p2;
							p1.x = vpt1[ipoint].x + x01;
							p1.y = vpt1[ipoint].y + y01;
							p2.x = vpt2[ipoint].x + x02;
							p2.y = vpt2[ipoint].y + y02;
							vPoint1.push_back(p1);
							vPoint2.push_back(p2);
						}
					}
				}
			}
		}
		if (vPoint1.size() < 7)
			continue;
		if (GetStaticCorrespondences(vPoint1, vPoint2, vPoint1, vPoint2) == 0)
			continue;
		///////////////////////////////////////////////////////////////////////
		// Registration
		for (vector<Point>::iterator iit = vPoint1.begin(); iit < vPoint1.end(); iit++)
		{
			vector<Point>::const_iterator it = search(featureSequence.begin(),featureSequence.end(),iit, iit+1, IsSamePoint);
			int iPoint = int(iit-vPoint1.begin());
			if (it != featureSequence.end())
			{
				int idx = int(it-featureSequence.begin());

				feature[idx].vx.push_back(vPoint2[iPoint].x);
				feature[idx].vy.push_back(vPoint2[iPoint].y);
				feature[idx].vCamera.push_back(vCurrentFC[iSecondFrame].cameraID);
				feature[idx].vFrame.push_back(vCurrentFC[iSecondFrame].takenFrame);
			}
			else
			{
				Feature fs;
				fs.vCamera.push_back(cFC.cameraID);
				fs.vCamera.push_back(vCurrentFC[iSecondFrame].cameraID);
				fs.vFrame.push_back(cFC.takenFrame);
				fs.vFrame.push_back(vCurrentFC[iSecondFrame].takenFrame);
				fs.vx.push_back(vPoint1[iPoint].x);
				fs.vx.push_back(vPoint2[iPoint].x);
				fs.vy.push_back(vPoint1[iPoint].y);
				fs.vy.push_back(vPoint2[iPoint].y);
				feature.push_back(fs);

				featureSequence.push_back(vPoint1[iPoint]);
			}
		}

		if (display)
		{
			IplImage *iplImg = cvLoadImage(vCurrentFC[iSecondFrame].imageFileName.c_str());
			for (int i = 0; i < vPoint1.size(); i++)
			{
				//cvCircle(iplImg, cvPoint(vPoint1[i].x,(int)vPoint1[i].y), 8, cvScalar(255,0,0), 1);
				cvCircle(iplImg, cvPoint(vPoint2[i].x,(int)vPoint2[i].y), 8, cvScalar(255,0,0), 1);
				cvLine(iplImg, cvPoint(vPoint1[i].x,(int)vPoint1[i].y), cvPoint(vPoint2[i].x,(int)vPoint2[i].y), cvScalar(0,0,0), 3);
			}
			if (iplImg->width < 1024)
			{
				cvShowImage("SIFT", iplImg);
				cout << "Frame 1 : " << cFC.cameraID << " " << cFC.takenFrame << "  Frame 2 : " << vCurrentFC[iSecondFrame].cameraID << " " << vCurrentFC[iSecondFrame].takenFrame << endl;
				cvWaitKey(0);
				cvReleaseImage(&iplImg);
			}
			else
			{
				double scale = (double)(iplImg->width)/1024;
				CvSize size = cvSize((int)(iplImg->width/scale),(int)(iplImg->height/scale)); 
				IplImage* tmpsize=cvCreateImage(size,IPL_DEPTH_8U,3);
				cvResize(iplImg,tmpsize,CV_INTER_LINEAR); 
				cvShowImage("SIFT", tmpsize);
				cout << "Frame 1 : " << cFC.cameraID << " " << cFC.takenFrame << "  Frame 2 : " << vCurrentFC[iSecondFrame].cameraID << " " << vCurrentFC[iSecondFrame].takenFrame << endl;
				cvWaitKey(50);
				cvReleaseImage(&tmpsize);
				cvReleaseImage(&iplImg);
			}	
		}
		cvReleaseImage(&originalImage2);
	}
	cvReleaseImage(&originalImage1);
	
}

void Iterate_SIFT_DYNAMIC(FrameCamera cFC, vector<FrameCamera> vCurrentFC, vector<Point> &featureSequence, vector<Feature> &feature, vector<DynamicObjectWindow> vDW, bool display)
{
	Image img1, img2;
	img1.read_file(cFC.imageFileName_d.c_str());
	ObjRecKeypointList* features1 = new ObjRecKeypointList ();
	features1->set_flag("max-num-features", SIFT_MAX_NUM_FEATURES_DYNAMIC);		features1->set_flag("feature-quality", SIFT_FEATURE_QUALITY_DYNAMIC);
	features1->set_flag("feature-strength", SIFT_FEATURE_STRENGTH_DYNAMIC);		features1->set_flag("use-upsampling-method", SIFT_UPSAMPLING_MODE_DYNAMIC);
	features1->extract(&img1);
	DynamicObjectWindow dw1, dw2;
	for (int iDW = 0; iDW < vDW.size(); iDW++)
	{
		if (vDW[iDW].croppedImage == cFC.imageFileName_d)
		{
			dw1 = vDW[iDW];
			break;
		}
	}

	for (int iSecondFrame = 0; iSecondFrame < vCurrentFC.size(); iSecondFrame++)
	{
		img2.read_file(vCurrentFC[iSecondFrame].imageFileName_d.c_str());
		ObjRecKeypointList* features2 = new ObjRecKeypointList ();
		features2->set_flag("max-num-features", SIFT_MAX_NUM_FEATURES_DYNAMIC);			features2->set_flag("feature-quality", SIFT_FEATURE_QUALITY_DYNAMIC);
		features2->set_flag("feature-strength", SIFT_FEATURE_STRENGTH_DYNAMIC);			features2->set_flag("use-upsampling-method", SIFT_UPSAMPLING_MODE_DYNAMIC);
		features2->extract(&img2);

		for (int iDW = 0; iDW < vDW.size(); iDW++)
		{
			if (vDW[iDW].croppedImage == vCurrentFC[iSecondFrame].imageFileName_d)
			{
				dw2 = vDW[iDW];
				break;
			}
		}

		vector<Point> vPoint1, vPoint2;
		if(!DoSIFT_DYNAMIC(cFC.imageFileName_d, features1, vCurrentFC[iSecondFrame].imageFileName_d, features2, vPoint1, vPoint2))
			continue;
		for (int iPoint = 0; iPoint < vPoint1.size(); iPoint++)
		{
			vPoint1[iPoint].x += dw1.x1;
			vPoint1[iPoint].y += dw1.y1;

			vPoint2[iPoint].x += dw2.x1;
			vPoint2[iPoint].y += dw2.y1;
		}

		// Registration
		for (vector<Point>::iterator iit = vPoint1.begin(); iit < vPoint1.end(); iit++)
		{
			vector<Point>::const_iterator it = search(featureSequence.begin(),featureSequence.end(),iit, iit+1, IsSamePoint);
			int iPoint = int(iit-vPoint1.begin());
			if (it != featureSequence.end())
			{
				int idx = int(it-featureSequence.begin());

				feature[idx].vx.push_back(vPoint2[iPoint].x);
				feature[idx].vy.push_back(vPoint2[iPoint].y);
				feature[idx].vCamera.push_back(vCurrentFC[iSecondFrame].cameraID);
				feature[idx].vFrame.push_back(vCurrentFC[iSecondFrame].takenFrame);
			}
			else
			{
				Feature fs;
				fs.vCamera.push_back(cFC.cameraID);
				fs.vCamera.push_back(vCurrentFC[iSecondFrame].cameraID);
				fs.vFrame.push_back(cFC.takenFrame);
				fs.vFrame.push_back(vCurrentFC[iSecondFrame].takenFrame);
				fs.vx.push_back(vPoint1[iPoint].x);
				fs.vx.push_back(vPoint2[iPoint].x);
				fs.vy.push_back(vPoint1[iPoint].y);
				fs.vy.push_back(vPoint2[iPoint].y);
				feature.push_back(fs);

				featureSequence.push_back(vPoint1[iPoint]);
			}
		}

		if (display)
		{
			IplImage *iplImg = cvLoadImage(vCurrentFC[iSecondFrame].imageFileName.c_str());
			cvRectangle(iplImg, cvPoint(dw2.x1,dw2.y1), cvPoint(dw2.x2,dw2.y2), cvScalar(255,0,0), 3);
			IplImage *iplImg1 = cvLoadImage(dw1.croppedImage.c_str());
			
			for (int i = 0; i < vPoint1.size(); i++)
			{
				//cvCircle(iplImg, cvPoint(vPoint1[i].x,(int)vPoint1[i].y), 8, cvScalar(255,0,0), 1);
				cvCircle(iplImg, cvPoint(vPoint2[i].x,(int)vPoint2[i].y), 8, cvScalar(255,0,0), 1);
				cvLine(iplImg, cvPoint(vPoint1[i].x,(int)vPoint1[i].y), cvPoint(vPoint2[i].x,(int)vPoint2[i].y), cvScalar(0,0,0), 3);

				cvCircle(iplImg1, cvPoint(vPoint1[i].x-dw1.x1,(int)vPoint1[i].y-dw1.y1), 8, cvScalar(255,0,0), 1);
			}
			if (iplImg->width < 1024)
			{
				cvShowImage("SIFT", iplImg);
				cout << "Frame 1 : " << cFC.cameraID << " " << cFC.takenFrame << "  Frame 2 : " << vCurrentFC[iSecondFrame].cameraID << " " << vCurrentFC[iSecondFrame].takenFrame << endl;
				cvWaitKey(500);
				cvReleaseImage(&iplImg);
			}
			else
			{
				double scale = (double)(iplImg->width)/1024;
				CvSize size = cvSize((int)(iplImg->width/scale),(int)(iplImg->height/scale)); 
				IplImage* tmpsize=cvCreateImage(size,IPL_DEPTH_8U,3);
				cvResize(iplImg,tmpsize,CV_INTER_LINEAR); 
				cvShowImage("SIFT", tmpsize);
				cvShowImage("SIFT1", iplImg1);
				cout << "Frame 1 : " << cFC.cameraID << " " << cFC.takenFrame << "  Frame 2 : " << vCurrentFC[iSecondFrame].cameraID << " " << vCurrentFC[iSecondFrame].takenFrame << endl;
				cvWaitKey(50);
				cvReleaseImage(&tmpsize);
				cvReleaseImage(&iplImg);
			}	
			cvReleaseImage(&iplImg1);
		}
	}
	features1->remove_ref();
}

bool IsSamePoint(Point p1, Point p2)
{
	if (DistancePixel(p1.x, p1.y, p2.x, p2.y) < ZERO_DISTANCE)
		return true;
	else
		return false;
}