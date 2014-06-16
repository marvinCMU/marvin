/*
 * dummy_test.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: iljoo
 */

#include "dummy_test.hpp"

bool gDummyEnableLocalization = false;
bool gDummyEnableDummyTest = false;
bool gDummyAutoTest = true;

ros::Publisher dummyTriggerPublisher;
ros::Publisher dummyRetailCommanderPublisher;
image_transport::Publisher dummyImagePublisher;
image_transport::Publisher dummyImagePublisherHiRes;

ros::Subscriber dummySendSubscriber;
ros::Subscriber dummyCommandSubscriber;
ros::Subscriber dummyBnetRankedExemSubscriber;
ros::Subscriber dummyRetailDetectionSubscriber;
ros::Subscriber dummyBnetResultSubscriber;

string mmfpsPath = ros::package::getPath("mmfpspkg");
string dummyBnetPath = mmfpsPath + string("/../ros/bnet");
string dummyBnetDataNet = dummyBnetPath + string("/learned_nets/dataNet.xdsl");
string dummyBnetNet = dummyBnetPath + string("/learned_nets/net.xdsl");
string dummySambaImg =  mmfpsPath + string("/data_tmp/samba/image.bmp");
string dummySambaHistogram =  mmfpsPath + string("/data_tmp/samba/object_histograms.txt");
string dummyTestDirPath = mmfpsPath + string("/node_dummy_tester/");
string dummyImagePath = dummyTestDirPath + string("/img/");
string dummyTriggerPath = dummyTestDirPath + string("dummyTrigger.txt");


//string dummyScriptPath = dummyTestDirPath + string("speech_clear_in_hand_repeat_1.txt");
//string dummyScriptPath = dummyTestDirPath + string("speech_clear_on_shelf_repeat_1.txt");
//string dummyScriptPath = dummyTestDirPath + string("speech_unclear_in_hand_repeat_1.txt");
//string dummyScriptPath = dummyTestDirPath + string("speech_unclear_on_shelf_repeat_5.txt");
//string dummyScriptPath = dummyTestDirPath + string("speech_partial_in_hand_repeat_1.txt");
string dummyScriptPath = dummyTestDirPath + string("speech_partial_on_shelf_repeat_1.txt");
//string dummyScriptPath = dummyTestDirPath + string("dummy_test.txt");

int dummyTestCnt = 0;

int main(int argc, char **argv){

  dummy_test_init(argc, argv);

  ros::Rate loop_rate(10);

  while (ros::ok())
  {
    if(gDummyAutoTest)
      dummyCreatTriggerFile(); // automated test : create a trigger file periodically
    dummyTrigger(); // pass trigger word when a dummy file exists

    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}

void dummy_test_init(int argc, char** argv)
{
  ros::init(argc, argv, "dummyTester");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);

  if (ros::param::has("/locationActive"))
      ros::param::get("/locationActive", gDummyEnableLocalization);
  else
    ROS_WARN("Could not find parameter %s.", "/locationActive");

  // delete bNet network file and histogram file
  //if( remove(dummyBnetDataNet.c_str()) != 0 )
  //    printf( "Error deleting file\n" );
  //if( remove(dummyBnetNet.c_str()) != 0 )
  //      printf( "Error deleting file\n" );
  if( remove(dummySambaImg.c_str()) != 0 )
        printf( "Error deleting file\n" );
  if( remove(dummySambaHistogram.c_str()) != 0 )
        printf( "Error deleting file\n" );

  // publisher
  dummyImagePublisher = it.advertise("gui/image_raw/original", 1);
  dummyImagePublisherHiRes = it.advertise("gui/image_raw/hi_res", 1);
  dummyTriggerPublisher = nh.advertise< std_msgs::String >("trigger/output", 1000);
  dummyRetailCommanderPublisher = nh.advertise< std_msgs::String >("retailCommander/output", 1000);

  // subscriber
  dummySendSubscriber = nh.subscribe("dummyTest/sendFake", 1000, callbackSendFakeData);
  dummyCommandSubscriber = nh.subscribe("dialog/retail", 1000, callbackCommandDone);
  dummyBnetRankedExemSubscriber  = nh.subscribe("bnet/ranked_exemplars", 1000, callbackRankedExamplarDone);
  dummyRetailDetectionSubscriber = nh.subscribe("retail/object_detection_result", 1000, callbackObjtReconDone);
  dummyBnetResultSubscriber = nh.subscribe("bnet/final_result", 1000, callbackSendFakeFeedback);

  // load test script file
  if(argc)
  {
    string tmpScriptPath;
    for (int n = 1; n < argc; n++)
    {
        cout << n << ": " << argv[ n ] << '\n';
    }
    tmpScriptPath = dummyTestDirPath + string(argv[1]);
    loadDummyTestScenario(tmpScriptPath, vDummyTestScenario, &gDummyTestResult);
  }
  else
    loadDummyTestScenario(dummyScriptPath, vDummyTestScenario, &gDummyTestResult);

}

