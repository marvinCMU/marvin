#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <boost/program_options/option.hpp>
#include <boost/program_options/config.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED

//#include "boost/filesystem/operations.hpp"
//#include "boost/filesystem/path.hpp"

#include "boost/process.hpp"
#include "boost/assign.hpp"
#include "SystemParams.h"

#define i2l_suffix "index2Label.txt"

using namespace std;
using namespace cv;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace ag = boost::algorithm;
namespace pr = boost::process;



#define i2l_suffix "index2Label.txt"
#define RECO_DEFAULT_TYPE "LBPH" //choices are LBPH, Eigen, Fisher.


//Wrapper on opencv face recognizer to allow it to accept string labels:
class FaceRecognizerWLabels
{
  Ptr<FaceRecognizer> model;
  vector<string> index2Label;
  map<string,int> label2Index;

  vector<Mat> imagesToTrain;
  vector<int> labelIndicesToTrain;

  string modelPath;
  string dataPath;

public:
  FaceRecognizerWLabels()
  {}
  FaceRecognizerWLabels(const string& recoType)
  {
    this->Initialize(recoType);
  }

  void Initialize(const string& recoType)
  {
    if (recoType == "LBPH")
    {
      cout << "Creating LBPH Model." << endl;
      model = createLBPHFaceRecognizer();
    }
    else
      if (recoType == "Eigen")
      {
        cout << "Creating Eigen model." << endl;
        model = cv::createEigenFaceRecognizer();
      }
    else if (recoType == "Fisher")
    {
      cout << "Creating Fisher model." << endl;
      model = cv::createFisherFaceRecognizer();
    }
    else
    {
      cerr << (string("Unrecognized model type ") + recoType);
      throw "Unrecognized model type";
    }
  }

  void PrepareToTrain()
  {
    imagesToTrain.clear();
    labelIndicesToTrain.clear();
  }

  void AddTrainingInstance(Mat& face, const string& label)
  {
//    imagesToTrain.push_back(face);
//    map<string,int>::iterator it = label2Index.find(label);
//    if (it == label2Index.end()) //new label
//      ;
//
//    labelIndicesToTrain.push_back(label2Index[label]);
  }

  void Load(const string& newModelPath)
  {
    modelPath=newModelPath;
    cout << "Loading model " << modelPath << endl;

    
    model->load(modelPath.c_str());
    //taylors edits

    index2Label.clear();
    label2Index.clear();

    string labelFile = string(modelPath+"."+i2l_suffix);
    ifstream in(labelFile.c_str());
    cout << "Loading file " << labelFile << endl;
    if (!in)
    {
      cout << "ERROR: Cannot find file " << labelFile << endl;
      throw("File not found");
    }

    int labelIndex;
    char comma;
    string tmpLabel;
    while (in)
    {
      in >> labelIndex >>comma >> tmpLabel;
      if (comma != ',')
        break;
      cout << "labe2Index[" << tmpLabel << "]=" << index2Label.size() << endl;
      label2Index[tmpLabel]=index2Label.size();
      index2Label.push_back(tmpLabel);
      cout << "index2Label.back()=" << index2Label.back() << endl; 
    }
  }

  void Save(const string& modelPath)
  {
    model->save(modelPath);
    string index2Label_path = modelPath + string(".")+string(i2l_suffix);
    ofstream out(index2Label_path.c_str());
    for (uint i=0; i<index2Label.size(); i++)
      out << i << ", " << index2Label[i] << endl;
    out.close();
  }

  string Predict(const Mat& face)
  {
    //imshow("seeing", face);
    uint labelIndex = model->predict(face);
    //cout<< "index2Label.size()= " << index2Label.size() << endl;
    if (labelIndex>=0 && labelIndex <index2Label.size())
      return index2Label[labelIndex];
    //cout << "unknown_index=" << labelIndex << endl;
    return "unknown_index";
  }

