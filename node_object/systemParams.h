#ifndef SYSTEMPARAMS_H_
#define SYSTEMPARAMS_H_

//Note before changing these that some matlab files depend on
//these as well. Look at lazyObjRecognizer.m and you might just
//want to do a search on disk to find any other instances :)
#define DIR_PATH 	      "/home/mmfps/mmfps/mmfpspkg/data_tmp/"
#define LOG_PATH              "gui/log.txt"
#define GUI_INPUT_DIR         "gui"
#define DETECTION_RESULTS_DIR "logging"
#define DETECTION_RESULTS_PATH "logging/log.txt"  //matlab outputs results to here
#define INDICATOR_FILE        "logging/done"      //matlab touches this file when done
#define DETECTED_IMAGES_DIR   "model_processed"
#define CAMERA_OUTPUT_DIR      "vision_processed"
#define IMAGES_BUFFER_DIR     "vision"
#define SPEECH_OUTPUT_DIR     "speech"
#define OBJECT_HISTOGRAM_DIR  "samba"


// object library and object number

#define OBJ_XML		"code_data/object_list.xml"

#define OBJ_NUM		12

// server option

#define USE_SRV		0

#define SRV_PORT	8080

// video source -- 0: camera, 1: clip, 2: disable

#define VID_SOURCE	1

#define VID_PATH	"test_video/clip1.avi"

// data option

#define DEF_URL		"http://www.google.com"

using namespace std;


#endif
