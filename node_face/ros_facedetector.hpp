#include "facedetectionizer.hpp"
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <compressed_image_transport/compressed_subscriber.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

ros::Publisher namePub;
ros::Subscriber faceCmd;

/*
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
  }*/

  cv::Mat convertImage(const sensor_msgs::ImageConstPtr& inputFrame)
  {
    cv_bridge::CvImagePtr cv_ptr;
    cv::Mat frame;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(inputFrame,
	  sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return frame;
    }
    frame = cv_ptr->image;
    return frame;
  }


void detect_face_callback(const sensor_msgs::ImageConstPtr& msg){

  cv::Mat frame = convertImage( msg );
  cv::namedWindow( "window", CV_WINDOW_AUTOSIZE );
  cv::imshow( "window", frame );
  ROS_INFO("Im looking for a face! changed ");
  /*
  string cascadeName = FACE_CASCADE_DEFAULT_PATH;
  string leftEyeCascade = L_EYE_CASCADE_DEFAULT_PATH;
  string rightEyeCascade = R_EYE_CASCADE_DEFAULT_PATH;
  FaceDetector faceDetector(cascadeName, leftEyeCascade, rightEyeCascade);
  FaceRecognizerWLabels model("LBPH");

  cv::Mat face;
  Rect pos;
  double scale = 1;
  int width = 200;
  int height = -1;
  bool faceFound =   faceDetector.GetCenterFace(frame, face, pos, scale,200,300);
  std_msgs::String label;

  if (faceFound){
    bool eyesDetected= faceDetector.StandardizeFaceWithEyes(frame, pos, face, width,height);
    ROS_INFO("eyesDetected= %i", eyesDetected);
    if(eyesDetected){
      label.data = model.Predict(face);
      ROS_INFO("face label: %s", label.data.c_str());
    }
    else{
      label.data = "noeyes";
    }
  }else{
    label.data = "noface";
  }


  */
  //namePub.publish(label);
}


int face_detector_main( int argc, char** argv )
{

  ros::init( argc, argv, "face_detector");
  ros::NodeHandle nh;
  namePub = nh.advertise<std_msgs::String>("reco_name", 5);
  image_transport::ImageTransport it(nh);
  image_transport::Subscriber sub = it.subscribe("camera/face/image_compressed", 5, detect_face_callback);
  ros::spin();
  return 0;
}