  void ReadCSV(const string& filename, char separator = ';')
  {
    int index=0;
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file)
      throw std::exception();
    string line, path, classLabel;
    while (getline(file, line))
    {
      stringstream liness(line);
      getline(liness, path, separator);
      getline(liness, classLabel);
      imagesToTrain.push_back(imread(path, 0));

      //taylors edits
      //std::cout<<"path: "<<path<<std::endl;
      // imshow("images to train", imread(path,0));

//TODO: Finish this:
      map<string,int>::iterator it = label2Index.find(classLabel);
      if (it == label2Index.end())
      {
        cout << "Found new label " << classLabel << " with index " << index << "." << endl;

        label2Index[classLabel]=index;
        index2Label.push_back(classLabel);
        cout << "index2Label.size()=" << index2Label.size() << endl;
        index++;
      }
      labelIndicesToTrain.push_back(label2Index[classLabel]);
    }
    cout << "label2Index.size()=" << label2Index.size() << ", index2Label.size()=" << index2Label.size() << endl;

  }

  void Clear()
  {
//    model.release();
//    model = createLBPHFaceRecognizer();
    cout << "****************Clearing all data!***************" << endl;
    this->imagesToTrain.clear();
    this->labelIndicesToTrain.clear();
    this->index2Label.clear();
    this->label2Index.clear();
  }


  //private:
  void _RefreshData(const string& outDataPath)
  {
    dataPath = outDataPath;

    //TODO HACK: This python script searches the data directory for jpg images and collects them
    //in the labels.csv file at the dataPath directory.
    //string syscmd = "rm " + dataPath + "/labels.csv; find " + dataPath + "/ -name \"labels.csv\" | xargs -i cat {} >> " + outDataPath + "/labels.csv";

    //----This uses the boost library to call a python script and wait for it to finish:
    string genLabelsExe = string(GEN_LABELS_PATH) + string("/gen_labels.py");
//    std::string exe = boost::process::find_executable_in_path(genLabelsExe);
    //Create a list of input arguments to python required for the gen_labels.py script:
    vector<std::string> args = boost::assign::list_of(dataPath)("labels.csv")("jpg");
    //boost::process::child c = boost::process::create_child(genLabelsExe, args);
    
    
    //taylors edits
    boost::process::context ctx;
    // ctx.stdout_behavior = boost::process::silence_stream();
    ctx.stdout_behavior = boost::process::redirect_stream_to_stdout();
    boost::process::child c = boost::process::launch(genLabelsExe, args, ctx);

    //boost::process::child c = boost::process::child::start_child(genLabelsExe, args);
    //boost::process::child c = boost::process::child::create_child(genLabelsExe, args);

    

    boost::process::status exit_code = c.wait(); //wait for command to finish.
    //----Done calling python script

    string labelsFile = dataPath + "/labels.csv";
//    string syscmd = "python " + GEN_LABELS_PATH +  "/gen_labels.py " + dataPath + " labels.csv jpg";

//    cout << syscmd << endl;
//    system(syscmd.c_str());
//    sleep(2); //TODO HACK: sleep to avoid synch problems.

    Clear();
    ReadCSV(labelsFile);
    cout << "Found " << index2Label.size() << " labels, and " << imagesToTrain.size() << " training images." << endl;
    cout << "labelIndicesToTrain.size()=" << labelIndicesToTrain.size() << endl;
  }

