 #include "facedetectionizer.hpp"
#include <ros/ros.h>
#include <exception>
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
#include <deque>
/*
ros::Subscriber faceCmd;
char key='i';
bool faceFound = false;
Mat face, rotatedFace;
Rect pos;
bool detectFace=false;
bool recognizeFace=false;
bool takeData=false;
bool updateModel=false;
bool eyesDetected=false;
int width = 200;
int height = -1; //calculate

IplImage* iplImg=NULL;


string cascadeName = FACE_CASCADE_DEFAULT_PATH;
string leftEyeCascade = L_EYE_CASCADE_DEFAULT_PATH;
string rightEyeCascade = R_EYE_CASCADE_DEFAULT_PATH;

Mat frame, frameCopy, image;
string inputName;
double scale = 1;
string dataPath = DATA_DEFAULT_PATH;
ofstream outstr;

bool suppressVideo = false;
bool loocv = false;
bool daemonMode = false;
string recoType= "LBPH";

string label = "unknown_index";

bool got_answer = false;

bool model_loaded = true;
//FaceDetector faceDetector(cascadeName, leftEyeCascade, rightEyeCascade);

*/


//ros::Publisher recognition_label_publisher;
//ros::Publisher recognition_image_publisher;

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
*/
/*
void faces_left_callback( const std_msgs::String::ConstPtr& msg ){
  if( strcmp( msg->data.c_str(), "false" ) == 0 ){
    ROS_INFO("retraining face recognizer");
    model._RefreshData( dataPath );
    got_answer = false;
    //iplImg = NULL;
  }
  }*/
/*
void gui_state_callback( const std_msgs::String::ConstPtr& msg ){
  if( strcmp( msg->data.c_str(), "waiting") == 0 )
    got_answer = false;
}
*/



/*
void recognize_face_callback( const sensor_msgs::ImageConstPtr& msg ){
  if(!model_loaded){
    model.Load( modelPath.c_str() );
    model_loaded = true;
  }
  recognizeFace= true;
  sensor_msgs::CvBridge bridge;
  std_msgs::String recognition_msg;
  std_msgs::String recognition_img_msg;
  iplImg = bridge.imgMsgToCv(msg, "bgr8");

  if( iplImg != NULL && !got_answer ){
    face = iplImg;
    label = model.Predict(face);
    recognition_msg.data = label;
    if(label.compare("unknown_index") != 0 ){
      ROS_INFO("recognized someone!: %s", label.c_str());
      got_answer = true;
      recognition_label_publisher.publish( recognition_msg );
    }
    label = "unknown_index";
  }

  }*/

#define MAX_IMAGE_BACKLOG 5

using std::string;

const bool SHOW_DEBUG_PICS = true;

struct Publishers{
  ros::Publisher recognitionLabel;
} publishers;

struct Subscribers{
  image_transport::Subscriber faceDetection;
} subscribers;

std::deque<cv::Mat> toProcess;//start here update subscriber
FaceRecognizerWLabels model(string("LBPH"));


void processFaceRecognition( cv::Mat face ){
  string label = model.Predict( face );
  std_msgs::String labelMsg;
  labelMsg.data = label;
  publishers.recognitionLabel.publish( labelMsg );
  ROS_INFO( "Identified: %s", label.c_str() );

  if(SHOW_DEBUG_PICS){
    if( label.compare("unknown_index") != 0){
      try{
      string path = string(FACE_LABELS_PATH) + label + string(".jpg");
      cv::Mat result = imread(path);
      imshow("Recognition Result", result );
      cv::waitKey(5);
      }
      catch(exception e){ ROS_INFO("Invalid path");}
    }
  }
}

void addToProcessList( const sensor_msgs::ImageConstPtr& msg ){
  ROS_INFO("FACE RECOGNITION RECIEVED A FACE");
  sensor_msgs::CvBridge bridge;
  cv::Mat image(bridge.imgMsgToCv( msg, "mono8" ));
  if(toProcess.size() < MAX_IMAGE_BACKLOG )  toProcess.push_back(image);

  if(SHOW_DEBUG_PICS){
    imshow("Face Recognition Got", image);
    cv::waitKey(5);
  }
}

void mainLoop(){
  while( ros::ok() ){
    while( !toProcess.empty()){
      processFaceRecognition( toProcess.front() );
      toProcess.pop_front();
    }
    ros::spinOnce();
  }
}

void initRos( int argc, char** argv ){
  ros::init( argc, argv, "face_detector");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);
  subscribers.faceDetection = it.subscribe( "face/face_detection_image", 5, addToProcessList );
  publishers.recognitionLabel = nh.advertise<std_msgs::String>("face/face_recognition_result", 1000 );

}
int main( int argc, char** argv )
{
  initRos( argc, argv );

  if(SHOW_DEBUG_PICS){
    cvNamedWindow("Face Recognition Got");
    cvNamedWindow("Recognition Result");
  }

  model.Load( MODEL_DEFAULT_PATH );
  mainLoop();
  
  
  /*  image_transport::Subscriber imgSub = it.subscribe( "face/face_detection_result/image/recognize",
						    5, recognize_face_callback ) ;
  //ros::Subscriber bbSub = nh.subscribe( "face/face_detection_result/bounding_box", 1000, bb_parser_callback );
  ros::Subscriber guiStateSub = nh.subscribe( "gui/state", 1000, gui_state_callback );
  ros::Subscriber facesLeftSub = nh.subscribe( "face/face_train/faces_left", 1000, faces_left_callback );
  
  recognition_label_publisher = nh.advertise< std_msgs::String >( "face/face_recognition_result", 1000 );

  std::string cmd = std::string("cat ") + dataPath
    + std::string("/model.xml.index2Label.txt | wc -l");
  std::string result = exec( cmd.c_str() );
  if( atoi( result.c_str() ) != 0 ){
    model.Load( modelPath.c_str() );
    model_loaded = false;
    }*/


  
  //cv::namedWindow( "B", 1 );


  //ros::spin();

  return 0;
}
