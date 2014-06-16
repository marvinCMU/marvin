/*
 * main.cpp
 *
 *  Created on: Sep 11, 2012
 *      Author: dhdash
 */

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

ros::Publisher namePub;
ros::Subscriber faceCmd;
char key='i';
string label;
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

ros::Publisher face_reco_pub;



/*
string cascadeName = FACE_CASCADE_DEFAULT_PATH;
string leftEyeCascade = L_EYE_CASCADE_DEFAULT_PATH;
string rightEyeCascade = R_EYE_CASCADE_DEFAULT_PATH;
string modelPath = MODEL_DEFAULT_PATH;
FaceDetector faceDetector(cascadeName, leftEyeCascade, rightEyeCascade);
FaceRecognizerWLabels model("LBPH");
*/


//This function takes a few different commands as input:
//"prepare_take_data:label" will create the directory structure to take data for a face with label "label".
//"take_data:label" will save a face (if found) with label "label" and will return the face position in pos.
//"get_label" will try to recognize face and return the label in returnVal and will return the face position in pos.
//"retrain" will cause the model to be retrained on all data.
bool face_detectionizer_ros(FaceDetectionizer& fd, const string& cmd, const Mat& img, string& returnVal, Rect& pos)
{
  vector<string> cmds;
  ag::split(cmds, cmd, ag::is_any_of(":"), ag::token_compress_on);

  if (cmds[0]=="prepare_take_data") 
    {
      assert(cmds.size()==2); //eg, cmd="prepare_take_data:denver"
      fd.PrepareToCollectData(cmds[1]);
      return true;
    }

  if (cmds[0]=="take_data")
    {
      assert(cmds.size()==2); //eg, cmd="take_data:denver"
      string label=cmds[1];
      return fd.DetectAndSaveData(img,label,pos);
    }

  if (cmds[0]=="get_label")
    {
      return fd.DetectAndLabel(img,returnVal,pos);
    }

  if (cmds[0]=="retrain")
    return fd.RetrainModelWithCurrentData();

  return false;
}



void detect_face_callback( const sensor_msgs::ImageConstPtr& msg ){
  detectFace= true;
  recognizeFace= true;
  sensor_msgs::CvBridge bridge;
  iplImg = bridge.imgMsgToCv(msg, "bgr8");
  string label = "NULL";

  if( iplImg != NULL ){
    //IplImage* iplImg = cvQueryFrame( capture );
    frame = iplImg;
    if( frame.empty() )
      return;
    if( iplImg->origin == IPL_ORIGIN_TL )
      frame.copyTo( frameCopy );
    else
      flip( frame, frameCopy, 0 );


    if (detectFace)
      {
	faceFound =   faceDetector.GetCenterFace(frameCopy, face, pos, scale,200,300);

	if (faceFound)
	  {
	    eyesDetected= faceDetector.StandardizeFaceWithEyes(frameCopy, pos, face, width,height);
	    cout << "eyesDetected=" << eyesDetected << endl << flush;
	  }

      }

    if (takeData)
      {
	if (faceFound)
	  {
	    faceDetector.SaveFace(face,label);
	  }
      }
    if (recognizeFace)
      if (faceFound)
	{
	  label = model.Predict(face);
	  cv::putText(frameCopy,label,cv::Point(pos.x,pos.y+pos.height),cv::FONT_HERSHEY_PLAIN,1.0,cv::Scalar(255,255,255,0));
	}


    if (!suppressVideo)
      {
	cv::imshow("result", frameCopy);
	if (eyesDetected)
	  cv::imshow("face", face);

	waitKey(1);
      }

  }

  if( label.compare("NULL") != 0  &&
      label.compare("unknown_index") != 0 ){
    std_msgs::String msg;
    msg.data = label;
    face_reco_pub.publish( msg );
  }

}


int main( int argc, char** argv )
{

  ros::init( argc, argv, "face_detector");
  ros::NodeHandle nh;

  namePub = nh.advertise<std_msgs::String>("reco_name", 5);
  image_transport::ImageTransport it(nh);
  image_transport::Subscriber sub = it.subscribe("gui/image_raw", 5, detect_face_callback);
  face_reco_pub = nh.advertise<std_msgs::String>( "face/face_detection_result", 1000 );

  model.Load(modelPath.c_str());

  cvNamedWindow( "result", 1 );

  ros::spin();

  cvDestroyWindow("result");

  return 0;
}