public:
  void ReTrain(const string& outDataPath)
  {
    cout << "Refreshing data" << endl;
    _RefreshData(outDataPath);
    cout << "Done! Retraining model:"<< endl;
    model->train(imagesToTrain, labelIndicesToTrain);
    cout << "Done!" << endl;

    Save(outDataPath+"/model.xml");
  }

  void LOOCV(const string& outDataPath)
  {
    _RefreshData(outDataPath);
    vector<Mat> images(imagesToTrain);
    vector<int> labels(labelIndicesToTrain);

    vector<int> confMatRow(imagesToTrain.size(),0);
    vector< vector<int> > confusionMatrix(imagesToTrain.size(), confMatRow);
    int correctNum=0;


    for (int i=imagesToTrain.size()-1; i>=0; i--)
    {
      Mat testface = images[i];
      int testIndex = labels[i];
      images.erase(images.begin()+i,images.begin()+i+1);
      labels.erase(labels.begin()+i,labels.begin()+i+1);

      //Train on data:
      model->train(images,labels);

      //Test:
      int predIndex = model->predict(testface);
      confusionMatrix[testIndex][predIndex]++;
      if (predIndex==testIndex)
        correctNum++;
      cout << i << "/" << imagesToTrain.size() << ": " << this->index2Label[testIndex] << "-->" << this->index2Label[predIndex] << endl;

      //Put the data back (since we're iterating in reverse, this is valid):
      images.push_back(testface);
      labels.push_back(testIndex);
    }
    cout << "-----------------CONFUSION MATRIX -------------------------" << endl;
    for (int i=0; i<index2Label.size(); i++)
      for (int j=0; j<index2Label.size(); j++)
      {
        if (confusionMatrix[i][j]>0)
          cout << index2Label[i] << "-->" << index2Label[j] << ": " << confusionMatrix[i][j] << endl;
      }
    cout << endl << "Overall Accuracy = " << correctNum << "/" << imagesToTrain.size()
         << " = " << (float)correctNum/imagesToTrain.size() << "." << endl;
  }


  void Update()
  {
    //model->update()
  }


};