void dummyCreatTriggerFile(void)
{
  static int triggerSleepCnt = 150;

  if(triggerSleepCnt++ < 200) // 20 seconds
  {
    return;
  }
  triggerSleepCnt = 0;
  string tmpCmd = "touch " + dummyTriggerPath;

  system(tmpCmd.c_str());

  ROS_INFO("Trigger file is created: test cnt %d", triggerSleepCnt);

}

void dummyTrigger(void)
{
  static int sleepCnt = 0;

  if((sleepCnt++%10) != 0)
  {
    return;
  }

  ifstream fin(dummyTriggerPath.c_str());
  if (!fin)
  {
    //string err = "No trigger file " + dummyTriggerPath;
    //ROS_INFO("%s, test cnt %d", err.c_str(), dummyTestCnt);
    return;
  }
  if( remove(dummyTriggerPath.c_str()) != 0 )
    printf( "Error deleting file\n" );
  //else
  //  printf( "File successfully deleted\n" );

  if(dummyTestCnt >= vDummyTestScenario.size())
  {
    ROS_INFO("Test done, test cnt %d", dummyTestCnt);
    finalResultPrint(&gDummyTestResult);
    //return;
    exit(1);
  }

  std_msgs::String msg;
  msg.data = string("marvin");
  ROS_INFO("Trigger : %s, test cnt %d", msg.data.c_str(), dummyTestCnt);

  dummyTriggerPublisher.publish(msg);
}

int loadDummyTestScenario(string filename, vector<dummyTestScenario> &vTestScenario)
{
  cout << "File name: " << filename << endl;

  ifstream fin;
  fin.open(filename.c_str(), ifstream::in);
  int testCnt = 0;

  if(fin == NULL)
  {
    cout << "File open error" << endl;
    return -1;
  }

  while(!fin.eof())
  {
    dummyTestScenario testScenario;
    if(getline(fin, testScenario.imgFile))
    {
      if(getline(fin, testScenario.command))
      {
        testScenario.testIdx = testCnt;
        vTestScenario.push_back(testScenario);
      }
      else
        break;
    }
    else
      break;

    printf("idx: %d, file: %s, cmd: %s\n",
        testScenario.testIdx,testScenario.imgFile.c_str(), testScenario.command.c_str());

    testCnt++;
  }

  fin.close();
  cout << "Loading done" << endl;
  return 0;
}

int loadDummyTestScenario(string filename, vector<dummyTestScenario> &vTestScenario, dummyAllTestResult *testResult)
{
  cout << "File name: " << filename << endl;

  ifstream fin;
  fin.open(filename.c_str(), ifstream::in);
  int testCnt = 0;

  if(fin == NULL)
  {
    cout << "File open error" << endl;
    return -1;
  }

  // add test scenario to result structure
  testResult->testScenarioName = filename;

  while(!fin.eof())
  {
    dummyTestScenario testScenario;
    dummyUnitTestResult unitTestResult;
    if(getline(fin, testScenario.imgFile))
    {
      if(getline(fin, testScenario.command))
      {
        testScenario.testIdx = testCnt;
        unitTestResult.curTestNum = testCnt;
        vTestScenario.push_back(testScenario);

        // add target object's name to result structure
        istringstream iss(testScenario.imgFile);
        getline(iss, unitTestResult.targetObjtName, '_');
        // add target speech command to result structure
        unitTestResult.targetSpeech = testScenario.command;
        // add unitTest result to list
        testResult->vTestResult.push_back(unitTestResult);
      }
      else
        break;
    }
    else
      break;

    printf("idx: %d, objt: %s, file: %s, cmd: %s\n",
        testScenario.testIdx, unitTestResult.targetObjtName.c_str(), testScenario.imgFile.c_str(), testScenario.command.c_str());

    testCnt++;
  }

  // add total number of test cases to result structure
  testResult->totalTestNum = vTestScenario.size();

  // update interested number of top list in final result
  testResult->numOfTop = 4;

  fin.close();
  cout << "Loading done" << endl;
  return 0;
}

