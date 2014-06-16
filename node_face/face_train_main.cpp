//start here -rewrite all of this, store the image labels on the tablet

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
#include <ctime>

#define FACES_TO_SAVE 30;

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
string modelPath = MODEL_DEFAULT_PATH;
bool suppressVideo = false;
bool loocv = false;
bool daemonMode = false;
string recoType= "LBPH";

FaceDetector faceDetector(cascadeName, leftEyeCascade, rightEyeCascade);
FaceRecognizerWLabels model(recoType);

ros::Publisher faces_left_publisher;

int faces_left = FACES_TO_SAVE;
std::string label = "NULL";
bool prep_done = false;

cv::Mat face_label;

std::string exec(char* cmd){
  FILE* pipe = popen(cmd, "r");
  if(!pipe) return "ERROR";
  //char buffer[100];
  std::string result = "";
  //  while(!feof(pipe)){
    //if(fgets(buffer, 100, pipe) != NULL){
      //result += buffer;
      //ROS_INFO("result: %s", result.c_str());
    //}
    //}
  pclose(pipe);
  //result.erase(result.begin()+result.size()-1, result.end());
  return result;
}

void face_label_callback( const sensor_msgs::ImageConstPtr& msg ){
  sensor_msgs::CvBridge bridge;
  IplImage* temp = bridge.imgMsgToCv(msg, "bgr8");
  cv::Mat temp2(temp);
  face_label = temp2;
}

void recognition_label_callback( const std_msgs::String::ConstPtr& msg ){
  label = msg->data;
  ROS_INFO("Label = %s", label.c_str() );

}

void training_label_callback( const std_msgs::String::ConstPtr& msg ){
  label = msg->data;
  ROS_INFO("Label = %s", label.c_str() );

}


void take_data_callback( const sensor_msgs::ImageConstPtr& msg ){
  sensor_msgs::CvBridge bridge;
  std_msgs::String faces_left_msg;
  iplImg = bridge.imgMsgToCv(msg, "bgr8");


  if(strcmp(label.c_str(), "NULL") != 0 ){  
    if( strcmp(label.c_str(), "new person") == 0 ){
      char timestamp[256];
      time_t rawtime;
      struct tm * timeinfo;
      time( &rawtime );
      timeinfo = localtime( &rawtime );
      strftime( timestamp, 256, "%H%M%S%d%C%m", timeinfo );
      label = timestamp;
      if( !face_label.empty() ){
	std::string filename = std::string( FACE_LABELS_PATH ) + label
	  +std::string(".jpg");
	cv::imwrite(filename.c_str(), face_label);
      }
    }

    if(!prep_done ) {
      faceDetector.PrepareToCollectData(dataPath, label);
      prep_done = true;
    }

    if( iplImg != NULL && prep_done ) {
      face = iplImg;
      faceDetector.SaveFace(face,label);
      //ROS_INFO("Face saved!");
      faces_left--;
    }


    faces_left_msg.data = "false";
    if( faces_left >= 0 ){
      faces_left_msg.data = "true";
    }
    else{
      std::string cmd;
    
      faces_left = FACES_TO_SAVE;
      label = "NULL";      
      prep_done = false;
      cmd = std::string("cp ") + dataPath +
	std::string("/default_model.xml ") + modelPath;
      char cmd_buffer[800];
      strcpy(cmd_buffer, cmd.c_str() );
      exec( cmd_buffer );


      //>labels.csv; find ./ -name "labels.csv" | xargs cat >> 
      //./labels_temp.csv ; mv ./labels_temp.csv ./labels.csv

      {
	using namespace std;
	cmd = string(">") + dataPath + string("/labels.csv; find ") + dataPath
	  + string(" -name \"labels.csv\" | xargs cat > ") + dataPath
	  + string("/labels_temp.csv; mv ") + dataPath + string("/labels_temp.csv ")
	  + dataPath + string("/labels.csv");
	char cmd_buffer_2[800];
	strcpy(cmd_buffer_2, cmd.c_str() );
	exec( cmd_buffer_2 );
	}

      model.ReTrain( dataPath );

    }
  }
  faces_left_publisher.publish( faces_left_msg );
}

int main( int argc, char** argv )
{
  ros::init( argc, argv, "face_detector");
  ros::NodeHandle nh;

  image_transport::ImageTransport it(nh);
  image_transport::Subscriber imgSub = it.subscribe("face/face_detection_result/image/take_data",
						    5, take_data_callback ) ;
  image_transport::Subscriber faceLabelSub = it.subscribe( "gui/image_raw",
							   5, face_label_callback );
  ros::Subscriber label_subscriber = nh.subscribe( "gui/face_train/label", 1000, training_label_callback );
  ros::Subscriber recognition_subscriber = nh.subscribe( "face/face_recognition_result", 1000,
							 recognition_label_callback );
  //ros::Subscriber = nh.subscribe( "face/face_detection/faces_left", 1000, faces_left_callback );
  faces_left_publisher = nh.advertise< std_msgs::String >( "face/face_train/faces_left", 1000 );

  cv::namedWindow( "disp2", CV_WINDOW_AUTOSIZE );

  ros::spin();
  return 0;
}