class FaceDetector
{
  CascadeClassifier cascade;
  CascadeClassifier leftEyeCascade;
  CascadeClassifier rightEyeCascade;
  ofstream* outLabels; //we keep an open stream for efficiency
  int fileIndex; //when saving we keep track of face index
  string dataPath; //path to save data to

public:



float Distance(const Point2f& p1, const Point2f& p2)
{
  float dx = p2.x-p1.x;
  float dy = p2.y-p1.y;

  return sqrt(dx*dx + dy*dy);
}

Point2f GetCenter(const Point2f& p1, const Point2f& p2)
{
  return Point2f((p2.x+p1.x)/2,(p2.y+p1.y)/2);
}

void GetScaledROIAndHeight(const Point2f& leftEye, const Point2f& rightEye, int width, int& height, Rect& roi)
{
  float dist = Distance(leftEye, rightEye);
  Point2f center = GetCenter(leftEye, rightEye);
  int sideOffset = 0.9*dist;
  int topOffset = 0.9*dist;
  int bottomOffset = 1.8*dist;

  float h_over_w = (double)(bottomOffset+topOffset)/(2*sideOffset);
  height = h_over_w*width;

  roi.x = center.x-sideOffset;
  roi.y = center.y-topOffset;
  roi.width = 2*sideOffset;
  roi.height = topOffset+bottomOffset;
}

bool CropAndResizeFace(const Mat& face, Mat& newFace,  const Rect& roi, int width, int height)
{
  Mat croppedFace = face(roi);

  //cv::imshow("cropped face", croppedFace)
  newFace.release();
  newFace = Mat(width,height,croppedFace.type());
  cv::resize(croppedFace,newFace,Size(width,height));

  //cout << "Cropped face resized to " << newFace.cols << "," << newFace.rows << endl;
  return true;
}
//center is the point to rotate about. newCenter will be the new center of the image.
bool RotateFace(Mat& face, Mat& newFace, Point2f leftEye, Point2f rightEye)
{
  Mat rot_mat(2,3,CV_32FC1);
  //    # get the direction
  Point2f eye_direction(rightEye.x - leftEye.x, rightEye.y - leftEye.y);
  float dist = Distance(leftEye,rightEye);
  float dx = eye_direction.x;
  //float angle = -1*std::acos(dx/dist);

  //    # calc rotation angle in radians
  float  angle = atan2(float(eye_direction.y/2),float(eye_direction.x/2));
  angle *= 180/3.14159265359;


  newFace = Mat(face.rows,face.cols,face.type());
  float scale = 1.0;

  //Point2f center = leftEye;
  Point2f center = GetCenter(leftEye,rightEye);
//  cv::circle(face,center,6,CV_RGB(0,0,0), 3, 8, 0);
//  imshow("centerpoint",face);
//  char key=cv::waitKey(0);

  /// Apply the Affine Transform just found to the src image
  try
  {
    rot_mat = getRotationMatrix2D(center,angle,scale);
    warpAffine( face, newFace, rot_mat, newFace.size() );

  }
  catch(...)
  {
    cerr << "Something went wrong in warpAffine, skipping this face";
    return false;
  }



  return true;
  //  def ScaleRotateTranslate(image, angle, center = None, new_center = None, scale = None, resample=Image.BICUBIC):

//  int x = center[0];
//  int y = center[1];
//  int nx = newCenter[0];
//  int ny = newCenter[1];
//  float sx = scale;
//  float sy = scale;
//
//  float cosine = cos(angle);
//  float sine = sin(angle);
//
//
//  float a = cosine/sx;
//  float b = sine/sx;
//  float c = x-nx*a-ny*b;
//  float d = -sine/sy;
//  float e = cosine/sy;
//  float f = y-nx*d-ny*e;

//    return image.transform(image.size, Image.AFFINE, (a,b,c,d,e,f), resample=resample)
}

//  def CropFace(image, eye_left=(0,0), eye_right=(0,0), offset_pct=(0.2,0.2), dest_sz = (70,70)):
//    # calculate offsets in original image
//    offset_h = math.floor(float(offset_pct[0])*dest_sz[0])
//    offset_v = math.floor(float(offset_pct[1])*dest_sz[1])
//    # get the direction
//    eye_direction = (eye_right[0] - eye_left[0], eye_right[1] - eye_left[1])
//    # calc rotation angle in radians
//    rotation = -math.atan2(float(eye_direction[1]),float(eye_direction[0]))
//    # distance between them
//    dist = Distance(eye_left, eye_right)
//    # calculate the reference eye-width
//    reference = dest_sz[0] - 2.0*offset_h
//    # scale factor
//    scale = float(dist)/float(reference)
//    # rotate original around the left eye
//    image = ScaleRotateTranslate(image, center=eye_left, angle=rotation)
//    # crop the rotated image
//    crop_xy = (eye_left[0] - scale*offset_h, eye_left[1] - scale*offset_v)
//    crop_size = (dest_sz[0]*scale, dest_sz[1]*scale)
//    image = image.crop((int(crop_xy[0]), int(crop_xy[1]), int(crop_xy[0]+crop_size[0]), int(crop_xy[1]+crop_size[1])))
//    # resize it
//    image = image.resize(dest_sz, Image.ANTIALIAS)
//    return image


