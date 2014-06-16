/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

#ifndef SPEECHMANAGER_H_
#define SPEECHMANAGER_H_

#include <ros/ros.h>
#include <std_srvs/Empty.h>
#include <std_msgs/String.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "SystemParams.h"
#include "SystemUtility.h"

// No functions designed yet, jsut keep the header
class SpeechManager {
public:
	std::string libPath;

	SpeechManager (std::string speechpath) {
		libPath = speechpath;
	}

	~SpeechManager () {}

};

/*int main (int argc, char *argv[]) {
	FaceManager *facemanager = new FaceManager("/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_face/");
	std::cout << facemanager->loadObject() << std::endl;
	facemanager->printObject();
	std::cout << facemanager->loadCommand() << std::endl;
	facemanager->printCommand();

	RetailManager *remanager = new RetailManager("/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_retail/");
	std::cout << remanager->loadObject() << std::endl;
	remanager->printObject();
	std::cout << remanager->loadCommand() << std::endl;
	remanager->printCommand();

	SpeechManager *spmanager = new SpeechManager("/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_speech/");
}*/

#endif

