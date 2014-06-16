#include <ros/ros.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <compressed_image_transport/compressed_subscriber.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>
#include <cv_bridge/CvBridge.h>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ctime>
#include "SystemParams.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>

namespace fs = boost::filesystem;

//std::string save_path(IMAGE_PATH);
ros::Publisher localizationResultsPub;

void image_callback(const sensor_msgs::ImageConstPtr& msg)
{
  ROS_INFO("SAMBA RECIEVED IMAGE!");
  sensor_msgs::CvBridge bridge;
  IplImage* temp = bridge.imgMsgToCv(msg, "bgr8");
  cv::Mat image(temp);

  //  char timestamp[256];
  //  time_t rawtime;
  //  struct tm * timeinfo;
  //  time( &rawtime );
  //  timeinfo = localtime( &rawtime );
  //  strftime( timestamp, 256, "%H%M%S%d%C%m", timeinfo );
  //if( !image.empty() ){
  //    std::string filename = save_path + timestamp + std::string(".bmp");
  std::string filename = LOCATION_IMAGE_PATH;
  cv::imwrite(filename.c_str(), image);
  ROS_INFO("saving location! %s", filename.c_str());
  //}

  /*<<<<<<< HEAD
    char timestamp[256];
    time_t rawtime;
    struct tm * timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    strftime( timestamp, 256, "%H%M%S%d%C%m", timeinfo );
    //if( !image.empty() ){
    std::string filename = save_path + timestamp + std::string(".bmp");
    cv::imwrite(filename.c_str(), image);
    //ROS_INFO("saving location! %s", filename.c_str());
    //}
    =======*/
  //Wait for localization to finish processing image:
  int maxSleeps = 4;
  int numSleeps = 0;
  ros::Rate r(1);
  bool timeout = false;
  while (!fs::exists(LOCATION_RESULTS_PATH))
    {
      ROS_INFO("Waiting for Localization to finish %i", numSleeps);
      r.sleep();//usleep(100); //sleep for 100 ms.
      if (numSleeps++ > maxSleeps)
	{
	  ROS_WARN("Localization did not respond in time, resetting.");
	  timeout = true;
	  break;
	}
    }
  stringstream data;
  if (!timeout)
    {
      ROS_INFO("Found localization results file!");

      ifstream in(LOCATION_RESULTS_PATH.c_str());
      string tmp;
      while (!in.eof())
	{
	  getline(in, tmp);
	  data << tmp << "\n";
	}
      in.close();
    }
  else
    data << "\n";

  std_msgs::String resultsMsg;
  resultsMsg.data = data.str();

  localizationResultsPub.publish(resultsMsg);

  //Delete indicator file:
  std::string cmd = "rm " + LOCATION_RESULTS_PATH;
  system(cmd.c_str());
}


int main(int argc, char** argv)
{
ros::init(argc, argv, "saver");
ros::NodeHandle nh;
image_transport::ImageTransport it(nh);
image_transport::Subscriber imgSub = it.subscribe("gui/image_raw/hi_res", 5,
		image_callback);
localizationResultsPub = nh.advertise<std_msgs::String>(
		"localization/object_histogram", 10);
ros::spin();
return 0;
}
