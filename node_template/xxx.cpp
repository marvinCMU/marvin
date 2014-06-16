/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

/*
 * Generic recognizer node template
 * change xxx to recognizer name
 * yyy is the node related to xxx
 */

#include <ros/ros.h>
#include <std_srvs/Empty.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>

#include "SystemParams.h"
#include "xxxRecognizer.h"
#include "xxxManager.h"

bool activationFlag = true;
bool serviceFlag = true;
xxxRecognizer *recognizer;
xxxManager *manager;
std::string recogCmd = "commandxxx";
std::string recogMsg = "messagexxx";
ros::Publisher pub;
std_srvs::Empty srv;
ros::ServiceClient yyySrt;
ros::ServiceClient yyyUpd;

// convert from ROS image format to CV Mat
cv::Mat convertImage (const sensor_msgs::ImageConstPtr& inputFrame) {
	cv_bridge::CvImagePtr cv_ptr;
	cv::Mat frame;
	try {
		cv_ptr = cv_bridge::toCvCopy (inputFrame, sensor_msgs::image_encodings::BGR8);
	} catch (cv_bridge::Exception& e) {
		std::cout << XXX_HEAD << "cv_bridge exception: " << e.what() << std::endl;
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
	std::cout << XXX_HEAD << "Updated recognizer command list: " << commandList << std::endl;
	std::cout << XXX_HEAD << "Updated recognizer object list: " << objectList << std::endl;
	yyyUpd.call (srv);
	serviceFlag = tmp;
	return true;
}

// parse and change recognizer parameters
void callbackCommand (const std_msgs::String::ConstPtr& msg) {
	std::string tmpStr = msg->data;
	int index = atoi (&tmpStr.at(0));
	recogCmd = manager->cmdVec[index];
	std::cout << XXX_HEAD << "Recognizer command changed to: " << recogCmd << std::endl;
}

// parse and change recognizer behavior based on outputs of other recognizers (if necessary)
void callbackMessage (const std_msgs::String::ConstPtr& msg) {
	std::string msgStr = msg->data;
	msgStr = parseMessage (msgStr);
	recogMsg = msgStr;
}

// parse the local format recognizer output to global format
std::string parseOutput (std::string str) {
	std::string tmp = str;
	// parse ...
	return tmp;
}

// callback function when receiving new frames (ROS image format)
void callbackImage (const sensor_msgs::ImageConstPtr& inputFrame) {
	if (!serviceFlag) return;
	cv::Mat frame = convertImage (inputFrame);
	// call recognizers to do recognition
	std::string result = recognizer.recog (frame, recogCmd);
	cout << XXX_HEAD << result << endl;
	// parse result to global format
	result = parseOutput (result);
	std::stringstream ss;
	ss << result;
	std_msgs::String msg;
	msg.data = ss.str ();
	pub.publish (msg);
	// activate other nodes
	yyySrt.call (srv);
	cv::waitKey (10);
}

int main (int argc, char **argv) {
	ros::init (argc, argv, "xxx_main");
	ros::NodeHandle nh;
	image_transport::ImageTransport it (nh);
	std::string xxxLibPath = "";
	std::string localVar1 = "";
	std::string localVar2 = "";

	// read recognizer parameters
	nh.getParam ("xxxRecog", activationFlag);
	nh.getParam ("xxxLibPath", libPath);
	nh.getParam ("/xxx/localVar1", localVar1);
	nh.getParam ("/xxx/localVar2", localVar2);

	// quit this node, if deactivated
	if (!activationFlag) {
		std::cout << XXX_HEAD << "Recognizer not activated" << std::endl;
		return 0;
	}

	// read recognizer library
	manager = new xxxManager (libPath);
	std::string commandList = manager->loadCommand ();
	std::string objectList = manager->loadObject ();
	std::cout << XXX_HEAD << "Recognizer command list: " << commandList << std::endl;
	std::cout << XXX_HEAD << "Recognizer object list: " << objectList << std::endl;

	// initialize recognizer
	recognizer = new xxxRecognizer (manager, xxxLibPath, localVar1, localVar2);

	// subscribers and publisher for node self
	ros::Subscriber subCmd = nh.subscribe ("other_node/command", 10, callbackCommand);
	ros::Subscriber subMsg = nh.subscribe ("other_node/message", 10, callbackMessage);
	image_transport::Subscriber subImg = it.subscribe ("camera/image_raw", 5, callbackImage);
	pub = nh.advertise<std_msgs::String> ("xxx/output", 10);

	// service to pause, resume and update node self
	ros::ServiceServer srvSrt = nh.advertiseService ("xxx/start", start);
	ros::ServiceServer srvStp = nh.advertiseService ("xxx/stop", stop);
	ros::ServiceServer srvUpd = nh.advertiseService ("xxx/update", update);

	// service to control other nodes
	yyySrt = nh.serviceClient<std_srvs::Empty>("yyy/xxxstart");
	yyyUpd = nh.serviceClient<std_srvs::Empty>("yyy/xxxupdate");

	std::cout << XXX_HEAD << "Recognizer launched" << std::endl;
	ros::spin();
	return 0;
}

