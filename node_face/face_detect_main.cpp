#include "facedetectionizer.hpp"
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
#include <sstream>
#include <string>
#include <deque>
#include "SystemParams.h"

/*ros::Publisher namePub;
 ros::Subscriber faceCmd;
 string label;
 bool faceFound = false;
 Mat face, rotatedFace;
 Rect pos;
 bool detectFace = false;
 bool recognizeFace = false;
 bool takeData = false;
 bool updateModel = false;
 bool eyesDetected = false;
 int width = 200;
 int height = -1; //calculate

 //IplImage* iplImg=NULL;
 cv::Mat input_frame;*/
/*
 string inputName;
 double scale = 1;
 string dataPath = DATA_DEFAULT_PATH;
 ofstream outstr;
 string modelPath = MODEL_DEFAULT_PATH;
 bool suppressVideo = false;
 bool loocv = false;
 bool daemonMode = false;
 string recoType = "LBPH";
 bool take_data = false; //CHANGE THIS
 */

//ros::Publisher face_detection_bb_publisher;
//ros::Publisher face_detector_ready_publisher;
//ros::Publisher no_data_publisher;
//image_transport::Publisher face_recognition_image_publisher;
//image_transport::Publisher face_take_data_image_publisher;
/*
 std::string exec(const char* cmd){
 FILE* pipe = popen(cmd, "r");
 if(!pipe) return "ERROR";
 char buffer[100];
 std::string result = "";
 while(!feof(pipe)){
 if(fgets(buffer, 100, pipe) != NULL){
 result += buffer;
 //ROS_INFO("result: %s", result.c_str());
 }
 }
 pclose(pipe);
 result.erase(result.begin()+result.size()-1, result.end());
 return result;

 }

 void detect_face()
 {

 void detect_face(){

 Mat frame, frameCopy;
 std_msgs::String bb_msg;
 bb_msg.data = "no face";
 string label = "NULL";
 sensor_msgs::ImagePtr face_img_msg;
 bool bad_data = false;
 std_msgs::String ready_msg;
 ready_msg.data = "no face";


 frame = input_frame;
 if( frame.empty() )
 return;
 frame.copyTo( frameCopy );
 //cv::flip( frame, frameCopy, 0 );

 faceFound = faceDetector.GetCenterFace( frameCopy, face, pos, scale, 200, 300 );

 if( faceFound ){
 ROS_INFO("Face found");
 cv::Mat tempFace = face;
 cv::Mat tempFrameCopy = frameCopy;
 eyesDetected= faceDetector.StandardizeFaceWithEyes(tempFrameCopy, pos, tempFace, width,height);
 }else{
 ROS_INFO("Face not found");
 face_detector_ready_publisher.publish(ready_msg);
 return;
 }
 if( eyesDetected ){
 ROS_INFO("eyes detected!");
 }else{
 ROS_INFO("eyes not detected!");
 }

 if( !face.empty() ){
 //ROS_INFO("Face found");
 ready_msg.data = "yes face";

 //eyesDetected= faceDetector.StandardizeFaceWithEyes(frameCopy, pos, face, width,height);

 cv::Mat correct_face;
 cv::cvtColor( face, correct_face, CV_GRAY2BGR );
 IplImage face_ipl = correct_face;
 CvMat face_cv = face;

 try{
 face_img_msg = sensor_msgs::CvBridge::cvToImgMsg( &face_ipl, "bgr8" );
 }catch( sensor_msgs::CvBridgeException &e ){
 ROS_WARN( "WARNING! image to message conversion failed" );
 ROS_ERROR("cv_bridge exception: %s", e.what());
 bad_data = true;
 }

 if( !bad_data ){
 std::stringstream ss;
 ss << pos.x << "\n" << pos.y << "\n" << pos.width << "\n" << pos.height << "\n"<<
 width <<"\n" << height << std::endl;
 bb_msg.data = ss.str();

 face_detection_bb_publisher.publish( bb_msg );

 if( take_data ){
 //ROS_INFO("Sending to take data");
 face_take_data_image_publisher.publish( face_img_msg );
 }else{
 //ROS_INFO("Sending to recognize");
 face_recognition_image_publisher.publish( face_img_msg );
 }
 }
 }else{
 //ROS_INFO("I don't see a face");
 }

 face_detector_ready_publisher.publish(ready_msg);
 }

 void faces_left_callback( const std_msgs::String::ConstPtr& msg ){
 if(strcmp( msg->data.c_str(), "false" ) == 0 ){
 take_data = false;
 }
 }

 void detect_face_callback( const sensor_msgs::ImageConstPtr& msg ){
 ROS_INFO("FACE DETECTOR GOT IMAGE");
 cv_bridge::CvImagePtr cv_ptr;
 try
 {
 cv_ptr = cv_bridge::toCvCopy(msg,
 sensor_msgs::image_encodings::BGR8);

 }
 catch (cv_bridge::Exception& e)
 {
 ROS_WARN( "WARNING! image to message conversion failed at the callback" );
 ROS_ERROR("cv_bridge exception: %s", e.what());
 return;
 }
 input_frame = cv_ptr->image;

 detect_face();
 }

 void take_data_callback( const std_msgs::String::ConstPtr& msg ){
 take_data = true;
 }*/