int debugPrint(dummyAllTestResult *testResult)
{
  if(testResult == NULL)
  {
    printf("NULL pointer\n");
    return -1;
  }

  printf("======================================================");
  printf("Debug Print dummyAllTestResult structure");
  printf("testScenarioName : %s\n", testResult->testScenarioName.c_str());
  printf("totalTestNum : %d\n", testResult->totalTestNum);
  for(int i=0; i<testResult->vTestResult.size(); i++)
  {
    printf("curTestNum : %d\n", testResult->vTestResult[i].curTestNum);
    printf("targetObjtName : %s\n", testResult->vTestResult[i].targetObjtName.c_str());
    printf("targetSpeech : %s\n", testResult->vTestResult[i].targetSpeech.c_str());
    printf("===> final result\n");
    printf("firedExamplarNum : %d\n", testResult->vTestResult[i].finalResult.firedExamplarNum);
    for(int j=0; j< testResult->vTestResult[i].finalResult.numOfObjectInResult; j++)
    {
      printf("finalResult name : %s\n", testResult->vTestResult[i].finalResult.vObject[j].name.c_str());
      printf("finalResult rank : %d\n", testResult->vTestResult[i].finalResult.vObject[j].rank);
    }
    printf("===> user feedback\n");
    printf("user ans : %d\n", testResult->vTestResult[i].finalFeedback.isCorrect);
    printf("selected objt : %s\n", testResult->vTestResult[i].finalFeedback.selectedObjt.c_str());
    printf("\n");
  }
}

int debugPrint(dummyUnitTestResult *testResult)
{
  if(testResult == NULL)
  {
    printf("NULL pointer\n");
    return -1;
  }

  printf("======================================================");
  printf("Debug Print dummyUnitTestResult structure");

  printf("curTestNum : %d\n", testResult->curTestNum);
  printf("targetObjtName : %s\n", testResult->targetObjtName.c_str());
  printf("targetSpeech : %s\n", testResult->targetSpeech.c_str());
  printf("===> final result\n");
  printf("firedExamplarNum : %d\n", testResult->finalResult.firedExamplarNum);
  for(int j=0; j< testResult->finalResult.numOfObjectInResult; j++)
  {
    printf("finalResult name : %s\n", testResult->finalResult.vObject[j].name.c_str());
    printf("finalResult rank : %d\n", testResult->finalResult.vObject[j].rank);
  }
  printf("===> user feedback\n");
  printf("user ans : %d\n", testResult->finalFeedback.isCorrect);
  printf("selected objt : %s\n", testResult->finalFeedback.selectedObjt.c_str());
  printf("\n");
}

int finalResultPrint(dummyAllTestResult *testResult)
{
  if(testResult == NULL)
  {
    printf("NULL pointer\n");
    return -1;
  }
  float avgAccuracy[100] = {0};
  float avgTimeTotalProcess = 0.0;
  float avgTimeObjectRecon = 0.0;
  int avgNumOfExamplar = 0;

  printf("======================================================\n");
  printf("Print dummyAllTestResult structure for graph\n");
  printf("testScenarioName : %s\n", testResult->testScenarioName.c_str());
  printf("totalTestNum : %d\n", testResult->totalTestNum);
  for(int i=0; i<testResult->vTestResult.size(); i++)
  {
    // Test number : Object Name : # of exam : Accuracy 1 : Accuracy 2 : Accuracy 3 : Accuracy 4 : Total Time : Object Recon Time
    printf("%5d %17s %4d %3.1f %3.1f %3.1f %3.1f %05.1f %05.1f %s\n",
        testResult->vTestResult[i].curTestNum,
        testResult->vTestResult[i].targetObjtName.c_str(),
        testResult->vTestResult[i].finalResult.firedExamplarNum,
        testResult->vTestResult[i].resultGraph.accuracyList[0]*100,
        testResult->vTestResult[i].resultGraph.accuracyList[1]*100,
        testResult->vTestResult[i].resultGraph.accuracyList[2]*100,
        testResult->vTestResult[i].resultGraph.accuracyList[3]*100,
        testResult->vTestResult[i].resultGraph.timeForTotalProcess,
        testResult->vTestResult[i].resultGraph.timeForObjectRecon,
        testResult->vTestResult[i].targetSpeech.c_str());

    // Accumulate values to calculate average
    for(int j=0; j<testResult->numOfTop; j++)
    {
      avgAccuracy[j] += testResult->vTestResult[i].resultGraph.accuracyList[j];
    }
    avgTimeTotalProcess += testResult->vTestResult[i].resultGraph.timeForTotalProcess;
    avgTimeObjectRecon += testResult->vTestResult[i].resultGraph.timeForObjectRecon;
    avgNumOfExamplar += testResult->vTestResult[i].finalResult.firedExamplarNum;
  }
  // Calculate average
  for(int j=0; j<testResult->numOfTop; j++)
  {
    avgAccuracy[j] /= testResult->totalTestNum;
  }
  avgTimeTotalProcess /= testResult->totalTestNum;
  avgTimeObjectRecon /= testResult->totalTestNum;
  avgNumOfExamplar /= testResult->totalTestNum;

  // Test number : Object Name : Accuracy 1 : Accuracy 2 : Accuracy 3 : Accuracy 3 : Total Time : Object Recon Time
  printf("%5d %17s %4d %3.1f %3.1f %3.1f %3.1f %5.1f %5.1f\n",
      0,
      "Total",
      avgNumOfExamplar,
      avgAccuracy[0]*100,
      avgAccuracy[1]*100,
      avgAccuracy[2]*100,
      avgAccuracy[3]*100,
      avgTimeTotalProcess,
      avgTimeObjectRecon );
}

