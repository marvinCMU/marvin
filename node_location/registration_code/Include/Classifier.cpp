#include "Classifier.h"
#include <time.h>
#include <vector>
#include <math.h>
#include <algorithm>

Classifier::Classifier(void)
{
}

void Classifier::Classify()
{
	CvMat bestF;	
	Ransac(bestF);
	F = cvCloneMat(&bestF);
}

void Classifier::SetCorrespondance(CvMat *tx1, CvMat *tx2, vector<int> featureID)
{
	x1 = cvCloneMat(tx1);
	x2 = cvCloneMat(tx2);
	vFeatureID = featureID;
}

void Classifier::SetRansacParam(double threshold, int maxIter /* = 1e+2 */)
{
	ransacThreshold = threshold;
	ransacMaxIter = maxIter;
}

void Classifier::Ransac(CvMat *bestF)
{
	CvMat *T1=cvCreateMat(3,3,CV_32FC1), *xTilde1=cvCreateMat(x1->rows,2,CV_32FC1);
	CvMat *T2=cvCreateMat(3,3,CV_32FC1), *xTilde2=cvCreateMat(x2->rows,2,CV_32FC1);
	Normalization(x1, xTilde1, T1);
	Normalization(x2, xTilde2, T2);

	vInlierIndex.clear();
	vOutlierIndex.clear();

	vector<int> vInlier, vOutlier;
	int maxInlier = 0;
	for (int iRansacIter = 0; iRansacIter < ransacMaxIter; iRansacIter++)
	{
		srand (time(NULL));
		int nInitial = 10;
		int *randIdx = (int *) malloc(nInitial * sizeof(int));// = {10, 8, 32, 39, 58, 66, 76, 87, 19, 39, 1, 38, 98, 24};
		for (int iIdx = 0; iIdx < nInitial; iIdx++)
			randIdx[iIdx] = rand()%x1->rows;
		//randIdx[0] = 0;	randIdx[1] = 1;	randIdx[2] = 2;
		//randIdx[3] = 3;	randIdx[4] = 4;	randIdx[5] = 5;
		//randIdx[6] = 6;	randIdx[7] = 7;

		CvMat *randx1 = cvCreateMat(nInitial, 2, CV_32FC1);
		CvMat *randx2 = cvCreateMat(nInitial, 2, CV_32FC1);
		for (int iIdx = 0; iIdx < nInitial; iIdx++)
		{
			cvSetReal2D(randx1, iIdx, 0, cvGetReal2D(xTilde1, randIdx[iIdx], 0));
			cvSetReal2D(randx1, iIdx, 1, cvGetReal2D(xTilde1, randIdx[iIdx], 1));
			cvSetReal2D(randx2, iIdx, 0, cvGetReal2D(xTilde2, randIdx[iIdx], 0));
			cvSetReal2D(randx2, iIdx, 1, cvGetReal2D(xTilde2, randIdx[iIdx], 1));
		}
		free(randIdx);
		CvMat *F_8 = cvCreateMat(3, 3, CV_32FC1);
		CvMat *F_8T = cvCreateMat(3,3,CV_32FC1);
		CvMat *FF = cvCreateMat(3, 3, CV_32FC1);
		EightPointAlgorithm(randx1, randx2, F_8);
		ScalarMul(F_8, 1/cvGetReal2D(F_8, 2,2), F_8);		
		
		//for (int i = 0; i < nInitial; i++)
		//{
		//	CvMat *trandx1 = cvCreateMat(3,1,CV_32FC1);
		//	CvMat *trandx2 = cvCreateMat(1,3,CV_32FC1);
		//	cvSetReal2D(trandx1, 0, 0, cvGetReal2D(randx1, i, 0));
		//	cvSetReal2D(trandx1, 1, 0, cvGetReal2D(randx1, i, 1));
		//	cvSetReal2D(trandx1, 2, 0, 1);
		//	cvSetReal2D(trandx2, 0, 0, cvGetReal2D(randx2, i, 0));
		//	cvSetReal2D(trandx2, 0, 1, cvGetReal2D(randx2, i, 1));
		//	cvSetReal2D(trandx2, 0, 2, 1);
		//	CvMat *dd = cvCreateMat(1,1,CV_32FC1);
		//	cvMatMul(trandx2, F_8, trandx2);
		//	cvMatMul(trandx2, trandx1, dd);
		//	cout << cvGetReal2D(dd, 0,0) << endl;
		//}
		
		cvTranspose(F_8, F_8T);
		cvMatMul(F_8T, F_8, FF);
		vInlier.clear();
		vOutlier.clear();
		// Distance function
		CvMat *D1=cvCreateMat(xTilde1->rows,1,CV_32FC1), *D2=cvCreateMat(xTilde1->rows,1,CV_32FC1), *D=cvCreateMat(xTilde1->rows,1,CV_32FC1);
		xPy_inhomo(xTilde2, xTilde1, F_8, D1);
		xPx_inhomo(xTilde1, FF, D2);
		for (int iIdx = 0; iIdx < D2->rows; iIdx++)
		{
			cvSetReal2D(D2, iIdx, 0, sqrt(cvGetReal2D(D2, iIdx, 0)));
		}
		cvDiv(D1, D2, D);
		cvMul(D, D, D);

		for (int iIdx = 0; iIdx < xTilde1->rows; iIdx++)
		{
			if (abs(cvGetReal2D(D, iIdx, 0)) < ransacThreshold)
			{
				vInlier.push_back(iIdx);	
			}
			else
			{
				vOutlier.push_back(iIdx);
			}
		}

		if (vInlier.size() > maxInlier)
		{
			maxInlier = vInlier.size();
			bestF = cvCloneMat(F_8);
			vInlierIndex = vInlier;
			vOutlierIndex = vOutlier;
		}
		cvReleaseMat(&randx1);
		cvReleaseMat(&randx2);
		cvReleaseMat(&F_8);
		cvReleaseMat(&F_8T);
		cvReleaseMat(&FF);
		cvReleaseMat(&D);
		cvReleaseMat(&D1);
		cvReleaseMat(&D2);
	}
	CvMat *T2F = cvCreateMat(3, 3, CV_32FC1), *T2T = cvCreateMat(3, 3, CV_32FC1);
	cvTranspose(T2, T2T);
	cvMatMul(T2T, bestF, T2F);
	cvMatMul(T2F, T1, bestF);

	if (vInlierIndex.size() > 0)
	{
		inlier1 = cvCreateMat(vInlierIndex.size(), 2, CV_32FC1);
		inlier2 = cvCreateMat(vInlierIndex.size(), 2, CV_32FC1);
		for (int i = 0; i < vInlierIndex.size(); i++)
		{
			CvMat *xx = cvCreateMat(1,2, CV_32FC1);
			CvMat *yy = cvCreateMat(1,2, CV_32FC1);
			GetSubMatRowwise(x1, vInlierIndex[i], vInlierIndex[i], xx);
			GetSubMatRowwise(x2, vInlierIndex[i], vInlierIndex[i], yy);
			cvSetReal2D(inlier1, i, 0, cvGetReal2D(xx, 0, 0));
			cvSetReal2D(inlier1, i, 1, cvGetReal2D(xx, 0, 1));
			cvSetReal2D(inlier2, i, 0, cvGetReal2D(yy, 0, 0));
			cvSetReal2D(inlier2, i, 1, cvGetReal2D(yy, 0, 1));
			
/*			CvMat *xxh = cvCreateMat(1,3, CV_32FC1);
			CvMat *yyh = cvCreateMat(1,3, CV_32FC1);
			Inhomo2Homo(xx, xxh);	Inhomo2Homo(yy, yyh);
			CvMat *xxhF = cvCreateMat(1, 3, CV_32FC1);
			CvMat *yyhT = cvCreateMat(3, 1, CV_32FC1);
			CvMat * xxhFyyhT = cvCreateMat(1,1, CV_32FC1);
			cvMatMul(xxh, bestF, xxhF);
			cvMatMul(xxhF, yyhT, xxhFyyhT);
			cout << vInlierIndex[i] << " " << cvGetReal2D(xxhFyyhT, 0, 0) << "  // ";	*/		
			cvReleaseMat(&xx);	cvReleaseMat(&yy);

		}
	}

	if (vOutlierIndex.size() > 0)
	{
		outlier1 = cvCreateMat(vOutlierIndex.size(), 2, CV_32FC1);
		outlier2 = cvCreateMat(vOutlierIndex.size(), 2, CV_32FC1);
		for (int i = 0; i < vOutlierIndex.size(); i++)
		{
			CvMat *xx = cvCreateMat(1,2, CV_32FC1);
			CvMat *yy = cvCreateMat(1,2, CV_32FC1);
			GetSubMatRowwise(x1, vOutlierIndex[i], vOutlierIndex[i], xx);
			GetSubMatRowwise(x2, vOutlierIndex[i], vOutlierIndex[i], yy);
			cvSetReal2D(outlier1, i, 0, cvGetReal2D(xx, 0, 0));
			cvSetReal2D(outlier1, i, 1, cvGetReal2D(xx, 0, 1));
			cvSetReal2D(outlier2, i, 0, cvGetReal2D(yy, 0, 0));
			cvSetReal2D(outlier2, i, 1, cvGetReal2D(yy, 0, 1));
/*			CvMat *xxh = cvCreateMat(1,3, CV_32FC1);
			CvMat *yyh = cvCreateMat(1,3, CV_32FC1);
			Inhomo2Homo(xx, xxh);	Inhomo2Homo(yy, yyh);
			CvMat *xxhF = cvCreateMat(1, 3, CV_32FC1);
			CvMat *yyhT = cvCreateMat(3, 1, CV_32FC1);
			CvMat * xxhFyyhT = cvCreateMat(1,1, CV_32FC1);
			cvMatMul(xxh, bestF, xxhF);
			cvMatMul(xxhF, yyhT, xxhFyyhT);
			cout << vOutlierIndex[i] << " " << cvGetReal2D(xxhFyyhT, 0, 0) << "  // ";	*/	
			cvReleaseMat(&xx);	cvReleaseMat(&yy);
		}
	}

	cvReleaseMat(&T2F);
	cvReleaseMat(&T2T);
	cvReleaseMat(&xTilde1);
	cvReleaseMat(&xTilde2);
	cvReleaseMat(&T1);
	cvReleaseMat(&T2);
}

