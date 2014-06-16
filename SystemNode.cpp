/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

#include <ros/ros.h>
#include <std_srvs/Empty.h>
#include <std_msgs/String.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "SystemManager.h"
#include "SystemParams.h"
#include "SystemUtility.h"

SystemManager* manager;
std_srvs::Empty srv;

// initialize the environment, including deleting all tmp files
void initEnvironment (std::string dirPath, std::string guiPath) {
	// string variable
	char *cmds = new char[100];
	char *logPath = new char[100];

	// delete all contents in the data folder
	sprintf(cmds, "%s%s%s", CMD_RMDIR, dirPath.c_str(), VIDEO_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_RMDIR, dirPath.c_str(), SPEECH_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_RMDIR, dirPath.c_str(), VIDEO_PRO_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_RMDIR, dirPath.c_str(), MODEL_PRO_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_RMDIR, dirPath.c_str(), LOG_TMP.c_str());
	system(cmds);

	// create empty folders
	sprintf(cmds, "%s%s%s", CMD_MKDIR, dirPath.c_str(), VIDEO_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_MKDIR, dirPath.c_str(), SPEECH_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_MKDIR, dirPath.c_str(), VIDEO_PRO_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_MKDIR, dirPath.c_str(), MODEL_PRO_TMP.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_MKDIR, dirPath.c_str(), LOG_TMP.c_str());
	system(cmds);

	// modify log files
	sprintf(logPath, "%s%s", dirPath.c_str(), "logging/log.txt");
	std::ofstream logFile (logPath);
	if (logFile.is_open()) {
		logFile << "00_00_00" << "\n";
		logFile << "hello world" << "\n";
		logFile << "world" << "\n";
		logFile << "hello world";
		logFile.close();
	} else 
		std::cout << SYSTEM_HEAD << "Cannot create/open log file!" << std::endl;

	// create blank images
	std::string imgPath = guiPath + IMG_BLANK;
	sprintf(cmds, "%s%s%s%s%s", CMD_CP, imgPath.c_str(), " ", dirPath.c_str(), "vision_processed/t_00_00_00.jpg");
	system(cmds);
	sprintf(cmds, "%s%s%s%s%s", CMD_CP, imgPath.c_str(), " ", dirPath.c_str(), "model_processed/m_00_00_00.jpg");
	system(cmds);
}


// update service to reload face recognizer library
bool updateFace (std_srvs::Empty::Request& request, std_srvs::Empty::Response& response) {
	manager->updateFace();
	std::cout << SYSTEM_HEAD << "Face library updated" << std::endl;
	return true;
}

// update service to reload retail recognizer library
bool updateRetail (std_srvs::Empty::Request& request, std_srvs::Empty::Response& response) {
	manager->updateRetail();
	std::cout << SYSTEM_HEAD << "Retail library updated" << std::endl;
	return true;
}

// update service to reload speech recognizer library
bool updateSpeech (std_srvs::Empty::Request& request, std_srvs::Empty::Response& response) {
	manager->updateSpeech();
	std::cout << SYSTEM_HEAD << "Speech models updated" << std::endl;
	return true;
}

int main (int argc, char **argv) {
	ros::init (argc, argv, "system_main");
	ros::NodeHandle nh;
	std::string speechLibPath = "";
	std::string faceLibPath = "";
	std::string retailLibPath = "";
	std::string locationLibPath = "";
	std::string bnetLibPath = "";
	std::string guiLibPath = "";
	std::string tmpLibPath = "";

	// read system parameters
	nh.getParam ("faceLibPath", faceLibPath);
	nh.getParam ("retailLibPath", retailLibPath);
	nh.getParam ("speechLibPath", speechLibPath);
	nh.getParam ("locationLibPath", locationLibPath);
	nh.getParam ("bnetLibPath", bnetLibPath);
	nh.getParam ("guiLibPath", guiLibPath);
	nh.getParam ("tmpLibPath", tmpLibPath);

	// init
	//initEnvironment (tmpLibPath, guiLibPath);

	// read recognizer library
	manager = new SystemManager (faceLibPath, retailLibPath, speechLibPath, "", "");
	manager->loadData();
	//manager->updateSpeech();
	manager->printData();

	// subscribers and publisher for node self
	

	// service to update node self
	ros::ServiceServer srvFaceUpd = nh.advertiseService ("sysnode/faceupdate", updateFace);
	ros::ServiceServer srvRetailUpd = nh.advertiseService ("sysnode/retailupdate", updateRetail);
	ros::ServiceServer srvSpeechUpd = nh.advertiseService ("sysnode/speechupdate", updateSpeech);

	std::cout << "<System> System node launched" << std::endl;
	ros::spin();
	return 0;
}

