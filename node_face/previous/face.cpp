/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

#include <ros/ros.h>
#include <std_srvs/Empty.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>

#include "SystemParams.h"
#include "denver_facedetector.hpp"
#include "FaceManager.h"

bool activationFlag = true;
bool serviceFlag = true;
FaceDetectionizer fd;
FaceManager *manager;
std::string recogCmd = "get_label";
std::string recogMsg = "";
ros::Publisher pub;
std_srvs::Empty srv;
ros::ServiceClient bnetSrt;
ros::ServiceClient bnetUpd;
ros::ServiceClient spehUpd;
static const char faceWindow[] = "Face Image";

// convert from ROS image format to CV Mat
cv::Mat convertImage (const sensor_msgs::ImageConstPtr& inputFrame) {
	cv_bridge::CvImagePtr cv_ptr;
	cv::Mat frame;
	try {
		cv_ptr = cv_bridge::toCvCopy (inputFrame, sensor_msgs::image_encodings::BGR8);
	} catch (cv_bridge::Exception& e) {
		std::cout << FACE_HEAD << "cv_bridge exception: " << e.what() << std::endl;
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
	std::cout << FACE_HEAD << "Updated recognizer command list: " << commandList << std::endl;
	std::cout << FACE_HEAD << "Updated recognizer object list: " << objectList << std::endl;
	bnetUpd.call (srv);
	spehUpd.call (srv);
	serviceFlag = tmp;
	return true;
}

// parse and change recognizer parameters
void callbackCommand (const std_msgs::String::ConstPtr& msg) {
	std::string tmpStr = msg->data;
	int index = atoi (&tmpStr.at(0));
	recogCmd = manager->cmdVec[index];
	if (recogCmd.compare("take_data") == 0) {
		tmpStr.erase(0, 1);
		if ( tmpStr == "" || tmpStr == " ")
			return;
		recogCmd = recogCmd + tmpStr;
	}
	std::cout << FACE_HEAD << "Recognizer command changed to: " << recogCmd << std::endl;
}

// parse and change recognizer behavior based on outputs of other recognizers (if necessary)
void callbackMessage (const std_msgs::String::ConstPtr& msg) {
	std::string msgStr = msg->data;
	// parse ...
	recogMsg = msgStr;
}

// parse the local format recognizer output to global format
std::string parseOutput (std::string str) {
	std::string tmp = str;
	// parse ...
	return tmp;
}

// callback function when receiving new frames (ROS image format)
void callbackImage(const sensor_msgs::ImageConstPtr& inputFrame) {
	if (!serviceFlag) return;
	cv::Mat frame = convertImage (inputFrame);
	cv::imshow (faceWindow, frame);
	// call recognizers to do recognition
	std::string result;
	Rect pos;
	pos.width=0;
    pos.height=0;
	bool faceDetected = face_detectionizer_ros(fd, recogCmd, frame, result, pos);
	// parse result to global format
	if (!faceDetected) {
		result = "No face detected";
		std::cout << FACE_HEAD << result << std::endl;
		//cv::rectangle(frame, pos, CV_RGB(0,128,255), 3, 8, 0 );
		//cv::putText(frame, returnString.c_str(),cv::Point(pos.x,pos.y+pos.height),cv::FONT_HERSHEY_PLAIN,1.0,cv::Scalar(255,255,255,0));
	} else if (recogCmd.compare("get_label") == 0) {
		std::cout << FACE_HEAD << result << std::endl;
		parseOutput (result);
		std::stringstream ss;
		ss << result;
		std_msgs::String msg;
		msg.data = ss.str();
		pub.publish(msg);
		// activate other nodes
		bnetSrt.call (srv);
	}
	cv::waitKey (10);
}

int main(int argc, char **argv) {
	ros::init (argc, argv, "face_main");
	ros::NodeHandle nh;
	image_transport::ImageTransport it (nh);
	std::string libPath = "";
	std::string faceCascadePath = "";
	std::string leftEyeCascadePath = "";
	std::string rightEyeCascadePath = "";
	std::string modelPath = "";
	std::string dataPath = "";
	std::string faceString = "";

	// read recognizer parameters
	nh.getParam ("faceActive", activationFlag);
	nh.getParam ("faceLibPath", libPath);
	nh.getParam ("/face/faceCascadePath", faceCascadePath);
	nh.getParam ("/face/leftEyeCascadePath", leftEyeCascadePath);
	nh.getParam ("/face/rightEyeCascadePath", rightEyeCascadePath);
	nh.getParam ("/face/modelPath", modelPath);
	nh.getParam ("/face/dataPath", dataPath);

	// quit this node, if deactivated
	if (!activationFlag) {
		std::cout << FACE_HEAD << "Recognizer not activated" << std::endl;
		return 0;
	}

	// read recognizer library
	manager = new FaceManager (libPath);
	std::string commandList = manager->loadCommand ();
	std::string objectList = manager->loadObject ();
	std::cout << FACE_HEAD << "Recognizer command list: " << commandList << std::endl;
	std::cout << FACE_HEAD << "Recognizer object list: " << objectList << std::endl;

	// initialize recognizer
	fd.Initialize (faceCascadePath, leftEyeCascadePath, rightEyeCascadePath, modelPath, dataPath);
	cv::namedWindow(faceWindow, CV_WINDOW_AUTOSIZE);

	// subscribers and publisher for node self
	ros::Subscriber subCmd = nh.subscribe ("speech/face/command", 10, callbackCommand);
	image_transport::Subscriber subImg = it.subscribe ("camera/image_raw", 5, callbackImage);
	pub = nh.advertise<std_msgs::String> ("face/output", 10);

	// service to pause, resume and update node self
	ros::ServiceServer srvSrt = nh.advertiseService ("face/start", start);
	ros::ServiceServer srvStp = nh.advertiseService ("face/stop", stop);
	ros::ServiceServer srvUpd = nh.advertiseService ("face/update", update);

	// service to control other nodes
	bnetSrt = nh.serviceClient<std_srvs::Empty>("bnet/facestart");
	bnetUpd = nh.serviceClient<std_srvs::Empty>("bnet/faceupdate");
	spehUpd = nh.serviceClient<std_srvs::Empty>("speech/faceupdate");

	std::cout << FACE_HEAD << "Recognizer launched" << std::endl;
	ros::spin();
	return 0;
}

