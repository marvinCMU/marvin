#include <ros/ros.h>
#include <image_transport/image_transport.h>

struct node_camera_opencv{
  
};
int main(int argc, char** argv){
  ROS_INFO("Starting up node_camera_opencv...");
  ros::init(argc, argv, "node_camera_opencv", ros::init_options::AnonymousName);
  ros::NodeHandle my_node;
  
  image_transport::ImageTransport it(my_node);
  image_transport::Subscriber sub = it.subscribe("in_image_base_topic", 1, imageCallback);
  image_transport::Pu

  while(ros::ok()){
    
  }
}