void Classifier::Ransac(CvMat &bestF)
{
	bestF = *cvCreateMat(3,3,CV_32FC1);
	CvMat *T1=cvCreateMat(3,3,CV_32FC1), *xTilde1=cvCreateMat(x1->rows,2,CV_32FC1);
	CvMat *T2=cvCreateMat(3,3,CV_32FC1), *xTilde2=cvCreateMat(x2->rows,2,CV_32FC1);
	Normalization(x1, xTilde1, T1);
	Normalization(x2, xTilde2, T2);

	vInlierIndex.clear();
	vOutlierIndex.clear();

	vector<int> vInlier, vOutlier;
	int maxInlier = 0;
	for (int iRansacIter = 0; iRansacIter < ransacMaxIter; iRansacIter++)
	{
		srand (time(NULL));
		int nInitial = 8;
		int *randIdx = (int *) malloc(nInitial * sizeof(int));// = {10, 8, 32, 39, 58, 66, 76, 87, 19, 39, 1, 38, 98, 24};
		for (int iIdx = 0; iIdx < nInitial; iIdx++)
			randIdx[iIdx] = rand()%x1->rows;
		//randIdx[0] = 0;	randIdx[1] = 1;	randIdx[2] = 2;
		//randIdx[3] = 3;	randIdx[4] = 4;	randIdx[5] = 5;
		//randIdx[6] = 6;	randIdx[7] = 7;

		CvMat *randx1 = cvCreateMat(nInitial, 2, CV_32FC1);
		CvMat *randx2 = cvCreateMat(nInitial, 2, CV_32FC1);
		for (int iIdx = 0; iIdx < nInitial; iIdx++)
		{
			cvSetReal2D(randx1, iIdx, 0, cvGetReal2D(xTilde1, randIdx[iIdx], 0));
			cvSetReal2D(randx1, iIdx, 1, cvGetReal2D(xTilde1, randIdx[iIdx], 1));
			cvSetReal2D(randx2, iIdx, 0, cvGetReal2D(xTilde2, randIdx[iIdx], 0));
			cvSetReal2D(randx2, iIdx, 1, cvGetReal2D(xTilde2, randIdx[iIdx], 1));
		}
		free(randIdx);
		CvMat *F_8 = cvCreateMat(3, 3, CV_32FC1);
		CvMat *F_8T = cvCreateMat(3,3,CV_32FC1);
		CvMat *FF = cvCreateMat(3, 3, CV_32FC1);
		EightPointAlgorithm(randx1, randx2, F_8);
		ScalarMul(F_8, 1/cvGetReal2D(F_8, 2,2), F_8);		

		//for (int i = 0; i < nInitial; i++)
		//{
		//	CvMat *trandx1 = cvCreateMat(3,1,CV_32FC1);
		//	CvMat *trandx2 = cvCreateMat(1,3,CV_32FC1);
		//	cvSetReal2D(trandx1, 0, 0, cvGetReal2D(randx1, i, 0));
		//	cvSetReal2D(trandx1, 1, 0, cvGetReal2D(randx1, i, 1));
		//	cvSetReal2D(trandx1, 2, 0, 1);
		//	cvSetReal2D(trandx2, 0, 0, cvGetReal2D(randx2, i, 0));
		//	cvSetReal2D(trandx2, 0, 1, cvGetReal2D(randx2, i, 1));
		//	cvSetReal2D(trandx2, 0, 2, 1);
		//	CvMat *dd = cvCreateMat(1,1,CV_32FC1);
		//	cvMatMul(trandx2, F_8, trandx2);
		//	cvMatMul(trandx2, trandx1, dd);
		//	cout << cvGetReal2D(dd, 0,0) << endl;
		//}

		cvTranspose(F_8, F_8T);
		cvMatMul(F_8T, F_8, FF);
		vInlier.clear();
		vOutlier.clear();
		// Distance function
		CvMat *D1=cvCreateMat(xTilde1->rows,1,CV_32FC1), *D2=cvCreateMat(xTilde1->rows,1,CV_32FC1), *D=cvCreateMat(xTilde1->rows,1,CV_32FC1);
		xPy_inhomo(xTilde2, xTilde1, F_8, D1);
		xPx_inhomo(xTilde1, FF, D2);
		for (int iIdx = 0; iIdx < D2->rows; iIdx++)
		{
			cvSetReal2D(D2, iIdx, 0, sqrt(cvGetReal2D(D2, iIdx, 0)));
		}
		cvDiv(D1, D2, D);
		cvMul(D, D, D);

		for (int iIdx = 0; iIdx < xTilde1->rows; iIdx++)
		{
			if (abs(cvGetReal2D(D, iIdx, 0)) < ransacThreshold)
			{
				vInlier.push_back(iIdx);	
			}
			else
			{
				vOutlier.push_back(iIdx);
			}
		}

		if (vInlier.size() > maxInlier)
		{
			maxInlier = vInlier.size();
			bestF = *cvCloneMat(F_8);
			vInlierIndex = vInlier;
			vOutlierIndex = vOutlier;
		}
		cvReleaseMat(&randx1);
		cvReleaseMat(&randx2);
		cvReleaseMat(&F_8);
		cvReleaseMat(&F_8T);
		cvReleaseMat(&FF);
		cvReleaseMat(&D);
		cvReleaseMat(&D1);
		cvReleaseMat(&D2);
	}
	CvMat *T2F = cvCreateMat(3, 3, CV_32FC1), *T2T = cvCreateMat(3, 3, CV_32FC1);
	cvTranspose(T2, T2T);
	cvMatMul(T2T, &bestF, T2F);
	cvMatMul(T2F, T1, &bestF);

	if (vInlierIndex.size() > 0)
	{
		inlier1 = cvCreateMat(vInlierIndex.size(), 2, CV_32FC1);
		inlier2 = cvCreateMat(vInlierIndex.size(), 2, CV_32FC1);
		for (int i = 0; i < vInlierIndex.size(); i++)
		{
			CvMat *xx = cvCreateMat(1,2, CV_32FC1);
			CvMat *yy = cvCreateMat(1,2, CV_32FC1);
			GetSubMatRowwise(x1, vInlierIndex[i], vInlierIndex[i], xx);
			GetSubMatRowwise(x2, vInlierIndex[i], vInlierIndex[i], yy);
			cvSetReal2D(inlier1, i, 0, cvGetReal2D(xx, 0, 0));
			cvSetReal2D(inlier1, i, 1, cvGetReal2D(xx, 0, 1));
			cvSetReal2D(inlier2, i, 0, cvGetReal2D(yy, 0, 0));
			cvSetReal2D(inlier2, i, 1, cvGetReal2D(yy, 0, 1));

			/*			CvMat *xxh = cvCreateMat(1,3, CV_32FC1);
			CvMat *yyh = cvCreateMat(1,3, CV_32FC1);
			Inhomo2Homo(xx, xxh);	Inhomo2Homo(yy, yyh);
			CvMat *xxhF = cvCreateMat(1, 3, CV_32FC1);
			CvMat *yyhT = cvCreateMat(3, 1, CV_32FC1);
			CvMat * xxhFyyhT = cvCreateMat(1,1, CV_32FC1);
			cvMatMul(xxh, bestF, xxhF);
			cvMatMul(xxhF, yyhT, xxhFyyhT);
			cout << vInlierIndex[i] << " " << cvGetReal2D(xxhFyyhT, 0, 0) << "  // ";	*/		
			cvReleaseMat(&xx);	cvReleaseMat(&yy);

		}
	}

	if (vOutlierIndex.size() > 0)
	{
		outlier1 = cvCreateMat(vOutlierIndex.size(), 2, CV_32FC1);
		outlier2 = cvCreateMat(vOutlierIndex.size(), 2, CV_32FC1);
		for (int i = 0; i < vOutlierIndex.size(); i++)
		{
			CvMat *xx = cvCreateMat(1,2, CV_32FC1);
			CvMat *yy = cvCreateMat(1,2, CV_32FC1);
			GetSubMatRowwise(x1, vOutlierIndex[i], vOutlierIndex[i], xx);
			GetSubMatRowwise(x2, vOutlierIndex[i], vOutlierIndex[i], yy);
			cvSetReal2D(outlier1, i, 0, cvGetReal2D(xx, 0, 0));
			cvSetReal2D(outlier1, i, 1, cvGetReal2D(xx, 0, 1));
			cvSetReal2D(outlier2, i, 0, cvGetReal2D(yy, 0, 0));
			cvSetReal2D(outlier2, i, 1, cvGetReal2D(yy, 0, 1));
			/*			CvMat *xxh = cvCreateMat(1,3, CV_32FC1);
			CvMat *yyh = cvCreateMat(1,3, CV_32FC1);
			Inhomo2Homo(xx, xxh);	Inhomo2Homo(yy, yyh);
			CvMat *xxhF = cvCreateMat(1, 3, CV_32FC1);
			CvMat *yyhT = cvCreateMat(3, 1, CV_32FC1);
			CvMat * xxhFyyhT = cvCreateMat(1,1, CV_32FC1);
			cvMatMul(xxh, bestF, xxhF);
			cvMatMul(xxhF, yyhT, xxhFyyhT);
			cout << vOutlierIndex[i] << " " << cvGetReal2D(xxhFyyhT, 0, 0) << "  // ";	*/	
			cvReleaseMat(&xx);	cvReleaseMat(&yy);
		}
	}

	cvReleaseMat(&T2F);
	cvReleaseMat(&T2T);
	cvReleaseMat(&xTilde1);
	cvReleaseMat(&xTilde2);
	cvReleaseMat(&T1);
	cvReleaseMat(&T2);
}


