#ifndef SIFTUTILITY_H
#define SIFTUTILITY_H

#include "MathUtility.h"
#include "StructDefinition.h"
#include "Classifier.h"
#include <fstream>
#include <iostream>
#include <string>
#include <evolution/Base.hpp>
#include <evolution/Resource.hpp>
#include <evolution/ObjRec.hpp>
#include <evolution/Gui.hpp>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

// ERSP
#define SIFT_FEATURE_QUALITY_STATIC 0.9//0.5
#define SIFT_FEATURE_STRENGTH_STATIC 1//0.9
#define SIFT_UPSAMPLING_MODE_STATIC false
#define SIFT_MAX_NUM_FEATURES_STATIC 0

#define SIFT_FEATURE_QUALITY_DYNAMIC 0.99
#define SIFT_FEATURE_STRENGTH_DYNAMIC 1
#define SIFT_UPSAMPLING_MODE_DYNAMIC true
#define SIFT_MAX_NUM_FEATURES_DYNAMIC 0

#define SIFT_DETECTION_THRESHOLD 0.9//0.99
#define SIFT_MIN_KEYPOINTS 4

#define ZERO_DISTANCE 1e+0

using namespace Evolution;
using namespace GUIUtils;
using namespace std;
void GetCorrespondences(ObjRecDatabase *database, ObjRecKeypointList* features, vector<Point> &vPoint1, vector<Point> &vPoint2);
void GetAppointingCorrespondences(vector<Point> vPoint11, vector<Point> vPoint12, vector<Point> vPoint21, vector<Point> vPoint22, vector<Point> &vPoint1, vector<Point> &vPoint2);
int GetStaticCorrespondences(vector<Point> x1, vector<Point> x2, vector<Point> &newx1, vector<Point> &newx2);
//int GetStaticCorrespondences(vector<Point> x1, vector<Point> x2, vector<int> &isInlier, int &nInlier);
int DoSIFT(string modelname1, ObjRecKeypointList* features1, string modelname2, ObjRecKeypointList* features2, vector<Point> &vPoint1, vector<Point> &vPoint2);
int DoSIFT_DYNAMIC(string modelname1, ObjRecKeypointList* features1, string modelname2, ObjRecKeypointList* features2, vector<Point> &vPoint1, vector<Point> &vPoint2);
void Iterate_SIFT_STATIC(FrameCamera cFC, vector<FrameCamera> vCurrentFC, vector<Point> &featureSequence, vector<Feature> &feature, bool display = 1);
void Iterate_SIFT_DYNAMIC(FrameCamera cFC, vector<FrameCamera> vCurrentFC, vector<Point> &featureSequence, vector<Feature> &feature, vector<DynamicObjectWindow> vDW, bool display = 1);
bool IsSamePoint(Point p1, Point p2);
void Iterate_SIFT_STATIC_SUB(FrameCamera cFC, vector<FrameCamera> vCurrentFC, vector<Point> &featureSequence, vector<Feature> &feature, int nSubx, int nSuby, bool display);
int GetStaticCorrespondences(vector<Point> x1, vector<Point> x2, vector<bool> &vIsInlier);


#endif //SIFTUTILITY_H
