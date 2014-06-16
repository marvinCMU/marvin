#include "ros/ros.h"
#include "std_msgs/String.h"

void gui_log_callback(const std_msgs::String::ConstPtr &msg){
  //ROS_INFO("I got it!: %s", msg->data.c_str());  
}

int main(int argc, char **argv){
  ros::init(argc, argv, "guiListener");
  ros::NodeHandle nh;
  ros::Subscriber sub = nh.subscribe("guiInfo", 1000, gui_log_callback);
  ros::spin();
  return 0;
}
