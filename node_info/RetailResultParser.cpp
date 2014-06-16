#include "ros/ros.h"
#include "std_msgs/String.h"
#include <iostream>
#include <fstream>
#include <string>

std::string current_timestamp;
bool got_answer = false;
enum Log_Format{
  TIME_STAMP,
  WORDS_SPOKEN,
  OBJECT,
  WORD_IDENTIFIED,
  BB_X1,
  BB_Y1,
  BB_X2,
  BB_Y2
};

void got_answer_callback( const std_msgs::String::ConstPtr& msg ){
  if( strcmp( msg->data.c_str(), "got it" ) == 0 ){
    got_answer = true;
  }else{
    got_answer = false;
  }
    
}

int main(int argc, char **argv){
  ros::init(argc, argv, "talker");
  ros::NodeHandle nh;

  ros::Publisher gui_info = nh.advertise<std_msgs::String>("retail/object_detection_result", 1000);
  ros::Subscriber objectAnswerSub = nh.subscribe( "bnet/object/got_answer" , 1000, got_answer_callback );

  ros::Rate loop_rate(10);
  std::string filebuffer;
  std::ifstream guifile("/home/mmfps/mmfps/mmfpspkg/data_tmp/gui/log.txt");
  std_msgs::String msg;
  std::string token;
  std::vector<std::string>lines;
  

  while(ros::ok()){
    if( !got_answer ){

      guifile.open("/home/mmfps/mmfps/mmfpspkg/data_tmp/gui/log.txt");
      if( guifile.is_open() ){
	filebuffer.assign( (std::istreambuf_iterator<char>(guifile)),
			   (std::istreambuf_iterator<char>() ) );
	guifile.seekg(0);
	while( getline( guifile, token ) ){
	  //ROS_INFO("token: %s ", token.c_str() );
	  lines.push_back(token);
	}
	//ROS_INFO("lines size: %d", lines.size() );
	if( lines.size() > 0){
	  //ROS_INFO("lines at 0: %s", lines.front().c_str() );
	  msg.data = filebuffer;
	  //ROS_INFO("filebuffer: %s", filebuffer.c_str() );

	  if(current_timestamp.empty()){
	    current_timestamp = lines[TIME_STAMP];
	  }
	  else if(lines[TIME_STAMP].compare(current_timestamp) != 0 && lines.size() >= 8){
	    current_timestamp = lines[TIME_STAMP];
	    gui_info.publish(msg);
	  }
	}
	guifile.close();
	lines.clear();

      }
    }
    ros::spinOnce();
    loop_rate.sleep();
  }
}
