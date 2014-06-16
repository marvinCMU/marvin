/*
 * backend_ros.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: iljoo
 */

#include <ros/ros.h>
#include <ros/time.h>
#include <ros/package.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <deque>

//opencv includes
#include <cv.h>
#include <opencv/highgui.h>
#include <opencv/cvwimage.h>
#include <opencv/highgui.h>
#include <cv_bridge/CvBridge.h>
#include <cv_bridge/cv_bridge.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
namespace ag = boost::algorithm;

#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/image_encodings.h>

using namespace std;

void backendPolling(void);
void callbackQueueMsg( const std_msgs::String::ConstPtr& msg );
void callbackDbMsg( const std_msgs::String::ConstPtr& msg );
void callbackFileMsg( const std_msgs::String::ConstPtr& msg );
void callbackSendFinalResult( const std_msgs::String::ConstPtr& msg );

bool gCloudService = true;
bool gCloudEnableLocalization = false;

string backendImgPath = ros::package::getPath("mmfpspkg") + string("/node_backend_aws/img/");

string userID;
string speech;
string imgName;

/* communication with backend communication handler */
ros::Publisher backendGetQueue;
ros::Publisher backendSetQueue;
ros::Subscriber backendQueueMsg;

ros::Publisher backendGetDB;
ros::Publisher backendSetDB;
ros::Subscriber backendDBMsg;

ros::Publisher backendGetFile;
ros::Subscriber backendFileMsg;

/* communication with marvin system */
ros::Publisher backendSystemTrigger; // transger trigger providing image to system
ros::Publisher backendSystemSpeechCommand; // transfer speech command to system
image_transport::Publisher backendImageOrigin; // transfer image to system
image_transport::Publisher backendImageHighRes; // transfer image to system
ros::Publisher backendSystemUserFeedback; // transfer user feedback to system
ros::Subscriber backendSystemFinalResult; // transfer Final result to cloud

void backend_ros_init(int argc, char** argv)
{
  ros::init(argc, argv, "backendRosHandler");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);

  if (ros::param::has("/CloudServiceActive"))
      ros::param::get("/CloudServiceActive", gCloudService);
  else
    ROS_WARN("Could not find parameter %s.", "/CloudServiceActive");

  if (ros::param::has("/locationActive"))
      ros::param::get("/locationActive", gCloudEnableLocalization);
  else
    ROS_WARN("Could not find parameter %s.", "/locationActive");

  // publisher : communication with backend communication handler
  backendGetQueue = nh.advertise< std_msgs::String >("backend/getQueue", 1000);
  backendSetQueue = nh.advertise< std_msgs::String >("backend/setQueue", 1000);
  backendGetDB = nh.advertise< std_msgs::String >("backend/getDB", 1000);
  backendSetDB = nh.advertise< std_msgs::String >("backend/setDB", 1000);
  backendGetFile = nh.advertise< std_msgs::String >("backend/getFile", 1000);

  // publisher : communication with marvin system
  backendSystemTrigger = nh.advertise< std_msgs::String >("trigger/output", 1000);
  backendSystemSpeechCommand = nh.advertise< std_msgs::String >("retailCommander/output", 1000);
  backendImageOrigin = it.advertise("gui/image_raw/original", 1);
  backendImageHighRes = it.advertise("gui/image_raw/hi_res", 1);
  backendSystemUserFeedback = nh.advertise< std_msgs::String >("gui/feedback_result", 1000);

  // subscriber : communication with backend communication handler
  backendQueueMsg = nh.subscribe("backend/queueMsg", 1000, callbackQueueMsg);
  backendDBMsg = nh.subscribe("backend/dbMsg", 1000, callbackDbMsg);
  backendFileMsg = nh.subscribe("backend/fileMsg", 1000, callbackFileMsg);

  // subscriber : communication with marvin system
  backendSystemFinalResult = nh.subscribe("bnet/final_result", 1000, callbackSendFinalResult);
}