  bool DetectEyes(const Mat& face, Rect& leftEye, Rect& rightEye)
  {
//    Mat gray;
//    cvtColor( face, gray, CV_BGR2GRAY );
//    equalizeHist( gray, gray );

    int left=face.cols/10;
    int center=face.cols/2;
    int right=face.cols*0.9;
    int top=0.25*face.rows;
    int bottom=0.55*face.rows;
    Rect leftROI(left, top, center-left, bottom-top);
    Rect rightROI(center,top,right-center,bottom-top);

    Mat faceCopy = face;


    cout << "LeftROI was: (" << leftROI.x << "," << leftROI.y << "," << leftROI.width << "," << leftROI.height << ")" << endl << flush;
    cout << "rightROI was: (" << rightROI.x << "," << rightROI.y << "," << rightROI.width << "," << rightROI.height << ")" << endl << flush;
    cout << "facedims are: (" << face.rows << ", " << face.cols << ")" << endl << flush;

    Mat leftEyeRegion(face, leftROI);
    Mat rightEyeRegion(face, rightROI);

    //Left eye:
    vector<Rect> eyes;
    leftEyeCascade.detectMultiScale( leftEyeRegion, eyes,
        1.1, 2, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        |CV_HAAR_SCALE_IMAGE,
        Size(25, 15) );
    if (eyes.size()==0)
      return false;

//    if (eyes.size() == 1)
    Rect aveLeftEye(0,0,0,0);
    Rect aveRightEye(0,0,0,0);

    //cout << "Found " << eyes.size() << " left eyes." << endl << flush;
    for (uint i=0; i<eyes.size(); i++)
    {
//      cout << "Found leftEye at: (" << eyes[i].x << "," << eyes[i].y << "," << eyes[i].width << "," << eyes[i].height << ")" << endl << flush;
      leftEye = Rect(leftROI.x + eyes[i].x,leftROI.y+eyes[i].y,eyes[i].width, eyes[0].height);
//      cout << "Found leftEye at: (" << leftEye.x << "," << leftEye.y << "," << leftEye.width << "," << leftEye.height << ")" << endl;
//      cv::rectangle(faceCopy, leftEye, CV_RGB(0,128,255), 3, 8, 0 );
      aveLeftEye.x += leftEye.x;
      aveLeftEye.y += leftEye.y;
      aveLeftEye.width += leftEye.width;
      aveLeftEye.height += leftEye.height;
    }
    aveLeftEye.x /= eyes.size();
    aveLeftEye.y /= eyes.size();
    aveLeftEye.width /= eyes.size();
    aveLeftEye.height /= eyes.size();
//    cv::rectangle(faceCopy, aveLeftEye, CV_RGB(255,0,0), 3, 8, 0 );
    leftEye=aveLeftEye;
//    else return false;

    //Right eye:
    eyes.clear();
    rightEyeCascade.detectMultiScale( rightEyeRegion, eyes,
        1.1, 2, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        |CV_HAAR_SCALE_IMAGE,
        Size(25, 15) );
    if (eyes.size()==0)
      return false;

    //cout << "Found " << eyes.size() << " right eyes." << endl << flush;
    for (uint i=0; i<eyes.size(); i++)
    {
      rightEye = Rect(rightROI.x + eyes[i].x,rightROI.y+eyes[i].y,eyes[i].width, eyes[i].height);
      //cout << "Found rightEye at: (" << rightEye.x << "," << rightEye.y << "," << rightEye.width << "," << rightEye.height << ")" << endl;
//      cv::rectangle(faceCopy, rightEye, CV_RGB(0,128,255), 3, 8, 0 );
      aveRightEye.x += rightEye.x;
      aveRightEye.y += rightEye.y;
      aveRightEye.width += rightEye.width;
      aveRightEye.height += rightEye.height;
    }
    aveRightEye.x /= eyes.size();
    aveRightEye.y /= eyes.size();
    aveRightEye.width /= eyes.size();
    aveRightEye.height /= eyes.size();

//    cv::rectangle(faceCopy, aveRightEye, CV_RGB(255,0,0), 3, 8, 0 );
    rightEye=aveRightEye;



    return true;
  }

