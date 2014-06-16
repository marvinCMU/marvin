
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <image_transport/image_transport.h>
#include <compressed_image_transport/compressed_subscriber.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include <initializer_list>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
namespace ag = boost::algorithm;

#include "SystemParams.h"
using namespace std;

#include <ros/package.h>
string mmfpsPath = ros::package::getPath("mmfpspkg");
string optionsFile = mmfpsPath + string("node_logger/options.txt");
string logPath = mmfpsPath + string("/node_logger/logs/");
string imagePath = logPath + string("/img");


//Global timer for Fanyi:
int iteration=0;

//OPTIONS------------------
string outputFile;
ofstream out;

void ReadOptionsFromDisk()
{
  ifstream in(optionsFile.c_str());
  if (!in)
    return;
  in >> outputFile;
  in >> iteration;  
}
//=----------------------


/**Takes in a sound stream converted to text and parses it to determine
 the command object pair
 example:
 input: msg.data = "Give me some information about that powder"
 output: "information powder"
 */

map<string, string> object2BBoxes; //Maps object ids to object bounding box strings
vector<string> callbackStrings; //mostly just for debugging
bool rankedExemplarsSent = false;
bool finalResultSent = false;


// convert from ROS image format to CV Mat
cv::Mat ConvertImage(const sensor_msgs::ImageConstPtr& inputFrame)
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


//Generic callback for a string message received:
void string_callback(string msgData, const string& msgType)
{
  ag::trim(msgData); //like python strip()
  find_and_replace(msgData,"\n","|");

  out << "[" << ulong(ros::Time::now().toSec()) << "] REPORT:" << msgType << ":" << msgData << endl << flush;
}

void image_callback(const sensor_msgs::ImageConstPtr& inputFrame, const string& msgType)
{
  stringstream msgData;
  msgData << imagePath << "/" << (unsigned long)(ros::Time::now().toSec()*1000) << ".jpg";

  cv::Mat frame=ConvertImage(inputFrame);

  if (!cv::imwrite(msgData.str(), frame))
  {
   ROS_ERROR("Could not save:%s", msgData.str().c_str());
  }

  string_callback(msgData.str(), msgType);
}

void vision_gender_recognition_callback(const std_msgs::String::ConstPtr& msg)
{
  //  string_callback(msg->data, "facedetect/result");
}
void face_recognition_callback(const std_msgs::String::ConstPtr& msg)
{
  //  string_callback(msg->data, "facerecognizer/result");
}
void localization_callback(const std_msgs::String::ConstPtr& msg)
{
  //  string_callback(msg->data, "facerecognizer/result");
}
void retail_speech_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "dialog/retail");
}
void person_speech_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "dialog/person");
}
void general_speech_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "dialog/general");
}
void face_detection_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "facedetect/result");
}
void retail_recognition_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "retail/object_detection_result");
}
void gui_feedback_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "gui/feedback");
  out << "REPORT:--DONE " << iteration++ <<"--" << endl << flush;
}
// void gui_image_callback(const sensor_msgs::ImageConstPtr& inputFrame)
// {
//   string msgData="IMAGE";
//   string_callback(msgData, "gui/image_raw");
// }

void gui_image_original_callback(const sensor_msgs::ImageConstPtr& inputFrame)
{
  image_callback(inputFrame, "gui/image_raw");
}

void gui_image_hi_res_callback(const sensor_msgs::ImageConstPtr& inputFrame)
{
  image_callback(inputFrame, "gui/image_raw/hi_res");
}

void bnet_final_result_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "bnet/final_result");
}
void bnet_ranked_exemplars_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "bnet/ranked_exemplars");
}
void trigger_callback(const std_msgs::String::ConstPtr& msg)
{
  string_callback(msg->data, "trigger/output");
}

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "logger");
  ros::NodeHandle nh;
  ros::Rate loop_rate(10);
  image_transport::ImageTransport it(nh);
  //  ros::Timer timer = nh.createTimer(ros::Duration(0.25), timerCallback);

  // ReadOptionsFromDisk();


  stringstream outputFile;
  outputFile << logPath << ulong(ros::Time::now().toSec()) << ".txt";
  stringstream linkCmd;
  linkCmd << "rm " << logPath << "/latest.txt; ln -s " << outputFile.str() << " " << logPath << "/latest.txt";
  system(linkCmd.str().c_str());
  
  ROS_INFO("Saving log info to %s.", outputFile.str().c_str());

  out.open(outputFile.str().c_str(), std::ofstream::out);
  
  ros::Subscriber guiFeedbackSub = nh.subscribe("gui/feedback_result", 1000,
      gui_feedback_callback);
  // ros::Subscriber guiImageRawSub = nh.subscribe("gui/image_raw", 1000,
  //     gui_image_callback);
  ros::Subscriber guiImageRawOriginalSub = nh.subscribe("gui/image_raw", 1000,
      gui_image_original_callback);
  //  ros::Subscriber guiImageRawHiDefSub = nh.subscribe("gui/image_raw/hi_res", 1000,
  //      gui_image_hi_res_callback);
  ros::Subscriber retailRecoSub = nh.subscribe("retail/object_detection_result",
      1000, retail_recognition_callback);
  ros::Subscriber bnFinalResult = nh.subscribe("bnet/final_result", 1000,
					       bnet_final_result_callback);
  ros::Subscriber bnetRankedExemplars = nh.subscribe("bnet/ranked_exemplars", 1000,
      bnet_ranked_exemplars_callback);

  ros::Subscriber triggerSub = nh.subscribe("dialog/trigger", 1000,
      trigger_callback);
  ros::Subscriber genSpeechSub = nh.subscribe("dialog/general", 1000,
      general_speech_callback);
  ros::Subscriber dmRetailSub = nh.subscribe("dialog/retail", 1000,
      retail_speech_callback);
  //  ros::Subscriber dmFaceSub = nh.subscribe("dialog/face", 1000,
  //      person_speech_callback);


  ros::spin();

  out.close();
  return 0;
}

