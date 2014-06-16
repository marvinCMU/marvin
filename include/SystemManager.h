/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

#ifndef SYSTEMMANAGER_H_
#define SYSTEMMANAGER_H_

#include <iostream>
#include <fstream>
#include <string>
#include "SystemParams.h"
#include "SystemUtility.h"
#include "FaceManager.h"
#include "RetailManager.h"
#include "SpeechManager.h"
#include "BnetManager.h"

class SystemManager {
public:
	std::string facePath;
	std::string retailPath;
	std::string speechPath;
	std::string bnetPath;
	std::string responderPath;
	FaceManager *facemanager;
	RetailManager *retailmanager;
	SpeechManager *speechmanager;

	SystemManager (std::string facepath, std::string retailpath, std::string speechpath, std::string bnetpath, std::string responderpath) {
		facePath = facepath;
		retailPath = retailpath;
		speechPath = speechpath;
		bnetPath = bnetpath;
		responderPath = responderpath;
		facemanager = new FaceManager (facePath);
		retailmanager = new RetailManager (retailPath);
		speechmanager = new SpeechManager (speechPath);
		//bnetmanager = new SpeechManager (bentPath);
		//respondermanager = new SpeechManager (responderPath);
	}

	~SystemManager () {}

	// load both face library and retail library
	void loadData () {
		std::string faceCmdList = facemanager->loadCommand();
		std::string faceObjList = facemanager->loadObject();
		std::string retailCmdList = retailmanager->loadCommand();
		std::string retailObjList = retailmanager->loadObject();
		std::cout << SYSTEM_MAN_HEAD << "All library loaded!" << std::endl;
	}

	// return whether any data loaded
	bool hasData () {
		return (facemanager->hasData() || retailmanager->hasData());
	}

	// print the whole library
	void printData () {
		facemanager->printCommand();
		facemanager->printObject();
		retailmanager->printCommand();
		retailmanager->printObject();
	}

	// update both face library and retail library
	void updateFile () {
		facemanager->updateCommandFile ();
		facemanager->updateObjectFile ();
		facemanager->updateSpeechFile ();
		retailmanager->updateCommandFile ();
		retailmanager->updateObjectFile ();
		retailmanager->updateSpeechFile ();
		loadData();
	}

	// update face library
	void updateFaceFile () {
		facemanager->updateCommandFile ();
		facemanager->updateObjectFile ();
		facemanager->updateSpeechFile ();
		loadData();
	}

	// update retail library
	void updateRetailFile () {
		retailmanager->updateCommandFile ();
		retailmanager->updateObjectFile ();
		retailmanager->updateSpeechFile ();
		loadData();
	}

	// update speech models for both libraries
	void updateSpeech () {
		facemanager->updateSpeechFile ();
		retailmanager->updateSpeechFile ();
		std::cout << SYSTEM_MAN_HEAD << "Speech models updated!" << std::endl;
	}

	// update both face library and retail library
	void updateFace () {
		facemanager->loadData ();
	}

	// update both face library and retail library
	void updateRetail () {
		retailmanager->loadData ();
	}

};

/*int main (int argc, char *argv[]) {
	SystemManager *manager = new SystemManager("/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_face/", "/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_retail/", "/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_speech/", "/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_bnet/", "/home/qifan/Dropbox/mmfps/mmfpspkg/library/lib_gui/");
	manager->loadData();
	manager->printData();
	manager->updateSpeech();
	return 0;
}*/

#endif

