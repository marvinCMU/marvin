// StaticReconstruction.cpp : Defines the entry point for the console application.
//

#include <flann/flann.hpp>
#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <direct.h>
#include "../Include/DataUtility.h"
#include "../Include/MultiviewGeometryUtility.h"
#include "../Include/MathUtility.h"
#include "sift.h"

#define DLT_RANSAC_THRESHOLD 10
#define DLT_RANSAC_MAX_ITER 4e+3

//#define FILE_PATH "D:/03 Works/Data/SocialDynamics/20111117/pinpong/LeftThigh/image/"
//#define FILE_PATH "E:/Basketball2_video/1/BB/image/"
#define FILE_PATH "image/"
#define INPUT_FOLDER "../../../reconstruction/"
#define OUTPUT_FOLDER "../absolute/"

void Undistortion(CvMat *K, CvMat *invK, double omega, vector<double> &vx,  vector<double> &vy)
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

int main ( int argc, char * * argv )
{
	//////////////////////////////////////////////////////////////////////////////////////////////
	// Load data
	string path = FILE_PATH;
	string middleFolder = INPUT_FOLDER;
	string outputFolder = OUTPUT_FOLDER;
	// Input file
	string structureFile = path + middleFolder + "structure.txt";
	string descriptorFile = path + middleFolder + "descriptors.txt";
	string infoFile = path + "filelist.list";
	// Output file
	string outputPath = path + outputFolder;
	string cameraFile = path + outputFolder + "camera.txt";
	string cameraFile1 = path + outputFolder + "camera_AD.txt";
	string usedSIFTFile = path + outputFolder + "correspondence2D3D.txt";
	string usedSIFTFile_ransac = path + outputFolder + "correspondence2D3D_ransac.txt";
	mkdir (outputPath.c_str());
	vector<string> vFilename;
	LoadFileListData(infoFile, vFilename);
	
	// Load calibration data
	ifstream fin_cal;
	string calibfile = path + "calib.txt";
	fin_cal.open(calibfile.c_str(), ifstream::in);
	string dummy;
	int im_width, im_height;
	double focal_x, focal_y, princ_x, princ_y, omega;
	fin_cal >> dummy >> im_width;
	fin_cal >> dummy >> im_height;
	fin_cal >> dummy >> focal_x;
	fin_cal >> dummy >> focal_y;
	fin_cal >> dummy >> princ_x;
	fin_cal >> dummy >> princ_y;
	fin_cal >> dummy >> omega;

	CvMat *K = cvCreateMat(3,3,CV_32FC1);
	cvSetIdentity(K);
	cvSetReal2D(K, 0, 0, focal_x);
	cvSetReal2D(K, 0, 2, princ_x);
	cvSetReal2D(K, 1, 1, focal_y);
	cvSetReal2D(K, 1, 2, princ_y);
	CvMat *invK = cvCreateMat(3,3,CV_32FC1);
	cvInvert(K, invK);
	fin_cal.close();
		
	vector<double> vX, vY, vZ;
	vector<int> vID;
	vector<vector<int>> vvDesc;
	LoadStructureData(structureFile, vID, vX, vY, vZ);
	LoadDescriptorData(descriptorFile, vvDesc);

	vector<Correspondence2D3D> vCorr_temp;
	SaveCorrespondence2D3DData(usedSIFTFile, vCorr_temp, 0, FILESAVE_WRITE_MODE);
	SaveCorrespondence2D3DData(usedSIFTFile_ransac, vCorr_temp, 0, FILESAVE_WRITE_MODE);
		
	//Retrieve 3D descriptor
	vector<vector<int>> vvDesc_temp;
	for (int iDesc = 0; iDesc < vID.size(); iDesc++)
	{
		vvDesc_temp.push_back(vvDesc[vID[iDesc]]);
	}
	vvDesc = vvDesc_temp;
	vvDesc_temp.clear();

	// Construct 3D KD tree
	flann::Matrix<float> descDatabase(new float[vvDesc.size()*128], vvDesc.size(), 128);

	for (int iDesc = 0; iDesc < vvDesc.size(); iDesc++)
	{
		for (int iDim = 0; iDim < 128; iDim++)
		{
			descDatabase[iDesc][iDim] = (float) vvDesc[iDesc][iDim];
		}
	}

	flann::Index<flann::L2<float>> index_database(descDatabase, flann::KDTreeIndexParams(8));
	index_database.buildIndex();

	////////////////////////////////////////////////////////////////////////////
	// Absolute Registration
	vector<CvMat *> vP;
	vector<int> vFrame;
	vector<vector<int>> vvInlier;
	for (int iFile = 0; iFile < vFilename.size(); iFile++)
	{
		vector<SIFT_Descriptor> vSift_desc;
		LoadSIFTData(path+vFilename[iFile], vSift_desc);

		vector<double> vx1, vy1;
		for (int isift = 0; isift < vSift_desc.size(); isift++)
		{
			vx1.push_back(vSift_desc[isift].x);
			vy1.push_back(vSift_desc[isift].y);
		}
		Undistortion(K, invK, omega, vx1, vy1);
		for (int isift = 0; isift < vSift_desc.size(); isift++)
		{
			vSift_desc[isift].x = vx1[isift];
			vSift_desc[isift].y = vy1[isift];
		}
		cout << "Number of sifts: " << vSift_desc.size() << endl;
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
		index_database.knnSearch(descTarget, result, dist, nn, flann::SearchParams(256));

		descTarget.free();

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

				corr.x = vX[result[iDesc][0]];
				corr.y = vY[result[iDesc][0]];
				corr.z = vZ[result[iDesc][0]];

				corr.id_2D = vSift_desc[iDesc].id;
				corr.id_3D = vID[result[iDesc][0]];

				vCorr.push_back(corr);
			}
		}

		result.free();
		dist.free();

		SaveCorrespondence2D3DData(usedSIFTFile, vCorr, iFile, FILESAVE_APPEND_MODE);	
			
		if (vCorr.size() < 20)
		{
			cout << "No corresondence" << endl;
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
		if (DLT_ExtrinsicCameraParamEstimationWRansac_EPNP_mem_abs(cX, cx, K, P, DLT_RANSAC_THRESHOLD, DLT_RANSAC_MAX_ITER, vInlier) < 20)
		{
			cout << "No ePNP solution " << vInlier.size() << endl;
			cvReleaseMat(&cX);
			cvReleaseMat(&cx);
			cvReleaseMat(&P);
			continue;
		}

		vector<int> vInlierID;
		
		for (int iInlier = 0; iInlier < vInlier.size(); iInlier++)
		{
			vCorr_ransac.push_back(vCorr[vInlier[iInlier]]);
			vInlierID.push_back(vCorr[vInlier[iInlier]].id_3D);
		}

		CvMat *cX_ransac = cvCreateMat(vCorr_ransac.size(), 3, CV_32FC1);
		CvMat *cx_ransac = cvCreateMat(vCorr_ransac.size(), 2, CV_32FC1);

		for (int iPoint = 0; iPoint < vCorr_ransac.size(); iPoint++)
		{
			cvSetReal2D(cX_ransac, iPoint, 0, vCorr_ransac[iPoint].x);
			cvSetReal2D(cX_ransac, iPoint, 1, vCorr_ransac[iPoint].y);
			cvSetReal2D(cX_ransac, iPoint, 2, vCorr_ransac[iPoint].z);

			cvSetReal2D(cx_ransac, iPoint, 0, vCorr_ransac[iPoint].u);
			cvSetReal2D(cx_ransac, iPoint, 1, vCorr_ransac[iPoint].v);
		}		
		AbsoluteCameraPoseRefinement(cX_ransac, cx_ransac, P, K);
		//PrintMat(P);
		vP.push_back(P);
		vFrame.push_back(iFile);
		vvInlier.push_back(vInlierID);

		SaveCorrespondence2D3DData(usedSIFTFile_ransac, vCorr_ransac, iFile, FILESAVE_APPEND_MODE);	
		SaveAbsoluteCameraData(cameraFile, vP, vFrame, vFilename.size(), K);
		SaveAbsoluteCameraData_AD(cameraFile1, vP, vFrame, vFilename.size(), K, vvInlier);

		cvReleaseMat(&cX);
		cvReleaseMat(&cx);
	}

	return 0;
}