void Classifier::EightPointAlgorithm(CvMat *x1_8, CvMat *x2_8, CvMat *F_8)
{
	CvMat *A = cvCreateMat(x1_8->rows, 9, CV_32FC1);
	CvMat *U = cvCreateMat(x1_8->rows, x1_8->rows, CV_32FC1);
	CvMat *D = cvCreateMat(x1_8->rows, 9, CV_32FC1);
	CvMat *V = cvCreateMat(9, 9, CV_32FC1);
	for (int iIdx = 0; iIdx < x1_8->rows; iIdx++)
	{
		double x11 = cvGetReal2D(x1_8, iIdx, 0);
		double x12 = cvGetReal2D(x1_8, iIdx, 1);
		double x21 = cvGetReal2D(x2_8, iIdx, 0);
		double x22 = cvGetReal2D(x2_8, iIdx, 1);
		cvSetReal2D(A, iIdx, 0, x21*x11);
		cvSetReal2D(A, iIdx, 1, x21*x12);
		cvSetReal2D(A, iIdx, 2, x21);
		cvSetReal2D(A, iIdx, 3, x22*x11);
		cvSetReal2D(A, iIdx, 4, x22*x12);
		cvSetReal2D(A, iIdx, 5, x22);
		cvSetReal2D(A, iIdx, 6, x11);
		cvSetReal2D(A, iIdx, 7, x12);
		cvSetReal2D(A, iIdx, 8, 1);
	}

	cvSVD(A, D, U, V, 0);
	cvSetReal2D(F_8, 0, 0, cvGetReal2D(V, 0, 8));	cvSetReal2D(F_8, 0, 1, cvGetReal2D(V, 1, 8));	cvSetReal2D(F_8, 0, 2, cvGetReal2D(V, 2, 8));
	cvSetReal2D(F_8, 1, 0, cvGetReal2D(V, 3, 8));	cvSetReal2D(F_8, 1, 1, cvGetReal2D(V, 4, 8));	cvSetReal2D(F_8, 1, 2, cvGetReal2D(V, 5, 8));
	cvSetReal2D(F_8, 2, 0, cvGetReal2D(V, 6, 8));	cvSetReal2D(F_8, 2, 1, cvGetReal2D(V, 7, 8));	cvSetReal2D(F_8, 2, 2, cvGetReal2D(V, 8, 8));

	CvMat *UD, *Vt;
	U = cvCreateMat(3, 3, CV_32FC1);
	D = cvCreateMat(3, 3, CV_32FC1);
	V = cvCreateMat(3, 3, CV_32FC1);
	UD = cvCreateMat(U->rows, D->cols, CV_32FC1);
	Vt = cvCreateMat(V->cols, V->rows, CV_32FC1);

	cvSVD(F_8, D, U, V, 0);
	cvSetReal2D(D, 2, 2, 0);
	
	cvMatMul(U, D, UD);
	cvTranspose(V, Vt);
	cvMatMul(UD, Vt, F_8);
	
	cvReleaseMat(&UD);
	cvReleaseMat(&Vt);
	cvReleaseMat(&A);
	cvReleaseMat(&U);
	cvReleaseMat(&D);
	cvReleaseMat(&V);
}

void Classifier::GetClassificationResultByFeatureID(vector<int> &vInlier, vector<int> &vOutlier)
{
	vInlier.clear();
	vOutlier.clear();
	for (int i = 0; i < vFeatureID.size(); i++)
	{
		vector<int>::const_iterator it = find(vInlierIndex.begin(), vInlierIndex.end(), i);
		if (it != vInlierIndex.end())
			vInlier.push_back(vFeatureID[i]);
		else
			vOutlier.push_back(vFeatureID[i]);
	}
}

Classifier::~Classifier(void)
{
	cvReleaseMat(&x1);
	cvReleaseMat(&x2);
	cvReleaseMat(&F);
	if (vInlierIndex.size() > 0)
	{
		cvReleaseMat(&inlier1);
		cvReleaseMat(&inlier2);
	}
	if (vOutlierIndex.size() > 0)
	{
		cvReleaseMat(&outlier1);
		cvReleaseMat(&outlier2);
	}
	vInlierIndex.clear();
	vOutlierIndex.clear();
	vFeatureID.clear();
}