int findNumOfString(string& source, string const& find, string const& replace)
{
  int numOfString = 0;
  for(std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
  {
      source.replace(i, find.length(), replace);
      i += replace.length() - find.length() + 1;
      numOfString++;
  }
  return numOfString;
}

void callbackSendFakeData( const std_msgs::String::ConstPtr& msg )
{
  cv::Mat image;
  string dummyImageFile;

  // Read and publish image from test script file
  if(dummyTestCnt < vDummyTestScenario.size())
  {
    dummyImageFile = dummyImagePath + vDummyTestScenario[dummyTestCnt].imgFile;
    image = cv::imread(dummyImageFile, CV_LOAD_IMAGE_COLOR);
    if(! image.data )
    {
      ROS_INFO("callbackSendFakeData : Could not open or find the image %s\n", dummyImageFile.c_str());
      return;
    }
    // Convert image to image message format
    cv_bridge::CvImage imageMsg( std_msgs::Header(),
                    sensor_msgs::image_encodings::BGR8, image);

    if(gDummyEnableLocalization)
    {
      // Publish image for object recognition
      dummyImagePublisher.publish( imageMsg.toImageMsg() );
      // Publish image for localization
      dummyImagePublisherHiRes.publish( imageMsg.toImageMsg() );
    }
    else
    {
      dummyImagePublisher.publish( imageMsg.toImageMsg() );
    }
    ROS_INFO("callbackSendFakeData : send fake image %s", dummyImageFile.c_str());

    // Read and publish command and publish
    std_msgs::String tmpMsg;
    tmpMsg.data = vDummyTestScenario[dummyTestCnt].command;
    dummyRetailCommanderPublisher.publish(tmpMsg);
    ROS_INFO("callbackSendFakeData : send fake command : %s", tmpMsg.data.c_str());
  }
  else
  {
    ROS_INFO("callbackSendFakeData : test done idx %d", dummyTestCnt);
    return;
  }

}

void callbackCommandDone(const std_msgs::String::ConstPtr& msg)
{
  ROS_INFO("callbackCommandDone : test cnt %d", dummyTestCnt);

  ////////////////////////////////////////////////////////////////////////////////////////
  // update result graph structure
  ////////////////////////////////////////////////////////////////////////////////////////

  // latency
  gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForTotalProcess = ros::Time::now().toNSec();
}

void callbackRankedExamplarDone(const std_msgs::String::ConstPtr& msg)
{
  ROS_INFO("callbackRankedExamplarDone : test cnt %d", dummyTestCnt);

  ////////////////////////////////////////////////////////////////////////////////////////
  // update result graph structure
  ////////////////////////////////////////////////////////////////////////////////////////

  // latency
  gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForObjectRecon = ros::Time::now().toNSec();

  // count number of examplars
  string msgData = msg->data;
  ag::trim(msgData); //like python strip()
  gDummyTestResult.vTestResult[dummyTestCnt].finalResult.firedExamplarNum = findNumOfString(msgData,"\n","|") + 1;
  printf("num of fired examplar %d\n", gDummyTestResult.vTestResult[dummyTestCnt].finalResult.firedExamplarNum);
}

void callbackObjtReconDone(const std_msgs::String::ConstPtr& msg)
{
  ROS_INFO("callbackObjtReconDone : test cnt %d", dummyTestCnt);

  ////////////////////////////////////////////////////////////////////////////////////////
  // update result graph structure
  ////////////////////////////////////////////////////////////////////////////////////////

  // latency
  gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForObjectRecon =
      (ros::Time::now().toNSec() - gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForObjectRecon) *  1e-6;
  ROS_INFO("callbackObjtReconDone : test cnt %d latency for objet recon %f ms", dummyTestCnt,
      gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForObjectRecon);
}

void callbackSendFakeFeedback( const std_msgs::String::ConstPtr& msg )
{
  ROS_INFO("callbackSendFakeFeedback : test cnt %d final result : %s", dummyTestCnt, msg->data.c_str());

  ////////////////////////////////////////////////////////////////////////////////////////
  // update result structure
  ////////////////////////////////////////////////////////////////////////////////////////
  istringstream iss(msg->data);
  string token;
  deque<string> lines;
  char* temp=NULL;
  char line[255];
  int tmpCnt = 0;

  ////////////////////////////////////////////////////////////////////////////////////////
  // update result structure
  ////////////////////////////////////////////////////////////////////////////////////////
  while( getline( iss, token ) )
  {
    lines.push_back(token);
  }

  lines.pop_front(); //retail
  //printf("%s\n",lines.front().c_str());

  //commands : only one for now, don't store
  lines.pop_front(); // read next line for next process
  //printf("%s\n",lines.front().c_str());

  //objects : multiple read in one line
  strcpy(line, lines.front().c_str());
  temp = strtok( line, " ");
  lines.pop_front(); // read next line for next process
  //printf("gDummyTestResult.vTestResult size %d\n",gDummyTestResult.vTestResult.size());
  gDummyTestResult.vTestResult[dummyTestCnt].finalResult.numOfObjectInResult = 0;
  //printf("%s\n",temp);
  tmpCnt = 0;
  while( temp!=NULL)
  {
    objectList tmpObjt;
    tmpObjt.name = string(temp);
    //printf("tmpObjt %d name %s\n", tmpCnt, tmpObjt.name.c_str());
    gDummyTestResult.vTestResult[dummyTestCnt].finalResult.vObject.push_back(tmpObjt);
    tmpCnt++;
    temp = strtok(NULL, " ");
  }
  gDummyTestResult.vTestResult[dummyTestCnt].finalResult.numOfObjectInResult = tmpCnt;

  //rank : the first string per each line
  tmpCnt = 0;
  while(tmpCnt < gDummyTestResult.vTestResult[dummyTestCnt].finalResult.numOfObjectInResult)
  {
    objectList tmpObjt;
    //printf("tmpObjt 1\n");
    if(lines.front().c_str() == NULL)
      break;
    strcpy(line, lines.front().c_str());
    temp = strtok( line, " ");
    lines.pop_front(); // read next line for next process

    if(tmpCnt < gDummyTestResult.vTestResult[dummyTestCnt].finalResult.vObject.size())
    {
      gDummyTestResult.vTestResult[dummyTestCnt].finalResult.vObject[tmpCnt].rank = atoi(temp);
      //printf("tmpObjt %d rank %d\n", tmpCnt, atoi(temp));
    }
    //printf("tmpObjt 2\n");

    tmpCnt++;
  }

  ////////////////////////////////////////////////////////////////////////////////////////
  // update result graph structure
  ////////////////////////////////////////////////////////////////////////////////////////
#if 1
  // accuracy
  int feedbackAnswer = 0;  // is the reported object what user wanted?
  int feedbackObjtect = 0;  // what object is what user meant?
  vector<float> tmpAccuaryList;
  for(int i=0; i < gDummyTestResult.numOfTop; i++)
  {
    float tmpAccuracy = 0;
    // check if the target object is within the i number of top list
    for(int j=0; j < i+1; j++)
    {
      if(j < gDummyTestResult.vTestResult[dummyTestCnt].finalResult.vObject.size())
      {
        if( strcmp(gDummyTestResult.vTestResult[dummyTestCnt].finalResult.vObject[j].name.c_str(),
            gDummyTestResult.vTestResult[dummyTestCnt].targetObjtName.c_str()) == 0)
        {
          /*printf("Target Obj %s is in %d : %s order in the final list %d\n",
              gDummyTestResult.vTestResult[dummyTestCnt].targetObjtName.c_str(),
              j, gDummyTestResult.vTestResult[dummyTestCnt].finalResult.vObject[j].name.c_str(),
              gDummyTestResult.numOfTop);*/
          tmpAccuracy = 1;
          feedbackAnswer = 1;
          feedbackObjtect = j;
        }
      }
    }
    /*printf("Target Obj %s's %d top accuracy is %0.2f\n",
        gDummyTestResult.vTestResult[dummyTestCnt].targetObjtName.c_str(),
        i, tmpAccuracy);*/
    tmpAccuaryList.push_back(tmpAccuracy);
  }
  gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.accuracyList = tmpAccuaryList;

  // latency
  gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForTotalProcess =
      (ros::Time::now().toNSec() - gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForTotalProcess) * 1e-6;
  printf("Total latency after user speech %f ms\n", gDummyTestResult.vTestResult[dummyTestCnt].resultGraph.timeForTotalProcess);
  printf("feedbackAnswer %d, feedbackObjtect %d\n", feedbackAnswer, feedbackObjtect);
#endif
  dummyTestCnt++;

  ////////////////////////////////////////////////////////////////////////////////////////
  // fake mouse click
  ////////////////////////////////////////////////////////////////////////////////////////
  if(!gDummyAutoTest)
    return;

  //Get system window
  Display *dpy;
  Window root_window;
  int x, y;

  if(feedbackAnswer) // Right Button
  {
    //Move mouse to right object and click
    x = 400;
    switch(feedbackObjtect)
    {
      case 0:
        y = 280;
        break;

      case 1:
        y = 480;
        break;

      case 2:
        y = 680;
        break;

      case 3:
        y = 880;
        break;

      default:
        printf("No matched object!!\n");
        y = 280;
        break;
    }
    printf("%d returned object is correct\n", feedbackObjtect);

    dpy = XOpenDisplay(0);
    root_window = XRootWindow(dpy, 0);
    XSelectInput(dpy, root_window, KeyReleaseMask);

    XWarpPointer(dpy, NULL, root_window, 0, 0, 0, 0, x, y);
    XFlush(dpy);
    XCloseDisplay(dpy);

    usleep(1000000);
    dummyMouseClick(Button1);

    //Move mouse to yes button and click
    x = 1450;
    y = 220;

    dpy = XOpenDisplay(0);
    root_window = XRootWindow(dpy, 0);
    XSelectInput(dpy, root_window, KeyReleaseMask);

    XWarpPointer(dpy, NULL, root_window, 0, 0, 0, 0, x, y);
    XFlush(dpy);
    XCloseDisplay(dpy);

    usleep(2000000);
    dummyMouseClick(Button1);

    ROS_INFO("callbackSendFakeFeedback : send fake mouse click at %d, %d", x, y);

  }
  else  // Wrong Button
  {
    //Move mouse to yes button and click
    x = 1550;
    y = 220;

    dpy = XOpenDisplay(0);
    root_window = XRootWindow(dpy, 0);
    XSelectInput(dpy, root_window, KeyReleaseMask);

    XWarpPointer(dpy, NULL, root_window, 0, 0, 0, 0, x, y);
    XFlush(dpy);
    XCloseDisplay(dpy);

    usleep(2000000);
    dummyMouseClick(Button1);

    ROS_INFO("callbackSendFakeFeedback : send fake mouse click at %d, %d", x, y);
  }
}

void dummyMouseClick(int button)
{
  Display *display = XOpenDisplay(NULL);

  XEvent event;

  if(display == NULL)
  {
    fprintf(stderr, "Errore nell'apertura del Display !!!\n");
    exit(EXIT_FAILURE);
  }

  memset(&event, 0x00, sizeof(event));

  event.type = ButtonPress;
  event.xbutton.button = button;
  //event.xbutton.same_screen = True;

  XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);

  event.xbutton.subwindow = event.xbutton.window;

  while(event.xbutton.subwindow)
  {
    event.xbutton.window = event.xbutton.subwindow;

    XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
  }

  if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");

  XFlush(display);

  usleep(100000);

  event.type = ButtonRelease;
  event.xbutton.state = 0x100;

  if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");

  XFlush(display);

  XCloseDisplay(display);
}
