#include <ros/ros.h>
#include <std_srvs/Empty.h>

int main(int argc, char **argv) {
	
	ros::init(argc, argv, "stop service test");
	ros::NodeHandle nh;
	ros::ServiceClient stopClient = nh.serviceClient<std_srvs::Empty>("image/stop");
	std_srvs::Empty srv;

	if (stopClient.call(srv)) {
		ROS_INFO("Stopped");
	} else {
		ROS_ERROR("Failed to call service ~face/stop");
		return 1;
	}
	return 0;
}
