#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <compressed_image_transport/compressed_subscriber.h>x
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <cv_bridge/CvBridge.h>

void imageCallback(const sensor_msgs::ImageConstPtr & msg)
{
  sensor_msgs::CvBridge bridge;
  try{
    //cvShowImage("view", bridge.imgMsgToCv(msg, "bgr8"));
  }
  catch( sensor_msgs::CvBridgeException& e){
    ROS_ERROR("Could not convert from %s to 'bgr8'.", msg->encoding.c_str());
  }
  //   sensor_msgs::CvBridge bridge;
  //  cv_bridge::
  //  cv::imwrite("/home/mmfps/mmfps/mmfpspkg/data_tmp/vision_processed/filename.jpg", bridge.imgMsgToCv(msg, "bgr8").image_data, NULL);
}

int main(int argc, char **argv){
  ros::init(argc, argv, "recognitionReceiver");
  ros::NodeHandle nh;
  cvNamedWindow("view");
  cvStartWindowThread();
  image_transport::ImageTransport it(nh);
  image_transport::Subscriber sub = it.subscribe("recognition/image_compressed", 5, imageCallback);
  ros::spin();
  cvDestroyWindow("view");
}
