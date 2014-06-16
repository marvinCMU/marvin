/* 
 * Initialize all the files
 * Project: Multimodal First Person Sensing (MFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include "systemParams.h"


#define	CMD_FILE 	"rm "
#define CMD_DIR	 	"rm -rf "
#define CMD_COPY 	"cp "
#define CMD_FOLDER	"mkdir "

using namespace std;

int main () {

	// parse directory path
	string dirPath = DIR_PATH;
	string envPath = getenv("HOME");
	int homeloc = dirPath.find("~");
	if (homeloc>=0)
	  dirPath.replace(homeloc, 1, envPath);
	
	// string variable
	char *cmds = new char[100];
	char *logPath = new char[100];
	
	// delete all contents in the data folder
	
	sprintf(cmds, "%s%s", CMD_DIR, dirPath.c_str());
	system(cmds);

	// sprintf(cmds, "%s%s%s", CMD_DIR, dirPath.c_str(), IMAGES_BUFFER_DIR);
	// system(cmds);
	// sprintf(cmds, "%s%s%s", CMD_DIR, dirPath.c_str(), SPEECH_OUTPUT_DIR);
	// system(cmds);
	// sprintf(cmds, "%s%s%s", CMD_DIR, dirPath.c_str(), CAMERA_OUTPUT_DIR);
	// system(cmds);
	// sprintf(cmds, "%s%s%s", CMD_DIR, dirPath.c_str(), DETECTED_IMAGES_DIR);
	// system(cmds);
	// sprintf(cmds, "%s%s%s", CMD_DIR, dirPath.c_str(), DETECTION_RESULTS_DIR);
	// system(cmds);
	// sprintf(cmds, "%s%s%s", CMD_DIR, dirPath.c_str(), GUI_INPUT_DIR);
	// system(cmds);

	// create empty folders
	sprintf(cmds, "%s%s", CMD_FOLDER, dirPath.c_str());
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_FOLDER, dirPath.c_str(), IMAGES_BUFFER_DIR);
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_FOLDER, dirPath.c_str(), SPEECH_OUTPUT_DIR);
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_FOLDER, dirPath.c_str(), CAMERA_OUTPUT_DIR);
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_FOLDER, dirPath.c_str(), DETECTED_IMAGES_DIR);
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_FOLDER, dirPath.c_str(), DETECTION_RESULTS_DIR);
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_FOLDER, dirPath.c_str(), GUI_INPUT_DIR);
	system(cmds);
	sprintf(cmds, "%s%s%s", CMD_FOLDER, dirPath.c_str(), OBJECT_HISTOGRAM_DIR);
	system(cmds);
	sprintf(cmds, "chmod 777 %s%s", dirPath.c_str(), OBJECT_HISTOGRAM_DIR);
	system(cmds);
	
	// initialize gui input files to hello world:
	sprintf(logPath, "%s%s", dirPath.c_str(), LOG_PATH);
	ofstream logFile (logPath);
	if (logFile.is_open()) {
		for (int i=0; i<4; i++){
			if (i==0) {
				logFile << "00_00_00" << "\n";
			} else if (i==1) {
				logFile << "hello world" << "\n";
			} else if (i==2) {
				logFile << "world" << "\n";
			} else if (i==3) {
				logFile << "hello world";
			} else {}
		}
		logFile.close();
	} else {
	  cout << "<Interface> Cannot create/open log file! logpath= "<<logPath << endl;
	}

	// create blank images
	sprintf(cmds, "%s%s%s%s%s", CMD_COPY, "code_data/blank.jpg ", dirPath.c_str(), CAMERA_OUTPUT_DIR, "/t_00_00_00.jpg");
	system(cmds);
	sprintf(cmds, "%s%s%s%s%s", CMD_COPY, "code_data/blank.jpg ", dirPath.c_str(), DETECTED_IMAGES_DIR, "/m_00_00_00.jpg");
	system(cmds);

	return 0;
}
