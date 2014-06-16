/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Kevin J. Walchko.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Kevin  nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Kevin J. Walchko on 6/20/2011
 *********************************************************************
 *
 * Simple camera driver using OpenCV 2.2 interface to grab images.
 *
 * rosrun camera_node opencv_cam _source:=# _size:=#x# _debug:=true/false
 *		where:
 *			source: > 0 selects a camera other than default
 *          size: my MacBook Pro can do 160x120, 320x240, 640x480
 *          fps: my MacBook Pro seems to ignore fps and always gives me ~15 fps
 *
 * Example:
 * rosrun camera_node opencv_cam _debug:=true _size:=160x120 _fps:=30
 *
 * Change Log:
 * 20 June 2011 Created
 *  3 Aug 2011 Major rewrite to use more of the updated methods for E Turtle
 *
 **********************************************************************
 *
 * 
 *
 */


#include <ros/ros.h>
//#include <sensor_msgs/image_encodings.h>
//#include <sensor_msgs/CameraInfo.h> // depreciated
//#include <camera_info_manager/camera_info_manager.h>
//#include <image_transport/image_transport.h> // handles raw or compressed images
//#include <cv_bridge/cv_bridge.h> // switches between cv and ros image formats 
//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>

//#include <iostream>

#include "CameraNode.h"

//using namespace ros;
//using namespace sensor_msgs;
//using namespace cv;
//using namespace cv_bridge;



/**
 * Still having problems with pass commandline args ... the param server
 * remembers the last params you put on it. You need to kill roscore to 
 * clear the param server.
 */
int main(int argc, char** argv)
{
    ros::init(argc, argv, "opencv_cam");
    ros::NodeHandle n("~");
    
    CameraNode ic(n);
    ic.spin();
    
    return 0;
}
