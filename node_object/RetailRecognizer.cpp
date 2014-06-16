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
#include <ros/ros.h>
#include <std_srvs/Empty.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "SystemParams.h"
#include "RetailManager.h"

struct tm *ptm;
int	lastsec = 0;
time_t rawtime;
bool activationFlag = true;
bool serviceFlag = true;
RetailManager *manager;
ros::Publisher pub;
std_srvs::Empty srv;
std::string tmpPath = "";

// convert from ROS image format to CV Mat
cv::Mat convertImage (const sensor_msgs::ImageConstPtr& inputFrame) {
	cv_bridge::CvImagePtr cv_ptr;
	cv::Mat frame;
	try {
		cv_ptr = cv_bridge::toCvCopy (inputFrame, sensor_msgs::image_encodings::BGR8);
	} catch (cv_bridge::Exception& e) {
		std::cout << RETAIL_HEAD << "cv_bridge exception: " << e.what() << std::endl;
		return frame;
	}
	frame = cv_ptr->image;
	return frame;
}

// start service to resume this node
bool start (std_srvs::Empty::Request& request, std_srvs::Empty::Response& response) {
	serviceFlag = true;
	return true;
}

// stop service to pause this node
bool stop (std_srvs::Empty::Request& request, std_srvs::Empty::Response& response) {
	serviceFlag = false;
	return true;
}

// update service to reload recognizer library
bool update (std_srvs::Empty::Request& request, std_srvs::Empty::Response& response) {
	bool tmp = serviceFlag;
	serviceFlag = false;
	std::string commandList = manager->loadCommand ();
	std::string objectList = manager->loadObject ();
	std::cout << RETAIL_HEAD << "Updated recognizer command list: " << commandList << std::endl;
	std::cout << RETAIL_HEAD << "Updated recognizer object list: " << objectList << std::endl;
	//yyyUpd.call (srv);
	serviceFlag = tmp;
	return true;
}

// parse and change recognizer parameters
void callbackCommand (const std_msgs::String::ConstPtr& msg) {
	std::string tmpStr = msg->data;
	//int index = atoi (&tmpStr.at(0));
	//recogCmd = manager->cmdVec[index];
	//std::cout << RETAIL_HEAD << "Recognizer command changed to: " << recogCmd << std::endl;
}

// parse and change recognizer behavior based on outputs of other recognizers (if necessary)
void callbackMessage (const std_msgs::String::ConstPtr& msg) {
	std::string msgStr = msg->data;
	//msgStr = parseMessage (msgStr);
	//recogMsg = msgStr;
}

// parse the local format recognizer output to global format
std::string parseOutput (std::string str) {
	std::string tmp = str;
	// parse ...
	return tmp;
}

// save frame to path
void saveFrame (std::string path, cv::Mat frame) {
	char* pic_dir = new char[100];
	char* timestamp = new char[8];
	std::string logtemp;
	
	time(&rawtime);
	ptm = gmtime(&rawtime); 
	if (ptm->tm_sec!=lastsec) {

		sprintf(timestamp, "%02d_%02d_%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		lastsec = ptm->tm_sec;
		sprintf(pic_dir, "%s/v_%-8s.jpg", path.c_str(), timestamp);
		
		if(!cv::imwrite(pic_dir,frame)) {
			logtemp = logtemp + "<Interface> Could not save file: " + path + "\n";
		    std::cout << RETAIL_HEAD << "Could not save:" << path << std::endl;
		}
		//cvCvtColor (img1, img1, CV_BGR2RGB);
	}

}

// callback function when receiving new frames (ROS image format)
void callbackImage (const sensor_msgs::ImageConstPtr& inputFrame) {
	if (!serviceFlag) return;
	cv::Mat frame = convertImage (inputFrame);

	std::string p = tmpPath + VIDEO_TMP;
	saveFrame(p, frame);
	// call recognizers to do recognition
	//std::string result = recognizer.recog (frame, recogCmd);
	//std::cout << RETAIL_HEAD << result << std::endl;
	// parse result to global format
	//result = parseOutput (result);
	//std::stringstream ss;
	//ss << result;
	//std_msgs::String msg;
	//msg.data = ss.str ();
	//pub.publish (msg);
	//cv::waitKey (10);
}

int main (int argc, char **argv) {
	ros::init (argc, argv, "retail_main");
	ros::NodeHandle nh;
	image_transport::ImageTransport it (nh);
	std::string libPath = "";
	
	// read recognizer parameters
	nh.getParam ("retailActive", activationFlag);
	nh.getParam ("retailLibPath", libPath);
	nh.getParam ("tmpLibPath", tmpPath);

	// quit this node, if deactivated
	if (!activationFlag) {
		std::cout << RETAIL_HEAD << "Recognizer not activated" << std::endl;
		return 0;
	}

	// read recognizer library
	manager = new RetailManager (libPath);
	std::string commandList = manager->loadCommand ();
	std::string objectList = manager->loadObject ();
	std::cout << RETAIL_HEAD << "Recognizer command list: " << commandList << std::endl;
	std::cout << RETAIL_HEAD << "Recognizer object list: " << objectList << std::endl;

	// initialize recognizer
	//recognizer = new retailRecognizer (manager, retailLibPath, localVar1, localVar2);

	// subscribers and publisher for node self
	//ros::Subscriber subCmd = nh.subscribe ("other_node/command", 10, callbackCommand);
	//ros::Subscriber subMsg = nh.subscribe ("other_node/message", 10, callbackMessage);
	image_transport::Subscriber subImg = it.subscribe ("gui/image_raw", 5, callbackImage);
	//pub = nh.advertise<std_msgs::String> ("retail/output", 10);

	// service to pause, resume and update node self
	//ros::ServiceServer srvSrt = nh.advertiseService ("retail/start", start);
	//ros::ServiceServer srvStp = nh.advertiseService ("retail/stop", stop);
	//ros::ServiceServer srvUpd = nh.advertiseService ("retail/update", update);

	// service to control other nodes
	//yyySrt = nh.serviceClient<std_srvs::Empty>("yyy/retailstart");
	//yyyUpd = nh.serviceClient<std_srvs::Empty>("yyy/retailupdate");

	std::cout << RETAIL_HEAD << "Recognizer launched" << std::endl;
	ros::spin();
	return 0;
}

