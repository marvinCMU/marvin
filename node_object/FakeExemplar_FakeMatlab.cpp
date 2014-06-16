
/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

/*
 * Generic recognizer node template
 * change retail to recognizer name
 * yyy is the node related to retail
 */
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ros/ros.h>
#include <std_srvs/Empty.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <compressed_image_transport/compressed_subscriber.h>
//#include <compressed_image_transport/compressed_publisher.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace fs = boost::filesystem;
namespace ag = boost::algorithm;
using namespace std;

#include "SystemParams.h"
#include "RetailManager.h"

class fakeExemplar
{ //this class takes image_raw images as input, saves the images to disk, then waits for
//the matlab exemplar to write results to disk, then reads those results and emits
//them as ros messages: detected_objects which are strings of labels of detected
//objects, and detected_images and original_image which are needed by the gui.

	struct tm *ptm;
	int lastsec;
	time_t rawtime;
	bool activationFlag;
	bool serviceFlag;
	std_srvs::Empty srv;
	std::string tmpPath;
	std::string libPath;

	image_transport::ImageTransport it;
	//  compressed_image_transport::CompressedSubscriber subImg;
	//compressed_image_transport::CompressedPublisher detectedImagePub;
	//compressed_image_transport::CompressedPublisher inputImagePub;
	image_transport::Publisher inputImagePub;
	image_transport::Publisher detectedImagePub;
	image_transport::Subscriber subImg;
	ros::Subscriber rankedExemplarSub;

	ros::Publisher objectRecResultsPub;
//  ros::Publisher inputImagePub;
//  ros::Publisher detectedImagePub;

	RetailManager* manager;

public:
	fakeExemplar(ros::NodeHandle nh) :
			it(nh)
	{
		lastsec = 0;

		subImg = it.subscribe("gui/image_raw", 5, &fakeExemplar::callbackImage,
				this);
		// read recognizer parameters
		nh.getParam("retailActive", activationFlag);
		nh.getParam("retailLibPath", libPath);
		nh.getParam("tmpLibPath", tmpPath);
		serviceFlag = true;

		//pub = nh.advertise<std_msgs::String> ("retail/output", 10);
		objectRecResultsPub = nh.advertise<std_msgs::String>("retail/object_detection_result", 10);
		inputImagePub = it.advertise("exemplar/inputImage", 10);
		detectedImagePub = it.advertise("exemplar/detectedImage", 10);
		rankedExemplarSub = nh.subscribe("bnet/ranked_exemplars", 1000, &fakeExemplar::RankedExemplar_callback, this);

		// read recognizer library
		manager = new RetailManager(libPath);
		std::string commandList = manager->loadCommand();
		std::string objectList = manager->loadObject();
		ROS_INFO("Recognizer command list: ");
		ROS_INFO("%s,", commandList.c_str());
		ROS_INFO("Recognizer object list: ");
		ROS_INFO("%s,", objectList.c_str());

		// quit this node, if deactivated
		if (!activationFlag)
		{
			ROS_INFO(
					"Recognizer not activated. Not really sure what to do with this information :)");
		}
		ROS_INFO("Recognizer launched!");
	}

	~fakeExemplar()
	{
		delete manager;
	}

	// convert from ROS image format to CV Mat
	cv::Mat convertImage(const sensor_msgs::ImageConstPtr& inputFrame)
	{
		cv_bridge::CvImagePtr cv_ptr;
		cv::Mat frame;
		try
		{
			cv_ptr = cv_bridge::toCvCopy(inputFrame,
					sensor_msgs::image_encodings::BGR8);
		} catch (cv_bridge::Exception& e)
		{
			ROS_ERROR("cv_bridge exception: %s", e.what());
			return frame;
		}
		frame = cv_ptr->image;
		return frame;
	}

	sensor_msgs::ImageConstPtr convertImage(const cv::Mat& inputFrame)
	{
		cv_bridge::CvImagePtr cv_ptr;
//TODO Finish this: Look at opencv_cam to see how to construct the header for 
	}

	// start service to resume this node
	bool start(std_srvs::Empty::Request& request,
			std_srvs::Empty::Response& response)
	{
		serviceFlag = true;
		return true;
	}

	// stop service to pause this node
	bool stop(std_srvs::Empty::Request& request,
			std_srvs::Empty::Response& response)
	{
		serviceFlag = false;
		return true;
	}

