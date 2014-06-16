/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */
#include "ros/ros.h"

#ifndef SYSTEMPARAMS_H_
#define SYSTEMPARAMS_H_

using namespace std;

#define	CMD_RM 		"rm "
#define CMD_RMDIR 	"rm -rf "
#define CMD_CP 		"cp "
#define CMD_MKDIR	"mkdir "

#define DEF_URL		"http://www.google.com"


const std::string FACE_OBJ_XML = "face_object.xml";
const std::string FACE_CMD_XML = "face_command.txt";

const std::string RETAIL_OBJ_XML = "retail_object.xml";
const std::string RETAIL_CMD_XML = "retail_command.txt";

const std::string VIDEO_TMP = "vision";
const std::string SPEECH_TMP = "speech";
const std::string MODEL_PRO_TMP = "model_processed";
const std::string VIDEO_PRO_TMP = "vision_processed";
const std::string LOG_TMP = "logging";
const std::string IMG_BLANK = "blank.jpg";

// for speech purposes
const std::string FACE_LAN = "face_speech.lang";
const std::string FACE_COR = "face_speech.corpus";
const std::string FACE_DIC = "face_speech.dic";
const std::string FACE_LM = "face_speech.lm";

const std::string RETAIL_LAN = "retail_speech.lang";
const std::string RETAIL_COR = "retail_speech.corpus";
const std::string RETAIL_DIC = "retail_speech.dic";
const std::string RETAIL_LM = "retail_speech.lm";

// for display
const std::string FACE_HEAD = "<Face> ";
const std::string RETAIL_HEAD = "<Retail> ";
const std::string LOCATION_HEAD = "<Location> ";
const std::string SPEECH_HEAD = "<Speech> ";
const std::string BNET_HEAD = "<Bnet> ";
const std::string SYSTEM_HEAD = "<System> ";

const std::string FACE_MAN_HEAD = "<FaceManager> ";
const std::string RETAIL_MAN_HEAD = "<RetailManager> ";
const std::string LOCATION_MAN_HEAD = "<LocationManager> ";
const std::string SPEECH_MAN_HEAD = "<SpeechManager> ";
const std::string BNET_MAN_HEAD = "<BnetManager> ";
const std::string SYSTEM_MAN_HEAD = "<SystemManager> ";

const std::string XXX_OBJ_XML = "xxx_object.xml";
const std::string XXX_CMD_XML = "xxx_command.txt";
const std::string XXX_HEAD = "<XXX> ";
const std::string XXX_MAN_HEAD = "<XXXManager> ";
 
//These are old values from systemParams.h, tweaked and copied over here: 
#include <ros/package.h>
const std::string ROS_ROOT = ros::package::getPath("mmfpspkg");
const std::string DIR_PATH = ROS_ROOT + string("/data_tmp/");
const std::string LOG_PATH =             "gui/log.txt";
const std::string GUI_INPUT_DIR =        "gui";
const std::string DETECTION_RESULTS_PATH = "logging/log.txt";  //matlab outputs results to here
const std::string INDICATOR_FILE =       "logging/done";      //matlab touches this file when done
const std::string RANKED_EXEMPLARS_FILE = DIR_PATH + string("/") + VIDEO_TMP + string("/ranked_exemplars.txt");
const std::string FIRED_EXEMPLARS_FILE = DIR_PATH + string("/") + VIDEO_PRO_TMP + string("/fired_exemplars.txt");
const std::string LOCATION_IMAGE_PATH = DIR_PATH + string("/samba/image.bmp");
const std::string LOCATION_RESULTS_PATH = DIR_PATH + string("/samba/object_histograms.txt");

const std::string DETECTED_IMAGES_DIR =   MODEL_PRO_TMP;
const std::string CAMERA_OUTPUT_DIR =     VIDEO_TMP;
const std::string IMAGES_BUFFER_DIR =    VIDEO_TMP;
const std::string SPEECH_OUTPUT_DIR =    SPEECH_TMP;

//FACE RECO PATHS:
const std::string FACE_CASCADE_DEFAULT_PATH = ROS_ROOT + "/node_face/haarcascades/haarcascade_frontalface_alt.xml";
const std::string L_EYE_CASCADE_DEFAULT_PATH = ROS_ROOT + "/node_face/haarcascades/haarcascade_mcs_lefteye.xml";
const std::string R_EYE_CASCADE_DEFAULT_PATH = ROS_ROOT + "/node_face/haarcascades/haarcascade_mcs_righteye.xml";
const std::string DATA_DEFAULT_PATH = ROS_ROOT + "/node_face/people";
const std::string MODEL_DEFAULT_PATH  = ROS_ROOT + "/node_face/people/model.xml";
const std::string GEN_LABELS_PATH = ROS_ROOT + "/node_face";
const std::string FACE_LABELS_PATH = ROS_ROOT + "/node_face/people/image_labels/";

//GUI PATHS:
const std::string SHADER_DIR = ROS_ROOT + "/node_gui/shaders/";
const std::string SCREENSHOT_DIR = ROS_ROOT + "/node_gui/Screenshots/";
const std::string IMAGE_DIR = ROS_ROOT + "/node_gui/images/";
const std::string RESULT_INFO_DIR = ROS_ROOT + "/node_gui/info_images/";
const std::string DUMMY_DIR = ROS_ROOT + "/node_dummy_tester/";

//Constants to use for reporting format:
#define REPORT_IN "REPORT:IN"
#define REPORT_OUT "REPORT:OUT"
#define REPORT_BNET "BNET"
#define REPORT_SPEECH "SPEECH"
#define REPORT_GUI "GUI"
#define REPORT_OBJECT_RECO "OBJECT_RECO"

#define REPORT_SPEECH_MSG "dialog/general"
#define REPORT_FIRED_EXEMPLARS "retail/object_detection_result"
#define REPORT_IMAGE "gui/image_raw"
#define REPORT_RANKED_EXEMPLARS "bnet/ranked_exemplars"
#define REPORT_FINAL_RESULT "bnet/final_result"
#define REPORT_GUI_FEEDBACK "gui/feedback_result"
#define REPORT_TRIGGER "trigger/output"

#define REPORT_FACE_RECO "FACE_RECO"
#define REPORT_FACE_DETECT "FACE_DETECT"
#define REPORT_LOCALIZATION "LOCALIZATION"

void find_and_replace(string& source, string const& find, string const& replace)
{
    for(std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
    {
        source.replace(i, find.length(), replace);
        i += replace.length() - find.length() + 1;
    }
}
//For vision_speech_model
#define SPEECH_TP_NUM 5
#define SPEECH_FP_NUM 1
#define SPEECH_TN_NUM 5
#define SPEECH_FN_NUM 1
#define DETECTOR_TP_NUM 10
#define DETECTOR_FP_NUM 1
#define EXEMPLAR_TP_NUM 5
#define EXEMPLAR_FP_NUM 1
#define EXEMPLAR_TN_NUM 50
#define EXEMPLAR_FN_NUM 50



template <class myType>
void GetROSParameter (const char* param, myType& myParam) 
{
  if (ros::param::has(param)){
    ros::param::get(param,myParam);
    stringstream msg;
    msg << "Found ROS Parameter " << param << " with value " << myParam;
    ROS_INFO(msg.str().c_str());
  }
  else
    ROS_WARN("Could not find parameter %s.", param);
}


#endif
