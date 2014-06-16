#ifndef CLASSIFIER_H
#define CLASSIFIER_H
#include <cv.h>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <vector>
#include "MathUtility.h"
#include "StructDefinition.h"

using namespace std;
class Classifier
{
public:
	Classifier(void);
	void SetRansacParam(double threshold, int maxIter = 1e+2);
	void SetCorrespondance(CvMat *tx1, CvMat *tx2, vector<int> featureID);	
	void Classify();
	void EightPointAlgorithm(CvMat *x1_8, CvMat *x2_8, CvMat *F_8);
	void GetClassificationResultByFeatureID(vector<int> &vInlier, vector<int> &vOutlier);

protected:
	void Ransac(CvMat *bestF);
	void Ransac(CvMat &bestF);
public:
	~Classifier(void);

public:
	CvMat *x1, *x2;
	CvMat *F;
	double ransacThreshold;
	int ransacMaxIter;
	vector<int> vInlierIndex, vOutlierIndex;
	CvMat *inlier1, *inlier2, *outlier1, *outlier2;
	vector<int> vFeatureID;
};
#endif //CLASSIFIER_H