  bool GetAllFaces(const Mat& img, vector<Rect>& faces, Mat& smallImg, double scale=1)
  {
    //vector<Rect> faces;
    smallImg.release();
    smallImg = Mat(cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );

    Mat gray;
    cvtColor( img, gray, CV_BGR2GRAY );
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );

    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        |CV_HAAR_SCALE_IMAGE
        ,
        Size(30, 30) );

    return faces.size()>0;
  }

  bool GetLargestFace(const Mat& img, Mat& face, Rect& pos, double scale=1)
  {
    bool faceFound=false;
    Mat smallImg;
    vector<Rect> faces;
    GetAllFaces(img,faces,smallImg);

    float largestArea=0;
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++)
    {
      float area = r->height*r->width;
      if (area > largestArea)
      {
        face = smallImg(*r);
        pos = *r;
        faceFound=true;
        largestArea = area;
      }
    }

    return faceFound;
  }

  //Designed to return the face overlapping the center of the image:
  bool GetCenterFace(const Mat& img, Mat& face, Rect& pos, double scale=1, int width=100, int height=100)
  {
    bool faceFound=false;
    Mat smallImg;
    vector<Rect> faces;
    GetAllFaces(img,faces,smallImg);

    //Find the center one:               
    Point imgCenter(smallImg.size().width/2, smallImg.size().height/2);
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++)
    {
      if (imgCenter.inside(*r))
      {
          face = smallImg(*r);
          pos = *r;
          faceFound= true;
      }
    }

    if (faceFound)
    {
      //faceFound = StandardizeFaceWithEyes(img, pos, face, width,height);
    }

    return faceFound;
  }

  bool StandardizeFaceWithEyes(const Mat& wholeImg, const Rect& pos, Mat& newFace, int width, int& height)
  {
    if (width < 1)
      return (false);

    Mat face = wholeImg(pos);

    //Detect eyes in the small face:
    Rect lEye, rEye;
    bool eyesFound = DetectEyes(face,lEye,rEye);
    //cout << "Eyes found DetectEyes = " <<eyesFound<< endl;

    if (eyesFound)
    {
      //cout << "Eyes found "<< endl;

      try
      {
        //Define eyes in the wholeImage frame:
        Rect LEye=lEye;
        Rect REye=rEye;
        LEye.x = lEye.x+pos.x;
        LEye.y = lEye.y+pos.y;
        REye.x = rEye.x+pos.x;
        REye.y = rEye.y+pos.y;


        //Expand face and rotate/crop:
        face.release();
        face=wholeImg;

        newFace.release();
        newFace = Mat(face.cols,face.rows,face.type());
        Point2f leftEye(LEye.x+LEye.width/2,LEye.y+LEye.height/2);
        Point2f rightEye(REye.x+REye.width/2,REye.y+REye.height/2);

        eyesFound = this->RotateFace(face,newFace,leftEye,rightEye);
	//cout << "Eyes found RotateFace = " <<eyesFound<< endl;
        if (eyesFound)
        {
          Rect roi;
          int tmpHeight;
          GetScaledROIAndHeight(leftEye, rightEye, width, tmpHeight, roi);
          if (height==-1) //see if height has been initialized (need to do this bc of roundoff error)
            height = tmpHeight;

          if (roi.x<0 || roi.y<0 || roi.width+roi.x>face.cols || roi.height+roi.y > face.rows)
          {
            cout << "***********Not enough face context to expand crop***************" << endl;
            eyesFound = false;
          }

          if (eyesFound){
            eyesFound=CropAndResizeFace(newFace,newFace,roi,width,height);
	    //cout << "Eyes found CropAndResizeFace = " <<eyesFound<< endl;
	  }
	  else{
	    // cout<<"Crop and resize face eyes not found"<<endl;
	  }

	}
	else{
	  //cout<<"Eyes not found at Rotate Face"<<endl;
	}
        if (eyesFound)
        {
          Mat gray;
          cvtColor( newFace, gray, CV_BGR2GRAY );
          equalizeHist( gray, newFace );
	}
      }
      catch(...)
      {
        //cout << "Something went wrong in RotateFace, skipping this one."<<endl;
        return false;
      }
    }
    else
    {
      //cout << "Eyes not found." << endl;
    }

    //cout<<"Returning eyes found = "<<eyesFound <<endl;
    return eyesFound;
  }

  bool Initialize(const string& cascadeName, const string& leftEyeCascadeName, const string& rightEyeCascadeName)
  {
    if( !cascade.load( cascadeName ) )
        cerr << "ERROR: Could not load classifier cascade" << cascadeName << endl;
    if( !leftEyeCascade.load( leftEyeCascadeName ) )
        cerr << "ERROR: Could not load classifier cascade" << leftEyeCascadeName << endl;
    if( !rightEyeCascade.load( rightEyeCascadeName ) )
        cerr << "ERROR: Could not load classifier cascade" << rightEyeCascadeName << endl;

  }

  FaceDetector(const string& cascadeName, const string& leftEyeCascadeName, const string& rightEyeCascadeName): outLabels(NULL)
  {
    if( !cascade.load( cascadeName ) )
        cerr << "ERROR: Could not load classifier cascade" << cascadeName << endl;
    if( !leftEyeCascade.load( leftEyeCascadeName ) )
        cerr << "ERROR: Could not load classifier cascade" << leftEyeCascadeName << endl;
    if( !rightEyeCascade.load( rightEyeCascadeName ) )
        cerr << "ERROR: Could not load classifier cascade" << rightEyeCascadeName << endl;

  }

  FaceDetector(): outLabels(NULL) {}

  ~FaceDetector()
  {
    if (outLabels != NULL)
      delete outLabels;
  }

  bool DetectAndDraw( Mat& img, double scale, int interval=1, int counter=1)
  {
      Mat face;
      Rect leftEye, rightEye;
      Rect pos;
      bool faceDetected=false;

      if (interval%counter==0)
      {
        faceDetected = this->GetCenterFace(img,face,pos,scale);

        if (faceDetected)
        {
          bool eyesDetected = DetectEyes(face,leftEye,rightEye);

          Point center;
          int radius;
          center.x = cvRound((pos.x + pos.width*0.5)*scale);
          center.y = cvRound((pos.y + pos.height*0.5)*scale);
          radius = cvRound((pos.width + pos.height)*0.25*scale);
          circle( img, center, radius, CV_RGB(0,128,255), 3, 8, 0 );
          cv::rectangle(img, leftEye, CV_RGB(0,128,255), 3, 8, 0 );
          cv::rectangle(img, rightEye, CV_RGB(0,128,255), 3, 8, 0 );

//          cv::imshow("face", face);
        }
      }
//      cv::imshow("result", img);
      return (faceDetected);
  }

  //Check if this full path exists, if not create it, if so increment a .N counter on the end.
  void EnsureNewDirExists(const string& tmpDataPath)
  {
    dataPath = tmpDataPath;
    fs::path fullPath( fs::initial_path<fs::path>() );
    int dirIndex=0;
    fullPath = fs::system_complete( fs::path( dataPath.c_str() ) );
    cout << "Looking for directory: " << fullPath << endl;
    while(fs::is_directory( fullPath ))
    {//increment an index for the path:
      cout << "Found it!" << endl;
      stringstream dirstream;
      dirstream << dataPath << "." << dirIndex << "/";
      fullPath = fs::system_complete( fs::path( dirstream.str().c_str() ) );
      dirIndex++;
      cout << "Looking for directory " << fullPath << endl;
    }
    cout << "Did not find it. Creating it." << endl;
    dataPath=fullPath.c_str();
    mkdir(dataPath.c_str(),0755);
  }

  void PrepareToCollectData(const string& rootDataPath, const string& label)
  {
    EnsureNewDirExists(rootDataPath + "/" + label);
    outLabels = new ofstream(string(dataPath + "/labels.csv").c_str());
    assert(outLabels);
    fileIndex = 0; //Since we created a new directory, index can start at 0
  }

  const char* _GetFN(const string& label)
  {
    stringstream fn;
    fn.setf(ios::fixed);
    fn << dataPath << '/' << setw(5) << std::setfill( '0' ) << fileIndex << ".jpg";

    return fn.str().c_str();
  }