/*  
  string cascadeName = FACE_CASCADE_DEFAULT_PATH;
  string leftEyeCascade = L_EYE_CASCADE_DEFAULT_PATH;
  string rightEyeCascade = R_EYE_CASCADE_DEFAULT_PATH;

  CvCapture* capture = 0;
  Matk frame, frameCopy, image;
  string inputName;
  double scale = 1;
  string dataPath = DATA_DEFAULT_PATH;
  ofstream outstr;
  string modelPath = MODEL_DEFAULT_PATH;
  bool suppressVideo = false;
  bool loocv = false;
  bool daemonMode = false;


  vector<string> recoTypes;
  string recoTypesStr = "LBPH,Eigen,Fisher";
  ag::split(recoTypes, recoTypesStr, ag::is_any_of(","), ag::token_compress_on);
  string recoType=recoTypes[0];

  //*******************PARSE OPTIONS ********************************
  po::options_description desc("Allowed options");
  desc.add_options()
            ("help", "produce help message")
            ("scale", po::value<double>(&scale)->default_value(1.0), "rescales images when searching for faces")
            ("cascade", po::value<string>(&cascadeName)->default_value(cascadeName), "haarcascade path for face")
            ("leftEyeCascade", po::value<string>(&leftEyeCascade)->default_value(leftEyeCascade), "haarcascade path for left eye")
            ("rightEyeCascade", po::value<string>(&rightEyeCascade)->default_value(rightEyeCascade), "haarcascade path for right eye")
            ("dataOut", po::value<string>(&dataPath)->default_value(dataPath), "directory to save all collected data.")
            ("inputName", po::value<string>(&inputName)->default_value("0"), "Camera number or avi filename.")
            ("modelPath", po::value<string>(&modelPath)->default_value(modelPath),"Full path to the face reco model to load.")
            ("loocv", po::value<bool>(&loocv)->default_value(loocv), "Perform LOOCV on the data.")
            ("suppressVideo", po::value<bool>(&suppressVideo)->default_value(suppressVideo), "If true will not show any video outpu.")
            ("daemonMode", po::value<bool>(&daemonMode)->default_value(daemonMode), "No video output. Wait for commands on stdin.")
            ("recognizerType", po::value<string>(&recoType)->default_value(recoType), string(string("Which reco algorithm to use.(Choices: ") + recoTypesStr).c_str())
            ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help"))
  {
    cout << "This program performs real-time data collection and face recognition. You need to know a set"
            "of commands to use it, but I don't have time now to lay them out for you. You'll have to read"
            "the source. Sorry :(" << endl;
    cout << desc << endl;
    return 1;
  }
  cout << "Using cascadeName = " << cascadeName << endl;
  cout << "Using scale = " << scale << endl;
  cout << "Using output data path = " << dataPath << endl;
  cout << "Using input source =" << inputName << endl;
  cout << "Using modelPath=" << modelPath << endl;

  //*******************END PARSE OPTIONS ********************************

  cout << "Creating face detector.\n";
  FaceDetector faceDetector(cascadeName, leftEyeCascade, rightEyeCascade);
  FaceRecognizerWLabels model(recoType);
  cout << "Creating face recognizer.\n";
  model.Load(modelPath.c_str());


  if (loocv)
  {
    cout << "Performing cross-validation..." << endl;
    model.LOOCV(dataPath);
    exit(1);
  }


  if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
  {
    //taylors edit
    //capture = cvCaptureFromCAM( 1 ) ;
    capture = cvCaptureFromCAM( inputName.empty() ? 0 : inputName.c_str()[0] - '0' );
    int c = inputName.empty() ? -1 : inputName.c_str()[0] - '0' ;
    if(!capture) cout << "Capture from CAM " <<  c << " didn't work" << endl;
  }
  else if( inputName.size() )
  {//read a single image or an avi file:
    image = imread( inputName, 1 );
    if( image.empty() )
    {
      capture = cvCaptureFromAVI( inputName.c_str() );
      if(!capture) cout << "Capture from AVI didn't work" << endl;
    }
  }

  //  cvNamedWindow( "result", 1 );
  char key='i';
  string label;
  bool faceFound = false;
  Mat face, rotatedFace;
  Rect pos;
  bool detectFace=true;
  bool recognizeFace=true;
  bool takeData=false;
  bool updateModel=false;
  bool eyesDetected=false;
  int width = 200;
  int height = -1; //calculate



  if (daemonMode)
  {//****************************DAEMON MODE **************************************
    while (true)
    {
      cout << "Daemon mode started. Waiting for commands." << endl;
      //3 possible commands:
      //'takedata:<label>:N' collects N faces of data and saves them with the given label.
      //'relearn' relearns the model with all data taken so far.
      //'detect:N' detects N frames worth of faces and outputs the most frequently occurring label.
      string command;
      cin >> command;

      vector<string> cmds;
      ag::split(cmds, command, ag::is_any_of(":"), ag::token_compress_on);

      for (int i=0; i<cmds.size(); i++)
        cout << cmds[i] << endl;

      if (cmds[0]=="takedata")
      {
        assert(cmds.size()==3);
        string label = cmds[1];
        int numfaces = atoi(cmds[2].c_str());
        cout << "Taking " << numfaces << "images for label: " << label << endl << flush;

        faceDetector.PrepareToCollectData(dataPath, label);

        for (int i=0; i<numfaces; i++)
        {
          IplImage* iplImg = cvQueryFrame( capture );
          frame = iplImg;
          if( frame.empty() )
            break;
          if( iplImg->origin == IPL_ORIGIN_TL )
            frame.copyTo( frameCopy );
          else
            flip( frame, frameCopy, 0 );

          faceFound =   faceDetector.GetCenterFace(frameCopy, face, pos, scale,200,300);
          if (faceFound)
          {
            cout << "Face found" << endl;
            faceDetector.SaveFace(face,label);
          }
          else
          {
            cout << "Looking for faces..." << i<<endl;
            i--; //need to do this iteration again.
          }
        }
      }
      else
      {
        if (cmds[0]=="recognize")
        {
          assert(cmds.size()==2);
          int numfaces = atoi(cmds[1].c_str());
          cout << "Collecting " << numfaces << " images to recognize face" << endl;

          for (int i=0; i<numfaces; i++)
          {
            IplImage* iplImg = cvQueryFrame( capture );
            frame = iplImg;
            if( frame.empty() )
              break;
            if( iplImg->origin == IPL_ORIGIN_TL )
              frame.copyTo( frameCopy );
            else
              flip( frame, frameCopy, 0 );

            faceFound =   faceDetector.GetCenterFace(frameCopy, face, pos, scale,200,300);
            if (faceFound)
            {
              cout <<  model.Predict(face) << ", " << flush;
            }
            else
            {
              cout << "." << flush;
              i--; //need to do this iteration again.
            }
          }
          cout << endl;
        }
        else
          if (cmds[0]=="quit")
            exit(1);
      }
    }





    //**********************END DAEMON MODE ********************************
  }


  //TODO:******** need to use a command char separate from key char:
  char command = 'i'; //'i'nitial state
  cout << "Waiting for instructions ...";

  if( capture )
  {
    for(;;)
    {
      IplImage* iplImg = cvQueryFrame( capture );
      frame = iplImg;
      if( frame.empty() )
        break;
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
            string label = model.Predict(face);
            cv::putText(frameCopy,label,cv::Point(pos.x,pos.y+pos.height),cv::FONT_HERSHEY_PLAIN,1.0,cv::Scalar(255,255,255,0));
          }


      if (!suppressVideo)
      {
	cv::imshow("result", frameCopy);
	if (eyesDetected)
          cv::imshow("face", face);

	key = waitKey(10);
      }

      //*************** INSTRUCT THE PROGRAM TO DO SOMETHING ****************
      switch (key)
      {
      case 'h'://Output a list of commands:
	cout << "Possible commands:" << endl;
	cout <<	"'d' - toggle face detection." << endl;
	cout <<	"'t' - toggle data collection (save faces to disk with an attahed label.)" << endl;
	cout <<	"'r' - toggle face recognition." << endl;
	cout <<	"'T' - re-train the model using whatever data is available." << endl;
	cout << "'q' - quit this program." << endl << flush;
	break;
	//****toggle face detection:****
      case 'd': //'d'etect face but don't collect data.
	detectFace = !detectFace;
	if (detectFace)
	  {
	    cout << "Face detection on.";
	  }
	else
	  {
	    cout << "Face detection off.";
	    faceFound=false;
	  }
	cout << endl << flush;
	break;
	//*****toggle taking data*****
        case 't': //'t'ake data
	  takeData = !takeData;
	  if (takeData)
	    {
	      cout << "Taking data on." << endl;
	      cout << "Enter a label for the acquired faces: ";
	      cin >> label;
	      faceDetector.PrepareToCollectData(dataPath, label);
	      detectFace=true;
	    }
	  else
	    {
	      cout << "Taking data off." << endl;
	    }
          break;
	  //****toggle face recognition****
        case 'r': //face 'r'ecognition
	  recognizeFace = !recognizeFace;
	  if (recognizeFace)
	    {
	      detectFace=true;
	      cout << "Recognizing faces." << endl;
	      cout << "Initializing face recognizer..." << endl;
	      model.Load(modelPath.c_str());
	    }
          break;
        case 'T': //'T'rain the model.
          cout << "Really re-train the model with existing data (y/n)?";
          cin >> command;
          if (command == 'y')
          {
            model.ReTrain(dataPath);
          }
          break;
        case 'u': //'u'pdate model with new face data (input label required)
          //TODO: Here get a label as input and set model to update
	  //need to save any model that has been updated so we need the path
	  //to a model file to save. Also we need to save the index2label file as
	  //well. Might as well go ahead and wrap the model to deal with string labels.
          cout << "Updating model not supported yet.";
          break;
        case 'q':
          goto _cleanup_;
      }
    }

    waitKey(0);

    _cleanup_:
    cvReleaseCapture( &capture );
    outstr.close();

  }
  else
    cout << "For some reason I couldn't initialize capture.";

  cvDestroyWindow("result");

  return 0;
}


*/
