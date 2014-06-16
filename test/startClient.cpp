#include <ros/ros.h>
#include <std_srvs/Empty.h>

int main(int argc, char **argv) {
	
	ros::init(argc, argv, "start service test");
	ros::NodeHandle nh;
	ros::ServiceClient startClient = nh.serviceClient<std_srvs::Empty>("image/start");
	std_srvs::Empty srv;

	if (startClient.call(srv)) {
		ROS_INFO("Started");
	} else {
		ROS_ERROR("Failed to call service ~face/start");
		return 1;
	}
	return 0;
}