void SaveFace(const Mat& face, const string& label)
  {
    string filename = _GetFN(label);
    cout << "Trying to save to " << filename << endl;
    cv::imwrite(filename.c_str(),face);
    (*outLabels) << filename << ";" << label << endl;
    fileIndex++;
  }

  bool CollectFaceInstance(Mat& img, const string& filename, const string& label, ostream& out, double scale=1.0)
  {
    Mat face;
    Rect pos;

    bool faceDetected = this->GetCenterFace(img,face,pos,scale);

    if (faceDetected)
    {
//      cv::imwrite(filename.c_str(),face);
      out << filename << ";" << label << endl;
//      cv::imshow("face", face);
    }
    return faceDetected;
  }
};

//#define FACE_CASCADE_DEFAULT_PATH "../../haarcascades/haarcascade_frontalface_alt.xml"
//#define L_EYE_CASCADE_DEFAULT_PATH "../../haarcascades/haarcascade_mcs_lefteye.xml"
//#define R_EYE_CASCADE_DEFAULT_PATH "../../haarcascades/haarcascade_mcs_righteye.xml"
//#define DATA_DEFAULT_PATH "../../face_data/people"
//#define MODEL_DEFAULT_PATH  "../../face_data/people/model.xml"

class FaceDetectionizer //Detects and recognizes faces.
{
  string dataPath;
  string modelPath;
  FaceRecognizerWLabels model;
  Rect lastPos;

public:
  FaceDetector faceDetector;