	// update service to reload recognizer library
	bool update(std_srvs::Empty::Request& request,
			std_srvs::Empty::Response& response)
	{
		bool tmp = serviceFlag;
		serviceFlag = false;
		std::string commandList = manager->loadCommand();
		std::string objectList = manager->loadObject();
		ROS_INFO("Command list and object list updated:");
		ROS_INFO("Recognizer command list: ");
		ROS_INFO("%s,", commandList.c_str());
		ROS_INFO("Recognizer object list: ");
		ROS_INFO("%s,", objectList.c_str());
		//yyyUpd.call (srv);
		serviceFlag = tmp;
		return true;
	}

	// parse and change recognizer parameters
	void callbackCommand(const std_msgs::String::ConstPtr& msg)
	{
		std::string tmpStr = msg->data;
		//int index = atoi (&tmpStr.at(0));
		//recogCmd = manager->cmdVec[index];
		//std::cout << RETAIL_HEAD << "Recognizer command changed to: " << recogCmd << std::endl;
	}

	// parse and change recognizer behavior based on outputs of other recognizers (if necessary)
	void callbackMessage(const std_msgs::String::ConstPtr& msg)
	{
		std::string msgStr = msg->data;
		//msgStr = parseMessage (msgStr);
		//recogMsg = msgStr;
	}

	// parse the local format recognizer output to global format
	std::string parseOutput(std::string str)
	{
		std::string tmp = str;
		// parse ...
		return tmp;
	}

	// save frame to path
	void saveFrame(std::string path, cv::Mat frame)
	{
		char* pic_dir = new char[100];
		char* timestamp = new char[8];
		std::string logtemp;

		time(&rawtime);
		ptm = gmtime(&rawtime);
		if (ptm->tm_sec != lastsec)
		{

			char timestamp[256];
			time_t rawtime;
			struct tm * timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(timestamp, 256, "/v_%H_%M_%S.jpg", timeinfo);
			std::string pic_dir = path + std::string(timestamp);

			//sprintf(timestamp, "%02d_%02d_%02d", ptm->tm_hour, ptm->tm_min,
			//ptm->tm_sec);
			//lastsec = ptm->tm_sec;
			//sprintf(pic_dir, "%s/v_%-8s.jpg", path.c_str(), timestamp);

			if (!cv::imwrite(pic_dir, frame))
			{
				logtemp = logtemp + "<Interface> Could not save file: " + path + "\n";
				ROS_ERROR("Could not save:%s", path.c_str());
			}
			//cvCvtColor (img1, img1, CV_BGR2RGB);
		}

	}

	//When a list of ranked exemplars is received, we just ouput the list to disk
	//and wait for Matlab to process it
	void RankedExemplar_callback(const std_msgs::String::ConstPtr& msg)
	{
	  string reportData = msg->data;
	  find_and_replace(reportData,"\n","|");

   ROS_INFO("%s:%s:%s",REPORT_IN,REPORT_OBJECT_RECO,REPORT_RANKED_EXEMPLARS);

	  //	  ROS_INFO("GOT RANKED EXEMPLAR %s", msg->data.c_str());
	  stringstream infoMsg; infoMsg << "FakeExemplar saving file " << RANKED_EXEMPLARS_FILE << endl << flush;
	  ROS_INFO(infoMsg.str().c_str());
	  
		ofstream out(RANKED_EXEMPLARS_FILE.c_str());
		out << msg->data.c_str();
		out.close();

		
//--------------Done with Debugging Region
 		//Wait for matlab to finish processing image:
 //		//TODO: Get rid of this in production:
 		vector<string> exemplars;
 		ROS_INFO("FakeExemplar received msg from bnet: %s",msg->data.c_str());

 		ag::split(exemplars, msg->data, ag::is_any_of("\n"), ag::token_compress_on);
 		float timeToSleep = exemplars.size()/100; //Assume 1s per 100 exemplars
 		ROS_INFO("exemplars.size()=%i",exemplars.size());

 		ros::Rate r(1);
 		ROS_ERROR("Simulating matlab computation time at %f seconds", timeToSleep);
 		r.sleep();//usleep(100); //sleep for 100 ms.

 		ROS_ERROR("Line 251 in FakeExemplarFakeMatlab.cpp: Denver faked a matlab response! Change this in production!");
 		stringstream resultsStr;
 		for (int i=0; i<5; i++)
 		  resultsStr << "0,0,0,0,-1," << exemplars[i] << ",0,0,0,0,0,0.9\n";
 		std_msgs::String resultsMsgFake;
 		resultsMsgFake.data = resultsStr.str();
 		objectRecResultsPub.publish(resultsMsgFake);
 		return;
//--------------Done with Debugging Region

  // //After debugging, uncomment the batch comment below:

  // 		int maxSleeps =20;
  // 		int numSleeps=0;
  // 		ros::Rate r(1);
  //     while (!fs::exists(FIRED_EXEMPLARS_FILE))
  //     {
  //       ROS_INFO("Waiting for Matlab to finish %i", numSleeps);
  //       r.sleep();//usleep(100); //sleep for 100 ms.
  //       if (numSleeps++ > maxSleeps)
  //       {
  //       	ROS_WARN("Matlab did not respond in time, resetting.");
  //       	return;
  //       }
  //     }

  // 		ifstream in(FIRED_EXEMPLARS_FILE.c_str());
  // 		stringstream data;
  // 		string tmp;
  // 		while (!in.eof())
  // 		{
  // 			getline(in, tmp);
  // 			data << tmp << "\n";
  // 		}
  // 		in.close();

  // 		std_msgs::String resultsMsg;
  // 		resultsMsg.data = data.str();

  //   reportData = data.str();
  //   find_and_replace(reportData,"\n","|");

  //   ROS_INFO("%s:%s:%s:%s",REPORT_OUT,REPORT_OBJECT_RECO,REPORT_FIRED_EXEMPLARS,reportData.c_str());
  // 		objectRecResultsPub.publish(resultsMsg);

  // 		//Delete indicator file:
  // 		std::string cmd = "rm " + FIRED_EXEMPLARS_FILE;
  // 		ROS_INFO("FakeExemplar executing the commmand: %s",cmd.c_str());
  // 		system(cmd.c_str());
	}