#define MAX_IMAGE_BACKLOG 5

using std::string;
bool SHOW_DEBUG_PICS = false;

std::deque<cv::Mat> toProcess;
FaceDetector faceDetector(FACE_CASCADE_DEFAULT_PATH, L_EYE_CASCADE_DEFAULT_PATH,
    R_EYE_CASCADE_DEFAULT_PATH);

struct Publishers
{
  image_transport::Publisher faceImage;
  ros::Publisher faceStatus;
} publishers;

struct Subscribers
{
  image_transport::Subscriber guiImage;
} subscribers;

void addToProcessList(const sensor_msgs::ImageConstPtr& msg)
{
  ROS_INFO("FACE DETECTION RECIEVED AN IMAGE");
  sensor_msgs::CvBridge bridge;
  cv::Mat image(bridge.imgMsgToCv(msg, "bgr8"));
  if (toProcess.size() < MAX_IMAGE_BACKLOG)
    toProcess.push_back(image);

  if (SHOW_DEBUG_PICS)
  {
    imshow("Face Detection Got", image);
    cv::waitKey(5);
  }
}

void processFaceDetection(cv::Mat image)
{
  cv::Mat face;
  Rect pos;
  double scale = 1;
  bool faceFound = false, eyesDetected = false;
  int width = 200, height = -1;
  std_msgs::String statusMsg;

  statusMsg.data = "absent";
  faceFound = faceDetector.GetCenterFace(image, face, pos, scale, 200, 300);
  if (faceFound)
  {
    ROS_INFO("Face found. ");
//    eyesDetected = faceDetector.StandardizeFaceWithEyes(image, pos, face, width,
//        height);

//    if (eyesDetected)
//    {
//      ROS_INFO("Face found and eyes detected");
      cv_bridge::CvImage imageMsg(std_msgs::Header(),
          sensor_msgs::image_encodings::MONO8, face);
      statusMsg.data = "present";
      publishers.faceImage.publish(imageMsg.toImageMsg());

      if (SHOW_DEBUG_PICS)
      {
        imshow("Face detector result", face);
        cv::waitKey(5);
      }
      //toProcess.clear();
//    }
//    else
//      ROS_INFO("Eyes not found.");
  }
  publishers.faceStatus.publish(statusMsg);

}

void mainLoop()
{
  while (ros::ok())
  {
    while (!toProcess.empty())
    {
      processFaceDetection(toProcess.front());
      toProcess.pop_front();
    }
    ros::spinOnce();
  }
}

void initRos(int argc, char** argv)
{
  ros::init(argc, argv, "face_detector");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);

  GetROSParameter<bool>("face/showDebugPics", SHOW_DEBUG_PICS);

  subscribers.guiImage = it.subscribe("gui/image_raw", 5, addToProcessList);

  publishers.faceImage = it.advertise("face/face_detection_image", 1);
  publishers.faceStatus = nh.advertise < std_msgs::String
      > ("face/face_detected_status", 1000);
  //ros::spin();
}

int main(int argc, char** argv)
{
  initRos(argc, argv);

  if (SHOW_DEBUG_PICS)
  {
    cvNamedWindow("Face Detection Got");
    cvNamedWindow("Face detector result");
  }

  mainLoop();

  //face_detection_bb_publisher = nh.advertise<std_msgs::String>( "face/face_detection_result/bounding_box", 1000 );
  //face_recognition_image_publisher = it.advertise( "face/face_detection_result/image/recognize", 5 );
  //face_take_data_image_publisher = it.advertise( "face/face_detection_result/image/take_data", 5 );
  //face_detector_ready_publisher = nh.advertise<std_msgs::String>( "face/face_detector/ready", 1000 );

  //ros::Subscriber faces_left_subscriber = nh.subscribe( "face/face_train/faces_left", 1000, faces_left_callback );
  //ros::Subscriber take_data_subscriber = nh.subscribe( "gui/take_data", 1000, take_data_callback );

  //ros::spin();

  return 0;
}