  FaceDetectionizer()
  {
    this->Initialize();
  }



/*bool Initialize(const string& cascadeName=FACE_CASCADE_DEFAULT_PATH, const string& leftEyeCascadeName=L_EYE_CASCADE_DEFAULT_PATH, 
		    const string& rightEyeCascadeName=R_EYE_CASCADE_DEFAULT_PATH, const string& mp=MODEL_DEFAULT_PATH, 
		    const string& dp = DATA_DEFAULT_PATH)
{
 faceDetector.Initialize(cascadeName, leftEyeCascadeName, rightEyeCascadeName);
 modelPath=mp;
 dataPath=dp;
 model.Load(modelPath.c_str());
}*/

bool Initialize(    const string& cascadeName=FACE_CASCADE_DEFAULT_PATH, const string& leftEyeCascadeName=L_EYE_CASCADE_DEFAULT_PATH, 
    		    const string& rightEyeCascadeName=R_EYE_CASCADE_DEFAULT_PATH, const string& mp=MODEL_DEFAULT_PATH, 
    		    const string& dp = DATA_DEFAULT_PATH, const string& recoType=RECO_DEFAULT_TYPE)
{
 faceDetector.Initialize(cascadeName, leftEyeCascadeName, rightEyeCascadeName);
 modelPath=mp;
 dataPath=dp;
 model.Initialize(recoType);
 model.Load(modelPath.c_str());
}

  //**************Detect and Recognize a center face*******
  bool DetectAndLabel(const Mat& img, string& label, Rect& pos);

  //*****************Data Collection***********************
  //**Call this before collecting data for a particular label
  bool PrepareToCollectData(const string& label);
  //**After calling PrepareToCollectData for a given label, call this to detect and save face for that label:
  bool DetectAndSaveData(const Mat& img, const string& label, Rect& pos);

  //**************Retrain with current data****************
  bool RetrainModelWithCurrentData();
};

  //**************Detect and Recognize a center face*******
bool FaceDetectionizer::DetectAndLabel(const Mat& frame, string& label, Rect& pos)
{
  Mat face;
  double scale = 1.0;
  int height=100;
  int width=100;

  bool faceFound =   faceDetector.GetCenterFace(frame, face, lastPos, scale, width,height);

  if (faceFound)
  {
    label = model.Predict(face);
    pos = lastPos;
    return true;
  }
  
  return false;
}

  //*****************Data Collection***********************
  //Call this before collecting data for a particular label
bool FaceDetectionizer:: PrepareToCollectData(const string& label)
{
  faceDetector.PrepareToCollectData(dataPath,label);
}

  //After preparing to take data for a given label, call this to detect and save face for that label:
bool FaceDetectionizer::DetectAndSaveData(const Mat& frame, const string& label, Rect& pos)
{
  Mat face;
  double scale = 1.0;
  int height=100;
  int width=100;
  bool faceFound =   faceDetector.GetCenterFace(frame, face, lastPos, scale,width,height);
  if (faceFound)
    {
      faceDetector.SaveFace(face,label);
      pos = lastPos;
      return true;
    }
  return false;
}

  //**************Retrain with current data****************
bool FaceDetectionizer:: RetrainModelWithCurrentData()
{
  model.ReTrain(dataPath);
}