int main(int argc, char **argv){

  backend_ros_init(argc, argv);

  ros::Rate loop_rate(10);

  while (ros::ok())
  {
    if(gCloudService)
      backendPolling();

    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}

void backendPolling(void)
{
  static int sleepCnt = 150;
  static int flag = 0;

  if((sleepCnt++%200) != 0 || flag)
  {
    return;
  }
  //flag = 1;

  std_msgs::String msg;
  msg.data = string("get queue");
  ROS_INFO("backendPolling : %s", msg.data.c_str());
  backendGetQueue.publish(msg);
  
  //msg.data = string("add db item test");
  //ROS_INFO("DB test : %s", msg.data.c_str());
  //backendGetDB.publish(msg);
  return;
}

void callbackQueueMsg( const std_msgs::String::ConstPtr& msg )
{
  ROS_INFO("callbackQueueMsg : \n%s", msg->data.c_str());
  
  char cmd[2048];
  char *temp;
  std_msgs::String tmpMsg;
  
  // Parse queue: ImageName / userId / speech  
  if (msg->data.size() < 2048) {
    strcpy(cmd, msg->data.c_str());
    temp = strtok(cmd, "\n");
    if(temp != NULL)
      imgName = string(temp);
    temp = strtok(NULL, "\n");
    if(temp != NULL)
      userID = string(temp);
    temp = strtok(NULL, "\n");
    if(temp != NULL)
      speech = string(temp);
  }
  
  printf("userID: %s, imgName: %s, speech: %s\n", userID.c_str(), imgName.c_str(), speech.c_str());
  
  ROS_INFO("callbackQueueMsg : %s Get an image %s from cloud", userID.c_str(), imgName.c_str()); 
  tmpMsg.data = imgName;
  backendGetFile.publish(tmpMsg);
  
  return;
}

void callbackDbMsg( const std_msgs::String::ConstPtr& msg )
{
  ROS_INFO("callbackDbMsg : %s", msg->data.c_str());

  return;
}

void callbackFileMsg( const std_msgs::String::ConstPtr& msg )
{
  ROS_INFO("callbackFileMsg : %s", msg->data.c_str());
  
  // Trigger the system
  std_msgs::String triggerMsg;
  triggerMsg.data = string("marvin");
  ROS_INFO("Trigger : %s", triggerMsg.data.c_str());
  backendSystemTrigger.publish(triggerMsg);

  // Read and publish image from test script file
  cv::Mat image;
  string cloudImageFile;
  cloudImageFile = backendImgPath + msg->data;
  image = cv::imread(cloudImageFile, CV_LOAD_IMAGE_COLOR);
  if(! image.data )
  {
    ROS_INFO("callbackFileMsg : Could not open or find the image %s\n", cloudImageFile.c_str());
    return;
  }
  // Convert image to image message format
  cv_bridge::CvImage imageMsg( std_msgs::Header(),
                  sensor_msgs::image_encodings::BGR8, image);

  if(gCloudEnableLocalization)
  {
    // Publish image for object recognition
    backendImageOrigin.publish( imageMsg.toImageMsg() );
    // Publish image for localization
    backendImageHighRes.publish( imageMsg.toImageMsg() );
  }
  else
  {
    backendImageOrigin.publish( imageMsg.toImageMsg() );
  }
  ROS_INFO("callbackFileMsg : send fake image %s", cloudImageFile.c_str());

  // need to modify : For the 1st test, text based commend
  std_msgs::String tmpMsg;
  tmpMsg.data = speech;
  backendSystemSpeechCommand.publish(tmpMsg);
  ROS_INFO("callbackFileMsg : send fake command : %s", tmpMsg.data.c_str());
  
  return;
}

void callbackSendFinalResult( const std_msgs::String::ConstPtr& msg )
{
  ROS_INFO("callbackSendFinalResult : %s", msg->data.c_str());
  
  char cmd[2048];
  char *temp;
  string tmpResult;
  string tmpUserID;
  
  // Remove unnecessary time message from the User ID  
  if (userID.size() < 2048) {
     strcpy(cmd, userID.c_str());
     temp = strtok(cmd, "_");
     tmpUserID = string(temp);
  }
  else {
    ROS_INFO("callbackSendFinalResult : too big userID");
    return;
  }
   
  // Remove unnecessary message from the final result  
  size_t strFound = msg->data.find("\n0 0 0 0");
  if (msg->data.size() < 2048) {
    memset(cmd, 0, 2048);
    strncpy(cmd, msg->data.c_str(), strFound);
    tmpResult = string(cmd);
    ROS_INFO("callbackSendFinalResult : %s", tmpResult.c_str());
  }
  else {
      ROS_INFO("callbackSendFinalResult : too big msg");
      return;
  }

  // send final result to cloud
  std_msgs::String finalResultMsg;
  finalResultMsg.data = tmpUserID + "#" + userID + "\n" + tmpResult;
  backendSetQueue.publish(finalResultMsg);
  
  // send feedback to the bayes net
  std_msgs::String feedbackMsg;
  feedbackMsg.data = string("::");
  backendSystemUserFeedback.publish(feedbackMsg);
  
  
  // remove the image file
  string cloudImageFile;
  cloudImageFile = backendImgPath + imgName;
  remove(cloudImageFile.c_str());
  ROS_INFO("callbackSendFinalResult : remove image %s", cloudImageFile.c_str());
  
  return;
}