	// callback function when receiving new frames (ROS image format)
	void callbackImage(const sensor_msgs::ImageConstPtr& inputFrame)
	{
   ROS_INFO("%s:%s:%s",REPORT_IN,REPORT_OBJECT_RECO,REPORT_IMAGE);

		if (!serviceFlag)
		{
			ROS_INFO("serviceFlag off, quitting.");
			return;
		}

		cv::Mat frame = convertImage(inputFrame);

		std::string p = tmpPath + std::string(CAMERA_OUTPUT_DIR);
		saveFrame(p, frame);
	}

		//Denver moved the code waiting for matlab to finish to the RankedExemplar_callback
		//because matlab needs taht info before it can process the image now.
//		//Temporary hack: Wait for matlab to finish, publish the results:
//		std::string resultsPath = std::string(DIR_PATH)
//				+ std::string(DETECTION_RESULTS_PATH);
//		std::string indicatorPath = std::string(DIR_PATH)
//				+ std::string(INDICATOR_FILE);
//
//		inputImagePub.publish(inputFrame);
//
////    while (!fs::exists(indicatorPath))
////    {
////      usleep(300); //sleep for 300 ms.
////    }
//		if (fs::exists(indicatorPath))
//		{
//			ROS_INFO("Found indicator file!");
//			//Delete indicator file:
//			std::string cmd = "rm " + std::string(DIR_PATH)
//					+ std::string(INDICATOR_FILE);
//			system(cmd.c_str());
//
//			ifstream resultsFile(resultsPath.c_str());
//			string resultsStr, timestamp, tmpstr;
//			if (resultsFile.is_open())
//			{
//				getline(resultsFile, timestamp);
//				getline(resultsFile, tmpstr);
//				getline(resultsFile, resultsStr);
//			}
//			ROS_INFO(
//					"Found results: (timestamp,tmpstr,resultsStr)=(%s,%s,%s)", timestamp.c_str(), tmpstr.c_str(), resultsStr.c_str());
//
//			std_msgs::String resultsMsg;
//			resultsMsg.data = resultsStr;
//			objectRecResultsPub.publish(resultsMsg);
//
//			//Get the image of the detected object:
//			string imagePath = DIR_PATH + DETECTED_IMAGES_DIR + string("m_")
//					+ timestamp + string(".jpg");
//			ROS_INFO("Looking for image %s", imagePath.c_str());
//			cv::Mat outputFrameCv = cv::imread(imagePath);
//
//			//    sensor_msgs::ImagePtr detectedImageMsg;
//			//    //TODO: Here convert image and publish message:
//
//			detectedImagePub.publish(inputFrame);
//		}
//	}
};

int main(int argc, char **argv)
{
	ros::init(argc, argv, "fake_exemplar");
	ros::NodeHandle nh;
	fakeExemplar fe(nh);

	ros::spin();
	return 0;
}

