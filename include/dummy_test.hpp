/*
 * dummy_test.hpp
 *
 *  Created on: Aug 21, 2013
 *      Author: iljoo
 */

#ifndef DUMMY_TEST_HPP_
#define DUMMY_TEST_HPP_

#include <ros/ros.h>
#include <ros/time.h>
#include <ros/package.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <deque>

//opencv includes
#include <cv.h>
#include <opencv/highgui.h>
#include <opencv/cvwimage.h>
#include <opencv/highgui.h>
#include <cv_bridge/CvBridge.h>
#include <cv_bridge/cv_bridge.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
namespace ag = boost::algorithm;

#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/image_encodings.h>

// mouse click
#include<X11/X.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>

using namespace std;

struct objectList
{
  string name;  // object's name : duck clock, minifan ...
  int rank;  // reported rank among the reported list from bNet : 1, 2, 3 ...
};

struct bNetResult
{
  int firedExamplarNum; // number of fired examplars from bNet
  int numOfObjectInResult;  // number of objects reported
  vector<objectList> vObject; // list of the objects
};

struct userFeedback
{
  int isCorrect; // Is the reported object as the 1st rank correct? : 0:no, 1:yes
  string selectedObjt; // selected object name from the reported objected by user
};

struct objtReconResult
{
  int numOfObjectInResult;  // number of objects reported
  vector<objectList> vObject; // list of the objects
};

struct testPerformanceGraph
{
    vector<float> accuracyList; // accuracy for each top number
    double timeForObjectRecon; // time for object recognition
    double timeForTotalProcess; // time until final result is done from bNet after user speech is done
};

struct dummyUnitTestResult
{
  int curTestNum; // current order of test trial
  string targetObjtName; // target object name : duck clock, minifan ...
  string targetSpeech; // target speech commands : tell me review of the shampoo
  objtReconResult esvmResult; // recognized object list
  bNetResult finalResult; // reported result from bNet
  userFeedback finalFeedback; // user feedback

  // result Graph
  testPerformanceGraph resultGraph;
};

struct dummyAllTestResult
{
  string testScenarioName; // test case : speech_clear_in_hand, speech_unclear_on_shelf
  int totalTestNum; // total number of test trials
  vector<dummyUnitTestResult> vTestResult;

  // result Graph
  int numOfTop; // number of top list
};

struct dummyTestScenario
{
  int testIdx;
  string imgFile;
  string command;
};

void dummyCreatTriggerFile(void);
void dummyTrigger(void);
void dummy_test_init(int argc, char** argv);
void callbackSendFakeData(const std_msgs::String::ConstPtr& msg);
void callbackCommandDone(const std_msgs::String::ConstPtr& msg);
void callbackRankedExamplarDone(const std_msgs::String::ConstPtr& msg);
void callbackObjtReconDone(const std_msgs::String::ConstPtr& msg);
void callbackSendFakeFeedback(const std_msgs::String::ConstPtr& msg);

int loadDummyTestScenario(string filename, vector<dummyTestScenario> &vTestScenario);
int loadDummyTestScenario(string filename, vector<dummyTestScenario> &vTestScenario, dummyAllTestResult *testResult);
void dummyMouseClick(int button);

int debugPrint(dummyAllTestResult *testResult);
int debugPrint(dummyUnitTestResult *testResult);
int finalResultPrint(dummyAllTestResult *testResult);

// list of test scenarios
vector<dummyTestScenario> vDummyTestScenario;

// list of test result
dummyAllTestResult gDummyTestResult;


#endif /* DUMMY_TEST_HPP_ */
