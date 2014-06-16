#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv/cvwimage.h>
#include <opencv/highgui.h>
#include <cv_bridge/CvBridge.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

std::string exec(char* cmd){
  FILE* pipe = popen(cmd, "r");
  if(!pipe) return "ERROR";
  char buffer[255];
  std::string result = "";
  while(!feof(pipe)){
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  result.erase(result.begin()+result.size()-1, result.end());
  return result;
}

int main(int argc, char **argv){

  std::string cur_filename, new_filename;

  ros::init( argc, argv, "recognitionInfo" );
  ros::NodeHandle nh;
  ros::Rate loop_rate(5);

  image_transport::ImageTransport it(nh);
  image_transport::Publisher pub = it.advertise("recognition/image", 1);
  IplImage* img;

  bool looking_for_first_image = true;

  while(looking_for_first_image){
    cur_filename = exec("find /home/mmfps/mmfps/mmfpspkg/data_tmp/vision_processed | sort -n | tail -1");
    //ROS_INFO("recon info filename= |%s|", cur_filename.c_str());
    img = cvLoadImage( cur_filename.c_str(), CV_LOAD_IMAGE_COLOR );
    if(img == NULL || !img->imageData){
      ROS_INFO("NO IMAGE DATA\n");
    }else{
      ROS_INFO("Good Data!");
      looking_for_first_image = false;
    }
    loop_rate.sleep();
  }
  cv::WImageBuffer3_b image( img );
  sensor_msgs::ImagePtr msg =sensor_msgs::CvBridge::cvToImgMsg(image.Ipl(), "bgr8");

  while(nh.ok()){
    pub.publish(msg);
    ros::spinOnce();
    loop_rate.sleep();
    new_filename = exec("find /home/mmfps/mmfps/mmfpspkg/data_tmp/vision_processed | sort -n | tail -1");
    //ROS_INFO("filename= %s", cur_filename.c_str());
    if( new_filename != cur_filename ){
      img = cvLoadImage( new_filename.c_str(), CV_LOAD_IMAGE_COLOR );
      if(img==NULL || !img->imageData){
	//ROS_INFO("NO IMAGE DATA\n");
      }else{
	cur_filename = new_filename;
	cv::WImageBuffer3_b image( img );
	msg = sensor_msgs::CvBridge::cvToImgMsg(image.Ipl(), "bgr8");
	ROS_INFO("filename= %s", new_filename.c_str());
      }
    }
  }
  return 0;
}
